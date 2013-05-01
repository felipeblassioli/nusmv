/**CHeaderFile*****************************************************************

  FileName    [operators.h]

  PackageName [enc]

  Synopsis    [Interface for operators are used by dd package]

  Description [Functions like add_plus, add_equal, etc., call these operators]

  SeeAlso     [operators.c]

  Author      [Marco Roveri, Roberto Cavada]

  Copyright   [
  This file is part of the ``enc'' package of NuSMV version 2. 
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

#ifndef __ENC_OPERATORS_H__
#define __ENC_OPERATORS_H__

#include "utils/utils.h"
#include "node/node.h"


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN node_ptr one_number;
EXTERN node_ptr zero_number;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN node_ptr node_plus ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_minus ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_times ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_divide ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_mod ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_lt ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_gt ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_union ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_setin ARGS((node_ptr n1, node_ptr n2));
EXTERN node_ptr node_equal ARGS((node_ptr n1, node_ptr n2));


#endif /* __ENC_OPERATORS_H__ */
