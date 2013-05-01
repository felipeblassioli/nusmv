/**CHeaderFile*****************************************************************

  FileName    [pslNode.h]

  PackageName [parser.psl]

  Synopsis    [PslNode interface]

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

  Revision    [$Id: pslNode.h,v 1.1.2.5 2005/11/16 11:44:40 nusmv Exp $]

******************************************************************************/

#ifndef __PSL_NODE_H__
#define __PSL_NODE_H__

#include "node/node.h"
#include "utils/utils.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef node_ptr PslNode_ptr;

typedef short int PslOp;

typedef enum PslOpConvType_TAG {
  PSL2SMV, 
  PSL2PSL, 
  SMV2PSL /* this is supported only for IDs */
} PslOpConvType;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro**********************************************************************
  Synopsis     [This value represents a null PslNode]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define PSL_NULL ((PslNode_ptr) NULL)


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN PslNode_ptr psl_node_get_left ARGS((PslNode_ptr n));
EXTERN PslNode_ptr psl_node_get_right ARGS((PslNode_ptr n));
EXTERN PslNode_ptr PslNode_convert_from_node_ptr ARGS((node_ptr expr));
EXTERN node_ptr PslNode_convert_to_node_ptr ARGS((PslNode_ptr expr));

EXTERN PslNode_ptr PslNode_new_context ARGS((PslNode_ptr ctx, PslNode_ptr node));

/* Predicates */
EXTERN boolean PslNode_is_handled_psl ARGS((PslNode_ptr e));
EXTERN boolean PslNode_is_propositional ARGS((const PslNode_ptr expr));
EXTERN boolean PslNode_is_obe ARGS((const PslNode_ptr expr));
EXTERN boolean PslNode_is_ltl ARGS((const PslNode_ptr expr));

/* convert */
PslNode_ptr PslNode_convert_id ARGS((PslNode_ptr id, PslOpConvType type));
PslNode_ptr PslNode_pslobe2ctl ARGS((PslNode_ptr expr, PslOpConvType type)); 
PslNode_ptr PslNode_pslltl2ltl ARGS((PslNode_ptr expr, PslOpConvType type));
PslNode_ptr PslNode_remove_sere ARGS((PslNode_ptr e));
PslNode_ptr PslNode_remove_forall_replicators ARGS((PslNode_ptr e));


/* Printing */
EXTERN int PslNode_print ARGS((FILE* f, PslNode_ptr expr));


#endif /* __PSL_NODE_H__ */
