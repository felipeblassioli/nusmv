/**CHeaderFile*****************************************************************

  FileName    [Expr.h]

  PackageName [fsm.sexp]

  Synopsis    [Interface for Expr type]

  Description []

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

  Revision    [$Id: Expr.h,v 1.1.2.2 2004/05/27 12:02:38 nusmv Exp $]

******************************************************************************/


#ifndef __FSM_SEXP_EXPR_H__
#define __FSM_SEXP_EXPR_H__

#include "utils/utils.h"
#include "node/node.h"

/**Type***********************************************************************

  Synopsis     [The Expr type ]

  Description  [An Expr is any expression represented as a sexpr object]  

******************************************************************************/
typedef node_ptr Expr_ptr;

#define EXPR(x) \
      ((Expr_ptr) x)

#define EXPR_CHECK_INSTANCE(x) \
      (nusmv_assert(EXPR(x) != EXPR(NULL)))


EXTERN Expr_ptr Expr_true ARGS(());
EXTERN Expr_ptr Expr_false ARGS(());
EXTERN Expr_ptr Expr_and ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_or ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_not ARGS((const Expr_ptr expr));
EXTERN Expr_ptr Expr_ite ARGS((const Expr_ptr cond, const Expr_ptr t, const Expr_ptr e));
EXTERN Expr_ptr Expr_next ARGS((const Expr_ptr a));



#endif /* __FSM_SEXP_EXPR_H__ */
