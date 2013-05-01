/**CFile***********************************************************************

  FileName    [simCons.c]

  PackageName [sim]

  Synopsis    [Consistency checking routines.]

  Description [External procedures included in this module:
		<ul>
		<li> <b>SimDllGetModels()</b> checks if the
                     number of open clauses is zero. If so, decrements 
		     the number of solutions to find;   
		<li> <b>SimDllGetModelsHorn()</b> checks if the
                     number of open non-horn clauses is zero. If so, 
		     decrements the number of solutions to find.
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

  Synopsis    [Consistency checkingl.]

  Description [Consistency checking.]

  SideEffects [none]

  SeeAlso     [Sim_DllSolve]

******************************************************************************/
int 
SimDllGetModels( 
   int       * stop	   
  )
{
  int redundant;

  if (SimCnt[SIMCUR_CL_NUM] == 0) {
    /* If the formula was declared satisfiable, check the assignment found. */
    if (SimDllCheck(&redundant) == SIM_UNSAT) {
      SimDllThrow(SIM_INTERNAL_ERROR, SIM_CONS_LOCATION,  
		  "Sim_DllCheck: assignment is not verified!");
    } else if (redundant != 0) { 
      fprintf(stderr, "Warning: there are %d redundant assignments\n", 
	      redundant); 
    } 
    /* If the number of open clauses is 0, check if the number of request 
       solution is reached. If so stop, otherwise go on in the enumeration. */
    if( --SimParam[SIM_SOL_NUM] == 0) {
      *stop = 1;
    } else {
      *stop = 0;
    }
    SimDllStatAdd(SIMSOL_NODE_NUM, SimCnt[SIMCUR_LEVEL]);
    SimDllStatUpdateMax(SIMDEPTH_MAX, SimCnt[SIMCUR_LEVEL]);
    return SIM_SAT;
  } else {
    /* If there are still open clauses at this point, return SIM_UNSAT. */
    *stop = 1;
    return SIM_UNSAT;
  }

} /* End of SimDllGetModels. */


/**Function********************************************************************

  Synopsis    [Consistency checking with horn relaxation.]

  Description [Consistency checking  with horn relaxation.]

  SideEffects [none]

  SeeAlso     [Sim_DllSolve]

******************************************************************************/
int 
SimDllGetModelsHorn(
   int       * stop       
  )
{

  int redundant;

  if (SimCnt[SIMCUR_NHCL_NUM] == 0) {
    /* If the formula was declared satisfiable, check the assignment found. */
    if (SimDllCheck(&redundant) == SIM_UNSAT) {
      SimDllThrow(SIM_INTERNAL_ERROR, SIM_CONS_LOCATION, 
		  "Sim_DllCheck: assignment is not verified!");
    } else if (redundant != 0) { 
      fprintf(stderr, "Warning: there are %d redundant assignments\n", 
	      redundant); 
    } 
  /* If the number of open clauses is 0, check if the number of request 
     solution is reached. If so stop, otherwise go on in the enumeration. */
    if( --SimParam[SIM_SOL_NUM] == 0) {
      *stop = 1;
    } else {
      *stop = 0;
    }
    SimDllStatAdd(SIMSOL_NODE_NUM, SimCnt[SIMCUR_LEVEL]);
    SimDllStatUpdateMax(SIMDEPTH_MAX, SimCnt[SIMCUR_LEVEL]);
    return SIM_SAT;
  } else {
    /* If there are still open clauses at this point, return SIM_UNSAT. */
    *stop = 1;
    return SIM_UNSAT;
  }


} /* End of SimDllGetModelsHorn */





