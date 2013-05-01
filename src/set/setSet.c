/**CFile***********************************************************************

  FileName    [setSet.c]

  PackageName [set]

  Synopsis    [Generic Set Data Structure]

  Description [This package provides an implementation of sets.
  It is possible to perform the test of equality among two sets in
  constant time by simply comparing the two sets. Thus it is possible
  to check if a union has increased the cardinality of a set inserting
  elements in one of the two operands by simply comparing the
  result of the union among the operands.
  ]

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

******************************************************************************/

#include "setInt.h" 

static char UTIL_UNUSED rcsid[] = "$Id: setSet.c,v 1.9 2002/06/28 15:15:22 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


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
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Create a generic empty set]

  Description        [This function creates an empty set.]

  SideEffects        []

******************************************************************************/
Set_t Set_MakeEmpty () {
  return((Set_t)Nil);
}

/**Function********************************************************************

  Synopsis           [Set Emptiness]

  Description        [Checks for Set Emptiness.]

  SideEffects        []

******************************************************************************/
boolean Set_IsEmpty (Set_t set) {
  return(((Set_t)Nil == set) ? true : false);
}

/**Function********************************************************************

  Synopsis           [Set memberships]

  Description        [Checks if the given element is a meber of the
  set. It returns <tt>True</tt> if it is a member, <tt>False</tt>
  otherwise.]

  SideEffects        []

******************************************************************************/
boolean Set_IsMember(Set_t set, Set_Element_t el) {
  return( (memberp((node_ptr)el, (node_ptr)set)) ? true : false );
}

/**Function********************************************************************

  Synopsis           [Set Cardinality]

  Description        [Computes the cardinality of the given set]

  SideEffects        []

******************************************************************************/
int Set_GiveCardinality(Set_t set) {
  return(llength((node_ptr)set));
}

/**Function********************************************************************

  Synopsis           [Creates a Singleton]

  Description        [Creates a set with a unique element.]

  SideEffects        []

******************************************************************************/
Set_t Set_MakeSingleton(Set_Element_t el) 
{
  return((Set_t)find_node(CONS,(node_ptr)el, Nil));
}
  
/**Function********************************************************************

  Synopsis           [Set Union]

  Description        [Computes the Union of two sets]

  SideEffects        []

******************************************************************************/
Set_t Set_Union(Set_t set1, Set_t set2) {
  if (set1 == (Set_t)Nil) return(set2);
  if (set2 == (Set_t)Nil) return(set1);
  if (car((node_ptr)set1) == car((node_ptr)set2)) {
    return((Set_t)find_node(CONS, car((node_ptr)set1), Set_Union((Set_t)cdr(set1), (Set_t)cdr(set2))));
  }
  else if ((int)car((node_ptr)set1) < (int)car((node_ptr)set2)) {
   return((Set_t)find_node(CONS, car((node_ptr)set1), Set_Union((Set_t)cdr(set1), set2)));
  }
  else {
    return((Set_t)find_node(CONS, car((node_ptr)set2), Set_Union((Set_t)cdr(set2), set1)));
  }
}

/**Function********************************************************************

  Synopsis           [Set Difference]

  Description        [Computes the Set Difference]

  SideEffects        []

******************************************************************************/
Set_t Set_Difference(Set_t set1, Set_t set2) {
  if ((set1 == (Set_t)Nil) || (set2 == (Set_t)Nil)) return(set1);
  if (car((node_ptr)set1) == car((node_ptr)set2)) {
    return(Set_Difference((Set_t)cdr(set1), (Set_t)cdr(set2)));
  }
  else if ((int)car((node_ptr)set1) < (int)car((node_ptr)set2)) {
    return((Set_t)find_node(CONS, car((node_ptr)set1), Set_Difference((Set_t)cdr(set1), set2)));
  }
  else {
    return(Set_Difference(set1, (Set_t)cdr(set2)));
  }
}

/**Function********************************************************************

  Synopsis           [Gets first element]

  Description        [Gets the first element of a set.]

  SideEffects        []

******************************************************************************/
Set_Element_t Set_GetFirst(Set_t set1) {
  nusmv_assert(set1 != (Set_t)Nil);
  return((Set_Element_t)car(set1));
}

/**Function********************************************************************

  Synopsis           [Gets the set out of the first element]

  Description        [Gets the set difference among the set and the
  singleton corresponding to the first element of the set.]

  SideEffects        []

******************************************************************************/
Set_t Set_GetRest(Set_t set1) {
  nusmv_assert(set1 != (Set_t)Nil);
  return((Set_Element_t)cdr(set1));
}

/**Function********************************************************************

  Synopsis           [Frees a set]

  Description        [Releases the memory associated to the given set]

  SideEffects        []

******************************************************************************/
void Set_ReleaseSet(Set_t set) {
  node_ptr l = (node_ptr)set;

  while (l != Nil) {
    node_ptr m = l;

    l = cdr(l);
    free_node(m);
  }
}

/**Function********************************************************************

  Synopsis           [Prints a set]

  Description        [Prints a set to the specified file stream]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Set_PrintSet(FILE * file, Set_t s)
{
  fprintf(file, "{");
  while (Set_IsEmpty(s) == false) {
    print_node(file, (node_ptr)Set_GetFirst(s));
    s = Set_GetRest(s);
    if (Set_IsEmpty(s) == false) fprintf(file, ", ");
  }
  fprintf(file, "}");
}

/**Function********************************************************************

  Synopsis           [Given a list, builds a corresponding set]

  Description        [Given a list, builds a corresponding set]

  SideEffects        []

  SeeAlso            [Set_MakeSingleton]

******************************************************************************/
Set_t Set_Make(node_ptr l)
{
  Set_t result = Set_MakeEmpty();

  for(; l != Nil; l = cdr(l)){
    Set_t s = Set_MakeSingleton((Set_Element_t)car(l));

    result = Set_Union(result, s);
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Given a set, builds a corresponding list]

  Description        [Given a set, builds a corresponding list]

  SideEffects        []

  SeeAlso            [Set_MakeSingleton]

******************************************************************************/
node_ptr Set_Set2List(Set_t l)
{
  node_ptr result = (node_ptr)l;

  return(result);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



