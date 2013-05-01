/**CFile*****************************************************************

  FileName    [Encoding.c]

  PackageName [enc.symb]

  Synopsis    [The symbolic Encoding implementation]

  Description []

  SeeAlso     [Encoding.h]

  Author      [Roberto Cavada, Daniel Sheridan, Marco Roveri]

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

#include "Encoding.h"
#include "Encoding_private.h"
#include "EncCache.h"

#include "parser/symbols.h" 
#include "parser/ord/ParserOrd.h"
#include "enc/encInt.h"

#include "compile/compile.h"
#include "opt/opt.h"

#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: Encoding.c,v 1.1.2.27.2.6 2005/08/30 12:01:09 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis     [A global variable shared between all the BddEnc class 
  instances]

  Description  [Initialized the first time an instance of class BddEnc is 
  created (see BddEnc constructor)]

******************************************************************************/
node_ptr boolean_type = Nil;
void encoding_quit_boolean_type() { boolean_type = Nil; } 

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void encoding_init ARGS((Encoding_ptr self));
static void encoding_deinit ARGS((Encoding_ptr self));

static void 
encoding_declare_input_var ARGS((Encoding_ptr self, node_ptr var, 
				 node_ptr range));

static node_ptr 
encoding_encode_scalar_var ARGS((Encoding_ptr self, node_ptr name, 
				 int suffix, node_ptr range, 
				 boolean is_input, NodeList_ptr group));

static void 
encoding_set_var_encoding ARGS((Encoding_ptr self, node_ptr var, 
				node_ptr encoding));


static void 
encoding_traverse_encoding ARGS((const Encoding_ptr self, 
				 node_ptr tree, NodeList_ptr list));

static boolean 
encoding_assign_define_body_to_symbols_lists ARGS((Encoding_ptr self, 
						   node_ptr name));

static void encoding_refill_symbols_lists ARGS((Encoding_ptr self));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Encoding_ptr Encoding_create()
{
  Encoding_ptr self = ALLOC(Encoding, 1);

  ENCODING_CHECK_INSTANCE(self);

  encoding_init(self); 

  /* init a shared variable (done by the first constructor call only) */
  if (boolean_type == Nil) { boolean_type = find_node(BOOLEAN, Nil, Nil); }
  return self;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Encoding_destroy(Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);

  encoding_deinit(self); 
  FREE(self);
}


/* helper macro for Encoding_push_status_and_reset */ 
#define PUSH_RESET_LIST(list)           \
{                                       \
  self->saved_lists.list = self->list;  \
  self->list = NodeList_create();       \
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Encoding_push_status_and_reset(Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  
  /* you are trying to push more times than once */
  nusmv_assert(!self->saved);

  PUSH_RESET_LIST(model_state_symbols);
  PUSH_RESET_LIST(model_input_symbols);
  PUSH_RESET_LIST(model_state_input_symbols);

  PUSH_RESET_LIST(input_variables);
  PUSH_RESET_LIST(model_input_variables);
  PUSH_RESET_LIST(state_variables);
  PUSH_RESET_LIST(all_symbols);
  PUSH_RESET_LIST(all_variables);
  PUSH_RESET_LIST(define_symbols);
  
  /* groups: */
  self->saved_lists.input_boolean_variables_group = 
    self->input_boolean_variables_group;
  self->input_boolean_variables_group = GroupSet_create();

  self->saved_lists.state_boolean_variables_group = 
    self->state_boolean_variables_group;
  self->state_boolean_variables_group = GroupSet_create();

  /* cache */
  EncCache_push_status_and_reset(self->cache);
  self->saved = true;

  self->saved_lists.refill_symbols_lists = self->refill_symbols_lists;
  self->refill_symbols_lists = true;
}


/* helper macro for Encoding_push_status_and_reset */ 
#define POP_LIST(list)                 \
{                                      \
  NodeList_destroy(self->list);        \
  self->list = self->saved_lists.list; \
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Encoding_pop_status(Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);

  /* you are trying to pop a not previously pushed status */
  nusmv_assert(self->saved);

  POP_LIST(model_state_symbols);
  POP_LIST(model_state_input_symbols);
  POP_LIST(model_input_symbols);

  POP_LIST(input_variables);
  POP_LIST(model_input_variables);
  POP_LIST(state_variables);
  POP_LIST(all_symbols);
  POP_LIST(all_variables);
  POP_LIST(define_symbols);

  /* groups: */
  GroupSet_destroy(self->input_boolean_variables_group);
  self->input_boolean_variables_group = 
    self->saved_lists.input_boolean_variables_group;

  GroupSet_destroy(self->state_boolean_variables_group);
  self->state_boolean_variables_group = 
    self->saved_lists.state_boolean_variables_group;

  /* cache: */
  EncCache_pop_status(self->cache);
  self->saved = false;

  self->refill_symbols_lists = self->saved_lists.refill_symbols_lists;
}


/**Function********************************************************************

  Synopsis           [Performs the symbolic variables encoding]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Encoding_encode_vars(Encoding_ptr self)
{
  ListIter_ptr iter;
  NodeList_ptr vars;

  ENCODING_CHECK_INSTANCE(self);

  vars = Encoding_get_all_model_vars_list(self);
  iter = NodeList_get_first_iter(vars);

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "\n");
  }
  
  /* Loops over all the read or defined variables, depending the input
     order file is provided or not, to create add/bdd variables. */
  while (!ListIter_is_end(iter)) {    
    /* The current variable: */
    node_ptr name = NodeList_get_elem_at(vars, iter); 

    if (opt_verbose_level_gt(options, 2)) {
      fprintf(nusmv_stderr, "Encoding variable '");
      print_node(nusmv_stderr, name);
      fprintf(nusmv_stderr, "'...\n");
    }
    
    /* not already encoded */
    nusmv_assert(Encoding_get_var_encoding(self, name) == (node_ptr) NULL ); 

    Encoding_encode_var(self, name);
    
    iter = ListIter_get_next(iter); 
  } /* loop on all model vars */
}


/**Function********************************************************************

  Synopsis           [Returns the symbolic encoding for a given variable]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr Encoding_get_var_encoding(const Encoding_ptr self, node_ptr name)
{
  node_ptr symb; 
 
  ENCODING_CHECK_INSTANCE(self);
  nusmv_assert(EncCache_is_symbol_var(self->cache, name)); 
  
  symb = EncCache_lookup_symbol(self->cache, name);
  return car(symb);
} 


/**Function********************************************************************

  Synopsis [Returns the list of boolean vars used in the encoding of
  given scalar var]

  Description        [Returned list must be destroyed by caller]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr 
Encoding_get_var_encoding_bool_vars(const Encoding_ptr self, node_ptr name)
{
  NodeList_ptr res; 
  node_ptr enc;
  ENCODING_CHECK_INSTANCE(self);
  
  enc = Encoding_get_var_encoding(self, name);
  res = NodeList_create();
  encoding_traverse_encoding(self, enc, res);
  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the range of a given variable]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr Encoding_get_var_range(const Encoding_ptr self, node_ptr name)
{
  node_ptr symb; 
 
  ENCODING_CHECK_INSTANCE(self);
  nusmv_assert(EncCache_is_symbol_var(self->cache, name)); 
  
  symb = EncCache_lookup_symbol(self->cache, name);
  return cdr(symb);
}


/**Function********************************************************************

  Synopsis           [Returs true if the given symbol is the name of 
  a bit variable that is part of a scalar var]

  Description        []

  SideEffects        []

  SeeAlso            [Encoding_get_scalar_var_of_bit]

******************************************************************************/
boolean Encoding_is_var_bit(const Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  return (node_get_type(name) == BIT);
}


/**Function********************************************************************

  Synopsis           [Returns the name of the scalar variable whose 
  the given bit belongs]

  Description        [Returns the name of the scalar variable whose 
  the given bit belongs. The given var MUST be a bit]

  SideEffects        []

  SeeAlso            [Encoding_is_var_bit]

******************************************************************************/
node_ptr Encoding_get_scalar_var_of_bit(const Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  nusmv_assert(Encoding_is_var_bit(self, name));

  return car(name);
}


/**Function********************************************************************

  Synopsis           [Returns the body of the given define name]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr Encoding_get_define_body(const Encoding_ptr self, node_ptr name)
{
  node_ptr symb; 
 
  ENCODING_CHECK_INSTANCE(self);
  nusmv_assert(EncCache_is_symbol_define(self->cache, name)); 
  
  symb = EncCache_lookup_symbol(self->cache, name);
  return cdr(symb);  
}


/**Function********************************************************************

  Synopsis           [Returns the flattenized body of the given define name]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr Encoding_get_define_flatten_body(const Encoding_ptr self, 
					  node_ptr name)
{
  node_ptr res;
  ENCODING_CHECK_INSTANCE(self);
  nusmv_assert(EncCache_is_symbol_define(self->cache, name)); 
  
  res = lookup_flatten_def_hash(name);  
  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the body of the given define name]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr Encoding_get_define_context(const Encoding_ptr self, node_ptr name)
{
  node_ptr symb; 
 
  ENCODING_CHECK_INSTANCE(self);
  nusmv_assert(EncCache_is_symbol_define(self->cache, name)); 
  
  symb = EncCache_lookup_symbol(self->cache, name);
  return car(symb);  
}


/**Function********************************************************************

  Synopsis           [Encode a variable into BDD]

  Description [This function encodes the variable <tt>name</tt>, that
  must have been previously declared.  The resulting encoding is still
  a symbolic expression representing the encoding.  
  
  The boolean variables created are grouped together, in such a way
  that the BDD dynamic reordering consider them as a single block.]

  SideEffects        []

  SeeAlso            [encoding_encode_var_recur]

******************************************************************************/
void Encoding_encode_var(Encoding_ptr self, node_ptr name)
{
  node_ptr scalar_enc;
  node_ptr range;
  NodeList_ptr group; 
  boolean is_input;

  ENCODING_CHECK_INSTANCE(self);

  range = Encoding_get_var_range(self, name);
  is_input = NodeList_belongs_to(self->input_variables, name); 

  if (range == Nil) { internal_error("Encoding_encode_var: range is NULL"); }

  group = NodeList_create(); 
 
  /* Actually it is a variable */
  if (range == boolean_range) {
    scalar_enc = boolean_type; 
    NodeList_append(group, name); /* group of a single, boolean var */
  }
  else { 
    scalar_enc = encoding_encode_scalar_var(self, name, 0, 
					    range, is_input, group); 
  }
  
  encoding_set_var_encoding(self, name, scalar_enc);
  if (is_input) { 
    GroupSet_add_group(self->input_boolean_variables_group, group); 
  }
  else { 
    GroupSet_add_group(self->state_boolean_variables_group, group); 
  }

}


/**Function********************************************************************

  Synopsis           [Insert a new constant]

  Description        [A new constant is created]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Encoding_declare_constant(Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  
  EncCache_new_constant(self->cache, name);
  NodeList_append(self->constants_list, name);
}


/**Function********************************************************************

  Synopsis           [Insert a new determinization boolean variable]

  Description        [A new boolean input variable is created,
  and given name is added to determinization variables list]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Encoding_declare_determ_var(Encoding_ptr self, node_ptr var)
{
  node_ptr def;

  ENCODING_CHECK_INSTANCE(self);

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "<Encoding> Declaring determinization var: ");
    print_node(nusmv_stderr, var);
    fprintf(nusmv_stderr, "\n");
  }

  def = EncCache_lookup_symbol(self->cache, var);
  if (def != (node_ptr) NULL) {
    internal_error("Encoding_declare_determ_var: Variable already declared\n");    
  }
  
  encoding_declare_input_var(self, var, boolean_range);  
  NodeList_append(self->det_boolean_variables, var);
}



/**Function********************************************************************

  Synopsis           [Insert a new input variable]

  Description        [A new input variable is created with the given range,
  but no type.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Encoding_declare_input_var(Encoding_ptr self, node_ptr var, 
				node_ptr range)
{
  node_ptr def; 

  ENCODING_CHECK_INSTANCE(self); 

  def = EncCache_lookup_symbol(self->cache, var);
  if (def != (node_ptr) NULL) {
    fprintf(stderr, "Error: Cannot declare input variable ");
    print_node(stderr, var);
    fprintf(stderr, ": symbol already declared.\n");
    nusmv_exit(1);
  }

  encoding_declare_input_var(self, var, range);

  if (var == proc_selector_internal_vname) {
    if (llength(range) > 1) {
      /* appends process selector to the model input vars only if 
	 its range is larger than [main] */
      NodeList_append(self->model_input_variables, var);
      NodeList_append(self->model_input_symbols, var);
    }
  }
  else {
    NodeList_append(self->model_input_variables, var);
    NodeList_append(self->model_input_symbols, var);
  }
}


/**Function********************************************************************

  Synopsis           [Insert a new state variable]

  Description        [A new state variable is created with the given range,
  but no type]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Encoding_declare_state_var(Encoding_ptr self, node_ptr var, 
				node_ptr range)
{
  node_ptr def;

  ENCODING_CHECK_INSTANCE(self);

  def = EncCache_lookup_symbol(self->cache, var);
  if (def != (node_ptr) NULL) {
    fprintf(stderr, "Error: cannot declare state variable ");
    print_node(stderr, var);
    fprintf(stderr, ": symbol already declared.\n");
    nusmv_exit(1);
  }
  
  if (range == boolean_range) {
    NodeList_append(self->state_boolean_variables, var);
  }
  
  if (! Encoding_is_var_bit(self, var)) {
    NodeList_append(self->model_state_symbols, var);
    self->refill_symbols_lists = true;

    NodeList_append(self->state_variables, var);
    NodeList_append(self->all_variables, var);
    NodeList_append(self->all_symbols, var);
  }

  EncCache_new_state_var(self->cache, var, range);
}


/**Function********************************************************************

  Synopsis           [Insert a new define]

  Description        [A new define is created with the given range,
  but no type]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Encoding_declare_define(Encoding_ptr self, node_ptr name, 
			     node_ptr ctx, node_ptr definition)
{
  node_ptr def;
  NodeList_ptr deps;
  
  ENCODING_CHECK_INSTANCE(self);

  def = EncCache_lookup_symbol(self->cache, name);
  if (def != (node_ptr) NULL) {
    internal_error("Encoding_declare_define: name already declared\n");
  }
  
  NodeList_append(self->define_symbols, name);
  NodeList_append(self->all_symbols, name);

  EncCache_new_define(self->cache, name, ctx, definition);

  /* This is delayed: */
  /*  encoding_assign_define_body_to_symbols_lists(self, name); */
}				    


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Encoding_get_constants_num(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return NodeList_get_length(self->constants_list);
}


/**Function********************************************************************

  Synopsis           []

  Description        [Self keeps the ownership of the returned instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr Encoding_get_constants_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->constants_list;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Encoding_get_state_vars_num(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return NodeList_get_length(self->state_variables);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Encoding_get_bool_state_vars_num(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return NodeList_get_length(self->state_boolean_variables);
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Encoding_get_input_vars_num(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return NodeList_get_length(self->input_variables);
}


/**Function********************************************************************

  Synopsis           [Returns the number of input vars that occurs in the 
  model.
  If process_selector has a range larger than one, it will occur in the 
  returned count as well, otherwise it won't occur.]
  
  Description        []
  
  SideEffects        []
  
  SeeAlso            []
  
******************************************************************************/
int Encoding_get_model_input_vars_num(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return NodeList_get_length(self->model_input_variables);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Encoding_get_bool_input_vars_num(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return NodeList_get_length(self->input_boolean_variables);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Encoding_get_defines_num(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return NodeList_get_length(self->define_symbols);
}


/**Function********************************************************************

  Synopsis           []

  Description        [Self keeps the ownership of the returned instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr Encoding_get_all_model_vars_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->all_variables;
}


/**Function********************************************************************

  Synopsis           []

  Description        [Self keeps the ownership of the returned instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr Encoding_get_all_model_symbols_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->all_symbols;
}


/**Function********************************************************************

  Synopsis           []

  Description        [Self keeps the ownership of the returned instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr Encoding_get_state_vars_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->state_variables;
}


/**Function********************************************************************

  Synopsis           []

  Description        [Self keeps the ownership of the returned instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr Encoding_get_input_vars_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->input_variables;
}


/**Function********************************************************************

  Synopsis           [Returns the list of input vars that occurs in the model
  If process_selector has a range larger than one, it will occur in the 
  returned list as well, otherwise it won't occur.]

  Description        [Self keeps the ownership of the returned instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr Encoding_get_model_input_vars_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->model_input_variables;
}


/**Function********************************************************************

  Synopsis           [Returns the list of input boolean variables]

  Description        [Take into consideration to call Encoding_sort_bool_vars 
  before calling this method, in order to get a sorted list of bool vars. 
  Self keeps the ownership of the returned instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr Encoding_get_bool_input_vars_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->input_boolean_variables;
}


/**Function********************************************************************

  Synopsis           [Returns the list of state boolean variables]

  Description        [Take into consideration to call Encoding_sort_bool_vars 
  before calling this method, in order to get a sorted list of bool vars. 
  Self keeps the ownership of the returned instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr Encoding_get_bool_state_vars_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->state_boolean_variables;
}


/**Function********************************************************************

  Synopsis           [Returns the list of boolean variables]

  Description        [Take into consideration to call Encoding_sort_bool_vars
  before calling this method, in order to get a sorted list of bool
  vars.  Sorted or not, first are returned input vars, and after them
  come state vars. 

  WARNING:  The *caller* is responsible for destroying the returned instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr Encoding_get_bool_vars_list(const Encoding_ptr self)
{
  node_ptr list; 
  ENCODING_CHECK_INSTANCE(self);
  list = append_ns(NodeList_to_node_ptr(self->input_boolean_variables), 
		   NodeList_to_node_ptr(self->state_boolean_variables));

  return NodeList_create_from_list(list);
}


/**Function********************************************************************

  Synopsis           [Returns the groups set of input boolean variables]

  Description [After the variables have been encoded, they are
  internally organized in separate groups. All variables appearing in
  the same group should be kept grouped by the specific encoding, to 
  prevent dynamic encoding to wander away them. 
  Self keeps the ownership of the returned instance of GroupsSet]

  SideEffects        []

  SeeAlso            [Encoding_get_bool_state_vars_groups]

******************************************************************************/
GroupSet_ptr Encoding_get_bool_input_vars_groups(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->input_boolean_variables_group;
}


/**Function********************************************************************

  Synopsis           [Returns the groups set of state boolean variables]

  Description        [After the variables have been encoded, they are
  internally organized in separate groups. All variables appearing in
  the same group should be kept grouped by the specific encoding, to 
  prevent dynamic encoding to wander away them. 
  Self keeps the ownership of the returned instance of GroupsSet]

  SideEffects        []

  SeeAlso            [Encoding_get_bool_input_vars_groups]

******************************************************************************/
GroupSet_ptr Encoding_get_bool_state_vars_groups(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->state_boolean_variables_group;  
}


/**Function********************************************************************

  Synopsis           [Sorts the internal boolean lists to respect the ordering 
  stated into the given ordering file]

  Description        [This method changes only the lists returned by methods 
  Encoding_get_bool_{,input,state}_vars_list.] 

  SideEffects        [{state,input}_boolean_variables might change]

  SeeAlso            []

******************************************************************************/
void Encoding_sort_bool_vars(Encoding_ptr self, const char* input_order_file)
{

  GroupSet_ptr input_groups;
  GroupSet_ptr state_groups;
  GroupSet_ptr the_groups_set;

  NodeList_ptr new_input_bool;
  NodeList_ptr new_state_bool;
  NodeList_ptr ord_vars;
  ListIter_ptr iter;
  ParserOrd_ptr parser;
  FILE* f;

  if (input_order_file == (char*) NULL)  return; 

  input_groups = GroupSet_create();
  state_groups = GroupSet_create();
  the_groups_set = GROUP_SET(NULL);

  parser = ParserOrd_create();
  f = fopen(input_order_file, "r");
  if (f == (FILE*) NULL) error_file_not_found(input_order_file);
  ParserOrd_parse_from_file(parser, f);
  fclose(f);

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "Ordering boolean variables...\n");
  }

  /* Iterates on the list A coming from the ordering file: 
     If the symbol is a bool var: append to the ord list B.
     If the symbol is a scalar var:
       Iterates on the encoding, adding any bool var that do not
       belong both to A and B.

     Any duplicated symbol will be warned. Any non-defined symbol will
     be warned as well.

     After that, any remaining defined boolean var will be added. */

  new_input_bool = NodeList_create();
  new_state_bool = NodeList_create();

  ord_vars = ParserOrd_get_vars_list(parser);
  iter = NodeList_get_first_iter(ord_vars);
  while (! ListIter_is_end(iter)) {
    node_ptr name = NodeList_get_elem_at(ord_vars, iter);
    NodeList_ptr the_list;

    if (! Encoding_is_symbol_var(self, name)) {
      warning_variable_not_declared(name);
      iter = ListIter_get_next(iter);
      continue;
    }

    if (Encoding_is_symbol_input_var(self, name)) {
      the_list = new_input_bool;
      the_groups_set = input_groups;
    }
    else {
      the_list = new_state_bool;
      the_groups_set = state_groups;      
    }

    if (Encoding_is_symbol_boolean_var(self, name)) {
      if (! NodeList_belongs_to(the_list, name)) {
	NodeList_ptr group = NodeList_create();

	NodeList_append(the_list, name);
	NodeList_append(group, name);

	/* single var per group: */
	GroupSet_add_group(the_groups_set, group);
      }
      else {
	warning_var_appear_twice_in_order_file(name);
      }
    }
    else {
      /* Variable is scalar. If one or more bits of that scalar var
         have been previously specified in the ordering file, than the
         single bits that belong to the scalar variable will NOT be
         grouped.  If no bit has been previously specified, than all
         the bits of the scalar var that have not been possibly
         specified in the ordering file will be grouped */
      ListIter_ptr iter2; 
      boolean grouped = true;
      NodeList_ptr bits; 
      NodeList_ptr group = NODE_LIST(NULL); 

      bits = Encoding_get_var_encoding_bool_vars(self, name);
      
      /* searches for previously specified bits, and sets the specific
	 flag for grouping: */
      iter2 = NodeList_get_first_iter(bits);
      while (! ListIter_is_end(iter2)) {
	node_ptr bit;

	bit = NodeList_get_elem_at(bits, iter2);
	if (NodeList_belongs_to(the_list, bit)) {
	  grouped = false;
	  break;
	}

	iter2 = ListIter_get_next(iter2);
      }

      /* adds all bits that do not occur in the ordering file, 
	 either grouping or not depending on specific flag: */
      iter2 = NodeList_get_first_iter(bits);
      while (! ListIter_is_end(iter2)) {
	node_ptr bit;
	bit = NodeList_get_elem_at(bits, iter2);
	
	if ( (group == NODE_LIST(NULL)) || ! grouped ) {
	  group = NodeList_create();
	  GroupSet_add_group(the_groups_set, group);
	}

	if (! NodeList_belongs_to(ord_vars, bit) 
	    && ! NodeList_belongs_to(the_list, bit)) {
	  NodeList_append(the_list, bit);
	  NodeList_append(group, bit);
	}

	iter2 = ListIter_get_next(iter2);
      }

      NodeList_destroy(bits);
    }
	
    iter = ListIter_get_next(iter);
  } /* loop on ordering file's vars */
  
  ParserOrd_destroy(parser);
  

  /* Finally add any 'forgotten' bool var, emitting a warning. 
     Also, all existing groups (default groups) have scrutinized, 
     and all variable belonging to a group that do not appear in the 
     ordering file will be put into a new group. 
     The idea is to keep grouped all previously grouped vars that do not 
     appear in the ordering file */
  {
    NodeList_ptr not_in_ord = NodeList_create();
    GroupSet_ptr groups_list[] = { self->input_boolean_variables_group, 
				   input_groups, 
				   self->state_boolean_variables_group, 
				   state_groups };

    NodeList_ptr vars_list[] = { new_input_bool, 
				  new_state_bool };
    int i;

    for (i=0; i < (sizeof(vars_list) / sizeof(vars_list[0])); ++i) {
      ListIter_ptr gr_iter;

      gr_iter = GroupSet_get_first_iter(groups_list[i*2]);
      while (! ListIter_is_end(gr_iter)) {
	NodeList_ptr group;
	ListIter_ptr var_iter;
	NodeList_ptr new_group = NodeList_create();

	group = GroupSet_get_group(groups_list[i*2], gr_iter);
	
	var_iter = NodeList_get_first_iter(group);
	while (! ListIter_is_end(var_iter)) {
	  node_ptr name; 

	  name = NodeList_get_elem_at(group, var_iter);
	  if (!NodeList_belongs_to(vars_list[i], name)) {
	    /* this variable appears in the model, or it is the
	       product of encoding (i.e. is a bit) but it does not
	       appear in the ordering file */
	    NodeList_append(new_group, name);
	    NodeList_append(vars_list[i], name);

	    if (!NodeList_belongs_to(self->det_boolean_variables, name)) {
	      NodeList_append(not_in_ord, name); /* used for final warning */
	    }
	  }

	  var_iter = ListIter_get_next(var_iter);
	}

	/* stores the new group if useful: */
	if (NodeList_get_length(new_group) > 0) {
	  GroupSet_add_group(groups_list[i*2+1], new_group);
	} 
	else { NodeList_destroy(new_group); }

	gr_iter = ListIter_get_next(gr_iter);
      }
    }

    if (NodeList_get_length(not_in_ord) > 0) {
      warning_missing_variables(not_in_ord);
    }

    NodeList_destroy(not_in_ord);
  }

  /* substitutes boolean vars list: */
  NodeList_destroy(self->input_boolean_variables);
  NodeList_destroy(self->state_boolean_variables);
  self->input_boolean_variables = new_input_bool;
  self->state_boolean_variables = new_state_bool;

  /* updates input vars list with the determ vars: */
  iter = NodeList_get_first_iter(self->det_boolean_variables);
  while (!ListIter_is_end(iter)) {
    node_ptr name = NodeList_get_elem_at(self->det_boolean_variables, iter);
    if (!NodeList_belongs_to(self->input_boolean_variables, name)) {
      NodeList_append(self->input_boolean_variables, name);
    }

    iter = ListIter_get_next(iter);
  }

  /* substitutes var groups: */
  GroupSet_destroy(self->input_boolean_variables_group);
  GroupSet_destroy(self->state_boolean_variables_group);
  self->input_boolean_variables_group = input_groups;
  self->state_boolean_variables_group = state_groups;

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "Ordering of boolean variables accomplished.\n");
  }
  
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr Encoding_get_defines_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  return self->define_symbols;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr Encoding_lookup_symbol(const Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  return EncCache_lookup_symbol(self->cache, name);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Encoding_is_symbol_declared(const Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  return EncCache_is_symbol_declared(self->cache, name);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Encoding_is_symbol_var(const Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  return EncCache_is_symbol_var(self->cache, name);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Encoding_is_symbol_constant(const Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  return EncCache_is_constant_defined(self->cache, name);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Encoding_is_symbol_determ_var(const Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  return NodeList_belongs_to(self->det_boolean_variables, name);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Encoding_is_symbol_state_var(const Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  return EncCache_is_symbol_state_var(self->cache, name);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Encoding_is_symbol_boolean_var(const Encoding_ptr self, node_ptr name)
{
  node_ptr enc;
  ENCODING_CHECK_INSTANCE(self);

  enc = Encoding_get_var_encoding(self, name);
  return enc == boolean_type;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Encoding_is_symbol_input_var(const Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  return EncCache_is_symbol_input_var(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Returns true if given symbol is an input variable that 
  belongs to the model, and it is not an internal input var, such as 
  a determinization var]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Encoding_is_symbol_model_input_var(const Encoding_ptr self, 
					   node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  return NodeList_belongs_to(self->model_input_variables, name);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Encoding_is_symbol_define(const Encoding_ptr self, node_ptr name)
{
  ENCODING_CHECK_INSTANCE(self);
  return EncCache_is_symbol_define(self->cache, name);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Encoding_is_constant_defined(const Encoding_ptr self, 
				     node_ptr constant)
{
  ENCODING_CHECK_INSTANCE(self);
  return EncCache_is_constant_defined(self->cache, constant);
}



/**Function********************************************************************

  Synopsis           [Succeeds if varlist contains input variables]

  Description        [Iterates through the elements in var_list
  checking each one to see if it is an input variable.]

  SideEffects        []

  See Also           []

******************************************************************************/
boolean Encoding_list_contains_input_vars(Encoding_ptr self, 
					  NodeList_ptr var_list)
{
  boolean res = false;
  ListIter_ptr iter;  

  ENCODING_CHECK_INSTANCE(self);

  iter = NodeList_get_first_iter(var_list);
  while (! ListIter_is_end(iter)) {
    node_ptr var;

    var = NodeList_get_elem_at(var_list, iter);
    res = Encoding_is_symbol_input_var(self, var);

    if (res) break;
    iter = ListIter_get_next(iter);
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Succeeds if varlist contains state variables]

  Description        [Iterates through the elements in var_list
  checking each one to see if it is a state variable.]

  SideEffects        []

  See Also           []

******************************************************************************/
boolean Encoding_list_contains_state_vars(Encoding_ptr self, 
					  NodeList_ptr var_list)
{
  boolean res = false;
  ListIter_ptr iter;  

  ENCODING_CHECK_INSTANCE(self);

  iter = NodeList_get_first_iter(var_list);
  while (! ListIter_is_end(iter)) {
    node_ptr var;

    var = NodeList_get_elem_at(var_list, iter);
    res = Encoding_is_symbol_state_var(self, var);

    if (res) break;
    iter = ListIter_get_next(iter);
  }

  return res;
}

/**Function********************************************************************

  Synopsis           [Returns the list of all symbols occurring in the model 
  that refer directly or indirectly to state variables]

  Description        [Returned list belongs to self, do not destroy it]

  SideEffects        []

  See Also           []

******************************************************************************/
NodeList_ptr Encoding_get_model_state_symbols_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  
  encoding_refill_symbols_lists(self);
  return self->model_state_symbols;
}


/**Function********************************************************************

  Synopsis           [Returns the list of all symbols occurring in the model 
  that refer directly or indirectly to input variables]

  Description        [Returned list belongs to self, do not destroy it]

  SideEffects        []

  See Also           []

******************************************************************************/
NodeList_ptr Encoding_get_model_input_symbols_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);
  
  encoding_refill_symbols_lists(self);
  return self->model_input_symbols;
}


/**Function********************************************************************

  Synopsis           [Returns the list of all symbols occurring in the model 
  that refer directly or indirectly to combinatory state/input parts 
  (currently, only to DEFINES whose body contains both state and input 
  vars)]

  Description        [Returned list belongs to self, do not destroy it]

  SideEffects        []

  See Also           []

******************************************************************************/
NodeList_ptr 
Encoding_get_model_state_input_symbols_list(const Encoding_ptr self)
{
  ENCODING_CHECK_INSTANCE(self);

  encoding_refill_symbols_lists(self);
  return self->model_state_input_symbols;
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
static void encoding_init(Encoding_ptr self)
{
  int j;
  NodeList_ptr* lists[] = { & self->model_state_symbols, 
			    & self->model_input_symbols,
			    & self->model_state_input_symbols, 
                            & self->input_variables, 
			    & self->model_input_variables, 
			    & self->input_boolean_variables, 
			    & self->det_boolean_variables, 
			    & self->constants_list, 
			    & self->state_variables, 
			    & self->state_boolean_variables, 
			    & self->define_symbols, 		       
			    & self->all_variables, 
			    & self->all_symbols };    
  
  for (j = 0; j < (sizeof(lists)/sizeof(lists[0])); ++j) {
    *lists[j] = NodeList_create();
  }

  /* groups: */
  self->input_boolean_variables_group = GroupSet_create();
  self->state_boolean_variables_group = GroupSet_create();

  self->cache = EncCache_create();  
  self->saved = false;

  self->refill_symbols_lists = true;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void encoding_deinit(Encoding_ptr self)
{
  int j;
  NodeList_ptr lists[] = { self->model_state_symbols, 
			   self->model_input_symbols,
			   self->model_state_input_symbols, 
                           self->input_variables, 
			   self->model_input_variables, 
			   self->input_boolean_variables, 
			   self->det_boolean_variables, 
			   self->constants_list, 
			   self->state_variables, 
			   self->state_boolean_variables, 
			   self->define_symbols, 		       
			   self->all_variables, 
			   self->all_symbols };    

  /* you are trying to destroy an instance whose status has been pushed, 
     but is still to be popped */
  nusmv_assert(!self->saved);

  EncCache_destroy(self->cache);

  /* groups */ 
  GroupSet_destroy(self->input_boolean_variables_group);
  GroupSet_destroy(self->state_boolean_variables_group);

  for (j=0; j < (sizeof(lists)/sizeof(lists[0])); ++j) {
    NodeList_destroy(lists[j]);
  }
}


/**Function********************************************************************

  Synopsis           [Internal version of Encoding_declare_input_var, 
  to be called internally]

  Description        [It does what Encoding_declare_input_var does, but 
  id does not add given symbol to the set of model input variables]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void encoding_declare_input_var(Encoding_ptr self, node_ptr var, 
				       node_ptr range)
{
  node_ptr def;

  def = EncCache_lookup_symbol(self->cache, var);
  if (def != (node_ptr) NULL) {
    internal_error("Encoding_declare_input_var: Variable already declared\n");
  }
  
  if (range == boolean_range) {
    NodeList_append(self->input_boolean_variables, var);
  }
  
  if (! Encoding_is_var_bit(self, var)) {
    NodeList_append(self->input_variables, var);
    NodeList_append(self->all_variables, var);
    NodeList_append(self->all_symbols, var);
  }

  EncCache_new_input_var(self->cache, var, range);
}


/**Function********************************************************************

  Synopsis           [Encodes a scalar variable, by creating all boolean 
  vars (bits) needed to encode the var itself.]

  Description        [All bits will be grouped together]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr 
encoding_encode_scalar_var(Encoding_ptr self, node_ptr name, int suffix, 
			   node_ptr range, boolean is_input, 
			   NodeList_ptr group)
{
  node_ptr var, left, right;

  /* Final case: we reached a leaf */
  if (cdr(range) == Nil) {
    var = find_atom(car(range));
    if (! Encoding_is_constant_defined(self, var)) {
      Encoding_declare_constant(self, var);
    }
    return var;
  }

  /* Intermediate case, declare the scalar variable */
  var = find_node(BIT, name, (node_ptr) suffix);
  if (! Encoding_is_symbol_var(self, var)) {
    if (is_input) { encoding_declare_input_var(self, var, boolean_range); }
    else { Encoding_declare_state_var(self, var, boolean_range); }
    encoding_set_var_encoding(self, var, boolean_type);
  }

  /* Add the bit to the same group: */
  if ( ! NodeList_belongs_to(group, var)) {
    NodeList_append(group, var);
  }

  /* Finally construct the sub binary tree, by decomposing left and 
     right sides: */
  left  = encoding_encode_scalar_var(self, name, suffix + 1, 
				     even_elements(range), is_input, group);
  right = encoding_encode_scalar_var(self, name, suffix + 1, 
				     odd_elements(range), is_input, group);

  return find_node(IFTHENELSE, find_node(COLON, var, left), right);
}


/**Function********************************************************************

  Synopsis           [Sets the symbolic encoding for a given variable that has
  already been added]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void encoding_set_var_encoding(Encoding_ptr self, node_ptr var, 
				      node_ptr encoding)
{
  node_ptr symb; 

  nusmv_assert( (encoding == boolean_type)
		|| (node_get_type(encoding) == ATOM)
		|| (node_get_type(encoding) == IFTHENELSE) );

  nusmv_assert(EncCache_is_symbol_var(self->cache, var)); 

  symb = EncCache_lookup_symbol(self->cache, var);
  setcar(symb, encoding);
}


/**Function********************************************************************

  Synopsis [Fills the given list with the BIT vars which appears into the 
  given var encoding]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void encoding_traverse_encoding(const Encoding_ptr self, 
				       node_ptr tree, NodeList_ptr list)
{
  node_ptr bit;
  
  /* constant or number terminate (numbers are not stored as constants): */
  if ( Encoding_is_symbol_constant(self, tree) 
       || (node_get_type(tree) == NUMBER) ) return;

  /* no other kind of node can appear at this level: */
  nusmv_assert(node_get_type(tree) == IFTHENELSE);

  bit = car(car(tree));
  if (! NodeList_belongs_to(list, bit)) {
    NodeList_append(list, bit);
  }
      
  encoding_traverse_encoding(self, cdr(car(tree)), list); /* 'then' */
  encoding_traverse_encoding(self, cdr(tree), list);      /* 'else' */
}  


/**Function********************************************************************

  Synopsis           [Used to update symbols lists when a define is defined. 
  Given define will be added to one of state, input, state-input symbols 
  lists.]

  Description        [It might be the case that given define contains symbols 
  that are not defined yet. This can happen during the flattening phase, that 
  declares defines and variables. If given define contains references that 
  have not been declared yet, than 1 is returned, otherwise 0 is returned.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean encoding_assign_define_body_to_symbols_lists(Encoding_ptr self, 
							    node_ptr name)
{ 
  boolean state, input, constant; 
  NodeList_ptr deps;
  node_ptr def;

  /* checks if already added to symbols lists: */
  if (NodeList_belongs_to(self->model_state_symbols, name)) return 0;
  if (NodeList_belongs_to(self->model_input_symbols, name)) return 0;
  if (NodeList_belongs_to(self->model_state_input_symbols, name)) return 0;

  def = Flatten_GetDefinition(self, name);
  deps = Compile_get_expression_dependencies(self, def);

  state = Encoding_list_contains_state_vars(self, deps);
  input = Encoding_list_contains_input_vars(self, deps);

  constant = (!(state || input)) && (def != (node_ptr) NULL);

  /* if !(state || input) then this define will be recalculated 
     when symbols list will be requested later */
  if (state && input)  NodeList_append(self->model_state_input_symbols, name);
  else if (state)  NodeList_append(self->model_state_symbols, name);   
  else if (input)  NodeList_append(self->model_input_symbols, name);   
  else if (constant) NodeList_append(self->model_state_symbols, name);

  self->refill_symbols_lists = true;
  NodeList_destroy(deps);

  return !(state || input || constant);
}    


/**Function********************************************************************

  Synopsis           [Reprocesses declared DEFINES and assign them to different 
  symbols lists, depending on the content of their bodies. ]

  Description        [This methods does the work only if required by internal 
  state]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void encoding_refill_symbols_lists(Encoding_ptr self)
{
  if (self->refill_symbols_lists) {
    NodeList_ptr defines;
    ListIter_ptr iter;
    
    defines = Encoding_get_defines_list(self);
    iter = NodeList_get_first_iter(defines);
    while (!ListIter_is_end(iter)) {
      node_ptr define;
      boolean res;

      define = NodeList_get_elem_at(defines, iter);

      if (cdr(define) == running_atom) {
	/* do not include 'running' DEFINE */
	iter = ListIter_get_next(iter);
	break;
      }

      res = encoding_assign_define_body_to_symbols_lists(self, define);

      /* at this step any occuring symbol within the define body must
	 be declared */
      nusmv_assert(!res); 

      iter = ListIter_get_next(iter);
    }
    
    self->refill_symbols_lists = false;
  }
}
