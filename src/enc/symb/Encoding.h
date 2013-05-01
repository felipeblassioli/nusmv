/**CHeaderFile*****************************************************************

  FileName    [Encoding.h]

  PackageName [enc.symb]

  Synopsis    [The generic Encoding object interface]

  Description []
                                               
  SeeAlso     [Encoding.c]

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

#ifndef __ENCODING_H__
#define __ENCODING_H__

#include "GroupSet.h"
#include "utils/utils.h"
#include "utils/NodeList.h"



/**Type***********************************************************************

  Synopsis    [Generic and symbolic encoding]

  Description []

******************************************************************************/
typedef struct Encoding_TAG*  Encoding_ptr;

#define ENCODING(x)  \
        ((Encoding_ptr) x)

#define ENCODING_CHECK_INSTANCE(x)  \
        (nusmv_assert(ENCODING(x) != ENCODING(NULL)))


/* ---------------------------------------------------------------------- */
/*     Public methods                                                     */
/* ---------------------------------------------------------------------- */

/* creation, destruction */
EXTERN Encoding_ptr Encoding_create ARGS((void));
EXTERN void Encoding_destroy ARGS((Encoding_ptr self));

/* vars encoding */
EXTERN void Encoding_encode_vars ARGS((Encoding_ptr self));
EXTERN void Encoding_encode_var ARGS((Encoding_ptr self, node_ptr name)); 

/* inner lists access: */
EXTERN NodeList_ptr 
Encoding_get_constants_list ARGS((const Encoding_ptr self));

EXTERN int Encoding_get_constants_num ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr 
Encoding_get_model_state_symbols_list ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr 
Encoding_get_model_input_symbols_list ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr 
Encoding_get_model_state_input_symbols_list ARGS((const Encoding_ptr self));


EXTERN NodeList_ptr 
Encoding_get_state_vars_list ARGS((const Encoding_ptr self));

EXTERN int Encoding_get_state_vars_num ARGS((const Encoding_ptr self));

EXTERN int Encoding_get_bool_state_vars_num ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr 
Encoding_get_input_vars_list ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr 
Encoding_get_model_input_vars_list ARGS((const Encoding_ptr self));

EXTERN int Encoding_get_input_vars_num ARGS((const Encoding_ptr self));

EXTERN int Encoding_get_model_input_vars_num ARGS((const Encoding_ptr self));

EXTERN int Encoding_get_bool_input_vars_num ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr 
Encoding_get_all_model_vars_list ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr 
Encoding_get_all_model_symbols_list ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr  
Encoding_get_defines_list ARGS((const Encoding_ptr self));

EXTERN int Encoding_get_defines_num ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr 
Encoding_get_bool_vars_list ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr 
Encoding_get_bool_input_vars_list ARGS((const Encoding_ptr self));

EXTERN NodeList_ptr 
Encoding_get_bool_state_vars_list ARGS((const Encoding_ptr self));

EXTERN GroupSet_ptr 
Encoding_get_bool_input_vars_groups ARGS((const Encoding_ptr self));

EXTERN GroupSet_ptr 
Encoding_get_bool_state_vars_groups ARGS((const Encoding_ptr self));


/* operations: */
EXTERN void 
Encoding_sort_bool_vars ARGS((Encoding_ptr self, 
			      const char* input_order_file));


EXTERN void Encoding_push_status_and_reset ARGS((Encoding_ptr self));

EXTERN void Encoding_pop_status ARGS((Encoding_ptr self));


/* variables information: */
EXTERN node_ptr 
Encoding_get_var_encoding ARGS((const Encoding_ptr self, node_ptr name));

EXTERN NodeList_ptr 
Encoding_get_var_encoding_bool_vars ARGS((const Encoding_ptr self, 
					  node_ptr name));

EXTERN node_ptr 
Encoding_get_var_range ARGS((const Encoding_ptr self, node_ptr name));

EXTERN boolean 
Encoding_is_var_bit ARGS((const Encoding_ptr self, node_ptr name));

EXTERN node_ptr
Encoding_get_scalar_var_of_bit ARGS((const Encoding_ptr self, node_ptr name));


/* defines information */
EXTERN node_ptr 
Encoding_get_define_body ARGS((const Encoding_ptr self, node_ptr name));

EXTERN node_ptr 
Encoding_get_define_flatten_body ARGS((const Encoding_ptr self, 
				       node_ptr name));

EXTERN node_ptr 
Encoding_get_define_context ARGS((const Encoding_ptr self, node_ptr name));

/* symbols declarations */
EXTERN void 
Encoding_declare_constant ARGS((Encoding_ptr self, node_ptr name));

EXTERN void 
Encoding_declare_determ_var ARGS((Encoding_ptr self, node_ptr var));

EXTERN void 
Encoding_declare_input_var ARGS((Encoding_ptr self, node_ptr var, 
				 node_ptr range));

EXTERN void 
Encoding_declare_state_var ARGS((Encoding_ptr self, node_ptr var, 
				 node_ptr range));

EXTERN void 
Encoding_declare_define ARGS((Encoding_ptr self, node_ptr name, 
			      node_ptr ctx, node_ptr definition));

/* symbols information retrieving */
EXTERN node_ptr 
Encoding_lookup_symbol ARGS((const Encoding_ptr self, node_ptr name));

EXTERN boolean 
Encoding_is_symbol_declared ARGS((const Encoding_ptr self, node_ptr name));

EXTERN boolean 
Encoding_is_symbol_constant ARGS((const Encoding_ptr self, node_ptr name));

EXTERN boolean 
Encoding_is_symbol_var ARGS((const Encoding_ptr self, node_ptr name));

EXTERN boolean 
Encoding_is_symbol_determ_var ARGS((const Encoding_ptr self, node_ptr name));

EXTERN boolean 
Encoding_is_symbol_state_var ARGS((const Encoding_ptr self, node_ptr name));

EXTERN boolean 
Encoding_is_symbol_input_var ARGS((const Encoding_ptr self, node_ptr name));

EXTERN boolean 
Encoding_is_symbol_model_input_var ARGS((const Encoding_ptr self, 
					 node_ptr name));

EXTERN boolean 
Encoding_is_symbol_boolean_var ARGS((const Encoding_ptr self, node_ptr name));

EXTERN boolean 
Encoding_is_symbol_define ARGS((const Encoding_ptr self, node_ptr name));

EXTERN boolean 
Encoding_is_constant_defined ARGS((const Encoding_ptr self, node_ptr constant));

EXTERN boolean 
Encoding_list_contains_input_vars ARGS((Encoding_ptr self, 
					NodeList_ptr var_list));

EXTERN boolean 
Encoding_list_contains_state_vars ARGS((Encoding_ptr self, 
					NodeList_ptr var_list));


#endif /* __ENCODING_H__ */
