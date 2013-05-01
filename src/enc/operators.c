/**CFile***********************************************************************

  FileName    [operators.c]

  PackageName [enc]

  Synopsis    [These operators are used by dd package]

  Description [Functions like add_plus, add_equal, etc., call these operators]

  SeeAlso     [operators.h]

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

#include "operators.h"
#include "parser/symbols.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: operators.c,v 1.1.2.5 2004/06/07 16:36:33 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef int (*INTPFII)(int, int);


/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int plus_op ARGS((int a, int b));
static int minus_op ARGS((int a, int b));
static int times_op ARGS((int a, int b));
static int divide_op ARGS((int a, int b));
static int mod_op ARGS((int a, int b));
static int lt_op ARGS((int a, int b));
static int gt_op ARGS((int a, int b));
static node_ptr numeric_op ARGS((INTPFII op, node_ptr n1, node_ptr n2));



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Adds two integer nodes.]

  Description        [Adds two integer nodes.]

  SideEffects        []

******************************************************************************/
node_ptr node_plus(node_ptr n1, node_ptr n2)
{ return(numeric_op(plus_op,n1,n2)); } /* No because used in interactive loop. */

/**Function********************************************************************

  Synopsis           [Subtracts two integer nodes.]

  Description        [Subtracts two integer nodes.]

  SideEffects        []

******************************************************************************/
node_ptr node_minus(node_ptr n1, node_ptr n2)
{ return(numeric_op(minus_op,n1,n2)); }

/**Function********************************************************************

  Synopsis           [Multiplies two integer nodes.]

  Description        [Multiplies two integer nodes.]

  SideEffects        []

******************************************************************************/
node_ptr node_times(node_ptr n1, node_ptr n2)
{ return(numeric_op(times_op,n1,n2)); }

/**Function********************************************************************

  Synopsis           [Divides two integer nodes.]

  Description        [Divides two integer nodes.]

  SideEffects        []

******************************************************************************/
node_ptr node_divide(node_ptr n1, node_ptr n2)
{ return(numeric_op(divide_op,n1,n2)); }

/**Function********************************************************************

  Synopsis           [Computes the modulo of the division between two
  integer nodes.]

  Description        [Computes the modulo of the division between two
  integer nodes.]

  SideEffects        []

******************************************************************************/
node_ptr node_mod(node_ptr n1, node_ptr n2)
{ return(numeric_op(mod_op,n1,n2)); }

/**Function********************************************************************

  Synopsis           [Checks if an integer node is less then the other.]

  Description        [Checks if an integer node is less then the other.]

  SideEffects        []

******************************************************************************/
node_ptr node_lt(node_ptr n1, node_ptr n2)
{ return(numeric_op(lt_op,n1,n2)); }

/**Function********************************************************************

  Synopsis           [Checks if an integer node is greater then the other.]

  Description        [Checks if an integer node is greater then the other.]

  SideEffects        []

******************************************************************************/
node_ptr node_gt(node_ptr n1, node_ptr n2)
{ return(numeric_op(gt_op,n1,n2)); }

/**Function********************************************************************

  Synopsis           [Computes the set union of two s_expr.]

  Description        [This function computes the sexp resulting from
  the union of s_expr "n1" and "n2".]

  SideEffects        []

******************************************************************************/
node_ptr node_union(node_ptr n1, node_ptr n2)
{
  if(n1 != Nil && node_get_type(n1) != CONS) n1 = find_node(CONS, n1, Nil);
  if(n2 != Nil && node_get_type(n2) != CONS) n2 = find_node(CONS, n2, Nil);
  if(n1 == Nil) return(n2);
  if(n2 == Nil) return(n1);
  if(car(n1) == car(n2))
    return(find_node(CONS, car(n1), node_union(cdr(n1), cdr(n2))));
  if(((int)car(n1)) < ((int)car(n2)))
    return(find_node(CONS, car(n1), node_union(cdr(n1), n2)));
  return(find_node(CONS, car(n2), node_union(n1, cdr(n2))));
}

/**Function********************************************************************

  Synopsis           [Set inclusion]

  Description        [Checks if s_expr "n1" is a subset of s_expr
  "n2", if it is the case them <code>one_number</code> is returned,
  else <code>zero_number</code> is returned.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr node_setin(node_ptr n1, node_ptr n2)
{
  if(n2 == Nil) return(zero_number);
  if(node_get_type(n2) != CONS){
    if(n1 == n2) return(one_number);
    return(zero_number);
  }
  if(n1 == car(n2)) return(one_number);
  return(node_setin(n1,cdr(n2)));
}

/**Function********************************************************************

  Synopsis           [Checks for node equality.]

  Description        [This function checks if symbols <code>n1</code> and
  <code>n2</code> are equal, if it is then it returns the symbol
  <code>1</code> (that's <code>one_number</code>) else the symbol <code>0</code>
  (that's <code>zero_number</code>).]

  SideEffects        []

  SeeAlso            [node_setin]

******************************************************************************/
node_ptr node_equal(node_ptr n1, node_ptr n2)
{
  if(n1 == n2) return(one_number);
  return(zero_number);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Adds two integers.]

  Description        [Adds two integers]

  SideEffects        []

******************************************************************************/
static int plus_op(int a, int b) { return (a+b); }

/**Function********************************************************************

  Synopsis           [Subtracts two integers]

  Description        [Subtracts two integers]

  SideEffects        []

******************************************************************************/
static int minus_op(int a, int b) { return(a-b); }

/**Function********************************************************************

  Synopsis           [Multiplies two integers]

  Description        [Multiplies two integers]

  SideEffects        []

******************************************************************************/
static int times_op(int a, int b) { return(a*b); }

/**Function********************************************************************

  Synopsis           [Divide two integers]

  Description        [Divide two integers, if a division by zero is
  performed, then an error occurs.]

  SideEffects        [required]

******************************************************************************/
static int divide_op(int a, int b)
{
  int r;
  if ( b == 0 ) { division_by_zero(); }
  r = a % b;
  if (r<0) return(a/b-1);
  return(a/b);
}

/**Function********************************************************************

  Synopsis           [Computes the modulo of the division of two integers]

  Description        [Computes the modulo of the division of two
  integers, if a division by zero is performed, then an error occurs.]

  SideEffects        []

******************************************************************************/
static int mod_op(int a, int b)
{
  int r;
  if ( b == 0 ) { division_by_zero(); }
  r = a % b;
  if (r<0) r += b;
  return(r);
}

/**Function********************************************************************

  Synopsis           [Checks if an integer is less then the other.]

  Description        [Checks if an integer is less then the other.]

  SideEffects        []

******************************************************************************/
static int lt_op(int a, int b)
{
  if (a < b) return(1);
  return(0);
}

/**Function********************************************************************

  Synopsis           [Checks if an integer is greater then the other.]

  Description        [Checks if an integer is greater then the other.]

  SideEffects        []

******************************************************************************/
static int gt_op(int a, int b)
{
  if (a > b) return(1);
  return(0);
}

/**Function********************************************************************

  Synopsis           [Applies generic function to two operands.]

  Description        [Applies generic function to two operands. The
  two operands have to be integers.]

  SideEffects        []

******************************************************************************/
static node_ptr numeric_op(INTPFII op, node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) != NUMBER) { error_not_a_number(n1); }
  if (node_get_type(n2) != NUMBER) { error_not_a_number(n2); }
  return find_node(NUMBER, (node_ptr) (*op)((int) car(n1), 
					    (int) car(n2)), Nil);
}

