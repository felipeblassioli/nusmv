/**CHeaderFile*****************************************************************

  FileName    [BddTrans.h]

  PackageName [trans.bdd]

  Synopsis    [The header file of the class BddTrans]

  Description [ The package <tt> trans.bdd </tt> implements 
  classes to store and maipulate transition relation in bdd form]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``trans.bdd'' package of NuSMV version 2. 
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

  Revision    [$Id: BddTrans.h,v 1.1.2.3 2004/05/31 09:59:59 nusmv Exp $]

******************************************************************************/

#ifndef __TRANS_BDD_BDD_TRANS_H__
#define __TRANS_BDD_BDD_TRANS_H__

#include "trans/generic/GenericTrans.h"

#include "ClusterList.h"
#include "ClusterOptions.h"

#include "utils/utils.h" /* for EXTERN and ARGS */
#include "dd/dd.h"

/**Type***********************************************************************

  Synopsis    [The structure used to represent the transition relation.]

  Description []

******************************************************************************/
typedef struct BddTrans_TAG* BddTrans_ptr;

#define BDD_TRANS(x)  \
        ((BddTrans_ptr) x)

#define BDD_TRANS_CHECK_INSTANCE(x)  \
        (nusmv_assert(BDD_TRANS(x) != BDD_TRANS(NULL)))


/* ---------------------------------------------------------------------- */
/* RC: Enable this to auto-check trans after creation                     */
// #define TRANS_DEBUG_THRESHOLD  
/* ---------------------------------------------------------------------- */


EXTERN BddTrans_ptr 
BddTrans_create ARGS((DdManager* dd_manager, 
		   const ClusterList_ptr clusters_bdd, 
		   bdd_ptr state_vars_cube, 
		   bdd_ptr input_vars_cube, 
		   bdd_ptr next_state_vars_cube, 			      
		   const TransType trans_type, 
		   const ClusterOptions_ptr cl_options));

EXTERN ClusterList_ptr BddTrans_get_forward  ARGS((const BddTrans_ptr self));
EXTERN ClusterList_ptr BddTrans_get_backward ARGS((const BddTrans_ptr self));

EXTERN void 
BddTrans_apply_synchronous_product ARGS((BddTrans_ptr self, 
					 const BddTrans_ptr other, 
					 bdd_ptr state_vars_cube,
					 bdd_ptr input_vars_cube, 
					 bdd_ptr next_state_vars_cube));

EXTERN bdd_ptr 
BddTrans_get_forward_image_state ARGS((const BddTrans_ptr self, bdd_ptr s));

EXTERN bdd_ptr 
BddTrans_get_forward_image_state_input ARGS((const BddTrans_ptr self, 
					     bdd_ptr s));

EXTERN bdd_ptr 
BddTrans_get_backward_image_state ARGS((const BddTrans_ptr self, bdd_ptr s));

EXTERN bdd_ptr 
BddTrans_get_backward_image_state_input ARGS((const BddTrans_ptr self, 
					      bdd_ptr s));

EXTERN bdd_ptr 
BddTrans_get_k_forward_image_state ARGS((const BddTrans_ptr self, 
					 bdd_ptr s, int k));

EXTERN bdd_ptr 
BddTrans_get_k_forward_image_state_input ARGS((const BddTrans_ptr self, 
					       bdd_ptr s, int k));

EXTERN bdd_ptr 
BddTrans_get_k_backward_image_state ARGS((const BddTrans_ptr self, 
					  bdd_ptr s, int k));

EXTERN bdd_ptr 
BddTrans_get_k_backward_image_state_input ARGS((const BddTrans_ptr self, 
		  			        bdd_ptr s, int k));

EXTERN void BddTrans_print_short_info ARGS((const BddTrans_ptr self, 
					    FILE* file));



#endif /* __TRANS_BDD_BDD_TRANS_H__ */
