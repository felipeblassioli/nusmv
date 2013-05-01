/**CHeaderFile*****************************************************************

  FileName    [BddEnc.h]

  PackageName [enc.bdd]

  Synopsis    [The Bdd encoding public interface]

  Description []
                                               
  SeeAlso     [BddEnc.c]

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

#ifndef __ENC_BDD_BDD_ENC_H__
#define __ENC_BDD_BDD_ENC_H__

#include "bdd.h"
#include "enc/symb/Encoding.h"
#include "utils/utils.h"
#include "utils/NodeList.h"
#include "node/node.h"
#include "dd/dd.h"
#include "fsm/sexp/Expr.h"
#include "fsm/bdd/bdd.h"


/**Type***********************************************************************

  Synopsis     [The BddEnc type ]

  Description  [The BddEnc type ]  

  Notes        []

******************************************************************************/
typedef struct BddEnc_TAG*  BddEnc_ptr;
#define BDD_ENC(x) \
          ((BddEnc_ptr) x)

#define BDD_ENC_CHECK_INSTANCE(x) \
          ( nusmv_assert(BDD_ENC(x) != BDD_ENC(NULL)) )




/**Type***********************************************************************

  Synopsis     [Used when dumping ordering file]

  Description  [Used when dumping ordering file]  

  Notes        [see method write_order]

******************************************************************************/
typedef enum {
  DUMP_DEFAULT, 
  DUMP_BITS, 
  DUMP_SCALARS_ONLY
} VarOrderingType;


/* ---------------------------------------------------------------------- */
/* Public methods                                                         */
/* ---------------------------------------------------------------------- */

/* Creation, destruction, status saving/restoring */
EXTERN BddEnc_ptr 
BddEnc_create ARGS((Encoding_ptr generic_encoding, DdManager* dd));

EXTERN void BddEnc_destroy ARGS((BddEnc_ptr self));

EXTERN void BddEnc_push_status_and_reset ARGS((BddEnc_ptr self));

EXTERN void BddEnc_pop_status ARGS((BddEnc_ptr self));

EXTERN void BddEnc_merge ARGS((BddEnc_ptr self));


/* Getters */
EXTERN DdManager* BddEnc_get_dd_manager ARGS((const BddEnc_ptr self));

EXTERN Encoding_ptr 
BddEnc_get_symbolic_encoding ARGS((const BddEnc_ptr self));

EXTERN node_ptr BddEnc_get_state_vars_add_list ARGS((BddEnc_ptr self));

EXTERN BddVarSet_ptr 
BddEnc_get_state_vars_support ARGS((const BddEnc_ptr self));

EXTERN BddVarSet_ptr 
BddEnc_get_next_state_vars_support ARGS((const BddEnc_ptr self));

EXTERN BddVarSet_ptr 
BddEnc_get_input_vars_support ARGS((const BddEnc_ptr self));

EXTERN node_ptr 
BddEnc_get_var_name_from_dd_index ARGS((const BddEnc_ptr self, int index));

EXTERN int 
BddEnc_get_var_index_from_name ARGS((const BddEnc_ptr self, node_ptr name));

EXTERN NodeList_ptr BddEnc_get_ordering ARGS((const BddEnc_ptr self));


/* Converters */
EXTERN add_ptr 
BddEnc_expr_to_add ARGS((BddEnc_ptr self, const Expr_ptr v, 
			 const node_ptr context));

EXTERN bdd_ptr 
BddEnc_expr_to_bdd ARGS((BddEnc_ptr self, const Expr_ptr expr, 
			 const node_ptr context));

EXTERN add_ptr 
BddEnc_state_var_to_next_state_var_add ARGS((const BddEnc_ptr self, 
					     add_ptr add));

EXTERN add_ptr 
BddEnc_next_state_var_to_state_var_add ARGS((const BddEnc_ptr self, 
					     add_ptr add));

EXTERN bdd_ptr 
BddEnc_state_var_to_next_state_var ARGS((const BddEnc_ptr self, 
					 bdd_ptr bdd));

EXTERN bdd_ptr 
BddEnc_next_state_var_to_state_var ARGS((const BddEnc_ptr self, 
					 bdd_ptr bdd));

EXTERN add_ptr 
BddEnc_constant_to_add ARGS((const BddEnc_ptr self, node_ptr constant));



/* Masks high level */
EXTERN add_ptr 
BddEnc_apply_state_vars_mask_add ARGS((BddEnc_ptr self, add_ptr states));

EXTERN add_ptr 
BddEnc_apply_input_vars_mask_add ARGS((BddEnc_ptr self, add_ptr inputs));

EXTERN add_ptr 
BddEnc_apply_state_input_vars_mask_add ARGS((BddEnc_ptr self, 
					     add_ptr states_inputs));

EXTERN BddStates 
BddEnc_apply_state_vars_mask_bdd ARGS((BddEnc_ptr self, BddStates states));

EXTERN BddInputs 
BddEnc_apply_input_vars_mask_bdd ARGS((BddEnc_ptr self, BddInputs inputs));

EXTERN BddStatesInputs 
BddEnc_apply_state_input_vars_mask_bdd ARGS((BddEnc_ptr self,
					     BddStatesInputs states_inputs));

/* masks getters: */
EXTERN add_ptr 
BddEnc_get_state_vars_mask_add ARGS((BddEnc_ptr self));

EXTERN add_ptr 
BddEnc_get_input_vars_mask_add ARGS((BddEnc_ptr self));

EXTERN add_ptr 
BddEnc_get_state_input_vars_mask_add ARGS((BddEnc_ptr self)); 

EXTERN bdd_ptr
BddEnc_get_state_vars_mask_bdd ARGS((BddEnc_ptr self));

EXTERN bdd_ptr
BddEnc_get_input_vars_mask_bdd ARGS((BddEnc_ptr self));

EXTERN bdd_ptr
BddEnc_get_state_input_vars_mask_bdd ARGS((BddEnc_ptr self));


/* Masks low level: */
EXTERN add_ptr 
BddEnc_get_var_name_mask ARGS((BddEnc_ptr self, node_ptr var));

EXTERN add_ptr 
BddEnc_get_var_encoding_mask ARGS((BddEnc_ptr self, add_ptr var_encoding));



/* Counting, minterms, picking */
EXTERN double 
BddEnc_count_states_of_add ARGS((const BddEnc_ptr self, add_ptr add));

EXTERN double 
BddEnc_count_states_of_bdd ARGS((const BddEnc_ptr self, bdd_ptr bdd));

EXTERN double 
BddEnc_count_inputs_of_bdd ARGS((const BddEnc_ptr self, bdd_ptr bdd));

EXTERN double 
BddEnc_count_states_inputs_of_bdd ARGS((const BddEnc_ptr self, bdd_ptr bdd));

EXTERN double 
BddEnc_get_minterms_of_add ARGS((const BddEnc_ptr self, add_ptr add)); 

EXTERN double
BddEnc_get_minterms_of_bdd ARGS((const BddEnc_ptr self, bdd_ptr bdd));

EXTERN bdd_ptr 
BddEnc_pick_one_state ARGS((const BddEnc_ptr self, bdd_ptr bdd));

EXTERN bdd_ptr
BddEnc_pick_one_input ARGS((const BddEnc_ptr self, bdd_ptr inputs));

EXTERN bdd_ptr 
BddEnc_pick_one_state_rand ARGS((const BddEnc_ptr self, bdd_ptr bdd));

EXTERN bdd_ptr
BddEnc_pick_one_input_rand ARGS((const BddEnc_ptr self, bdd_ptr inputs));

EXTERN boolean 
BddEnc_pick_all_terms_states_inputs ARGS((const BddEnc_ptr self, 
					  bdd_ptr bdd, 
					  bdd_ptr* result_array, 
					  const int array_len));

EXTERN boolean 
BddEnc_pick_all_terms_states ARGS((const BddEnc_ptr self, bdd_ptr bdd, 
				   bdd_ptr* result_array, 
				   const int array_len));

EXTERN boolean 
BddEnc_pick_all_terms_inputs ARGS((const BddEnc_ptr self, bdd_ptr bdd, 
				   bdd_ptr* result_array, 
				   const int array_len));


/* Evaluation */
EXTERN add_ptr 
BddEnc_eval_sign_add ARGS((BddEnc_ptr self, add_ptr a, int flag));

EXTERN bdd_ptr 
BddEnc_eval_sign_bdd ARGS((BddEnc_ptr self, bdd_ptr a, int flag));

EXTERN int 
BddEnc_eval_num ARGS((BddEnc_ptr self, node_ptr e, node_ptr context));

EXTERN add_ptr 
BddEnc_eval_constant ARGS((BddEnc_ptr self, Expr_ptr expr, node_ptr context));

EXTERN add_ptr 
BddEnc_get_symbol_add ARGS((BddEnc_ptr self, node_ptr name));


/* Printing */
EXTERN void BddEnc_print_bdd_begin ARGS((BddEnc_ptr self, node_ptr symbols, 
					 boolean changes_only));

EXTERN void BddEnc_print_bdd_end ARGS((BddEnc_ptr self));

EXTERN int BddEnc_print_bdd ARGS((BddEnc_ptr self, bdd_ptr bdd, FILE* file));

EXTERN void BddEnc_print_set_of_states ARGS((BddEnc_ptr self, 
					     bdd_ptr states,
					     boolean changes_only, 
					     FILE* file));

EXTERN void BddEnc_print_set_of_inputs ARGS((BddEnc_ptr self, 
					     bdd_ptr inputs,
					     boolean changes_only, 
					     FILE* file));

EXTERN void 
BddEnc_print_set_of_state_input_pairs ARGS((BddEnc_ptr self, 
					    bdd_ptr state_input_pairs,
					    boolean changes_only, 
					    FILE* file));



/* other features */
EXTERN void BddEnc_write_order ARGS((const BddEnc_ptr self, 
				     const char *output_order_file_name, 
				     const VarOrderingType dump_type));


#endif /* __ENC_BDD_BDD_ENC_H__ */
