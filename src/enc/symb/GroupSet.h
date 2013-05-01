/**CHeaderFile*****************************************************************

  FileName    [GroupSet.h]

  PackageName [enc.symb]

  Synopsis    [Represents a set of groups of variables to be kept grouped]

  Description []
                                               
  SeeAlso     [GroupSet.c]

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

#ifndef __ENC_SYMB_GROUP_SET_H__
#define __ENC_SYMB_GROUP_SET_H__


#include "utils/utils.h"
#include "utils/NodeList.h"

typedef struct GroupSet_TAG*  GroupSet_ptr;

#define GROUP_SET(x)  \
        ((GroupSet_ptr) (x))

#define GROUP_SET_CHECK_INSTANCE(x)  \
        (nusmv_assert(GROUP_SET(x) != GROUP_SET(NULL)))



EXTERN GroupSet_ptr GroupSet_create ARGS(());
EXTERN void GroupSet_destroy ARGS((GroupSet_ptr self));

EXTERN void GroupSet_add_group ARGS((GroupSet_ptr self, NodeList_ptr group));

EXTERN ListIter_ptr 
GroupSet_get_first_iter ARGS((const GroupSet_ptr self));

EXTERN ListIter_ptr 
GroupSet_get_next_iter ARGS((const GroupSet_ptr self, 
			     const ListIter_ptr iter));

EXTERN NodeList_ptr 
GroupSet_get_group ARGS((const GroupSet_ptr self, const ListIter_ptr iter));



#endif /* __ENC_SYMB_GROUP_SET_H__ */
