/**CFile***********************************************************************

  FileName    [mcAGonly.c]

  PackageName [mc]

  Synopsis    [This file contains the code to deal with AG formulas in a special way.]

  Description [This file contains the code to deal with AG formulas
  only, using special purpose algorithms. This functionality is invoked
  with the -AG option and works only in conjunction with the -f
  (forward search) option.]

  Comments [None]

  SeeAlso     [mcMc.c mcEval.c mcExplain.c]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``mc'' package of NuSMV version 2. 
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

#include "mc.h"
#include "mcInt.h" 

#include "node/node.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "trace/Trace.h"
#include "trace/TraceManager.h"
#include "enc/enc.h"


static char rcsid[] UTIL_UNUSED = "$Id: mcAGonly.c,v 1.5.4.23.2.1 2005/06/27 14:46:38 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static boolean is_AG_only_formula(node_ptr n);
static boolean is_AG_only_formula_recur(node_ptr n, int* ag_count);
static node_ptr make_AG_counterexample ARGS((BddFsm_ptr, BddStates));
      

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [This function checks for SPEC of the form AG
  alpha in "context".]

  Description [The implicit assumption is that "spec" must be an AG
  formula (i.e. it must contain only conjunctions and AG's).  No attempt
  is done to normalize the formula (e.g. push negations). The AG mode
  relies on the previous computation and storage of the reachable
  state space (<tt>reachable_states_layers</tt>), they are used in
  counterexample computation.] 

  SideEffects        []

  SeeAlso            [check_spec]

******************************************************************************/
void Mc_CheckAGOnlySpec(Prop_ptr prop) {
  BddFsm_ptr fsm;
  Expr_ptr spec = Prop_get_expr(prop);
  Trace_ptr trace;
  BddEnc_ptr enc = Enc_get_bdd_encoding();


  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "evaluating ");
    print_spec(nusmv_stderr, prop);
    fprintf(nusmv_stderr, "\n");
  }

  if (opt_cone_of_influence(options) == true) {
    Prop_apply_coi_for_bdd(prop, global_fsm_builder);
  }

  fsm = Prop_get_bdd_fsm(prop);
  if (fsm == BDD_FSM(NULL)) {
    Prop_set_fsm_to_master(prop);
    fsm = Prop_get_bdd_fsm(prop);
    BDD_FSM_CHECK_INSTANCE(fsm);
  }

  if (is_AG_only_formula(spec)) {
    trace = check_AG_only(fsm, enc, prop, spec, Nil);
  }
  else {
    warning_non_ag_only_spec(prop);
    return;
  }

  if (trace == TRACE(NULL)) Prop_set_status(prop, Prop_True);
  else {
    int tr = TraceManager_register_trace(global_trace_manager, trace);

    /* Print the trace using default plugin */
    fprintf(nusmv_stdout,
	    "-- as demonstrated by the following execution sequence\n");

    TraceManager_execute_plugin(global_trace_manager,
                    TraceManager_get_default_plugin(global_trace_manager), tr); 

    Prop_set_status(prop, Prop_False);
    Prop_set_trace(prop, Trace_get_id(trace));
  }
}


/**Function********************************************************************

  Synopsis           [This function checks for SPEC of the form AG
  alpha in "context".]

  Description [The implicit assumption is that "spec" must be an AG
  formula (i.e. it must contain only conjunctions and AG's).  No attempt
  is done to normalize the formula (e.g. push negations). The AG mode
  relies on the previous computation and storage of the reachable
  state space (<tt>reachable_states_layers</tt>), they are used in
  counterexample computation.] 

  SideEffects        []

  SeeAlso            [check_spec]

******************************************************************************/
Trace_ptr check_AG_only(BddFsm_ptr fsm, BddEnc_ptr enc, Prop_ptr prop, 
			Expr_ptr spec, node_ptr context)
{
  Trace_ptr res;

  if (spec == Nil) return TRACE(NULL);

  switch (node_get_type(spec)) {

  case CONTEXT:
    res = check_AG_only(fsm, enc, prop, cdr(spec), car(spec));
    break;

  case AND:
    res = check_AG_only(fsm, enc, prop, car(spec), context);
    if (res == TRACE(NULL)) {
      res = check_AG_only(fsm, enc, prop, cdr(spec), context);
    }
    break;

  case AG:
    {
      bdd_ptr tmp_1, tmp_2, acc;
      bdd_ptr invar_bdd, reachable_bdd;
      DdManager* dd = BddEnc_get_dd_manager(enc);
      bdd_ptr s0 = eval_spec(fsm, enc, car(spec), context);

      invar_bdd = BddFsm_get_state_constraints(fsm);
      reachable_bdd = BddFsm_get_reachable_states(fsm);

      tmp_1 = bdd_not(dd, s0);
      tmp_2 = bdd_and(dd, invar_bdd, tmp_1);
      acc = bdd_and(dd, reachable_bdd, tmp_2);

      bdd_free(dd, s0);
      bdd_free(dd, tmp_2);
      bdd_free(dd, reachable_bdd);
      bdd_free(dd, tmp_1);
      bdd_free(dd, invar_bdd);

      fprintf(nusmv_stdout, "-- ");
      print_spec(nusmv_stdout, prop);

      if (bdd_is_zero(dd, acc)) {
        fprintf(nusmv_stdout, "is true\n");
        bdd_free(dd, acc);
        res = TRACE(NULL);
      } 
      else {
	node_ptr path;

        fprintf(nusmv_stdout, "is false\n");        
        path = make_AG_counterexample(fsm, acc);

        res = Trace_create_from_state_input_list(enc, "AG Only counterexample",
						 TRACE_TYPE_CNTEXAMPLE, path);
         
        /* free the list "path" */ 
        walk_dd(dd, bdd_free, path);
	free_list(path);

        bdd_free(dd, acc);
      }
      
      break; 
    } /* case AG */
    
  default:
    if (opt_verbose_level_gt(options, 0)) {    
      fprintf(nusmv_stdout, "*** WARNING - ");
      print_spec(nusmv_stdout, prop);
      fprintf(nusmv_stdout, "skipped: it is not an AG-only formula\n");
    }
    res = TRACE(NULL);

  } /* switch */

  return res;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [This function constructs a counterexample
  starting from state target_state]

  Description        [Compute a counterexample starting from a given state.
  Returned counterexample is a sequence of "state (input, state)*"]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr
make_AG_counterexample(BddFsm_ptr fsm, BddStates target_states)
{
  BddEnc_ptr enc = Enc_get_bdd_encoding();
  DdManager* dd = BddEnc_get_dd_manager(enc);

  node_ptr counterexample = Nil;
  bdd_ptr state;
  int distance;
  int i;
  
  distance = BddFsm_get_distance_of_states(fsm, target_states); 

  /* returns an empty list if any of given target states is not reachable */
  if (distance == -1) return Nil;

  /* pushes the one state from target states (all reachable) at the end: */
  state = BddEnc_pick_one_state(enc, target_states);
  counterexample = cons((node_ptr) state, counterexample);

  for (i = distance-1; i >= 0; --i) {
    BddStates pre_image;
    BddStates reachables;
    BddInputs inputs; 
    bdd_ptr input;

    pre_image = BddFsm_get_backward_image(fsm, state);
    reachables = BddFsm_get_reachable_states_at_distance(fsm, i);
    
    bdd_and_accumulate(dd, &pre_image, reachables);
    bdd_free(dd, reachables);
 
    /* transitions from the reachable pre image to the current state at i+1 */
    inputs = BddFsm_states_to_states_get_inputs(fsm, pre_image, state);
    input = BddEnc_pick_one_input(enc, inputs);
    nusmv_assert(!bdd_is_zero(dd, input));
    bdd_free(dd, inputs);
    counterexample = cons((node_ptr) input, counterexample);

    state = BddEnc_pick_one_state(enc, pre_image);
    bdd_free(dd, pre_image);
    nusmv_assert(!bdd_is_zero(dd, state));
    counterexample = cons((node_ptr) state, counterexample);
  }
 
  return counterexample;
}


/**Function********************************************************************

  Synopsis           [Checks if the formulas is of type AGOnly.]

  Description        [returns true , if the formula is AGOnly formula.]

  SideEffects        []

  SeeAlso            [is_AG_only_formula_recur]

******************************************************************************/
static boolean is_AG_only_formula(node_ptr n)
{
  int ag_count = 0;
  return is_AG_only_formula_recur(n, &ag_count);
}

/**Function********************************************************************

  Synopsis           [Recursive function that helps is_AG_only_formula.]

  Description        []

  SideEffects        []

  SeeAlso            [is_AG_only_formula]

******************************************************************************/
static boolean is_AG_only_formula_recur(node_ptr n, int* ag_count)
{
  if (n == Nil) return true;

  switch (node_get_type(n)) {

  case CONTEXT: 
      return is_AG_only_formula_recur(cdr(n), ag_count); 

  case NOT:    
      return is_AG_only_formula_recur(car(n), ag_count);

  case AND:    
  case OR:     
  case XOR:     
  case XNOR:    
  case IMPLIES: 
  case IFF:    
    return ((is_AG_only_formula_recur(car(n), ag_count)) &&
	    (is_AG_only_formula_recur(cdr(n), ag_count)));
    
  case EX:   /* Non-AG formula */
  case AX:  
  case EF:  
  case AF:  
  case EG:  
  case EU:  
  case AU:  
  case EBU: 
  case ABU: 
  case EBF: 
  case ABF: 
  case EBG: 
  case ABG: 
    return false; 
    
  case AG:    
    *ag_count += 1;
    if(*ag_count > 1) return false; /* More than one AG */
    return is_AG_only_formula_recur(car(n), ag_count); 

  default:      
    { /* for all the other cases, we can safely assume it to be AG Only formula. */
      return true;
    }
  }
}

