/**CHeaderFile*****************************************************************

  FileName    [EncCache.h]

  PackageName [enc.symb]

  Synopsis    [The generic encoding cache public interface]

  Description []
                                               
  SeeAlso     [EncCache.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.symb'' package of NuSMV version 2. 
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

#ifndef __ENC_SYMB_ENC_CACHE_H__
#define __ENC_SYMB_ENC_CACHE_H__

#include "utils/utils.h"
#include "node/node.h"


/**Type***********************************************************************

  Synopsis     [The EncCache type ]

  Description  [The EncCache type ]  

  Notes        []

******************************************************************************/
typedef struct EncCache_TAG*  EncCache_ptr;
#define ENC_CACHE(x) \
          ((EncCache_ptr) x)

#define ENC_CACHE_CHECK_INSTANCE(x) \
          ( nusmv_assert(ENC_CACHE(x) != ENC_CACHE(NULL)) )


/* ---------------------------------------------------------------------- */
/* Public methods                                                         */
/* ---------------------------------------------------------------------- */
EXTERN EncCache_ptr EncCache_create ARGS((void));
EXTERN void EncCache_destroy ARGS((EncCache_ptr self));
EXTERN void EncCache_push_status_and_reset ARGS((EncCache_ptr self));
EXTERN void EncCache_pop_status ARGS((EncCache_ptr self));

EXTERN void EncCache_new_constant ARGS((EncCache_ptr self, node_ptr c));
EXTERN void EncCache_new_input_var ARGS((EncCache_ptr self, node_ptr var, 
					 node_ptr range));
EXTERN void EncCache_new_state_var ARGS((EncCache_ptr self, node_ptr var, 
					 node_ptr range));
EXTERN void EncCache_new_define ARGS((EncCache_ptr self, node_ptr name, 
				      node_ptr ctx, node_ptr definition));
EXTERN node_ptr 
EncCache_lookup_symbol ARGS((EncCache_ptr self, node_ptr name));

EXTERN node_ptr EncCache_get_constants_list ARGS((EncCache_ptr self));
EXTERN node_ptr EncCache_get_symbols_list ARGS((EncCache_ptr self));

EXTERN boolean 
EncCache_is_symbol_state_var ARGS((EncCache_ptr self, node_ptr name));

EXTERN boolean 
EncCache_is_symbol_input_var ARGS((EncCache_ptr self, node_ptr name));

EXTERN boolean EncCache_is_symbol_var ARGS((EncCache_ptr self, node_ptr name));

EXTERN boolean 
EncCache_is_symbol_declared ARGS((EncCache_ptr self, node_ptr name));

EXTERN boolean 
EncCache_is_symbol_define ARGS((EncCache_ptr self, node_ptr name));

EXTERN boolean EncCache_is_constant_defined ARGS((EncCache_ptr self, 
						  node_ptr constant));

#endif /* __ENC_SYMB_ENC_CACHE_H__ */
