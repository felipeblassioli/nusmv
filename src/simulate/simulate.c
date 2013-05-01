/**CFile***********************************************************************

  FileName    [simulate.c]

  PackageName [simulate]

  Synopsis    [Simulator routines]

  Description [This file contains functions for image computation, for state
        picking in a set (according to three different policies), for states display
        and constraints insertion.]

  Author      [Andrea Morichetti, Roberto Cavada]

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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "simulate.h"
#include "simulateInt.h"
#include "simulateTransSet.h"
#include "utils/error.h" /* for CATCH */
#include "utils/utils_io.h"
#include "mc/mc.h"
#include "parser/symbols.h"
#include "trace/Trace.h"
#include "trace/TraceManager.h"
#include "enc/enc.h"
#include "trace/TraceLabel.h"

static char rcsid[] UTIL_UNUSED = "$Id: simulate.c,v 1.13.2.36.2.3 2005/11/16 12:09:47 nusmv Exp $";


/**Variable********************************************************************

  Synopsis    [Stack and context for non-local goto inside simulator]

  Description [Stack and context for non-local goto inside simulator]

******************************************************************************/
#if SOLARIS
static sigjmp_buf simulate_jmp_buff;
#else
static jmp_buf simulate_jmp_buff;
#endif

static  void (*saved_simulate_sigterm)(int);

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static bdd_ptr simulate_accumulate_constraints ARGS((BddEnc_ptr enc, bdd_ptr bdd, 
						     int max_size));
static bdd_ptr simulate_request_constraints ARGS((BddEnc_ptr enc));

static void simulate_sigterm ARGS((int));

static void simulate_choose_next ARGS((BddFsm_ptr fsm, 
				       BddEnc_ptr enc, 
				       bdd_ptr from_state, 
				       bdd_ptr next_state_set, 
				       Simulation_Mode mode, 
				       int display_all, 
				       bdd_ptr* which_input, 
				       bdd_ptr* which_state));


/**Function********************************************************************

  Synopsis           [Multiple step simulation]

  Description        [Multiple step simulation: loops n times over the choice of
  a state according to the picking policy given at command line. It returns a
  list of at least n+1 referenced states (the first one is always the "current
  state" from which any simulation must start). The obtained list can contain
  a minor number of states if there are no future states at some point.]

  SideEffects        []

  SeeAlso            [Simulate_ChooseOneState]

******************************************************************************/
node_ptr Simulate_MultipleSteps(BddFsm_ptr fsm, BddEnc_ptr enc, 
				bdd_ptr constraint,
                                Simulation_Mode mode, int n, int display_all)
{
  DdManager* dd = BddEnc_get_dd_manager(enc);
  bdd_ptr current_state;
  bdd_ptr input;
  bdd_ptr state;
  bdd_ptr mask;
  node_ptr result = Nil;
  int i = 1;

  current_state = current_state_bdd_get();
  /* we append the current state in first position of the trace */
  result = cons((node_ptr) current_state, result);
  mask = BddEnc_get_state_vars_mask_bdd(Enc_get_bdd_encoding());
  while (i <= n) {
    bdd_ptr next_constr_set = (bdd_ptr)NULL;

    if (opt_verbose_level_gt(options, 1)) {
      switch (mode) {
      case Interactive:
        fprintf(nusmv_stderr,
                "********  Interactive mode Simulation step %d  *********\n",i);
        break;
      case Random:
        fprintf(nusmv_stderr,
                "**********  Random mode Simulation step %d  **********\n",i);
        break;
      case Deterministic:
        fprintf(nusmv_stderr,
                "*******  Deterministic mode Simulation step %d  *******\n",i);
        break;
      }
    }
    next_constr_set = BddFsm_get_forward_image(fsm, current_state);
    
    bdd_and_accumulate(dd, &next_constr_set, mask);
    bdd_and_accumulate(dd, &next_constr_set, constraint);
    
    
    if (bdd_is_zero(dd, next_constr_set)) {
      fprintf(nusmv_stderr, "No future state exists");
      fprintf(nusmv_stderr, (llength(result) == 1 ? ": trace not built.\n" : "."));
      fprintf(nusmv_stderr, "Simulation stopped at step %d.\n", i);
      bdd_free(dd, next_constr_set);
      result = reverse(result);
      /* We don't free the current_state variable because the
         list "result" must contain referenced states */
      return(result);
    }

    Simulate_ChooseOneStateInput(fsm, enc, 
				 current_state, next_constr_set, 
				 mode, display_all, 
				 &input, &state);

    if (state == (bdd_ptr) NULL || bdd_is_zero(dd, state)) {
      fprintf(nusmv_stderr,
        "\nCan't find a future state. Simulation stopped at step %d.\n", i);
      if (state != (bdd_ptr) NULL) { bdd_free(dd, state); }
      bdd_free(dd, next_constr_set);
      result = reverse(result);
      return result;
    }

    if (input != (bdd_ptr) NULL) {
      result = cons((node_ptr) input, result);
    } 
    else {
      /* there are no inputs */
      result = cons((node_ptr) bdd_one(dd), result);
    }

    result = cons((node_ptr) state, result);

    /* we don't free the current_state variable because the final list "result"
       must contain referenced states: they will be freed after their insertion
       in the traces_hash table */
    i++;
    bdd_free(dd, next_constr_set);
    current_state = state;
  }

  result = reverse(result);
  return result;
}

/**Function********************************************************************

  Synopsis           [Chooses one state among future states]

  Description        [Chooses a state among future states depending on the
  given simulation policy (random, deterministic or interactive). In case of
        interactive simulation, the system stops and allows the user to pick
        a state from a list of possible items. If the number of future states
        is too high, the system requires some further constraints to limit that
        number and will asks for them until the number of states is lower than
        an internal threshold. Entered expressions are accumulated in one big
        constraint used only in the actual step of the simulation. It will be
        discarded after a state will be chosen.]

  SideEffects        [A referenced state (BDD) is returned. NULL if failed.]

  SeeAlso            [Simulate_MultipleStep]

******************************************************************************/
bdd_ptr Simulate_ChooseOneState(BddFsm_ptr fsm, 
				BddEnc_ptr enc, 
				bdd_ptr next_set, Simulation_Mode mode,
				int display_all)
{
  bdd_ptr state; 
  bdd_ptr dummy_input;  

  simulate_choose_next(fsm, enc, NULL, next_set, mode, display_all, 
		       &dummy_input, &state); 

  nusmv_assert(dummy_input == (bdd_ptr) NULL);
  return state;
}



/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Simulate_ChooseOneStateInput(BddFsm_ptr fsm, 
				  BddEnc_ptr enc, 
				  bdd_ptr from_state, 
				  bdd_ptr next_set, Simulation_Mode mode,
				  int display_all, 
				  bdd_ptr* input,  
				  bdd_ptr* state)
{
  simulate_choose_next(fsm, enc, from_state, next_set,
		       mode, display_all, 
		       input, state);
}




/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
int Simulate_CmdPickOneState(BddFsm_ptr fsm, Simulation_Mode mode,
                             int display_all, char * strConstr)
{  
  BddEnc_ptr enc = Enc_get_bdd_encoding();
  DdManager* dd = BddEnc_get_dd_manager(enc);
  bdd_ptr initial_set;
  bdd_ptr chosen = (bdd_ptr) NULL;
  int i;

  {
    bdd_ptr init_bdd = BddFsm_get_init(fsm);
    bdd_ptr invar_bdd = BddFsm_get_state_constraints(fsm);
    bdd_ptr tmp;

    tmp = bdd_and(dd, init_bdd, invar_bdd);    
    initial_set = BddEnc_apply_state_vars_mask_bdd(enc, tmp);

    bdd_free(dd, tmp);
    bdd_free(dd, init_bdd);
    bdd_free(dd, invar_bdd);
  }

  if (strConstr != NIL(char)) {
    node_ptr parsed = Nil;
    char* arg[2];

    arg[0] = NIL(char);
    arg[1] = strConstr;

    if ((Parser_ReadCmdFromString(2, arg, 
				  "CONSTRAINT ", ";\n", &parsed) == 0) &&
	(parsed != Nil) && (node_get_type(parsed) == CONSTRAINT)) {
      node_ptr constraints;
      bdd_ptr bdd_constraints;

      /* verify wrt input that are not allowed in constraints: */
      CATCH {
	Encoding_ptr senc;
	NodeList_ptr vars;
	
	senc = BddEnc_get_symbolic_encoding(enc);
	vars = Compile_get_expression_dependencies(senc, car(parsed));

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

      /* converts constratins to BDDs: */
      CATCH {
	constraints = car(parsed);
	bdd_constraints = BddEnc_expr_to_bdd(enc, constraints, Nil);
	bdd_and_accumulate(dd, &initial_set, bdd_constraints);
	bdd_free(dd, bdd_constraints);
      }
      FAIL {
	fprintf(nusmv_stderr, "Parsing error: constraints must be "
		"\"simple expressions\".\n");
	bdd_free(dd, initial_set);
	return 1;
      }	
    }
    else {
      fprintf(nusmv_stderr, "Parsing error: constraints must be "
	      "\"simple expressions\".\n");
      bdd_free(dd, initial_set);
      return 1;
    }
  } /* end of constraints processing */

  i = BddEnc_count_states_of_bdd(enc, initial_set);

  if (i == 0) {
    fprintf(nusmv_stderr, "The set of initial states is EMPTY. "
	    "No state can be chosen.\n");
    bdd_free(dd, initial_set);
    return 1;
  }

  chosen = Simulate_ChooseOneState(fsm, enc, initial_set, mode, display_all);
  bdd_free(dd, initial_set);

  if ((chosen == (bdd_ptr) NULL) || bdd_is_zero(dd, chosen)) {
    fprintf(nusmv_stderr, "Chosen state is the EMPTY set. "
	    "No state has been chosen.\n");
    if (chosen != (bdd_ptr) NULL) bdd_free(dd, chosen);
    return 1;
  }
  else {
    TraceLabel label;
    Trace_ptr trace;
    int trace_id;

    /* Now the new label we get would be the label of the next trace that
     * is going to be registered. */
    label = TraceLabel_create(TraceManager_get_size(global_trace_manager), 0);;
    current_state_set(chosen, label);
    /* picked state is set as the 'current_state' and as the first state
       of a new trace */
    trace = Trace_create(enc, "Simulation Trace", 
			 TRACE_TYPE_SIMULATION, chosen);

    trace_id = TraceManager_register_trace(global_trace_manager, trace);
    TraceManager_set_current_trace_number(global_trace_manager, trace_id);

    bdd_free(dd, chosen);
  }

  return 0;
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



/**Function********************************************************************

  Synopsis           []

  Description [from_state can be NULL from the initial set of states.
  At the end which_input will contained the chosen input (if any, NULL
  otherwise) and which_state will contain the chosen state]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void simulate_choose_next(BddFsm_ptr fsm, 
				 BddEnc_ptr enc, 
				 bdd_ptr from_state, 
				 bdd_ptr next_state_set, 
				 Simulation_Mode mode, 
				 int display_all, 
				 bdd_ptr* which_input, 
				 bdd_ptr* which_state)
{
  DdManager* dd = BddEnc_get_dd_manager(enc);
  int i;  

  *which_state = NULL;
  *which_input = NULL;

  if (mode == Interactive) {
    SimulateTransSet_ptr trans_set = SIMULATE_TRANS_SET(NULL);

    /* to detect ctrl+c interrupt */
    saved_simulate_sigterm = signal(SIGINT, simulate_sigterm);
    if (SETJMP(simulate_jmp_buff, 1) == 0) {
      double states_count;
      int max_choice;
      int choice = 0;
      bdd_ptr constraints;
      bdd_ptr constrained_next_set; 

      /* if required, this will ask for further constraints: */
      constraints = simulate_accumulate_constraints(enc, next_state_set, 
					     opt_shown_states_level(options));
      
      bdd_and_accumulate(dd, &constraints, next_state_set);
      constrained_next_set = BddEnc_apply_state_vars_mask_bdd(enc, constraints);
      bdd_free(dd, constraints);
      
      states_count = BddEnc_count_states_of_bdd(enc, constrained_next_set);    
      trans_set = SimulateTransSet_create(fsm, enc, from_state, 
					  constrained_next_set, states_count);
      bdd_free(dd, constrained_next_set);    

      max_choice = SimulateTransSet_print(trans_set, (display_all == 0), 
					  nusmv_stdout);
      if (max_choice > 0) {
	char line[CHOICE_LENGTH];

	for (i=0; i<CHOICE_LENGTH; i++) line[i] = '\0';
	fprintf(nusmv_stdout,
		"\nChoose a state from the above (0-%d): ", max_choice);
	
        while ( (fgets(line, CHOICE_LENGTH, nusmv_stdin) != (char) NULL) ||
		(line[0] != '\n') ) {
          if ((sscanf(line, "%d", &choice) != 1) || (choice < 0) ||
	      (choice > max_choice)) {
            fprintf(nusmv_stdout, 
		    "Choose a state from the above (0-%d): ", max_choice);
            continue;
          }
          else break;
        }	
      }
      else {
	/* there is only one possible choice here: */
	fprintf(nusmv_stdout,
                "\nThere's only one available state. Press Return to Proceed.");
        while ((choice = getc(nusmv_stdin)) != '\n') ; /* consumes chars */
        choice = 0;
      }
      
      fprintf(nusmv_stdout, "\nChosen state is: %d\n", choice);
      SimulateTransSet_get_state_input_at(trans_set, choice, 
					  which_state, which_input);
    } /* setjump */

    /* construction might be failed, so the instance is checked before
       being destroyed: */
    if (trans_set != SIMULATE_TRANS_SET(NULL)) {
      SimulateTransSet_destroy(trans_set);
    }

  } /* if mode is interactive */

  else { /* random and deterministic modes: */
    bdd_ptr next_state;
    bdd_ptr input = (bdd_ptr) NULL;

    nusmv_assert( (mode == Random) || (mode == Deterministic) );

    if (mode == Random) {
      next_state = BddEnc_pick_one_state_rand(enc, next_state_set);
      
      if (from_state != (bdd_ptr) NULL) {
	bdd_ptr inputs, masked_inputs;
	
	inputs = BddFsm_states_to_states_get_inputs(fsm, from_state, next_state);
	masked_inputs = BddEnc_apply_input_vars_mask_bdd(enc, inputs);
	bdd_free(dd, inputs);

	input = BddEnc_pick_one_input_rand(enc, masked_inputs);
	bdd_free(dd, masked_inputs);
      }
    }
    else {  /* deterministic */
      next_state = BddEnc_pick_one_state(enc, next_state_set);

      if (from_state != (bdd_ptr) NULL) {      
	bdd_ptr inputs, masked_inputs;

	inputs = BddFsm_states_to_states_get_inputs(fsm, from_state, next_state);
	masked_inputs = BddEnc_apply_input_vars_mask_bdd(enc, inputs);
	bdd_free(dd, inputs);

	input = BddEnc_pick_one_input(enc, masked_inputs);
	bdd_free(dd, masked_inputs); 
      }
    }

    *which_state = next_state;
    *which_input = input;
  }
}


/**Function********************************************************************

  Synopsis           [Signal handler]

  Description        [SIGINT signal handler inside the simulator.]

  SideEffects        []

******************************************************************************/
static void simulate_sigterm(int sig) {
  signal(SIGINT, saved_simulate_sigterm);
  LONGJMP(simulate_jmp_buff, 1);
}



/**Function********************************************************************

  Synopsis           [required]

  Description       [There are 4 condition to be verified in order to accept
        new further constraints:

        1) entered expression must be a non-zero set;
        
	2) entered expression must be consistent with the accumulated
        constraints (i.e. the product (further /\ accumulated) must be
        non-zero;
        
	3) if (further /\ accumulated) is non-zero, it also must be
        non-zero the product (further /\ accumulated) /\ next_set of
        states
        
	4) cardinality of the set obtained from the last product must
	   be <= shown_states 
        
        ]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static bdd_ptr simulate_accumulate_constraints(BddEnc_ptr enc, bdd_ptr bdd, 
					       int max_size)
{
  DdManager* dd;
  double size; 
  double old_size = -1; 
  bdd_ptr result; 
  bdd_ptr masked_bdd;

  dd = BddEnc_get_dd_manager(enc);  
  result = bdd_one(dd);

  masked_bdd = BddEnc_apply_state_vars_mask_bdd(enc, bdd);
  size = BddEnc_count_states_of_bdd(enc, masked_bdd);
  bdd_free(dd, masked_bdd);

  while (size > max_size) {
    bdd_ptr local_constr, constraints, constrained_bdd;

    if (old_size != size) {
      fprintf(nusmv_stdout,
	      "Too many (%.0g) future states to visualize. "
	      "Please specify further constraints: \n", size);
      old_size = size;
    }

    local_constr = simulate_request_constraints(enc);
    if (local_constr != (bdd_ptr) NULL) {
      constraints = bdd_and(dd, local_constr, result);
      bdd_free(dd, local_constr);
    }
    
    /* Incompatible constrains: */
    if (bdd_is_zero(dd, constraints)) {
      fprintf(nusmv_stderr,
	      "Entered expression is incompatible with old constraints."
	      " Try again.\n");
      bdd_free(dd, constraints);
      continue;
    }

    /* too strong constraints: */
    constrained_bdd = bdd_and(dd, bdd, constraints);
    masked_bdd = BddEnc_apply_state_vars_mask_bdd(enc, constrained_bdd);
    bdd_free(dd, constrained_bdd);
    constrained_bdd = masked_bdd;

    if (bdd_is_zero(dd, constrained_bdd)) {
      fprintf(nusmv_stderr,
	      "Set of future states is EMPTY: constraints too strong?"
	      " Try again.\n");
      bdd_free(dd, constrained_bdd);
      bdd_free(dd, constraints);
      continue;      
    }

    /* accumulates contraints for result */
    bdd_free(dd, result);  
    result = bdd_dup(constraints);
    
    /* recalculates loop conditions and cleans up */
    
    size = BddEnc_count_states_of_bdd(enc, constrained_bdd);
    bdd_free(dd, constrained_bdd);
    bdd_free(dd, constraints);
  }
  
  return result;
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static bdd_ptr simulate_request_constraints(BddEnc_ptr enc)
{
  DdManager* dd = BddEnc_get_dd_manager(enc);
  Encoding_ptr senc = BddEnc_get_symbolic_encoding(enc);
  bdd_ptr result = (bdd_ptr) NULL;

  while (true) {
    char* strConstr[] = { NULL, NULL};
    char str[CONSTR_LENGTH];
    int i;

    for (i=0; i < CONSTR_LENGTH; i++) str[i] = '\0'; /* cleans up the string */

    /* Continue if receive a "good" string */
    fprintf(nusmv_stdout, "> ");
    if ( (fgets(str, CONSTR_LENGTH, nusmv_stdin) != (char) NULL) || 
	 (str[0] != '\n') ) {
      node_ptr parsed_command;

      /* frees the string if previously allocated */
      if (strConstr[1] != NULL) { FREE(strConstr[1]); }
      strConstr[1] = util_strsav(str);      

      /* handling of error conditions: */
      if (Parser_ReadCmdFromString(2, strConstr, "CONSTRAINT ", ";\n", 
				   &parsed_command) == 1) {
	fprintf(nusmv_stderr,
		"Parsing error: constraints must be \"simple_expressions\".\n");
	continue;
      }
      
      if ((parsed_command == Nil) || 
	  (node_get_type(parsed_command) != CONSTRAINT)) {
	fprintf(nusmv_stderr,
		"Parsing error: constraints must be a \"simple_expressions\".\n");
	continue;
      }

      CATCH {
	NodeList_ptr vars; 
	vars = Compile_get_expression_dependencies(senc, car(parsed_command));
	if (Encoding_list_contains_input_vars(senc, vars)) {
	  /* input vars are not allowed */
	  fprintf(nusmv_stderr,
		  "Parsing error: constraints cannot contain input variables.\n"
		  "Try again.\n");
	  continue;
	}
      }
      FAIL {
	/* an error occurred here (likely it is an undefined symbol) */
	fprintf(nusmv_stderr, "Try again.\n");		  
	continue;
      }

      /* From here on given expression for the constraint should be
	 ok. Carries on the conversion of give constraint
	 expression: */      
      CATCH {
	result = BddEnc_expr_to_bdd(enc,  car(parsed_command), Nil);
      }
      FAIL { 
	fprintf(nusmv_stderr, "An error occurred during constraints conversion to BDD."
		"Try again.\n");
	continue;
      }

      if ( bdd_is_zero(dd, result) ) {
	fprintf(nusmv_stderr,
		"Entered constraints are equivalent to the empty set. Try again.\n");
	bdd_free(dd, result);
	continue;
      }
      else { 
	/* here given constraints are ok, exits and returns them */
	break;
      }
    } /* end if (fgets) */

  } /* end while (true) */    
  
  return result;
}



