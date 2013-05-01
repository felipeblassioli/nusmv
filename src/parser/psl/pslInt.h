/**CHeaderFile*****************************************************************

  FileName    [pslInt.h]

  PackageName [parser.psl]

  Synopsis    [Psl package private interface]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``parser.psl'' package of NuSMV version 2. 
  Copyright (C) 2005 by ITC-irst. 

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

  For more information on NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

  Revision    [$Id: pslInt.h,v 1.1.2.7 2005/11/16 11:44:40 nusmv Exp $]

******************************************************************************/

#ifndef __PSL_INT_H__
#define __PSL_INT_H__

#include "pslNode.h"
#include "utils/utils.h"
#include "utils/error.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro**********************************************************************
  Synopsis     [This integer value represents the count of a count-free 
  starred repeated sere. For example {a}\[*\] that has no count.]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define PSL_EMPTYSTAR PSL_NULL


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN PslNode_ptr 
psl_new_node ARGS((PslOp _type, PslNode_ptr left, PslNode_ptr right));

PslNode_ptr 
psl_conv_new_node ARGS((PslOpConvType conv, PslOp op, 
			PslNode_ptr left, PslNode_ptr right));

EXTERN PslNode_ptr psl_node_get_left ARGS((PslNode_ptr n)); 
EXTERN PslNode_ptr psl_node_get_right ARGS((PslNode_ptr n)); 
EXTERN void psl_node_set_left ARGS((PslNode_ptr n, PslNode_ptr l));
EXTERN void psl_node_set_right ARGS((PslNode_ptr n, PslNode_ptr r));

EXTERN PslNode_ptr psl_node_make_true ARGS((void));
EXTERN PslNode_ptr psl_node_make_false ARGS((void));

EXTERN PslNode_ptr psl_node_make_number ARGS((int value));

EXTERN PslOp psl_node_get_op ARGS((PslNode_ptr n));

EXTERN int psl_conv_op ARGS((int op, PslOpConvType type));

EXTERN boolean psl_node_is_sere ARGS((PslNode_ptr expr));

EXTERN boolean 
psl_node_is_handled_star ARGS((PslNode_ptr expr, boolean toplevel));

EXTERN PslNode_ptr 
psl_node_sere_star_get_count ARGS((const PslNode_ptr e));

EXTERN boolean psl_node_sere_is_propositional ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_repeated ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_star ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_standalone_star ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_plus ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_standalone_plus ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_star_count ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_repeated_get_expr ARGS((PslNode_ptr e));

EXTERN boolean psl_node_sere_is_stareq ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_starminusgt ARGS((PslNode_ptr e));

EXTERN PslNode_ptr 
psl_node_make_sere_compound ARGS((PslNode_ptr seq1, 
				  PslOp op, PslNode_ptr seq2));

EXTERN boolean psl_node_is_sere_compound_binary ARGS((PslNode_ptr e));

EXTERN boolean psl_node_is_suffix_implication ARGS((PslNode_ptr expr));
EXTERN boolean psl_node_is_suffix_implication_weak ARGS((PslNode_ptr expr));
EXTERN boolean psl_node_is_suffix_implication_strong ARGS((PslNode_ptr expr));

EXTERN PslNode_ptr 
psl_node_suffix_implication_get_premise ARGS((PslNode_ptr e));

EXTERN PslNode_ptr 
psl_node_suffix_implication_get_consequence ARGS((PslNode_ptr e));

EXTERN boolean psl_node_sere_is_concat_holes_free ARGS((PslNode_ptr e));

EXTERN boolean psl_node_sere_is_concat_fusion ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_concat_fusion_holes_free ARGS((PslNode_ptr e));

EXTERN PslNode_ptr psl_node_prune ARGS((PslNode_ptr tree, PslNode_ptr branch));

EXTERN boolean psl_node_is_propstar ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_2ampersand ARGS((PslNode_ptr e));

EXTERN PslNode_ptr 
psl_node_make_cons ARGS((PslNode_ptr elem, PslNode_ptr next));

EXTERN PslNode_ptr 
psl_node_make_cons_new ARGS((PslNode_ptr elem, PslNode_ptr next));

EXTERN boolean psl_node_is_cons ARGS((PslNode_ptr e)); 
EXTERN PslNode_ptr psl_node_cons_get_element ARGS((PslNode_ptr e)); 
EXTERN PslNode_ptr psl_node_cons_get_next ARGS((PslNode_ptr e)); 
EXTERN PslNode_ptr psl_node_cons_reverse ARGS((PslNode_ptr e));

EXTERN boolean psl_node_is_ite ARGS((PslNode_ptr _ite));
EXTERN PslNode_ptr psl_node_get_ite_cond ARGS((PslNode_ptr _ite));
EXTERN PslNode_ptr psl_node_get_ite_then ARGS((PslNode_ptr _ite));
EXTERN PslNode_ptr psl_node_get_ite_else ARGS((PslNode_ptr _ite));

EXTERN boolean psl_node_is_case ARGS((PslNode_ptr _case));
EXTERN PslNode_ptr psl_node_get_case_cond ARGS((PslNode_ptr _case));
EXTERN PslNode_ptr psl_node_get_case_then ARGS((PslNode_ptr _case));
EXTERN PslNode_ptr psl_node_get_case_next ARGS((PslNode_ptr _case));

EXTERN boolean psl_node_is_range ARGS((PslNode_ptr expr)); 
EXTERN PslNode_ptr psl_node_range_get_low ARGS((PslNode_ptr expr)); 
EXTERN PslNode_ptr psl_node_range_get_high ARGS((PslNode_ptr expr)); 

EXTERN boolean psl_node_is_number ARGS((PslNode_ptr e)); 
EXTERN int psl_node_number_get_value ARGS((PslNode_ptr e)); 

EXTERN boolean 
psl_node_is_num_equal ARGS((PslNode_ptr _id1, PslNode_ptr _id2));

EXTERN boolean psl_node_is_boolean_type ARGS((PslNode_ptr expr));
EXTERN boolean psl_node_is_infinite ARGS((PslNode_ptr expr));

EXTERN boolean psl_node_is_id ARGS((PslNode_ptr expr)); 
EXTERN boolean psl_node_is_id_equal ARGS((PslNode_ptr _id1, PslNode_ptr _id2));

EXTERN boolean psl_node_is_leaf ARGS((PslNode_ptr expr));

EXTERN boolean psl_node_is_repl_prop ARGS((PslNode_ptr _prop));
EXTERN PslNode_ptr psl_node_repl_prop_get_replicator ARGS((PslNode_ptr _prop));
EXTERN PslNode_ptr psl_node_repl_prop_get_property ARGS((PslNode_ptr _prop));

EXTERN boolean psl_node_is_replicator ARGS((PslNode_ptr _repl));
EXTERN PslNode_ptr psl_node_get_replicator_id ARGS((PslNode_ptr _repl));
EXTERN PslNode_ptr psl_node_get_replicator_range ARGS((PslNode_ptr _repl));
EXTERN PslNode_ptr psl_node_get_replicator_value_set ARGS((PslNode_ptr _repl));

EXTERN PslNode_ptr 
psl_node_make_extended_next ARGS((PslOp op, PslNode_ptr expr,
				  PslNode_ptr when, 
				  PslNode_ptr condition));

EXTERN boolean psl_node_is_extended_next ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_extended_next_get_expr ARGS((PslNode_ptr next)); 
EXTERN PslNode_ptr psl_node_extended_next_get_when ARGS((PslNode_ptr next)); 

EXTERN PslNode_ptr 
psl_node_extended_next_get_condition ARGS((PslNode_ptr next)); 


#endif /* __PSL_INT_H__ */
