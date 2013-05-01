/**CFile***********************************************************************

  FileName    [mcInvar.c]

  PackageName [mc]

  Synopsis    [Dedicated algorithms for the verification of
  invariants on-the-fly wrt reachability analysis.]

  Description [Dedicated algorithms for the verification of
  invariants on-the-fly wrt reachability analysis.]

  SeeAlso     [mcMc.c]

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

  For more information of NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

******************************************************************************/

#include "mc.h"
#include "mcInt.h" 

#include "parser/symbols.h"
#include "utils/error.h"
#include "utils/utils_io.h"
#include "trace/Trace.h"
#include "trace/TraceManager.h"
#include "enc/enc.h"

static char rcsid[] UTIL_UNUSED = "$Id: mcInvar.c,v 1.8.2.17 2004/05/07 13:04:51 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

extern bdd_ptr actions_bdd;
extern node_ptr all_symbols;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* Enable/Disable the forward/backward heuristic */
#define FB_HEURISTIC

/* The heuristic to use in the forward/backward search */
#define FORWARD_BACKWARD_HEURISTICS 0

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Verifies that M,s0 |= AG alpha]

  Description [Verifies that M,s0 |= AG alpha, with alpha propositional.]

  SideEffects []

  SeeAlso     [check_spec check_ltlspec]

******************************************************************************/
void Mc_CheckInvar(Prop_ptr prop) 
{
  int status;
  Expr_ptr  spec = Prop_get_expr(prop);
  BddFsm_ptr fsm;
  
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "evaluating ");
    print_invar(nusmv_stderr, spec);
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

  status = check_invariant_forward(fsm, prop);
}

/**Function********************************************************************

  Synopsis           [Extracts and prints a counterexample for AG alpha.]

  Description        [Extracts a counterexample that leads to a state
  not satisfying the invariant AG alpha. The counterexample produced
  is the shortest execution trace that exploits the falsity of the
  invariant. The computed counterexample is printed out.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int compute_and_print_path(BddFsm_ptr fsm, bdd_ptr reached_goal_bdd,
			   node_ptr path_slices, int nstep, int flag)
{
  int i;
  int tr;
  node_ptr path;
  bdd_ptr state;
  BddEnc_ptr enc; 
  DdManager* dd;
  
  enc = Enc_get_bdd_encoding();
  dd = BddEnc_get_dd_manager(enc);

  state = BddEnc_pick_one_state_rand(enc, reached_goal_bdd);
  path = cons((node_ptr) state, Nil);

  /* We ignore the first element of path_slices, which is the current
     frontier of reachable states */
  nstep = nstep - 1;  
  if (path_slices != Nil) path_slices = cdr(path_slices);
  for(i = nstep; i >= 0; --i) {
    bdd_ptr intersection, image;
    bdd_ptr inputs, input;

    if (car(path_slices) == Nil) internal_error("path_slices == Nil: %d", i);

    image = BddFsm_get_backward_image(fsm, state);
    intersection = bdd_and(dd, image, (bdd_ptr) car(path_slices));

    if (bdd_is_zero(dd, intersection)) {
      bdd_free(dd, image);
      bdd_free(dd, state);
      bdd_free(dd, intersection);
      internal_error("Intersection == emptyset: Step %d",i);
    }

    /* appends input, state pair */
    inputs = BddFsm_states_to_states_get_inputs(fsm, intersection, state);
    input = BddEnc_pick_one_input(enc, inputs);
    bdd_free(dd, inputs);
    path = cons((node_ptr) input, path);
    
    state = BddEnc_pick_one_state(enc, intersection);
    path = cons((node_ptr) state, path);

    bdd_free(dd, image);
    bdd_free(dd, intersection);
    path_slices = cdr(path_slices);
  }
  {
    Trace_ptr trace;

    path = (flag == 0) ? path : reverse(path);
    trace = Trace_create_from_state_input_list(enc, "AG alpha Counterexample",
    					       TRACE_TYPE_CNTEXAMPLE, path); 


    tr = TraceManager_register_trace(global_trace_manager, trace); 

    TraceManager_execute_plugin(global_trace_manager,
                    TraceManager_get_default_plugin(global_trace_manager), tr);

    walk_dd(dd, bdd_free, path);
    free_list(path);
  }
  return tr;
}

/**Function********************************************************************

  Synopsis           [Performs on the fly verification of the
  invariant during reachability analysis.]

  Description        [During the computation of reachable states it
  checks invariants. If the invariant is not satisfied, then an
  execution trace leading to a state not satisfing the invariant is
  printed out.]

  SideEffects        []

  SeeAlso            [check_invariant_forward_opt]

******************************************************************************/
int check_invariant_forward(BddFsm_ptr fsm, Prop_ptr inv_prop)
{ 
  int result = 0;
  bdd_ptr reachable_states;
  int nstep = 0;
  node_ptr plan_reach_list = Nil;
  bdd_ptr invar_bdd, goal_states, init_bdd;
  bdd_ptr target_states, f_frontier, tmp;
  Expr_ptr inv_expr = Prop_get_expr(inv_prop); 
  BddEnc_ptr enc = Enc_get_bdd_encoding();
  DdManager* dd = BddEnc_get_dd_manager(enc);

  invar_bdd = BddFsm_get_state_constraints(fsm);
  init_bdd = BddFsm_get_init(fsm);
  
  f_frontier = bdd_and(dd, init_bdd, invar_bdd);
  tmp = eval_spec(fsm, enc, inv_expr, Nil);
  goal_states = bdd_not(dd, tmp);
  bdd_free(dd, tmp);
  target_states = bdd_and(dd, goal_states, f_frontier);

  /* The goal states have to be intersected with the invariants */
  bdd_and_accumulate(dd, &goal_states, invar_bdd);

  if (opt_verbose_level_gt(options, 1)) {
    fprintf(nusmv_stderr,"Size of invariant states: %d bdd nodes, %g states\n",
            bdd_size(dd, goal_states), 
	    BddEnc_count_states_of_bdd(enc, goal_states));
    fprintf(nusmv_stderr,"Size of initial states: %d bdd nodes\n", 
	    bdd_size(dd, init_bdd));
    fprintf(nusmv_stderr,"Starting to slice the space of reachable states ...\n");
  }

  plan_reach_list = cons((node_ptr) bdd_dup(f_frontier), plan_reach_list);
  reachable_states = init_bdd;
  while (bdd_isnot_zero(dd, f_frontier) && 
	 bdd_is_zero(dd, target_states)) {
    bdd_ptr f_prev_reachable = bdd_dup(reachable_states);
    
    if (opt_verbose_level_gt(options, 1))
      fprintf(nusmv_stderr, "iteration %d: BDD size: %d bdd nodes, states = %g; frontier size = %d bdd nodes.\n",
              nstep, bdd_size(dd, reachable_states),
              BddEnc_count_states_of_bdd(enc, reachable_states),
              bdd_size(dd, f_frontier));
    
    /* Intersect the frontier with normal assignments */
    bdd_and_accumulate(dd, &f_frontier, invar_bdd);
    if (opt_verbose_level_gt(options, 1))
      fprintf(nusmv_stderr,"invariant combined: %d bdd nodes\n",
              bdd_size(dd, f_frontier));
    {
      bdd_ptr image = BddFsm_get_forward_image(fsm, f_frontier);

      /* the image is intersected with the invariant constraints */
      
      bdd_or_accumulate(dd, &reachable_states, image);
      bdd_free(dd, image);
    }
    if (opt_verbose_level_gt(options, 1))
      fprintf(nusmv_stderr, "forward step done: %d bdd nodes\n",
              bdd_size(dd, reachable_states));

    {
      bdd_ptr not_f_prev_reachable = bdd_not(dd, f_prev_reachable);
      
      bdd_free(dd, f_frontier);
      f_frontier = bdd_and(dd, reachable_states, not_f_prev_reachable);
      bdd_free(dd, not_f_prev_reachable);
    }
    plan_reach_list = cons((node_ptr) bdd_dup(f_frontier), plan_reach_list);

    if (opt_verbose_level_gt(options, 1))
      fprintf(nusmv_stderr,"new frontier computed: %d bdd nodes\n",
              bdd_size(dd, f_frontier));

    bdd_free(dd, target_states);
    target_states = bdd_and(dd, goal_states, f_frontier);

    if (opt_verbose_level_gt(options, 1)) {
      if (bdd_isnot_zero(dd, target_states)) {
        fprintf(nusmv_stderr,
                "States not satisfying the invariant reached at step %d: %d bdd nodes, %g states\n",
                nstep+1, bdd_size(dd, target_states), 
		BddEnc_count_states_of_bdd(enc, target_states));
      }
    }
    bdd_free(dd, f_prev_reachable);
    nstep++;
  }

  fprintf(nusmv_stdout, "-- ");
  print_invar(nusmv_stdout, inv_expr);
  if (bdd_isnot_zero(dd, target_states)) {
    int tr;
    fprintf(nusmv_stdout, " is false\n");
    Prop_set_status(inv_prop, Prop_False); 
    
    if (opt_counter_examples(options)) {
      fprintf(nusmv_stdout,
	      "-- as demonstrated by the following execution sequence\n");
      tr = compute_and_print_path(fsm, target_states, plan_reach_list, nstep, 0);
      Prop_set_trace(inv_prop, tr+1);
    }
    result = 0;
  }
  else {
    fprintf(nusmv_stdout, " is true\n");
    Prop_set_status(inv_prop, Prop_True); 
    result = 1;
  }
  bdd_free(dd, target_states);
  bdd_free(dd, f_frontier);
  bdd_free(dd, reachable_states);
  bdd_free(dd, goal_states);
  bdd_free(dd,invar_bdd);
  
  walk_dd(dd, bdd_free, plan_reach_list);
  free_list(plan_reach_list); 

  return result;
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int compute_and_print_path_fb(BddFsm_ptr fsm, bdd_ptr target_states,
			      bdd_ptr f_target, node_ptr f_plan_reach_list, 
			      int f_step, bdd_ptr b_target, 
			      node_ptr b_plan_reach_list, int b_step)
{
  int res = -1;
  /* 
     Warning this routine is not correct. It uses the compute_and_print_path,
     but it must use an ad hoc function, since the
     compute_and_print_path generates a new trace at each call.
  */
  if (f_step != 0) res = compute_and_print_path(fsm, target_states,
                                                f_plan_reach_list, f_step, 0);

  if (b_step != 0) res = compute_and_print_path(fsm, target_states,
                                                b_plan_reach_list, b_step, 1);
  return res;
}


/**Function********************************************************************

  Synopsis           [Performs on the fly verification of the
  invariant during reachability analysis.]

  Description        [During the computation of reachable states it
  checks invariants. If the invariant is not satisfied, then an
  execution trace leading to a state not satisfing the invariant is
  printed out. This function differs from check_invariant_forward
  since it performs backward and forward search.]

  SideEffects        []

  SeeAlso            [check_invariant_forward]

******************************************************************************/
#if 0
void check_invariant_forward_backward(Fsm_BddPtr fsm, Expr_ptr inv_expr)
{ 
  int f_step = 0;
  int b_step = 0;
  int turn = 1;

  BddEnc_ptr enc = Enc_get_bdd_encoding();
  DdManager* dd = BddEnc_get_dd_manager(enc);
  node_ptr f_plan_reach_list = Nil;
  node_ptr b_plan_reach_list = Nil;
  bdd_ptr zero = bdd_zero(dd);
  /* computes the BDD representing the invar to check */
  bdd_ptr init_bdd = Compile_FsmBddGetInit(fsm);
  bdd_ptr invar_bdd = Compile_FsmBddGetInvar(fsm);
  bdd_ptr f_target = eval_spec(fsm, enc, inv_expr, Nil);
  bdd_ptr b_target = bdd_dup(init_bdd);
  bdd_ptr f_cur_reachable = bdd_dup(b_target);
  bdd_ptr b_cur_reachable = bdd_dup(f_target);
  bdd_ptr f_prev_reachable = bdd_dup(f_cur_reachable);
  bdd_ptr b_prev_reachable = bdd_dup(b_cur_reachable);
  bdd_ptr f_frontier = bdd_dup(f_cur_reachable);
  bdd_ptr b_frontier = bdd_dup(b_cur_reachable);
  bdd_ptr target_states = bdd_and(dd, f_target, b_target);

  f_plan_reach_list = cons((node_ptr)bdd_dup(f_frontier), f_plan_reach_list);
  b_plan_reach_list = cons((node_ptr)bdd_dup(b_frontier), b_plan_reach_list);
  if (opt_verbose_level_gt(options, 1)) {
    fprintf(nusmv_stderr, "size of target sets: forward = %d bdd nodes, backward = %d bdd nodes.\n",
            bdd_size(dd, f_target), bdd_size(dd, b_target));
    fprintf(nusmv_stderr, "Starting search ...\n");
  }
  
  while ((f_frontier != zero) && (b_frontier != zero) && (target_states == zero)) {
    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stderr, "Iteration %d, %d forward, %d backward.\n",
              f_step + b_step, f_step, b_step);
      fprintf(nusmv_stderr, "forward frontier size = %d bdd nodes, %g states.\n",
              bdd_size(dd, f_frontier),
              BddEnc_count_states_of_bdd(enc, f_frontier));
      fprintf(nusmv_stderr, "backward frontier size = %d bdd nodes, %g states.\n",
              bdd_size(dd, b_frontier),
              BddEnc_count_states_of_bdd(enc, b_frontier));
    }
#ifndef FB_HEURISTIC
    if (FORWARD_BACKWARD_HEURISTICS == 1) { /* Step forward */
#else
      if /* (turn == 1) */
      (bdd_size(dd, f_frontier) < bdd_size(dd, b_frontier)) { 
      turn = 0;
#endif
      {
        bdd_ptr image = Img_ImageFwd(fsm, f_frontier);
 
        bdd_free(dd, f_prev_reachable);
        f_prev_reachable = bdd_dup(f_cur_reachable);
        bdd_or_accumulate(dd, &f_cur_reachable, image);
        bdd_free(dd, image);
      }

      if (opt_verbose_level_gt(options, 1))
        fprintf(nusmv_stderr, "forward step done: %d bdd nodes\n",
                bdd_size(dd, f_cur_reachable));
      {
        bdd_ptr not_f_prev_reachable = bdd_not(dd, f_prev_reachable);

        f_frontier = bdd_and(dd, f_cur_reachable, not_f_prev_reachable);
        bdd_free(dd, not_f_prev_reachable);
      }

      if (opt_verbose_level_gt(options, 1))
        fprintf(nusmv_stderr,"new frontier computed: %d bdd nodes.\n",
                bdd_size(dd, f_frontier));
      bdd_and_accumulate(dd, &f_frontier, invar_bdd);
      if (opt_verbose_level_gt(options, 1))
        fprintf(nusmv_stderr,"frontier simplification with invariants, new size = %d bdd nodes.\n",
                bdd_size(dd, f_frontier));
      bdd_free(dd, b_target);
      b_target = bdd_dup(f_frontier);
      bdd_free(dd, target_states);
      target_states = bdd_and(dd, f_frontier, f_target);
      f_plan_reach_list = cons((node_ptr)bdd_dup(f_frontier), f_plan_reach_list);
      if (opt_verbose_level_gt(options, 1)) {
        if (target_states == zero) {
          fprintf(nusmv_stderr, "States satisfying invariant set reached at iteration %d forward, %d backward.\n", f_step, b_step);
        } else {
          fprintf(nusmv_stderr, "States not satisfying invariant set reached at step %d, size = %d bdd nodes.", f_step + b_step, bdd_size(dd, target_states));
        }
      }
      f_step++;
      } else {/* Step backward */
#ifdef FB_HEURISTIC
      turn = 1;
#endif
      {
        bdd_ptr image = Img_ImageFwd(fsm, b_frontier);

        bdd_free(dd, b_prev_reachable);
        b_prev_reachable = bdd_dup(b_cur_reachable);
        bdd_or_accumulate(dd, &b_cur_reachable, image);
        bdd_free(dd, image);
      }

      if (opt_verbose_level_gt(options, 1))
        fprintf(nusmv_stderr, "backward step done, size = %d bdd nodes.\n",
                bdd_size(dd, b_cur_reachable));
      {
        bdd_ptr not_b_prev_reachable = bdd_not(dd, b_prev_reachable);

        b_frontier = bdd_and(dd, b_cur_reachable, not_b_prev_reachable);
        bdd_free(dd, not_b_prev_reachable);
      }

      if (opt_verbose_level_gt(options, 1))
        fprintf(nusmv_stderr,"new frontier computed, size = %d bdd nodes.\n",
                bdd_size(dd, b_frontier));
      bdd_and_accumulate(dd, &b_frontier, invar_bdd);
      if (opt_verbose_level_gt(options, 1))
        fprintf(nusmv_stderr, "frontier simplification with invariants, new size = %d bdd nodes.\n",
                bdd_size(dd, b_frontier));

      bdd_free(dd, f_target);
      f_target = bdd_dup(b_frontier);
      bdd_free(dd, target_states);
      target_states = bdd_and(dd, b_frontier, b_target);
      b_plan_reach_list = cons((node_ptr)bdd_dup(b_frontier), b_plan_reach_list);
      if (opt_verbose_level_gt(options, 1)) {
        if (target_states == zero) {
          fprintf(nusmv_stderr, "States satisfying invariant set reached at iteration %d forward, %d backward\n", f_step, b_step);
        } else {
          fprintf(nusmv_stderr, "States not satisfying invariant set reached at step %d, size = %d bdd nodes.\n", f_step + b_step, bdd_size(dd, target_states));
        }
      }
      b_step++;
    }
  }
  if ((f_frontier == 0) || (b_frontier == zero)) {
    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stderr, "\n****************************************\n");
      fprintf(nusmv_stderr, "The invariant is satisfied.\n");
      fprintf(nusmv_stderr, "****************************************\n");
    }
  } else {
    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stderr, "\n****************************************\n");
      fprintf(nusmv_stderr, "A counterexample is:\n");
      fprintf(nusmv_stderr, "****************************************\n");
    }
    compute_and_print_path_fb(fsm, target_states, f_target, f_plan_reach_list, f_step, b_target, b_plan_reach_list, b_step);
  }
  walk_dd(dd, (void (*)())bdd_free, f_plan_reach_list);
  walk_dd(dd, (void (*)())bdd_free, b_plan_reach_list);
  
  bdd_free(dd, f_target);
  bdd_free(dd, b_target);
  bdd_free(dd, f_cur_reachable);
  bdd_free(dd, b_cur_reachable);
  bdd_free(dd, f_prev_reachable);
  bdd_free(dd, b_prev_reachable);
  bdd_free(dd, f_frontier);
  bdd_free(dd, b_frontier);
  bdd_free(dd, target_states);  
  bdd_free(dd, zero);
  {
    node_ptr l = f_plan_reach_list;

    while (l) {
      node_ptr m = l;

      l = cdr(l);
      free_node(m);
    }
  }
  {
    node_ptr l = b_plan_reach_list;

    while (l) {
      node_ptr m = l;

      l = cdr(l);
      free_node(m);
    }
  }
}
#endif /* if 0 */

/**Function********************************************************************

  Synopsis           [Print an invariant specification]

  Description        [Print an invariant specification]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void print_invar(FILE *file, node_ptr n)
{
  node_ptr context = Nil;
  
  if (node_get_type(n) == CONTEXT) {
    context = car(n);
    n = cdr(n);
  }
  indent_node(file, "invariant ", n, " ");
  if (context) {
    fprintf(file, "(in module ");
    print_node(file, context);
    fprintf(file, ")");
  }
}
