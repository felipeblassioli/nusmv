/**CHeaderFile*****************************************************************

  FileName    [SexpFsm.h]

  PackageName [fsm.sexp]

  Synopsis    [The SexpFsm API]

  Description [Class SexpFsm declaration]
                                               
  SeeAlso     [SexpFsm.c]

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

#ifndef __FSM_SEXP_SEXP_FSM_H__
#define __FSM_SEXP_SEXP_FSM_H__

#include "sexp.h"
#include "Expr.h"


/**Type***********************************************************************

  Synopsis     [The SexpFsm type ]

  Description  [The SexpFsm type ]  

  Notes        [This type is just an alias for an already existing type, due
                to the "just-wrap-it" implementation policy w.r.t. the SexpFsm]

******************************************************************************/
typedef struct SexpFsm_TAG* SexpFsm_ptr;


#define SEXP_FSM(x) \
         ((SexpFsm_ptr) x)

#define SEXP_FSM_CHECK_INSTANCE(x) \
         (nusmv_assert(SEXP_FSM(x) != SEXP_FSM(NULL)))


enum SexpFsmType { SEXP_FSM_TYPE_SCALAR, SEXP_FSM_TYPE_BOOLEAN };

/*****************************************************************************/
/*                                                                           */
/* The following functions catch all the unpolished behaviours of the BddFsm */
/* object w.r.t. the concept of "SexpFsm". These functions are here waiting  */
/* for a proper implementation and encapsulation. For the time being there   */
/* is a simple wrapper to the old code.                                      */
/*                                                                           */
/*****************************************************************************/

EXTERN SexpFsm_ptr SexpFsm_create ARGS((VarSet_ptr vars_list, 
					node_ptr justice,
					node_ptr compassion, 
					const enum SexpFsmType type));

EXTERN SexpFsm_ptr 
SexpFsm_create_from_members ARGS((Expr_ptr init, 
				  Expr_ptr invar, 
				  Expr_ptr trans, 
				  Expr_ptr input, 
				  node_ptr justice, node_ptr compassion, 
				  const enum SexpFsmType type));

EXTERN SexpFsm_ptr SexpFsm_copy ARGS((const SexpFsm_ptr self));

EXTERN void SexpFsm_destroy ARGS((SexpFsm_ptr self));

EXTERN SexpFsm_ptr SexpFsm_scalar_to_boolean ARGS((const SexpFsm_ptr self));

EXTERN boolean SexpFsm_is_boolean ARGS((const SexpFsm_ptr self));
               
EXTERN Expr_ptr SexpFsm_get_init ARGS((const SexpFsm_ptr self));

EXTERN Expr_ptr SexpFsm_get_invar ARGS((const SexpFsm_ptr self));

EXTERN Expr_ptr SexpFsm_get_trans ARGS((const SexpFsm_ptr self));

EXTERN Expr_ptr SexpFsm_get_input ARGS((const SexpFsm_ptr self));

EXTERN Expr_ptr 
SexpFsm_get_var_init ARGS((const SexpFsm_ptr self, node_ptr v));

EXTERN Expr_ptr 
SexpFsm_get_var_invar ARGS((const SexpFsm_ptr self, node_ptr v));

EXTERN Expr_ptr 
SexpFsm_get_var_trans ARGS((const SexpFsm_ptr self, node_ptr v));

EXTERN Expr_ptr 
SexpFsm_get_var_input ARGS((const SexpFsm_ptr self, node_ptr v));


EXTERN node_ptr SexpFsm_get_justice ARGS((const SexpFsm_ptr self));

EXTERN node_ptr SexpFsm_get_compassion ARGS((const SexpFsm_ptr self));

EXTERN VarSet_ptr SexpFsm_get_vars_list ARGS((const SexpFsm_ptr self));


#endif /* __FSM_SEXP_SEXP_FSM_H__ */
