/**CFile*****************************************************************

  FileName    [EncCache.c]

  PackageName [enc.symb]

  Synopsis    [The EncCache class implementation]

  Description []

  SeeAlso     [EncCache.h]

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

#include "EncCache.h"

#include "enc/encInt.h"
#include "parser/symbols.h"
#include "compile/compile.h"
#include "utils/assoc.h"
#include "utils/error.h"


static char rcsid[] UTIL_UNUSED = "$Id: EncCache.c,v 1.1.2.7.2.1 2004/08/04 13:18:35 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/**Struct**********************************************************************

  Synopsis    [The EncCache class]

  Description []
  
******************************************************************************/
typedef struct EncCache_TAG
{
  /* Cache for encoding association. Contains node of these kinds:
     for variables    VAR, IVAR: encoding of range, range
     for defines      CONTEXT: ctx, definition body */
  hash_ptr symbol_hash;

  /* Several operation with the same key are performed on the symbol
     hash.  To improve performance, the last operated key and the
     associated value are stored separately here. */
  node_ptr last_symbol_hash_key; 
  node_ptr last_symbol_hash_value; 
  
  /* keep track of all constants (leafs) */
  hash_ptr constant_hash; /* for fast searching */

  /* for tricky code in LTL MC */
  struct {
    hash_ptr symbol_hash;
    hash_ptr constant_hash;
  } saved_hash;
  
} EncCache;


/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void enc_cache_init ARGS((EncCache_ptr self));
static void enc_cache_deinit ARGS((EncCache_ptr self));

static void 
enc_cache_new_symbol ARGS((EncCache_ptr self, node_ptr name, node_ptr value));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Class constructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
EncCache_ptr EncCache_create()
{
  EncCache_ptr self = ALLOC(EncCache, 1);

  ENC_CACHE_CHECK_INSTANCE(self);

  enc_cache_init(self); 
  return self;  
}


/**Function********************************************************************

  Synopsis           [Class destructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void EncCache_destroy(EncCache_ptr self)
{
  ENC_CACHE_CHECK_INSTANCE(self);

  enc_cache_deinit(self);
  FREE(self);  
}


/**Function********************************************************************

  Synopsis      []

  Description   []
  
  SeeAlso       []

  SideEffects        []

******************************************************************************/
void EncCache_push_status_and_reset(EncCache_ptr self)
{
  ENC_CACHE_CHECK_INSTANCE(self);
  
  self->saved_hash.symbol_hash = self->symbol_hash;
  self->saved_hash.constant_hash = self->constant_hash;
  
  /* we can pay this... */
  self->last_symbol_hash_key = Nil;
  self->last_symbol_hash_value = Nil;

  self->symbol_hash = copy_assoc(self->symbol_hash);
  self->constant_hash = copy_assoc(self->constant_hash);
}


/**Function********************************************************************

  Synopsis      []

  Description   []
  
  SeeAlso       []

  SideEffects        []

******************************************************************************/
void EncCache_pop_status(EncCache_ptr self)
{
  ENC_CACHE_CHECK_INSTANCE(self);

  free_assoc(self->symbol_hash);
  free_assoc(self->constant_hash);
  
  self->last_symbol_hash_key = Nil;
  self->last_symbol_hash_value = Nil;

  self->symbol_hash = self->saved_hash.symbol_hash; 
  self->constant_hash = self->saved_hash.constant_hash;  
}


/**Function********************************************************************

  Synopsis      [A new leaf (constant) is added to the constants list]

  Description   []
  
  SeeAlso       []

  SideEffects        []

******************************************************************************/
void EncCache_new_constant(EncCache_ptr self, node_ptr c)
{
  ENC_CACHE_CHECK_INSTANCE(self);

  if (EncCache_is_constant_defined(self, c)) {
    internal_error("EncCache_new_constant: constant already declared\n");
  }

  /* Hash only ATOMS, DOT and ARRAY included to keep process names */
  if ( (node_get_type(c) == ATOM) 
       || (node_get_type(c) == DOT)  
       || (node_get_type(c) == ARRAY) ) {
    insert_assoc(self->constant_hash, c, (node_ptr) true);
    insert_check_constant_hash(c, c); /* Add to the hash used for checking */
  }
}


/**Function********************************************************************

  Synopsis           [The variable encoding must be associated later]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void EncCache_new_input_var(EncCache_ptr self, node_ptr var, 
			    node_ptr range)
{
  ENC_CACHE_CHECK_INSTANCE(self);

  enc_cache_new_symbol(self, var, new_node(IVAR, Nil, range));
}


/**Function********************************************************************

  Synopsis           [The variable encoding must be associated later]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void EncCache_new_state_var(EncCache_ptr self, node_ptr var, 
			    node_ptr range)
{
  ENC_CACHE_CHECK_INSTANCE(self);

  enc_cache_new_symbol(self, var, new_node(VAR, Nil, range));
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void EncCache_new_define(EncCache_ptr self, node_ptr name, 
			 node_ptr ctx, node_ptr definition)
{
  ENC_CACHE_CHECK_INSTANCE(self);

  enc_cache_new_symbol(self, name, new_node(CONTEXT, ctx, definition));
}


/**Function********************************************************************

  Synopsis           [Returns the definition of the given symbol]

  Description        [Returned node can be either VAR, IVAR or CONTEXT. Value
  is searched first in the cache, then in the symbol hash]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr EncCache_lookup_symbol(EncCache_ptr self, node_ptr name)
{
  ENC_CACHE_CHECK_INSTANCE(self);

  if (self->last_symbol_hash_key != name) {
    self->last_symbol_hash_value = find_assoc(self->symbol_hash, name);
    self->last_symbol_hash_key = name;
  }
  
  return self->last_symbol_hash_value;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean EncCache_is_symbol_state_var(EncCache_ptr self, node_ptr name)
{
  node_ptr symb;
  boolean res; 

  ENC_CACHE_CHECK_INSTANCE(self);

  symb = EncCache_lookup_symbol(self, name);
  if (symb == (node_ptr) NULL)  res = false;
  else res = (node_get_type(symb) == VAR); 
    
  return res;  
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean EncCache_is_symbol_input_var(EncCache_ptr self, node_ptr name)
{
  node_ptr symb;
  boolean res; 

  ENC_CACHE_CHECK_INSTANCE(self);

  symb = EncCache_lookup_symbol(self, name);
  if (symb == (node_ptr) NULL)  res = false;
  else res = (node_get_type(symb) == IVAR); 
    
  return res;  
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean EncCache_is_symbol_var(EncCache_ptr self, node_ptr name)
{
  node_ptr symb;
  boolean res; 

  ENC_CACHE_CHECK_INSTANCE(self);

  symb = EncCache_lookup_symbol(self, name);
  if (symb == (node_ptr) NULL)  res = false;
  else {
    res = ( (node_get_type(symb) == VAR) 
	    || (node_get_type(symb) == IVAR) );
  }
    
  return res;  
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean EncCache_is_symbol_declared(EncCache_ptr self, node_ptr name)
{
  node_ptr symb;

  ENC_CACHE_CHECK_INSTANCE(self);

  symb = EncCache_lookup_symbol(self, name);
  return (symb != (node_ptr) NULL);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean EncCache_is_symbol_define(EncCache_ptr self, node_ptr name)
{
  node_ptr symb;
  boolean res; 

  ENC_CACHE_CHECK_INSTANCE(self);

  symb = EncCache_lookup_symbol(self, name);
  if (symb == (node_ptr) NULL)  res = false;
  else res = (node_get_type(symb) == CONTEXT);
    
  return res;    
}



/**Function********************************************************************

  Synopsis           [Returns true if the given constant has already been 
  added]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean EncCache_is_constant_defined(EncCache_ptr self, node_ptr constant)
{
  ENC_CACHE_CHECK_INSTANCE(self);
  return (find_assoc(self->constant_hash, constant) != (node_ptr) NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis      [Insert a new value in the symbol hash]

  Description   [This takes into account also the caching of last accessed 
  value]
  
  SideEffects        []

******************************************************************************/
static void 
enc_cache_new_symbol(EncCache_ptr self, node_ptr name, node_ptr value)
{
  insert_assoc(self->symbol_hash, name, value);
  self->last_symbol_hash_key = name;
  self->last_symbol_hash_value = value;
}


/**Function********************************************************************

  Synopsis           [Private initializer]

  Description        [Private initializer, called by the constructor]

  SideEffects        []

  SeeAlso            [bdd_enc_cache_deinit]

******************************************************************************/
static void enc_cache_init(EncCache_ptr self)
{
  self->symbol_hash = new_assoc();
  nusmv_assert(self->symbol_hash != (hash_ptr) NULL);

  self->last_symbol_hash_key = (node_ptr) NULL;
  self->last_symbol_hash_value = (node_ptr) NULL;

  self->constant_hash = new_assoc();
  nusmv_assert(self->constant_hash != (hash_ptr) NULL);

}


/**Function********************************************************************

  Synopsis           [Private deinitializer]

  Description        [Private deinitializer, called by the destructor]

  SideEffects        []

  SeeAlso            [bdd_enc_cache_init]

******************************************************************************/
static void enc_cache_deinit(EncCache_ptr self)
{
  free_assoc(self->symbol_hash);  
  free_assoc(self->constant_hash);  
}
