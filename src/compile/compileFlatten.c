/**CFile***********************************************************************

  FileName    [compileFlatten.c]

  PackageName [compile]

  Synopsis    [Flattening of the model.]

  Description [Performs the flattening of the model. We start from the
  module <code>main</code> and we recursively instantiate all the modules
  or processes declared in it.<br>
  Consider the following example:
  <blockquote>
  MODULE main<br>
   ...<br>
   VAR<br>
     a : boolean;<br>
     b : foo;<br>
   ...<br><br>

  MODULE foo<br>
   VAR <br>
     z : boolean;<br>
   ASSIGN<br>
     z := 1;<br>
  </blockquote>
  The flattening instantiate the module foo in the <code>main</code>
  module. You can refer to the variables "<code>z</code>" declared in the
  module <code>foo</code> after the flattening by using the dot notation
  <code>b.z</code>.]

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2.
  Copyright (C) 1998-2005 by CMU and ITC-irst.

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

  For more information on NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

******************************************************************************/

#include "compileInt.h"
#include "utils/ustring.h"
#include "utils/assoc.h"
#include "parser/symbols.h"
#include "parser/psl/pslNode.h"
#include "utils/error.h"
#include "enc/enc.h"
#include "enc/symb/Encoding.h"


static char rcsid[] UTIL_UNUSED = "$Id: compileFlatten.c,v 1.25.4.30.2.7 2005/07/07 16:45:28 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef enum {
  State_Variables_Instantiation_Mode,
  Input_Variables_Instantiation_Mode
} Instantiation_Variables_Mode_Type;

typedef enum {
  Get_Definition_Mode,
  Expand_Definition_Mode
} Definition_Mode_Type;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [Body of define in evaluation]

  Description [Indicates that the body of a define is under the
  flattening, it is usde to discover possible recursive definitions.]

  SideEffects []

  SeeAlso      [Flatten_GetDefinition]

******************************************************************************/
#define BUILDING_FLAT_BODY (node_ptr)-11


/**Variable********************************************************************

  Synopsis    [Contains the list of process names.]

  Description [The list of process names. It represents the range of the
  <tt>PROCESS_SELECTOR_VAR_NAME</tt> input variable.]

******************************************************************************/
static node_ptr process_selector_range = Nil;
 
/**Variable********************************************************************

  Synopsis    [The internal name of the <tt>PROCESS_SELECTOR_VAR_NAME</tt>
  input variable.]

  Description [Stores the internal name of the
  <tt>PROCESS_SELECTOR_VAR_NAME</tt> input variable. It is a
  contextualized variable (i.e if
  <tt>PROCESS_SELECTOR_VAR_NAME="_process_selector_"</tt>, then the
  value of this variable is: "<tt>main._process_selector_</tt>.]

******************************************************************************/
node_ptr proc_selector_internal_vname = Nil;

/**Variable********************************************************************

  Synopsis    [Contains the list of process.running names.]

  Description [The list of process.running defined symbols.]

******************************************************************************/
node_ptr process_running_symbols = Nil;

/**Variable********************************************************************

  Synopsis    [The mode to perform variable instantiation.]

  Description [Depending the value of this variable we perform
  instantiation of state variables or input variables.]

******************************************************************************/
static Instantiation_Variables_Mode_Type variable_instantiate_mode = 
      State_Variables_Instantiation_Mode;

void set_variable_instantiation_to_input () {
  variable_instantiate_mode = Input_Variables_Instantiation_Mode;
}
static void set_variable_instantiation_to_state (void) {
  variable_instantiate_mode = State_Variables_Instantiation_Mode;
}
static int variable_instantiation_is_input (void) {
  return(variable_instantiate_mode == Input_Variables_Instantiation_Mode);
}

/**Variable********************************************************************

  Synopsis    [The expansion mode for definitions is sexp flattening.]

  Description [Depending on the value of this variable, a definition
  is expanded or not during the flattening of a sexp.]

******************************************************************************/
static Definition_Mode_Type definition_mode = Get_Definition_Mode;

void set_definition_mode_to_get () {
  definition_mode = Get_Definition_Mode;
}
void set_definition_mode_to_expand () {
  definition_mode = Expand_Definition_Mode;
}
int definition_mode_is_expand (void) {
  return(definition_mode == Expand_Definition_Mode);
}


/**Variable********************************************************************

  Synopsis    [The hash containing the definition of each module read in.]

  Description [This hash uses the name of the module as index, and for
  each module it stores the following data structure:<br>
  <center><code>&lt;LAMBDA , arguments, module_body&gt;</code></center><br>
  I.e. it is a node, whose type is <code>LAMBDA</code> and whose "car" are
  the module arguments and the "cdr" is the module body (i.e. normal
  assignments, init assignments and next assignments.
  ]

******************************************************************************/
static hash_ptr module_hash;
void init_module_hash()
{
  /* Auxiliary variable used to traverse the parse tree. */
  node_ptr m;
  /* The parse tree representing the input files. */
  extern node_ptr parse_tree;

  module_hash = new_assoc();
  m = parse_tree;
  while (m != Nil) {
    node_ptr cur_module = car(m);

    if (node_get_type(cur_module) == MODULE) {
      node_ptr name = find_atom(car(car(cur_module)));
      node_ptr params = cdr(car(cur_module));
      node_ptr def = cdr(cur_module);

      if (find_assoc(module_hash, name)) error_redefining(name);
      insert_module_hash(name, new_node(LAMBDA, params, reverse(def)));
    }
    m = cdr(m);
  }
}
void insert_module_hash(node_ptr x, node_ptr y) { insert_assoc(module_hash, x, y);}
node_ptr lookup_module_hash(node_ptr x) {return(find_assoc(module_hash, x));}
void clear_module_hash() {if (module_hash != NULL) clear_assoc(module_hash);}

/**Variable********************************************************************

  Synopsis    [The <code>param_hash</code> associates actual to formal
  paramaters of a module.]

  Description [This hash is used by <code>make_params</code> to detect
  multiple substitution for parameters and to perform substitution of
  formal parameters with actual parameters.]

  SeeAlso     [make_params]

******************************************************************************/
static hash_ptr param_hash;
void init_param_hash() { param_hash = new_assoc(); }
void insert_param_hash(node_ptr x, node_ptr y) { insert_assoc(param_hash, x, y);}
node_ptr lookup_param_hash(node_ptr x) {return(find_assoc(param_hash, x));}
void clear_param_hash() {clear_assoc(param_hash);}

/**Variable********************************************************************

  Synopsis    [The <code>flatten_constant_hash</code> hash to keep track of
  constants.]

  Description [This hash is used during flattening to discriminate among contant atoms and variable.]

  SeeAlso     [Flatten_GetDefinition]

******************************************************************************/
static hash_ptr flatten_constant_hash;
void init_flatten_constant_hash() { flatten_constant_hash = new_assoc(); }
void insert_flatten_constant_hash(node_ptr x, node_ptr y) { insert_assoc(flatten_constant_hash, x, y);}
node_ptr lookup_flatten_constant_hash(node_ptr x) {return(find_assoc(flatten_constant_hash, x));}
void clear_flatten_constant_hash() {clear_assoc(flatten_constant_hash);}
static void update_flatten_constant_hash(node_ptr range) 
{
  while (range != Nil) {
    node_ptr name = car(range);
    
    if (node_get_type(name) == DOT) {
      name = CompileFlatten_resolve_name(name, Nil);
    }
    else {
      name = find_atom(name);
    }
    insert_flatten_constant_hash(name, name);
    range = cdr(range);
  }
}

/**Variable********************************************************************

  Synopsis    [The hash of flatten_def]

  Description [This hash associates to an atom corresponding to a
  defined symbol the corresponding flattened body.]

******************************************************************************/
static hash_ptr flatten_def_hash = (hash_ptr)NULL;
void init_flatten_def_hash() {
  flatten_def_hash = new_assoc();
  nusmv_assert(flatten_def_hash != (hash_ptr)NULL);
}
void insert_flatten_def_hash(node_ptr key, node_ptr value) {
  nusmv_assert(flatten_def_hash != (hash_ptr)NULL);
  insert_assoc(flatten_def_hash, key, (node_ptr)value);
}
node_ptr lookup_flatten_def_hash(node_ptr key) {
  nusmv_assert(flatten_def_hash != (hash_ptr)NULL);
  return((node_ptr)find_assoc(flatten_def_hash, key));
}

static assoc_retval flatten_def_hash_free(char *key, char *data, char * arg) {
  node_ptr element = (node_ptr)data;

  /* Notice that this hash may contain elements set to
     BUILDING_FLAT_BODY in cases of errors inside the flattening
     procedure */
  if (element != (node_ptr)NULL && element != BUILDING_FLAT_BODY) {
    free_node(element);
  }

  return(ASSOC_DELETE);
}

void clear_flatten_def_hash() 
{
  clear_assoc_and_free_entries(flatten_def_hash, flatten_def_hash_free);
}

/**Variable********************************************************************

  Synopsis    [The hash of assign_db]

  Description [This hash associates to an atom corresponding to a
  defined symbol the corresponding flattened body.]

******************************************************************************/
static hash_ptr assign_db_hash = (hash_ptr)NULL;
void init_assign_db_hash() {
  assign_db_hash = new_assoc();
  nusmv_assert(assign_db_hash != (hash_ptr)NULL);
}
void insert_assign_db_hash(node_ptr key, node_ptr value) {
  nusmv_assert(assign_db_hash != (hash_ptr)NULL);
  insert_assoc(assign_db_hash, key, (node_ptr)value);
}
node_ptr lookup_assign_db_hash(node_ptr key) {
  nusmv_assert(assign_db_hash != (hash_ptr)NULL);
  return((node_ptr)find_assoc(assign_db_hash, key));
}

static assoc_retval assign_db_hash_free(char *key, char *data, char * arg) {
  node_ptr element = (node_ptr)data;

  if (element != (node_ptr)NULL) {
    free_node(element);
  }
  return(ASSOC_DELETE);
}

void clear_assign_db_hash() {clear_assoc_and_free_entries(assign_db_hash, assign_db_hash_free);}

/**Variable********************************************************************

  Synopsis    [Variable containing the current context in the
  instantiation phase.]

  Description [Variable containing the current context in the
  instantiation phase. It is used in the instantiation of the
  arguments of modules or processes.]

******************************************************************************/
static node_ptr param_context = Nil;

/**Variable********************************************************************

  Synopsis    [The stack containing the nesting for modules.]

  Description [This variable contains the nesting of modules. It is
  used in the instantiation phase to check for recursively defined modules.]

******************************************************************************/
static node_ptr module_stack = Nil;



/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void 
compileFlattenHierarchy ARGS((Encoding_ptr, 
			      node_ptr, node_ptr, node_ptr *, node_ptr *, 
			      node_ptr *, node_ptr *, node_ptr *, 
			      node_ptr *, node_ptr *, node_ptr *, node_ptr *, 
			      node_ptr *, node_ptr *, node_ptr *, node_ptr));

static void 
instantiate ARGS((Encoding_ptr, node_ptr, node_ptr, node_ptr *, node_ptr *, 
		  node_ptr *, node_ptr *, node_ptr *, node_ptr *, node_ptr *, 
		  node_ptr *, node_ptr *, node_ptr *, node_ptr *, node_ptr *, 
		  node_ptr));

static void 
instantiate_by_name ARGS((Encoding_ptr senc, node_ptr, node_ptr, node_ptr *, 
			  node_ptr *, node_ptr *, node_ptr *, node_ptr *, 
			  node_ptr *, node_ptr *, node_ptr *, node_ptr *, 
			  node_ptr *, node_ptr *, node_ptr *, node_ptr));

static node_ptr put_in_context ARGS((node_ptr));

static node_ptr 
Flatten_FlattenSexpRecur ARGS((const Encoding_ptr senc, node_ptr, node_ptr));

static void 
compileFlattenProcessRecur ARGS((Encoding_ptr senc, node_ptr, 
				 node_ptr, node_ptr));

static void 
compileFlattenSexpModelAux ARGS((Encoding_ptr senc, node_ptr, int));

static void 
compileFlattenSexpModelRecur ARGS((Encoding_ptr senc, node_ptr, int));

static Set_t 
compileFlattenConstantSexpCheck ARGS((Encoding_ptr, node_ptr, int));

static Expr_ptr 
compile_flatten_resolve_name_recur ARGS((Expr_ptr n, node_ptr context));

static void 
create_process_symbolic_variables ARGS((Encoding_ptr senc, node_ptr));

static node_ptr make_atom_set ARGS((node_ptr));

static node_ptr mk_true ARGS((void));
static node_ptr mk_false ARGS((void));
static node_ptr mk_and ARGS((node_ptr, node_ptr));
static char * type_to_string ARGS((int));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Traverse the module hierarchy, extracts the informations and flatten the hierarchy.]

  Description        [Traverses the module hierarchy and extracts the
  information needed to compile the automaton. The hierarchy of modules
  is flattened, the variables are contextualized, the various parts of
  the model read in are extracted (i.e. the formulae to be verified,
  the initial expressions, ...)...<br>
  Moreover all these informations are flattened.]

  SideEffects        [None]

  SeeAlso            [compileFlattenHierarchy]

******************************************************************************/
void Compile_FlattenHierarchy(
 Encoding_ptr senc, /* the symbolic encoding */			      
 node_ptr root_name /* the <code>ATOM</code> representing the module at 
		       the top of the hierarchy. */,
 node_ptr name /* the name of the module at the top of the hierarchy. */,
 node_ptr *trans /* the list of TRANS actually recognized */,
 node_ptr *init /* the list of INIT actually recognized */,
 node_ptr *invar /* the list of INVAR actually recognized */,
 node_ptr *spec /* the list of SPEC actually recognized */,
 node_ptr *compute /* the list of COMPUTE actually recognized */,
 node_ptr *ltl_spec /* the list of LTLSPEC actually recognized */,
 node_ptr *psl_spec /* the list of PSLSPEC actually recognized */,
 node_ptr *invar_spec /* the list of INVARSPEC actually recognized */,
 node_ptr *justice /* the list of JUSTICE actually recognized */,
 node_ptr *compassion /* the list of COMPASSION actually recognized */,
 node_ptr *procs /* the list of processes actually recognized */,
 node_ptr actual /* the actual module arguments */)
{
  node_ptr trans_expr      = Nil;
  node_ptr init_expr       = Nil;
  node_ptr invar_expr      = Nil;
  node_ptr spec_expr       = Nil;
  node_ptr compute_expr    = Nil;
  node_ptr ltl_expr        = Nil;
  node_ptr psl_expr        = Nil;
  node_ptr invars_expr     = Nil;
  node_ptr justice_expr    = Nil;
  node_ptr compassion_expr = Nil;
  node_ptr assign_expr     = Nil;
  node_ptr procs_expr      = Nil;

  compileFlattenHierarchy(senc, 
			  root_name, name, &trans_expr, &init_expr,
                          &invar_expr, &spec_expr, &compute_expr,
                          &ltl_expr, &psl_expr, &invars_expr, &justice_expr,
                          &compassion_expr, &assign_expr, &procs_expr, actual);
  /* Before flattening processes process variable must be created */
  create_process_symbolic_variables(senc, procs_expr);

  *trans      = Compile_FlattenSexp(senc, trans_expr, name);
  *init       = Compile_FlattenSexp(senc, init_expr, name);
  *invar      = Compile_FlattenSexp(senc, invar_expr, name);
  /* define this if you want properties falttened earlier, otherwise
     the flattening of properties must be performed further. */
#ifdef EARLY_FLATTEN_PROPERTY
  *spec       = Compile_FlattenSexp(senc, spec_expr, name);
  *compute    = Compile_FlattenSexp(senc, compute_expr, name);
  *ltl_spec   = Compile_FlattenSexp(senc, ltl_expr, name);
  *psl_spec   = psl_expr;
  *invar_spec = Compile_FlattenSexp(senc, invars_expr, name);
#else
  *spec       = spec_expr;
  *compute    = compute_expr;
  *ltl_spec   = ltl_expr;
  *psl_spec   = psl_expr;
  *invar_spec = invars_expr;
#endif
  *justice    = justice_expr;
  *compassion = compassion_expr;

  if (*compassion != Nil) {
    fprintf(nusmv_stdout, 
	    "WARNING *** The model contains COMPASSION declarations.        ***\n"
	    "WARNING *** Full fairness is not yet fully supported in NuSMV. ***\n"
	    "WARNING *** Currently, COMPASSION declarations are only        ***\n"
	    "WARNING *** supported for BDD-based LTL Model Checking.        ***\n"
  	    "WARNING *** Results of CTL Model Checking and of Bounded       ***\n"
	    "WARNING *** Model Checking may be wrong.                       ***\n");
  }

  *procs = compileFlattenProcess(senc, procs_expr);
  compileFlattenSexpModel(senc, *init, *invar, *trans);

  compileCheckForInputVars(senc, *trans, *init, *invar, *procs);
}

/**Function********************************************************************

  Synopsis [Traverse the module tableau, extracts the informations and
  flatten the tableau.]

  Description        [Traverses the module tableau and extracts the
  information needed to compile the automaton. The tableau of modules
  is flattened, the variables are contextualized, the various parts of
  the model read in are extracted (i.e. the formulae to be verified,
  the initial expressions, ...)...<br>
  Moreover all these informations are flattened.]

  SideEffects        [None]

  SeeAlso            [Compile_FlattenHierarchy, compileFlattenHierarchy]

******************************************************************************/
void Compile_FlattenTableau(
 Encoding_ptr senc, /* the symbolic encoding */			    
 node_ptr root_name /* the <code>ATOM</code> representing the module at the top of the hierarchy. */,
 node_ptr name /* the name of the module at the top of the hierarchy. */,
 node_ptr *trans /* the list of TRANS actually recognized */,
 node_ptr *init /* the list of INIT actually recognized */,
 node_ptr *invar /* the list of INVAR actually recognized */,
 node_ptr *spec /* the list of SPEC actually recognized */,
 node_ptr *compute /* the list of COMPUTE actually recognized */,
 node_ptr *ltl_spec /* the list of LTLSPEC actually recognized */,
 node_ptr *psl_spec /* the list of PSLSPEC actually recognized */,
 node_ptr *invar_spec /* the list of INVARSPEC actually recognized */,
 node_ptr *justice /* the list of JUSTICE actually recognized */,
 node_ptr *compassion /* the list of COMPASSION actually recognized */,
 node_ptr *procs /* the list of processes actually recognized */,
 node_ptr actual /* the actual module arguments */)
{
  node_ptr trans_expr      = Nil;
  node_ptr init_expr       = Nil;
  node_ptr invar_expr      = Nil;
  node_ptr spec_expr       = Nil;
  node_ptr compute_expr    = Nil;
  node_ptr ltl_expr        = Nil;
  node_ptr psl_expr        = Nil;
  node_ptr invars_expr     = Nil;
  node_ptr justice_expr    = Nil;
  node_ptr compassion_expr = Nil;
  node_ptr assign_expr     = Nil;
  node_ptr procs_expr      = Nil;

  compileFlattenHierarchy(senc, 
			  root_name, name, &trans_expr, &init_expr,
                          &invar_expr, &spec_expr, &compute_expr,
                          &ltl_expr, &psl_expr, &invars_expr, &justice_expr,
                          &compassion_expr, &assign_expr, &procs_expr, actual);

  *trans      = Compile_FlattenSexp(senc, trans_expr, name);
  *init       = Compile_FlattenSexp(senc, init_expr, name);
  *invar      = Compile_FlattenSexp(senc, invar_expr, name);
  /* define this if you want properties falttened earlier, otherwise
     the flattening of properties must be performed further. */
#ifdef EARLY_FLATTEN_PROPERTY
  *spec       = Compile_FlattenSexp(senc, spec_expr, name);
  *compute    = Compile_FlattenSexp(senc, compute_expr, name);
  *ltl_spec   = Compile_FlattenSexp(senc, ltl_expr, name);
  *psl_spec   = psl_expr;
  *invar_spec = Compile_FlattenSexp(senc, invars_expr, name);
#else
  *spec       = spec_expr;
  *compute    = compute_expr;
  *ltl_spec   = ltl_expr;
  *psl_spec   = psl_expr;
  *invar_spec = invars_expr;
#endif
  *justice    = Compile_FlattenSexp(senc, justice_expr, name);
  *compassion = Compile_FlattenSexp(senc, compassion_expr, name);

  *procs      = procs_expr;
}


/**Function********************************************************************

  Synopsis           [Flatten a hierarchy of SMV processes.]

  Description        [This functions takes in input the list of
  processes resulting from the instantiation step and builds a hash
  table <tt>ASSIGN</tt>that associates to each variable the following
  informations:
  <ul>
    <li><tt>init(var) -> (list_i init_assign_i)</tt><br>
        where <tt>init_assign_i</tt> is the right side of the initialization
        assignement of the variable <tt>var</tt> in process <tt>i</tt>.
    <li><tt>next(var) -> (list_i Pi.running -> next_assign_i)</tt><br>
        where  <tt>next_assign_i</tt> is the right side of the next
        assignement for the variable <tt>var</tt> in process <tt>i</tt>.
        When other processes not affecting the variable are running,
        the variable stutter.
    <li><tt>var -> (list_i Pi.running -> normal_assign_i)</tt><br>
        where  <tt>normal_assign_i</tt> is the right side of the
        normal (invariant) assignement for the variable <tt>var</tt>
        in process <tt>i</tt>. When other variables not affecting the
        variable are running, the variable is free to evolve randomly
        assuming a value among its possible values.
  </ul>]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr compileFlattenProcess(Encoding_ptr senc, node_ptr procs_expr)
{
  node_ptr l = procs_expr;
  node_ptr result = Nil;
  node_ptr running = sym_intern(RUNNING_SYMBOL);

  while (l != Nil) { /* Loops over processes */
    node_ptr process_running;
    node_ptr process_name = car(car(l));
    node_ptr process_assignments = Compile_FlattenSexp(senc, cdr(car(l)), Nil);

    result = cons(cons(process_name, process_assignments), result);

    /* The encoder is not required at this stage */
    process_running = CompileFlatten_resolve_name(running, process_name);
    compileFlattenProcessRecur(senc, process_assignments, Nil, 
			       process_running);
    l = cdr(l);
  }

  return reverse(result);
}


/**Function********************************************************************

  Synopsis           [Builds the flattened version of an expression.]

  Description        [Builds the flattened version of an
  expression. It does not expand defined symbols with the
  corresponding body.]

  SideEffects        []

  SeeAlso            [Flatten_GetDefinition]

******************************************************************************/
node_ptr Compile_FlattenSexp(const Encoding_ptr senc, node_ptr sexp, 
			     node_ptr context) 
{
  node_ptr result;
  set_definition_mode_to_get();
  result = Flatten_FlattenSexpRecur(senc, sexp, context);
  return(result);
}

/**Function********************************************************************

  Synopsis           [Flattens an expression and expands defined symbols.]

  Description        [Flattens an expression and expands defined symbols.]

  SideEffects        []

  SeeAlso            [Flatten_GetDefinition]

******************************************************************************/
node_ptr 
Compile_FlattenSexpExpandDefine(const Encoding_ptr senc, 
				node_ptr sexp, node_ptr context) 
{
  node_ptr result;
  set_definition_mode_to_expand();
  result = Flatten_FlattenSexpRecur(senc, sexp, context);
  return result;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [required]

  Description         [Associate to each variables the small pieces of
  the trans where the variable occurs, in a way similar to
  <tt>Compile_FlattenProcess</tt>]

  SideEffects        []

  SeeAlso            [Compile_FlattenProcess]

******************************************************************************/
void compileFlattenSexpModel(Encoding_ptr senc, 
			     node_ptr init_expr, node_ptr invar_expr,
			     node_ptr trans_expr)
{
  compileFlattenSexpModelAux(senc, init_expr, INIT);
  compileFlattenSexpModelAux(senc, invar_expr, INVAR);
  compileFlattenSexpModelAux(senc, trans_expr, TRANS);
}


/**Function********************************************************************

  Synopsis           [Returns a range going from a to b]

  Description        [Returns a range going from a to b. An empty range (Nil) 
  is returned whether given 'a' is greater than 'b']

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr CompileFlatten_expand_range(int a, int b)
{
  node_ptr res = Nil;

  if ((a == 0) && (b == 1)) res = boolean_range;
  else {
    int i;
    for (i=b ; i>=a ; i--) {
      res = find_node(CONS, find_node(NUMBER, (node_ptr) i, Nil), res);
    }
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Takes an expression representing an identifier
  and recursively evaluates it.]

  Description        [Takes an expression representing an identifier
  and recursively evaluates it. This function is repsonsible of
  building the symbolic name of the given symbol <code>n</code> in
  module instance <code>context</code>. If the given symbol is a formal
  parameter of the module instance, than the actual parameter is
  built. If the given symbol is an array, than the expression
  referring to the element of the array must evaluate to an unique
  integer (not to a set of integers) in the range of array bound.]

  SideEffects        []

  SeeAlso            [compile_flatten_resolve_name_recur]

******************************************************************************/
Expr_ptr CompileFlatten_resolve_name(Expr_ptr expr, node_ptr context)
{
  node_ptr res;
  int temp = yylineno;

  if (expr == EXPR(NULL)) return expr;

  yylineno = node_get_lineno(expr);
  res = compile_flatten_resolve_name_recur(expr, context);
  yylineno = temp;

  return res;
}



/**Function********************************************************************

  Synopsis           [Quits the flattener module]

  Description        [Resets all internal structures, in order to correctly 
  shut down the flattener. Calls clear_* local functions, and reset all 
  private variables. ]

  SideEffects [This module will be initialized, all previously
  iniitalized data will be lost]

  SeeAlso            []

******************************************************************************/
void CompileFlatten_quit_flattener()
{
  clear_param_hash();
  clear_module_hash();
  clear_flatten_def_hash();
  clear_flatten_constant_hash();
  clear_assign_db_hash();
  
  /* ---------------------------------------------------------------------- */
  /*                        Reseting of variables                           */
  /* ---------------------------------------------------------------------- */

  /* lists: */
  free_list(process_selector_range);
  process_selector_range = Nil;  

  free_list(process_running_symbols);
  process_running_symbols = Nil;

  free_list(module_stack);
  module_stack = Nil;

  /* simple nodes: */
  if (proc_selector_internal_vname != Nil) {
    free_node(proc_selector_internal_vname);
    proc_selector_internal_vname = Nil;
  }
  
  if (param_context != Nil) {
    free_node(param_context);
    param_context = Nil;  
  }

  /* other vars: */
  variable_instantiate_mode = State_Variables_Instantiation_Mode;
  definition_mode = Get_Definition_Mode;
}


/**Function********************************************************************

  Synopsis           [Instantiates the given variable.]

  Description        [It takes as input a variable and a context, and
  depending on the type of the variable some operation are performed in order to
  instantiate it in the given context:
  <br><br>
  <ul>
    <li><b>BOOLEAN</b><br>
        if the variable is of type boolean, then we add an entry in
        <code>symbol_hash</code> saying that the variable range is <code>{0,1}</code>.</li>
    <li><b>RANGE</b><br>
        if the variable is a range of the form <code>M..N</code>, then
        we add an entry in the <code>symbol_hash</code> saying that the
        variable range is <code>{M, M+1, ..., N-1, N}</code>. If
        <code>M</code> is less or equal to <code>N</code>, than an error occurs.</li>
    <li><b>SCALAR</b><br>
        if the variable is a scalar variable whose possible values are
        <code>{a1, a2, ..., aN}</code>, then we add an entry in the
        <code>symbol_hash</code> saying that the variable range is
        <code>{a1, ..., aN}</code>. </li>
    <li><b>ARRAY</b><br>
        for each element of the array it is created the corresponding
        symbol. Suppose you have the following definition "<code>VAR
        x : array 1..4 of boolean;</code>". We call this function
        for 4 times, asking at each call <code>i</code> (<code>i</code> from 1
        to 4) to declare the boolean variable <code>x\[i\]</code>.</li>
    <li><b>MODULE</b><br>
        If the variable is an instantiation of a module, than their
        arguments (if any) are contextualized, and passed as actual
        parameter to <code>instantiate_by_name<code> with the name of the
        instantiated module as root name (to extract its definition)
        and as variable name as the name of the module (to perform
        flattening).</li>
    <li><b>PROCESS</b><br>
        If the variable is of type process, than we extract the
        module name and args, we perform the contextualization of the
        process arguments and we perform a call to
        <tt>compileFlattenHierarchy</tt> using the variable name as process
        name (to perform flattening), the module name as root name (to
        extract its definition) and the computed actual parameters.</li>
  </ul><br>
  The variables of type boolean, scalar, and array depending the kind
  of variable instantiation mode are appended to
  <tt>input_variables</tt> or to <tt>state_variables</tt>.
]

  SideEffects        []

  SeeAlso            [instantiate_vars]

******************************************************************************/
void instantiate_var(Encoding_ptr senc, node_ptr name, node_ptr type, 
		     node_ptr *trans, node_ptr *init, node_ptr *invar, 
		     node_ptr *spec, node_ptr *compute, 
		     node_ptr *ltl_spec, node_ptr *psl_spec, 
		     node_ptr *invar_spec, 
		     node_ptr *justice, node_ptr *compassion, 
		     node_ptr *assign, node_ptr *procs, 
		     node_ptr context)
{
  yylineno = node_get_lineno(type);
  if (Encoding_is_symbol_declared(senc, name)) error_redefining(name);

  switch (node_get_type(type)) {

  case BOOLEAN: 
    update_flatten_constant_hash(boolean_range);
    if (variable_instantiation_is_input()) {
      Encoding_declare_input_var(senc, name, boolean_range);
    } else {
      Encoding_declare_state_var(senc, name, boolean_range);
    }
    break;
    
  case TWODOTS: {
    node_ptr expanded_range = Nil;
    int dim1, dim2;
    
    dim1 = (int) car(car(type));
    dim2 = (int) car(cdr(type));

    /* Checks if the range is a "range", a range is from "a" to "b"
       with the constraint that "b >= a" */
    expanded_range = CompileFlatten_expand_range(dim1, dim2);

    if (expanded_range == Nil) { error_empty_range(name, dim1, dim2); }
    update_flatten_constant_hash(expanded_range);

    if (dim1 == dim2) {
      /* Single-value range */
      node_ptr val = car(expanded_range);
      if (!Encoding_is_symbol_constant(senc, val)) {
	Encoding_declare_constant(senc, val);
      }
      Encoding_declare_define(senc, name, context, val);

      fprintf(nusmv_stderr, "WARNING: variable ");
      print_node(nusmv_stderr, name);
      fprintf(nusmv_stderr, " has been encoded as a constant\n");
    }
    else {
      if (variable_instantiation_is_input()) {
        Encoding_declare_input_var(senc, name, expanded_range);
      } else {
        Encoding_declare_state_var(senc, name, expanded_range);
      }
    }
    break;
  }

  case SCALAR: {
    update_flatten_constant_hash(car(type));

    /* Checks whether the variable has a single-value range. In that
       case it creates a new define, instead of a new variable.  The
       user is warned about this action. */
    if (cdr(car(type)) == Nil) {
      /* Single-value range */
      node_ptr val = car(car(type));
      if (!Encoding_is_symbol_constant(senc, val)) {
	Encoding_declare_constant(senc, val);
      }
      Encoding_declare_define(senc, name, context, val);

      fprintf(nusmv_stderr, "WARNING: variable ");
      print_node(nusmv_stderr, name);
      fprintf(nusmv_stderr, " has been encoded as a constant\n");
    }
    else {
      if (variable_instantiation_is_input()) {
	Encoding_declare_input_var(senc, name, car(type));
      } 
      else { Encoding_declare_state_var(senc, name, car(type)); }
    }

    break;
  }

  case MODTYPE: {
      node_ptr actual;

      param_context = context;
      actual = map(put_in_context, cdr(type));
      instantiate_by_name(senc, car(type), name, trans, init, invar, spec, 
			  compute, ltl_spec, psl_spec, invar_spec, 
			  justice, compassion, 
			  assign, procs, actual);
      break;
  }

  case PROCESS: {
    node_ptr actual;
    node_ptr pmod_name = car(car(type));
    node_ptr pmod_args = cdr(car(type));

    param_context = context;
    actual = map(put_in_context, pmod_args);

    compileFlattenHierarchy(senc, pmod_name, name, trans, init, invar, spec,
			    compute, ltl_spec, psl_spec, invar_spec, justice, 
			    compassion, assign, procs, actual);
    break;
  }

  case ARRAY: {
    int dim1, dim2;
    int i;
   
    dim1 = (int) car(car(car(type)));
    dim2 = (int) car(cdr(car(type)));
   
    for (i=dim1; i<=dim2; ++i) {
      node_ptr index = find_node(NUMBER, (node_ptr) i, Nil);
      insert_flatten_constant_hash(index, index);
      /* Creates the name[i] variable */
      instantiate_var(senc, find_node(ARRAY, name, index),
                      cdr(type), trans, init, invar, spec, compute, ltl_spec,
                      psl_spec, invar_spec, justice, compassion, assign, 
		      procs, context);
    }
    break;
  }

  default:
    internal_error("instantiate_vars: type = %d", node_get_type(type));
  }

}


/**Function********************************************************************

  Synopsis           [Recursively applies <tt>instantiate_var</tt>.]

  Description        [Recursively applies <tt>instantiate_var</tt> to
  a given list of variables declaration, and performs some check for
  multiple variable definitions.]

  SideEffects        []

  SeeAlso            [instantiate_var]

******************************************************************************/
void instantiate_vars(Encoding_ptr senc, node_ptr var_list, 
		      node_ptr mod_name, node_ptr *trans,
		      node_ptr *init, node_ptr *invar, node_ptr *spec,
		      node_ptr * compute, node_ptr *ltl_spec, 
		      node_ptr *psl_spec,
		      node_ptr *invar_spec, node_ptr *justice, 
		      node_ptr *compassion, node_ptr *assign, 
		      node_ptr *procs)
{
  node_ptr rev_vars_list;
  node_ptr iter;

  rev_vars_list = reverse_ns(var_list);
  iter = rev_vars_list;
  while (iter != Nil) {
    node_ptr cur_var = car(iter);
    node_ptr name = CompileFlatten_resolve_name(car(cur_var), mod_name);
    node_ptr type = cdr(cur_var);
    
    instantiate_var(senc, name, type, trans, init, invar, spec, compute, 
		    ltl_spec, psl_spec, invar_spec, justice, compassion, assign, 
		    procs, mod_name);

    iter = cdr(iter);
  }

  free_list(rev_vars_list);
}

/**Function********************************************************************

  Synopsis           [Gets the flattened version of an atom.]

  Description        [Gets the flattened version of an atom. If the
  atom is a define then it is expanded. If the definition mode
  is set to "expand", then the expanded flattened version is returned,
  otherwise, the atom is returned.]

  SideEffects        [The <tt>flatten_def_hash</tt> is modified in
  order to memoize previously computed definition expansion.]

******************************************************************************/
node_ptr Flatten_GetDefinition(const Encoding_ptr senc, node_ptr atom)
{
  node_ptr result = Nil;
  node_ptr definition = Encoding_lookup_symbol(senc, atom);

  if (Encoding_is_symbol_var(senc, atom)) result = atom;
  else if (Encoding_is_symbol_constant(senc, atom)) result = atom;
  else if (Encoding_is_symbol_define(senc, atom)) {
    node_ptr exp = lookup_flatten_def_hash(atom);

    /* Check for circular recursive definitions */
    if (exp == BUILDING_FLAT_BODY) { error_circular(atom); }
    if (exp == (node_ptr) NULL) {
      /* The body of a definition is flattened and the flattening is
	 saved.  The flattened body is not returned. */
      insert_flatten_def_hash(atom, BUILDING_FLAT_BODY);
      io_atom_push(atom);
      {
        Definition_Mode_Type old_definition_mode = definition_mode;
        /* 
           We need to store the previous definition expansion mode,
           and force it to be expand here since we are attempting to
           expand the body of defined symbols, and we need to to do it
           recursively. If this is not done, then the expansion of
           further defined symbols occurring in the body is not
           performed.
        */
        set_definition_mode_to_expand();
        exp = Flatten_FlattenSexpRecur(senc, definition, Nil);
        definition_mode = old_definition_mode;
      }
      io_atom_pop();
      insert_flatten_def_hash(atom, exp);
    }
    
    if (definition_mode_is_expand()) result = exp;
    else result = atom;
  }
  else { 
    error_undefined(atom); 
  }
  return result;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Put a variable in the current "context"]

  Description        [Put a variable in the current "context", which
  is stored in <code>param_context</code>.]

  SideEffects        [None]

  SeeAlso            [param_context]

******************************************************************************/
static node_ptr put_in_context(node_ptr v)
{
  return(find_node(CONTEXT, param_context, v));
}


/**Function********************************************************************

  Synopsis           [Builds the parameters of a module from the list of formal parameters of the module itself.]

  Description        [Builds the parameters of a module from the list
  of formal parameters of the module itself and a <tt>basename</tt>.<br>
  There must be a one to one correspondence between the elements of
  <tt>actual</tt> and <tt>formal</tt> parameters. If the number of elements of
  the lists are different then, an error occurs.]

  SideEffects        [In the <tt>param_hash</tt>, the new parameter is
  associated to the old one.]

******************************************************************************/
static void make_params(node_ptr basename, node_ptr actual, node_ptr formal)
{
  while(formal) {
    node_ptr old, new;

    if (!actual) rpterr("too few actual parameters");
    new = find_node(DOT, basename, find_atom(car(formal)));
    old = car(actual);
    formal = cdr(formal);
    actual = cdr(actual);
    if (lookup_param_hash(new)) { error_multiple_substitution(new); }
    insert_param_hash(new, old);
  }
  if (actual) rpterr("too many actual parameters");
}


/**Function********************************************************************

  Synopsis           [Instantiates all in the body of a module.]

  Description        [This function is responsible of the
  instantiation of the body of a module. The module definition is
  <tt>mod_def</tt> and the module name <tt>mod_name</tt> are passed as
  arguments. First we instantiate the arguments of the given
  module. Then it loops over the module definition searching for
  defined symbols (i.e. those introduced by the keyword
  <tt>DEFINE</tt>) and inserts their definition in the
  <tt>symbol_hash</tt>. After this preliminary phase it loops again
  over module body in order to performs the other instantiation, and
  to extract all the information needed to compile the automaton,
  i.e. the list of processes, the TRANS statements, the INIT
  statements, ... and so on.]

  SideEffects        []

  SeeAlso            [instantiate_var instantiate_vars]

******************************************************************************/
static void instantiate(Encoding_ptr senc, node_ptr mod_def, 
			node_ptr mod_name, node_ptr *trans,
                        node_ptr *init, node_ptr *invar, node_ptr *spec,
                        node_ptr *compute, node_ptr *ltl_spec,
			node_ptr *psl_spec, 
                        node_ptr *invar_spec, node_ptr *justice, 
			node_ptr * compassion, node_ptr *assign, 
			node_ptr *procs, node_ptr actual)
{
  node_ptr mod_body_decls;
  node_ptr tmp_trans        = Nil;
  node_ptr tmp_init         = Nil;
  node_ptr tmp_invar        = Nil;
  node_ptr tmp_spec         = Nil;
  node_ptr tmp_compute      = Nil;
  node_ptr tmp_ltlspec      = Nil;
  node_ptr tmp_pslspec      = Nil;
  node_ptr tmp_invar_spec   = Nil;
  node_ptr tmp_justice      = Nil;
  node_ptr tmp_compassion   = Nil;
  node_ptr tmp_assign       = Nil;
  node_ptr tmp_procs        = Nil;
  node_ptr mod_formal_args  = car(mod_def); /* Module formal parameters */
  node_ptr mod_body         = cdr(mod_def); /* Module body */

  make_params(mod_name, actual, mod_formal_args); /* creates local parameters */
  /*
    We first instantiate all the definitions, in case they are
    constants used in the array declarations
  */
  mod_body_decls = mod_body;
  while (mod_body_decls != Nil) { /* loop over module declaration */
    node_ptr cur_decl = car(mod_body_decls);
    
    mod_body_decls = cdr(mod_body_decls);
    switch (node_get_type(cur_decl)) {

    case DEFINE: 
      {
	node_ptr define_iter = car(cur_decl);
      
	while (define_iter != Nil) { /* loop over DEFINE declaration */
	  node_ptr cur_define = car(define_iter);
	  node_ptr name = CompileFlatten_resolve_name(car(cur_define), mod_name);
	  node_ptr definition = cdr(cur_define);
	  
	  yylineno = node_get_lineno(define_iter);
	  if (! Encoding_is_symbol_declared(senc, name)) { 
	    Encoding_declare_define(senc, name, mod_name, definition);
	  }
	  else { error_redefining(name); }
	  
	  define_iter = cdr(define_iter);
	}/* loop on defines */
      }
      break;

    default: break;
    }
  }

  /* Now, we instantiate all the other elements of a module. */
  mod_body_decls = mod_body;
  while (mod_body_decls != Nil) { /* loop again over module declaration */
    node_ptr cur_decl = car(mod_body_decls);

    mod_body_decls = cdr(mod_body_decls);
    switch(node_get_type(cur_decl)) {

    case ISA:
      instantiate_by_name(senc, car(cur_decl), mod_name, &tmp_trans, 
			  &tmp_init, &tmp_invar, &tmp_spec, &tmp_compute, 
			  &tmp_ltlspec, &tmp_pslspec, 
			  &tmp_invar_spec, &tmp_justice, 
			  &tmp_compassion, &tmp_assign, &tmp_procs, Nil);
      break;

    case VAR:
      instantiate_vars(senc, car(cur_decl), mod_name, &tmp_trans, &tmp_init, 
		       &tmp_invar, &tmp_spec, &tmp_compute, &tmp_ltlspec, 
		       &tmp_pslspec, 
		       &tmp_invar_spec, &tmp_justice, &tmp_compassion, 
		       &tmp_assign, &tmp_procs);
      break;

    case IVAR:
      set_variable_instantiation_to_input();
      instantiate_vars(senc, car(cur_decl), mod_name, &tmp_trans, &tmp_init, 
		       &tmp_invar, &tmp_spec, &tmp_compute, &tmp_ltlspec, 
		       &tmp_pslspec, 
		       &tmp_invar_spec, &tmp_justice, &tmp_compassion, 
		       &tmp_assign, &tmp_procs);
      
      set_variable_instantiation_to_state();
      break;
      
    case TRANS:
      tmp_trans = find_node( AND, tmp_trans, 
			     find_node(CONTEXT, mod_name, car(cur_decl)) );
      break;

    case INIT:
      tmp_init = find_node(AND, tmp_init, find_node(CONTEXT, mod_name, car(cur_decl)));
      break;

    case INVAR:
      tmp_invar = find_node(AND, tmp_invar, find_node(CONTEXT, mod_name, car(cur_decl)));
      break;

    case SPEC:
      tmp_spec = cons(find_node(CONTEXT, mod_name, car(cur_decl)), tmp_spec);
      break;

    case LTLSPEC:
      tmp_ltlspec = cons(find_node(CONTEXT, mod_name, car(cur_decl)), tmp_ltlspec);
      break;

    case PSLSPEC:
      tmp_pslspec = cons(PslNode_new_context(
                	     PslNode_convert_id(mod_name, SMV2PSL), 
			     car(cur_decl)), 
			 tmp_pslspec);
      break;

    case INVARSPEC:
      tmp_invar_spec = cons(find_node(CONTEXT, mod_name, car(cur_decl)), tmp_invar_spec);
      break;

    case COMPUTE:
      tmp_compute = cons(find_node(CONTEXT, mod_name, car(cur_decl)), tmp_compute);
      break;

    case JUSTICE:
      tmp_justice = cons(find_node(CONTEXT, mod_name, car(cur_decl)), tmp_justice);
      break;

    case COMPASSION:
      tmp_compassion = cons(cons(find_node(CONTEXT, mod_name, car(car(cur_decl))), 
				 find_node(CONTEXT, mod_name, cdr(car(cur_decl)))),
			    tmp_compassion);
      break;

    case ASSIGN:
      /* an assign may be void */
      if (car(cur_decl)) {
	tmp_assign = find_node(AND, find_node(CONTEXT, mod_name, car(cur_decl)), 
			       tmp_assign);
      }
      break;
      
    default: break;
    }
  }
  
  *trans = find_node(AND, *trans, tmp_trans);
  *init  = find_node(AND, *init, tmp_init);
  *invar = find_node(AND, *invar, tmp_invar);
  *spec  = append(tmp_spec, *spec);
  *compute  = append(tmp_compute, *compute);
  *ltl_spec = append(tmp_ltlspec, *ltl_spec);
  *psl_spec = append(tmp_pslspec, *psl_spec);
  *invar_spec = append(tmp_invar_spec, *invar_spec);
  *justice  = append(tmp_justice, *justice);
  *compassion  = append(tmp_compassion, *compassion);
  *assign = find_node(AND, *assign, tmp_assign);
  *procs = append(*procs, tmp_procs);
}


/**Function********************************************************************

  Synopsis           [Starts the flattening from a given point in the
  module hierarchy.]

  Description        [Uses <tt>root_name</tt> as root module in the
  flattening of the hierarchy. The name of the module in the hierarchy
  is <tt>name</tt>. First checks if the module exists. Then it checks
  if the module is recursively defined, and if the case an error is
  printed out. If these checks are passed, then it proceeds in the
  instantiation of the body of the module.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void instantiate_by_name(Encoding_ptr senc, node_ptr root_name, 
				node_ptr name,
                                node_ptr *trans, node_ptr *init,
                                node_ptr *invar, node_ptr *spec,
                                node_ptr *compute, node_ptr *ltl_spec,
				node_ptr *psl_spec,
                                node_ptr *invar_spec, node_ptr *justice,
                                node_ptr *compassion, node_ptr *assign, 
				node_ptr *procs, node_ptr actual)
{
  node_ptr s;
  node_ptr mod_name = find_atom(root_name);         /* find module name */
  node_ptr mod_def  = lookup_module_hash(mod_name); /* find module definition  */

  yylineno = node_get_lineno(root_name);
  if (! mod_def) { 
    /* The module is undefined */
    error_undefined(root_name); 
  }
   
  /* scans module_stack in order to find if there are recursively
     defined modules */
  s = module_stack;
  while (s != Nil) {
    if (car(s) == mod_name) {
      rpterr("module \"%s\" is recursively defined",
             get_text((string_ptr)car(root_name)));
    }
    s = cdr(s);
  }

  /* append current module to module_stack */
  module_stack = cons(mod_name, module_stack);

  instantiate(senc, mod_def, name, trans, init, invar, spec, compute, 
	      ltl_spec, psl_spec, invar_spec, justice, compassion, 
	      assign, procs, actual);

  /* elimination of current module form module_stack */
  s = cdr(module_stack);
  free_node(module_stack);
  module_stack = s;
}


/**Function********************************************************************

  Synopsis           [Traverses the module hierarchy and extracts the
  information needed to compile the automaton.]

  Description        [Traverses the module hierarchy and extracts the
  information needed to compile the automaton. The hierarchy of modules
  is flattened, the variables are contextualized, the various parts of
  the model read in are extracted (i.e. the formulae to be verified,
  the initial expressions, ...)...<br>
  Notice that this function only manage s_expr and not ADD or BDD.]

  SideEffects        [None]

******************************************************************************/
static void compileFlattenHierarchy(
 Encoding_ptr senc, /* the symbolic encoding */
 node_ptr root_name /* the <code>ATOM</code> representing the module 
		       at the top of the hierarchy. */,
 node_ptr name /* the name of the module at the top of the hierarchy. */,
 node_ptr *trans /* the list of TRANS actually recognized */,
 node_ptr *init /* the list of INIT actually recognized */,
 node_ptr *invar /* the list of INVAR actually recognized */,
 node_ptr *spec /* the list of SPEC actually recognized */,
 node_ptr *compute /* the list of COMPUTE actually recognized */,
 node_ptr *ltl_spec /* the list of LTLSPEC actually recognized */,
 node_ptr *psl_spec /* the list of PSLSPEC actually recognized */,
 node_ptr *invar_spec /* the list of INVARSPEC actually recognized */,
 node_ptr *justice /* the list of JUSTICE actually recognized */,
 node_ptr *compassion /* the list of COMPASSION actually recognized */,
 node_ptr *assign /* the list of ASSIGN actually recognized */,
 node_ptr *procs /* the list of processes actually recognized */,
 node_ptr actual /* the actual module arguments */)
{
  node_ptr tmp_assign = Nil;

  instantiate_by_name(senc, 
		      root_name, name, trans, init, invar, spec, compute,
                      ltl_spec, psl_spec, invar_spec, justice, compassion, 
		      &tmp_assign, procs, actual);

  *procs = cons(cons(name, tmp_assign), *procs);
}


/**Function********************************************************************

  Synopsis           [Recursive function for flattenig a sexp.]

  Description        [Recursive function for flattenig a sexp.]

  SideEffects        []

  SeeAlso            [Compile_FlattenSexp Compile_FlattenSexpExpandDefine]

******************************************************************************/
static node_ptr 
Flatten_FlattenSexpRecur(const Encoding_ptr senc, node_ptr sexp, 
			 node_ptr context)
{
  node_ptr result = Nil;
  int temp = yylineno;

  if (sexp == Nil) return(sexp);

  yylineno = node_get_lineno(sexp);

  switch (node_get_type(sexp)) {
    /* base cases for which no flattening necessary */
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
      result = sexp;
      break;

  case ATOM:
    {
      node_ptr param, constant;
      node_ptr atom = find_atom(sexp);
      node_ptr name = find_node(DOT, context, atom);

      /* It is a constant. No flattening necessary */
      constant = lookup_flatten_constant_hash(atom);
      if (constant != (node_ptr) NULL) {
        result = atom;
        break;
      }
      constant = lookup_flatten_constant_hash(name);
      if (constant != (node_ptr) NULL) {
        result = name;
        break;
      }      

      param = lookup_param_hash(name);
      if (param != (node_ptr)NULL) {
        /* The result of the flattening is then flattening of parameters */
        result = Flatten_FlattenSexpRecur(senc, param, context);
        break;
      }
      /* It can be a defined symbol, a running condition or a variable */
      result = 
	Flatten_GetDefinition(senc, CompileFlatten_resolve_name(sexp, context));
      break;
    }
  case SELF:
    {
      fprintf(nusmv_stderr,
        "Flatten_FlattenSexpRecur: invalid usage of identifier \"self\"\n");

      error_reset_and_exit(1);
      break;
    }
  case DOT:
  case ARRAY:
    {
      node_ptr name = CompileFlatten_resolve_name(sexp, context);
      node_ptr constant = lookup_flatten_constant_hash(name);

      if (constant != (node_ptr)NULL) {
        result = name;
      }
      else {
        result = Flatten_GetDefinition(senc, name);
      }
      break;
    }
  case CONTEXT:
    {
      /* (CONTEXT (cxt . expr)) */
      result = Flatten_FlattenSexpRecur(senc, cdr(sexp), car(sexp));
      break;
    }
    /* Unary operators */
  case NOT:
  case NEXT:
    {
      node_ptr body = Flatten_FlattenSexpRecur(senc, car(sexp), context);

      result = new_node(node_get_type(sexp), body, Nil);
      break;
    }
    /* binary operators */
  case CONS:
  case AND:
  case OR:
  case XOR:
  case XNOR:
  case IMPLIES:
  case IFF:
  case PLUS:
  case MINUS:
  case TIMES:
  case DIVIDE:
  case MOD:
  case LT:
  case GT:
  case LE:
  case GE:
  case UNION:
  case SETIN:
  case EQUAL:
  case NOTEQUAL:
    {
      node_ptr left  = Flatten_FlattenSexpRecur(senc, car(sexp), context);
      node_ptr right = Flatten_FlattenSexpRecur(senc, cdr(sexp), context);

      result = new_node(node_get_type(sexp), left, right);
      break;
    }
  case CASE:
    {
      node_ptr condition =  Flatten_FlattenSexpRecur(senc, car(car(sexp)), context);
      node_ptr then_arg  =  Flatten_FlattenSexpRecur(senc, cdr(car(sexp)), context);
      node_ptr else_arg  =  Flatten_FlattenSexpRecur(senc, cdr(sexp), context);

      result = new_node(CASE, new_node(COLON, condition, then_arg), else_arg);
      break;
    }
  case TWODOTS:
    {
      /* We don't need to expand it, eval did it */
      result = sexp;
      break;
    }
    /* CTL Unary operators */
  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
    {
      node_ptr body = Flatten_FlattenSexpRecur(senc, car(sexp), context);

      result = new_node(node_get_type(sexp), body, Nil);
      break;
    }
    /* CTL bynary operators */
  case EU:
  case AU:
    {
      node_ptr left  = Flatten_FlattenSexpRecur(senc, car(sexp), context);
      node_ptr right = Flatten_FlattenSexpRecur(senc, cdr(sexp), context);

      result = new_node(node_get_type(sexp), left, right);
      break;
    }
    /* CTL bounded Temporal Operators */
  case EBF:
  case ABF:
  case EBG:
  case ABG:
  case EBU:
  case ABU:
    {
      node_ptr body  = Flatten_FlattenSexpRecur(senc, car(sexp), context);
      node_ptr range = cdr(sexp);

      /* checks the range: */
      if (! Utils_check_subrange_not_negative(range) ) {
	error_invalid_subrange(range);
      }

      result = new_node(node_get_type(sexp), body, range);
      break;
    }
    /* LTL unary temporal operators */
  case OP_NEXT:
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_ONCE:
  case OP_GLOBAL:
  case OP_HISTORICAL:
    {
      node_ptr body  = Flatten_FlattenSexpRecur(senc, car(sexp), context);

      result = new_node(node_get_type(sexp), body, Nil);
      break;
    }
    /* LTL binary temporal operators */
  case UNTIL:
  case RELEASES:
  case SINCE:
  case TRIGGERED:
    {
      node_ptr left   = Flatten_FlattenSexpRecur(senc, car(sexp), context);
      node_ptr right  = Flatten_FlattenSexpRecur(senc, cdr(sexp), context);

      result = new_node(node_get_type(sexp), left, right);
      break;
    }
    /* MIN and MAX operators */
  case MINU:
  case MAXU:
    {
      node_ptr left   = Flatten_FlattenSexpRecur(senc, car(sexp), context);
      node_ptr right  = Flatten_FlattenSexpRecur(senc, cdr(sexp), context);

      result = new_node(node_get_type(sexp), left, right);
      break;
    }
  case EQDEF:
    {
      node_ptr left  = car(sexp);
      node_ptr right = cdr(sexp) ;
      node_ptr res_left, res_right;

      switch(node_get_type(left)) {
      case SMALLINIT:
      case NEXT:
        {
          /* we are dealing with init(x) := init_expr or next(x) := next_expr */
          node_ptr name = Flatten_FlattenSexpRecur(senc, car(left), context);
          if (! Encoding_is_symbol_declared(senc, name)) {
            error_undefined(name);
	  }
          res_left = new_node(node_get_type(left), name, Nil);
          res_right = Flatten_FlattenSexpRecur(senc, right, context);
          break;
        }
      default:
        {
          /* we are dealing with x := simple_expr */
          res_left  = Flatten_FlattenSexpRecur(senc, left, context);
          res_right = Flatten_FlattenSexpRecur(senc, right, context);
          break;
        }
      }
      result = new_node(EQDEF, res_left, res_right);
      break;
    }
  default:
    {
      fprintf(nusmv_stderr,
	      "Flatten_FlattenSexpRecur: undefined node type (%d)\n",
	      node_get_type(sexp));
      error_reset_and_exit(1);
      break;
    }
  }
  nusmv_assert(result != Nil);

  yylineno = temp;
  return(result);
}


/**Function********************************************************************

  Synopsis           [Recursive definition of Compute_FlattenProcess]

  Description        [Recursive definition of Compute_FlattenProcess]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void 
compileFlattenProcessRecur(const Encoding_ptr senc, node_ptr assign, 
			   node_ptr context, node_ptr running)
{
  if (assign == Nil) return;
  yylineno = node_get_lineno(assign);
  switch (node_get_type(assign)) {
  case CONS:
  case AND:
    compileFlattenProcessRecur(senc, car(assign), context, running);
    compileFlattenProcessRecur(senc, cdr(assign), context, running);
    break;

  case CONTEXT:
    compileFlattenProcessRecur(senc, cdr(assign), car(assign), running);
    break;

  case EQDEF: {
    node_ptr vname, lhsa, stored;
    node_ptr left  = car(assign);
    node_ptr right = cdr(assign);

    switch (node_get_type(left)) {
    case SMALLINIT: /* init assignement */
      {
        vname = CompileFlatten_resolve_name(car(left), context);
        lhsa = find_node(node_get_type(left), vname, Nil);
        stored = lookup_assign_db_hash(lhsa);
        if (stored != (node_ptr)NULL) {
          if (car(stored) == Nil) {
            setcar(stored, right);
          }
          else {
            error_redefining(vname);
          }
        }
        else {
          insert_assign_db_hash(lhsa, new_node(SMALLINIT, cons(right, Nil), mk_true()));
        }
        break;
      }

    case NEXT: /* next assignement */
      {
        node_ptr condition = new_node(COLON, running, right);

        vname = CompileFlatten_resolve_name(car(left), context);
        lhsa = find_node(node_get_type(left), vname, Nil);
        stored = lookup_assign_db_hash(lhsa);
        if (stored != (node_ptr)NULL) {
          if (car(stored) != Nil) {
            /* The previous case list the is saved and a side effect is
               performed on the saved entry to take care of the new case
               condition. */
            setcar(stored, new_node(CASE, condition, car(stored)));
          }
          else {
            /* case internal default */
            node_ptr cdefault = new_node(TRUEEXP, Nil, Nil);
            /* 1 : vname */
            node_ptr inertia  = new_node(COLON, cdefault, vname);
            node_ptr base     = new_node(CASE, inertia, cdefault);
            /* We save the entry */
            setcar(stored, new_node(CASE, condition, base));
          }
        }
        else {
          /* case internal default */
          node_ptr cdefault = new_node(TRUEEXP, Nil, Nil);
          /* 1 : vname */
          node_ptr inertia  = new_node(COLON, cdefault, vname);
          node_ptr base     = new_node(CASE, inertia, cdefault);
          /* We save the entry */
          insert_assign_db_hash(lhsa, new_node(NEXT, new_node(CASE, condition, base), mk_true()));
        }
        break;
      }
      
    default: /* Invariant assignement */
      {
        vname = lhsa = CompileFlatten_resolve_name(left, context);
        stored = lookup_assign_db_hash(lhsa);
        if (stored != (node_ptr)NULL) {
          if (car(stored) == Nil) {
            setcar(stored, right);
          }
          else {
            error_redefining(vname);
          }
        }
        else {
          insert_assign_db_hash(lhsa, new_node(INVAR, cons(right, Nil), mk_true()));
        }
        break;
      }
    }
    break;
  }

  default:
    internal_error("compileFlattenProcessRecur: type = %d", 
		   node_get_type(assign));
    break;
  }
}

/**Function********************************************************************

  Synopsis           [Builds the atom set of the given range]

  Description        [Given a range it builds the corresponding
  internal representation in term of UNION.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr make_atom_set(node_ptr l)
{
  node_ptr result;

  if (l == Nil) {
    fprintf(nusmv_stderr, "make_atom_set: l = Nil\n");
    error_reset_and_exit(1);
  }
  if (cdr(l) != Nil) {
    result = new_node(UNION, car(l), make_atom_set(cdr(l)));
  }
  else {
    result = car(l);
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void 
compileFlattenSexpModelAux(Encoding_ptr senc, node_ptr expr, int type)
{
  int saved_yylineno = yylineno;

  if (expr == Nil) return;
  yylineno = node_get_lineno(expr);
  compileFlattenSexpModelRecur(senc, expr, type);
  yylineno = saved_yylineno;
  return;
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void compileFlattenSexpModelRecur(Encoding_ptr senc, node_ptr expr, 
					 int type)
{

  if (expr == Nil) return;
  if (node_get_type(expr) == AND) {
    compileFlattenSexpModelAux(senc, car(expr), type);
    compileFlattenSexpModelAux(senc, cdr(expr), type);
  }
  else {
    Set_t dep = (Set_t) Formula_GetDependencies(senc, expr, Nil);

    if (Set_IsEmpty(dep) == true) {
      dep = compileFlattenConstantSexpCheck(senc, expr, type);
    }

    while (Set_IsEmpty(dep) == false) {
      node_ptr var = (node_ptr) Set_GetFirst(dep);

      switch (type) {
      case INIT:
	{
	  node_ptr index = find_node(SMALLINIT, var, Nil);
	  node_ptr stored = lookup_assign_db_hash(index);
	  
	  if (stored != (node_ptr)NULL) {
	    setcdr(stored, mk_and(expr, cdr(stored)));
	  }
	  else {
	    insert_assign_db_hash(index, new_node(SMALLINIT, Nil, expr));
	  }
	  break;
	}
	  
      case INVAR:
	{
	  node_ptr index = var;
	  node_ptr stored = lookup_assign_db_hash(index);

	  if (stored != (node_ptr)NULL) {
	    setcdr(stored, mk_and(expr, cdr(stored)));
	  }
	  else {
	    insert_assign_db_hash(index, new_node(INVAR, Nil, expr));
	  }
	  break;
	}

      case TRANS:
	{
	  node_ptr index = find_node(NEXT, var, Nil);
	  node_ptr stored = lookup_assign_db_hash(index);

	  if (stored != (node_ptr)NULL) {
	    setcdr(stored, mk_and(expr, cdr(stored)));
	  }
	  else {
	    insert_assign_db_hash(index, new_node(NEXT, Nil, expr));
	  }
	  break;
	}

      default:
	{
	  fprintf(nusmv_stderr, "compileFlattenSexpModelAux: Unknown expr type\n");
	  error_reset_and_exit(1);
	  break;
	}
      }

      dep = Set_GetRest(dep);
    } /* while loop*/

  }
}


/**Function********************************************************************

  Synopsis           [Called when a constant has been found in INVAR, INIT or 
  TRANS]

  Description        [If the constant is trivially true, it is skipped;
  otherwise it will be associated to each fsm of all state variables]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static Set_t compileFlattenConstantSexpCheck(Encoding_ptr senc, node_ptr expr, 
					     int type)
{
  Set_t res; 

  fprintf(nusmv_stderr, "Constant expression found in a %s statement in",
	  type_to_string(type));
  start_parsing_err();
  fprintf(nusmv_stderr, " The expression is \"");
  print_node(nusmv_stderr, expr);
  fprintf(nusmv_stderr, "\"");

  if ( ((node_get_type(expr) == NUMBER) && (node_get_int(expr) == 1)) || 
       (expr == mk_true()) ) {
    fprintf(nusmv_stderr, " (Skipped)\n");
    res = Set_MakeEmpty();
  }
  else {
    node_ptr vars; 

    fprintf(nusmv_stderr, "\n");
    vars = NodeList_to_node_ptr(Encoding_get_state_vars_list(senc));
    res = Set_Make(vars);
  }
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Creates the internal process selector variable.]

  Description        [Creates the internal process selector
  variable. The internal process selector variable is as default
  positioned at the top of the ordering. It is attached  to
  <tt>input_variables</tt> and <tt>all_variables</tt> too. ]

  SideEffects        [<tt>input_variables</tt> and
  <tt>all_variables</tt> are affected.]

  SeeAlso            []

******************************************************************************/
static void 
create_process_symbolic_variables(Encoding_ptr senc, node_ptr procs_expr)
{
  node_ptr main_atom = sym_intern("main");

  /* We create the internal representation of the process selector variable */
  proc_selector_internal_vname = 
    find_node(DOT, Nil, sym_intern(PROCESS_SELECTOR_VAR_NAME));

  if (! Encoding_is_symbol_declared(senc, proc_selector_internal_vname)) {
    /* In the input file the process_selector variable has not been
      specified.  Thus, we create it, and all the info necessary to
      its creation. */

    node_ptr l;
    /* We extract the range of the process_selector_var_name */
    process_selector_range = map(car, procs_expr);

    /* Side effect on the list to replace Nil with "main" */
    for(l = process_selector_range; l != Nil; l = cdr(l)) {
      if (car(l) == Nil) {
	/* main internally stored as Nil is put in the range as (ATOM
	   main) */
	setcar(l,main_atom);
      }
    }

    update_flatten_constant_hash(process_selector_range);

    Encoding_declare_input_var(senc, proc_selector_internal_vname, 
			       process_selector_range);
  }
  else {
    node_ptr l;
    node_ptr p_s_r = map(car, procs_expr);
    process_selector_range = 
      Encoding_get_var_range(senc, proc_selector_internal_vname);

    /* We check that for each process defined there is an entry in the
      range _process_selector_ variable. */
    for(l = p_s_r; l != Nil; l = cdr(l)) {
      node_ptr el = car(l);

      if (el == Nil)  el = main_atom;
      if (!in_list(el, process_selector_range)) {
        range_error(el, proc_selector_internal_vname);
      }
    }
    free_list(p_s_r);
  }

  /* We create the define corresponding to Pi.running */
  {
    node_ptr l;
    node_ptr list_of_processes = process_selector_range;

    /* The internal symbol running is created and stored */
    running_atom = sym_intern(RUNNING_SYMBOL);
    for(l = list_of_processes; l != Nil; l = cdr(l)) {
      node_ptr pi, pir, pir_body;

      pi = car(l);
      if (pi == main_atom) {
        /* internally main is represented as Nil */
        pir = CompileFlatten_resolve_name(running_atom, Nil);
      } 
      else {
        pir = CompileFlatten_resolve_name(running_atom, pi);
      }

      /* storing the defined name int the appropriate list */
      process_running_symbols = cons(pir, process_running_symbols);

      /* creating the body: _process_selector = Pi */
      pir_body = find_node(EQUAL, proc_selector_internal_vname, pi);

      if (Encoding_is_symbol_declared(senc, pir)) {
        fprintf(nusmv_stderr, "Warning: symbol \"");
        print_node(nusmv_stderr, pir);
        fprintf(nusmv_stderr, "\" already defined.\n");
        fprintf(nusmv_stderr, "Using the one defined in the input file.\n");
      }
      else {
        /* The flatten hash has to be filled with the flattened
	   body of the newly defined symbol. */
        insert_flatten_def_hash(pir, Compile_FlattenSexp(senc, pir_body, Nil));

	/* declare the define: */
	Encoding_declare_define(senc, pir, Nil, pir_body);
      }
    }
  }
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        []

  SeeAlso            [optional]

******************************************************************************/
static node_ptr mk_and(node_ptr a, node_ptr b)
{
  return(find_node(AND, a, b));
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        []

  SeeAlso            [optional]

******************************************************************************/
static node_ptr mk_true(void)
{
  return(find_node(TRUEEXP, Nil, Nil));
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        []

  SeeAlso            [optional]

******************************************************************************/
static node_ptr mk_false(void)
{
  return(find_node(FALSEEXP, Nil, Nil));
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        []

  SeeAlso            [optional]

******************************************************************************/
static char * type_to_string(int type) 
{
  switch (type) {
  case INIT:
    return("INIT");
  case INVAR:
    return("INVAR");
  case TRANS:
    return("TRANS");
  default:
    nusmv_assert(false); 
  }
}


/**Function********************************************************************

  Synopsis           [Performs the recursive step of
  <code>CompileFlatten_resolve_name</code>.]

  Description        [Performs the recursive step of
  <code>CompileFlatten_resolve_name</code>.]

  SideEffects        []

  SeeAlso            [CompileFlatten_resolve_name]

******************************************************************************/
static Expr_ptr 
compile_flatten_resolve_name_recur(Expr_ptr n, node_ptr context)
{
  /* Return value in case an error occurs */
# define TYPE_ERROR ((node_ptr) -1)  

  node_ptr temp, name;

  switch (node_get_type(n)) {

  case CONTEXT: return CompileFlatten_resolve_name(cdr(n), car(n));

  case ATOM:
    name = find_node(DOT, context, find_atom(n));
    temp = lookup_param_hash(name);

    if (temp != (node_ptr) NULL) {
      temp = CompileFlatten_resolve_name(temp, context);
      if (temp == TYPE_ERROR) {
	rpterr("type error, module parameter = %s",
	       get_text((string_ptr) car(n)));
      }
      return temp;
    }
    else return name;
    
  case NUMBER: return find_atom(n);

  case BIT: 
    temp = CompileFlatten_resolve_name(car(n), context);
    if (temp == TYPE_ERROR) { rpterr("type error, operator bit"); }
    return find_node(BIT, temp, cdr(n)); /* cdr(n) is a int */ 

  case DOT:
    temp = CompileFlatten_resolve_name(car(n), context);
    if (temp == TYPE_ERROR) { rpterr("type error, operator = ."); }
    return find_node(DOT, temp, find_atom(cdr(n)));

  case ARRAY:
    temp = CompileFlatten_resolve_name(car(n), context);
    if (temp == TYPE_ERROR) { rpterr("type error, operator = []"); }
    return find_node(ARRAY, temp, find_atom(cdr(n))); /* cdr(n) is NUMBER */ 

  case SELF: return context;

  default: 
    fprintf(nusmv_stderr, "compile_flatten_resolve_name_recur: unrecognized type of:\n");
    print_sexp(nusmv_stderr, n);
    return TYPE_ERROR;
  }
}

