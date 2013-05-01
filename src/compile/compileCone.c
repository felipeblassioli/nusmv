/**CFile***********************************************************************

  FileName    [compileCone.c]

  PackageName [compile]

  Synopsis    [Computes the cone of influence of the model variables.]

  Description [This file contains the functions needed for computing
  the cone of influence (COI) of a given formula. The COI of all the
  variables in the model is pre-computed ancd cached the first time
  a cone of influence is required (function <code>initCoi</code>.
  Functions are also provided that compute the dependency variables
  for a formula, namely those variables that appear in the formula
  or in one of the definitions the formula depends on.]

  SeeAlso     []

  Author      [Marco Roveri and Marco Pistore]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by ITC-irst.

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

#include "compileInt.h"
#include "set/set.h"
#include "utils/ustring.h"
#include "utils/assoc.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "enc/enc.h"

static char rcsid[] UTIL_UNUSED = "$Id: compileCone.c,v 1.11.4.12.2.1 2005/07/14 15:58:25 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [Indicates that the dependency computation is ongoing.]

  Description  [The value used during the building of dependencies of
  defined symbols to keep track that compuation is ongoing to discover
  circular definitions.]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define BUILDING_DEP_SET (Set_t)-10

/**Macro***********************************************************************

  Synopsis     [Indicates that the COI computation should be verbose.]

  Description  [Indicates that the COI computation should be verbose.]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define COI_VERBOSE (opt_verbose_level_gt(options, 2))


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [The hash of cone for a variable]

  Description [This hash associates to a variable the corresponding
  cone of influence, once it has been computes.]

  SeeAlso     []

******************************************************************************/
static hash_ptr coi_hash = (hash_ptr)NULL;
void init_coi_hash() {
  coi_hash = new_assoc();
  nusmv_assert(coi_hash != (hash_ptr)NULL);
}
void insert_coi_hash(node_ptr key, Set_t value) {
  nusmv_assert(coi_hash != (hash_ptr)NULL);
  insert_assoc(coi_hash, key, (node_ptr)value);
}
Set_t lookup_coi_hash(node_ptr key) {
  nusmv_assert(coi_hash != (hash_ptr)NULL);
  return((Set_t)find_assoc(coi_hash, key));
}

static assoc_retval coi_hash_free(char *key, char *data, char * arg)
{
  Set_t element = (node_ptr)data;

  if (element != (Set_t)NULL) {
    Set_ReleaseSet(element);
  }
  return(ASSOC_DELETE);
}

void clear_coi_hash() {
  clear_assoc_and_free_entries(coi_hash, coi_hash_free);
}


/**Variable********************************************************************

  Synopsis    [The hash of dependencies for defined symbols]

  Description [This hash associates to a defined atom the
  corresponding set]

  SeeAlso     []

******************************************************************************/
static hash_ptr define_dep_hash = (hash_ptr)NULL;
void init_define_dep_hash() {
  define_dep_hash = new_assoc();
  nusmv_assert(define_dep_hash != (hash_ptr)NULL);
}
void insert_define_dep_hash(node_ptr key, Set_t value) {
  nusmv_assert(define_dep_hash != (hash_ptr)NULL);
  insert_assoc(define_dep_hash, key, (node_ptr)value);
}
Set_t lookup_define_dep_hash(node_ptr key) {
  nusmv_assert(define_dep_hash != (hash_ptr)NULL);
  return((Set_t)find_assoc(define_dep_hash, key));
}

static assoc_retval define_dep_hash_free(char *key, char *data, char * arg)
{
  Set_t element = (node_ptr)data;

  if ((element != (Set_t) NULL) && (element != BUILDING_DEP_SET)) {
    Set_ReleaseSet(element);
  }
  return(ASSOC_DELETE);
}

void clear_define_dep_hash() 
{
  clear_assoc_and_free_entries(define_dep_hash, define_dep_hash_free);
}


/**Variable********************************************************************

  Synopsis    [The hash of dependencies for a given formula.]

  Description [This hash associates to each formula the corresponding
  set of dependencies.]

  SeeAlso     []

******************************************************************************/
static  hash_ptr dependencies_hash = (hash_ptr)NULL;
void init_dependencies_hash() {
  dependencies_hash = new_assoc();
  nusmv_assert(dependencies_hash != (hash_ptr)NULL);
}
void insert_dependencies_hash(node_ptr key, Set_t value) {
  nusmv_assert(dependencies_hash != (hash_ptr)NULL);
  insert_assoc(dependencies_hash, key, (node_ptr)value);
}
Set_t lookup_dependencies_hash(node_ptr key) {
  nusmv_assert(dependencies_hash != (hash_ptr)NULL);
  return((Set_t)find_assoc(dependencies_hash, key));
}

static assoc_retval dependencies_hash_free(char *key, char *data, char * arg)
{
  Set_t element = (node_ptr)data;

  if (element != (Set_t)NULL) {
    Set_ReleaseSet(element);
  }
  return(ASSOC_DELETE);
}

void clear_dependencies_hash() {
  clear_assoc_and_free_entries(dependencies_hash, dependencies_hash_free);
}

static node_ptr mk_hash_key(node_ptr e, node_ptr c) {
  return(find_node(CONTEXT, c, e));
}


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static Set_t 
formulaGetDependenciesRecur ARGS((const Encoding_ptr senc, 
				  node_ptr, node_ptr));

static Set_t 
formulaGetDefinitionDependencies ARGS((const Encoding_ptr senc, node_ptr));

static void coiInit ARGS((const Encoding_ptr senc));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Computes dependencies of a given SMV expression]

  Description        [The set of dependencies of a given formula are
  computed. A traversal of the formula is performed. Each time a
  variable is encountered, it is added to the so far computed
  set. When a formula depends on a next variable, then the
  corresponding current variable is added to the set. When an atom is
  found a call to <tt>formulaGetDefinitionDependencies</tt> is
  performed to compute the dependencies.]

  SideEffects        []

  SeeAlso            [formulaGetDefinitionDependencies]

******************************************************************************/
Set_t Formula_GetDependencies(const Encoding_ptr senc, node_ptr formula, 
			      node_ptr context)
{
  Set_t result;
  int temp = yylineno;
  node_ptr hash_key;

  if (formula == Nil) return(Set_MakeEmpty());

  hash_key = mk_hash_key(formula, context);
  result = lookup_dependencies_hash(hash_key);
  if (result == (Set_t)NULL) {
    yylineno = node_get_lineno(formula);
    result = formulaGetDependenciesRecur(senc, formula, context);
    yylineno = temp;
    insert_dependencies_hash(hash_key, result);
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Compute the dependencies of two set of formulae]

  Description [Given a formula and a list of fairness constraints, the
  set of variables occurring in them is computed.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t Formulae_GetDependencies(const Encoding_ptr senc, node_ptr formula, 
			       node_ptr justice, node_ptr compassion)
{
  Set_t result;
  Set_t result1, result2, result3;

  result1 = Formula_GetDependencies(senc, formula, Nil);
  result2 = Formula_GetDependencies(senc, justice, Nil);
  result3 = Formula_GetDependencies(senc, compassion, Nil);

  result = Set_Union(Set_Union(result1, result2),result3);
  return result;
}


/**Function********************************************************************

  Synopsis           [Compute the COI of a given set of variables]

  Description        [Computes the COI of a given set of variables.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t ComputeCOI(const Encoding_ptr senc, Set_t base)
{
  Set_t coi = base;

  if (! cmp_struct_get_coi(cmps)) {
    coiInit(senc);
    cmp_struct_set_coi(cmps);
  }

  while(Set_IsEmpty(base) == false) {
    node_ptr name = Set_GetFirst(base);
    coi = Set_Union(coi, lookup_coi_hash(name));
    base = Set_GetRest(base);
  }
  return coi;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Compute the dependencies of an atom]

  Description        [This function computes the dependencies of an
  atom. If the atom corresponds to a variable then the singleton with
  the variable is returned. If the atom corresponds to a "running"
  condition the singleton with variable PROCESS_SELECTOR_VAR_NAME is
  returned. Otherwise if the atom corresponds to a defined symbol the
  dependency set corresponding to the body of the definition is
  computed and returned.]

  SideEffects        [The <tt>define_dep_hash</tt> is modified in
  order to memoize previously computed dependencies of defined symbols.]

  SeeAlso            [Formula_GetDependencies]

******************************************************************************/
static Set_t 
formulaGetDefinitionDependencies(const Encoding_ptr senc, node_ptr formula)
{
  Set_t result = Set_MakeEmpty();

  if (Encoding_is_symbol_var(senc, formula)) {
    result = Set_MakeSingleton((Set_Element_t)formula);
  }
  else if (Encoding_is_symbol_define(senc, formula)) {
    /* It is a define and the type must be CONTEXT */
    Set_t res = lookup_define_dep_hash(formula);
    if (res == BUILDING_DEP_SET) { error_circular(formula); }
    if (res != (Set_t) NULL) {
      /* dependencies of the defined symbol have been previously
	 computed */
      result = res;
    }
    else {
      /* We mark the formula as open and we start looking for the body
	 dependencies. */
      node_ptr def = Encoding_lookup_symbol(senc, formula);
      insert_define_dep_hash(formula, BUILDING_DEP_SET);
      io_atom_push(formula);
      result = Formula_GetDependencies(senc, def, Nil);
      io_atom_pop();
      
      /* We mark the formula as closed, storing the computed
	 dependencies for further use. */
      insert_define_dep_hash(formula, result);
    }
  }
  
  else {
    fprintf(nusmv_stderr, "Undefined symbol \"");
    print_node(nusmv_stderr, formula);
    fprintf(nusmv_stderr, "\"\n");
    nusmv_exit(1);
  }    

  return result;
}


/**Function********************************************************************

  Synopsis           [Recursive call to Formula_GetDependencies.]

  Description        [Recursive call to Formula_GetDependencies.]

  SideEffects        []

  SeeAlso            [Formula_GetDependencies formulaGetDefinitionDependencies]

******************************************************************************/
static Set_t formulaGetDependenciesRecur(const Encoding_ptr senc, 
					 node_ptr formula, node_ptr context)
{
  if (formula == Nil) return Set_MakeEmpty();

  switch (node_get_type(formula)) {
  case CONTEXT:
    return Formula_GetDependencies(senc, cdr(formula), car(formula));

  case TRUEEXP:
  case FALSEEXP:
    return Set_MakeEmpty();

  case NUMBER:
    return Set_MakeEmpty();

  case ATOM:
    {
      node_ptr name = find_node(DOT, context, find_atom(formula));
      node_ptr param = lookup_param_hash(name);
      boolean is_symb = Encoding_is_symbol_declared(senc, name);
      node_ptr constant = lookup_flatten_constant_hash(find_atom(formula));

      if (constant == (node_ptr) NULL) {
	/* try with a flatten version of atom: */
	constant = lookup_flatten_constant_hash(name);
      }

      if (((param  != (node_ptr)NULL) && is_symb) ||
          (is_symb && (constant != (node_ptr)NULL))  ||
          ((param  != (node_ptr) NULL) && (constant != (node_ptr) NULL))) {
	error_ambiguous(formula);
      }
      if (param != (node_ptr)NULL) {
        return Formula_GetDependencies(senc, param, context);
      }
      if (constant != (node_ptr)NULL) {
        return Set_MakeEmpty();
      }
      /* it should be a defined symbol, running, or a variable */
      return formulaGetDefinitionDependencies(senc, 
			   CompileFlatten_resolve_name(formula, context));
    }

  case DOT:
  case ARRAY:
    {
      node_ptr name = CompileFlatten_resolve_name(formula, context);
      add_ptr temp = (add_ptr) lookup_flatten_constant_hash(name);

      if (temp != (add_ptr)NULL) {
        return Set_MakeEmpty();
      }
      else {
        return formulaGetDefinitionDependencies(senc, name);
      }
    }

  case CONS:
    {
      Set_t left = Formula_GetDependencies(senc, car(formula), context);
      Set_t right = Formula_GetDependencies(senc, cdr(formula), context);
      return Set_Union(left, right);
    }

    /* Sets */
  case TWODOTS:
    return Set_MakeEmpty();

  case UNION:
  case SETIN:
    {
      Set_t left = Formula_GetDependencies(senc, car(formula), context);
      Set_t right = Formula_GetDependencies(senc, cdr(formula), context);

      return Set_Union(left, right);
    }

    /* Numerical Operations */
  case PLUS:
  case MINUS:
  case TIMES:
  case DIVIDE:
  case MOD:
    {
      Set_t left = Formula_GetDependencies(senc, car(formula), context);
      Set_t right = Formula_GetDependencies(senc, cdr(formula), context);

      return Set_Union(left, right);
    }

    /* Comparison Operations */
  case EQUAL:
  case NOTEQUAL:
  case LT:
  case GT:
  case LE:
  case GE:
    {
      Set_t left = Formula_GetDependencies(senc, car(formula), context);
      Set_t right = Formula_GetDependencies(senc, cdr(formula), context);

      return Set_Union(left, right);
    }

  case NEXT:
    {
      /* next(alpha), with alpha possibly complex, thus ... */
      return Formula_GetDependencies(senc, car(formula), context);
    }

    /* Unary boolean connectives */
  case NOT:
    return Formula_GetDependencies(senc, car(formula), context);

    /* Binary boolean connectives */
  case AND:
  case OR:
  case XOR:
  case XNOR:
  case IMPLIES:
  case IFF:
    {
      Set_t left = Formula_GetDependencies(senc, car(formula), context);
      Set_t right = Formula_GetDependencies(senc, cdr(formula), context);

      return Set_Union(left, right);
    }

  case CASE:
    {
      Set_t partial;
      Set_t condition =  Formula_GetDependencies(senc, car(car(formula)), context);
      Set_t then_arg  =  Formula_GetDependencies(senc, cdr(car(formula)), context);
      Set_t else_arg  =  Formula_GetDependencies(senc, cdr(formula), context);

      partial = Set_Union(condition, then_arg);
      return Set_Union(partial, else_arg);
    }

    /* CTL unary Temporal Operators */
  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
    return(Formula_GetDependencies(senc, car(formula), context));

    /* CTL binary  Temporal Operators */
  case EU:
  case AU:
    {
      Set_t left = Formula_GetDependencies(senc, car(formula), context);
      Set_t right = Formula_GetDependencies(senc, cdr(formula), context);

      return Set_Union(left, right);
    }

    /* CTL unary bounded Temporal Operators */
  case EBF:
  case ABF:
  case EBG:
  case ABG:
    return Formula_GetDependencies(senc, car(formula), context);

    /* CTL binary bounded Temporal Operators */
  case EBU:
  case ABU:
    return Formula_GetDependencies(senc, car(formula), context);

    /* LTL unary Temporal Operators */
  case OP_NEXT:
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_ONCE:
  case OP_GLOBAL:
  case OP_HISTORICAL:
    return Formula_GetDependencies(senc, car(formula), context);

    /* LTL binary Temporal Operators */
  case UNTIL:
  case RELEASES:
  case SINCE:
  case TRIGGERED:
    {
      Set_t left = Formula_GetDependencies(senc, car(formula), context);
      Set_t right = Formula_GetDependencies(senc, cdr(formula), context);

      return Set_Union(left, right);
    }

    /* MIN MAX operators */
  case MAXU:
  case MINU:
    {
      Set_t left = Formula_GetDependencies(senc, car(formula), context);
      Set_t right = Formula_GetDependencies(senc, cdr(formula), context);

      return Set_Union(left, right);
    }

  default:
    {
      fprintf(nusmv_stderr,
        "Formula_GetDependencies: Reached undefined connective (%d)\n",
        node_get_type(formula));
      nusmv_exit(1);
      break;
    }
  }
  return Set_MakeEmpty();
}

/**Function********************************************************************

  Synopsis           [Pre-compute the COI of the variables]

  Description        [Computes the COI of all the variables in the current model]

  SideEffects        []

  SeeAlso            [ComputeCOI]

******************************************************************************/
static void coiInit(const Encoding_ptr senc)
{
  Set_t nonassign_vars = Set_MakeEmpty();
  NodeList_ptr vars;
  ListIter_ptr iter;

  init_coi_hash();

  if (COI_VERBOSE) {
    fprintf(nusmv_stdout,"*** INIT COI ***\n");
  }

  vars = Encoding_get_all_model_vars_list(senc); 
  for (iter = NodeList_get_first_iter(vars); 
       ! ListIter_is_end(iter); iter = ListIter_get_next(iter)) {
    node_ptr var = NodeList_get_elem_at(vars, iter);

    node_ptr invar_var = var;
    node_ptr init_var = find_node(SMALLINIT, var, Nil);
    node_ptr next_var = find_node(NEXT, var, Nil);

    node_ptr invar_e = lookup_assign_db_hash(invar_var);
    node_ptr init_e  = lookup_assign_db_hash(init_var);
    node_ptr next_e  = lookup_assign_db_hash(next_var);

    node_ptr true_node = find_node(TRUEEXP, Nil, Nil);

    Set_t base = Set_MakeSingleton(var);
    int nonassign = 0;

    if (invar_e != (node_ptr)NULL) {
      base = Set_Union(Formula_GetDependencies(senc, car(invar_e), Nil), base);
      if (cdr(invar_e) != true_node) {
	nonassign = 1;
	base = Set_Union(Formula_GetDependencies(senc, cdr(invar_e), Nil), 
			 base);
      }
    }

    if (init_e != (node_ptr)NULL) {
      base = Set_Union(Formula_GetDependencies(senc, car(init_e), Nil), base);
      if (cdr(init_e) != true_node) {
	nonassign = 1;
	base = Set_Union(Formula_GetDependencies(senc, cdr(init_e), Nil), base);
      }
    }

    if (next_e != (node_ptr)NULL) {
      base = Set_Union(Formula_GetDependencies(senc, car(next_e), Nil), base);
      if (cdr(next_e) != true_node) {
	nonassign = 1;
	base = Set_Union(Formula_GetDependencies(senc, cdr(next_e), Nil), base);
      }
    }

    insert_coi_hash(var, base);

    if(nonassign) {
      nonassign_vars = Set_Union(nonassign_vars, Set_MakeSingleton(var));
    }

    if (COI_VERBOSE) {
      fprintf(nusmv_stdout,"Variable  ");
      print_node(nusmv_stdout,var);
      fprintf(nusmv_stdout,"\n");

      if(nonassign) {
	fprintf(nusmv_stdout,"  Has non-assign constraints\n");
      }

      fprintf(nusmv_stdout,"  Initial coi: ");
      print_node(nusmv_stdout,base);
      fprintf(nusmv_stdout,"\n");
    }
  } /* vars iteration */

  {
    boolean changed = true;

    while (changed) {
      
      if (COI_VERBOSE) {
	fprintf(nusmv_stdout,"*** ITERATE COI ***\n");
      }

      changed = false;

      for (iter = NodeList_get_first_iter(vars); 
	   ! ListIter_is_end(iter); iter = ListIter_get_next(iter)) {

	node_ptr var = NodeList_get_elem_at(vars, iter);

	Set_t coi = lookup_coi_hash(var);
	Set_t old_coi = coi;

	Set_t tmp;

	int nonassign = (Set_IsMember(nonassign_vars, var) == true);

	if (COI_VERBOSE) {
	  fprintf(nusmv_stdout,"Variable ");
	  print_node(nusmv_stdout, var);
	  fprintf(nusmv_stdout,"\n");

	  if(nonassign) {
	    fprintf(nusmv_stdout,"  Has non-assign constraints\n");
	  }

	  fprintf(nusmv_stdout,"  Old coi: ");
	  print_node(nusmv_stdout, old_coi);
	  fprintf(nusmv_stdout,"\n");
	}

	for(tmp = old_coi; Set_IsEmpty(tmp) == false;
	    tmp = Set_GetRest(tmp)) {
	  node_ptr name = (node_ptr)Set_GetFirst(tmp);
	  coi = Set_Union(coi, lookup_coi_hash(name));;
	}

	if (coi != old_coi) {
	  changed = true;
	  insert_coi_hash(var, coi);
	  
	  if (COI_VERBOSE) {
	    fprintf(nusmv_stdout,"  New coi: ");
	    print_node(nusmv_stdout, coi);
	    fprintf(nusmv_stdout,"\n");
	  }
	}
	else {
	  if (COI_VERBOSE) {
	    fprintf(nusmv_stdout,"  No changes\n");
	  }
	}

	if (nonassign) {
	  for(tmp = coi; Set_IsEmpty(tmp) == false;
	      tmp = Set_GetRest(tmp)) {
	    node_ptr name = (node_ptr)Set_GetFirst(tmp);

	    if (Set_IsMember(nonassign_vars, name) == false) {
	      if (COI_VERBOSE) {
		fprintf(nusmv_stdout,
			"  Found non-assignment constraints for ");
		print_node(nusmv_stdout,name);
		fprintf(nusmv_stdout,"\n");
	      }
	      nonassign_vars = Set_Union(nonassign_vars,
					 Set_MakeSingleton(name));
	      {
		Set_t other_coi = lookup_coi_hash(name);

		if (Set_IsMember(other_coi, var) == false) {
		  changed = true;
		  other_coi = Set_Union(other_coi, Set_MakeSingleton(var));
		  insert_coi_hash(name, other_coi);
		}
	      }
	    }
	  }
	}
      }
    }

    if (COI_VERBOSE) {
      fprintf(nusmv_stdout,"*** END COI ***\n");
    }
  }
}

