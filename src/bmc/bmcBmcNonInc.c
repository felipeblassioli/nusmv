/**CFile***********************************************************************

  FileName [bmcBmcNonInc.c]

  PackageName [bmc]

  Synopsis [High level functionalities layer for non incremental sat
  solving]

  Description []

  SeeAlso  []

  Author   [Roberto Cavada]

  Copyright [ This file is part of the ``bmc'' package of NuSMV
  version 2.  Copyright (C) 2004 by ITC-irst.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  as published by the Free Software Foundation; either version 2 of
  the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.

  For more information on NuSMV see <http://nusmv.irst.itc.it> or
  email to <nusmv-users@irst.itc.it>.  Please report bugs to
  <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to
  <nusmv@irst.itc.it>. ]

******************************************************************************/

#include "bmcBmc.h"
#include "dag/dag.h"

#include "node/node.h"
#include "be/be.h"
#include "mc/mc.h" /* for print_spec */
#include "sat/sat.h" /* for solver and result */
#include "sat/SatSolver.h"
#include "sat/SatIncSolver.h"
#include "prop/prop.h"

#include "bmcInt.h"
#include "bmcGen.h"
#include "bmcSatTrace.h"
#include "bmcDump.h"
#include "bmcModel.h"
#include "bmcVarsMgr.h"
#include "bmcWff.h"
#include "bmcConv.h"
#include "bmcUtils.h"

#include "mc/mc.h" /* for print_{explanation, spec, invar} */
#include "utils/error.h"

#include "trace/Trace.h"
#include "trace/TraceManager.h"

#include "enc/enc.h"
#include "utils/error.h"

#ifdef BENCHMARKING
  #include <time.h>
  clock_t start_time;
#endif


static char rcsid[] UTIL_UNUSED = "$Id: bmcBmcNonInc.c,v 1.1.2.14 2005/06/27 14:46:38 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Performs simulation]

  Description [Generate a problem with no property, and search for a
  solution, which represents a simulation trace.  Returns 1 if solver
  could not be created, 0 if everything went smooth]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int Bmc_Simulate(const Bmc_Fsm_ptr be_fsm, const int k, 
		  const boolean print_trace, 
		  const boolean changes_only)
{
  be_ptr prob; /* The problem in BE format */
  SatSolver_ptr solver;
  SatSolverResult sat_res;
  BmcVarsMgr_ptr vars_mgr; 
  Be_Cnf_ptr cnf; 

  vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr,
      "\nGenerating simulation trace of length %d (no loopback)\n",
      k);
  }

  solver = Sat_CreateNonIncSolver(get_sat_solver(options));  
  if (solver == SAT_SOLVER(NULL)) {
    fprintf(nusmv_stderr, 
	    "Non-incremental sat solver '%s' is not available.\n", 
	    get_sat_solver(options));
    
    return 1;
  }

  prob = Bmc_Model_GetPathWithInit(be_fsm, k);
  cnf = Be_ConvertToCnf(BmcVarsMgr_GetBeMgr(vars_mgr), prob);

  SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));
  SatSolver_set_polarity(solver, cnf, 1, 
			 SatSolver_get_permanent_group(solver));
  sat_res = SatSolver_solve_all_groups(solver);

  /* Processes the result: */
  switch (sat_res) {
  
  case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
    fprintf(nusmv_stdout,
	    "The model deadlocks before requested length %d!\n", k);
    break;

  case SAT_SOLVER_SATISFIABLE_PROBLEM:
    {
      BmcSatTrace_ptr sat_trace;
      Trace_ptr trace;
      lsList be_model;
      node_ptr path; 
      int bmc_tr;

      be_model = Be_CnfModelToBeModel(BmcVarsMgr_GetBeMgr(vars_mgr), 
				      SatSolver_get_model(solver));

      sat_trace = BmcSatTrace_create(prob, be_model);
      path = NodeList_to_node_ptr(
		    BmcSatTrace_get_symbolic_model(sat_trace, vars_mgr, k));

      trace = Trace_create_from_state_input_list(Enc_get_bdd_encoding(), 
						 "BMC Counterexample",
						 TRACE_TYPE_CNTEXAMPLE,
						 path);

      bmc_tr = TraceManager_register_trace(global_trace_manager, trace);
    
      if (print_trace) {
	if (changes_only) {
	  TraceManager_execute_plugin(global_trace_manager, 0, bmc_tr);
	}
	else {
	  TraceManager_execute_plugin(global_trace_manager, 1, bmc_tr);
	}
      }

      BmcSatTrace_destroy(&sat_trace);
      lsDestroy(be_model, NULL); 
      break;
    }
  case SAT_SOLVER_INTERNAL_ERROR:
    internal_error("Sorry, solver answered with a fatal Internal "
		   "Failure during problem solving.\n");
    
  case SAT_SOLVER_TIMEOUT:
  case SAT_SOLVER_MEMOUT:
    internal_error("Sorry, solver ran out of resources and aborted "
		   "the execution.\n");

  default:
    internal_error(" Bmc_Simulate: Unexpected value in sat result");
  } /* switch */


  SatSolver_destroy(solver);
  Be_Cnf_Delete(cnf); 

  return 0;
}




/**Function********************************************************************

  Synopsis           [Given a LTL property generates and solve the problems
  for all Ki (k_min<=i<=k_max). If bIncreaseK is 0 then k_min==k_max==k and
  only one problem is generated. If bIncreaseK is 1 then k_min == 0 and
  k_max == k.
  Each problem Ki takes into account of all possible loops from k_min to Ki
  if loopback is '*' (BMC_ALL_LOOPS). <BR>
  Also see the Bmc_GenSolve_Action possible values. Returns 1 if solver could 
  not be created, 0 if everything went smooth]

  Description [Returns 1 if solver could not be created, 0 if
  everything went smooth]

  SideEffects        []

  SeeAlso            [Bmc_GenSolve_Action]

******************************************************************************/
int Bmc_GenSolveLtl(Prop_ptr ltlprop,
		    const int k, const int relative_loop,
		    const boolean must_inc_length,
		    const boolean must_solve, 
		    const Bmc_DumpType dump_type,
		    const char* dump_fname_template)
{
  node_ptr bltlspec;  /* Its booleanization */
  Bmc_Fsm_ptr be_fsm; /* The corresponding be fsm */
  BmcVarsMgr_ptr vars_mgr;
 
  /* ---------------------------------------------------------------------- */
  /* Here a property was selected                                           */
  /* ---------------------------------------------------------------------- */
  int k_max = k;
  int k_min = 0;
  int increasingK;
  boolean found_solution;
  
  /* checks that a property was selected: */
  nusmv_assert(ltlprop != PROP(NULL));

  /* checks if it has already been checked: */
  if (Prop_get_status(ltlprop) != Prop_Unchecked) {
    /* aborts this check */
    return 0;
  }

  found_solution = false;
  if (!must_inc_length) k_min = k_max;
  
  { /* booleanized, negated and NNFed formula: */
    node_ptr fltlspec;  
    fltlspec = Compile_FlattenSexpExpandDefine(Enc_get_symb_encoding(), 
					       Prop_get_expr(ltlprop), 
					       Nil);

    bltlspec = Bmc_Wff_MkNnf(Bmc_Wff_MkNot(detexpr2bexpr(fltlspec)));
  }

  if (opt_cone_of_influence(options) == true) {
    Prop_apply_coi_for_bmc(ltlprop, global_fsm_builder);
  }

  be_fsm = Prop_get_be_fsm(ltlprop);
  if (be_fsm == (Bmc_Fsm_ptr) NULL) {
    Prop_set_fsm_to_master(ltlprop);
    be_fsm = Prop_get_be_fsm(ltlprop);
    nusmv_assert(be_fsm != (Bmc_Fsm_ptr) NULL);
  }

  vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);

  /* Start problems generations: */
  for (increasingK = k_min; (increasingK <= k_max) && ! found_solution; 
       ++increasingK) {
    int l;
    char szLoop[16]; /* to keep loopback string */
    be_ptr prob; /* The problem in BE format */
    Be_Cnf_ptr cnf; /* The CNFed be problem */

    /* the loopback value could be depending on the length
       if it were relative: */
    l = Bmc_Utils_RelLoop2AbsLoop(relative_loop, increasingK);

    /* this is for verbose messages */
    Bmc_Utils_ConvertLoopFromInteger(relative_loop, szLoop, sizeof(szLoop));

    /* prints a verbose message: */
    if (opt_verbose_level_gt(options, 0)) {
      if (Bmc_Utils_IsNoLoopback(l)) {
        fprintf(nusmv_stderr,
                "\nGenerating problem with bound %d, no loopback...\n",
                increasingK);
      }
      else if (Bmc_Utils_IsAllLoopbacks(l)) {
        fprintf(nusmv_stderr,
                "\nGenerating problem with bound %d, all possible loopbacks...\n",
                increasingK);
      }
      else {
        /* l can be negative iff loopback from the user pov is < -length */
        if ((l < increasingK) && (l >= 0)) {
          fprintf(nusmv_stderr,
                  "\nGenerating problem with bound %d, loopback %s...\n",
                  increasingK, szLoop);
        }
      }
    } /* verbose message */

    /* checks for loopback vs k compatibility */
    if (Bmc_Utils_IsSingleLoopback(l) && ((l >= increasingK) || (l < 0))) {
      fprintf(nusmv_stderr,
              "\nWarning: problem with bound %d and loopback %s is not allowed: skipped\n",
              increasingK, szLoop);
      continue;
    }

    /* generates the problem: */
#ifdef BENCHMARKING
    fprintf(nusmv_stdout,":START:benchmarking Generation\n");
    start_time = clock();
 #endif

    prob = Bmc_Gen_LtlProblem(be_fsm, bltlspec, increasingK, l);

#ifdef BENCHMARKING
    fprintf(nusmv_stdout,":UTIME = %.4f secs.\n",((double)(clock()-start_time))/CLOCKS_PER_SEC);
    fprintf(nusmv_stdout,":STOP:benchmarking Generation\n");
#endif

    /* Problem is cnf-ed */
    cnf = (Be_Cnf_ptr) NULL;

    /* Problem dumping: */
    if (dump_type != BMC_DUMP_NONE) {
      cnf = Be_ConvertToCnf(BmcVarsMgr_GetBeMgr(vars_mgr), prob);
      Bmc_Dump_WriteProblem(vars_mgr, cnf, ltlprop, increasingK, l, 
			    dump_type, dump_fname_template);
    }

    /* SAT problem solving */
    if (must_solve) {
      SatSolver_ptr solver;
      SatSolverResult sat_res;
      
      /* Sat construction */
      solver = Sat_CreateNonIncSolver(get_sat_solver(options));
      if (solver == SAT_SOLVER(NULL)) {
	fprintf(nusmv_stderr, 
		"Non-incremental sat solver '%s' is not available.\n", 
		get_sat_solver(options));

	if (cnf != (Be_Cnf_ptr) NULL) Be_Cnf_Delete(cnf); 
	return 1;
      }

      /* Cnf construction (if needed): */
      if (cnf == (Be_Cnf_ptr) NULL) {
	cnf = Be_ConvertToCnf(BmcVarsMgr_GetBeMgr(vars_mgr), prob);
      }

#ifdef BENCHMARKING
      fprintf(nusmv_stdout, ":START:benchmarking Solving\n");
      start_time = clock();
#endif      
      
      /* SAT invokation */
      SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));
      SatSolver_set_polarity(solver, cnf, 1, 
			     SatSolver_get_permanent_group(solver));
      sat_res = SatSolver_solve_all_groups(solver);

#ifdef BENCHMARKING
      fprintf(nusmv_stdout, ":UTIME = %.4f secs.\n",
	      ((double)(clock()-start_time))/CLOCKS_PER_SEC);
      fprintf(nusmv_stdout, ":STOP:benchmarking Solving\n");
#endif

      /* Processes the result: */
      switch (sat_res) {

      case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
	{
	  char szLoopMsg[16]; /* for loopback part of message */
	  memset(szLoopMsg, 0, sizeof(szLoopMsg));

	  if (Bmc_Utils_IsAllLoopbacks(l)) {
	    strncpy(szLoopMsg, "", sizeof(szLoopMsg)-1);
	  }
	  else if (Bmc_Utils_IsNoLoopback(l)) {
	    strncpy(szLoopMsg, " and no loop", sizeof(szLoopMsg)-1);
	  }
	  else {
	    /* loop is Natural: */
	    strncpy(szLoopMsg, " and loop at ", sizeof(szLoopMsg)-1);
	    strncat(szLoopMsg, szLoop, sizeof(szLoopMsg)-1-strlen(szLoopMsg));
	  }

	  fprintf(nusmv_stdout,
		  "-- no counterexample found with bound %d%s",
		  increasingK, szLoopMsg);
	  if (opt_verbose_level_gt(options, 2)) {
	    fprintf(nusmv_stdout, " for "); 		  
	    print_spec(nusmv_stdout, ltlprop);
	  }
	  fprintf(nusmv_stdout, "\n");

	  break;
	}

      case SAT_SOLVER_SATISFIABLE_PROBLEM:
        fprintf(nusmv_stdout, "-- ");
        print_spec(nusmv_stdout, ltlprop);
        fprintf(nusmv_stdout, "  is false\n");
        Prop_set_status(ltlprop, Prop_False);

	found_solution = true;

	if (opt_counter_examples(options)) {
	  Trace_ptr trace = 
	    Bmc_Utils_generate_and_print_cntexample(vars_mgr, 
						    solver, 
						    Enc_get_bdd_encoding(),
						    prob, increasingK, 
						    "BMC Counterexample");
	  Prop_set_trace(ltlprop, Trace_get_id(trace));
        }

        break;

      case SAT_SOLVER_INTERNAL_ERROR:
        internal_error("Sorry, solver answered with a fatal Internal "
		       "Failure during problem solving.\n");

      case SAT_SOLVER_TIMEOUT:
      case SAT_SOLVER_MEMOUT:
        internal_error("Sorry, solver ran out of resources and aborted "
		       "the execution.\n");

      default:
	internal_error("Bmc_GenSolveLtl: Unexpected value in sat result");
	
      } /* switch */

      SatSolver_destroy(solver);
    } /* must solve */

    if (cnf != (Be_Cnf_ptr) NULL) {
      Be_Cnf_Delete(cnf); 
      cnf = (Be_Cnf_ptr) NULL;
    }

  } /* for all problems length */

  return 0;
}



/**Function********************************************************************

  Synopsis           [Generates DIMACS version and/or solve and INVARSPEC
  problems]

  Description [Returns 1 if solver could not be created, 0 if
  everything went smooth]

  SideEffects        []

  SeeAlso            [Bmc_GenSolvePbs]

******************************************************************************/
int Bmc_GenSolveInvar(Prop_ptr invarprop,
		      const boolean must_solve, 
		      const Bmc_DumpType dump_type,
		      const char* dump_fname_template)
{
  node_ptr binvarspec;  /* Its booleanization */
  Bmc_Fsm_ptr be_fsm; /* The corresponding be fsm */
  BmcVarsMgr_ptr vars_mgr;
  be_ptr prob;
  Be_Cnf_ptr cnf;

  /* checks that a property was selected: */
  nusmv_assert(invarprop != PROP(NULL));

  /* checks if it has already been checked: */
  if (Prop_get_status(invarprop) != Prop_Unchecked) {
    /* aborts this check */
    return 0;
  }

  { /* booleanized, negated and NNFed formula: */
    node_ptr finvarspec;  
    finvarspec = Compile_FlattenSexpExpandDefine(Enc_get_symb_encoding(), 
						 Prop_get_expr(invarprop), 
						 Nil);
    binvarspec = Bmc_Wff_MkNnf(detexpr2bexpr(finvarspec));
  }


  if (opt_cone_of_influence(options) == true) {
    Prop_apply_coi_for_bmc(invarprop, global_fsm_builder);
  }

  be_fsm = Prop_get_be_fsm(invarprop); 
  if (be_fsm == (Bmc_Fsm_ptr) NULL) {
    Prop_set_fsm_to_master(invarprop);
    be_fsm = Prop_get_be_fsm(invarprop);
    nusmv_assert(be_fsm != (Bmc_Fsm_ptr) NULL);
  }

  vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);

  /* generates the problem: */
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "\nGenerating invariant problem\n");
  }

  prob = Bmc_Gen_InvarProblem(be_fsm, binvarspec);
  
  /* Problem is cnf-ed */
  cnf = (Be_Cnf_ptr) NULL;

  /* Problem dumping: */
  if (dump_type != BMC_DUMP_NONE) {
    cnf = Be_ConvertToCnf(BmcVarsMgr_GetBeMgr(vars_mgr), prob);
    Bmc_Dump_WriteProblem(vars_mgr, cnf, invarprop, 
			  1, Bmc_Utils_GetNoLoopback(), 
			  dump_type, dump_fname_template);
  }

  /* SAT problem solving */
  if (must_solve) {
    SatSolver_ptr solver;
    SatSolverResult sat_res;
      

    /* Sat construction */
    solver = Sat_CreateNonIncSolver(get_sat_solver(options));
    if (solver == SAT_SOLVER(NULL)) {
      fprintf(nusmv_stderr, 
	      "Non-incremental sat solver '%s' is not available.\n", 
	      get_sat_solver(options));

      if (cnf != (Be_Cnf_ptr) NULL) Be_Cnf_Delete(cnf); 
      return 1;      
    }

    /* Cnf construction (if needed): */
    if (cnf == (Be_Cnf_ptr) NULL) {
      cnf = Be_ConvertToCnf(BmcVarsMgr_GetBeMgr(vars_mgr), prob);
    }
 
    /* SAT invokation */
    SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));
    SatSolver_set_polarity(solver, cnf, 1, 
			   SatSolver_get_permanent_group(solver));
    sat_res = SatSolver_solve_all_groups(solver);

    /* Processes the result: */
    switch (sat_res) {

    case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
      fprintf(nusmv_stdout, "-- ");
      print_invar(nusmv_stdout, Prop_get_expr(invarprop));
      fprintf(nusmv_stdout, "  is true\n");
      Prop_set_status(invarprop, Prop_True);
      break;

    case SAT_SOLVER_SATISFIABLE_PROBLEM:
      fprintf(nusmv_stdout, "-- cannot prove the ");
      print_invar(nusmv_stdout, Prop_get_expr(invarprop));
      fprintf(nusmv_stdout, " is true or false : the induction fails\n");
	
      if (opt_counter_examples(options)) {
	Bmc_Utils_generate_and_print_cntexample(vars_mgr, 
						solver, 
						Enc_get_bdd_encoding(), 
						prob, 1, 
						"BMC Failed Induction");
      }

      break;

    case SAT_SOLVER_INTERNAL_ERROR:
      internal_error("Sorry, solver answered with a fatal Internal "
		     "Failure during problem solving.\n");

    case SAT_SOLVER_TIMEOUT:
    case SAT_SOLVER_MEMOUT:
      internal_error("Sorry, solver ran out of resources and aborted "
		     "the execution.\n");

    default:
      internal_error("Bmc_GenSolveLtl: Unexpected value in sat result");
	
    } /* switch */
      
    SatSolver_destroy(solver);
  } /* must solve */

  if (cnf != (Be_Cnf_ptr) NULL) {
    Be_Cnf_Delete(cnf); 
    cnf = (Be_Cnf_ptr) NULL;
  }

  return 0;
}


/**Function********************************************************************

  Synopsis           [Solve and INVARSPEC problems by using 
  Een/Sorensson method non-incrementally]

  Description        [Returns 1 if solver could not be created, 0 if
  everything went smooth]

  SideEffects        []

  SeeAlso            [Bmc_GenSolvePbs]

******************************************************************************/
int Bmc_GenSolveInvar_EenSorensson(Prop_ptr invarprop,  
				   const int max_k, 
				   const Bmc_DumpType dump_type,
				   const char* dump_fname_template)
{
  node_ptr binvarspec;  /* Its booleanization */
  Bmc_Fsm_ptr be_fsm; /* The corresponding be fsm */
  BmcVarsMgr_ptr vars_mgr;
  Be_Manager_ptr be_mgr;

  /* checks that a property was selected: */
  nusmv_assert(invarprop != PROP(NULL));

  /* checks if it has already been checked: */
  if (Prop_get_status(invarprop) != Prop_Unchecked) {
    /* aborts this check */
    return 0;
  }

  { /* booleanized, negated and NNFed formula: */
    node_ptr finvarspec;  
    finvarspec = Compile_FlattenSexpExpandDefine(Enc_get_symb_encoding(), 
						 Prop_get_expr(invarprop), 
						 Nil);
    binvarspec = Bmc_Wff_MkNnf(detexpr2bexpr(finvarspec));
  }

  if (opt_cone_of_influence(options) == true) {
    Prop_apply_coi_for_bmc(invarprop, global_fsm_builder);
  }

  be_fsm = Prop_get_be_fsm(invarprop); 
  if (be_fsm == (Bmc_Fsm_ptr) NULL) {
    Prop_set_fsm_to_master(invarprop);
    be_fsm = Prop_get_be_fsm(invarprop);
    nusmv_assert(be_fsm != (Bmc_Fsm_ptr) NULL);
  }

  vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);
  be_mgr = BmcVarsMgr_GetBeMgr(vars_mgr);

  {   /* generates the problem: */
    be_ptr be_invarspec;
    be_ptr be_init;
    boolean solved; 
    char template_name[BMC_DUMP_FILENAME_MAXLEN];
    int k;
    lsList crnt_state_be_vars;

    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "\nGenerating invariant problem (Een/Sorensson)\n");
    }
    
    be_invarspec = Bmc_Conv_Bexp2Be(vars_mgr, binvarspec);
    be_init = Bmc_Model_GetInit0(be_fsm);

    k = 0;
    solved = false;

    /* retrieves the list of bool variables needed to calculate the
       state uniqueness, taking into account of coi if enabled. */
    crnt_state_be_vars = 
      Bmc_Utils_get_vars_list_for_uniqueness(vars_mgr, invarprop);

    while (!solved & (k <= max_k)) {
      be_ptr be_base;
      Be_Cnf_ptr cnf;
      int i;

      if (opt_verbose_level_gt(options, 1)) {
	fprintf(nusmv_stderr, "\nBuilding the base for k=%d\n", k);
      }

      /* Get the unrolling (s_0,...,s_k)*/
      be_base = Bmc_Model_GetUnrolling(be_fsm, 0, k);
      /* Set the initial condition to hold in s_0 */
      be_base = Be_And(be_mgr, be_base, be_init);
      /* The invariant property should be true in all s_0...s_{k-1}*/
      for (i = 0; i < k; i++) {
	be_base = Be_And(be_mgr, be_base, 
			 BmcVarsMgr_ShiftCurrNext2Time(vars_mgr, 
						       be_invarspec, i));
      }
      
      /* The invariant property should be violated in s_k */
      be_base = Be_And(be_mgr, be_base, 
		 Be_Not(be_mgr, 
			BmcVarsMgr_ShiftCurrNext2Time(vars_mgr, be_invarspec, k)));
      
      /* Problem is cnf-ed */
      cnf = (Be_Cnf_ptr) NULL;

      /* Problem dumping: */
      if (dump_type != BMC_DUMP_NONE) {
	cnf = Be_ConvertToCnf(BmcVarsMgr_GetBeMgr(vars_mgr), be_base);

	strncpy(template_name, dump_fname_template, sizeof(template_name)-2);
	template_name[sizeof(template_name)-1] = '\0'; /* terms the string */
	strncat(template_name, "_base",
		sizeof(template_name) - strlen(template_name) - 1);
	template_name[sizeof(template_name)-1] = '\0'; /* terms the string */	
	
	Bmc_Dump_WriteProblem(vars_mgr, cnf, invarprop, 
			      1, Bmc_Utils_GetNoLoopback(), 
			      dump_type, template_name);
      }

      /* SAT problem solving */
      {
	SatSolver_ptr solver;
	SatSolverResult sat_res;
 
	/* Sat construction */
	solver = Sat_CreateNonIncSolver(get_sat_solver(options));
	if (solver == SAT_SOLVER(NULL)) {
	  fprintf(nusmv_stderr, 
		  "Non-incremental sat solver '%s' is not available.\n", 
		  get_sat_solver(options));

	  if (cnf != (Be_Cnf_ptr) NULL) Be_Cnf_Delete(cnf); 
	  return 1; 
	}
     
	/* Cnf construction (if needed): */
	if (cnf == (Be_Cnf_ptr) NULL) {
	  cnf = Be_ConvertToCnf(BmcVarsMgr_GetBeMgr(vars_mgr), be_base);
	}
 
	/* SAT invokation */
	SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));
	SatSolver_set_polarity(solver, cnf, 1, 
			       SatSolver_get_permanent_group(solver));
	sat_res = SatSolver_solve_all_groups(solver);

	/* Processes the result: */
	switch (sat_res) {

	case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
	  /* continue the loop */
	  break;

	case SAT_SOLVER_SATISFIABLE_PROBLEM:
	  fprintf(nusmv_stdout, "-- ");
	  print_invar(nusmv_stdout, Prop_get_expr(invarprop));	 
	  fprintf(nusmv_stdout, "  is false\n");
	  Prop_set_status(invarprop, Prop_False);

	  solved = true;

	  if (opt_counter_examples(options)) {
	    Trace_ptr trace = 
	      Bmc_Utils_generate_and_print_cntexample(vars_mgr, 
						      solver, 
						      Enc_get_bdd_encoding(), 
						      be_base, k, 
						      "BMC Counterexample");	    
	    Prop_set_trace(invarprop, Trace_get_id(trace));	    
	  } 
	  
	  break;
	  
	case SAT_SOLVER_INTERNAL_ERROR:
	  internal_error("Sorry, solver answered with a fatal Internal "
			 "Failure during problem solving.\n");
	  
	case SAT_SOLVER_TIMEOUT:
	case SAT_SOLVER_MEMOUT:
	  internal_error("Sorry, solver ran out of resources and aborted "
			 "the execution.\n");

	default:
	  internal_error("Bmc_GenSolveLtl: Unexpected value in sat result");
	
	} /* switch */
      
	SatSolver_destroy(solver);
      } /* solving */

      
      /* base cnf no longer useful here */
      if (cnf != (Be_Cnf_ptr) NULL) {
	Be_Cnf_Delete(cnf); 
	cnf = (Be_Cnf_ptr) NULL;
      }


      /* induction step */
      if (!solved) { 
	bdd_ptr be_step; 
	bdd_ptr be_unique;
	int j;

	if (opt_verbose_level_gt(options, 0)) {
	  fprintf(nusmv_stderr, "\nBuilding the step for k=%d\n", k);
	}

	/* Get the unrolling (s_0,...,s_k)*/
	be_step = Bmc_Model_GetUnrolling(be_fsm, 0, k);
	/* The invariant property should be true in all s_0...s_{k-1}*/
 	for (i = 0; i < k; i++) {
	  be_step = Be_And(be_mgr,
			   be_step,
			   BmcVarsMgr_ShiftCurrNext2Time(vars_mgr, 
							 be_invarspec, i));
	}

	/* The invariant property should be violated in s_k */
 	be_step = Be_And(be_mgr, be_step, 
		 Be_Not(be_mgr, BmcVarsMgr_ShiftCurrNext2Time(vars_mgr, 
							       be_invarspec, 
							       k)));

	/* All states s_0,...,s_{k-1} should be different.
	 * Insert and force to true s_j != s_i for each 0 <= j < i <= k-1
	 * in frame 0 */
	be_unique = Be_Truth(be_mgr);
	for (i = 0; i < k ; i++) {
	  for (j = 0; j < i; j++) {
	    be_ptr not_equal = Be_Falsity(be_mgr);
	    be_ptr be_var;
	    lsGen gen;

	    lsForEachItem(crnt_state_be_vars, gen, be_var) {
	      be_ptr be_xor = Be_Xor(be_mgr,
		       BmcVarsMgr_ShiftCurrNext2Time(vars_mgr, be_var, i),
		       BmcVarsMgr_ShiftCurrNext2Time(vars_mgr, be_var, j));
	      not_equal = Be_Or(be_mgr, not_equal, be_xor);
	    }

	    be_unique = Be_And(be_mgr, be_unique, not_equal);
	  }
	} /* for i */
	be_step = Be_And(be_mgr, be_step, be_unique);

	/* Problem dumping: */
	if (dump_type != BMC_DUMP_NONE) {
	  cnf = Be_ConvertToCnf(BmcVarsMgr_GetBeMgr(vars_mgr), be_step);
	  
	  strncpy(template_name, dump_fname_template, sizeof(template_name)-2);
	  template_name[sizeof(template_name)-1] = '\0'; /* terms the string */
	  strncat(template_name, "_step", 
		  sizeof(template_name) - strlen(template_name) - 1);
	  template_name[sizeof(template_name)-1] = '\0'; /* terms the string */	
	  
	  Bmc_Dump_WriteProblem(vars_mgr, cnf, invarprop, 
				1, Bmc_Utils_GetNoLoopback(), 
				dump_type, template_name);
	}

	/* SAT problem solving */
	{
	  SatSolver_ptr solver;
	  SatSolverResult sat_res;
 
	  /* Sat construction */
	  solver = Sat_CreateNonIncSolver(get_sat_solver(options));
	  if (solver == SAT_SOLVER(NULL)) {
	    fprintf(nusmv_stderr, 
		    "Non-incremental sat solver '%s' is not available.\n", 
		    get_sat_solver(options));

	    if (cnf != (Be_Cnf_ptr) NULL) Be_Cnf_Delete(cnf); 
	    return 1; 
	  }
     
	  /* Cnf construction (if needed): */
	  if (cnf == (Be_Cnf_ptr) NULL) {
	    cnf = Be_ConvertToCnf(BmcVarsMgr_GetBeMgr(vars_mgr), be_step);
	  }
 
	  /* SAT invokation */
	  SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));
	  SatSolver_set_polarity(solver, cnf, 1, 
				 SatSolver_get_permanent_group(solver));
	  sat_res = SatSolver_solve_all_groups(solver);

	  /* Processes the result: */
	  switch (sat_res) {

	  case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
	    fprintf(nusmv_stdout, "-- ");
	    print_invar(nusmv_stdout, Prop_get_expr(invarprop));
	    fprintf(nusmv_stdout, "  is true\n");
	    Prop_set_status(invarprop, Prop_True);
	    solved = true;
	    break;

	  case SAT_SOLVER_SATISFIABLE_PROBLEM:
	    /* Prints out the current state of solving, and continues
	       the loop */
	    fprintf(nusmv_stdout,
		    "-- no proof or counterexample found with bound %d", k);
	    if (opt_verbose_level_gt(options, 2)) {
	      fprintf(nusmv_stdout, " for "); 		  
	      print_invar(nusmv_stdout, Prop_get_expr(invarprop));
	    }
	    fprintf(nusmv_stdout, "\n");
	    break;
	  
	  case SAT_SOLVER_INTERNAL_ERROR:
	    internal_error("Sorry, solver answered with a fatal Internal "
			   "Failure during problem solving.\n");
	  
	  case SAT_SOLVER_TIMEOUT:
	  case SAT_SOLVER_MEMOUT:
	    internal_error("Sorry, solver ran out of resources and aborted "
			   "the execution.\n");

	  default:
	    internal_error("Bmc_GenSolveLtl: Unexpected value in sat result");
	
	  } /* switch */
      
	  SatSolver_destroy(solver);
	} /* solving */

      
	/* base cnf no longer useful here */
	if (cnf != (Be_Cnf_ptr) NULL) {
	  Be_Cnf_Delete(cnf); 
	  cnf = (Be_Cnf_ptr) NULL;
	}
      
      } /* induction step */
      
      k = k + 1;
    } /* while !solved */


    lsDestroy(crnt_state_be_vars, NULL);
  } /* problem generation */

  return 0;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



/**AutomaticEnd***************************************************************/
