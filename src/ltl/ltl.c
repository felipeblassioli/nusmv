/**CFile***********************************************************************

  FileName    [ltl.c]

  PackageName [ltl]

  Synopsis    [Routines to perform reduction of LTL model checking to
  CTL model checking.]

  Description [Here we perform the reduction of LTL model checking to
  CTL model checking. The technique adopted has been taken from [1].
  <ol>
    <li>
       O. Grumberg E. Clarke and K. Hamaguchi. "Another Look at LTL
       Model Checking".  <em>Formal Methods in System Design</em>,
       10(1):57--71, February 1997.
    </li>
  </ol>
  ]

  SeeAlso     [mc]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``ltl'' package of NuSMV version 2. 
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

  For more information on NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

******************************************************************************/

#include "ltl/ltl.h"
#include "ltlInt.h" 
#include "ltl/ltl2smv/ltl2smv.h" 
#include "parser/symbols.h"
#include "parser/parser.h"
#include "prop/prop.h"
#include "fsm/sexp/Expr.h" /* for Expr_ptr */
#include "fsm/bdd/FairnessList.h" 
#include "mc/mc.h"

#include "utils/error.h" /* for CATCH */
#include "utils/utils_io.h"
#include "utils/utils.h"
#include "trace/Trace.h"
#include "trace/TraceManager.h"

#include "enc/enc.h"


static char rcsid[] UTIL_UNUSED = "$Id: ltl.c,v 1.33.4.36.2.6 2005/11/10 09:08:30 nusmv Exp $";

/* 
   Uncomment this if you want to use, thus invoke an external ltl2smv
   translator.
*/
/* #define USE_EXTERNAL_LTL2SMV */

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/**Macro***********************************************************************

  Synopsis     [The LTL to NuSMV translator.]

  Description  [The name of the external program that performs
  translation of a LTL formula to the NuSMV code that represents the
  corresponding tableau.]

  SideEffects []

******************************************************************************/
#define LTL_TRANSLATOR "ltl2smv"

/**Macro***********************************************************************

  Synopsis     [The base module name declared by the <tt>LTL_TRANSLATOR</tt>.]

  Description  [The base module name declared by the <tt>LTL_TRANSLATOR</tt>.]

******************************************************************************/
#define LTL_MODULE_BASE_NAME "ltl_spec_"

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void ltlInsertModuleHashReadModule ARGS((node_ptr));
static void ltlPushStatus ARGS((void));
static void ltlPopStatus ARGS((void));
static void ltlPropAddTableau ARGS((Prop_ptr, node_ptr, node_ptr, node_ptr, 
				    node_ptr, node_ptr));

static int ltlBuildTableauAndPropFsm ARGS((Encoding_ptr senc, Prop_ptr));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [The main routine to perform LTL model checking.]

  Description [The main routine to perform LTL model checking. It
  first takes the LTL formula, prints it in a file. It calls the LTL2SMV
  translator on it an reads in the generated tableau. The tableau is
  instantiated, compiled and then conjoined with the original model
  (both the set of fairness conditions and the transition relation are
  affected by this operation, for this reason we save the current
  model, and after the verification of the property we restore the
  original one).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Ltl_CheckLtlSpec(Prop_ptr prop) 
{
  BddFsm_ptr fsm;
  BddEnc_ptr enc;
  bdd_ptr s0;
  int tableau_result;
  boolean full_fairness;
  node_ptr spec_formula = Nil;
  

  if (opt_cone_of_influence(options)) {
    /* This is an ack in the current version to correctly initialize
       the COI structure when only LTL model checking is performed
       Indeed, after the ltlPushStatus the symbolic encoding contains
       only tableau variables. But if the COI was not invoked before
       on CTL properties this causes the COI structure to be wrongly
       initialized.
    */
    Set_t t = Set_MakeEmpty();

    t = ComputeCOI(BddEnc_get_symbolic_encoding(Enc_get_bdd_encoding()), t);
    Set_ReleaseSet(t);
  }

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "evaluating ");
    print_ltlspec(nusmv_stderr, prop);
    fprintf(nusmv_stderr, "\n");
  }

  ltlPushStatus();

  enc = Enc_get_bdd_encoding();

  CATCH {
    tableau_result = 
      ltlBuildTableauAndPropFsm(BddEnc_get_symbolic_encoding(enc), prop);
  } 
  FAIL {
    ltlPopStatus();
    nusmv_exit(1);
  }
  
  if (tableau_result) {
    ltlPopStatus();
    fprintf(nusmv_stderr, 
            "Ltl_CheckLtlSpec: Problems in Tableau generation.\n");
    nusmv_exit(1);
  }

  fsm = Prop_get_bdd_fsm(prop);
  BDD_FSM_CHECK_INSTANCE(fsm);

  /* If the compassion list is not empty, then activate the full
     fairness algorithm. */
  full_fairness = ! FairnessList_is_empty(
                             FAIRNESS_LIST(BddFsm_get_compassion(fsm)));

  if (full_fairness) {
    s0 = feasible(fsm, enc);   
  }
  else { /* Old Clarke/Grumberg/Hamaguchi algorithm */
    bdd_ptr tmp;
    spec_formula = 
      new_node(NOT,
               new_node(EG,
                        new_node(TRUEEXP,Nil,Nil), Nil), Nil);

    if (opt_verbose_level_gt(options, 2)) {
      Prop_ptr phi = Prop_create_partial(spec_formula, Prop_Ctl);
      fprintf(nusmv_stderr, "Checking CTL ");
      print_spec(nusmv_stderr, phi);
      fprintf(nusmv_stderr, " generated from the tableau.\n");
      Prop_destroy(phi);
    }

    /* Verification of the property: */
    CATCH {
      s0 = eval_spec(fsm, enc, spec_formula, Nil);
    } FAIL {
      ltlPopStatus();
      fprintf(nusmv_stderr, 
              "Ltl_CheckLtlSpec: Problems in Tableau verification.\n");
      return;
    }

    /* Negate the result */
    tmp = bdd_not(dd_manager, s0);
    bdd_free(dd_manager, s0);
    s0 = tmp;

    /* Intersect with init, invar and fair states */
    {
     bdd_ptr init  = BddFsm_get_init(fsm);
     bdd_ptr invar = BddFsm_get_state_constraints(fsm);
     bdd_ptr fair =  BddFsm_get_fair_states(fsm);

     bdd_and_accumulate(dd_manager, &s0, init);
     bdd_and_accumulate(dd_manager, &s0, invar);
     bdd_and_accumulate(dd_manager, &s0, fair);
     bdd_free(dd_manager, fair);
     bdd_free(dd_manager, invar);
     bdd_free(dd_manager, init);
    }
  }
  /* Prints out the result, if not true explain. */
  fprintf(nusmv_stdout, "-- ");
  print_spec(nusmv_stdout, prop);
  
  if (bdd_is_zero(dd_manager, s0)) { 
    fprintf(nusmv_stdout, "is true\n");
    Prop_set_status(prop, Prop_True);
    ltlPopStatus();
  }
  else {
    node_ptr exp;
    Trace_ptr trace;
    int trace_index = 0;
    fprintf(nusmv_stdout, "is false\n");
    Prop_set_status(prop, Prop_False);

    if (opt_counter_examples(options)) {
      char* trace_title = NULL;
      char* trace_title_postfix = " Counterexample";
      bdd_ptr tmp = BddEnc_pick_one_state(enc, s0);

      bdd_free(dd_manager, s0);
      s0 = tmp;

      if (full_fairness) {
	exp = witness(fsm, enc, s0); 
      }
      else {
	bdd_ref(s0); /* to put s0 in the list */
	exp = reverse(explain(fsm, enc, cons((node_ptr)s0, Nil), 
                              spec_formula, Nil));
      }
      if (exp == Nil) {
	/* The counterexample consists of one initial state */
	exp = cons((node_ptr)s0, Nil);
      }
      
      /* The trace title depends on the property type. For example it
       is in the form "LTL Counterexample" */
      trace_title = ALLOC(char, Prop_get_type_as_string(prop) + 
			  strlen(trace_title_postfix) + 1);
      nusmv_assert(trace_title != (char*) NULL);
      strcpy(trace_title, Prop_get_type_as_string(prop));
      strcat(trace_title, trace_title_postfix);
      
      trace = Trace_create_from_state_input_list(enc, trace_title,
						 TRACE_TYPE_CNTEXAMPLE, exp);

      FREE(trace_title);

      trace_index = TraceManager_register_trace(global_trace_manager, trace);
      Prop_set_trace(prop, Trace_get_id(trace));

      fprintf(nusmv_stdout,
	      "-- as demonstrated by the following execution sequence\n");

      /* This is an hack: pop must be executed *before* running the
	 dump of counter-example: */
      ltlPopStatus();

      TraceManager_execute_plugin(global_trace_manager, 
                    TraceManager_get_default_plugin(global_trace_manager),
                    trace_index);

      walk_dd(dd_manager, bdd_free, exp);
      free_list(exp);
    }
    else {
      /* counter examples are not generated */
      ltlPopStatus();
    }
  }
  
  bdd_free(dd_manager, s0);
}

/**Function********************************************************************

  Synopsis           [Print the LTL specification.]

  Description        [Print the LTL specification.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void print_ltlspec(FILE* file, Prop_ptr prop)
{
  indent(file);
  fprintf(file, "LTL specification ");
  Prop_print(prop, file); 
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Add the tableau module to the list of know modules]

  Description        [Add the tableau module to the list of know modules]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ltlInsertModuleHashReadModule(node_ptr parse_tree)
{ /* We insert the definition of the current module in the module_hash in
     order to make it available for the Compile_FlattenHierarchy routines. */
  node_ptr cur_module = car(reverse(parse_tree));
  node_ptr name = find_atom(car(car(cur_module)));
  node_ptr params = cdr(car(cur_module));
  node_ptr def = cdr(cur_module);

  if (lookup_module_hash(name)) error_redefining(name);
  insert_module_hash(name, new_node(LAMBDA, params, reverse(def)));
}

/**Function********************************************************************

  Synopsis           [Save the status of global variables]

  Description        [Save the status of global variables]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ltlPushStatus()
{  
  Encoding_push_status_and_reset(Enc_get_symb_encoding());
  BddEnc_push_status_and_reset(Enc_get_bdd_encoding());
}

/**Function********************************************************************

  Synopsis           [Restore the status of global variables previously saved]

  Description        [Restore the status of global variables previously saved]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ltlPopStatus()
{
  BddEnc_pop_status(Enc_get_bdd_encoding());
  Encoding_pop_status(Enc_get_symb_encoding());
}

/**Function********************************************************************

  Synopsis           [Main routine to add the tableau to the FSM]

  Description        [The bdd fsm into the property will change]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ltlPropAddTableau(Prop_ptr prop, 
			      node_ptr trans_expr, 
			      node_ptr init_expr, node_ptr invar_expr, 
			      node_ptr justice_expr, node_ptr compassion_expr)
{
  SexpFsm_ptr tableau_sexp_fsm;
  BddFsm_ptr  prop_bdd_fsm, tableau_bdd_fsm;
  TransType   trans_type;
  BddEnc_ptr enc;

  enc = Enc_get_bdd_encoding();
  BddEnc_merge(enc);

  /* Creation of the corresponding FSMs: */
  tableau_sexp_fsm = SexpFsm_create_from_members(init_expr, invar_expr,
                                                 trans_expr, 
						 Expr_true(), /* for inputs */
						 justice_expr,
                                                 compassion_expr, 
						 SEXP_FSM_TYPE_SCALAR);

  prop_bdd_fsm = Prop_get_bdd_fsm(prop);
  if (prop_bdd_fsm == BDD_FSM(NULL)) {
    if (opt_cone_of_influence(options)) {
      Prop_apply_coi_for_bdd(prop, global_fsm_builder);
      prop_bdd_fsm = Prop_get_bdd_fsm(prop);
    }
    else {
      Prop_set_fsm_to_master(prop);
      prop_bdd_fsm = Prop_get_bdd_fsm(prop);
    }
  }
  
  BDD_FSM_CHECK_INSTANCE(prop_bdd_fsm);
  
  trans_type = 
    GenericTrans_get_type( GENERIC_TRANS(BddFsm_get_trans(prop_bdd_fsm)) );

  tableau_bdd_fsm = FsmBuilder_create_bdd_fsm(global_fsm_builder, 
					      enc, 
					      tableau_sexp_fsm, trans_type);
  
  /* Carries out the synchronous product. this has side-effect on the
     bdd fsm inside the property, so any previously existing reference
     to it still will work */
  BddFsm_apply_synchronous_product(prop_bdd_fsm, tableau_bdd_fsm);
  
  BddFsm_destroy(tableau_bdd_fsm);
}


/**Function********************************************************************

  Synopsis           [Creates the tableau]

  Description [Creates the tableau for a LTL property.  The FSM of the
  property contains the tableau. Returns 1 if an error is encountered
  during the tableau generation, 0 otherwise]

  SideEffects        [The bdd fsm into the prop will change]

  SeeAlso            []

******************************************************************************/
static int ltlBuildTableauAndPropFsm(Encoding_ptr senc_tableau, Prop_ptr prop)
{
  static int ltl_spec_counter = -1;

  int cmd_len; 
  Expr_ptr spec;
  Expr_ptr ltl_formula; 
  node_ptr context;
  char* Tmp_Input_Fname;
  char* Tmp_Output_Fname;
  char* old_input_file;
  char* Ltl_Module_Name;
  char* External_Command;
  char ltl_spec_counter_str[6]; 
  FILE* file;
  WFFR_TYPE rw;

  ++ltl_spec_counter;   /* to declare unique identifiers in the tableau */

  spec = Prop_get_expr(prop);


  rw = ltlRewriteWffIfInputPresent(senc_tableau, spec);

  if (WFFR_ISNOT_REWRITTEN(rw)) {
    if (node_get_type(spec) == CONTEXT) {
      context     = car(spec);
      ltl_formula = cdr(spec);
    }
    else {
      /* the formula has been flattened before */
      context     = Nil;
      ltl_formula = spec;
    }
  }
  else {
    /* if the formula contains inputs and thus it has been rewritten,
       then it has also been flattened and define expanded, thus the
       context must be Nil (i.e. the main) and the ltl_formula is the
       result of the rewriting. */
    context = Nil; 
    ltl_formula = WFFR_GET_WFF(rw);
  }

  /* The formula has to be negated */
  ltl_formula = Expr_not(ltl_formula);

  /* create and open the file where to print the LTL formula to
     provide in input to the tableau */
  Tmp_Input_Fname = Utils_get_temp_filename_in_dir((char*) NULL, "ltl2smvXXXXXX");
  if (Tmp_Input_Fname == NIL(char)) {
    fprintf(nusmv_stderr, "Could not create input temporary file. ");
    nusmv_exit(1);
  }

  file = fopen(Tmp_Input_Fname, "w");
  if (file == NIL(FILE)) {
    fprintf(nusmv_stderr, "Could not create input temporary file '%s'\n", 
	    Tmp_Input_Fname);
    nusmv_exit(1);
  }
  print_node(file, ltl_formula);
  fclose(file);
  
  /* create the file where the SMV code produced by the tableau is written */
  Tmp_Output_Fname = Utils_get_temp_filename_in_dir((char*) NULL, "ltl2smvXXXXXX");
  if (Tmp_Output_Fname == NIL(char)) {
    fprintf(nusmv_stderr, "Could not create output temporary file. ");
    unlink(Tmp_Input_Fname);
    FREE(Tmp_Input_Fname);
    nusmv_exit(1);
  }

  snprintf(ltl_spec_counter_str, 
	   sizeof(ltl_spec_counter_str) - sizeof(ltl_spec_counter_str[0]), 
	   "%d", ltl_spec_counter);
  
#ifdef USE_EXTERNAL_LTL2SMV
  /* Invoke an external command */
  {
    cmd_len = 4; /* Number of spaces */
    
    cmd_len += strlen(LTL_TRANSLATOR);
    cmd_len += strlen(ltl_spec_counter_str);
    cmd_len += strlen(Tmp_Input_Fname);
    cmd_len += strlen(Tmp_Output_Fname) + 1; /* For str terminator */
    External_Command = ALLOC(char, cmd_len);
  }

  if (External_Command == NIL(char)) {
    fprintf(nusmv_stderr, "Unable to allocate External_Command\n");
    unlink(Tmp_Input_Fname);
    FREE(Tmp_Input_Fname);
    FREE(Tmp_Output_Fname);
    nusmv_exit(1);
  }

  /* Write the LTL formula into the temporary file. */
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Computing the corresponding tableau....");
  }

  snprintf(External_Command, (cmd_len - sizeof(char)), "%s %d %s %s", 
	   LTL_TRANSLATOR, ltl_spec_counter, Tmp_Input_Fname,
           Tmp_Output_Fname);

  if (system(External_Command) != 0) {
    fprintf(nusmv_stderr, "Unable to execute the command:\n%s\n",
            External_Command);
    unlink(Tmp_Input_Fname);
    FREE(Tmp_Input_Fname);
    FREE(Tmp_Output_Fname);
    nusmv_exit(1);
  }

  FREE(External_Command);
#else

  /* Invoke an internal program */
  ltl2smv(ltl_spec_counter, Tmp_Input_Fname, Tmp_Output_Fname);
  
#endif

  /* Deleting input file for tableau construction */
  unlink(Tmp_Input_Fname);
  FREE(Tmp_Input_Fname);

  if (opt_verbose_level_gt(options, 0)) fprintf(nusmv_stderr, ".... done\n");

  /* We save the current input file */
  old_input_file = get_input_file(options);
  set_input_file(options, Tmp_Output_Fname);

  /* Parse the new input file and store the result in parse_tree */
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Parsing the generated tableau....");
  }
  
  if (Parser_ReadSMVFromFile(Tmp_Output_Fname)) {
    set_input_file(options, old_input_file);
    unlink(Tmp_Output_Fname);
    FREE(Tmp_Output_Fname);
    nusmv_exit(1);
  }

  /* Deleting output file of tableau construction */
  unlink(Tmp_Output_Fname);
  FREE(Tmp_Output_Fname);

  if (opt_verbose_level_gt(options, 0)) fprintf(nusmv_stderr, "....done\n");
  
  {
    int j = 0;
      
    j += strlen(LTL_MODULE_BASE_NAME);
    j += strlen(ltl_spec_counter_str);
    Ltl_Module_Name = ALLOC(char, j+1);

    if (Ltl_Module_Name == NIL(char)) {
      fprintf(nusmv_stderr, "Unable to allocate \"Ltl_Module_Name\".\n");
      set_input_file(options, old_input_file);
      nusmv_exit(1);
    }
  }

  sprintf(Ltl_Module_Name, "%s%d", LTL_MODULE_BASE_NAME, ltl_spec_counter);

  /* 
     We insert the definition of the current module in the module_hash
     in order to make it available for the Compile_FlattenHierarchy
     routines.
  */
  ltlInsertModuleHashReadModule(parse_tree);

  {
    node_ptr ltl_init_expr       = Nil;
    node_ptr ltl_invar_expr      = Nil;
    node_ptr ltl_trans_expr      = Nil;
    node_ptr ltl_procs_expr      = Nil;
    node_ptr ltl_justice_expr    = Nil;
    node_ptr ltl_compassion_expr = Nil;
    node_ptr ltl_spec_expr       = Nil;
    node_ptr ltl_compute_expr    = Nil;
    node_ptr ltl_ltlspec_expr    = Nil;
    node_ptr ltl_pslspec_expr    = Nil;
    node_ptr ltl_invar_spec_expr = Nil;
    node_ptr ltl_i_init          = Nil;
    node_ptr ltl_i_invar         = Nil;
    node_ptr ltl_i_trans         = Nil;

    if (WFFR_IS_REWRITTEN(rw)) {
      if (opt_verbose_level_gt(options, 0)) {
        fprintf(nusmv_stderr,
                "Adding state variables to model to handle with inputs within the formula.\n");
      }
      /*
        In this way the new variables introduced to handle with the
        inputs will be added before the tableau variables.
      */
      ltlHandleInputsVars(senc_tableau, rw,
                          &ltl_i_init, &ltl_i_invar, &ltl_i_trans);
    }

    /* Rewriting structure no longer needed */
    ltlFreeWFFR_TYPE(rw);

    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Flattening the generated tableau....");
    }  

    /* 
       We call Compile_FlattenTableau with the name of the generated
       tableau, and as root name the actual property context. In this
       way local variables of the tableau and local variables of the
       formula will be contextualized to the right module.
    */    
    Compile_FlattenTableau(senc_tableau, sym_intern(Ltl_Module_Name), context,
                           &ltl_trans_expr, &ltl_init_expr, &ltl_invar_expr,
                           &ltl_spec_expr, &ltl_compute_expr, 
                           &ltl_ltlspec_expr,  &ltl_pslspec_expr,
			   &ltl_invar_spec_expr, 
                           &ltl_justice_expr, &ltl_compassion_expr,
                           &ltl_procs_expr, Nil);

    /* If rewriting had effect update the FSM accordingly */
    if (ltl_i_init != Nil)
      ltl_trans_expr = find_node(AND, ltl_i_init, ltl_init_expr);
    if (ltl_i_invar != Nil)
      ltl_invar_expr = find_node(AND, ltl_i_invar, ltl_invar_expr);
    if (ltl_i_trans != Nil)
      ltl_trans_expr = find_node(AND, ltl_i_trans, ltl_trans_expr);

    ltl_justice_expr     = reverse(ltl_justice_expr);
    ltl_compassion_expr  = reverse(ltl_compassion_expr);

    /* Check if we are using an old version of ltl2smv */
    if (ltl_spec_expr != Nil) {
      fprintf(nusmv_stderr, 
              "Error: CTL specification in tableau construction.\n");
      fprintf(nusmv_stderr,
              "       You should probably update your version of ltl2smv.\n");
      set_input_file(options, old_input_file);
      return(1);
    }

    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, ".... done\n");
      fprintf(nusmv_stderr, "Creating LTL tableau variables...\n");
    }

    { /* setting input_order_file to NULL in order to avoid duplicate
         variables warning */
      char* iof = get_input_order_file(options);
      set_input_order_file(options, NULL);
      CATCH {
	Encoding_encode_vars(senc_tableau);
      } FAIL { 
        set_input_order_file(options, iof); 
        set_input_file(options, old_input_file);
        return(1);
      }
      set_input_order_file(options, iof);       
    }

    /* The error trapping mechanism is enough in this block. All the
       other errors even external to this block are trapped and the
       else of the CATCH is executed. */
    CATCH { 
      ltlPropAddTableau(prop, ltl_trans_expr, ltl_init_expr, ltl_invar_expr, 
			ltl_justice_expr, ltl_compassion_expr);
    } FAIL {
      set_input_file(options, old_input_file);
      return 1;
    }

    set_input_file(options, old_input_file);
  }

  return 0;
}




