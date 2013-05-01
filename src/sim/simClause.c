/**CFile***********************************************************************

  FileName    [simClause.c]

  PackageName [sim]

  Synopsis    [Primitives for clauses.]

  Description [Internal procedures included in this module:
		<ul>
		<li> <b>SimClauseInit()</b> clause construtor
		<li> <b>SimClauseClear()</b> clause destructor
		</ul>]
		
  SeeAlso     [sim.h]

  Author      [Armando Tacchella]

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

  Synopsis    [Initializes a clause.]

  Description [Initializes a clause.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
SimClause_t*
SimClauseInit(
  int   clId,
  short clSize)		  
{
  
  SimClause_t * cl = (SimClause_t*)malloc(sizeof(SimClause_t));
  SimDllMemCheck(cl, "SimClauseInit");

  /* Initialize data fields. */
  cl -> openLitNum = cl -> posLitNum = 0;
  cl -> sub = 0;
  cl -> backClauses = clId;

  /* Initialize 0-terminated list of proposition references. */
  VinitReserve0(cl -> lits, clSize);
  SimDllMemCheck(V(cl -> lits), "SimClauseInit");
  
#ifdef HORN_RELAXATION
  cl -> backNhClauses = -1;
#endif
#ifdef LEARNING
  cl -> backUnitLearned = SIMALLOW_UNIT;
  cl -> learned = -1;
#endif

  return cl;

} /* End of SimClauseInit. */


/**Function********************************************************************

  Synopsis    [Clears a clause.]

  Description [Clears a clause.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void 
SimClauseClear(
  SimClause_t * cl)
{
  
  Vclear(cl -> lits);
  free(cl);

  return;

}
