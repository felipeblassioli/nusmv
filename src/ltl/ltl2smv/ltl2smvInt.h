/**CHeaderFile*****************************************************************

  FileName    [ltl2smvInt.h]

  PackageName [ltl2smv]

  Synopsis    [Internal header of the <tt>ltl2smv</tt> package.]

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``ltl2smv'' package of NuSMV version 2. 
  Copyright (C) 1998-2004 by CMU and ITC-irst. 

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

  Revision    [$Id: ltl2smvInt.h,v 1.1.2.1 2004/09/14 16:14:22 nusmv Exp $]

******************************************************************************/

#ifndef __LTL_2_SMV_INT_H__
#define __LTL_2_SMV_INT_H__

#include "ltl2smv.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct node_struct node;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN node* ltl2smv_Specf;     /* spec formula */
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void ltl2smv_inc_linenumber ARGS((void));
EXTERN int ltl2smv_get_linenumber ARGS((void));
EXTERN char* ltl2smv_mycpy ARGS((char* text));
EXTERN node* ltl2smv_gen_node ARGS((char op, node* left, node* right));
EXTERN node* ltl2smv_gen_leaf ARGS((char* str));
EXTERN char* ltl2smv_mystrconcat3 ARGS((char* s1, char* s2, char* s3));
EXTERN node* ltl2smv_mymalloc_node ARGS((void));


#endif /* __LTL_2_SMV_INT_H__ */
