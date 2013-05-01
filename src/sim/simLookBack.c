/**CFile***********************************************************************

  FileName    [simLookBack.c]

  PackageName [sim]

  Synopsis    [A compendium of look back techniques.]

  Description [Internal procedures included in this module:
		<ul>
		<li> <b>SimDllChronoBt()</b> performs stackwise chronological
                     backtracking 
                <li> <b>SimDllBackjumping()</b> performs non-chronological 
		     dependency directed backtracking
		<li> <b>SimDllInitWr()</b>      
		<li> <b>SimDllResolveWithWr()</b>
                <li> <b>SimDllMakeClFromWr()</b> 
                <li> <b>SimDllLearnClause()</b> 
                <li> <b>SimDllUnlearnClause()</b> 
		</ul>]
		
  SeeAlso     [simSolve.c]

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
/* Stucture declarations                                                     */
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


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Stackwise chronological backtracking.]

  Description [Backtracks to the most recent open choice point, i.e.,
               the most recent proposition assigned with SIMLSPLIT mode.
	       Returns a reference to the proposition at the choice
	       point or NULL when there is no more choice, i.e., an
	       attempt to backtrack beyond the very first choice point
	       is made.] 

  SideEffects [none]

  SeeAlso     [Sim_DllSolve SimDllRetractProp]

******************************************************************************/
SimProp_t * 
SimDllChronoBt(
  int       * sign,
  int       * mode
  )
{
 
  SimProp_t * p = 0;

  SimOnTraceDo(SimDllTraceLeafs);

  SimDllStatUpdateMax(SIMDEPTH_MAX, SimCnt[SIMCUR_LEVEL]);
  SimContradictionFoundOn(Vback(SimStack), SimCnt[SIMCUR_LEVEL]);

  /* Reset BCP and MLF stack. */
  Vflush(SimBcpStack);
#ifdef PURE_LITERAL
  Vflush(SimMlfStack);
#endif

  /* Pop off propositions from the stack and retract them
     until a SIMLSPLIT is reached or no more backtracking is possible. */
  while ((Vsize(SimStack) > 0) && 
	 ((p = VextractBack(SimStack)) -> mode != SIMLSPLIT)) {
    SimDllRetractProp(p);
  }
  
  /* If backtracking is still possible, branch on the other side. */ 
  if (p -> mode == SIMLSPLIT) {
    *sign = (p -> teta == SIM_TT ? SIM_FF : SIM_TT);
    *mode = SIMRSPLIT;
    SimDllRetractProp(p);
    /* Update the current level. */
    SimCnt[SIMCUR_LEVEL] = p -> level;
  } else {
    p = 0;
  }

  SimDllStatUpdateMin(SIMDEPTH_MIN, SimCnt[SIMCUR_LEVEL]);
  SimDllStatInc(SIMFDA_NUM);

  return p;
  
} /* End of SimDllChronoBt. */


/**Function********************************************************************

  Synopsis    [Stackwise conflict directed backjumping.]

  Description [Backtracks to the most recent open choice point that was
               responsible for the contradiction, possibly skipping other 
	       choice points. If LEARNING is defined, also stores working 
	       reasons  with some criterion and propagates unit learned 
               clauses. Returns a reference to the 
	       proposition at the choice point or 0 when there 
	       is no more choice, i.e., an attempt to backtrack beyond 
	       the very first choice point is made.] 

  SideEffects [none]

  SeeAlso     [Sim_DllSolve SimDllRetractProp]

******************************************************************************/
SimProp_t * 
SimDllBackjumping(
  int       * sign,
  int       * mode
  )
{
#ifdef BACKJUMPING
  SimProp_t   * p;
  SimClause_t * wrClause = 0;
  int           skipped = 0;

#ifdef LEARNING
  int           i;
  int           res;
  SimClause_t * cl;
#endif  

  SimOnTraceDo(SimDllTraceLeafs);

  /* Reset BCP and MLF stacks. */
  Vflush(SimBcpStack);
#ifdef PURE_LITERAL
  Vflush(SimMlfStack);
#endif

  /* Initialize the working reason with
     the literals of the conflict (empty) clause. */
  SimDllInitWr(SimConflictCl);

  SimDllStatUpdateMax(SIMDEPTH_MAX, SimCnt[SIMCUR_LEVEL]);
  SimContradictionFoundOn(Vback(SimStack), SimCnt[SIMCUR_LEVEL]);
  SimWrIs(SimWrLits);

  /* Pop off propositions from the search stack until a SIMLSPLIT 
     member of the working reason is reached. */
  while (Vsize(SimStack) > 0) {

    /* Get the proposition. */
    p = VextractBack(SimStack);
    
    /* If the proposition is in the current working reason. */
    if (V(SimLitInWr)[p -> prop] != 0) {

      switch (p -> mode) {

      case SIMUNIT:
      case SIMRSPLIT:
      case SIMFAILED:
	/* Resolve the reason of p with the current working reason. */
	SimDllResolveWithWr(p -> reason, p);
	SimNewWrFrom1(p -> reason);
        SimWrIs(SimWrLits);
	/* If the empty clause is deduced, the formula is unsatisfiable. */
	if (Vsize(SimWrLits) == 0) {
	  return 0;
	}
#ifdef LEARNING
	/* If the conditions are satisfied, learn the working reason. */
	if ((SimParam[SIM_LEARN_TYPE] == SIM_RELEVANCE) || 
	    (Vsize(SimWrLits) <= SimParam[SIM_LEARN_ORDER])) {  
          /* An optimization: if learning is SIM_RELEVANCE, clauses that
             are bound to be deleted in this loop are not learned. */
	  if ((wrClause = SimDllMakeClauseFromWr(1)) != 0) {
            SimDllLearnClause(wrClause);  
          }
	} else {
	  wrClause = 0;
	}
#endif  
	break;

      case SIMLSPLIT:

	/* Change the sign and the mode. */
	*sign = (p -> teta == SIM_TT ? SIM_FF : SIM_TT);
	*mode = SIMRSPLIT;

	/* Undo the assignment here. */
	SimDllRetractProp(p);
	SimDllStatUpdateMin(SIMDEPTH_MIN, SimCnt[SIMCUR_LEVEL]);

#ifdef LEARNING
	/* Update the current level. */
        SimCnt[SIMCUR_LEVEL]--;
	/* Propagate the learned clauses that are unit. */
	res = SIM_SAT;
	for (i = Vsize(SimUnitLearned) - 1; i >= 0; i--) {
	  if (V(SimUnitLearned)[i] -> learned > SimCnt[SIMCUR_LEVEL]) {
	    cl = V(SimUnitLearned)[i];
	    VforceBack(SimBcpStack, cl);
	    SimDllStatInc(SIMULEARN_NUM);
	    if ((res = SimDllBcp()) == SIM_UNSAT) {
	      break;
	    }
	  }
	}
	if (res == SIM_UNSAT) {
	  /* Contradiction: continue backtracking. */
	  Vflush(SimBcpStack);
	  /* Clean the working reason and initialize it with
	     the literals of the conflict (empty) clause. */
	  SimDllInitWr(SimConflictCl);
	  /* Some trace. */
	  SimContradictionFoundOn(Vback(SimStack), SimCnt[SIMCUR_LEVEL]);
	  SimWrIs(SimWrLits);
	  /* Go on with the backtracking loop. */
	  continue;
	} else {
	  /* No contradiction. */
	  if (p -> teta != SIM_DC) {
	    /* p was assigned, call the heuristics. */
	    if (SimDllFormulaIsEmpty) {
	      return 0;
	    } else {
	      return SimDllChooseLiteral(sign, mode);
	    }
	  } else {
	    /* Restore the correct level. */
	    SimCnt[SIMCUR_LEVEL]++;
	  }
	}
#endif

	/* Assign a reason to SIMRSPLIT. */
	if (wrClause == 0) { 
	  p -> reason = SimDllMakeClauseFromWr(0); 
	} else { 
	  p -> reason = wrClause; 
	} 
	SimDllStatInc(SIMFDA_NUM);
	return p;
      
      }

    } /* (V(SimLitInWr)[p -> prop] != 0) */

    SimSkippingProp(p, SimCnt[SIMCUR_LEVEL]);
    
    switch (p -> mode) {
    case SIMLSPLIT:
      SimDllStatInc(SIMSKIP_NUM);
      skipped++;
      SimDllStatUpdateMax(SIMSKIP_MAX, skipped);
    case SIMRSPLIT:
      /* Undo the assignment. */
      SimDllRetractProp(p);
      /* Update the current level. */
      SimCnt[SIMCUR_LEVEL]--;
      break;
    default:
      /* Undo the assignment. */
      SimDllRetractProp(p);
    }
   
  }  
#endif

  /* The stack is empty: end of search. */
  return 0;

} /* End of SimDllBackjumping. */


/**Function********************************************************************

  Synopsis    [Initializes the working reason. ]

  Description [Cleans the flags to test membership in the working reason and 
              stores the literals of the conflict (empty) clause.]

  SideEffects [none]

  SeeAlso     [SimDllBackjumping]

******************************************************************************/
void 
SimDllInitWr(
  SimClause_t * cl)
{

#ifdef BACKJUMPING
  SimProp_t  * p;
  SimProp_t ** genp;

  /* Clean the membership flags and reset the literals in the Wr. */
  Vforeach0(SimWrLits, genp, p) {
    V(SimLitInWr)[SimRef(p) -> prop] = 0;
  }
  Vflush(SimWrLits);

  /* Update the set of literals in the reason. */
  Vforeach0(cl -> lits, genp, p) {
    VforceBack(SimWrLits, p);
    V(SimLitInWr)[SimRef(p) -> prop] = Vsize(SimWrLits);
  }
#endif

  return;

} /* End of SimDllInitWr */


/**Function********************************************************************

  Synopsis    [Resolves the given clause with the working reason. ]

  Description [Resolves the given clause with the working reason. ]

  SideEffects [none]

  SeeAlso     [SimDllBackjumping]

******************************************************************************/
void 
SimDllResolveWithWr(
  SimClause_t * cl, 
  SimProp_t   * pRes)
{

#ifdef BACKJUMPING
  SimProp_t  * p;
  SimProp_t ** genp;
  int          i;

  Vforeach0(cl -> lits, genp, p) {
    if (V(SimLitInWr)[SimRef(p) -> prop] == 0) {
      VforceBack(SimWrLits, p);
      V(SimLitInWr)[SimRef(p) -> prop] = Vsize(SimWrLits);
    } 
  }

  /* Remove from the set the proposition prop. */
  i = V(SimLitInWr)[pRes -> prop];
  V(SimWrLits)[i - 1] = Vback(SimWrLits);
  p = V(SimWrLits)[i - 1];
  V(SimLitInWr)[SimRef(p) -> prop] = i;
  VdropBack0(SimWrLits);
  V(SimLitInWr)[pRes -> prop] = 0;
#endif

  return;

} /* End of SimDllResolveWithWr */


/**Function********************************************************************

  Synopsis    [Creates a clause from the working reason. ]

  Description [Creates a clause from the working reason. ]

  SideEffects [none]

  SeeAlso     [SimDllBackjumping]

******************************************************************************/
SimClause_t * 
SimDllMakeClauseFromWr(
  int         optimize
)
{

  SimClause_t * cl = 0;

#ifdef BACKJUMPING
  int          del;
  SimProp_t  * p;
  SimProp_t ** genp;

  del = 0;
  /* Create a new clause. */
  cl =  SimClauseInit(0, Vsize(SimWrLits));

  /* Copy literals into the new clause. */  
#ifdef LEARNING
  if (optimize && (SimParam[SIM_LEARN_TYPE] == SIM_RELEVANCE)) {
    /* With relevance learning, check the assignment level of the literals. */
    Vforeach0(SimWrLits, genp, p) {
      VforceBack0(cl -> lits, p);
      if (SimRef(p) -> level == SimCnt[SIMCUR_LEVEL]) {
        ++del;
        if (del > SimParam[SIM_LEARN_ORDER]) {
	  /* The clause will be deleted anyway: doing it now saves time. */
          SimClauseClear(cl);
          return 0;
        }
      }
    }
  } else    
#endif
  Vforeach0(SimWrLits, genp, p) {
    VforceBack0(cl -> lits, p);
  }

#endif
  return cl;

} /* End of SimDllMakeClauseFromWr. */


/**Function********************************************************************

  Synopsis    [Learns a clause.]

  Description [Adds the clause to the list of learned clauses and links the 
               occurrences of propositions in the clause.]

  SideEffects []

  SeeAlso     [SimDllBackjumping]

******************************************************************************/
void 
SimDllLearnClause(
  SimClause_t * cl)
{

#ifdef LEARNING
  SimProp_t   * p;
  SimProp_t ** genp;

  SimDllStatInc(SIMLEARN_NUM);
  if (Vsize(cl -> lits) <= SimParam[SIM_LEARN_ORDER]) {
    SimDllStatInc(SIMSLEARN_NUM);
  }

  /* Put the clause in the database. */
  cl -> backClauses = Vsize(SimLearned);
  VpushBackGrow(SimLearned, cl, SIMLEARNED_GROW);

  /* Stamp the clause with the current level. */
  cl -> learned = SimCnt[SIMCUR_LEVEL];

  /* Update occurrences of the propositions in the clause. */
  Vforeach0(cl -> lits, genp, p) {
    if (SimIsPos(p)) {
      VpushBackGrow0(p -> posLits, cl, SIMOCC_GROW);
    } else {
      p = SimRef(p);
      VpushBackGrow0(p -> negLits, cl, SIMOCC_GROW);
    }
  }

  SimDllLearningClause(cl); 
  SimDllStatUpdateMin(SIMLEARN_MIN, Vsize(cl -> lits));
  SimDllStatUpdateMax(SIMLEARN_MAX, Vsize(cl -> lits));
  SimDllStatAdd(SIMLEARN_AVE, Vsize(cl -> lits));
#endif

  return;

} /* End of SimDllLearnClause. */


/**Function********************************************************************

  Synopsis    [Delete clause from learned clauses database.]

  Description [Delete clause from learned clauses database.]

  SideEffects [none]

  SeeAlso     [SimDllBackjumping]

******************************************************************************/
void 
SimDllUnlearnClause(
 SimClause_t * cl)
{

#ifdef LEARNING
  SimProp_t  * p;
  SimProp_t ** genp;
  int          i;

  /* Update occurrences of the propositions in the clause. */
  Vforeach0(cl -> lits, genp, p) {
    if (SimIsPos(p)) {
      for (i = Vsize(p -> posLits) - 1; 
	   (V(p -> posLits)[i] != 0) && (V(p -> posLits)[i] != cl); i--);
      SimDllAssert((V(p -> posLits)[i] != 0), SIM_LRN_LOCATION, 
		   "SimDllUnlearnClause: could not find occurrence");
      V(p -> posLits)[i] = Vback(p -> posLits);
      VdropBack0(p -> posLits);
    } else {
      p = SimRef(p);
      for (i = Vsize(p -> negLits) - 1; 
	   (V(p -> negLits)[i] != 0) && (V(p -> negLits)[i] != cl); i--);
      SimDllAssert((V(p -> negLits)[i] != 0), SIM_LRN_LOCATION, 
		   "SimDllUnlearnClause: could not find occurrence");
      V(p -> negLits)[i] = Vback(p -> negLits);
      VdropBack0(p -> negLits);
    }
  }

  /* Remove the clause from learned database (swap it with the tail). */
  i =  cl -> backClauses;
  V(SimLearned)[i] = Vback(SimLearned);
  V(SimLearned)[i] -> backClauses = i;
  V(SimLearned)[i] -> backClauses = i;
  VdropBack(SimLearned);

  /* Eventually remove the clause from the unit learned stack. */
  if ((i = cl -> backUnitLearned) != SIMFORBID_UNIT) {
    V(SimUnitLearned)[i] = Vback(SimUnitLearned);
    VdropBack(SimUnitLearned);
    V(SimUnitLearned)[i] -> backUnitLearned = i;
  }

  SimClauseClear(cl);
#endif

  return;

} /* End of SimDllUnlearnClause. */








