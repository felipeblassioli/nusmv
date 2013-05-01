/**CFile***********************************************************************

  FileName    [simProp.c]

  PackageName [sim]

  Synopsis    [Primitives for propositions]

  Description [Internal procedures included in this module:
		<ul>
		<li> <b>SimPropInit()</b> proposition construtor
		<li> <b>SimPropClear()</b> proposition destructor
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

  Synopsis    [Initializes a proposition.]

  Description [Initializes a proposition.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
SimProp_t*
SimPropInit(
  int   prop,
  short litSize)		  
{
  
  SimProp_t * p = (SimProp_t*)malloc(sizeof(SimProp_t));
  SimDllMemCheck(p, "SimPropInit");

  /* Initialize data fields. */
  p -> prop = prop;
  p -> teta = SIM_DC;
  p -> backProps = -1;
  p -> backModelProps = -1;

  /* Initialize 0-terminated list of clauses references. */
  VinitReserve0(p -> posLits, litSize);
  SimDllMemCheck(V(p -> posLits), "SimPropInit");
  VinitReserve0(p -> negLits, litSize);
  SimDllMemCheck(V(p -> negLits), "SimPropInit");

#ifdef BACKJUMPING
  p -> reason = 0;
#endif

  return p;

} /* End of SimPropInit. */


/**Function********************************************************************

  Synopsis    [Clears a proposition.]

  Description [Clears a proposition.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void 
SimPropClear(
  SimProp_t * p)
{
  
  Vclear(p -> posLits);
  Vclear(p -> negLits);
  free(p);

  return;

}



