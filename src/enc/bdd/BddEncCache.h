/**CHeaderFile*****************************************************************

  FileName    [BddEncCache.h]

  PackageName [enc.bdd]

  Synopsis    [The Bdd encoding cache public interface]

  Description []
                                               
  SeeAlso     [BddEncCache.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.bdd'' package of NuSMV version 2. 
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

#ifndef __ENC_BDD_BDD_ENC_CACHE_H__
#define __ENC_BDD_BDD_ENC_CACHE_H__

#include "bdd.h"
#include "BddEnc.h"

#include "utils/utils.h"
#include "node/node.h"

/**Type***********************************************************************

  Synopsis     [The BddEncCache type ]

  Description  [The BddEncCache type ]  

  Notes        []

******************************************************************************/
typedef struct BddEncCache_TAG*  BddEncCache_ptr;
#define BDD_ENC_CACHE(x) \
          ((BddEncCache_ptr) x)

#define BDD_ENC_CACHE_CHECK_INSTANCE(x) \
          ( nusmv_assert(BDD_ENC_CACHE(x) != BDD_ENC_CACHE(NULL)) )


/* ---------------------------------------------------------------------- */
/* Public methods                                                         */
/* ---------------------------------------------------------------------- */

EXTERN BddEncCache_ptr BddEncCache_create ARGS((BddEnc_ptr enc));
EXTERN void BddEncCache_destroy ARGS((BddEncCache_ptr self));

EXTERN void BddEncCache_push_status_and_reset ARGS((BddEncCache_ptr self));

EXTERN void BddEncCache_pop_status ARGS((BddEncCache_ptr self));

EXTERN void 
BddEncCache_new_constant ARGS((BddEncCache_ptr self, node_ptr constant, 
			       add_ptr constant_add));

EXTERN boolean 
BddEncCache_is_constant_defined ARGS((BddEncCache_ptr self, 
				      node_ptr constant));

EXTERN add_ptr 
BddEncCache_lookup_constant ARGS((BddEncCache_ptr self, node_ptr constant));

EXTERN void BddEncCache_new_var ARGS((BddEncCache_ptr self, node_ptr var_name, 
				      add_ptr var_add));

EXTERN boolean 
BddEncCache_is_var_defined ARGS((BddEncCache_ptr self, node_ptr var_name));

EXTERN add_ptr 
BddEncCache_lookup_var ARGS((BddEncCache_ptr self, node_ptr var_name));

EXTERN void 
BddEncCache_set_definition ARGS((BddEncCache_ptr self, node_ptr name, 
				 add_ptr def));

EXTERN add_ptr 
BddEncCache_get_definition ARGS((BddEncCache_ptr self, node_ptr name));

EXTERN void 
BddEncCache_set_evaluation ARGS((BddEncCache_ptr self, node_ptr name, 
				 add_ptr add));

EXTERN add_ptr BddEncCache_get_evaluation ARGS((BddEncCache_ptr self, 
						node_ptr name));


#endif /* __ENC_BDD_BDD_ENC_CACHE_H__ */
