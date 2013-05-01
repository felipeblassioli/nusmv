/**CHeaderFile*****************************************************************

  FileName    [nodeInt.h]

  PackageName [node]

  Synopsis    [The internal header of the <tt>node</tt> package.]

  Description [None]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``node'' package of NuSMV version 2. 
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

  Revision    [$Id: nodeInt.h,v 1.3.4.2.2.1 2005/11/16 12:09:46 nusmv Exp $]

******************************************************************************/

#ifndef _node_int_h
#define _node_int_h

#include <stdio.h>

#include "utils/utils.h"
#include "util.h"
#include "node/node.h"
#include "utils/error.h"
#include "parser/symbols.h"
#include "utils/assoc.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define NODE_MEM_CHUNK 1022
#define NODE_HASH_SIZE 16381

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct node_mgr_ node_mgr_;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [The data structure of the <tt>node</tt> manager.]

  Description [The <tt>node</tt> manager. It provides memory
  management, and hashing.]

  SeeAlso     [DdManager]

******************************************************************************/
struct node_mgr_ {
  long allocated;            /* Number of nodes allocated till now */
  long memused;              /* Total memory allocated by the node manager */
  node_ptr * nodelist;       /* The node hash table */
  node_ptr * memoryList;     /* Memory manager for symbol table */
  node_ptr nextFree;         /* List of free nodes */
  hash_ptr subst_hash;       /* The substitution hash */
};

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN node_ptr insert_node ARGS((node_ptr node));

#endif /* _node_int_h */
