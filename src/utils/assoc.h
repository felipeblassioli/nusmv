/**CHeaderFile*****************************************************************

  FileName    [assoc.h]

  PackageName [util]

  Synopsis    [Simple assscoiative list]

  Description [Provides the user with a data structure that
  implemnts an associative list. If there is already an entry with
  the same ky in the table, than the value associated is replaced with
  the new one.]

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
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

  Revision    [$Id: assoc.h,v 1.5.6.4 2003/12/19 10:58:36 nusmv Exp $]

******************************************************************************/

#ifndef _ASSOC_H
#define _ASSOC_H

#include "util.h" /* for ARGS and EXTERN */
#include "node/node.h" /* for node_ptr */
#include "st.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define ASSOC_DELETE ST_DELETE
#define ASSOC_CONTINUE ST_CONTINUE

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct st_table * hash_ptr;
typedef enum st_retval assoc_retval;

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN hash_ptr new_assoc ARGS((void));
EXTERN void free_assoc ARGS((hash_ptr hash));
EXTERN hash_ptr copy_assoc ARGS((hash_ptr hash));
EXTERN node_ptr find_assoc ARGS((hash_ptr, node_ptr));
EXTERN void insert_assoc ARGS((hash_ptr, node_ptr, node_ptr));
EXTERN void clear_assoc_and_free_entries ARGS((hash_ptr, ST_PFSR));
EXTERN void clear_assoc ARGS((hash_ptr hash));

#endif /* _ASSOC_H */
