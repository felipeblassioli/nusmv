/**CHeaderFile*****************************************************************

  FileName    [bmcFsm.h]

  PackageName [bmc]

  Synopsis    [Public interface of the Finite State Machine class in BE format]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by ITC-irst and University of Trento.

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

  Revision    [$Id: bmcFsm.h,v 1.2.4.3.2.1 2004/07/27 12:12:12 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_FSM__H
#define _BMC_FSM__H


#include "utils/utils.h"
#include "bmcVarsMgr.h"

/* ====================================================================== */
/* in order to include "compile.h"!!!! */
#include "sm/sm.h"
#include "util.h"
#include "utils/utils.h"
#include "node/node.h"
#include "dd/dd.h"
#include "set/set.h"
#include "rbc/rbc.h"
#include "parser/parser.h"
#include "compile/compile.h"
/* ====================================================================== */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct*********************************************************************

  Synopsis           [This is the Bmc_Fsm accessor type]
  Description        []
  SideEffects        []
  SeeAlso            [Bmc_Fsm]

******************************************************************************/
typedef struct Bmc_Fsm_TAG* Bmc_Fsm_ptr;


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


EXTERN Bmc_Fsm_ptr 
Bmc_Fsm_CreateFromSexprFSM ARGS((BmcVarsMgr_ptr vars_manager, 
				 const SexpFsm_ptr bfsm)); 

EXTERN Bmc_Fsm_ptr 
Bmc_Fsm_Create ARGS((BmcVarsMgr_ptr vars_manager, 
		     const be_ptr init, 
		     const be_ptr invar, 
		     const be_ptr trans, 
		     const node_ptr list_of_be_fairness)); 

EXTERN void Bmc_Fsm_Delete                ARGS((Bmc_Fsm_ptr* self_ref)); 
EXTERN Bmc_Fsm_ptr Bmc_Fsm_Dup            ARGS((Bmc_Fsm_ptr fsm)); 
EXTERN be_ptr Bmc_Fsm_GetInit             ARGS((const Bmc_Fsm_ptr self)); 
EXTERN be_ptr Bmc_Fsm_GetInvar            ARGS((const Bmc_Fsm_ptr self)); 
EXTERN be_ptr Bmc_Fsm_GetTrans            ARGS((const Bmc_Fsm_ptr self)); 
EXTERN node_ptr Bmc_Fsm_GetListOfFairness ARGS((const Bmc_Fsm_ptr self)); 
EXTERN BmcVarsMgr_ptr Bmc_Fsm_GetVarsManager ARGS((const Bmc_Fsm_ptr self)); 

/**AutomaticEnd***************************************************************/

#endif /* _BMC_FSM__H */

