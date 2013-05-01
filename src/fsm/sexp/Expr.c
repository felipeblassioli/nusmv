/**CFile***********************************************************************

  FileName    [Expr.c]

  PackageName [fsm.sexp]

  Synopsis    [Abstraction for expression type impemented as node_ptr]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.sexp'' package of NuSMV version 2. 
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


#include "Expr.h"
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: Expr.c,v 1.1.2.5 2004/05/07 13:04:48 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define NODE_PTR(x) \
      ((node_ptr) x)


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

Expr_ptr Expr_true()
{
  return EXPR( find_node(TRUEEXP, Nil, Nil) );
}

Expr_ptr Expr_false() 
{
  return EXPR( find_node(FALSEEXP, Nil, Nil) );
}

boolean Expr_is_true(const Expr_ptr expr)
{
  return (expr == Expr_true());
}

boolean Expr_is_false(const Expr_ptr expr)
{
  return (expr == Expr_false());
}

Expr_ptr Expr_and(const Expr_ptr a, const Expr_ptr b)
{
  Expr_ptr result; 

  if (Expr_is_true(a))        result = b;
  else if (Expr_is_true(b))   result = a;
  else if (Expr_is_false(a))  result = a;
  else if (Expr_is_false(b))  result = b;
  else result = EXPR( find_node(AND, NODE_PTR(a), NODE_PTR(b)) );

  return result;
}

Expr_ptr Expr_not(const Expr_ptr expr)
{
  Expr_ptr result; 
  
  result = EXPR( find_node(NOT, NODE_PTR(expr), Nil) );

  return result;
}

Expr_ptr Expr_or(const Expr_ptr a, const Expr_ptr b)
{
  Expr_ptr result; 

  if (Expr_is_true(a))        result = a;
  else if (Expr_is_true(b))   result = b;
  else if (Expr_is_false(a))  result = b;
  else if (Expr_is_false(b))  result = a;
  else result = EXPR( find_node(OR, NODE_PTR(a), NODE_PTR(b)) );

  return result;
}


Expr_ptr Expr_ite(const Expr_ptr cond, const Expr_ptr t, const Expr_ptr e)
{
  Expr_ptr result; 
  node_ptr tmp;
  
  tmp = find_node(COLON, NODE_PTR(cond), NODE_PTR(t));
  result = EXPR( find_node(CASE, tmp, NODE_PTR(e)) );

  return result;
}


Expr_ptr Expr_next(const Expr_ptr a)
{
  Expr_ptr result; 

  result = EXPR( find_node(NEXT, NODE_PTR(a), Nil)  );

  return result;
}
