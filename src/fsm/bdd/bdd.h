/**CHeaderFile*****************************************************************

  FileName    [bdd.h]

  PackageName [fsm.bdd]

  Synopsis    [Declares the public interface for the package fsm.bdd]

  Description []
  
  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.bdd'' package of NuSMV version 2. 
  Copyright (C) 2003 by ITC-irst. 

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


#ifndef __FSM_BDD_BDD_H__
#define __FSM_BDD_BDD_H__

#include "utils/utils.h"  /* for EXTERN and ARGS */
#include "dd/dd.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef bdd_ptr BddStates;
#define BDD_STATES(x) \
          ((BddStates) x)

typedef bdd_ptr BddInputs;
#define BDD_INPUTS(x) \
          ((BddInputs) x)

typedef bdd_ptr BddStatesInputs;
#define BDD_STATES_INPUTS(x) \
          ((BddStatesInputs) x)

typedef bdd_ptr BddInvarStates;
#define BDD_INVAR_STATES(x) \
          ((BddInvarStates) x)

typedef bdd_ptr BddInvarInputs;
#define BDD_INVAR_INPUTS(x) \
          ((BddInvarInputs) x)


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void Bdd_Init ARGS((void));
EXTERN void Bdd_End ARGS((void));

/**AutomaticEnd***************************************************************/


#endif /* __FSM_BDD_BDD_H__ */
