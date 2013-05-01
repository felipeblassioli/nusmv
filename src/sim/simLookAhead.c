/**CFile***********************************************************************

  FileName    [simLookAhead.c]

  PackageName [sim]

  Synopsis    [A compendium of lookahead techniques.]

  Description [Internal procedures included in this module:
	        <ul>
		<li> <b>SimDllBcp()</b> performs Binary Constraint Propagation;
		<li> <b>SimDllMlf()</b> performs Monotone Literal Fixing.
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
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Performs Boolean Constraint Propagation (Unit resolution)]

  Description [Keeps propagating unit clauses until an empty clause is 
               found or no more unit clauses are created. If
	       BACKJUMPING is definded stores the reason of the unit
	       propagation. If LEARNING is defined discards subsumed clauses.
	       Returns SIM_UNSAT if an empty clause is found,
	       SIM_SAT otherwise. ]

  SideEffects [none]

  SeeAlso     [Sim_DllSolve SimDllExtendProp]

******************************************************************************/
int 
SimDllBcp()
{

  int            res;
  SimClause_t  * cl;
  SimProp_t   ** genp;
  SimProp_t    * p;

  /* Be optimistic! */
  res = SIM_SAT;

  /* Cycle through unit clauses starting from Bcp stack top: stop if
     res becomes SIM_UNSAT (empty clause detected by SimDllExtendProp). */
  while ((Vsize(SimBcpStack) > 0) && (res == SIM_SAT)) {

    /* Pop off from the stack a unit clause pointer. */
    cl = VextractBack(SimBcpStack);
    /* If the clause lost its eligibility for bcp, go on. */
    if (!SimClauseIsOpen(cl)) {
      continue;
    }
    SimDllAssert((cl -> openLitNum == 1), SIM_BCP_LOCATION, 
		 "SimDllBcp: more than one open literal!");

    /* Search for the lone unvalued literal. */
    Vforeach0(cl -> lits, genp, p) {
      if (SimRef(p) -> teta == SIM_DC) {
	break;
      }
    }
  
#ifdef LEARNING
    if ((cl -> learned != -1) && (p == 0)) {
      continue;
    }
#endif
    SimDllAssert((p != 0), SIM_BCP_LOCATION,
		 "SimDllBcp: cannot find unvalued literal!"); 

    /* Propagate the valuation. */
    res = 
      SimDllExtendProp(SimRef(p), (SimIsPos(p) ? SIM_TT : SIM_FF), SIMUNIT);

#ifdef BACKJUMPING
    SimRef(p) -> reason = cl;
#endif

    SimDllStatInc(SIMUNIT_NUM);
    SimPropHasValueBy(SimRef(p), SimCnt[SIMCUR_LEVEL], "unit");
  }
  
  return res;

} /* End of SimDllBcp. */


/**Function********************************************************************

  Synopsis    [Performs Monotone Literal Fixing (Pure literal). ]

  Description [Propagates all the pure literals in the MLF stack. If
               the literal was propagate with another mode it is discarded.]

  SideEffects [none]

  SeeAlso     [Sim_DllSolve SimDllExtendProp]

******************************************************************************/
void 
SimDllMlf()  
{
#ifdef PURE_LITERAL
  int           teta;
  SimProp_t   * p;


  /* Cycle through pure literals starting from Mlf stack top. */
  while (Vsize(SimMlfStack) > 0) {

    /* Check if the number of open clauses became zero. */
    if (SimDllFormulaIsEmpty) { 
      Vflush(SimMlfStack);
      break;
    }

    /* Pop off from the stack a proposition pointer. */
    p = VextractBack(SimMlfStack);
    /* If the proposition lost its eligibility for mlf, go on. */
    if ((p -> mode != SIMPUREPOS) && (p -> mode != SIMPURENEG)) {
      continue;
    }

    /* Understand the sign of the literal. */
    teta = (p -> mode == SIMPUREPOS ? SIM_TT : SIM_FF);

    /* Propagate the valuation. */
    SimDllExtendProp(p, teta, p -> mode);
  
    SimDllStatInc(SIMPURE_NUM);
    SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL], "pure literal");
    
  }
#endif

  return;

} /* End of SimDllMlf. */







