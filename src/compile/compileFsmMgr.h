/**CHeaderFile*****************************************************************

  FileName    [compileFsmMgr.h]

  PackageName [compile]

  Synopsis [Public interface for a high level object that can contruct
  FSMs]

  Description [Declares the interface of an high-level object that
  lives at top-level, that is used to help contruction of FSMs.  It
  can control information that are not shared between lower levels, so
  it can handle with objects that have not the full knowledge of the
  whole system]
  
  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and ITC-irst. 

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



#ifndef __COMPILE_FSM_MGR_H__
#define __COMPILE_FSM_MGR_H__

#include "utils/utils.h"
#include "dd/dd.h"
#include "fsm/sexp/SexpFsm.h"
#include "fsm/bdd/BddFsm.h"

typedef struct FsmBuilder_TAG* FsmBuilder_ptr;



#define FSM_BUILDER(x) \
         ((FsmBuilder_ptr) x)

#define FSM_BUILDER_CHECK_INSTANCE(x) \
         ( nusmv_assert(FSM_BUILDER(x) != FSM_BUILDER(NULL)) )


/* ---------------------------------------------------------------------- */
/* Public interface                                                       */
/* ---------------------------------------------------------------------- */
EXTERN FsmBuilder_ptr 
FsmBuilder_create ARGS((DdManager* dd));

EXTERN void  FsmBuilder_destroy ARGS((FsmBuilder_ptr self));


EXTERN SexpFsm_ptr 
FsmBuilder_create_sexp_fsm ARGS((const FsmBuilder_ptr self, 
				 const Encoding_ptr senc, 
				 VarSet_ptr vars_list, 
				 const enum SexpFsmType type));

EXTERN BddFsm_ptr 
FsmBuilder_create_bdd_fsm ARGS((const FsmBuilder_ptr self, 
				BddEnc_ptr enc, 
				const SexpFsm_ptr sexp_fsm, 
				const TransType trans_type));


#endif /* __COMPILE_FSM_MGR_H__ */
