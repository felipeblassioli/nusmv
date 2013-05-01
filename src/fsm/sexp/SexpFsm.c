/**CFile*****************************************************************

  FileName    [SexpFsm.c]

  PackageName [fsm.sexp]

  Synopsis    [The SexpFsm implementation]

  Description [This file contains the implementation of a wrapper of (a part 
               of) the SexpFsm API onto the NuSMV code structure.
               Only functions needed by the BddFsm are provided.]

  SeeAlso     [SexpFsm.h]

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

#include "SexpFsm.h"
#include "sexpInt.h"

#include "utils/assoc.h"
#include "parser/symbols.h"

/* there are still some variables to be accessed there: */
#include "compile/compile.h" 

static char rcsid[] UTIL_UNUSED = "$Id: SexpFsm.c,v 1.1.2.8 2004/05/27 12:02:38 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Type************************************************************************

  Synopsis [A fsm for a single variable. It is represented as a triple
  of Expr_ptr ]

  Description [Private structure, internally used]

  SeeAlso     []

******************************************************************************/
typedef node_ptr VarFsm_ptr;
#define VAR_FSM(x)  ((VarFsm_ptr) x)


/**Type************************************************************************

  Synopsis    [Sexp FSM, the type exported by this package]

  Description [Allows sharing of structures between derived fsm. You
  can derive a fsm from an existing one, by copying it, or by
  converting it from scalar to boolean. In general ay operation which
  takes one fsm and returns a different fsm is a derivating operation. 
  A set of FSMs which share the same structures, is called "family".]

  SeeAlso     []

******************************************************************************/
typedef struct SexpFsm_TAG {
  int* family_counter; 

  VarSet_ptr vars_list;
  hash_ptr hash_var_fsm; 
  
  Expr_ptr init;
  Expr_ptr invar;
  Expr_ptr trans;
  Expr_ptr input;

  node_ptr justice;    /* list of expressions */
  node_ptr compassion; /* list of couple of expressions */

  boolean  is_booleanized; /* is fsm booleanized? */

} SexpFsm;



/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define NODE_PTR(x)  ((node_ptr) x)  /* upcast from VarFsm_ptr */


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void sexp_fsm_create_shared_structures ARGS((SexpFsm_ptr self));
static void sexp_fsm_try_to_destroy_shared_structures ARGS((SexpFsm_ptr self));
static void sexp_fsm_copy_shared_structures ARGS((const SexpFsm_ptr self, 
						  SexpFsm_ptr copy));

static void 
sexp_fsm_hash_var_fsm_init ARGS((SexpFsm_ptr self, VarSet_ptr vars_list));

static void sexp_fsm_init ARGS((SexpFsm_ptr self, 
				VarSet_ptr vars_list, 
				node_ptr justice,
				node_ptr compassion, 
				const boolean booleanize));


static Expr_ptr sexp_fsm_simplify_expr ARGS((hash_ptr hash, Expr_ptr expr, 
					     const int group, 
					     const boolean booleanize));
			    
static hash_ptr simplifier_hash_create ARGS((void));
static void simplifier_hash_destroy ARGS((hash_ptr hash));
static void simplifier_hash_add_expr ARGS((hash_ptr hash, 
					   Expr_ptr expr, const int group));
static boolean simplifier_hash_query_expr ARGS((hash_ptr hash, Expr_ptr expr, 
						const int group));


static void sexp_fsm_hash_var_fsm_create ARGS((SexpFsm_ptr self));
static void sexp_fsm_hash_var_fsm_destroy ARGS((SexpFsm_ptr self));
static assoc_retval 
sexp_fsm_callback_var_fsm_free ARGS((char *key, char *data, char * arg));
static VarFsm_ptr 
sexp_fsm_hash_var_fsm_lookup_var ARGS((SexpFsm_ptr self, node_ptr var));
static void 
sexp_fsm_hash_var_fsm_insert_var ARGS((SexpFsm_ptr self, 
				       node_ptr var, VarFsm_ptr varfsm));


static VarFsm_ptr var_fsm_create ARGS((Expr_ptr init, Expr_ptr invar, 
				       Expr_ptr next));

static void var_fsm_destroy ARGS((VarFsm_ptr self));
static Expr_ptr var_fsm_get_init ARGS((VarFsm_ptr self));
static Expr_ptr var_fsm_get_invar ARGS((VarFsm_ptr self));
static Expr_ptr var_fsm_get_next ARGS((VarFsm_ptr self));
static Expr_ptr var_fsm_get_input ARGS((VarFsm_ptr self));



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Costructor for a scalar and boolean sexp fsm]

  Description        []

  SideEffects        []

******************************************************************************/
SexpFsm_ptr SexpFsm_create(VarSet_ptr vars_list, 
			   node_ptr justice, node_ptr compassion, 
			   const enum SexpFsmType type)
{
  SexpFsm_ptr self;

  /* allocation: */
  self = ALLOC(SexpFsm, 1);
  SEXP_FSM_CHECK_INSTANCE(self);

  self->hash_var_fsm = (hash_ptr) NULL;
  sexp_fsm_create_shared_structures(self);

  /* initialization: */
  sexp_fsm_hash_var_fsm_init(self, vars_list);

  sexp_fsm_init(self, vars_list, justice, compassion, 
		type == SEXP_FSM_TYPE_BOOLEAN);
 
  return self;
}



/**Function********************************************************************

  Synopsis           [Costructor for a scalar and boolean sexp fsm]

  Description        [This method is temporary exported to be used 
  from the LTL side, to compile the table fsm. 
  Warning: the variable FSMs will be not constructed, so the user cannot 
  use methods to obtain the single variable fsm information. 
  ]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr SexpFsm_create_from_members(Expr_ptr init, 
					Expr_ptr invar, 
					Expr_ptr trans, 
					Expr_ptr input, 
					node_ptr justice, node_ptr compassion, 
					const enum SexpFsmType type)
{
  SexpFsm_ptr self;

  /* allocation: */
  self = ALLOC(SexpFsm, 1);
  SEXP_FSM_CHECK_INSTANCE(self);

  self->hash_var_fsm = (hash_ptr) NULL;
  sexp_fsm_create_shared_structures(self);

  
  /* inits all private members: */
  self->vars_list = VAR_SET(NULL);

  self->init       = init;
  self->invar      = invar;
  self->trans      = trans;
  self->input      = input;
  self->justice    = justice;
  self->compassion = compassion;
  self->is_booleanized = (type == SEXP_FSM_TYPE_BOOLEAN); 

  return self;
}



/**Function********************************************************************

  Synopsis           [Copy costructor]

  Description        []

  SideEffects        []

******************************************************************************/
SexpFsm_ptr SexpFsm_copy(const SexpFsm_ptr self)
{
  SexpFsm_ptr copy;

  SEXP_FSM_CHECK_INSTANCE(self);

  copy = ALLOC(SexpFsm, 1);
  SEXP_FSM_CHECK_INSTANCE(copy);

  copy->vars_list  = self->vars_list;
  copy->init       = self->init;
  copy->invar      = self->invar;
  copy->trans      = self->trans;
  copy->input      = self->input;
  copy->justice    = self->justice;
  copy->compassion = self->compassion;
  copy->is_booleanized = self->is_booleanized;
  
  sexp_fsm_copy_shared_structures(self, copy);

  return copy;
}


/**Function********************************************************************

  Synopsis           [Destructor]

  Description        []

  SideEffects        []

******************************************************************************/
void SexpFsm_destroy(SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);

  /* ------------------------------------------- */
  /* Free here all not-family structures         */
  /* ------------------------------------------- */    
  
  /* ------------------------------------------- */    

  sexp_fsm_try_to_destroy_shared_structures(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Construct a boolean fsm from a scalar one]

  Description        [Returns a copy if self is already booleanized]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr SexpFsm_scalar_to_boolean(const SexpFsm_ptr self)
{
  SexpFsm_ptr result;
  
  SEXP_FSM_CHECK_INSTANCE(self);

  if (SexpFsm_is_boolean(self)) {
    result = SexpFsm_copy(self);
  }
  else {
    result = ALLOC(SexpFsm, 1);
    SEXP_FSM_CHECK_INSTANCE(result);
    result->hash_var_fsm = (hash_ptr) NULL;

    sexp_fsm_copy_shared_structures(self, result);

    sexp_fsm_init(result, self->vars_list, 
		  self->justice, self->compassion, true);
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Returns true if self if a booleanized fsm, false if 
  it is scalar]

  Description        []

  SideEffects        []

******************************************************************************/
boolean SexpFsm_is_boolean(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->is_booleanized;
}


/**Function********************************************************************

  Synopsis           [Returns an Expr that collects init states for all 
  variables handled by self]

  Description        []

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_init(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->init;
}


/**Function********************************************************************

  Synopsis           [Returns an Expr that collects invar states for all 
  variables handled by self]

  Description        []

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_invar(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->invar;
}


/**Function********************************************************************

  Synopsis           [Returns an Expr that collects all next states for all 
  variables handled by self]

  Description        []

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_trans(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->trans;
}


/**Function********************************************************************

  Synopsis           [Returns an Expr that collects all input states for all 
  variables handled by self]

  Description        []

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_input(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  /* Currently no constraints over input are allowed, thus we return
     true to inidicate this. */
  return self->input;
}


/**Function********************************************************************

  Synopsis      [ Gets the sexp expression defining the initial state for
                  the variable "v". ]

  Description   [ Gets the sexp expression defining the initial state for
                  the variable "v". ]

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_var_init(const SexpFsm_ptr self, node_ptr var_name)
{
  VarFsm_ptr var_fsm;
  SEXP_FSM_CHECK_INSTANCE(self);

  var_fsm = find_assoc(self->hash_var_fsm, var_name);
  return var_fsm_get_init(var_fsm);
}


/**Function********************************************************************

  Synopsis      [ Gets the sexp expression defining the state constraints
                  for the variable "v". ]

  Description   [ Gets the sexp expression defining the state constraints
                  for the variable "v". ]

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_var_invar(const SexpFsm_ptr self, node_ptr var_name)
{
  VarFsm_ptr var_fsm;

  SEXP_FSM_CHECK_INSTANCE(self);
  var_fsm = find_assoc(self->hash_var_fsm, var_name);
  return var_fsm_get_invar(var_fsm);
}


/**Function********************************************************************

  Synopsis      [ Gets the sexp expression defining the transition relation
                  for the variable "v". ]

  Description   [ Gets the sexp expression defining the transition relation
                  for the variable "v". ]

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_var_trans(const SexpFsm_ptr self, node_ptr var_name)
{
  VarFsm_ptr var_fsm;

  SEXP_FSM_CHECK_INSTANCE(self);
  var_fsm = find_assoc(self->hash_var_fsm, var_name);
  return var_fsm_get_next(var_fsm);
}


/**Function********************************************************************

  Synopsis      [ Gets the sexp expression defining the input relation
                  for the variable "v". ]

  Description   []

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_var_input(const SexpFsm_ptr self, node_ptr var_name)
{
  VarFsm_ptr var_fsm;

  SEXP_FSM_CHECK_INSTANCE(self);
  var_fsm = find_assoc(self->hash_var_fsm, var_name);
  return var_fsm_get_input(var_fsm);
}




/**Function********************************************************************

  Synopsis      [ Gets the list of sexp expressions defining the set of justice 
                  constraints for this machine. ]

  Description   [ Gets the list of sexp expressions defining the set of justice 
                  constraints for this machine. ]

  SideEffects        []

******************************************************************************/
node_ptr SexpFsm_get_justice(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->justice;
}


/**Function********************************************************************

  Synopsis      [ Gets the list of sexp expressions defining the set of
                  compassion constraints for this machine. ]

  Description   [ Gets the list of sexp expressions defining the set of 
                  compassion constraints for this machine. ]

  SideEffects        []

******************************************************************************/
node_ptr SexpFsm_get_compassion(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->compassion;
}




VarSet_ptr SexpFsm_get_vars_list(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->vars_list;
}


/*---------------------------------------------------------------------------*/
/* Static function definitions                                               */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis      [Call this from any constructor which creates a new family]

  Description   [Every time a fsm is created from scratch (i.e. via its 
  constructor), a new family is created. When a fsm D is derived from an 
  existing base fsm B, D belongs to the family of B]

  SideEffects        []

******************************************************************************/
static void sexp_fsm_create_shared_structures(SexpFsm_ptr self)
{
  /* a new family is allocated */
  self->family_counter = ALLOC(int, 1);
  nusmv_assert(self->family_counter != NULL);
  *(self->family_counter) = 1;

  /* ---------------------------------------- */
  /* put here all shared structure allocation */
  /* ---------------------------------------- */
  sexp_fsm_hash_var_fsm_create(self);
  
}


/**Function********************************************************************

  Synopsis      [Call this from every destructor which tries to destroy a 
  family. If the family is over, all shared structure will be destroyed]

  Description [All structures will be destroyed only if the family
  counter reaches the zero value]

  SideEffects        []

******************************************************************************/
static void sexp_fsm_try_to_destroy_shared_structures(SexpFsm_ptr self)
{
  nusmv_assert(self->family_counter != NULL); 

  *(self->family_counter) = *(self->family_counter) - 1;  
  nusmv_assert(*(self->family_counter)  >= 0);

  if ( (*(self->family_counter)) == 0 ) {
    FREE(self->family_counter);    
    self->family_counter = NULL;

    /* --------------------------------------------------------------------- */
    /* family is actually over. Put here destruction of any shared structure */
    /* --------------------------------------------------------------------- */
    sexp_fsm_hash_var_fsm_destroy(self);

  }
}


/**Function********************************************************************

  Synopsis      [Call this to copy data from fsm belonging to the same family. 
  This also increments the family counter]

  Description   []

  SideEffects        []

******************************************************************************/
static void sexp_fsm_copy_shared_structures(const SexpFsm_ptr self, 
					    SexpFsm_ptr copy)
{
  *(self->family_counter) = *(self->family_counter) + 1;  
  copy->family_counter = self->family_counter;  


  /* -------------------------------------- */
  /* put here copy of all shared structures */
  /* -------------------------------------- */

  copy->hash_var_fsm = self->hash_var_fsm;
}




/**Function********************************************************************

  Synopsis      [Initializes the vars fsm hash]

  Description   []

  SideEffects        []

******************************************************************************/
static void sexp_fsm_hash_var_fsm_init(SexpFsm_ptr self, VarSet_ptr vars_list)
{
  node_ptr iter;

  iter = NODE_PTR(vars_list);
  while (iter != Nil) {
    int saved_yylineno = yylineno;

    VarFsm_ptr var_fsm;

    node_ptr var_name = car(iter);
    node_ptr var_name_i = find_node(SMALLINIT, var_name, Nil);
    node_ptr var_name_n = find_node(NEXT, var_name, Nil);

    node_ptr init_e     = lookup_assign_db_hash(var_name_i);
    node_ptr invar_e    = lookup_assign_db_hash(var_name);
    node_ptr next_e     = lookup_assign_db_hash(var_name_n);

    Expr_ptr init_sexp  = Expr_true();
    Expr_ptr invar_sexp = Expr_true();
    Expr_ptr trans_sexp = Expr_true();

    if (init_e != Nil) {
      yylineno = var_name_i->lineno;
      if (car(init_e) != Nil) {
        Expr_ptr init_assign_expr;

        yylineno = init_e->lineno;
        init_assign_expr = EXPR( find_node(EQDEF, var_name_i, car(init_e)) );
        init_sexp = Expr_and(init_sexp, init_assign_expr);
      }
      if (cdr(init_e) != Nil) {
        init_sexp = Expr_and(init_sexp, EXPR(cdr(init_e)));
      }
    }

    if (invar_e != Nil) {
      if (car(invar_e) != Nil) {
        Expr_ptr invar_assign_expr;

        yylineno = invar_e->lineno;
	invar_assign_expr = EXPR( new_node(EQDEF, var_name, car(invar_e)) );
        invar_sexp = Expr_and(invar_sexp, invar_assign_expr);
      }
      if (cdr(invar_e) != Nil) {
        invar_sexp = Expr_and(invar_sexp, EXPR(cdr(invar_e)));
      }
    }

    if (next_e != Nil) {
      yylineno = var_name_n->lineno;
      if (car(next_e) != Nil) {
        Expr_ptr next_assign_expr; 

        yylineno = next_e->lineno;
        next_assign_expr = EXPR( new_node(EQDEF, var_name_n, car(next_e)) );
        trans_sexp = Expr_and(trans_sexp, next_assign_expr);
      }
      if (cdr(next_e) != Nil) {
        trans_sexp = Expr_and(trans_sexp, EXPR(cdr(next_e)));
      }
    }

    /* inserts the var fsm inside the hash table */
    var_fsm = var_fsm_create(init_sexp, invar_sexp, trans_sexp);
    sexp_fsm_hash_var_fsm_insert_var(self, var_name, var_fsm);

    iter = cdr(iter);
    yylineno = saved_yylineno;
  } /* loop */
}



/**Function********************************************************************

  Synopsis      [Initializes either the boolean or scalar sexp fsm]

  Description   []

  SideEffects        []

******************************************************************************/
static void sexp_fsm_init(SexpFsm_ptr self, 
			  VarSet_ptr vars_list, 
			  node_ptr justice,
			  node_ptr compassion, 
			  const boolean booleanize)
{
  Expr_ptr init  = Expr_true();
  Expr_ptr invar = Expr_true();
  Expr_ptr next  = Expr_true();
  Expr_ptr input  = Expr_true();

  hash_ptr hash; 
  node_ptr iter;
  
  /* We set the verbose level to 0 and then we restore the original
     value. This because booleanization uses eval */  
  int curr_verbosity = get_verbose_level(options);
  set_verbose_level(options, 0);
  
  hash = simplifier_hash_create();
  
  iter = vars_list;
  while (iter != Nil) {
    node_ptr var = car(iter);
    VarFsm_ptr varfsm = sexp_fsm_hash_var_fsm_lookup_var(self, var);
    
    if (varfsm != VAR_FSM(NULL)) {
      Expr_ptr tmp;

      /* inits */
      tmp = var_fsm_get_init(varfsm);
      if (tmp != Nil) {
	tmp = sexp_fsm_simplify_expr(hash, tmp, INIT, booleanize);
	init = Expr_and(init, tmp);
      }

      /* invars */
      tmp = var_fsm_get_invar(varfsm);
      if (tmp != Nil) {
	tmp = sexp_fsm_simplify_expr(hash, tmp, INVAR, booleanize);
	invar = Expr_and(invar, tmp);
      }
      
      /* next */
      tmp = var_fsm_get_next(varfsm);
      if (tmp != Nil) {
	tmp = sexp_fsm_simplify_expr(hash, tmp, TRANS, booleanize);
	next = Expr_and(next, tmp);
      }
    }

    iter = cdr(iter);
  } /* loop */
  
  simplifier_hash_destroy(hash); 
  set_verbose_level(options, curr_verbosity);

  /* inits all private members: */
  self->vars_list = vars_list;

  self->init       = init;
  self->invar      = invar;
  self->trans      = next;
  self->input      = input;

  self->is_booleanized = booleanize;

  /* Booleanize fairness if needed: */
  if (self->is_booleanized) {
    self->justice = expr2bexpr(justice);
    self->compassion = expr2bexpr(compassion);
  }
  else {
    self->justice    = justice;
    self->compassion = compassion;
  }

}


/**Function********************************************************************

  Synopsis           [removes duplicates from expression containing AND nodes]

  Description        [group identifies INVAR, TRANS or INIT group. 
  If booleanize is True, the expression will be also booleanized]

  SideEffects        []

******************************************************************************/
static Expr_ptr sexp_fsm_simplify_expr(hash_ptr hash, Expr_ptr expr, 
				       const int group, 
				       const boolean booleanize)
{
  Expr_ptr result;
  
  if ((expr == EXPR(NULL)) || simplifier_hash_query_expr(hash, expr, group)) {
    result = Expr_true();
  }
  else {
    switch (node_get_type(NODE_PTR(expr))) {
    case AND:
      {
	Expr_ptr left, right;
	left  = sexp_fsm_simplify_expr(hash, car(NODE_PTR(expr)), 
				       group, booleanize);

	right = sexp_fsm_simplify_expr(hash, cdr(NODE_PTR(expr)), 
				       group, booleanize);

	result = Expr_and(left, right);
	break;
      }

    default:
      if (booleanize) result = EXPR( expr2bexpr(NODE_PTR(expr)) );
      else            result = expr;

    } /* switch */
    
    simplifier_hash_add_expr(hash, expr, group);
  }
  
  return result;
}



/**Function********************************************************************

  Synopsis           [This is used when creating cluster list from vars list]

  Description        []

  SideEffects        []

******************************************************************************/
static hash_ptr simplifier_hash_create()
{
  hash_ptr result;

  result = st_init_table(st_ptrcmp, st_ptrhash);
  nusmv_assert(result != ((hash_ptr) NULL));

  return result;
}

/**Function********************************************************************

  Synopsis           [Call after sexp_fsm_cluster_hash_create]

  Description        []

  SideEffects        []

******************************************************************************/
static void simplifier_hash_destroy(hash_ptr hash)
{
  nusmv_assert(hash != (hash_ptr) NULL);
  st_free_table(hash);  
}


/**Function********************************************************************

  Synopsis           [To insert a new node in the hash]

  Description        [group is INIT, INVAR or TRANS]

  SideEffects        [The hash can change]

******************************************************************************/
static void 
simplifier_hash_add_expr(hash_ptr hash, Expr_ptr expr, const int group)
{
  int res;
  
  res = st_add_direct(hash, (char*) expr, (char*) group);
  nusmv_assert(res != ST_OUT_OF_MEM);
}


/**Function********************************************************************

  Synopsis           [Queries for an element in the hash, returns True if 
  found]

  Description        []

  SideEffects        []

******************************************************************************/
static boolean 
simplifier_hash_query_expr(hash_ptr hash, Expr_ptr expr, 
			   const int group)
{
  int hashed_group; 
  boolean result;

  result = st_lookup(hash, (char *) expr, (char **) &hashed_group);
  
  return (result && (hashed_group == group));
}


/**Function********************************************************************

  Synopsis           [Call to create the hash for var fsm inside the fsm]

  Description        [Private method, used internally]

  SideEffects        []

******************************************************************************/
static void sexp_fsm_hash_var_fsm_create(SexpFsm_ptr self) 
{
  nusmv_assert(self->hash_var_fsm == (hash_ptr) NULL);

  self->hash_var_fsm = new_assoc();  
}


/**Function********************************************************************

  Synopsis           [Call to destroy the var fsm hash]

  Description        [Private method, used internally]

  SideEffects        []

******************************************************************************/
static void sexp_fsm_hash_var_fsm_destroy(SexpFsm_ptr self) 
{
  nusmv_assert(self->hash_var_fsm != (hash_ptr) NULL);

  clear_assoc_and_free_entries(self->hash_var_fsm, 
			       sexp_fsm_callback_var_fsm_free);
}


/**Function********************************************************************

  Synopsis [Private callback that destroys a single variable fsm
  contained into the var fsm hash]

  Description        []

  SideEffects        []

******************************************************************************/
static assoc_retval sexp_fsm_callback_var_fsm_free(char *key, 
						   char *data, char * arg)
{
  VarFsm_ptr varfsm = VAR_FSM(data);

  var_fsm_destroy(varfsm);  
  return ASSOC_DELETE;
}


/**Function********************************************************************

  Synopsis [Given a variable name, returns the corresponding variable
  fsm, or NULL if not found]

  Description        []

  SideEffects        []

******************************************************************************/
static VarFsm_ptr 
sexp_fsm_hash_var_fsm_lookup_var(SexpFsm_ptr self, node_ptr var) 
{
  nusmv_assert(self->hash_var_fsm != (hash_ptr) NULL);

  return VAR_FSM( find_assoc(self->hash_var_fsm, var) );
}


/**Function********************************************************************

  Synopsis           [Adds a var fsm to the internal hash. Private.]

  Description        []

  SideEffects        []

******************************************************************************/
static void 
sexp_fsm_hash_var_fsm_insert_var(SexpFsm_ptr self, 
				 node_ptr var, VarFsm_ptr varfsm)
{
  nusmv_assert(self->hash_var_fsm != (hash_ptr) NULL);
  
  insert_assoc(self->hash_var_fsm, var, varfsm);
}




/**Function********************************************************************

  Synopsis           [Creates a var fsm]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static VarFsm_ptr var_fsm_create(Expr_ptr init, 
				 Expr_ptr invar, 
				 Expr_ptr next)
{
  return VAR_FSM( cons(NODE_PTR(init), 
		       cons(NODE_PTR(invar), NODE_PTR(next))) );
}


/**Function********************************************************************

  Synopsis           [It does not destroy the init, trans and invar nodes. 
  It destroys only the support nodes]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void var_fsm_destroy(VarFsm_ptr self)
{
  node_ptr node = NODE_PTR(self);

  free_node(cdr(node));
  free_node(node);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Expr_ptr var_fsm_get_init(VarFsm_ptr self)
{
  return EXPR( car(NODE_PTR(self)) );
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Expr_ptr var_fsm_get_invar(VarFsm_ptr self)
{
  return EXPR( car(cdr(NODE_PTR(self))) );
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Expr_ptr var_fsm_get_next(VarFsm_ptr self)
{
  return EXPR( cdr(cdr(NODE_PTR(self))) );
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Expr_ptr var_fsm_get_input(VarFsm_ptr self)
{
  /* Currently no constraints over input are allowed, thus we return
     true to inidicate this. */
  return Expr_true();
}


