/**CHeaderFile*****************************************************************

  FileName    [bmcVarsMgr.h]

  PackageName [bmc]

  Synopsis    [Public interface for the VarsMgr class]

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

  Revision    [$Id: bmcVarsMgr.h,v 1.5.4.3.2.2 2004/07/28 13:37:27 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_VARS_MGR__H
#define _BMC_VARS_MGR__H


#include "utils/utils.h"
#include "be/be.h"
#include "enc/symb/Encoding.h"
#include "node/node.h"
#include "utils/assoc.h" /* for hash_ptr */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [The Variables Manager accessor type]

  Description [Whenever you want to access a VarsMgr instance you will 
  handle with this type]

  SeeAlso     [VarsMgr type]

******************************************************************************/
typedef struct BmcVarsMgr_TAG* BmcVarsMgr_ptr;


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define BMC_VARS_MGR(x) \
   ((BmcVarsMgr_ptr) x)

#define BMC_VARS_MGR_CHECK_INSTANCE(x) \
   (nusmv_assert(BMC_VARS_MGR(x) != BMC_VARS_MGR(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/* ----------------------------- */
/*  Constructors and destructor  */
/* ----------------------------- */

EXTERN BmcVarsMgr_ptr BmcVarsMgr_Create ARGS((Encoding_ptr senc));

EXTERN void BmcVarsMgr_Delete ARGS((BmcVarsMgr_ptr* self_ref));


/* --------- */
/*  Getters  */
/* --------- */

EXTERN Encoding_ptr BmcVarsMgr_GetSymbEncoding ARGS((const BmcVarsMgr_ptr self));
EXTERN Be_Manager_ptr BmcVarsMgr_GetBeMgr ARGS((const BmcVarsMgr_ptr self));
EXTERN int BmcVarsMgr_GetStateVarsNum ARGS((const BmcVarsMgr_ptr self));
EXTERN int BmcVarsMgr_GetInputVarsNum ARGS((const BmcVarsMgr_ptr self));
EXTERN int BmcVarsMgr_GetStateInputVarsNum ARGS((const BmcVarsMgr_ptr self));
EXTERN int BmcVarsMgr_GetMaxTime ARGS((const BmcVarsMgr_ptr self));

EXTERN int 
BmcVarsMgr_GetFirstInputVarIndex ARGS((const BmcVarsMgr_ptr self));

EXTERN int 
BmcVarsMgr_GetFirstCurrStateVarIndex ARGS((const BmcVarsMgr_ptr self));

EXTERN int 
BmcVarsMgr_GetFirstNextStateVarIndex ARGS((const BmcVarsMgr_ptr self));

/* ---------- */
/*  Shifters  */
/* ---------- */

EXTERN be_ptr
BmcVarsMgr_ShiftCurrState2Next ARGS((const BmcVarsMgr_ptr self, 
				     const be_ptr exp));

EXTERN be_ptr
BmcVarsMgr_ShiftCurrNext2Time ARGS((const BmcVarsMgr_ptr self,
				    const be_ptr exp, const int time));

EXTERN be_ptr
BmcVarsMgr_ShiftCurrNext2Times ARGS((const BmcVarsMgr_ptr self,
				     const be_ptr exp,
				     const int ctime, const int ntime));

EXTERN be_ptr
BmcVarsMgr_MkAndCurrNextInterval ARGS((const BmcVarsMgr_ptr self,
				       const be_ptr exp,
				       const int from, const int to));

EXTERN be_ptr
BmcVarsMgr_MkOrCurrNextInterval ARGS((const BmcVarsMgr_ptr self,
				      const be_ptr exp,
				      const int from, const int to));


/* ----------------------------------------------------------------------- */
/* name <-> curr <-> next <-> var_index <-> be_index <-> timed conversions */
/* ----------------------------------------------------------------------- */

/* from name to other: */
EXTERN be_ptr 
BmcVarsMgr_Name2Curr ARGS((const BmcVarsMgr_ptr self, 
			   const node_ptr var_name));

EXTERN be_ptr 
BmcVarsMgr_Name2Next ARGS((const BmcVarsMgr_ptr self, 
			   const node_ptr var_name));

EXTERN be_ptr 
BmcVarsMgr_Name2Timed ARGS((const BmcVarsMgr_ptr self,
			    const node_ptr var_name, const int time, 
			    const int max_time));

EXTERN int
BmcVarsMgr_Name2VarIndex ARGS((const BmcVarsMgr_ptr self, 
			       const node_ptr var_name));

/* from current to other: */
EXTERN node_ptr
BmcVarsMgr_Curr2Name ARGS((const BmcVarsMgr_ptr self, const be_ptr curvar));

EXTERN be_ptr
BmcVarsMgr_Curr2Next ARGS((const BmcVarsMgr_ptr self, const be_ptr curvar));

EXTERN be_ptr
BmcVarsMgr_Curr2Timed ARGS((const BmcVarsMgr_ptr self,
			    const be_ptr curvar, const int time, 
			    const int max_time));

/* from next to other: */
EXTERN node_ptr 
BmcVarsMgr_Next2Name ARGS((const BmcVarsMgr_ptr self, const be_ptr nextvar));

EXTERN be_ptr 
BmcVarsMgr_Next2Curr ARGS((const BmcVarsMgr_ptr self, const be_ptr nextvar));

EXTERN be_ptr
BmcVarsMgr_Next2Timed ARGS((const BmcVarsMgr_ptr self,
			    const be_ptr nextvar, const int time));

/* from timed to other: */
EXTERN node_ptr 
BmcVarsMgr_Timed2Name ARGS((const BmcVarsMgr_ptr self, const be_ptr timedvar));

EXTERN be_ptr
BmcVarsMgr_Timed2Curr ARGS((const BmcVarsMgr_ptr self, const be_ptr timedvar));

EXTERN be_ptr
BmcVarsMgr_Timed2Next ARGS((const BmcVarsMgr_ptr self, const be_ptr timedvar));


/* var index (state and input variable index) to other: */
EXTERN node_ptr
BmcVarsMgr_VarIndex2Name ARGS((const BmcVarsMgr_ptr self, const int index));

EXTERN be_ptr 
BmcVarsMgr_VarIndex2Curr ARGS((const BmcVarsMgr_ptr self, const int index));

EXTERN be_ptr 
BmcVarsMgr_VarIndex2Next ARGS((const BmcVarsMgr_ptr self, const int index));

EXTERN be_ptr
BmcVarsMgr_VarIndex2Timed ARGS((const BmcVarsMgr_ptr self,
				const int index, const int time, 
				const int max_time));
EXTERN int 
BmcVarsMgr_VarIndex2BeIndex ARGS((const BmcVarsMgr_ptr self,
				  const int var_idx, const int time, 
				  const int max_time));

/* --------------------------------------------------- */
/* Time <-> var index <-> be index conversion routines */
/* --------------------------------------------------- */

EXTERN int 
BmcVarsMgr_Time2BeIndex ARGS((const BmcVarsMgr_ptr self, const int time));

EXTERN int 
BmcVarsMgr_BeIndex2Time ARGS((const BmcVarsMgr_ptr self, const int be_idx));

EXTERN int 
BmcVarsMgr_BeIndex2VarIndex ARGS((const BmcVarsMgr_ptr self, 
				  const int be_idx));

/* --------------------------------------------------------------- */
/* Tests on index to decide whether a variable is state or input   */
/* --------------------------------------------------------------- */

EXTERN boolean
BmcVarsMgr_IsBeIndexStateVar ARGS((const BmcVarsMgr_ptr self, 
				   const int index, const int max_time));

EXTERN boolean
BmcVarsMgr_IsBeIndexInputVar ARGS((const BmcVarsMgr_ptr self, 
				   const int index, const int max_time));

EXTERN boolean
BmcVarsMgr_IsBeIndexInVarsBlock ARGS((const BmcVarsMgr_ptr self, 
				     const int index));

EXTERN boolean
BmcVarsMgr_IsBeIndexInCurrStateBlock ARGS((const BmcVarsMgr_ptr self, 
					   const int index));

EXTERN boolean
BmcVarsMgr_IsBeIndexInInputBlock ARGS((const BmcVarsMgr_ptr self, 
				       const int index));

EXTERN boolean
BmcVarsMgr_IsBeIndexInCurrStateInputBlock ARGS((const BmcVarsMgr_ptr self, 
						const int index));

EXTERN boolean
BmcVarsMgr_IsBeIndexInNextStateBlock ARGS((const BmcVarsMgr_ptr self, 
					   const int index));


/**AutomaticEnd***************************************************************/

#endif /* _BMC_VARS_MGR__H */

