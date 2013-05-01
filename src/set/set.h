/**CHeaderFile*****************************************************************

  FileName    [set.h]

  PackageName [set]

  Synopsis    [Generic Set Data Structure]

  Description [This package provides an implementation of sets.
  It is possible to perform the test of equality among two sets in
  constant time by simply comparing the two sets. Thus it is possible
  to check if a union has increased the cardinality of a set inserting
  elements in one of the two operands by simply comparing the
  result of the union among the operands.]

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``set'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by ITC-irst. 

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

  Revision    [$Id: set.h,v 1.3.4.1.2.1 2005/11/16 12:09:47 nusmv Exp $]

******************************************************************************/

#ifndef _set
#define _set

#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef node_ptr Set_t;
typedef node_ptr Set_Element_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Set_t Set_MakeEmpty ARGS((void));
EXTERN boolean Set_IsEmpty ARGS((Set_t));
EXTERN boolean Set_IsMember ARGS((Set_t, Set_Element_t));
EXTERN int Set_GiveCardinality ARGS((Set_t));
EXTERN Set_t Set_MakeSingleton ARGS((Set_Element_t));
EXTERN Set_t Set_Union ARGS((Set_t, Set_t));
EXTERN Set_t Set_Difference ARGS((Set_t, Set_t));
EXTERN Set_Element_t Set_GetFirst ARGS((Set_t));
EXTERN Set_t Set_GetRest ARGS((Set_t));
EXTERN void Set_ReleaseSet ARGS((Set_t));
EXTERN void Set_PrintSet ARGS((FILE *, Set_t));
EXTERN Set_t Set_Make ARGS((node_ptr));
EXTERN node_ptr Set_Set2List ARGS((Set_t));

/**AutomaticEnd***************************************************************/

#endif /* _set */
