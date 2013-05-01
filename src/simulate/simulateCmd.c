/**CFile***********************************************************************

  FileName    [simulateCmd.c]

  PackageName [simulate]

  Synopsis    [Model Checker Simulator Commands]

  Description [This file contains commands to be used for the simulation feature.]

  SeeAlso     [simulate.c]

  Author      [Andrea Morichetti]

  Copyright   [
  This file is part of the ``simulate'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and ITC-irst. 

  NuSMV version 2 is free software; you can redistribute it and/or 
  modify it under the terms of the GNU Lesser General Public 
  License as published by the Free Software Foundation; either 
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information of NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

******************************************************************************/

#include "simulateInt.h"
#include "utils/error.h" /* for CATCH */
#include "utils/utils_io.h"
#include "prop/prop.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "mc/mc.h"
#include "trace/TraceManager.h"
#include "enc/enc.h"
#include "trace/TraceLabel.h"
#include "cmd/cmd.h"

static char rcsid[] UTIL_UNUSED = "$Id: simulateCmd.c,v 1.14.2.26.2.2 2005/03/17 14:51:21 nusmv Exp $";


/* Prototypes of command functions */
int CommandSimulate   ARGS((int argc, char **argv));
int CommandPickState  ARGS((int argc, char **argv));
int CommandGotoState ARGS((int argc, char **argv));
int CommandPrintCurrentState ARGS((int argc, char **argv));

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [It's used to store the current state when the interactive
  model inspection feature is enabled.]

  Description [It's used to store the current state when the interactive
  model inspection feature is enabled.]

******************************************************************************/
static bdd_ptr current_state_bdd = (bdd_ptr) NULL;


/**Variable********************************************************************

  Synopsis    [It's used to store the current state label when the interactive
  model inspection feature is enabled.]

  Description [It's used to store the current state label when the interactive
  model inspection feature is enabled.]

******************************************************************************/
static TraceLabel current_state_label = TRACE_LABEL_INVALID;


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageSimulate   ARGS((void));
static int UsagePickState  ARGS((void));
static int UsageGotoState  ARGS((void));
static int UsagePrintCurrentState ARGS((void));
static boolean current_state_exist ARGS((void)); 
static void current_state_bdd_set ARGS((bdd_ptr state)); 
static boolean current_state_bdd_exist ARGS((void)); 
static TraceLabel current_state_label_get ARGS((void)); 
static void current_state_label_set ARGS((TraceLabel label)); 
static boolean current_state_label_exist ARGS((void)); 

static void 
simulate_store_and_print_trace ARGS((node_ptr p, boolean printyesno, 
				     boolean only_changes));



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the simulate package.]

  Description        [Initializes the simulate package.]

  SideEffects        []

******************************************************************************/
void Simulate_Init(void)
{
  Cmd_CommandAdd("simulate", CommandSimulate, 0);
  Cmd_CommandAdd("pick_state", CommandPickState, 0);
  Cmd_CommandAdd("goto_state", CommandGotoState, 0);
  Cmd_CommandAdd("print_current_state", CommandPrintCurrentState, 0);
}

/**Function********************************************************************

  Synopsis           [Quits the simulate package]

  Description        [Quits the simulate package]

  SideEffects        []

******************************************************************************/
void Simulate_End(void)
{
  current_state_bdd_free();
  current_state_label_reset();
}

/**Function********************************************************************

  Synopsis           [Picks a state from the set of initial states]

  CommandName        [pick_state]

  CommandSynopsis    [Picks a state from the set of initial states]

  CommandArguments   [\[-h\] \[-v\] \[-r | -i \[-a\]\] \[-c "constraints"\]]

  CommandDescription [

  Chooses an element from the set of initial states, and makes it the
  <tt>current state</tt> (replacing the old one). The chosen state is
  stored as the first state of a new trace ready to be lengthened by
  <tt>steps</tt> states by the <tt>simulate</tt> command. The state can be
  chosen according to different policies which can be specified via command
  line options. By default the state is chosen in a deterministic way.
  <p>
  Command Options:<p>
  <dl>
    <dt> <tt>-v</tt>
       <dd> Verbosely prints out chosen state (all state variables, otherwise
       it prints out only the label <tt>t.1</tt> of the state chosen, where
       <tt>t</tt> is the number of the new trace, that is the number of
       traces so far generated plus one).
    <dt> <tt>-r</tt>
       <dd> Randomly picks a state from the set of initial states.
    <dt> <tt>-i</tt>
       <dd> Enables the user to interactively pick up an initial state. The
       user is requested to choose a state from a list of possible items
       (every item in the list doesn't show state variables unchanged with
       respect to a previous item). If the number of possible states is too
       high, then the user has to specify some further constraints as
       "simple expression".
    <dt> <tt>-a</tt>
       <dd> Displays all state variables (changed and unchanged with respect
       to a previous item) in an interactive picking. This option
       works only if the <tt>-i</tt> options has been specified.
    <dt> <tt>-c "constraints"</tt>
       <dd> Uses <tt>constraints</tt> to restrict the set of initial states
       in which the state has to be picked. <tt>constraints</tt> must be
       enclosed between double quotes <tt>" "</tt>.
  </dl> ]

  SideEffects        [The state chosen is stored in the traces_hash table as
  the first state of a new trace]

  SeeAlso            [goto_state simulate]

******************************************************************************/
int CommandPickState(int argc, char ** argv)
{
  int res;
  int c = 0;
  boolean verbose = false;
  int display_all = 0;
  char *strConstr = NIL(char);
  Simulation_Mode mode = Deterministic;
  short int usedMode = 0;


  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hriavc:")) != EOF) {
    switch(c){
    case 'h': return UsagePickState();
    case 'r':
      if (++usedMode > 1) return UsagePickState();
      mode = Random;
      break;
    case 'i':
      if (++usedMode > 1) return UsagePickState();
      mode = Interactive;
      break;
    case 'a':
      display_all = 1;
      break;
    case 'v':
      verbose = true;
      break;
    case 'c':
      strConstr = util_strsav(util_optarg);
      break;
    default: return UsagePickState();
    }
  }

  if ((mode != Interactive) && (display_all == 1)) return UsagePickState();
  if (argc != util_optind) return UsagePickState();

  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr, "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr, "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }
  if (cmp_struct_get_build_model(cmps) == 0) {
    fprintf(nusmv_stderr, "A model must be built before. Use the \"build_model\" command.\n");
    return 1;
  }

  res = Simulate_CmdPickOneState(Prop_master_get_bdd_fsm(), mode,
                                 display_all, strConstr);

  if (verbose == true) Cmd_CommandExecute("print_current_state -v");
  return res;
}

static int UsagePickState () {
  fprintf(nusmv_stderr, "usage: pick_state [-h] [-v] [-r | -i [-a]] [-c \"constr\"]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -v \t\tVerbosely prints picked state.\n");
  fprintf(nusmv_stderr, "  -r \t\tRandomly picks a state from the set of the initial states\n");
  fprintf(nusmv_stderr, "     \t\t(otherwise choice is deterministic).\n");
  fprintf(nusmv_stderr, "  -i \t\tLets the user interactively pick a state from\n");
  fprintf(nusmv_stderr, "     \t\tthe set of initial ones.\n");
  fprintf(nusmv_stderr, "  -a \t\tDisplays all the state variables (changed and\n");
  fprintf(nusmv_stderr, "   \t\tunchanged) in an interactive session.\n");
  fprintf(nusmv_stderr, "   \t\tIt works only together with -i option.\n");
  fprintf(nusmv_stderr, "  -c \"constr\"   Sets constraints for the initial set of states.\n");
  fprintf(nusmv_stderr, "     \t\tNote: must be enclosed between double quotes \" \".\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Performs a simulation from the current selected state]

  SideEffects        [Generated referenced states traces are stored to be
  analyzed by the user in a second time]

  SeeAlso            [pick_state goto_state]

  CommandName        [simulate]

  CommandSynopsis    [Performs a simulation from the current selected state]

  CommandArguments   [\[-h\] \[-p | -v\] \[-r | -i \[-a\]\] 
  \[-c "constraints"\] steps
  ]

  CommandDescription [
  Generates a sequence of at most <tt>steps</tt> states (representing a
  possible execution of the model), starting from the <em>current state</em>. 
  The current state must be set via the <em>pick_state</em> or 
  <em>goto_state</em> commands.<p> 
  It is possible to run the simulation in three ways (according to different 
  command line policies):
  deterministic (the default mode), random and interactive.<p>
  The resulting sequence is stored in a trace indexed with an integer number
  taking into account the total number of traces stored in the system. There is
  a different behavior in the way traces are built, according to how 
  <em>current state</em> is set: <em>current state</em> is always put at 
  the beginning of a new trace (so it will contain at most <it>steps + 1</it> 
  states) except when it is the last state of an existent old trace. 
  In this case the old trace is lengthened by at most <it>steps</it> states.
  <p>
  Command Options:<p>
  <dl>
    <dt> <tt>-p</tt>
       <dd> Prints current generated trace (only those variables whose value 
       changed from the previous state).
    <dt> <tt>-v</tt>
       <dd> Verbosely prints current generated trace (changed and unchanged 
       state variables).
    <dt> <tt>-r</tt>
       <dd> Picks a state from a set of possible future states in a random way.
    <dt> <tt>-i</tt>
       <dd> Enables the user to interactively choose every state of the trace,
       step by step. If the number of possible states is too high, then 
       the user has to specify some constraints as simple expression. 
       These constraints are used only for a single simulation step and 
       are <em>forgotten</em> in the following ones. They are to be intended 
       in an opposite way with respect to those constraints eventually entered 
       with the pick_state command, or during an interactive simulation 
       session (when the number of future states to be displayed is too high),
       that are <em>local</em> only to a single step of the simulation and 
       are <em>forgotten</em> in the next one.
    <dt> <tt>-a</tt>
       <dd> Displays all the state variables (changed and unchanged) during 
       every step of an interactive session. This option works only if the
       <tt>-i</tt> option has been specified.
    <dt> <tt>-c "constraints"</tt>
       <dd> Performs a simulation in which computation is restricted to states
       satisfying those <tt>constraints</tt>. The desired sequence of states 
       could not exist if such constraints were too strong or it may happen 
       that at some point of the simulation a future state satisfying those
       constraints doesn't exist: in that case a trace with a number of 
       states less than <tt>steps</tt> trace is obtained.
       Note: <tt>constraints</tt> must be enclosed between double quotes 
       <tt>" "</tt>.
    <dt> <tt>steps</tt>
       <dd> Maximum length of the path according to the constraints. 
       The length of a trace could contain less than <tt>steps</tt> states: 
       this is the case in which simulation stops in an intermediate 
       step because it may not exist any future state satisfying those 
       constraints.
    </dl> ]

******************************************************************************/
int CommandSimulate(int argc, char **argv)
{
  BddEnc_ptr enc;
  bdd_ptr bdd_constraints = (bdd_ptr) NULL;
  boolean isconstraint = false;
  boolean printrace = false;
  int display_all = 0;
  int c = 0;
  boolean only_changes = 1;
  int steps = 0;
  char * strConstr[2]; /* the string array of constraints to parsificate */
  Simulation_Mode mode = Deterministic;


  strConstr[0] = NIL(char);
  util_getopt_reset();
  while((c = util_getopt(argc,argv,"c:hpvria")) != EOF){
    switch(c){
    case 'h': return UsageSimulate();
    case 'p':
      if (printrace == true) return UsageSimulate();
      printrace = true;
      only_changes = true;
      break;
    case 'v':
      if (printrace == true) return UsageSimulate();
      printrace = true;
      only_changes = false;
      break;
    case 'r':
      if (mode == Interactive) return UsageSimulate();
      mode = Random;
      break;
    case 'i':
      if (mode == Random) return UsageSimulate();
      mode = Interactive;
      break;
    case 'a':
      display_all = 1;
      break;
    case 'c':
      isconstraint = true;
      strConstr[1] = util_strsav(util_optarg);
      break;
    default:
      return UsageSimulate();
    }
  }

  if ((mode != Interactive) && (display_all == 1)) return UsageSimulate();

  /* controls of commands hierarchy */
  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }
  if (cmp_struct_get_build_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be built before. Use the \"build_model\" command.\n");
    return 1;
  }

  if (!cmp_struct_get_build_model(cmps)) {
    fprintf(nusmv_stderr,
	    "The current partition method %s has not yet be computed.\n",
	    TransType_to_string(get_partition_method(options)));
    fprintf(nusmv_stderr, "Use \t \"build_model -f -m %s\"\nto build the transition relation.\n", 
	    TransType_to_string(get_partition_method(options)));
    return 1;
  }

  if ((argc - util_optind) != 1) return UsageSimulate(); 

  if (!(current_state_exist())) {
    fprintf(nusmv_stderr,
            "No current state set. Use the \"pick_state\" command.\n");
    return 1;
  }

  enc = Enc_get_bdd_encoding();

  if (isconstraint == true) {
    node_ptr parsed_command = Nil;
    
    if (Parser_ReadCmdFromString(2, strConstr, "CONSTRAINT ", ";\n",
                                 &parsed_command) == 0) {
      /* checks for input presence (not allowed in constraints) */
      CATCH {
	Encoding_ptr senc;
	NodeList_ptr vars;
	
	senc = BddEnc_get_symbolic_encoding(enc);
	vars = Compile_get_expression_dependencies(senc, car(parsed_command));

	if (Encoding_list_contains_input_vars(senc, vars)) {
	  /* input vars are not allowed */
	  fprintf(nusmv_stderr,
		  "Error: constraints cannot contain input variables.\n");
	  return 1;
	}
      }
      FAIL {
	/* an error occurred here (likely it is an undefined symbol) */
	return 1;
      }      

      CATCH {
        if ((parsed_command != Nil) &&
            (node_get_type(parsed_command) == CONSTRAINT)) {
          node_ptr constraints = car(parsed_command);
	  bdd_constraints = BddEnc_expr_to_bdd(enc, constraints, Nil);
        }
      } /* end CATCH */
      FAIL {
        fprintf(nusmv_stderr,
                "Parsing error: constraints have to be \"simple_expr\".\n");
        return 1;
      }
    }
    else {
      fprintf(nusmv_stderr,
              "Parsing error: constraints have to be \"simple_expr\".\n");
      return 1;
    }
  }
  else {
    bdd_constraints = bdd_one(dd_manager);
  }

  {
    char *err_occ[1];
    char *steps_no_str;

    err_occ[0] = "";

    steps_no_str = util_strsav(argv[util_optind]);
 
    steps = strtol(steps_no_str, err_occ, 10);
    if  ((strcmp(err_occ[0], "") != 0)) {
      fprintf(nusmv_stderr, \
              "Error: \"%s\" is not a valid value (must be a positive integer).\n",
              err_occ[0]);
      bdd_free(dd_manager, bdd_constraints);
      return 1;
    }
  }
  {
    node_ptr current_trace = Nil;
    BddFsm_ptr fsm;
    TraceLabel curr_lbl; 

    fsm = Prop_master_get_bdd_fsm();
    
    curr_lbl = current_state_label_get();
    nusmv_assert(curr_lbl != TRACE_LABEL_INVALID);

    fprintf( nusmv_stdout, "********  Simulation Starting From State %d.%d "
	     "  ********\n", 
	     TraceLabel_get_trace(curr_lbl) + 1, 
	     TraceLabel_get_state(curr_lbl) + 1);

    /* Important: the simulation ALWAYS starts from the current selected state */
    current_trace = Simulate_MultipleSteps(fsm, enc, bdd_constraints, mode, steps, 
					   display_all);
    if (current_trace == Nil) {
      bdd_free(dd_manager, bdd_constraints);
      return 1;
    }

    /* prints the trace and inserts it in the traces hash, then frees it*/
    simulate_store_and_print_trace(current_trace, printrace, only_changes);

    /* Update the current state. */
    {
      int trace_id;
      Trace_ptr curr_trace;
      BddStates curr_state;
      TraceLabel new_label;

      trace_id = TraceManager_get_current_trace_number(global_trace_manager);

      curr_trace = TraceManager_get_trace_at_index(global_trace_manager,
                                                   trace_id);
      curr_state = Trace_get_ith_state(curr_trace,
                                       Trace_get_length(curr_trace));

      new_label = TraceLabel_create(trace_id, Trace_get_length(curr_trace));
      
      current_state_set(curr_state, new_label);

      bdd_free(BddEnc_get_dd_manager(Trace_get_enc(curr_trace)),curr_state);
    }

    walk_dd(dd_manager, bdd_free, current_trace);
    bdd_free(dd_manager, bdd_constraints);
  }
  return 0;
}

static int UsageSimulate(){
  fprintf(nusmv_stderr, "usage: simulate [-h] [-p | -v] [-r | -i [-a]] [-c \"constr\"] steps\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -p \t\tPrints current generated trace (only changed variables).\n");
  fprintf(nusmv_stderr, "  -v \t\tVerbosely prints current generated trace (all variables).\n");
  fprintf(nusmv_stderr, "  -r \t\tSets picking mode to random (default is deterministic).\n");
  fprintf(nusmv_stderr, "  -i \t\tEnters simulation's interactive mode.\n");
  fprintf(nusmv_stderr, "  -a \t\tDisplays all the state variables (changed and unchanged)\n");
  fprintf(nusmv_stderr, "     \t\tin every step of an interactive session.\n");
  fprintf(nusmv_stderr, "     \t\tIt works only together with -i option.\n");
  fprintf(nusmv_stderr, "  -c \"constr\"\tSets constraint (simple expression) for the next steps.\n");
  fprintf(nusmv_stderr, "     \t\tNote: must be enclosed between double quotes \" \".\n");
  fprintf(nusmv_stderr, "  steps \tMaximum length of the path.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Goes to a given state of a trace]

  CommandName        [goto_state]          

  CommandSynopsis    [Goes to a given state of a trace]  

  CommandArguments   [\[-h\] state]  

  CommandDescription [Makes <tt>state</tt> the <em>current
  state</em>. This command is used to navigate alongs traces
  produced by NuSMV. During the navigation, there is a <em>current
  state</em>, and the <em>current trace</em> is the trace the
  <em>current state</em> belongs to.]  

  SideEffects        [<em>state</em> became the current state.]

******************************************************************************/

int CommandGotoState(int argc, char **argv)
{
  int c;
  int status = 0;
  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"h")) != EOF) {
    switch (c) {
    case 'h': return UsageGotoState();
    default:  return UsageGotoState();
    }
  }
  if (argc == 1) return UsageGotoState();

  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }

  if (!cmp_struct_get_build_model(cmps)) {
    fprintf(nusmv_stderr, "The current partition method %s has not yet be computed.\n", 
	    TransType_to_string(get_partition_method(options)));
    fprintf(nusmv_stderr, "Use \t \"build_model -f -m %s\"\nto build the transition relation.\n", 
	    TransType_to_string(get_partition_method(options)));
    return 1;
  }

  {
    TraceLabel label;

    argv += util_optind-1;
    argc -= util_optind-1;
    label = TraceLabel_create_from_string(argv[1]);

    if (label != TRACE_LABEL_INVALID) {
      if (TraceManager_is_label_valid(global_trace_manager, label)) {
        Trace_ptr from_trace, new_trace;
        TraceIterator_ptr iter;
        bdd_ptr state;
        int new_trace_id;
        node_ptr new_label;
        int from_trace_no = TraceLabel_get_trace(label);
        int from_state_no = TraceLabel_get_state(label);
        BddEnc_ptr enc = Enc_get_bdd_encoding();

        from_trace = TraceManager_get_trace_at_index(global_trace_manager, 
						     from_trace_no);
        iter = TraceManager_get_iterator_from_label(global_trace_manager, 
						    label);
        state = TraceNode_get_state(Trace_get_node(from_trace, iter));
        new_trace = Trace_copy_prefix_until_iterator(from_trace, iter);

        /* Now the new label we get would be the label of the next
         * trace that is going to be registered. */

        new_label = TraceLabel_create(
		       TraceManager_get_size(global_trace_manager),
		       from_state_no);

        new_trace_id = TraceManager_register_trace(global_trace_manager, 
						   new_trace);
        TraceManager_set_current_trace_number(global_trace_manager, 
					      new_trace_id);
        current_state_set(state, new_label);

        fprintf(nusmv_stdout, "The current state for new trace is:\n");
        fprintf(nusmv_stdout, "-> State %d.%d <-\n", 
		TraceLabel_get_trace(new_label)+1, 
		TraceLabel_get_state(new_label)+1);

        BddEnc_print_bdd_begin(enc,
                      SexpFsm_get_vars_list(Prop_master_get_scalar_sexp_fsm()),
                      true);
        set_indent_size(2);
        BddEnc_print_bdd(enc, state, nusmv_stdout); 
        reset_indent_size();
        BddEnc_print_bdd_end(enc);
      }
      else {
	fprintf(nusmv_stderr, "The label %d.%d is undefined.\n", 
		TraceLabel_get_trace(label) + 1, 
		TraceLabel_get_state(label) + 1);
      }
    }
    else {
      fprintf(nusmv_stderr, "Parsing error: expected \"goto_state state\".\n");
      status = 1;
    }
  }
  return status;
}

static int UsageGotoState()
{
  fprintf(nusmv_stderr, "usage: goto_state [-h] state\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   state \tSets current state to \"state\".\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Prints the current state]

  CommandName        [print_current_state]         

  CommandSynopsis    [Prints out the current state]  

  CommandArguments   [\[-h\] \[-v\]]  

  CommandDescription [Prints the name of the <em>current state</em> if
  defined.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-v</tt>
       <dd> Prints the value of all the state variables of the <em>current 
       state</em>.
  </dl>
  ]  

  SideEffects        []

******************************************************************************/
int CommandPrintCurrentState(int argc, char ** argv)
{
  int c;
  int Verbosely = 1;
  
  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"hv")) != EOF) {
    switch (c) {
    case 'h': return UsagePrintCurrentState();
    case 'v': {
      Verbosely = 0;
      break;
    }
    default:  return UsagePrintCurrentState();
    }
  }

  if (argc != util_optind) return UsagePrintCurrentState();

  if ((current_state_label != TRACE_LABEL_INVALID) && 
      (current_state_bdd != (bdd_ptr)NULL)) {
    BddEnc_ptr enc = Enc_get_bdd_encoding();

    fprintf(nusmv_stdout, "Current state is %d.%d\n", 
	    TraceLabel_get_trace(current_state_label) + 1, 
	    TraceLabel_get_state(current_state_label) + 1);

    if (Verbosely == 0) {
      BddEnc_print_bdd_begin(enc,
                    SexpFsm_get_vars_list(Prop_master_get_scalar_sexp_fsm()),
                    false);
					     
      BddEnc_print_bdd(enc, current_state_bdd, nusmv_stdout);
      BddEnc_print_bdd_end(enc);
    }
  }
  else {
    if (TraceManager_get_current_trace_number(global_trace_manager) >= 0) {
      fprintf(nusmv_stdout, "The current state has not yet been defined.\n");
      fprintf(nusmv_stdout, 
	      "Use \"goto_state\" to define the current state.\n");
    }
    else {
      fprintf(nusmv_stdout, 
	      "There is no trace actually stored in the system.\n");
      fprintf(nusmv_stdout, 
	      "Use \"pick_state\" to define the current state.\n");
    }
    return 1;
  }
  return 0;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

bdd_ptr current_state_bdd_get(void) 
{
  return (current_state_exist() == true) ? 
    bdd_dup(current_state_bdd) : (bdd_ptr) NULL;
}

void current_state_bdd_reset(void) 
{
  if (current_state_bdd_exist()) current_state_bdd = (bdd_ptr) NULL;
}

void current_state_set(bdd_ptr state, TraceLabel label) 
{
  current_state_bdd_set(state);
  current_state_label_set(label);
}

void current_state_label_reset(void) 
{
  if (current_state_label_exist()) current_state_label = TRACE_LABEL_INVALID;
}

void current_state_bdd_free(void) 
{
  if (current_state_bdd_exist()) {
    bdd_free(dd_manager, current_state_bdd);
  }
}
 

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

static int UsagePrintCurrentState()
{
  fprintf(nusmv_stderr, "usage: print_current_state [-h] [-v]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, 
	  "   -v \t\tPrints the value of each state variable in the current state.\n");
  return 1;
}


static boolean current_state_exist(void) 
{
  return ((current_state_label_exist() == true) && 
	  (current_state_bdd_exist() == true));
}

static void current_state_bdd_set(bdd_ptr state) 
{
  if (current_state_exist() == true) { current_state_bdd_free(); }
  current_state_bdd = bdd_dup(state);
}

static boolean current_state_bdd_exist(void) 
{
  return (current_state_bdd != (bdd_ptr)NULL);
}

static TraceLabel current_state_label_get(void) 
{
  return (current_state_exist() == true) ? 
    current_state_label : TRACE_LABEL(NULL);
}

static void current_state_label_set(TraceLabel label) 
{ 
  current_state_label = label;
}

static boolean current_state_label_exist(void) 
{
  return (current_state_label != TRACE_LABEL_INVALID);
}


/**Function********************************************************************

  Synopsis           [Stores a trace in the hash table and prints it.]

  Description        [Stores a trace in the hash table and prints it if the
  variable printyesno is true (this is set by the user via the command simulate
  options -p or -v). It returns the index of the stored trace inside the
  trace-manager.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void simulate_store_and_print_trace(node_ptr p, boolean printyesno, 
					   boolean only_changes)
{
  int trace_id;
  Trace_ptr trace;
  TraceIterator_ptr iter;

  trace_id = TraceManager_get_current_trace_number(global_trace_manager);
  trace = TraceManager_get_trace_at_index(global_trace_manager, trace_id);
  iter = Trace_get_iterator(trace, Trace_get_last_node(trace));

  Trace_append_from_list_input_state(trace, p); 

  if (opt_verbose_level_gt(options, 1) && printyesno) {
    fprintf(nusmv_stdout,
	    "#####################################################\n"
	    "######         Print Of Current Trace         #######\n"
	    "#####################################################\n");
  }

  if (printyesno) {
    TracePlugin_ptr plugin;
    /* only the TraceExplainer plugin can be used here: */
    if (only_changes) {
      plugin = TraceManager_get_plugin_at_index(global_trace_manager, 0);
    } 
    else {
      plugin = TraceManager_get_plugin_at_index(global_trace_manager, 1);
    } 

    TracePlugin_action(plugin, trace, iter);
  }

  return ;
}
