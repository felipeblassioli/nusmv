/**CFile*****************************************************************

  FileName    [BddEnc.c]

  PackageName [enc.bdd]

  Synopsis    [The BddEnc implementation]

  Description [This file contains the implementation of a wrapper of (a part 
               of) the BddEnc API onto the NuSMV code structure.
               Only functions needed by the BddFsm are provided.]

  SeeAlso     [BddEnc.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.bdd'' package of NuSMV version 2. 
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

#include "bddInt.h"
#include "BddEnc.h"
#include "BddEnc_private.h"
#include "BddEncCache.h"

#include "utils/error.h"
#include "utils/ustring.h"
#include "utils/utils_io.h"
#include "utils/ucmd.h"
#include "utils/range.h"

#include "parser/symbols.h"
#include "compile/compile.h"


static char rcsid[] UTIL_UNUSED = "$Id: BddEnc.c,v 1.1.2.55.2.8 2005/07/14 15:58:25 nusmv Exp $";



/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------- */
/*             Note:                                                      */
/*             BddEnc class defined into BddEnc_private.h                 */
/* ---------------------------------------------------------------------- */



/**Struct**********************************************************************

  Synopsis    [Private structure used to print BDDs]

  Description [The BddEnc class provides support for printing of BDDs
  via methods print_bdd_begin, print_bdd, print_bdd_end. Since these
  calls can be nested, a stack of BddEncPrintInfo instances is used]
  
******************************************************************************/
typedef struct BddEncPrintInfo_TAG 
{
  hash_ptr hash;
  node_ptr symbols;
  boolean  changes_only;

} BddEncPrintInfo;



/* used by private callbacks */
typedef add_ptr (*ADDPFDA)(DdManager *, add_ptr);
typedef add_ptr (*ADDPFDAA)(DdManager *, add_ptr, add_ptr);
typedef add_ptr (*ADDPFDAII)(DdManager *, add_ptr, int, int);
typedef add_ptr (*ADDPFDAAII)(DdManager *, add_ptr, add_ptr, int, int);



/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define BDD_ENC_CLEAN_ADD_MEMBER(_self, member) \
   if (_self->member != (add_ptr) NULL) {       \
     add_free(_self->dd, _self->member);        \
     _self->member = (add_ptr) NULL;            \
   } 

#define BDD_ENC_CLEAN_BDD_MEMBER(_self, member) \
   if (_self->member != (bdd_ptr) NULL) {       \
     bdd_free(_self->dd, _self->member);        \
     _self->member = (bdd_ptr) NULL;            \
   } 

#define BDD_ENC_COPY_ADD_MEMBER(_self, from, to)    \
   {                                                \
     BDD_ENC_CLEAN_ADD_MEMBER(_self, to);           \
     if (_self->from != (add_ptr) NULL) {           \
       _self->to = add_dup(_self->from);            \
     }                                              \
   }

#define BDD_ENC_COPY_BDD_MEMBER(_self, from, to)    \
   {                                                \
     BDD_ENC_CLEAN_BDD_MEMBER(_self, to);           \
     if (_self->from != (bdd_ptr) NULL) {           \
       _self->to = bdd_dup(_self->from);            \
     }                                              \
   }


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void bdd_enc_init ARGS((BddEnc_ptr self, Encoding_ptr generic_encoding, 
			       DdManager* dd));

static void bdd_enc_deinit ARGS((BddEnc_ptr self));

static void bdd_enc_add_input_var_to_minterm_vars ARGS((BddEnc_ptr self));
static void bdd_enc_add_state_var_to_minterm_vars ARGS((BddEnc_ptr self));
static void bdd_enc_add_next_state_var_to_minterm_vars ARGS((BddEnc_ptr self));

static void bdd_enc_add_state_var ARGS((BddEnc_ptr self, node_ptr name, 
					boolean flag));

static void bdd_enc_add_input_var ARGS((BddEnc_ptr self, node_ptr name, 
					boolean flag));

static void bdd_enc_set_input_variables_add ARGS((BddEnc_ptr self, 
						  add_ptr add));

static void 
bdd_enc_accumulate_input_variables_cube ARGS((BddEnc_ptr self, add_ptr add));

static void 
bdd_enc_set_state_variables_add ARGS((BddEnc_ptr self, add_ptr add));

static void 
bdd_enc_accumulate_state_variables_cube ARGS((BddEnc_ptr self, add_ptr add));

static void 
bdd_enc_set_next_state_variables_add ARGS((BddEnc_ptr self, add_ptr add));

static void 
bdd_enc_accumulate_next_state_variables_cube ARGS((BddEnc_ptr self, 
						   add_ptr add));

static add_ptr bdd_enc_get_input_variables_add ARGS((const BddEnc_ptr self));
static add_ptr bdd_enc_get_state_variables_add ARGS((const BddEnc_ptr self));

static add_ptr 
bdd_enc_get_next_state_variables_add ARGS((const BddEnc_ptr self));

static add_ptr bdd_enc_eval ARGS((BddEnc_ptr self, Expr_ptr expr, 
				  node_ptr context));

static add_ptr 
bdd_enc_eval_recur_case_atom ARGS((BddEnc_ptr self, 
				   Expr_ptr expr, node_ptr ctx));

static add_ptr
bdd_enc_eval_recur_case_dot_array ARGS((BddEnc_ptr self, 
					Expr_ptr expr, node_ptr ctx));

static add_ptr bdd_enc_eval_recur ARGS((BddEnc_ptr self, 
					Expr_ptr expr, node_ptr ctx));

static add_ptr 
bdd_enc_unary_op ARGS((BddEnc_ptr self, ADDPFDA op, node_ptr n,
		       int resflag, int argflag, node_ptr context));

static add_ptr 
bdd_enc_binary_op ARGS((BddEnc_ptr self, ADDPFDAA op, node_ptr n,
			int resflag, int argflag1,int argflag2,
			node_ptr context));

static add_ptr 
bdd_enc_ternary_op ARGS((BddEnc_ptr self, ADDPFDAII op, node_ptr n,
			 int resflag, int argflag, node_ptr context));

static add_ptr 
bdd_enc_quaternary_op ARGS((BddEnc_ptr self, ADDPFDAAII op, node_ptr n,
			    int resflag, int argflag1, int argflag2, 
			    node_ptr context));

static add_ptr 
bdd_enc_if_then_else_op ARGS((BddEnc_ptr self, node_ptr node, 
			      node_ptr context));

static void bdd_enc_pre_encode ARGS((BddEnc_ptr self));


static void bdd_enc_begin_group ARGS((BddEnc_ptr self));
static int bdd_enc_end_group ARGS((BddEnc_ptr self));

static add_ptr 
bdd_enc_get_var_mask_add_recur ARGS((BddEnc_ptr self, 
				     add_ptr var, add_ptr cube));

static add_ptr 
bdd_enc_get_vars_list_mask ARGS((BddEnc_ptr self, NodeList_ptr vars));

static assoc_retval hash_bdd_free ARGS((char* key, char* data, char* arg));

static assoc_retval hash_node_free ARGS((char* key, char* data, char* arg));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Constructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddEnc_ptr BddEnc_create(Encoding_ptr generic_encoding, DdManager* dd)
{
  BddEnc_ptr self = ALLOC(BddEnc, 1);

  BDD_ENC_CHECK_INSTANCE(self);

  bdd_enc_init(self, generic_encoding, dd); 
  bdd_enc_pre_encode(self);

  return self;
}


/**Function********************************************************************

  Synopsis           [Destructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEnc_destroy(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);

  bdd_enc_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Saves the state of self. This is a feature used by 
  bdd-based ltl model checking. Also prepares self for a new encoding]

  Description        [Not all the state is saved, only the parts that are 
  touched by ltl model checking. Call BddEnc_restore after this. 
  After you called this, you can read a new file, flat the hierarchy, and 
  call BddEnc_encode_vars. The merge the saved state by calling BddEnc_merge]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEnc_push_status_and_reset(BddEnc_ptr self)
{

  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert(!self->saved);

  BDD_ENC_COPY_ADD_MEMBER(self, state.__input_variables_add, 
			  saved_state.__input_variables_add);
  BDD_ENC_COPY_ADD_MEMBER(self, state.__state_variables_add, 
			  saved_state.__state_variables_add);
  BDD_ENC_COPY_ADD_MEMBER(self, state.__next_state_variables_add, 
			  saved_state.__next_state_variables_add);
  BDD_ENC_COPY_BDD_MEMBER(self, state.__input_variables_bdd, 
			  saved_state.__input_variables_bdd);
  BDD_ENC_COPY_BDD_MEMBER(self, state.__state_variables_bdd, 
			  saved_state.__state_variables_bdd);
  BDD_ENC_COPY_BDD_MEMBER(self, state.__next_state_variables_bdd, 
			  saved_state.__next_state_variables_bdd);

  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__input_variables_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__state_variables_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__next_state_variables_bdd);

  /* masks are cleaned (they will have to be re-calculated */
  BDD_ENC_CLEAN_ADD_MEMBER(self, __state_vars_mask_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, __input_vars_mask_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, __state_input_vars_mask_add);
  BDD_ENC_CLEAN_BDD_MEMBER(self, __state_vars_mask_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, __input_vars_mask_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, __state_input_vars_mask_bdd);

  
  self->saved_state.minterm_input_vars_dim = 
    self->state.minterm_input_vars_dim;
  self->saved_state.minterm_state_vars_dim = 
    self->state.minterm_state_vars_dim;
  self->saved_state.minterm_next_state_vars_dim = 
    self->state.minterm_next_state_vars_dim;
  self->saved_state.minterm_state_input_vars_dim = 
    self->state.minterm_state_input_vars_dim; 

  /* counters */
  self->saved_state.num_of_state_vars = self->state.num_of_state_vars;
  self->saved_state.num_of_input_vars = self->state.num_of_input_vars;  

  /* lists: */
  self->saved_state.state_vars_add_list = self->state.state_vars_add_list;
  self->state.state_vars_add_list = Nil;

  BddEncCache_push_status_and_reset(self->cache);
  
  /* saves the groups before adding new groups from the tableau: */
  nusmv_assert(self->group_tree == (MtrNode*) NULL);
  self->group_tree = Mtr_CopyTree(Cudd_ReadTree(self->dd), 1);

  self->saved = true;

  if (opt_verbose_level_gt(options, 4)) {
    fprintf(nusmv_stderr, "<BddEnc> Status pushed\n");
  }
}


/**Function********************************************************************

  Synopsis           [Restores the state of self previously saved with 
  BddEnc_push_status_and_reset. This is a feature used by bdd-based ltl 
  model checking]

  Description        [Call after BddEnc_push_status_and_reset after this]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEnc_pop_status(BddEnc_ptr self)
{
  int i;
  node_ptr iter;

  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert(self->saved);

  BddEncCache_pop_status(self->cache);

  BDD_ENC_COPY_ADD_MEMBER(self, saved_state.__input_variables_add, 
			  state.__input_variables_add);
  BDD_ENC_COPY_ADD_MEMBER(self, saved_state.__state_variables_add,
			  state.__state_variables_add);
  BDD_ENC_COPY_ADD_MEMBER(self, saved_state.__next_state_variables_add, 	
		  state.__next_state_variables_add);

  BDD_ENC_COPY_BDD_MEMBER(self, saved_state.__input_variables_bdd, 
			  state.__input_variables_bdd);
  BDD_ENC_COPY_BDD_MEMBER(self, saved_state.__state_variables_bdd, 
			  state.__state_variables_bdd);
  BDD_ENC_COPY_BDD_MEMBER(self, saved_state.__next_state_variables_bdd, 
			  state.__next_state_variables_bdd);

  BDD_ENC_CLEAN_ADD_MEMBER(self, saved_state.__input_variables_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, saved_state.__state_variables_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, saved_state.__next_state_variables_add);
  BDD_ENC_CLEAN_BDD_MEMBER(self, saved_state.__input_variables_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, saved_state.__state_variables_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, saved_state.__next_state_variables_bdd);
  
  /* masks are cleaned (they will have to be re-calculated */
  BDD_ENC_CLEAN_ADD_MEMBER(self, __state_vars_mask_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, __input_vars_mask_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, __state_input_vars_mask_add);
  BDD_ENC_CLEAN_BDD_MEMBER(self, __state_vars_mask_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, __input_vars_mask_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, __state_input_vars_mask_bdd);


  /* ------------------------------------------------------------ */
  /*                   Resets the minterms arrays                 */
  /* ------------------------------------------------------------ */

  /* inputs: */
  for (i = self->saved_state.minterm_input_vars_dim;
       i < self->state.minterm_input_vars_dim; ++i) {
    self->minterm_input_vars[i] = NULL;
  }
  self->state.minterm_input_vars_dim = 
    self->saved_state.minterm_input_vars_dim;
  
  /* state */
  for (i = self->saved_state.minterm_state_vars_dim;
       i < self->state.minterm_state_vars_dim; ++i) {
    self->minterm_state_vars[i] = NULL;
  }
  self->state.minterm_state_vars_dim = 
    self->saved_state.minterm_state_vars_dim;

  /* next state */
  for (i = self->saved_state.minterm_next_state_vars_dim;
       i < self->state.minterm_next_state_vars_dim; ++i) {
    self->minterm_next_state_vars[i] = NULL;
  }
  self->state.minterm_next_state_vars_dim = 
    self->saved_state.minterm_next_state_vars_dim;
  
  /* current states and inputs */
  for (i = self->saved_state.minterm_state_input_vars_dim;
       i < self->state.minterm_state_input_vars_dim; ++i) {
    self->minterm_state_input_vars[i] = NULL;
  }
  self->state.minterm_state_input_vars_dim = 
    self->saved_state.minterm_state_input_vars_dim;


  /* counters: */
  self->state.num_of_state_vars = self->saved_state.num_of_state_vars;
  self->state.num_of_input_vars = self->saved_state.num_of_input_vars;

  /* lists: */
  iter = self->state.state_vars_add_list;
  while (iter != Nil) {
    node_ptr n = iter;
    add_ptr add = (add_ptr) car(iter);

    if (add != (add_ptr) NULL) { add_free(self->dd, add); }
    iter = cdr(iter);
    free_node(n);
  }
  self->state.state_vars_add_list = self->saved_state.state_vars_add_list;
  self->saved_state.state_vars_add_list = Nil;

  /* removes all groups created by merging operations, and restores
     the original groups: */
  Cudd_FreeTree(self->dd);
  Cudd_SetTree(self->dd, self->group_tree);
  self->group_tree = (MtrNode*) NULL;
  
  self->saved = false;

  if (opt_verbose_level_gt(options, 4)) {
    fprintf(nusmv_stderr, "<BddEnc> Status restored\n");
  }
}


/**Function********************************************************************

  Synopsis           [Gets the DD manager this encoding refers to.]

  Description        [Gets the DD manager this encoding refers to.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
DdManager* BddEnc_get_dd_manager(const BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return self->dd;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Encoding_ptr BddEnc_get_symbolic_encoding(const BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  return self->senc;
}


/**Function********************************************************************

  Synopsis           [Gets the support of the set of state variables]

  Description        [Returned bdd is referenced, the caller must free it after 
  it is no longer used. Result is cached if not previously converted from 
  internal ADD representation]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_state_vars_support(const BddEnc_ptr self)
{
  bdd_ptr res; 

  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->state.__state_variables_bdd != (bdd_ptr) NULL) {
    res = bdd_dup(self->state.__state_variables_bdd);
  } 
  else if (self->state.__state_variables_add != (add_ptr) NULL) {
    self->state.__state_variables_bdd = 
      add_to_bdd(self->dd, self->state.__state_variables_add);
    res = bdd_dup(self->state.__state_variables_bdd);
  }
  else res = (bdd_ptr) NULL;
  
  return BDD_VAR_SET(res);
}


/**Function********************************************************************

  Synopsis           [Gets the support of the set of next-state variables]

  Description        [Returned bdd is referenced, the caller must free it after 
  it is no longer used.  Result is cached if not previously converted from 
  internal ADD representation]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_next_state_vars_support(const BddEnc_ptr self)
{ 
  bdd_ptr res; 

  BDD_ENC_CHECK_INSTANCE(self);

  if (self->state.__next_state_variables_bdd != (bdd_ptr) NULL) {
    res = bdd_dup(self->state.__next_state_variables_bdd);
  } 
  else if (self->state.__next_state_variables_add != (add_ptr) NULL) {
    self->state.__next_state_variables_bdd = 
      add_to_bdd(self->dd, self->state.__next_state_variables_add);
    res = bdd_dup(self->state.__next_state_variables_bdd);
  }
  else res = (bdd_ptr) NULL;

  return BDD_VAR_SET(res);
}


/**Function********************************************************************

  Synopsis           [Gets the support of the set of input variables]

  Description        [Returned bdd is referenced, the caller must free it after 
  it is no longer used.  Result is cached if not previously converted from 
  internal ADD representation ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_input_vars_support(const BddEnc_ptr self)
{
  bdd_ptr res; 

  BDD_ENC_CHECK_INSTANCE(self);

  if (self->state.__input_variables_bdd != (bdd_ptr) NULL) {
    res = bdd_dup(self->state.__input_variables_bdd);
  } 
  else if (self->state.__input_variables_add != (add_ptr) NULL) {
    self->state.__input_variables_bdd = 
      add_to_bdd(self->dd, self->state.__input_variables_add);
    res = bdd_dup(self->state.__input_variables_bdd);
  }
  else res = (bdd_ptr) NULL;
  
  return BDD_VAR_SET(res);
}


/**Function********************************************************************

  Synopsis           [Returns the ADD representing the expression "v"]

  Description        [Returned add is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEnc_expr_to_add(BddEnc_ptr self, const Expr_ptr expr, 
			   const node_ptr context)
{
  add_ptr res;

  BDD_ENC_CHECK_INSTANCE(self);

  if (expr == EXPR(NULL)) { res = add_one(self->dd); } 
  else { res = bdd_enc_eval(self, expr, context); }

  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the BDD representing the expression "v"]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_expr_to_bdd(BddEnc_ptr self, const Expr_ptr expr, 
			   const node_ptr context)
{
  bdd_ptr res;
  add_ptr tmp;

  BDD_ENC_CHECK_INSTANCE(self);

  tmp = BddEnc_expr_to_add(self, expr, context);

  CATCH {
    res = add_to_bdd(self->dd, tmp);
  } 
  FAIL {
    internal_error("BddEnc_expr_to_bdd: cannot convert non-propositional" \
		   " expression\n");
  }

  add_free(self->dd, tmp);  
  return res;
}


/**Function********************************************************************

  Synopsis            [Exchange next state variables for state variables, in
                      terms of ADD]

  Description         [Given an ADD whose variables are STATE variables,
                      returns an isomorphic ADD where NEXT-STATE
                      variables have been substituted for the
                      corrisponding STATE variables]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEnc_state_var_to_next_state_var_add(const BddEnc_ptr self, 
					       add_ptr add)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  return add_permute(self->dd, add, self->current2next);
}


/**Function********************************************************************

  Synopsis           [Exchange state variables for next state variables in terms 
  of ADD]

  Description        [Given an ADD whose variables are NEXT-STATE variables,
                      returns an isomorphic ADD where STATE variables
                      have been substituted for the corrisponding
                      STATE variables]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEnc_next_state_var_to_state_var_add(const BddEnc_ptr self, 
					       add_ptr add)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return add_permute(self->dd, add, self->next2current);
}


/**Function********************************************************************

  Synopsis           [Exchange next state variables for state variables]

  Description        [Given a BDD whose variables are STATE variables,
                      returns an isomorphic BDD where NEXT-STATE
                      variables have been substituted for the
                      corrisponding STATE variables]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_state_var_to_next_state_var(const BddEnc_ptr self, bdd_ptr bdd)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  return bdd_permute(self->dd, bdd, self->current2next);
}


/**Function********************************************************************

  Synopsis           [Exchange state variables for next state variables]

  Description        [Given a BDD whose variables are NEXT-STATE variables,
                      returns an isomorphic BDD where STATE variables
                      have been substituted for the corrisponding
                      STATE variables]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_next_state_var_to_state_var(const BddEnc_ptr self, bdd_ptr bdd)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_permute(self->dd, bdd, self->next2current);
}


/**Function********************************************************************

  Synopsis           [Return the list of variables corresponding
  to the current order of variables in the encoding]

  Description        [It returns the list of variables corresponding
  to the current order of variables in the encoding]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr BddEnc_get_ordering(const BddEnc_ptr self)
{
  int cvl, max_level;
  boolean flag = false;
  NodeList_ptr current_ordering;

  BDD_ENC_CHECK_INSTANCE(self);

  current_ordering = NodeList_create();
  max_level = dd_get_size(self->dd);

  if (opt_verbose_level_gt(options, 5)) {
    fprintf(nusmv_stderr, "Number of variables: %d\n", max_level);
  }

  for (cvl = 0; cvl < max_level; ++cvl) {
    int index = dd_get_var_at_level(self->dd, cvl);
    node_ptr name = BddEnc_get_var_name_from_dd_index(self, index);

    if (opt_verbose_level_gt(options, 5)) {
      fprintf(nusmv_stderr,"level %d with variable %d\n", cvl, index);
    }

    if (name == proc_selector_internal_vname)  flag = true;
    
    /* avoid adding NEXT variables */
    if (name != Nil && (node_get_type(name) !=  NEXT)) { 
      NodeList_append(current_ordering, name); 
    }
  }

  if (!flag) { 
    /*
      The _process_select_ variable is inserted at the top of the
      ordering, if not otherwise specified.
    */
    NodeList_prepend(current_ordering, proc_selector_internal_vname);
  }
  
  return current_ordering;
}



/**Function********************************************************************

  Synopsis           [Call before a group of BddEnc_print_bdd calls]

  Description [This sets some fileds used by BddEnc_print_bdd.  Also
  clears the table used when printing only changed states.  After
  having called BddEnc_print_bdd, call BddEnc_print_bdd_end.  If
  <tt>changes_only</tt> is true, than only state variables which
  assume a different value from the previous printed one are printed
  out.]

  SideEffects        []

******************************************************************************/
void BddEnc_print_bdd_begin(BddEnc_ptr self, node_ptr symbols, 
			    boolean changes_only)
{ 
  BddEncPrintInfo* info;

  BDD_ENC_CHECK_INSTANCE(self);
  
  info = ALLOC(BddEncPrintInfo, 1);
  nusmv_assert(info != (BddEncPrintInfo*) NULL);

  info->hash = new_assoc();
  info->symbols = symbols;
  info->changes_only = changes_only;

  self->print_stack = cons( (node_ptr) info, self->print_stack);
}


/**Function********************************************************************

  Synopsis           [Must be called after each call to 
  BddEnc_print_bdd_begin]

  Description        [Must be called after each call to 
  BddEnc_print_bdd_begin, in order to clean up some internal structure]

  SideEffects        []

******************************************************************************/
void BddEnc_print_bdd_end(BddEnc_ptr self)
{
  BddEncPrintInfo* info;
  node_ptr curr;

  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert(self->print_stack != Nil); /*print_bdd_begin previously called*/
  
  curr = self->print_stack;
  info = ( BddEncPrintInfo*) car(curr);
  self->print_stack = cdr(curr);

  clear_assoc_and_free_entries(info->hash, hash_node_free);
  FREE(info);
  free_node(curr);
}


/**Function********************************************************************

  Synopsis [Prints the given bdd. In particular prints only the
  symbols occuring in the symbols list passed to print_bdd_begin]

  Description        [Before calling this method, you must call 
  print_bdd_begin. Then you can call this method once or more, but 
  eventually you will have to call print_bdd_end to commit. 
  Returns the number of symbols actually printed]

  SideEffects        []

******************************************************************************/
int BddEnc_print_bdd(BddEnc_ptr self, bdd_ptr bdd, FILE* file)
{
  BddEncPrintInfo* info;
  node_ptr los;
  add_ptr add;
  int count;

  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert(self->print_stack != Nil); /*print_bdd_begin previously called*/
  
  info = ( BddEncPrintInfo*) car(self->print_stack);

  add = bdd_to_add(self->dd, bdd);

  count = 0;
  los = info->symbols;
  while (los != (node_ptr) NULL) {
    node_ptr cur_sym;
    node_ptr cur_sym_value;
    add_ptr cur_sym_vals;
    add_ptr tmp_add;
    
    cur_sym = car(los);
    
    if (cur_sym == proc_selector_internal_vname) {
      /* process name is not printed if there are no processes: */
      if ( llength(Encoding_get_var_range(self->senc, 
			  proc_selector_internal_vname)) < 2 ) {
	los = cdr(los);
	continue;
      }
    }

    cur_sym_vals = bdd_enc_eval(self, cur_sym, Nil);
    tmp_add = add_if_then(self->dd, add, cur_sym_vals);

    cur_sym_value = add_value(self->dd, tmp_add);
    add_free(self->dd, tmp_add);
    add_free(self->dd, cur_sym_vals);
    
    if (info->changes_only) {
      if (cur_sym_value == find_assoc(info->hash, cur_sym)) {
	los = cdr(los);
	continue;
      }
      insert_assoc(info->hash, cur_sym, cur_sym_value);
    }

    indent_node(file, "", cur_sym, " = ");
    print_node(file, cur_sym_value);
    fprintf(file, "\n");

    count += 1;
    los = cdr(los);
  } /* while loop */

  add_free(self->dd, add);
  return count;
}


/**Function********************************************************************

  Synopsis           [Prints a set of states]

  Description        [Prints a set of states]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEnc_print_set_of_states(BddEnc_ptr self, bdd_ptr states, 
				boolean changes_only, FILE* file)
{
  bdd_ptr* array;
  int array_size, j;
  node_ptr vars; 
  boolean res;

  BDD_ENC_CHECK_INSTANCE(self);

  array_size = BddEnc_count_states_of_bdd(self, states);
  array = ALLOC(bdd_ptr, array_size);
  nusmv_assert(array != (bdd_ptr*) NULL);

  res = BddEnc_pick_all_terms_states(self, states, array, array_size);
  nusmv_assert(!res); /* an error occurred */
  
  vars = NodeList_to_node_ptr(Encoding_get_state_vars_list(self->senc));
  BddEnc_print_bdd_begin(self, vars, changes_only);

  inc_indent_size();
  for (j=0; j < array_size; ++j) {
    fprintf(file, "------- State %4.d ------\n", j+1);
    
    BddEnc_print_bdd(self, array[j], file);
  }  
  fprintf(nusmv_stdout, "-------------------------\n");
  dec_indent_size();

  BddEnc_print_bdd_end(self);
  
  for (j=0; j < array_size; ++j) {
    bdd_free(self->dd, array[j]);
  }
  FREE(array);
}


/**Function********************************************************************

  Synopsis           [Prints a set of input pairs]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEnc_print_set_of_inputs(BddEnc_ptr self, bdd_ptr inputs,
				boolean changes_only, FILE* file)
{
  bdd_ptr* array;
  int array_size, j;
  node_ptr vars; 
  boolean res;

  BDD_ENC_CHECK_INSTANCE(self);

  array_size = BddEnc_count_inputs_of_bdd(self, inputs);
  array = ALLOC(bdd_ptr, array_size);
  nusmv_assert(array != (bdd_ptr*) NULL);

  res = BddEnc_pick_all_terms_inputs(self, inputs, array, array_size);
  nusmv_assert(!res); /* an error occurred */
  
  vars = NodeList_to_node_ptr(Encoding_get_input_vars_list(self->senc));

  BddEnc_print_bdd_begin(self, vars, changes_only);
  inc_indent_size();
  for (j=0; j < array_size; ++j) {
    fprintf(file, "------- Input %4.d ------\n", j+1);
    
    BddEnc_print_bdd(self, array[j], file);
  }
  fprintf(nusmv_stdout, "-------------------------\n");
  dec_indent_size();

  BddEnc_print_bdd_end(self);
  
  for (j=0; j < array_size; ++j) {
    bdd_free(self->dd, array[j]);
  }
  FREE(array);
}


/**Function********************************************************************

  Synopsis           [Prints a set of state-input pairs]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEnc_print_set_of_state_input_pairs (BddEnc_ptr self, 
					    bdd_ptr state_input_pairs,
					    boolean changes_only, 
					    FILE* file)
{
  bdd_ptr* array;
  int array_size, j;
  node_ptr svars, ivars; 
  boolean res;

  BDD_ENC_CHECK_INSTANCE(self);

  array_size = BddEnc_get_minterms_of_bdd(self, state_input_pairs);
  array = ALLOC(bdd_ptr, array_size);
  nusmv_assert(array != (bdd_ptr*) NULL);

  res = BddEnc_pick_all_terms_states_inputs(self, state_input_pairs, 
					    array, array_size);
  nusmv_assert(!res); /* an error occurred */

  svars = NodeList_to_node_ptr(Encoding_get_state_vars_list(self->senc));
  ivars = NodeList_to_node_ptr(Encoding_get_input_vars_list(self->senc));

  inc_indent_size();
  BddEnc_print_bdd_begin(self, svars, changes_only);
  for (j=0; j < array_size; ++j) {
    fprintf(file, "------- State-Input Pair %4.d ------\n", j+1);

    /* prints the set of states... */
    BddEnc_print_bdd(self, array[j], file);
    
    /* ...and then the set of (corresponding) inputs: */
    inc_indent_size();
    BddEnc_print_bdd_begin(self, ivars, changes_only);
    BddEnc_print_bdd(self, array[j], file);
    BddEnc_print_bdd_end(self);
    dec_indent_size();
  }

  fprintf(nusmv_stdout, "-------------------------\n");
  dec_indent_size();
  BddEnc_print_bdd_end(self);
  
  for (j=0; j < array_size; ++j) {
    bdd_free(self->dd, array[j]);
  }
  FREE(array);
}


/**Function********************************************************************

  Synopsis           [Prints out the symbolic names of boolean
  variables stored in a cube.]

  Description        [Given a cube of boolean BDD variables, this
  function prints out the symbolic names of the corresponding
  variables. The symbolic name of the variables to be printed out are
  listed in <tt>list_of_sym</tt>.]

  SideEffects        [None]

******************************************************************************/
void BddEnc_print_vars_in_cube(BddEnc_ptr self, bdd_ptr cube, 
			       node_ptr list_of_sym, 
			       FILE* file)
{
  node_ptr los = list_of_sym;
  add_ptr a_cube;

  BDD_ENC_CHECK_INSTANCE(self);

  a_cube = bdd_to_add(self->dd, cube);

  fprintf(file, "Current VARS:\n");
  while (los != (node_ptr) NULL) {
    node_ptr cur_sym = car(los);
    add_ptr cur_sym_vals = bdd_enc_eval(self, cur_sym, Nil);
    add_ptr cur_sym_cube = add_support(self->dd, cur_sym_vals);
    add_ptr is_in = add_cube_diff(self->dd, a_cube, cur_sym_cube);

    if (is_in != a_cube) {
      indent_node(file, "", cur_sym, " ");
    }
    add_free(self->dd, cur_sym_vals);
    add_free(self->dd, cur_sym_cube);
    add_free(self->dd, is_in);

    los = cdr(los);
  } /* while loop */

  fprintf(file, "\nNext VARS:\n");  
  los = list_of_sym;
  while (los != (node_ptr) NULL) {
    node_ptr cur_sym = car(los);
    add_ptr cur_sym_vals = bdd_enc_eval(self, cur_sym, Nil);
    add_ptr next_cur_sym_vals = add_permute(self->dd, cur_sym_vals, 
					    self->current2next);

    add_ptr next_cur_sym_cube = add_support(self->dd, next_cur_sym_vals);
    add_ptr next_is_in = add_cube_diff(self->dd, 
				       a_cube, next_cur_sym_cube);

    if (next_is_in != a_cube) {
      indent_node(file, "", find_node(NEXT, cur_sym, Nil), " ");
    }
    add_free(self->dd, cur_sym_vals);
    add_free(self->dd, next_cur_sym_vals);
    add_free(self->dd, next_cur_sym_cube);
    add_free(self->dd, next_is_in);

    los = cdr(los);
  } /* while loop */

  add_free(self->dd, a_cube);
  fprintf(file,"\n");
}


/**Function********************************************************************

  Synopsis           [Return the number of states of a given ADD.]

  Description        [Return the number of minterms (i.e. states)
  represented by an ADD.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
double BddEnc_count_states_of_add(const BddEnc_ptr self, add_ptr add) 
{
  BDD_ENC_CHECK_INSTANCE(self);

  return add_count_minterm(self->dd, add, (self->state.num_of_state_vars / 2));
}


/**Function********************************************************************

  Synopsis           [Return the number of states of a given BDD.]

  Description        [Return the number of states represented by a BDD.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
double BddEnc_count_states_of_bdd(const BddEnc_ptr self, bdd_ptr bdd) 
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_count_minterm(self->dd, bdd, (self->state.num_of_state_vars / 2));
}


/**Function********************************************************************

  Synopsis           [Return the number of inputs of a given BDD.]

  Description        [Return the number of inputs represented by a BDD.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
double BddEnc_count_inputs_of_bdd(const BddEnc_ptr self, bdd_ptr bdd) 
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_count_minterm(self->dd, bdd, (self->state.num_of_input_vars));
}


/**Function********************************************************************

  Synopsis           [Return the number of states inputs of a given BDD.]

  Description        [Return the number of states inputs represented by a BDD.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
double BddEnc_count_states_inputs_of_bdd(const BddEnc_ptr self, bdd_ptr bdd) 
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_count_minterm(self->dd, bdd,
                           ((self->state.num_of_input_vars) +
                            (self->state.num_of_state_vars / 2)));
}



/**Function********************************************************************

  Synopsis           [Return the number of minterms of a given ADD.]

  Description        [Return the number of minterms 
  represented by a ADD.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
double BddEnc_get_minterms_of_add(const BddEnc_ptr self, add_ptr add) 
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  return add_count_minterm(self->dd, add, 
			   self->state.minterm_state_input_vars_dim);
}


/**Function********************************************************************

  Synopsis           [Return the number of minterms of a given BDD.]

  Description        [Return the number of minterms 
  represented by a BDD.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
double BddEnc_get_minterms_of_bdd(const BddEnc_ptr self, bdd_ptr bdd) 
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  return bdd_count_minterm(self->dd, bdd, 
			   self->state.minterm_state_input_vars_dim);
}


/**Function********************************************************************

  Synopsis           [Extracts a minterm from a given BDD.]

  Description        [Extracts a minterm from a given BDD. Returned 
  bdd is referenced]

  SideEffects        []

  SeeAlso            [bdd_pick_one_minterm]

******************************************************************************/
bdd_ptr BddEnc_pick_one_state(const BddEnc_ptr self, bdd_ptr states)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  return bdd_pick_one_minterm(self->dd, states, self->minterm_state_vars, 
			      self->state.minterm_state_vars_dim);
}


/**Function********************************************************************

  Synopsis           [Extracts a minterm from a given BDD.]

  Description        [Extracts a minterm from a given BDD. Returned 
  bdd is referenced]

  SideEffects        []

  SeeAlso            [bdd_pick_one_minterm]

******************************************************************************/
bdd_ptr BddEnc_pick_one_input(const BddEnc_ptr self, bdd_ptr inputs)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  return bdd_pick_one_minterm(self->dd, inputs, self->minterm_input_vars, 
			      self->state.minterm_input_vars_dim);
}


/**Function********************************************************************

  Synopsis           [Returns the array of All Possible Minterms]

  Description        [Takes a minterm and returns an array of all its terms,
  according to internally kept vars. Notice that
  the array of the result has to be previously allocated, and its size
  must be greater or equal the number of the minterms.
  The returned array contains referenced BDD so it is necessary to
  dereference them after their use. Returns true if an error occurred]

  SideEffects        [result_array will change]

  SeeAlso            [bdd_pick_all_terms]

******************************************************************************/
boolean BddEnc_pick_all_terms_states_inputs(const BddEnc_ptr self, 
					    bdd_ptr bdd, 
					    bdd_ptr* result_array, 
					    const int array_len)
{
  int res = 1; 
  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->state.minterm_state_input_vars_dim > 0) {
    res = bdd_pick_all_terms(self->dd, bdd, self->minterm_state_input_vars, 
			     self->state.minterm_state_input_vars_dim, 
			     result_array, array_len);
  }
  return res == 1;  
}


/**Function********************************************************************

  Synopsis           [Returns the array of All Possible Minterms]

  Description        [Takes a minterm and returns an array of all its terms,
  according to internally kept vars. Notice that
  the array of the result has to be previously allocated, and its size
  must be greater or equal the number of the minterms.
  The returned array contains referenced BDD so it is necessary to
  dereference them after their use. Returns true if an error occurred]

  SideEffects        [result_array will change]

  SeeAlso            [bdd_pick_all_terms]

******************************************************************************/
boolean BddEnc_pick_all_terms_states(const BddEnc_ptr self, bdd_ptr bdd, 
				     bdd_ptr* result_array, 
				     const int array_len)
{
  int res = 1; 
  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->state.minterm_state_vars_dim > 0) {
    res = bdd_pick_all_terms(self->dd, bdd, self->minterm_state_vars, 
			     self->state.minterm_state_vars_dim, 
			     result_array, array_len);
  }
  return res == 1;  
}


/**Function********************************************************************

  Synopsis           [Returns the array of All Possible Minterms]

  Description        [Takes a minterm and returns an array of all its terms,
  according to internally kept vars. Notice that
  the array of the result has to be previously allocated, and its size
  must be greater or equal the number of the minterms.
  The returned array contains referenced BDD so it is necessary to
  dereference them after their use. Returns true if an error occurred]

  SideEffects        [result_array will change]

  SeeAlso            [bdd_pick_all_terms]

******************************************************************************/
boolean BddEnc_pick_all_terms_inputs(const BddEnc_ptr self, bdd_ptr bdd, 
				     bdd_ptr* result_array, 
				     const int array_len)
{
  int res = 1; 
  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->state.minterm_input_vars_dim > 0) {
    res = bdd_pick_all_terms(self->dd, bdd, self->minterm_input_vars, 
			     self->state.minterm_input_vars_dim, 
			     result_array, array_len);
  }
  return res == 1;  
}



/**Function********************************************************************

  Synopsis           [Extracts a random minterm from a given BDD.]

  Description        [Extracts a random minterm from a given BDD.
  Returned bdd is referenced]

  SideEffects        []

  SeeAlso            [bdd_pick_one_minterm_rand]

******************************************************************************/
bdd_ptr BddEnc_pick_one_state_rand(const BddEnc_ptr self, bdd_ptr states)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_pick_one_minterm_rand(self->dd, states, self->minterm_state_vars, 
				   self->state.minterm_state_vars_dim);
}


/**Function********************************************************************

  Synopsis           [Extracts a random minterm from a given BDD.]

  Description        [Extracts a random minterm from a given BDD.
  Returned bdd is referenced]

  SideEffects        []

  SeeAlso            [bdd_pick_one_minterm_rand]

******************************************************************************/
bdd_ptr BddEnc_pick_one_input_rand(const BddEnc_ptr self, bdd_ptr inputs)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_pick_one_minterm_rand(self->dd, inputs, self->minterm_input_vars, 
				   self->state.minterm_input_vars_dim);
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
node_ptr BddEnc_get_var_name_from_dd_index(const BddEnc_ptr self, int index)
{
  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert((index >= 0) && (index < MAX_VAR_INDEX));

  return self->variable_names[index];
}


/**Function********************************************************************

  Synopsis           [Returns the DD index of the given variable]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
int BddEnc_get_var_index_from_name(const BddEnc_ptr self, node_ptr name)
{
  add_ptr add;
  int res;

  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert(Encoding_is_symbol_var(self->senc, name));

  add = BddEnc_get_symbol_add(self, name);
  res = add_index(self->dd, add);
  add_free(self->dd, add);
  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the ADD leaf corresponding to the given atom]

  Description        [Returns the ADD leaf corresponding to the given atom, 
  if defined, NULL otherwise. The returned ADD - if any - is referenced.
  If the inner flag enforce_constant is set, 
  
  Suppose to have a declaration of this kind:<br> 
  <pre>
  VAR 
    state : {idle, stopped}
  <pre>
  then in the constant hash for the atom <tt>idle</tt> there is the
  corresponding leaf ADD, i.e. the ADD whose value is the symbol
  <tt>idle</tt>.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEnc_constant_to_add(const BddEnc_ptr self, node_ptr constant)
{
  add_ptr add; 

  BDD_ENC_CHECK_INSTANCE(self);

  add = BddEncCache_lookup_constant(self->cache, constant);
  if ((add == (add_ptr) NULL) && self->enforce_constant) {
    constant = lookup_flatten_constant_hash(constant);
    if (constant != (node_ptr) NULL) { 
      /* We declare the constant so that it is known to the system. */
      Encoding_declare_constant(self->senc, constant);
      add = add_leaf(self->dd, constant);
      BddEncCache_new_constant(self->cache, constant, add);
    }
  }
  
  return add;
}


/**Function********************************************************************

  Synopsis           [Merges the previously saved bdds (and other parts) 
  and the current encoding]

  Description        [Call after the sequence BddEnc_push_status_and_reset, 
  BddEnc_encode_vars, and before BddEnc_pop_status]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEnc_merge(BddEnc_ptr self)
{
  int i;
  NodeList_ptr list;
  ListIter_ptr iter;
  GroupSet_ptr groups[2];

  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert(self->saved);

  /* performs the pre-encode in 2nd stage */
  if (opt_verbose_level_gt(options, 5)) {
    fprintf(nusmv_stdout, " <BddEnc> merging encoding (in 2nd stage)\n");
    inc_indent_size();
  }

  /* pre-encodes all constants: */
  list = Encoding_get_constants_list(self->senc);
  iter = NodeList_get_first_iter(list);
  while (! ListIter_is_end(iter)) {
    node_ptr constant = NodeList_get_elem_at(list, iter);
    add_ptr add_constant; 

    if (BddEncCache_is_constant_defined(self->cache, constant)) {
      iter = ListIter_get_next(iter);
      continue;
    }

    if (opt_verbose_level_gt(options, 4)) {
      fprintf(nusmv_stdout, " <BddEnc> pre-allocating constant (in 2nd stage)");
      print_node(nusmv_stdout, constant);
      fprintf(nusmv_stdout, "\n");
    }

    add_constant = add_leaf(self->dd, constant);
    BddEncCache_new_constant(self->cache, constant, add_constant);
    add_free(self->dd, add_constant);
    iter = ListIter_get_next(iter);
  }


  /* Iterates on bool vars (in separated groups), allocating and
     encoding state & input variables: */

  if (opt_verbose_level_gt(options, 4)) {
    inc_indent_size();
  }
  
  groups[0] = Encoding_get_bool_input_vars_groups(self->senc);
  groups[1] = Encoding_get_bool_state_vars_groups(self->senc);

  for (i = 0; i < sizeof(groups)/sizeof(groups[0]); ++i) {
    ListIter_ptr gr_iter = GroupSet_get_first_iter(groups[i]);
    while (! ListIter_is_end(gr_iter)) {

      bdd_enc_begin_group(self);
      
      list = GroupSet_get_group(groups[i], gr_iter);
      iter = NodeList_get_first_iter(list);

      if (opt_verbose_level_gt(options, 4)) {
	inc_indent_size();
      }

      while (! ListIter_is_end(iter)) {
	node_ptr name = NodeList_get_elem_at(list, iter);

	if (opt_verbose_level_gt(options, 4)) {
	  fprintf(nusmv_stdout, "<BddEnc> pre-allocating variable ");
	  print_node(nusmv_stdout, name);
	  fprintf(nusmv_stdout, " (in 2nd stage)\n");
	}

	if (Encoding_is_symbol_input_var(self->senc, name)) {
	  bdd_enc_add_input_var(self, name, true);      
	}
	else if (Encoding_is_symbol_state_var(self->senc, name)) {
	  bdd_enc_add_state_var(self, name, true);      
	}
							      
	iter = ListIter_get_next(iter);
      }

      if (opt_verbose_level_gt(options, 4)) {
	dec_indent_size();
      }

      bdd_enc_end_group(self);

      gr_iter = ListIter_get_next(gr_iter);
    } /* loop on groups */
  } /* for loop */

  if (opt_verbose_level_gt(options, 4)) {
    dec_indent_size();
  }
}


/**Function********************************************************************

  Synopsis           [Complements an ADD according to a flag.]

  Description        [Given the ADD <code>a</code>, this function returns
  the negation of ADD <code>a</code> or <code>a</code> itself according the
  value of <code>flag</code>. If <code>flag = -1</code> then returns <code>not
  a</code>, else returns <code>a</code>. It is important that the ADD is a
  zero/one ADD (i.e. it has only zero or one as leaf).]

  SideEffects        []

  SeeAlso            [bdd_enc_eval]

******************************************************************************/
add_ptr BddEnc_eval_sign_add(BddEnc_ptr self, add_ptr a, int flag)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (flag == -1 ) { a = add_not(self->dd, a); }
  else  { add_ref(a); }
  
  return a;
}


/**Function********************************************************************

  Synopsis           [Complements a BDD according to a flag.]

  Description        [Given the BDD <code>a</code>, this function returns
  the negation of BDD <code>a</code> or <code>a</code> itself according the
  value of <code>flag</code>. If <code>flag = -1</code> then returns <code>not
  a</code>, else returns <code>a</code>. It is important that the BDD is a
  zero/one BDD (i.e. it has only zero or one as leaf).]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_eval_sign_bdd(BddEnc_ptr self, bdd_ptr a, int flag)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (flag == -1 ) { a = bdd_not(self->dd, a); }
  else  { bdd_ref(a); }
  
  return a;
}


/**Function********************************************************************

  Synopsis           [Evaluates a number in a context.]

  Description        [Evaluate the <em>NUMBER</em> represented by <code>e</code> in
  context <code>context</code>. <em>NUMBERS</em> can be encoded in
  different ways in different processes.]

  SideEffects        []

  SeeAlso            [bdd_enc_eval]

******************************************************************************/
int BddEnc_eval_num(BddEnc_ptr self, node_ptr e, node_ptr context)
{
  node_ptr n;
  add_ptr d;
  boolean old;

  BDD_ENC_CHECK_INSTANCE(self);

  old = self->enforce_constant;
  self->enforce_constant = true;
  d = bdd_enc_eval(self, e, context);
  self->enforce_constant = old;

  nusmv_assert(add_isleaf(d));

  n = add_get_leaf(self->dd, d);
  add_free(self->dd, d);

  if (node_get_type(n) != NUMBER) { 
    rpterr("BddEnc_eval_num: numeric constant required"); 
  }

  return (int) car(n);
}


/**Function********************************************************************

  Synopsis           [Evaluates a constant expression.]

  Description        [Evaluate a constant expression. If the
  expression does not evaluate to a constant, then an internal error
  is generated. Returned add is referenced.]

  SideEffects        []

  SeeAlso            [eval eval_num]

******************************************************************************/
add_ptr BddEnc_eval_constant(BddEnc_ptr self, Expr_ptr expr, node_ptr context) 
{
  add_ptr result;

  boolean enforce_constant_saved;

  enforce_constant_saved = self->enforce_constant;
  self->enforce_constant = true;
  result = bdd_enc_eval(self, expr, context);
  self->enforce_constant = enforce_constant_saved;

  if (add_isleaf(result) == 0) {
    internal_error("BddEnc_eval_constant: Evaluating a non" \
		   " constant expression");
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Given a symbol, the corresponding ADD is returned.]

  Description        [Given the symbol represented by <code>n</code>, this
  function returns the ADD of its definition, or NULL if not defined. 
  Errors occurs if circularly defined. Returned add is referenced]

  SideEffects        []

  SeeAlso            [BddEnc_expr_to_add]

******************************************************************************/
add_ptr BddEnc_get_symbol_add(BddEnc_ptr self, node_ptr name)
{
  add_ptr res;
  node_ptr def;

  BDD_ENC_CHECK_INSTANCE(self);

  def = Encoding_lookup_symbol(self->senc, name);

  if (def == (node_ptr) NULL) return (add_ptr) NULL;

  /* do we required symbol to be a constant? */
  if ( self->enforce_constant 
       && Encoding_is_symbol_var(self->senc, name) ) {
    self->enforce_constant = false;
    rpterr("constant required");
  }

  if (Encoding_is_symbol_define(self->senc, name)) {
    res = BddEncCache_get_definition(self->cache, name);
    if (res == EVALUATING) { error_circular(name); }
    if (res != (add_ptr) NULL) return res; /* already referenced */
  }
  
  /* Otherwise starts the evaluation... */
  if (opt_verbose_level_gt(options, 3)) {
    inc_indent_size();
    indent_node(nusmv_stderr, "<BddEnc> evaluating ", name, ":\n");
  }

  /* gets the encoding associated with possible variable */
  if (Encoding_is_symbol_var(self->senc, name)) {
    if (Encoding_is_symbol_boolean_var(self->senc, name)) {
      res = BddEncCache_lookup_var(self->cache, name);
      nusmv_assert(res != (add_ptr) NULL);

      if (opt_verbose_level_gt(options, 3)) { dec_indent_size(); }
      return res;
    }

    /* otherwise selects the symbolic encoding, and continue */
    def = Encoding_get_var_encoding(self->senc, name); 
  }  
 
  io_atom_push(name); /* for error reporting */
  BddEncCache_set_definition(self->cache, name, EVALUATING);
  res = bdd_enc_eval(self, def, Nil);
  BddEncCache_set_definition(self->cache, name, res);
  io_atom_pop();

  if (opt_verbose_level_gt(options, 3)) {
    indent_node(nusmv_stderr, "size of ", name, " = ");
    fprintf(nusmv_stderr, "%d ADD nodes\n", add_size(self->dd, res));
    dec_indent_size();
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Returnes a list of ADDs corresponding to the model state 
  vars]

  Description        [This function is used by the printer of reachable states. 
  The ownership of the returned ADDs - as well the list itself - is kept 
  by self (i.e. single ADDs are not referenced)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr BddEnc_get_state_vars_add_list(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->state.state_vars_add_list == Nil) {
    NodeList_ptr list;
    ListIter_ptr iter;

    list = Encoding_get_state_vars_list(self->senc);
    iter = NodeList_get_first_iter(list);
    while (! ListIter_is_end(iter)) {
      node_ptr name = NodeList_get_elem_at(list, iter);
      add_ptr add;
      add = BddEnc_get_symbol_add(self, name);
      nusmv_assert(add != (add_ptr) NULL);
      
      self->state.state_vars_add_list = cons((node_ptr) add, 
					     self->state.state_vars_add_list);
    
      iter = ListIter_get_next(iter);
    } /* while */
  }
  
  return self->state.state_vars_add_list;
} 


/**Function********************************************************************

  Synopsis           [Writes on a file the variable order.]

  Description [This function writes the variable order currently in
  use in the system in the specified output file. The file generated
  as output can be used as input order file for next computations. If
  the specified output file is an empty string ("" or NULL, see
  util_is_string_null) output is redirected to stdout.  The output
  content depends on the value of dump_type.]

  SideEffects        []

  SeeAlso            [Compile_ReadOrder]

******************************************************************************/
void BddEnc_write_order(const BddEnc_ptr self, 
			const char *output_order_file_name, 
			const VarOrderingType dump_type)
{
  FILE* oof;
  NodeList_ptr ordering;
  NodeList_ptr scalar_vars; 
  ListIter_ptr iter;

  BDD_ENC_CHECK_INSTANCE(self);
  
  scalar_vars = NodeList_create();

  /* First collects the list of vars to be dumped: */
  ordering = BddEnc_get_ordering(self);
  iter = NodeList_get_first_iter(ordering);
  while (! ListIter_is_end(iter)) {
    node_ptr name = NodeList_get_elem_at(ordering, iter);
    
    if (Encoding_is_var_bit(self->senc, name) && (dump_type != DUMP_BITS)) {
      /* retrieve the corresponding scalar variable: */
      name = Encoding_get_scalar_var_of_bit(self->senc, name);
    }

    if (! NodeList_belongs_to(scalar_vars, name)) {
      NodeList_append(scalar_vars, name);
    }
    
    iter = ListIter_get_next(iter);
  }
  

  /* Opens the file: */
  if (!util_is_string_null(output_order_file_name)) {
    oof = fopen(output_order_file_name, "w");
    if (oof == NULL) {
      rpterr("output_order: unable to open file %s", output_order_file_name);
    }
  }
  else oof = nusmv_stdout;  /* uses stdout */

  /* Actually dump the list: */
  iter = NodeList_get_first_iter(scalar_vars);
  while (! ListIter_is_end(iter)) {
    node_ptr name = NodeList_get_elem_at(scalar_vars, iter);
    print_node(oof, name);
    fprintf(oof, "\n"); 

    iter = ListIter_get_next(iter);
  }

  /* close the file: */
  if (oof != nusmv_stdout) { 
    if (fclose(oof) == EOF) {
      rpterr("cannot close %s", output_order_file_name);
    }
  }

  NodeList_destroy(scalar_vars);

  if (opt_verbose_level_gt(options, 0)) {
    if (output_order_file_name != NULL) {
      fprintf(nusmv_stderr, "NuSMV: variable order output to file %s\n", 
	      output_order_file_name);
    }
  }
}


/**Function********************************************************************

  Synopsis [Returns the mask (as ADD) in terms of state
  variables]

  Description        [Returned add is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEnc_get_state_vars_mask_add(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->__state_vars_mask_add == (add_ptr) NULL) {
    self->__state_vars_mask_add = bdd_enc_get_vars_list_mask(self, 
				   Encoding_get_state_vars_list(self->senc));
    nusmv_assert(self->__state_vars_mask_add != (add_ptr) NULL);
  }

  return add_dup(self->__state_vars_mask_add);
}



/**Function********************************************************************

  Synopsis           [Returns the mask (as ADD) in terms of input variables]

  Description        [Returned add is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEnc_get_input_vars_mask_add(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->__input_vars_mask_add == (add_ptr) NULL) {
    self->__input_vars_mask_add = bdd_enc_get_vars_list_mask(self, 
				   Encoding_get_input_vars_list(self->senc));
    nusmv_assert(self->__input_vars_mask_add != (add_ptr) NULL);
  }

  return add_dup(self->__input_vars_mask_add);
}


/**Function********************************************************************

  Synopsis           [Returns the mask (as ADD) in terms of state and 
  input variables]

  Description        [Returned add is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEnc_get_state_input_vars_mask_add(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->__state_input_vars_mask_add == (add_ptr) NULL) {
    self->__state_input_vars_mask_add = bdd_enc_get_vars_list_mask(self, 
   			   Encoding_get_all_model_vars_list(self->senc));
    nusmv_assert(self->__state_input_vars_mask_add != (add_ptr) NULL);
  }

  return add_dup(self->__state_input_vars_mask_add);
}


/**Function********************************************************************

  Synopsis           [Returns the mask (as BDD) in terms of state variables]

  Description        [Returned bdd is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_get_state_vars_mask_bdd(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->__state_vars_mask_bdd == (bdd_ptr) NULL) {
    add_ptr mask_add;

    mask_add = BddEnc_get_state_vars_mask_add(self);
    self->__state_vars_mask_bdd = add_to_bdd(self->dd, mask_add);
    add_free(self->dd, mask_add);
  }

  return bdd_dup(self->__state_vars_mask_bdd);
}


/**Function********************************************************************

  Synopsis           [Returns the mask (as BDD) in terms of input variables]

  Description        [Returned bdd is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_get_input_vars_mask_bdd(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->__input_vars_mask_bdd == (bdd_ptr) NULL) {
    add_ptr mask_add;

    mask_add = BddEnc_get_input_vars_mask_add(self);
    self->__input_vars_mask_bdd = add_to_bdd(self->dd, mask_add);
    add_free(self->dd, mask_add);
  }

  return bdd_dup(self->__input_vars_mask_bdd);
}


/**Function********************************************************************

  Synopsis           [Returns the mask (as BDD) in terms of state and input
  variables]

  Description        [Returned bdd is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_get_state_input_vars_mask_bdd(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  
  if (self->__state_input_vars_mask_bdd == (bdd_ptr) NULL) {
    add_ptr mask_add;

    mask_add = BddEnc_get_state_input_vars_mask_add(self);
    self->__state_input_vars_mask_bdd = add_to_bdd(self->dd, mask_add);
    add_free(self->dd, mask_add);
  }

  return bdd_dup(self->__state_input_vars_mask_bdd);
}


/**Function********************************************************************

  Synopsis           [Applies a mask to the given add which must contain only 
  state variables]

  Description        [Returned add is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEnc_apply_state_vars_mask_add(BddEnc_ptr self, add_ptr states)
{
  add_ptr mask, res;
  
  BDD_ENC_CHECK_INSTANCE(self);
  
  mask = BddEnc_get_state_vars_mask_add(self);
  res = add_and(self->dd, states, mask);
  add_free(self->dd, mask);
  
  return res;
}

/**Function********************************************************************

  Synopsis           [Applies a mask to the given add which must contain only 
  input variables]

  Description [Returned add is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEnc_apply_input_vars_mask_add(BddEnc_ptr self, add_ptr inputs)
{
  add_ptr mask, res;
  
  BDD_ENC_CHECK_INSTANCE(self);
  
  mask = BddEnc_get_input_vars_mask_add(self);
  res = add_and(self->dd, inputs, mask);
  add_free(self->dd, mask);
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Applies a mask to the given add which must contain  
  state and input variables]

  Description [Returned add is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEnc_apply_state_input_vars_mask_add(BddEnc_ptr self, 
					       add_ptr states_inputs)
{
  add_ptr mask, res;
  
  BDD_ENC_CHECK_INSTANCE(self);
  
  mask = BddEnc_get_state_input_vars_mask_add(self);
  res = add_and(self->dd, states_inputs, mask);
  add_free(self->dd, mask);
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Applies a mask to the given BDD which must contain only 
  state variables]

  Description [Returned bdd is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddStates BddEnc_apply_state_vars_mask_bdd(BddEnc_ptr self, BddStates states)
{
  bdd_ptr mask;
  BddStates res;
  
  BDD_ENC_CHECK_INSTANCE(self);
  
  mask = BddEnc_get_state_vars_mask_bdd(self);
  res = BDD_STATES( bdd_and(self->dd, states, mask) );
  bdd_free(self->dd, mask);
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Applies a mask to the given BDD which must contain only 
  input variables]

  Description [Returned bdd is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddInputs BddEnc_apply_input_vars_mask_bdd(BddEnc_ptr self, BddInputs inputs)
{
  bdd_ptr mask;
  BddInputs res;
  
  BDD_ENC_CHECK_INSTANCE(self);
  
  mask = BddEnc_get_input_vars_mask_bdd(self);
  res = BDD_INPUTS( bdd_and(self->dd, inputs, mask) );
  bdd_free(self->dd, mask);
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Applies a mask to the given BDD which must contain  
  state and input variables]

  Description [Returned bdd is referenced. Calculated mask will be
  cached for future use]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddStatesInputs 
BddEnc_apply_state_input_vars_mask_bdd(BddEnc_ptr self,
				       BddStatesInputs states_inputs)
{
  bdd_ptr mask;
  BddStatesInputs res;
  
  BDD_ENC_CHECK_INSTANCE(self);
  
  mask = BddEnc_get_state_input_vars_mask_bdd(self);
  res = BDD_STATES_INPUTS( bdd_and(self->dd, states_inputs, mask) );
  bdd_free(self->dd, mask);
  
  return res;
}



/**Function********************************************************************

  Synopsis           [Given a variable name, returns the mask of its encoding]

  Description        [Returns the mask that removes repetitions of leaves in
                      a variable encoding. Returned ADD is
                      referenced. Automatic reordering, if enabled, is
                      temporary disabled during this computation.]

  SideEffects        []

  SeeAlso            [BddEnc_get_var_encoding_mask,
                      bdd_enc_get_var_mask_add_recur]

******************************************************************************/
add_ptr BddEnc_get_var_name_mask(BddEnc_ptr self, node_ptr var_name)
{
  add_ptr var_encoding;
  add_ptr res;

  BDD_ENC_CHECK_INSTANCE(self);
  
  var_encoding = BddEnc_get_symbol_add(self, var_name);
  res = BddEnc_get_var_encoding_mask(self, var_encoding);
  add_free(self->dd, var_encoding);
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Given a variable encoding, returns the correponding mask]

  Description        [Returns the mask that removes repetitions of leaves in
                      a variable encoding. Returned ADD is
                      referenced. Automatic reordering, if enabled, is
                      temporary disabled during this computation.]

  SideEffects        []

  SeeAlso            [BddEnc_get_var_name_mask,
                      bdd_enc_get_var_mask_add_recur]

******************************************************************************/
add_ptr BddEnc_get_var_encoding_mask(BddEnc_ptr self, add_ptr var_encoding)
{
  dd_reorderingtype reord_type;
  boolean reordering_enabled;
  add_ptr var_cube; 
  add_ptr res;

  BDD_ENC_CHECK_INSTANCE(self);
  
  /* disables reordering, to prevent problems rising from the recursion call */
  reordering_enabled = (dd_reordering_status(self->dd, &reord_type) == 1);
  if (reordering_enabled) { dd_autodyn_disable(self->dd); }

  var_cube = add_support(self->dd, var_encoding);
  res = bdd_enc_get_var_mask_add_recur(self, var_encoding, var_cube);
  add_free(self->dd, var_cube);  

  /* re-enables reordering if it is required */
  if (reordering_enabled) { dd_autodyn_enable(self->dd,  reord_type); }    

  return res;
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Partial initializer. Call BddEnc_encode_vars to 
  complete]

  Description        [Partial initializer. Call BddEnc_encode_vars to 
  complete]

  SideEffects        []

  SeeAlso            [BddEnc_encode_vars]

******************************************************************************/
static void 
bdd_enc_init(BddEnc_ptr self, Encoding_ptr generic_encoding, DdManager* dd)
{
  int j; 
  
  self->senc = generic_encoding;
  self->dd = dd;

  self->group_begin_start = -1; /* an invalid index */

  /* the caches */
  self->cache = BddEncCache_create(self);

  /* all other members: */
  self->saved = false;
  self->enforce_constant = false;

  /* Lists: */
  self->state.state_vars_add_list = Nil;

  /* ADDs and BDDs */
  self->state.__input_variables_add = add_one(self->dd);
  self->state.__state_variables_add = add_one(self->dd);
  self->state.__next_state_variables_add = add_one(self->dd);  
  self->state.__input_variables_bdd = (bdd_ptr) NULL;
  self->state.__state_variables_bdd = (bdd_ptr) NULL;
  self->state.__next_state_variables_bdd = (bdd_ptr) NULL;

  self->saved_state.__input_variables_add = (add_ptr) NULL;
  self->saved_state.__state_variables_add = (add_ptr) NULL;
  self->saved_state.__next_state_variables_add = (add_ptr) NULL;  
  self->saved_state.__input_variables_bdd = (bdd_ptr) NULL;
  self->saved_state.__state_variables_bdd = (bdd_ptr) NULL;
  self->saved_state.__next_state_variables_bdd = (bdd_ptr) NULL;  

  /* masks: */
  self->__state_vars_mask_add = (add_ptr) NULL;
  self->__input_vars_mask_add = (add_ptr) NULL;
  self->__state_input_vars_mask_add = (add_ptr) NULL;
  self->__state_vars_mask_bdd = (bdd_ptr) NULL;
  self->__input_vars_mask_bdd = (bdd_ptr) NULL;
  self->__state_input_vars_mask_bdd = (bdd_ptr) NULL;
  

  /* arrays: */
  for (j = 0; j < MAX_VAR_INDEX; ++j) {
    self->variable_names[j] = Nil;

    self->current2next[j] = j;
    self->next2current[j] = j;
    self->minterm_input_vars[j] = (bdd_ptr) NULL;
    self->minterm_state_vars[j] = (bdd_ptr) NULL;
    self->minterm_next_state_vars[j] = (bdd_ptr) NULL;
    self->minterm_state_input_vars[j] = (bdd_ptr) NULL;
  }

  /* counters: */
  self->state.minterm_input_vars_dim = 0;
  self->saved_state.minterm_input_vars_dim = 0;
  self->state.minterm_state_vars_dim = 0;
  self->state.minterm_next_state_vars_dim = 0;
  self->saved_state.minterm_state_vars_dim = 0;
  self->saved_state.minterm_next_state_vars_dim = 0;
  self->state.minterm_state_input_vars_dim = 0;
  self->saved_state.minterm_state_input_vars_dim = 0;

  self->state.num_of_state_vars = 0;
  self->state.num_of_input_vars = 0;

  self->group_tree = (MtrNode*) NULL;

  /* other fields for print_bdd: */
  self->print_stack = Nil;
}


/**Function********************************************************************

  Synopsis           [Private method called by the destructor]

  Description        [Called by the destructor]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void bdd_enc_deinit(BddEnc_ptr self)
{
  /* Frees ADDs and BDDs */
  BDD_ENC_CLEAN_ADD_MEMBER(self, state.__input_variables_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, state.__state_variables_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, state.__next_state_variables_add);

  BDD_ENC_CLEAN_ADD_MEMBER(self, saved_state.__input_variables_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, saved_state.__state_variables_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, saved_state.__next_state_variables_add);

  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__input_variables_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__state_variables_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__next_state_variables_bdd);

  BDD_ENC_CLEAN_BDD_MEMBER(self, saved_state.__input_variables_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, saved_state.__state_variables_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, saved_state.__next_state_variables_bdd);


  /* masks: */
  BDD_ENC_CLEAN_ADD_MEMBER(self, __state_vars_mask_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, __input_vars_mask_add);
  BDD_ENC_CLEAN_ADD_MEMBER(self, __state_input_vars_mask_add);
  BDD_ENC_CLEAN_BDD_MEMBER(self, __state_vars_mask_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, __input_vars_mask_bdd);
  BDD_ENC_CLEAN_BDD_MEMBER(self, __state_input_vars_mask_bdd);


  /* print hashes: there must be no pending printing op */
  nusmv_assert(self->print_stack == Nil);
  
  /* clean arrays: */
  {
    int j;
    for (j = 0; j < MAX_VAR_INDEX; ++j) {
      BDD_ENC_CLEAN_BDD_MEMBER(self, minterm_input_vars[j]);
      BDD_ENC_CLEAN_BDD_MEMBER(self, minterm_state_vars[j]);
      BDD_ENC_CLEAN_BDD_MEMBER(self, minterm_next_state_vars[j]);
      BDD_ENC_CLEAN_BDD_MEMBER(self, minterm_state_input_vars[j]);
    }
  }  

  /* clean lists of ADDs/BDDs: */
  {
    node_ptr iter = self->state.state_vars_add_list;
    while (iter != Nil) {
      add_ptr add = (add_ptr) car(iter);
      if (add != (add_ptr) NULL) { add_free(self->dd, add); }
      iter = cdr(iter);
    }
  }

  /* groups: */
  nusmv_assert(self->group_tree == (MtrNode*) NULL);

  /* keep this clean up at the end: */
  BddEncCache_destroy(self->cache);
  self->dd = (DdManager*) NULL;
}


/**Function********************************************************************

  Synopsis           [Adds an input variable to the array necessary to 
  extract minterms from a BDD.]

  Description        [Adds an input variable to the array necessary to 
  extract minterms from a BDD. Take into account of the members 
  state.num_of_input_vars and state.num_of_state_vars in order to add the new 
  minterm]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void bdd_enc_add_input_var_to_minterm_vars(BddEnc_ptr self)
{ 
  int num_of_vars = self->state.num_of_input_vars 
    + self->state.num_of_state_vars;
  bdd_ptr bdd;

  if (num_of_vars >= MAX_VAR_INDEX) { error_too_many_vars(); }
  nusmv_assert( self->minterm_input_vars[self->state.minterm_input_vars_dim] 
		== (bdd_ptr) NULL ); /* index not already used */
  nusmv_assert( 
     self->minterm_state_input_vars[self->state.minterm_state_input_vars_dim] 
		== (bdd_ptr) NULL ); /* index not already used */

  /* vars indices start from 1 */
  bdd = bdd_new_var_with_index(self->dd, num_of_vars);
  self->minterm_input_vars[self->state.minterm_input_vars_dim] = bdd_dup(bdd);

  self->minterm_state_input_vars[self->state.minterm_state_input_vars_dim] = 
    bdd_dup(bdd);

  bdd_free(self->dd, bdd);
    
  self->state.minterm_input_vars_dim += 1;
  self->state.minterm_state_input_vars_dim += 1;
}


/**Function********************************************************************

  Synopsis           [Adds an input variable to the array necessary to 
  extract minterms from a BDD.]

  Description        [Adds an input variable to the array necessary to 
  extract minterms from a BDD. Take into account of the members 
  state.num_of_input_vars and state.num_of_state_vars in order to add the new 
  minterm]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void bdd_enc_add_state_var_to_minterm_vars(BddEnc_ptr self)
{ 
  int num_of_vars = self->state.num_of_input_vars 
    + self->state.num_of_state_vars;
  bdd_ptr bdd;

  if (num_of_vars >= MAX_VAR_INDEX) { error_too_many_vars(); }
  nusmv_assert( self->minterm_state_vars[self->state.minterm_state_vars_dim] 
		== (bdd_ptr) NULL ); /* index not already used */
  nusmv_assert( 
     self->minterm_state_input_vars[self->state.minterm_state_input_vars_dim] 
		== (bdd_ptr) NULL ); /* index not already used */

  /* vars indices start from 1 */
  bdd = bdd_new_var_with_index(self->dd, num_of_vars);
  self->minterm_state_vars[self->state.minterm_state_vars_dim] = bdd_dup(bdd);
  
  self->minterm_state_input_vars[self->state.minterm_state_input_vars_dim] = 
    bdd_dup(bdd);
  
  bdd_free(self->dd, bdd);
    
  self->state.minterm_state_vars_dim += 1;
  self->state.minterm_state_input_vars_dim += 1;
}


/**Function********************************************************************

  Synopsis           [Adds an input variable to the array necessary to 
  extract minterms from a BDD.]

  Description        [Adds an input variable to the array necessary to 
  extract minterms from a BDD. Take into account of the members 
  state.num_of_input_vars and state.num_of_state_vars in order to add the new 
  minterm]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void bdd_enc_add_next_state_var_to_minterm_vars(BddEnc_ptr self)
{ 
  bdd_ptr bdd;
  int num_of_vars = self->state.num_of_input_vars 
    + self->state.num_of_state_vars;

  if (num_of_vars >= MAX_VAR_INDEX) { error_too_many_vars(); }
  nusmv_assert( 
       self->minterm_next_state_vars[self->state.minterm_next_state_vars_dim] 
       == (bdd_ptr) NULL ); /* index not already used */

  /* vars indices start from 1 */
  bdd = bdd_new_var_with_index(self->dd, num_of_vars);

  self->minterm_next_state_vars[self->state.minterm_next_state_vars_dim] = 
    bdd_dup(bdd);

  bdd_free(self->dd, bdd);
    
  self->state.minterm_next_state_vars_dim += 1;
}


/**Function********************************************************************

  Synopsis           [Adds a new boolean variable to the DD package.]

  Description        [Adds a new boolean variable to the DD manager.
  If it is a state variable, then also the "next" variable is created,
  and the data structure to perform substitution of "current" and "next"
  variables is filled in.]

  SideEffects        []

  SeeAlso            [bdd_enc_add_state_var]

******************************************************************************/
static void bdd_enc_add_state_var(BddEnc_ptr self, node_ptr name, boolean flag)
{
  add_ptr add;
  int num_of_vars;
  int reord_status;
  dd_reorderingtype rt;

  /* first, current: */
  self->state.num_of_state_vars += 1;
  num_of_vars = self->state.num_of_input_vars + self->state.num_of_state_vars;

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "  BDD variable %d, ", num_of_vars);
  }

  /* If dynamic reordering is enabled, it is temporarily disabled
     since otherwise the grouping mechanism may fail due to
     overlapping of groups  */
  reord_status = dd_reordering_status(self->dd, &rt);
  if (reord_status == 1) { dd_autodyn_disable(self->dd); }

  add = add_new_var_with_index(self->dd, num_of_vars);
  bdd_enc_accumulate_state_variables_cube(self, add);
  BddEncCache_new_var(self->cache, name, add);
  add_free(self->dd, add);

  /* minterms for current */
  if (flag) { bdd_enc_add_state_var_to_minterm_vars(self); }

  /* associates dd index and name */
  self->variable_names[num_of_vars] = name; /* current */

  /* then, next: */
  self->state.num_of_state_vars += 1;
  num_of_vars += 1;

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, " next BDD variable %d\n", num_of_vars);
  }

  add = add_new_var_with_index(self->dd, num_of_vars);
  bdd_enc_accumulate_next_state_variables_cube(self, add);
  BddEncCache_new_var(self->cache, find_node(NEXT, name, Nil), add);
  add_free(self->dd, add); 

  /* minterms for next: */
  if (flag) { bdd_enc_add_next_state_var_to_minterm_vars(self); }
 
  /* sets up index tables: */
  self->current2next[num_of_vars-1] = num_of_vars;
  self->next2current[num_of_vars] = num_of_vars - 1;

  /* associates dd index and name for next */
  self->variable_names[num_of_vars] = find_node(NEXT, name, Nil);

  /* blocks the current and next ADDs to stay closed even after
     reordering, but only if no group is being created. If a group
     creation is being in process (i.e. begin_group has been called)
     the group creation is delayed until end_group is called. */
  if (self->group_begin_start == -1) {
    dd_new_var_block(self->dd, num_of_vars - 1, 2); 
  }

  /* If dynamic reordering was enabled, then it is re-enabled */
  if (reord_status == 1) { dd_autodyn_enable(self->dd, rt); }
}


/**Function********************************************************************

  Synopsis           [Adds a new boolean variable to the DD package.]

  Description        [Adds a new boolean variable to the DD
  manager. This function is used to create the boolean variables
  needed to encode input variables.]

  SideEffects        []

  SeeAlso            [bdd_enc_add_state_var]

******************************************************************************/
static void bdd_enc_add_input_var(BddEnc_ptr self, node_ptr name, boolean flag)
{
  add_ptr add;
  int num_of_vars;
  
  /* allocates a new input var: */
  self->state.num_of_input_vars += 1;
  num_of_vars = self->state.num_of_input_vars + self->state.num_of_state_vars;
  if (num_of_vars >= MAX_VAR_INDEX) { error_too_many_vars(); }

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "  BDD input variable %d\n", num_of_vars);
  }

  /* dd encoding */
  add = add_new_var_with_index(self->dd, num_of_vars);
  bdd_enc_accumulate_input_variables_cube(self, add);
  BddEncCache_new_var(self->cache, name, add);
  add_free(self->dd, add);

  /* optional minterms: */
  if (flag) { bdd_enc_add_input_var_to_minterm_vars(self); }

  /* indices: */
  self->current2next[num_of_vars] = num_of_vars;
  self->next2current[num_of_vars] = num_of_vars;

  /* associates dd index and name */
  self->variable_names[num_of_vars] = name; /* current */
}


/**Function********************************************************************

  Synopsis           [Private method. Set internal members]

  Description        [Sets both members input_variables_add 
  and input_variables_bdd. Always use this to set the value of this member]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void bdd_enc_set_input_variables_add(BddEnc_ptr self, add_ptr add)
{
  BDD_ENC_CLEAN_ADD_MEMBER(self, state.__input_variables_add);
  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__input_variables_bdd);

  if (add != (add_ptr) NULL) {
    self->state.__input_variables_add = add_dup(add);
  }
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void 
bdd_enc_accumulate_input_variables_cube(BddEnc_ptr self, add_ptr add)
{
  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__input_variables_bdd);
  if (add != (add_ptr) NULL) { 
    if (self->state.__input_variables_add != (add_ptr) NULL) {
      add_and_accumulate(self->dd, &(self->state.__input_variables_add), add);
    }
    else { self->state.__input_variables_add = add_dup(add); }
  }
}


/**Function********************************************************************

  Synopsis           [Private method. Set internal members]

  Description        [Sets both members state_variables_add 
  and state_variables_bdd. Always use this to set the value of this member]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void bdd_enc_set_state_variables_add(BddEnc_ptr self, add_ptr add)
{
  BDD_ENC_CLEAN_ADD_MEMBER(self, state.__state_variables_add);
  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__state_variables_bdd);

  if (add != (add_ptr) NULL) {
    self->state.__state_variables_add = add_dup(add);
  }
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void 
bdd_enc_accumulate_state_variables_cube(BddEnc_ptr self, add_ptr add)
{
  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__state_variables_bdd);
  if (add != (add_ptr) NULL) { 
    if (self->state.__state_variables_add != (add_ptr) NULL) {
      add_and_accumulate(self->dd, &(self->state.__state_variables_add), add);
    }
    else { self->state.__state_variables_add = add_dup(add); }
  }
}


/**Function********************************************************************

  Synopsis           [Private method. Set internal members]

  Description        [Sets both members state_variables_add 
  and state_variables_bdd. Always use this to set the value of this member]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void bdd_enc_set_next_state_variables_add(BddEnc_ptr self, add_ptr add)
{
  BDD_ENC_CLEAN_ADD_MEMBER(self, state.__next_state_variables_add);
  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__next_state_variables_bdd);

  if (add != (add_ptr) NULL) {
    self->state.__next_state_variables_add = add_dup(add);
  }
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void 
bdd_enc_accumulate_next_state_variables_cube(BddEnc_ptr self, add_ptr add)
{
  BDD_ENC_CLEAN_BDD_MEMBER(self, state.__next_state_variables_bdd);
  if (add != (add_ptr) NULL) { 
    if (self->state.__next_state_variables_add != (add_ptr) NULL) {
      add_and_accumulate(self->dd, 
			 &(self->state.__next_state_variables_add), add);
    }
    else { self->state.__next_state_variables_add = add_dup(add); }
  }
}


/**Function********************************************************************

  Synopsis           [Private accessor]

  Description        [Use this to retrieve value stored into the 
  corresponding class member. Returned value is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static add_ptr bdd_enc_get_input_variables_add(const BddEnc_ptr self)
{
  add_ptr res;
  
  if (self->state.__input_variables_add != (add_ptr) NULL) {
    res = add_dup(self->state.__input_variables_add);
  }
  else res = (add_ptr) NULL;
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Private accessor]

  Description        [Use this to retrieve value stored into the 
  corresponding class member. Returned value is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static add_ptr bdd_enc_get_state_variables_add(const BddEnc_ptr self)
{
  add_ptr res;
  
  if (self->state.__state_variables_add != (add_ptr) NULL) {
    res = add_dup(self->state.__state_variables_add);
  }
  else res = (add_ptr) NULL;
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Private accessor]

  Description        [Use this to retrieve value stored into the 
  corresponding class member. Returned value is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static add_ptr bdd_enc_get_next_state_variables_add(const BddEnc_ptr self)
{
  add_ptr res;
  
  if (self->state.__next_state_variables_add != (add_ptr) NULL) {
    res = add_dup(self->state.__next_state_variables_add);
  }
  else res = (add_ptr) NULL;
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Given an expression the corresponding ADD is
  returned back.]

  Description        [This function takes an expression in input and
  gives as output the corresponding ADD.<p>
  This function if receives in input a domain variables, it returns as
  its evaluation the ADD representing its boolean encoding. It has as
  leaves the value associated to that path.<br>
  For instance consider the declaration:<br>
  <code>VAR x : 1..6;</code><br>
  it is encoded with three boolean variables as below:
  <pre>

              x1
              /\\ 
            1/  \\0
            /    \\
           /      \\
          x2       x2
         /\\        / \\
        /  \\       |  \\
      x3    \\      x3  \\  
     /  \\    \\    /  \\  \\ 
    /    \\   |   /    \\  \\
    1    5   3   2    6   4

  </pre>
  If the expression is complex, then it recursively apply to the
  operands, and then apply the operation to the operands, returning
  the resulting ADD. Returned ADD is referenced]

  SideEffects        []

  SeeAlso            [eval_recur]

******************************************************************************/
static add_ptr bdd_enc_eval(BddEnc_ptr self, Expr_ptr expr, node_ptr context)
{
  add_ptr res;
  node_ptr hash_entry = find_node(CONTEXT, context, expr);

  if (expr == Nil) return add_one(self->dd);

  res = BddEncCache_get_evaluation(self->cache, hash_entry);
  if ((res == (add_ptr) NULL) || (res == EVALUATING)) {
    int temp = yylineno;
    yylineno = node_get_lineno(expr);
    res = bdd_enc_eval_recur(self, expr, context);
    yylineno = temp;
    
    /* inserts the evaluated add in the cache */
    BddEncCache_set_evaluation(self->cache, hash_entry, res);
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Performs the recursive step of the <code>eval</code> function.]

  Description        [The expression <code>n</code> is recursively
  compiled in ADD:
  <ul>
  <li> If it is an <em>ATOM</em>:
      <ul>
      <li> If it is a program parameter, then its actual name is
           extracted from the parameter hash, and it is evaluated. The
           result of evaluation is returned.</li> 
      <li> If it is a constant, then the corresponding ADD is returned
           back.</li>
      </ul></li>
  <li> If it is a binary operator, then the operands are evaluated,
       and then the binary operator is applied to the operands.</li>
  </ul>
  ]

  SideEffects        []

  SeeAlso            [eval get_definition]

******************************************************************************/
static add_ptr 
bdd_enc_eval_recur_case_atom(BddEnc_ptr self, Expr_ptr expr, node_ptr ctx)

{
  node_ptr dot_name  = find_node(DOT, ctx, find_atom(expr));
  node_ptr param = lookup_param_hash(dot_name);
  add_ptr constant = BddEncCache_lookup_constant(self->cache, find_atom(expr));
  boolean is_symbol = Encoding_is_symbol_declared(self->senc, dot_name);
 
  if (constant == (add_ptr) NULL) {
    /* try with the flattend version: */
    constant = BddEncCache_lookup_constant(self->cache, dot_name);
  }

  /* if the atom belongs to any combination of parameters, symbols
     or constants an error occurred. */
  if ( ((param != (node_ptr) NULL) && is_symbol) 
       || ((constant != (add_ptr) NULL)  && is_symbol) 
       || ((param != (node_ptr) NULL) && (constant != (add_ptr) NULL)) ) {
    add_free(self->dd, constant);
    rpterr("atom \"%s\" is ambiguous", str_get_text(node_get_lstring(expr)));
  }
  
  /* the atom is a parameter */
  if (param != (node_ptr) NULL) {
    if (constant != (add_ptr) NULL) { add_free(self->dd, constant); }
    return bdd_enc_eval(self, param, ctx);
  }
  
  /* the atom is a constant */
  if (constant != (add_ptr) NULL) return constant; /*was already referenced*/
  else return bdd_enc_eval_recur_case_dot_array(self, expr, ctx);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static add_ptr
bdd_enc_eval_recur_case_dot_array(BddEnc_ptr self, Expr_ptr expr, node_ptr ctx)
{
  node_ptr name = CompileFlatten_resolve_name(expr, ctx);
  add_ptr constant = BddEnc_constant_to_add(self, name);
  
  if (constant == (add_ptr) NULL) {
    add_ptr res = BddEnc_get_symbol_add(self, name);
    if (res == (add_ptr) NULL) { error_undefined(name); }
    return res;
  }
  else {
    /* It is the name of a process, thus it has to be considered as
       a constant atom */
    return constant; /* already referenced */
  }
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static add_ptr bdd_enc_eval_recur(BddEnc_ptr self, Expr_ptr expr, node_ptr ctx)
{
  if (expr == Nil)  return add_one(self->dd);

  switch (node_get_type(expr)) {

  case ATOM:  
    return bdd_enc_eval_recur_case_atom(self, expr, ctx);
    
  case BIT:
    {
      node_ptr name = CompileFlatten_resolve_name(expr, ctx);
      add_ptr temp = BddEncCache_lookup_var(self->cache, name); 
      return temp;
    }

  case DOT:
  case ARRAY: 
    return bdd_enc_eval_recur_case_dot_array(self, expr, ctx);

  case CONTEXT: return bdd_enc_eval(self, cdr(expr), car(expr));

  case CONS:
  case AND:  return bdd_enc_binary_op(self, add_and, expr, 1, 1, 1, ctx);

  case OR:       return bdd_enc_binary_op(self, add_or, expr, 1, 1, 1, ctx);
  case NOT:      return bdd_enc_unary_op(self, add_not, expr, 1, 1, ctx);
  case IMPLIES:  return bdd_enc_binary_op(self, add_or, expr, 1, -1, 1, ctx);
  case IFF:      return bdd_enc_binary_op(self, add_xor, expr, -1, 1, 1, ctx);
  case XOR:      return bdd_enc_binary_op(self, add_xor, expr, 1, 1, 1, ctx);
  case XNOR:     return bdd_enc_binary_op(self, add_xor, expr, 1, 1, -1, ctx);

  case IFTHENELSE:
  case CASE:     
    return bdd_enc_if_then_else_op(self, expr, ctx);

  case EQUAL:    return bdd_enc_binary_op(self, add_equal, expr, 1, 1, 1, ctx);
  case NOTEQUAL: return bdd_enc_binary_op(self, add_equal, expr, -1, 1, 1, ctx);
  case PLUS:     return bdd_enc_binary_op(self, add_plus, expr, 1, 1, 1, ctx);
  case MINUS:    return bdd_enc_binary_op(self, add_minus, expr, 1, 1, 1, ctx);
  case TIMES:    return bdd_enc_binary_op(self, add_times, expr, 1, 1, 1, ctx);
  case DIVIDE:   return bdd_enc_binary_op(self, add_divide, expr, 1, 1, 1, ctx);
  case MOD:      return bdd_enc_binary_op(self, add_mod, expr, 1, 1, 1, ctx);
  case LT:       return bdd_enc_binary_op(self, add_lt, expr, 1, 1, 1, ctx);
  case GT:       return bdd_enc_binary_op(self, add_gt, expr, 1, 1, 1, ctx);
  case LE:       return bdd_enc_binary_op(self, add_gt, expr, -1, 1, 1, ctx);
  case GE:       return bdd_enc_binary_op(self, add_lt, expr, -1, 1, 1, ctx);
  case TRUEEXP:  return add_one(self->dd);
  case FALSEEXP: return add_zero(self->dd);
  case NUMBER:   return add_leaf(self->dd, find_atom(expr));

  case NEXT:     
    {
      add_ptr res, tmp;
      
      res = BddEncCache_lookup_var(self->cache, expr);
      if (res == (add_ptr) NULL) {
	tmp = bdd_enc_eval(self, car(expr), ctx);
	set_the_node(expr); 
	res = BddEnc_state_var_to_next_state_var_add(self, tmp);
	add_free(self->dd, tmp);
      }
      return res;
    }
 
  case EQDEF:
    {
      node_ptr t1, t2, r;

      switch (node_get_type(car(expr))) {
      case SMALLINIT: 
	/* we deal with statements of this kind: init(x) := init_expr */
	t1 = t2 = car(car(expr)); /* (ATOM x Nil) */
	break;

      case NEXT: /* we deal wit statements of this kind: next(x) := next_expr */
	t1 = car(expr); /* (NEXT (ATOM x Nil) Nil) */
	t2 = car(car(expr)); /* (ATOM x Nil) */
	break;

      default: /* we deal wit statements of this kind: x := simple_expr */
	t1 = t2 = car(expr); /* (ATOM x Nil) */
      }

      /* we contextualize "x" and we extract its informations */
      r = CompileFlatten_resolve_name(t2, ctx);
      if (! Encoding_is_symbol_declared(self->senc, r)) { 
	error_undefined(t2); 
      }
      if (! Encoding_is_symbol_var(self->senc, r)) { 
	error_redefining(t2); 
      }

      if (opt_verbose_level_gt(options, 4)) {
	inc_indent_size();
	indent_node(nusmv_stderr, "evaluating ", t1, ":\n");
      }

      /* We evaluate the right hand side of the assignment (lhs := rhs) */
      {
	node_ptr symb = Encoding_lookup_symbol(self->senc, r);
	add_ptr lhs, rhs;
	add_ptr res;
	
	rhs = bdd_enc_eval(self, cdr(expr), ctx);
        
	/* We check that the value of the rhs are admissible values for the lhs */
	Utils_set_data_for_range_check(t2, cdr(symb)); /* var and range */
	add_walkleaves(Utils_range_check, rhs);

        /* We perform set inclusion of the evaluation of the rhs with the lhs */
	lhs = bdd_enc_eval(self, t1, ctx);
	res = add_setin(self->dd, lhs, rhs);
	add_free(self->dd, lhs);
	add_free(self->dd, rhs);
	
	if (opt_verbose_level_gt(options, 4)) {
	  indent_node(nusmv_stderr, "size of ", t1, " = ");
	  fprintf(nusmv_stderr, "%d ADD nodes\n", add_size(self->dd, res));
	  dec_indent_size();
	}
	return res;
      }
    }

  case TWODOTS:
    {
      int dim1, dim2, i;
      node_ptr t = Nil;

      dim1 = BddEnc_eval_num(self, car(expr), ctx);
      dim2 = BddEnc_eval_num(self, cdr(expr), ctx);
      for (i = dim2; i >= dim1; --i) {
        t = find_node(CONS, find_node(NUMBER, (node_ptr) i, Nil), t);
      }

      if (t == Nil) { rpterr("empty range: %d..%d", dim1, dim2); }
      return add_leaf(self->dd, t);
    }
 
  case UNION: return bdd_enc_binary_op(self, add_union, expr, 1, 1, 1, ctx);
  case SETIN: return bdd_enc_binary_op(self, add_setin, expr, 1, 1, 1, ctx);

  default:
    internal_error("eval_recur: type = %d\n", node_get_type(expr));

  } /* switch */

  return (add_ptr) NULL;
}


/**Function********************************************************************

  Synopsis           [Applies unary operation.]

  Description        [Takes in input the expression <code>n</code> and a
  unary operation <code>op</code>. Evaluates <code>n</n> and applies to this
  partial result the unary operator <code>op</code>. The sign of the
  partial result and of the result depends respectively from the flag
  <code>argflag</code> and <code>resflag</code>.] 

  SideEffects        []

  SeeAlso            [eval, binary_op, ternary_op, quaternary_op]

******************************************************************************/
static add_ptr bdd_enc_unary_op(BddEnc_ptr self, ADDPFDA op, node_ptr n,
				int resflag, int argflag, node_ptr context)
{
  add_ptr tmp_1, tmp_2, res;
  add_ptr arg = bdd_enc_eval(self, car(n), context);

  set_the_node(n);

  /* compute and ref argument of operation according its sign */
  tmp_1 = BddEnc_eval_sign_add(self, arg, argflag);

  /* apply and ref the result of the application of "op" to previous arg. */
  tmp_2 = op(self->dd, tmp_1);

  /* compute and ref the result according to sign of the result */
  res = BddEnc_eval_sign_add(self, tmp_2, resflag);

  /* free temporary results */
  add_free(self->dd, arg);
  add_free(self->dd, tmp_1);
  add_free(self->dd, tmp_2);

  return res;
}


/**Function********************************************************************

  Synopsis           [Applies binary operation.]

  Description        [Takes in input the expression <code>n</code> and a
  binary operation <code>op</code>. Extracts from <code>n</n> the operands
  and evaluates them. The binary operator <code>op</code> is then applied
  to these partial results. The sign of the partial results and of the
  result depends respectively from the flags <code>argflag1</code>,
  <code>argflag2</code> and <code>resflag</code>.]

  SideEffects        []

  SeeAlso            [eval, unary_op, ternary_op, quaternary_op]

******************************************************************************/
static add_ptr bdd_enc_binary_op(BddEnc_ptr self, ADDPFDAA op, node_ptr n,
				 int resflag, int argflag1,int argflag2, 
				 node_ptr context)
{
  add_ptr tmp_1, tmp_2, tmp_3, res;
  add_ptr arg1 = bdd_enc_eval(self, car(n), context);
  add_ptr arg2 = bdd_enc_eval(self, cdr(n), context);

  set_the_node(n);

  tmp_1 = BddEnc_eval_sign_add(self, arg1, argflag1);
  tmp_2 = BddEnc_eval_sign_add(self, arg2, argflag2);
  tmp_3 = op(self->dd, tmp_1, tmp_2);
  res   = BddEnc_eval_sign_add(self, tmp_3, resflag);

  add_free(self->dd, arg1);
  add_free(self->dd, arg2);
  add_free(self->dd, tmp_1);
  add_free(self->dd, tmp_2);
  add_free(self->dd, tmp_3);
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Applies ternary operation.]

  Description        [Takes in input the expression <code>n</code> and a
  ternary operation <code>op</code>. Extracts from <code>n</n> the operands
  and evaluates them.<br>
  The second and third arguments have to evaluate to numbers. And
  <code>op</code> is a function that takes as input an ADD an two integers.
  The ternary operator <code>op</code> is then applied to these partial
  results. The sign of the partial result and of the result depends
  respectively from the flags <code>argflag</code> and <code>resflag</code>.]

  SideEffects        []

  SeeAlso            [eval, unary_op, binary_op, quaternary_op]

******************************************************************************/
static add_ptr bdd_enc_ternary_op(BddEnc_ptr self, ADDPFDAII op, node_ptr n,
				  int resflag, int argflag, node_ptr context)
{
  add_ptr tmp_1, tmp_2, res;
  add_ptr arg1 = bdd_enc_eval(self, car(n), context);
  int arg2 = BddEnc_eval_num(self, car(cdr(n)), context);
  int arg3 = BddEnc_eval_num(self, cdr(cdr(n)), context);

  set_the_node(n);

  tmp_1 = BddEnc_eval_sign_add(self, arg1, argflag);
  tmp_2 = op(self->dd, tmp_1, arg2, arg3);
  res   = BddEnc_eval_sign_add(self, tmp_2, resflag);

  add_free(self->dd, arg1);
  add_free(self->dd, tmp_1);
  add_free(self->dd, tmp_2);

  return res;
}


/**Function********************************************************************

  Synopsis           [Applies quaternary operation.]

  Description        [Takes in input the expression <code>n</code> and a
  quaternary operation <code>op</code>. Extracts from <code>n</n> the operands
  and evaluates them.<br>
  The third and fourth arguments have to evaluate to numbers. And
  <code>op</code> is a function that takes as input two ADD and two integers.
  The quaternary operator <code>op</code> is then applied to these partial
  results. The sign of the partial result and of the result depends
  respectively from the flags <code>argflag1</code>, <code>argflag2</code> and
  <code>resflag</code>.]

  SideEffects        []

  SeeAlso            [eval, unary_op, binary_op, ternary_op]

******************************************************************************/
static add_ptr 
bdd_enc_quaternary_op(BddEnc_ptr self, ADDPFDAAII op, node_ptr n,
		      int resflag, int argflag1, int argflag2, 
		      node_ptr context)
{
  add_ptr tmp_1, tmp_2, tmp_3;
  add_ptr res = (add_ptr) NULL;
  add_ptr arg1 = bdd_enc_eval(self, car(car(n)), context);
  add_ptr arg2 = bdd_enc_eval(self, cdr(car(n)), context);
  int arg3 = BddEnc_eval_num(self, car(cdr(n)), context);
  int arg4 = BddEnc_eval_num(self, cdr(cdr(n)), context);

  set_the_node(n);

  tmp_1 = BddEnc_eval_sign_add(self, arg1, argflag1);
  tmp_2 = BddEnc_eval_sign_add(self, arg2, argflag1);
  tmp_3 = op(self->dd, tmp_1, tmp_2, arg3, arg4);
  res = BddEnc_eval_sign_add(self, res, resflag);

  add_free(self->dd, arg1);
  add_free(self->dd, arg2);
  add_free(self->dd, tmp_1);
  add_free(self->dd, tmp_2);
  add_free(self->dd, tmp_3);

  return res;
}


/**Function********************************************************************

  Synopsis           [Evaluates if_then_else expressions returning
  the ADD representing <em>IF ifarg THEN thenarg ELSE elsarg</em>.]

  Description        [Evaluates if_then_else expressions returning the
  ADD representing <em>IF ifarg THEN thenarg ELSE elsarg</em>, where
  <code>ifarg</code>, <code>thenarg</code>, <code>elsearg</code> are the ADD
  obtained by evaluating <code>ifexp</code>, <code>thenexp</code>,
  <code>elseexp</code> respectively in context <code>context</code>. The
  resulting ADD is saved in the ADD hash before returning it to the
  calling function.]

  SideEffects        []

  SeeAlso            [add_ifthenelse]

******************************************************************************/
static add_ptr bdd_enc_if_then_else_op(BddEnc_ptr self, node_ptr node, 
				       node_ptr context)
{
  add_ptr res;
  add_ptr ifarg = bdd_enc_eval(self, car(car(node)), context);
  add_ptr thenarg = bdd_enc_eval(self, cdr(car(node)), context);
  add_ptr elsearg = bdd_enc_eval(self, cdr(node), context);

  res = add_ifthenelse(self->dd, ifarg, thenarg, elsearg);
  
  /* free temporary results */
  add_free(self->dd, ifarg);
  add_free(self->dd, thenarg);
  add_free(self->dd, elsearg);

  return res;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void bdd_enc_pre_encode(BddEnc_ptr self)
{
  int i;
  NodeList_ptr list;
  ListIter_ptr iter;
  GroupSet_ptr groups[2];
  
  if (opt_verbose_level_gt(options, 4)) {
    inc_indent_size();
  }

  /* pre-encodes all constants: */
  list = Encoding_get_constants_list(self->senc);
  iter = NodeList_get_first_iter(list);
  while (! ListIter_is_end(iter)) {
    node_ptr constant = NodeList_get_elem_at(list, iter);
    add_ptr add_constant; 

    if (opt_verbose_level_gt(options, 4)) {
      fprintf(nusmv_stdout, "<BddEnc> pre-allocating constant ");
      print_node(nusmv_stdout, constant);
      fprintf(nusmv_stdout, "\n");
    }

    add_constant = add_leaf(self->dd, constant);
    BddEncCache_new_constant(self->cache, constant, add_constant);
    add_free(self->dd, add_constant);
    iter = ListIter_get_next(iter);
  }

  if (opt_verbose_level_gt(options, 4)) {
    dec_indent_size();
  }


  /* asks for sorting bool vars if needed: */
  Encoding_sort_bool_vars(self->senc, get_input_order_file(options));

  /* Iterates on bool vars (in separated groups), allocating and
     encoding state & input variables: */

  if (opt_verbose_level_gt(options, 4)) {
    inc_indent_size();
  }
  
  groups[0] = Encoding_get_bool_input_vars_groups(self->senc);
  groups[1] = Encoding_get_bool_state_vars_groups(self->senc);

  for (i = 0; i < sizeof(groups)/sizeof(groups[0]); ++i) {
    ListIter_ptr gr_iter = GroupSet_get_first_iter(groups[i]);
    while (! ListIter_is_end(gr_iter)) {

      bdd_enc_begin_group(self);
      
      list = GroupSet_get_group(groups[i], gr_iter);
      iter = NodeList_get_first_iter(list);

      if (opt_verbose_level_gt(options, 4)) {
	inc_indent_size();
      }

      while (! ListIter_is_end(iter)) {
	node_ptr name = NodeList_get_elem_at(list, iter);

	if (opt_verbose_level_gt(options, 4)) {
	  fprintf(nusmv_stdout, "<BddEnc> pre-allocating variable ");
	  print_node(nusmv_stdout, name);
	  fprintf(nusmv_stdout, "\n");
	}

	if (Encoding_is_symbol_input_var(self->senc, name)) {
	  bdd_enc_add_input_var(self, name, true);      
	}
	else if (Encoding_is_symbol_state_var(self->senc, name)) {
	  bdd_enc_add_state_var(self, name, true);      
	}
	
	iter = ListIter_get_next(iter);
      } /* loop on vars of a group */

      if (opt_verbose_level_gt(options, 4)) {
	dec_indent_size();
      }

      bdd_enc_end_group(self);

      gr_iter = ListIter_get_next(gr_iter);
    } /* loop on groups */
  } /* for loop */

  if (opt_verbose_level_gt(options, 4)) {
    dec_indent_size();
  }

	
}


/**Function********************************************************************

  Synopsis           [Call *before* adding one or more new variables. 
  Then, call end_group ]

  Description        [The dynamic reordering, if enabled, will be   
  temporary disabled, until end_group is called]

  SideEffects        []

  SeeAlso            [bdd_enc_end_group]

******************************************************************************/
static void bdd_enc_begin_group(BddEnc_ptr self)
{
  nusmv_assert(self->group_begin_start == -1); /* no multiple begin */
  
  /* blocks dynamic reordering, if enabled */
  self->dyn_reord_flag = dd_reordering_status(self->dd,
					      &(self->dyn_reord_type)); 
  if (self->dyn_reord_flag == 1) { dd_autodyn_disable(self->dd); }

  /* stores the index of variable will be allocated. 1 is added since
     variables indices start from 1 */
  self->group_begin_start = self->state.num_of_input_vars + 
    self->state.num_of_state_vars + 1;
}


/**Function********************************************************************

  Synopsis           [Call to create a variables group]

  Description        [All variables added between the calls to 
  begin_group and this method will be grouped. If only one var has been 
  added, no group will be created. 
  The dynamic reordering, if was enabled before calling begin_group, is 
  re-enabled.
  Returns the number of vars really blocked]

  SideEffects        []

  SeeAlso            [bdd_enc_begin_group]

******************************************************************************/
static int bdd_enc_end_group(BddEnc_ptr self)
{
  int vars; 
  
  nusmv_assert(self->group_begin_start >= 0); /* begin already called */

  /* number of vars to block. 1 is added since variables indices start
     from 1 */
  vars = self->state.num_of_input_vars + self->state.num_of_state_vars -
    self->group_begin_start + 1;

  /* blocks if required: */
  if (vars > 1) {
    dd_new_var_block(self->dd, self->group_begin_start, vars);
  } 
  else vars = 0;

  if (vars > 0) {
    if (opt_verbose_level_gt(options, 5)) {
      fprintf(nusmv_stdout, 
	      "<BddEnc> Created a group of %d variables, at index %d\n\n", 
	      vars, self->group_begin_start);
    }
  }

  /* reset index, to enable next call to begin_group */
  self->group_begin_start = -1;

  /* re-enable dynamic reordering, if it was enabled */
  if (self->dyn_reord_flag == 1) { 
    dd_autodyn_enable(self->dd, self->dyn_reord_type); 
  }

  return vars;
}


/**Function********************************************************************

  Synopsis           [Private service]

  Description        [Used when destroying NodeList containing BDDs]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static assoc_retval hash_bdd_free(char* key, char* data, char* arg)
{
  bdd_ptr element = (bdd_ptr) data;

  if (element != (bdd_ptr) NULL) {
    bdd_free((DdManager*) arg, element);    
  }
  return ASSOC_DELETE;  
}


/**Function********************************************************************

  Synopsis           [Private service]

  Description        [Used when destroying node list containing node_ptr]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static assoc_retval hash_node_free(char* key, char* data, char* arg)
{
  node_ptr element = (node_ptr) data;

  if (element != Nil) { free_node(element); }
  return ASSOC_DELETE;  
}


/**Function********************************************************************

  Synopsis           [Calculates the mask of the var encoding passed as
  argument]

  Description        [Private method that recursively calculates the mask of
    the var encoding passed as argument. Passed cube is the cube of
    the encoding, used while visiting the encoding, in order to find
    indexes of sub-variable. Returned add is referenced, and the
    automatic reordering *must* be disabled before calling this
    method. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static add_ptr 
bdd_enc_get_var_mask_add_recur(BddEnc_ptr self, 
			       add_ptr var_encoding, add_ptr cube) 
{
  add_ptr res; 

  if (add_isleaf(cube))  {
    /* base step: visit is over, exits: */
    nusmv_assert(add_isleaf(var_encoding));
    res = add_one(self->dd);
  }
  else {
    /* inductive step, evaluates whether it is a constant or a var: */

    if (add_isleaf(var_encoding)) {
      /* It is a constant: fills the remaining empty nodes */
      add_ptr t; 
      add_ptr zero = add_zero(self->dd);

      t = bdd_enc_get_var_mask_add_recur(self, var_encoding, 
					 add_then(self->dd, cube));
      nusmv_assert(t != (add_ptr) NULL);

      res = add_build(self->dd, add_index(self->dd, cube), t, zero);
      nusmv_assert(res != (add_ptr) NULL);
      
      add_free(self->dd, t);
      add_free(self->dd, zero); 
    }
    else {
      /* it is a variable, keep visiting the dag, searching for gaps
	 to fill */
      add_ptr t, e;

      t =  bdd_enc_get_var_mask_add_recur(self, 
					  add_then(self->dd, var_encoding), 
					  add_then(self->dd, cube));
      nusmv_assert(t != (add_ptr) NULL);

      e =  bdd_enc_get_var_mask_add_recur(self, 
					  add_else(self->dd, var_encoding), 
					  add_then(self->dd, cube));
      nusmv_assert(e != (add_ptr) NULL);
      
      res = add_build(self->dd, add_index(self->dd, cube), t, e);
      nusmv_assert(res != (add_ptr) NULL);

      add_free(self->dd, e);
      add_free(self->dd, t);
    }
  }

  return res;  
}
				
				
/**Function********************************************************************

  Synopsis    [Given a list of variables, it returns the corresponding
  mask]

  Description        [Private service used by higher level mask-related 
  methods]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static add_ptr bdd_enc_get_vars_list_mask(BddEnc_ptr self, NodeList_ptr vars)
{
  ListIter_ptr iter; 
  add_ptr mask; 

  mask = add_one(self->dd);  
  
  iter = NodeList_get_first_iter(vars);
  while (!ListIter_is_end(iter)) {
    node_ptr var;
    add_ptr var_mask_add;

    var = NodeList_get_elem_at(vars, iter);
    nusmv_assert(Encoding_is_symbol_var(self->senc, var));
    
    var_mask_add = BddEnc_get_var_name_mask(self, var);
    add_and_accumulate(self->dd, &mask, var_mask_add);
    add_free(self->dd, var_mask_add);
    
    iter = ListIter_get_next(iter);
  }

  return mask;
}

