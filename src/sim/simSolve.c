/**CFile***********************************************************************

  FileName    [simSolve.c]

  PackageName [sim]

  Synopsis    [DLL algorithm]

  Description [External procedures included in this module:
		<ul>
		<li> <b>Sim_DllSolve()</b> the main DLL routine
		</ul>
	       Internal procedures included in this module:
	        <ul>
		<li> <b>SimDllExtendProp()</b> propagates a truth value;
		<li> <b>SimDllRetractProp()</b> undoes the effects of a
		     propagation.  
		 <li> <b>SimDllCheck()</b> solution checking
		</ul>]
		
  SeeAlso     [simHeur.c simLookAhead.c simLookBack.c simCons.c]

  Author      [Armando Tacchella Davide Zambonin]

  Copyright   [
  This file is part of the ``sim'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by University of Genova. 

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

  Revision    [v. 1.0]


******************************************************************************/


#include "simInt.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
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


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Solves a formula using the Davis-Logemann-Loveland 
               algorithm. ]

  Description [A heuristic function (SimDllChooseLiteral) is used to choose
               a literal at each node of the search tree; 
	       SimDllCheckConsistency is used to (i) declare a formula
	       satisfiable, and (ii) stop the search at some point; 
	       the SimDllBacktracking function is responsible for
	       restoring to a 
	       previous choice point (if any) in the search tree when
	       a dead end (SIM_UNSAT subproblem) is reached.  
	       Returns SIM_SAT if the formula is satisifable;
	       SIM_UNSAT otherwise.]   

  SideEffects [none]

  SeeAlso     [SimDllCheckConsistency SimDllChooseLiteral SimDllBacktrack]

******************************************************************************/
int  
Sim_DllSolve()
{

  int          teta, mode;
  SimProp_t * p;
  int          stop = 0;

  /* Complete the data structure. */
  SimDllBuild();
  
  /* Start the timer. */
  SimDllStartTimer(SIMSEARCH_TIME);

  /* Assignments enumeration loop: stop == 1 means that the enumeration 
     must end. */
  do {

  /* Single assignment search loop: p == 0 means that the search
     must end. */
    do {

      /* Propagate unit clauses and check for contradictions. */
      if (SimDllBcp() == SIM_SAT) {
	/* Propagate pure literals. */
	SimDllDoMlf;
	/* If a solution was found, exit from the search loop: if stop is 1, 
	   the requested number of solution is found. */
	if (SimDllCheckConsistency(&stop) == SIM_SAT) {
	  break;
	}
	/* Select the next proposition to assign. */
	p = SimDllChooseLiteral(&teta, &mode);
      } else {
	/* Return to a previous open node. */
	p = SimDllBacktrack(&teta, &mode);
      }
      
      if (p != 0) {
	/* If the search is not ended, assign the proposition. */
	SimLetPropHaveValue(p, teta, SimCnt[SIMCUR_LEVEL]);
	(void) SimDllExtendProp(p, teta, mode);
      } else {
	/* Verify if current assignment is a solution, and if the */
	/* requested number of solutions is found. */
	SimDllCheckConsistency(&stop);
      }
      
      SimDllStatInc(SIMCYCLES_NUM);

    } while (p != 0);

    if (!stop) {
      /* If the number of requested solutions is not found, backtrack
	 chronologically and continue the search. */ 
      p = SimDllChronoBt(&teta, &mode);
      if (p != 0) {
	/* If there are yet assignments to try. */
	(void) SimDllExtendProp(p, teta, mode);
      } else {
	/*The enumeration must end. */
	stop = 1;
      }
    }

  } while (!stop);
  
  /* Stop the timer. */
  SimDllStopTimer(SIMSEARCH_TIME);

  /* Return SIM_SAT if all the requested solutions are found, 
     SIM_UNSAT otherwise. */
  return (SimParam[SIM_SOL_NUM] == 0 ? SIM_SAT : SIM_UNSAT);
  
} /* End of Sim_DllSolve. */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Valuates a proposition to SIM_TT]

  Description [Considers a proposition and its valuation:
               propagates the valuation through the formula.
	       Unit clauses are detected and pushed onto the Bcp
	       stack. If HORN_RELAXATION is enabled, 
	       the non-Horn to Horn transitions are monitored and the
               non horn index is suitably updated. If BACKJUMPING is
	       defined, stores the empty clauses causeed by conflicts.
	       If LEARNING is defined, the valuation is propagated to
	       the learned clauses (unit resolutions only).
	       Returns SIM_UNSAT if an empty clause is found; SIM_SAT
	       otherwise.]

  SideEffects [none]

  SeeAlso     [SimDllRetractProp SimDllBcp SimDllMlf]

******************************************************************************/
int 
SimDllExtendPropTT(
  SimProp_t * p, 
  int         mode)
{
 
  SimClause_t  * cl;
  SimClause_t ** gencl;
  int            status;

  /* Be optimistic! */
  status = SIM_SAT;
  /* Check if the proposition is to be assigned: if it is not, then
     some error occurred. */
  SimDllAssert((p -> teta == SIM_DC), SIM_EP_LOCATION,
	       "SimDllExtendPropTT: attempt to reassing proposition");
  
  /* Set sign, mode, level and push the proposition in the search stack. */
  p -> teta = SIM_TT;
  p -> mode = mode;
  p -> level = SimCnt[SIMCUR_LEVEL];
  VforceBack(SimStack, p);

  /* Unit subsumptions (static occurrences). */
  Vforeach0(p -> posLits, gencl, cl) {
   /* Freeze the clause. */
   if (SimClauseIsOpen(cl)) {
     cl -> sub = p;
     /* Decrement open clauses. */
     SimCnt[SIMCUR_CL_NUM] -= 1;
#ifdef HORN_RELAXATION
      /* Possibly decrement open non horn clauses also. */
      if (cl -> posLitNum > 1) {
	SimCnt[SIMCUR_NHCL_NUM] -= 1;
      }
#endif
    }
  } /* Vforeach0(p -> posLits, gencl, cl) */

  /* Unit resolutions (static occurrences). */
  Vforeach0(p -> negLits, gencl, cl) {
    /* If the clause is open, decrement the number of open literals. */
    if (SimClauseIsOpen(cl)) {
      cl -> openLitNum -= 1;
      /* If the clause is open and the formula is not UNSAT, 
	 test for empty and unit clauses. */
      if (status != SIM_UNSAT) {
	if (cl -> openLitNum == 1) {
	  /* An unit clause is found. */
	  VforceBack(SimBcpStack, cl);
	} else if (cl -> openLitNum == 0) {
	  /* An empty clause is found. */
	  status = SIM_UNSAT;
	  SimDllStatInc(SIMFAIL_NUM);
#ifdef BACKJUMPING
	  /* Store the empty clause as the conflicting clause. */
	  SimConflictCl = cl;
#endif
	}
      }
    }
  } /* Vforeach0(p -> posLits, gencl, cl) */
  
#ifdef LEARNING
  /* Unit resolutions (dynamic occurrences). */
  gencl += 1;
  VforeachGen0(gencl, cl) {
    /* Learned clauses are always open. */
    cl -> openLitNum -= 1;
    /* If the clause is open and the formula is not UNSAT,
       test for empty and unit clauses. */
    if (status != SIM_UNSAT) {
      if (cl -> openLitNum == 1) {
	/* An unit clause is found. */
	VforceBack(SimBcpStack, cl);
      } else if (cl -> openLitNum == 0) {
	/* An empty clause is found. */
	status = SIM_UNSAT;
	SimDllStatInc(SIMFAIL_NUM);
	SimDllStatInc(SIMCLASH_NUM);
	/* Store the empty clause as the conflicting clause. */
	SimConflictCl = cl;
      }
    }
  } /* Vforeach0(gencl + 1, gencl, cl) */
#endif
  
  return status;

} /* End of SimDllExtendPropTT. */


/**Function********************************************************************

  Synopsis    [Valuates a proposition to SIM_FF]

  Description [Considers a proposition and its valuation:
               propagates the valuation through the formula.
	       Unit clauses are detected and pushed onto the Bcp
	       stack. If HORN_RELAXATION is enabled, 
	       the non-Horn to Horn transitions are monitored and the
               non horn index is suitably updated. If BACKJUMPING is
	       defined, stores the empty clauses causeed by conflicts.
	       If LEARNING is defined, the valuation is propagated to
	       the learned clauses (unit resolutions only).
	       Returns SIM_UNSAT if an empty clause is found; SIM_SAT
	       otherwise.]

  SideEffects [none]

  SeeAlso     [SimDllRetractProp SimDllBcp SimDllMlf]

******************************************************************************/
int 
SimDllExtendPropFF(
  SimProp_t * p, 
  int         mode)
{
 
  SimClause_t  * cl;
  SimClause_t ** gencl;
  int            status;
#ifdef HORN_RELAXATION
  int            i;
#endif

  /* Be optimistic! */
  status = SIM_SAT;
  /* Check if the proposition is to be assigned: if it is not, then
     some error occurred. */
  SimDllAssert((p -> teta == SIM_DC), SIM_EP_LOCATION,
	       "SimDllExtendPropFF: attempt to reassing proposition");
  
  /* Set sign, mode, level and push the proposition in the search stack. */
  p -> teta = SIM_FF;
  p -> mode = mode;
  p -> level = SimCnt[SIMCUR_LEVEL];
  VforceBack(SimStack, p);

  /* Unit subsumption (static occurrences). */
  Vforeach0(p -> negLits, gencl, cl) {
   /* Freeze the clause. */
   if (SimClauseIsOpen(cl)) {
     cl -> sub = p;
     /* Decrement open clauses. */
     SimCnt[SIMCUR_CL_NUM] -= 1;
#ifdef HORN_RELAXATION
      /* Possibly decrement open non horn clauses also. */
      if (cl -> posLitNum > 1) {
	SimCnt[SIMCUR_NHCL_NUM] -= 1;
      }
#endif
    }
  } /* Vforeach0(p -> negLits, gencl, cl) */

  /* Unit resolution (static occurrences). */
  Vforeach0(p -> posLits, gencl, cl) {
    /* If the clause is open, decrement the number of open literals. */
    if (SimClauseIsOpen(cl)) {
      cl -> openLitNum -= 1;
      /* If the clause is open, test for empty and unit clauses. */
      if (status != SIM_UNSAT) {
	if (cl -> openLitNum == 1) {
	  /* An unit clause is found. */
	  VforceBack(SimBcpStack, cl);
	} else if (cl -> openLitNum == 0) {
	  /* An empty clause is found. */
	  status = SIM_UNSAT;
	  SimDllStatInc(SIMFAIL_NUM);
#ifdef BACKJUMPING
	  /* Store the empty clause as the conflicting clause. */
	  SimConflictCl = cl;
#endif
	}
      }
#ifdef HORN_RELAXATION
      /* If the clause is becoming horn, unlink it. */
      if (cl -> posLitNum == 2) {
	i = cl -> backNhClauses;
	V(SimNhClauses)[i] = VextractBack(SimNhClauses);
	V(SimNhClauses)[i] -> backNhClauses = i;
	/* Decrement the number of open non-horn clauses. */
	SimCnt[SIMCUR_NHCL_NUM] -= 1;
      }
      cl -> posLitNum -= 1;
#endif
    }
  } /* Vforeach0(p -> posLits, gencl, cl) */

#ifdef LEARNING
  /* Unit resolutions (dynamic occurrences). */
  gencl += 1;
  VforeachGen0(gencl, cl) {
    /* Learned clauses are always open. */
    cl -> openLitNum -= 1;
    /* If the clause is open and the formula is not UNSAT,
       test for empty and unit clauses. */
    if (status != SIM_UNSAT) {
      if (cl -> openLitNum == 1) {
	/* An unit clause is found. */
	VforceBack(SimBcpStack, cl);
      } else if (cl -> openLitNum == 0) {
	/* An empty clause is found. */
	status = SIM_UNSAT;
	SimDllStatInc(SIMFAIL_NUM);
	SimDllStatInc(SIMCLASH_NUM);
	/* Store the empty clause as the conflicting clause. */
	SimConflictCl = cl;
      }
    }
  } /* Vforeach0(gencl + 1, gencl, cl) */
#endif  
  
  return status;

} /* End of SimDllExtendPropFF. */


/**Function********************************************************************

  Synopsis    [Unvaluates a proposition (assigned to SIM_TT)]

  Description [Considers a proposition: the effects of the propagations
               are undone through the formula.
	       Unit clauses are detected and pushed onto the Bcp
	       stack. If HORN_RELAXATION is enabled, 
               the non-horn to horn transitions are monitored and the 
               non-horn index is suitably restored. If BACKJUMPING is
	       enabled, removes the reason of failure driven
	       assignments. If LEARNING is defined, updates learned
	       clauses, and detects unit on learned clauses.] 

  SideEffects [none]

  SeeAlso     [SimDllRetractProp SimDllBcp SimDllMlf]

******************************************************************************/
void 
SimDllRetractPropTT(
  SimProp_t  * p)
{

#ifdef LEARNING
  int            i, open;
#endif
  SimClause_t  * cl;
  SimClause_t ** gencl;

  /* Check if the proposition is assigned. */
  SimDllAssert((p -> teta != SIM_DC), SIM_RP_LOCATION, 
	       "SimDllRetractPropTT: attempt to retract a DC");
  SimRetractingProp(p, SimCnt[SIMCUR_LEVEL]);

#ifdef BACKJUMPING
  if (p -> mode >= SIMRSPLIT) {
    /* Delete the reason in case of right splits and failed literals. */
    SimClauseClear(p -> reason);
  }
#endif

  /* Retract the valuation. */
  p -> teta = SIM_DC;

  /* Retracting unit subsumptions (static occurrences). */
  Vforeach0(p -> posLits, gencl, cl) {
    /* Unfreeze the clause. */
    if (cl -> sub == p) {
      cl -> sub = 0;
      SimCnt[SIMCUR_CL_NUM] += 1;
#ifdef HORN_RELAXATION
      /* Possibly increment open non horn clauses. */
      if (cl -> posLitNum > 1) {
	SimCnt[SIMCUR_NHCL_NUM] += 1;
      }
#endif
    }
  } /* i < subSize */

  /* Retracting unit resolutions (static occurrences). */
  Vforeach0(p -> negLits, gencl, cl) {
    if (SimClauseIsOpen(cl)) {
      cl -> openLitNum += 1;
    }
  } /* Vforeach0(p -> negLits, gencl, cl) */

#ifdef LEARNING
  /* Retract unit resolutions (dynamic occurrences). */
  /* Reverse iteration is to avoid problems when unlearning clauses. */
  VforeachRev0(p -> negLits, gencl, cl) {
    open = ++(cl -> openLitNum);
    if (open > SimParam[SIM_LEARN_ORDER]) {
      /* Remove learned clauses. */
      SimDllUnlearnClause(cl);
    } else if ((open == 1) && (cl -> backUnitLearned == SIMALLOW_UNIT)) {
      /* Push the clause into the unit learned stack. */
      cl -> backUnitLearned = Vsize(SimUnitLearned);
      VforceBack(SimUnitLearned, cl);
    } else if ((open == 2) && (cl -> backUnitLearned != SIMFORBID_UNIT)) {
      /* Remove the clause from the stack (swapping it with the stack tail). */
      i = cl -> backUnitLearned;
      V(SimUnitLearned)[i] = VextractBack(SimUnitLearned);
      V(SimUnitLearned)[i] -> backUnitLearned = i;
      cl -> backUnitLearned = SIMFORBID_UNIT;
    }
  }  /* Vforeach0(gencl + 1, gencl, cl) */
#endif

  return;

} /* End of SimDllRetractPropTT. */


/**Function********************************************************************

  Synopsis    [Unvaluates a proposition (assigned to SIM_FF)]

  Description [Considers a proposition: the effects of the propagations
               are undone through the formula.
	       Unit clauses are detected and pushed onto the Bcp
	       stack. If HORN_RELAXATION is enabled, 
               the non-horn to horn transitions are monitored and the 
               non-horn index is suitably restored. If BACKJUMPING is
	       enabled, removes the reason of failure driven
	       assignments. If LEARNING is defined, updates learned
	       clauses, and detects unit on learned clauses.] 

  SideEffects [none]

  SeeAlso     [SimDllRetractProp SimDllBcp SimDllMlf]

******************************************************************************/
void 
SimDllRetractPropFF(
  SimProp_t  * p)
{

#ifdef LEARNING
  int            i, open;
#endif
  SimClause_t  * cl;
  SimClause_t ** gencl;

  /* Check if the proposition is assigned. */
  SimDllAssert((p -> teta != SIM_DC), SIM_RP_LOCATION, 
	       "SimDllRetractPropFF: attempt to retract a DC");
  SimRetractingProp(p, SimCnt[SIMCUR_LEVEL]);

#ifdef BACKJUMPING
  if (p -> mode >= SIMRSPLIT) {
    /* Delete the reason in case of right splits and failed literals. */
    SimClauseClear(p -> reason);
  }
#endif

  /* Retract the valuation. */
  p -> teta = SIM_DC;

  /* Retracting unit subsumptions (static occurrences). */
  Vforeach0(p -> negLits, gencl, cl) {
    /* Unfreeze the clause. */
    if (cl -> sub == p) {
      cl -> sub = 0;
      SimCnt[SIMCUR_CL_NUM] += 1;
#ifdef HORN_RELAXATION
      /* Possibly increment open non horn clauses. */
      if (cl -> posLitNum > 1) {
	SimCnt[SIMCUR_NHCL_NUM] += 1;
      }
#endif
    }
  } /* i < subSize */

  /* Retracting unit resolutions (static occurrences). */
  Vforeach0(p -> posLits, gencl, cl) {
    if (SimClauseIsOpen(cl)) {
      cl -> openLitNum += 1;
#ifdef HORN_RELAXATION
      /* If the clause is becoming non-horn, link it. */
      if (cl -> posLitNum == 1) {
	cl -> backNhClauses = Vsize(SimNhClauses);
	VforceBack(SimNhClauses, cl);
	++SimCnt[SIMCUR_NHCL_NUM];
      }
      ++(cl -> posLitNum);
#endif
    }
  } /* Vforeach0(p -> negLits, gencl, cl) */

#ifdef LEARNING
  /* Retract unit resolutions (dynamic occurrences). */
  /* Reverse iteration is to avoid problems when unlearning clauses. */
  VforeachRev0(p -> posLits, gencl, cl) {
    open = ++(cl -> openLitNum);
    if (open > SimParam[SIM_LEARN_ORDER]) {
      /* Remove learned clauses. */
      SimDllUnlearnClause(cl);
    } else if ((open == 1) && (cl -> backUnitLearned == SIMALLOW_UNIT)) {
      /* Push the clause into the unit learned stack. */
      cl -> backUnitLearned = Vsize(SimUnitLearned);
      VforceBack(SimUnitLearned, cl);
    } else if ((open == 2) && (cl -> backUnitLearned != SIMFORBID_UNIT)) {
      /* Remove the clause from the stack (swapping it with the stack tail). */
      i = cl -> backUnitLearned;
      V(SimUnitLearned)[i] = VextractBack(SimUnitLearned);
      V(SimUnitLearned)[i] -> backUnitLearned = i;
      cl -> backUnitLearned = SIMFORBID_UNIT;
    }
  }  /* Vforeach0(gencl + 1, gencl, cl) */
#endif

  return;

} /* End of SimDllRetractPropTT. */


/**Function********************************************************************

  Synopsis    [Checks a solution found by Sim_DllSolve. ]

  Description [Reinitializes the number of open literals in each
               clause and then propagates the stack contents throughout
	       the formula (from stack bottom to stack top) using a simple
	       algorithm. Returns SIM_UNSAT if:
	       <ul>
	           <li> the stack contains a proposition which has SIM_DC
		        or SIM_NT in its teta field, or
	           <li> an empty clause is found, or
		   <li> the number of open clauses does not become
              	        0 after the stack has been examined,
			
	       </ul>
	       and SIM_SAT otherwise. If HORN_RELAXATION is defined checks
	       the number of open non-Horn clauses. If LEARNING is
	       defined checks also resolutions on learned clauses.]

  SideEffects [redundant is set to the number of unnecessary assignments.]

  SeeAlso     []

******************************************************************************/
int  
SimDllCheck(
  int       * redundant) 
{
  
  int            clNum, i, j;
  SimProp_t    * p;
  SimClause_t  * cl;
  SimClause_t ** gencl;

#ifdef HORN_RELAXATION
  SimProp_t   ** genp;
  clNum = SimCnt[SIMNHCL_NUM];
#else
  clNum = SimCnt[SIMCL_NUM];
#endif

  /* Reset openLitNum in each clause to its original value and
     force the clause open. */
  for (i = 0; i < Vsize(SimClauses); i++) {
    cl = V(SimClauses)[i];
    cl -> openLitNum = Vsize(cl -> lits);
    cl -> sub = 0;
#ifdef HORN_RELAXATION
    /* Reset posLitNum in each clause. */
    cl -> posLitNum = 0;
    Vforeach0(cl -> lits, genp, p) {
      if (SimIsPos(p)) {
	cl -> posLitNum += 1;
      }
    }
#endif
  }

  /* Propagate the stack contents again. */
  for (j = 0; (j < Vsize(SimStack)) && (clNum > 0); j++) {

    /* Get the j-th proposition in stack order. */
    p = V(SimStack)[j];

    if (p -> teta == SIM_TT) {

      /* Check subsumptions (original occurrences only). */
      Vforeach0(p -> posLits, gencl, cl) {
	if (SimClauseIsOpen(cl)) {
	  cl -> sub = p;
#ifdef HORN_RELAXATION
	  if ((cl -> posLitNum) > 1) {
	    clNum -= 1;
	  }
#else
	  clNum -= 1;
#endif
	}
      }
      /* Check resolutions (original occurrences only). */
      Vforeach0(p -> negLits, gencl, cl) {
	if (SimClauseIsOpen(cl)) {
	  cl -> openLitNum -= 1;
	}
	if (cl -> openLitNum == 0) {
	  /* The clause is empty: it should not be the case! */
	  clNum = -1;
	  break;
	}
      }

    } else if (p -> teta == SIM_FF) {

      /* Check subsumptions (original occurrences only). */
      Vforeach0(p -> negLits, gencl, cl) {
	if (SimClauseIsOpen(cl)) {
	  cl -> sub = p;
#ifdef HORN_RELAXATION
	  if ((cl -> posLitNum) > 1) {
	    clNum -= 1;
	  }
#else
	  clNum -= 1;
#endif
	}
      }
      /* Check resolutions (original occurrences only). */
      Vforeach0(p -> posLits, gencl, cl) {
	if (SimClauseIsOpen(cl)) {
	  cl -> openLitNum -= 1;
#ifdef HORN_RELAXATION
	  /* Check if the clause is becoming Horn. */
	  if ((cl -> posLitNum) == 2) {
	    clNum -= 1;
	  }
	  cl -> posLitNum -= 1;
#endif
	}
	if (cl -> openLitNum == 0) {
	  /* The clause is empty: it should not be the case! */
	  clNum = -1;
	  break;
	}
      }

    } else {
      /* The proposition was inserted in the stack with SIM_DC or SIM_NT. */
      break;
    }
    
  } /*  (j < SimDllStackIdx) && (clNum > 0) */

  if (clNum != 0) {
    /* If there are still open clauses, the solution is wrong. */
    return SIM_UNSAT;
  } else {
#ifdef HORN_RELAXATION
    for (; (j < Vsize(SimStack)) && (V(SimStack)[j] -> mode == SIMUNIT); j++);
#endif
    *redundant = Vsize(SimStack) - j;
    /* The solution is OK (modulo redundant assignments). */
    return SIM_SAT;
  }

} /* End of SimDllCheck. */









