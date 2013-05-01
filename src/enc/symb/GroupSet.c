/**CFile*****************************************************************

  FileName    [GroupSet.c]

  PackageName [enc.symb]

  Synopsis    [Represents a list of group of variables. Each group 
  is expected to be kept grouped]

  Description [When the order of bool vars is important, a list of 
  of group of vars is returned. All vars appearing into a group should be 
  grouped by the specific encoding]

  SeeAlso     []

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

#include "GroupSet.h"
#include "utils/NodeList.h"


static char rcsid[] UTIL_UNUSED = "$Id: GroupSet.c,v 1.1.2.2.2.1 2005/07/14 15:58:26 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Group Set]

  Description [Set of groups of vars]
  
******************************************************************************/
typedef struct GroupSet_TAG 
{
  NodeList_ptr groups;
} GroupSet;


/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void group_set_init ARGS((GroupSet_ptr self));
static void group_set_deinit ARGS((GroupSet_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Constructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
GroupSet_ptr GroupSet_create()
{
  GroupSet_ptr self = ALLOC(GroupSet, 1);

  GROUP_SET_CHECK_INSTANCE(self);

  group_set_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void GroupSet_destroy(GroupSet_ptr self)
{
  GROUP_SET_CHECK_INSTANCE(self);

  group_set_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis [Adds a new group to the set. Addition is performed only
  if the group does not belong to the set]

  Description        [The given group becomes owned by self, user loses 
  ownership of it]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void GroupSet_add_group(GroupSet_ptr self, NodeList_ptr group)
{
  GROUP_SET_CHECK_INSTANCE(self);

  if (! NodeList_belongs_to(self->groups, (node_ptr) group)) {
    NodeList_append(self->groups, (node_ptr) group);
  }
}


/**Function********************************************************************

  Synopsis           [To be used for iteration]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ListIter_ptr GroupSet_get_first_iter(const GroupSet_ptr self)
{
  GROUP_SET_CHECK_INSTANCE(self);

  return NodeList_get_first_iter(self->groups);
}


/**Function********************************************************************

  Synopsis           [Return the group pointed by the iterator]

  Description        [Returned NodeList is still owned by self, user 
  have not to destroy it]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr GroupSet_get_group(const GroupSet_ptr self, 
				const ListIter_ptr iter)
{
  GROUP_SET_CHECK_INSTANCE(self);
  
  return NODE_LIST(NodeList_get_elem_at(self->groups, iter));
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void group_set_init(GroupSet_ptr self)
{
  self->groups = NodeList_create();
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void group_set_deinit(GroupSet_ptr self)
{
  ListIter_ptr iter = GroupSet_get_first_iter(self);
  while (! ListIter_is_end(iter)) {
    NodeList_ptr group = GroupSet_get_group(self, iter);
    NodeList_destroy(group);

    iter = ListIter_get_next(iter);
  }

  NodeList_destroy(self->groups);
}
