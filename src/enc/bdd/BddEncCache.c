/**CFile*****************************************************************

  FileName    [BddEncCache.c]

  PackageName [enc.bdd]

  Synopsis    [The BddEncCache class implementation]

  Description []

  SeeAlso     [BddEncCache.h]

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
#include "BddEncCache.h"
#include "BddEnc_private.h"

#include "utils/error.h"
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: BddEncCache.c,v 1.1.2.12 2004/05/31 09:07:33 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/**Struct**********************************************************************

  Synopsis    [The BddEncCache class]

  Description []
  
******************************************************************************/
typedef struct BddEncCache_TAG
{
  /* corresponding bdd encoding */
  BddEnc_ptr enc;

  /* This hash associates to an atom the corresponding ADD leaf if
  defined. Suppose to have a declaration of this kind: 
     VAR state : {idle, stopped} 
  then in the constant hash for the atom idle there is the
  corresponding leaf ADD, i.e. the ADD whose value is the symbol
  idle. This hash is used by the evaluator */
  hash_ptr constant_hash;

  /* associates var names with corresponding ADDs */
  hash_ptr vars_hash;

  /* This hash is used by eval to cache the result of sexp
     evaluation. */
  hash_ptr eval_hash;

  /* this is used only by method BddEnc_get_definition, to keep track
     of evaluation of definitions, */
  hash_ptr definition_hash;

  /* for tricky code: status save and restore */
  struct {
    hash_ptr constant_hash;
    hash_ptr vars_hash;
    hash_ptr eval_hash;
    hash_ptr definition_hash;
  } saved_hash;

  boolean saved; 

} BddEncCache;


/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void bdd_enc_cache_init ARGS((BddEncCache_ptr self, BddEnc_ptr enc));
				     
static void bdd_enc_cache_deinit ARGS((BddEncCache_ptr self));

static assoc_retval hash_dup_add ARGS((char* key, char* add, char* ddmgr));

static assoc_retval 
hash_free_add_evaluating ARGS((char* key, char* data, char* arg));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddEncCache_ptr BddEncCache_create(BddEnc_ptr enc)
{
  BddEncCache_ptr self = ALLOC(BddEncCache, 1);

  BDD_ENC_CACHE_CHECK_INSTANCE(self);

  bdd_enc_cache_init(self, enc); 
  return self;  
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEncCache_destroy(BddEncCache_ptr self)
{
  BDD_ENC_CACHE_CHECK_INSTANCE(self);

  bdd_enc_cache_deinit(self);
  FREE(self);  
}


/**Macro***********************************************************************

  Synopsis           [Saves and reset the given hash member]

  Description        [Used only by method BddEncCache_push_status_and_reset]

  SideEffects        []

******************************************************************************/
#define BDD_ENC_CACHE_SAVE_RESET_CACHE(x, dd_mgr)           \
{                                                           \
  self->saved_hash.x = self->x;                             \
  self->x = copy_assoc(self->x);                            \
  st_foreach(self->x, &hash_dup_add, (char*) dd_mgr);       \
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEncCache_push_status_and_reset(BddEncCache_ptr self)
{
  DdManager* dd;
  
  BDD_ENC_CACHE_CHECK_INSTANCE(self);
  nusmv_assert(!self->saved); /* not already saved */

  dd = BddEnc_get_dd_manager(self->enc);

  BDD_ENC_CACHE_SAVE_RESET_CACHE(constant_hash, dd);
  BDD_ENC_CACHE_SAVE_RESET_CACHE(vars_hash, dd);
  BDD_ENC_CACHE_SAVE_RESET_CACHE(eval_hash, dd);
  BDD_ENC_CACHE_SAVE_RESET_CACHE(definition_hash, dd);

  self->saved = true;
}


/**Macro***********************************************************************

  Synopsis           [Saves and reset the given hash member]

  Description        [Used only by method BddEncCache_push_status_and_reset]

  SideEffects        []

******************************************************************************/
#define BDD_ENC_CACHE_POP_CACHE(x, dd_mgr)                         \
{                                                                  \
  st_foreach(self->x, &hash_free_add_evaluating, (char*) dd_mgr);  \
  free_assoc(self->x);                                             \
  self->x = self->saved_hash.x;                                    \
  self->saved_hash.x = (hash_ptr) NULL;                            \
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEncCache_pop_status(BddEncCache_ptr self)
{
  DdManager* dd;

  BDD_ENC_CACHE_CHECK_INSTANCE(self);
  nusmv_assert(self->saved); /* previously saved */

  dd = BddEnc_get_dd_manager(self->enc);

  BDD_ENC_CACHE_POP_CACHE(constant_hash, dd);
  BDD_ENC_CACHE_POP_CACHE(vars_hash, dd);
  BDD_ENC_CACHE_POP_CACHE(eval_hash, dd);
  BDD_ENC_CACHE_POP_CACHE(definition_hash, dd);

  self->saved = false;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEncCache_new_constant(BddEncCache_ptr self, node_ptr constant, 
			      add_ptr constant_add)
{
  BDD_ENC_CACHE_CHECK_INSTANCE(self);

  /* we don't store number constants into the symb cache, so we are
     lazy in that case */
  nusmv_assert(
      Encoding_is_symbol_constant(BddEnc_get_symbolic_encoding(self->enc), 
				  constant) 
      || (node_get_type(constant) == NUMBER));

  /* Not already defined. We reuse already defined leaf */
  if (! BddEncCache_is_constant_defined(self, constant)) {
    add_ref(constant_add);
    insert_assoc(self->constant_hash, constant, (node_ptr) constant_add);
  }
} 


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean BddEncCache_is_constant_defined(BddEncCache_ptr self, 
					node_ptr constant)
{
  BDD_ENC_CACHE_CHECK_INSTANCE(self);
  return (find_assoc(self->constant_hash, constant) != (node_ptr) NULL);
}

/**Function********************************************************************

  Synopsis      [Returns the ADD corresponding to the given constant, or 
  NULL if not defined]

  Description   [Returned ADD is referenced]
  
  Notes         []

  SideEffects        []

******************************************************************************/
add_ptr BddEncCache_lookup_constant(BddEncCache_ptr self, node_ptr constant)
{
  add_ptr res;

  BDD_ENC_CACHE_CHECK_INSTANCE(self);

  res = (add_ptr) find_assoc(self->constant_hash, constant);
  if (res != (add_ptr) NULL) { add_ref(res); }
  
  return res;  
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEncCache_new_var(BddEncCache_ptr self, node_ptr var_name,
			 add_ptr var_add)
{
  Encoding_ptr senc;
  BDD_ENC_CACHE_CHECK_INSTANCE(self);
  
  senc = BddEnc_get_symbolic_encoding(self->enc);

  /* must be a variable declared inside the symbolic encoding... */
  if (! Encoding_is_symbol_var(senc, var_name)) {
    /* is it a non registered NEXT variable? */
    if (node_get_type(var_name) == NEXT) {
      if (! Encoding_is_symbol_var(senc, car(var_name))) {
	internal_error("BddEncCache_new_var: trying to create a new var not previously registered\n");
      }
    }
  }

  /* not already defined */
  nusmv_assert(! BddEncCache_is_var_defined(self, var_name)); 
  
  add_ref(var_add);
  insert_assoc(self->vars_hash, var_name, (node_ptr) var_add);
} 


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean BddEncCache_is_var_defined(BddEncCache_ptr self, node_ptr var_name)
{
  BDD_ENC_CACHE_CHECK_INSTANCE(self);
  return (find_assoc(self->vars_hash, var_name) != (node_ptr) NULL);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEncCache_lookup_var(BddEncCache_ptr self, node_ptr var_name)
{
  add_ptr res;

  BDD_ENC_CACHE_CHECK_INSTANCE(self);

  res = (add_ptr) find_assoc(self->vars_hash, var_name);
  if (res != (add_ptr) NULL) { add_ref(res); }
  
  return res;  
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEncCache_set_definition(BddEncCache_ptr self, node_ptr name,
				add_ptr def)
{
  BDD_ENC_CACHE_CHECK_INSTANCE(self);

  if ((def != EVALUATING) && (def != (add_ptr) NULL)) { add_ref(def); }
  insert_assoc(self->definition_hash, name, (node_ptr) def); 
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEncCache_get_definition(BddEncCache_ptr self, node_ptr name)
{
  add_ptr res;
  
  BDD_ENC_CACHE_CHECK_INSTANCE(self);

  res = (add_ptr) find_assoc(self->definition_hash, name);
  if ((res != EVALUATING) && (res != (add_ptr) NULL)) { add_ref(res); }

  return res;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddEncCache_set_evaluation(BddEncCache_ptr self, node_ptr name,
				add_ptr add)
{
  BDD_ENC_CACHE_CHECK_INSTANCE(self);

  if ((add != EVALUATING) && (add != (add_ptr) NULL)) { add_ref(add); }
  insert_assoc(self->eval_hash, name, (node_ptr) add); 
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
add_ptr BddEncCache_get_evaluation(BddEncCache_ptr self, node_ptr name)
{
  add_ptr res;
  
  BDD_ENC_CACHE_CHECK_INSTANCE(self);

  res = (add_ptr) find_assoc(self->eval_hash, name);
  if ((res != EVALUATING) && (res != (add_ptr) NULL)) { add_ref(res); }

  return res;
}




/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Private initializer]

  Description        [Private initializer, called by the constructor]

  SideEffects        []

  SeeAlso            [bdd_enc_cache_deinit]

******************************************************************************/
static void bdd_enc_cache_init(BddEncCache_ptr self, BddEnc_ptr enc)
{
  self->enc = enc;

  self->constant_hash = new_assoc();
  nusmv_assert(self->constant_hash != (hash_ptr) NULL);

  self->vars_hash = new_assoc();
  nusmv_assert(self->vars_hash != (hash_ptr) NULL);
  
  self->eval_hash = new_assoc();
  nusmv_assert(self->eval_hash != (hash_ptr) NULL);

  self->definition_hash = new_assoc();
  nusmv_assert(self->definition_hash != (hash_ptr) NULL);

  self->saved = false;
}



/**Function********************************************************************

  Synopsis           [Private deinitializer]

  Description        [Private deinitializer, called by the destructor]

  SideEffects        []

  SeeAlso            [bdd_enc_cache_init]

******************************************************************************/
static void bdd_enc_cache_deinit(BddEncCache_ptr self)
{
  DdManager* dd = BddEnc_get_dd_manager(self->enc);

  st_foreach(self->constant_hash, &hash_free_add_evaluating, (char*) dd);
  free_assoc(self->constant_hash);  

  st_foreach(self->vars_hash, &hash_free_add_evaluating, (char*) dd);
  free_assoc(self->vars_hash);  

  st_foreach(self->eval_hash, &hash_free_add_evaluating, (char*) dd);
  free_assoc(self->eval_hash); 

  st_foreach(self->definition_hash, &hash_free_add_evaluating, (char*) dd);
  free_assoc(self->definition_hash); 

}


/**Function********************************************************************

  Synopsis           [Private micro function used when destroying caches of
  adds]

  Description        [Called when pushing the status, and during
  deinitialization]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static assoc_retval 
hash_free_add_evaluating(char* key, char* data, char* arg) 
{
  if ((data != (char*) NULL) && ((add_ptr) data != EVALUATING)) {
    add_free((DdManager*) arg, (add_ptr) data);
  }
  return ASSOC_DELETE;
}



/**Function********************************************************************

  Synopsis           [Private micro function used when copying caches of adds]

  Description        [Called when pushing the status]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static assoc_retval hash_dup_add(char* key, char* add, char* ddmgr) 
{
  if ((add != (char*) NULL) && ((add_ptr) add != EVALUATING) ) {
    add_ref((add_ptr) add);
  }

  return ASSOC_CONTINUE;
}
