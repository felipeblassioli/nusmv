/**CFile***********************************************************************

  FileName    [compileCheck.c]

  PackageName [compile]

  Synopsis    [Performs semantic checks on the model.]

  Description [The routines to perform some the semantic checks.<br>
  The performed checks are:
  <ul>
  <li>undefined symbols</li>
  <li>multiple definition of symbols</li>
  <li>circular definition of symbols</li>
  <li>assignment to input variables</li>
  </ul>]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and ITC-irst. 

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
#include "utils/ustring.h"
#include "utils/error.h"
#include "utils/assoc.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "utils/utils_io.h" 
#include "enc/enc.h"

static char rcsid[] UTIL_UNUSED = "$Id: compileCheck.c,v 1.5.2.24.2.3 2005/07/14 15:58:25 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [The constant hash used in the checking phase.]

  Description [The role is similar to that of <tt>constant_hash</tt>,
  but the associated value is not an ADD, it's the atom itself. It is
  used only for checking purpose.]

  SeeAlso     [constant_hash]

******************************************************************************/
static hash_ptr check_constant_hash;
void init_check_constant_hash() {
  check_constant_hash = new_assoc();
 }
void insert_check_constant_hash(node_ptr x, node_ptr y) {
  insert_assoc(check_constant_hash, x, y);
}
node_ptr lookup_check_constant_hash(node_ptr x) {
  return(find_assoc(check_constant_hash, x));
}
void clear_check_constant_hash() {
  clear_assoc(check_constant_hash);
}

/**Variable********************************************************************

  Synopsis    [Hash used to check assignments.]

  Description [This hash is used to detect multiple definitions. It
  associates to each symbol of the form <em>next(x)</em>,
  <em>init(x)</em>, <em>x</em>, the line number of the input file
  where a value is assigned to it (eg. <em>next(x) := expr</em>).]

  SeeAlso     []

******************************************************************************/
static hash_ptr global_assign_hash;
static void init_global_assign_hash() { global_assign_hash = new_assoc(); }
static void insert_global_assign_hash(node_ptr x, node_ptr y) {
  insert_assoc(global_assign_hash, x, y);
}
static node_ptr lookup_global_assign_hash(node_ptr x) {
  return(find_assoc(global_assign_hash, x));
}
static void clear_global_assign_hash() {clear_assoc(global_assign_hash);}


/**Variable********************************************************************

  Synopsis    [Hash used to check multiple and circular assignments.]

  Description [This hash is used in two different phases of the
  checking. 
  <ol>
  <li>The first deal with multiple definition. During this phase
      the data associated to symbols of the form <em>next(x)</em>,
      <em>init(x)</em> is the body of the assignment (eg, if you have the
      following assignment <em>next(x) := x & y;</em>, then the data
      associated to the symbol <em>next(x)</em> is <em>x & y</em>).</li>
  <li>The second deal with circular definition. In this phase the data
       associated to each symbol is extracted to check the body, and it is
       replaced with <tt>FAILURE_NODE</tt> or <tt>CLOSED_NODE</tt>.</li>
  </ol>]

  SeeAlso     []

******************************************************************************/
static hash_ptr assign_hash;
static void init_assign_hash() { assign_hash = new_assoc(); }
static void insert_assign_hash(node_ptr x, node_ptr y) { insert_assoc(assign_hash, x, y);}
static node_ptr lookup_assign_hash(node_ptr x) {return(find_assoc(assign_hash, x));}
static void clear_assign_hash() {clear_assoc(assign_hash);}


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void check_circular_assign ARGS((BddEnc_ptr enc, node_ptr, 
					node_ptr, boolean, boolean, boolean));

static void init_check_program_recur ARGS((node_ptr l));
static void init_check_program ARGS((node_ptr procs_exp));
static void check_circ ARGS((BddEnc_ptr enc, node_ptr n, 
			     node_ptr context, boolean, boolean));

static void check_assign ARGS((BddEnc_ptr enc, node_ptr n, node_ptr context, 
			       int mode));

static void check_assign_both ARGS((node_ptr v, int node_type, int lineno));
static void error_circular_assign ARGS((node_ptr n));
static void error_nested_next ARGS((node_ptr s));
static void error_unexpected_next ARGS((node_ptr s));
static void print_io_atom_stack_assign ARGS((FILE *));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Semantic checks on the model.]

  Description        [The main routine performing all the
  check on the read model.<br>
  First it loops on the body of processes to verify that there are no
  multiple assignments and circular definitions.<br>
  Then performs circularity checks on the list of SPECS,
  LTL SPECS, INVARIANT SPECS, FAIRNESS constraints.<br>
  The last operation performed consists in detecting multiple
  assignments between different modules.]

  SideEffects  []

******************************************************************************/
void Compile_CheckProgram(node_ptr procs, node_ptr spec_expr, node_ptr ltlspec_expr,
			  node_ptr invar_expr, node_ptr justice_expr, 
			  node_ptr compassion_expr)
{
  node_ptr procs_list=procs;
  BddEnc_ptr enc = Enc_get_bdd_encoding();

  /* Initialization of the hashes */
  init_global_assign_hash();
  init_assign_hash();

  /* Initializes check_constant_hash with process_selector elements. */
  init_check_program(procs);
  
  while(procs_list) { /* Loops over processes */
    node_ptr context = car(car(procs_list));
    node_ptr assign_expr = cdr(car(procs_list));

    /* Checks for multiple assignments: */
    if (opt_verbose_level_gt(options, 1)) {
      print_in_process("checking for multiple assignments", context);
    }

    check_assign(enc, assign_expr, Nil, 0);

    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stdout, "Done\n");
    }

    /* Checks for circular assignments: */
    if (opt_verbose_level_gt(options, 1)) {
      print_in_process("checking for circular assignments", context);
    }

    check_assign(enc, assign_expr, Nil, 1);

    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stdout, "Done\n");
    }

    clear_assign_hash();

    procs_list = cdr(procs_list);
  }

  {
    NodeList_ptr list;
    ListIter_ptr iter; 
    node_ptr l;

    /* checks CTL SPECs */
    l = spec_expr;
    while (l) { /*  Loops over ctl spec expressions */
      yylineno = node_get_lineno(cdr(car(l)));
      check_circ(enc, car(l), Nil, false, false);
      l = cdr(l);
    }

    /* checks LTL SPECs */
    l = ltlspec_expr;
    while (l) { /*  Loops over ltl spec expressions */
      yylineno = node_get_lineno(cdr(car(l)));
      check_circ(enc, car(l), Nil, false, false);
      l = cdr(l);
    }

    /* checks INVAR SPECs */
    l = invar_expr;
    while (l) { /* Loops over invariant spec  expressions */
      yylineno = node_get_lineno(cdr(car(l)));
      check_circ(enc, car(l), Nil, false, false);
      l = cdr(l);
    }

    /* checks JUSTICE */
    l = justice_expr;
    while (l) { /* Loops over fairness expressions */
      yylineno = node_get_lineno(cdr(car(l)));
      check_circ(enc, car(l), Nil, false, false);
      l = cdr(l);
    }

    /* checks COMPASSION */
    l = compassion_expr;
    while (l) { /* Loops over fairness expressions */
      yylineno = node_get_lineno(cdr(car(l)));
      check_circ(enc, car(car(l)), Nil, false, false);
      check_circ(enc, cdr(car(l)), Nil, false, false);
      l = cdr(l);
    }
  
    /* checks state variables */
    list = Encoding_get_state_vars_list(BddEnc_get_symbolic_encoding(enc));
    iter = NodeList_get_first_iter(list);
    while (! ListIter_is_end(iter)) { /* Loops over variable list */
      node_ptr v = NodeList_get_elem_at(list, iter);
      int lineno = (int) lookup_global_assign_hash(v);
  
      if (lineno != 0) {
        check_assign_both(v, NEXT, lineno);
        check_assign_both(v, SMALLINIT, lineno);
      }
      iter = ListIter_get_next(iter);
    }
  }

  clear_global_assign_hash();
  clear_assign_hash();
  if (opt_batch(options)) { clear_check_constant_hash(); }
}

/**Function********************************************************************

  Synopsis           [Gets list of variables in the expression]

  Description        [Calls Formula_GetDependencies to get the set of
  variables which appear in the expression. This is then converted to the
  type NodeList_ptr and returned. It is the responsibility of the
  caller to destroy the NodeList object]

  SideEffects        [NodeList object created]

  See Also           []

******************************************************************************/
NodeList_ptr Compile_get_expression_dependencies(const Encoding_ptr senc,
						 Expr_ptr expression)
{
  Set_t dependencies = Formula_GetDependencies(senc, (node_ptr) expression, Nil);
  node_ptr list = Set_Set2List(dependencies);

  return NodeList_create_from_list(list);
}


/**Function********************************************************************

  Synopsis           [Checks expressions for illegal occurrences of input vars]

  Description        [Checks the TRANS, INIT, INVAR and ASSIGN statements to
  make sure that input variables are not used where they should not be. That
  is, anywhere in a TRANS, INIT or INVAR statement and within next expressions
  in the init and next sections of an ASSIGN statement.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void compileCheckForInputVars(Encoding_ptr senc,
			      node_ptr trans_expr, 
			      node_ptr init_expr,
			      node_ptr invar_expr,
			      node_ptr assign_expr)
{
  compileCheckInitForInputVars(senc, init_expr); 
  compileCheckInvarForInputVars(senc, invar_expr);
  compileCheckTransForInputVars(senc, trans_expr);
  compileCheckAssignForInputVars(senc, assign_expr);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the data structure to perform semantic checks.]

  Description        [Initializes the data structure to perform
  semantic checks. The check constant hash is filled in with the names
  of the declared processes.]

  SideEffects        []

******************************************************************************/
static void init_check_program(node_ptr procs_exp)
{
  node_ptr procs_name_list = map(car, procs_exp);
  init_check_program_recur(procs_name_list);
}

/**Function********************************************************************

  Synopsis           [Recursive step of <tt>init_check_program</tt>]

  Description        [Performs recursive step of
  <tt>init_check_program</tt>. Loops over the list of process names
  and inserts the process symbolic name in the <tt>check_constant_hash</tt>.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void init_check_program_recur(node_ptr l)
{
  if (l == Nil) internal_error("init_check_program_recur: l = Nil");
  if (cdr(l) == Nil) {
    node_ptr v = find_atom(car(l));
    if (lookup_check_constant_hash(v)) return;
    if (v && (node_get_type(v) == ATOM)) insert_check_constant_hash(v, v);
    return;
  }
  else {
    init_check_program_recur(odd_elements(l));
    init_check_program_recur(even_elements(l));
  }
  return;
}

/**Function********************************************************************

  Synopsis           [Checks for circular definitions.]

  Description        [This function checks for circular definition of
  any kind. This function is able to detect circularity of the
  following kinds:
  <ul>
     <li><code>next(x) := alpha(next(x))<code></li>
     <li><code>next(x) := next(alpha(x))<code></li<
     <li>any combination of the two above.</li>
     <li><code>x := alpha(x)</code>
  </ul>
  where <code>alpha(x)</code> (<code>alpha(next(x))</code>) is a
  formula in which the variable <code>x</code> (<code>next(x)</code>)
  occurs. Notice that <code>next(alpha(x))</code> can be rewritten in
  term of <code>next(x)</code>, since the next operator distributes
  along all kind of operators.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void check_circ(BddEnc_ptr enc, node_ptr n, node_ptr context, 
		       boolean is_next, boolean lhs_is_next)
{
  if (n == Nil) return;

  switch (node_get_type(n)) {

  case NUMBER:
  case TRUEEXP:
  case FALSEEXP:
    return;

  case ATOM:
    {
      node_ptr constant = lookup_check_constant_hash(find_atom(n));
      node_ptr name; 

      if (!is_next) {
	node_ptr par;
	boolean is_symb;
	name =  find_node(DOT, context, find_atom(n));
	par = lookup_param_hash(name);

	is_symb = 
	  Encoding_is_symbol_declared(BddEnc_get_symbolic_encoding(enc), name);
	
	if ((par && is_symb) || (is_symb && constant) || (constant && par)) {
	  rpterr("atom \"%s\" is ambiguous.\n", 
		 str_get_text(node_get_lstring(n)));
	}
	
	if (par) { 
	  check_circ(enc, par, context, is_next, lhs_is_next); 
	  return;
	}

	if (constant) return;
	check_circular_assign(enc, name, context, is_next, false, lhs_is_next);
	return;
      } 
      else { /* next case */
	if (constant) return; /* if it is a constant does not matter */
        name = find_node(DOT, context, find_atom(n));
        check_circular_assign(enc, name, context, is_next, false, lhs_is_next); 
        return;
      }
    } /* end of case ATOM */
    
  case BIT:
  case DOT:
  case ARRAY: 
    {
      node_ptr t = CompileFlatten_resolve_name(n, context);      
      check_circular_assign(enc, t, context, is_next, false, lhs_is_next);
      return;
    }

  case CONTEXT:
    check_circ(enc, cdr(n), car(n), is_next, lhs_is_next);
    return;

  case NEXT: 
    /* to handle with nesting of formulas, and unexpected next: */
    if (is_next) { error_nested_next(n); }
    if (!lhs_is_next) { error_unexpected_next(n); }

    check_circ(enc, car(n), context, true, lhs_is_next);
    return;
  
  default:
    check_circ(enc, car(n), context, is_next, lhs_is_next);
    check_circ(enc, cdr(n), context, is_next, lhs_is_next);
  }

}


/**Function********************************************************************

  Synopsis           [Performs circular assignment checking]

  Description        [Checks for circular assignments in the model. If
  there are any, then an error is generated. NEXT operator, if any, 
  must be stripped away from given expression 'n', and in that case is_next 
  must be set to true. Parameter is_lhs is true at the first call (done 
  with the first left-hand-side value (the assigned value)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void check_circular_assign(BddEnc_ptr enc, node_ptr n, 
				  node_ptr context, boolean is_next, 
				  boolean is_lhs, boolean lhs_is_next)
{
  node_ptr t;
  node_ptr next_n;
  boolean is_rhs_next;

  if ((n != Nil) && (is_next) && (node_get_type(n) == NUMBER)) return;

  next_n = find_node(NEXT, n, Nil);
  if (is_next) {
    t = lookup_assign_hash(next_n);
  }
  else {
    /* check if it is a next assignment or a normal assignment */
    t = lookup_assign_hash(n);
    if (t == Nil) {
      /* check if it is an init assignment */
      t = lookup_assign_hash(find_node(SMALLINIT, n, Nil));;
    }
  }
  
  if (t == CLOSED_NODE) return;
  if (t == FAILURE_NODE) { error_circular_assign(n); }

  if (t == Nil) {
    Encoding_ptr senc;
    senc = BddEnc_get_symbolic_encoding(enc);      
    /* it might be a define: */
    if (Encoding_is_symbol_define(senc, n)) {
      /* switch to define ctx and body, and continue: */
      context = Encoding_get_define_context(senc, n);
      t = Encoding_get_define_body(senc, n);
    } 
    else return;
  }
  else {
    is_rhs_next = (node_get_type(t) == NEXT);
    if (!is_lhs && is_next && is_rhs_next) { error_nested_next(n); }

    is_lhs = true; /* we found an assignment: restart the search */
  }

  if (t == Nil) {
    if (lookup_check_constant_hash(n)) return;
    error_undefined(n);
  }

  insert_assign_hash(is_next ? next_n : n, FAILURE_NODE);
  io_atom_push(is_next? next_n : n);

  /* if this is the first time this function is called, rhs decides if 
     there is NEXT operator, otherwise keeps the current mode */
  check_circ(enc, t, context, is_lhs? is_rhs_next : is_next, lhs_is_next);

  io_atom_pop();
  insert_assign_hash(is_next ? next_n : n, CLOSED_NODE);
}

/**Function********************************************************************

  Synopsis           [Checks for multiple or circular assignments.]

  Description        [This function detects either multiple or
  circular assignments in "context" regarding to "mode".
  If mode is equal to 0 (zero) then it checks for multiple assignments
  or symbols redefinition. Otherwise it performs checks for circular
  assignments.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void check_assign(BddEnc_ptr enc, node_ptr n, node_ptr context, 
			 int mode)
{
  if (n == Nil) return;
  yylineno = node_get_lineno(n);

  switch (node_get_type(n)) {
  case AND:
    check_assign(enc, car(n), context, mode);
    check_assign(enc, cdr(n), context, mode);
    break;

  case CONTEXT:
    check_assign(enc, cdr(n), car(n), mode);
    break;

  case EQDEF:
    {
      node_ptr t1, t2;
      NodeList_ptr var_list;
      if ((node_get_type(car(n)) == SMALLINIT) ||
          (node_get_type(car(n)) == NEXT)) {
	t1 = CompileFlatten_resolve_name(car(car(n)), context);
	t2 = find_node(node_get_type(car(n)), t1, Nil);
      }
      else {
        t1 = t2 = CompileFlatten_resolve_name(car(n), context);
      }

      if (mode == 0) { 
	/* Checking for multiple assignments */
	Encoding_ptr senc = BddEnc_get_symbolic_encoding(enc);

	if (! Encoding_is_symbol_declared(senc, t1)) { error_undefined(t1); }

        if (Encoding_is_symbol_input_var(senc, t1)) {
          error_assign_input_var(car(n));
        }
	if (! Encoding_is_symbol_state_var(senc, t1)) {
	  error_redefining(t1);
	}

	if (lookup_assign_hash(t2)) { error_multiple_assignment(t2); }
	insert_assign_hash(t2, find_node(CONTEXT, context, cdr(n)));
	insert_global_assign_hash(t2, (node_ptr) yylineno);
        }
      else { /* Checking for circular assignments */
	if (node_get_type(t2) == NEXT) {
	  check_circular_assign(enc, car(t2), context, true, true, true);
	}
	else {
	  check_circular_assign(enc, t2, context, false, true, false);
	}
      }
      break;
    }

  default:
    internal_error("check_assign: type = %d", node_get_type(n));
  }
}

/**Function********************************************************************

  Synopsis           [Given a variable, it checks if there are
  multiple assignments to it.]

  Description        [Checks if there exists in the model an
  assignments of type <tt>node_type</tt> for variable <tt>v</tt>. If
  such an assignment exists, then an error is generated.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void check_assign_both(node_ptr v, int node_type, int lineno)
{
  node_ptr v1 = find_node(node_type, v, Nil);
  int lineno2 = (int) lookup_global_assign_hash(v1);

  if (lineno2) error_assign_both(v, v1, lineno, lineno2);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void print_io_atom_stack_assign(FILE * fd){
  while(!io_atom_isempty()){
    node_ptr s = io_atom_head();

    io_atom_pop();
    fprintf(fd, "in definition of ");
    print_node(fd, s);
    {
      int lineno = (int) lookup_global_assign_hash(s);

      if (lineno) fprintf(fd," at line %d", lineno);
      fprintf(fd, "\n");
    }
  }
}

static void error_circular_assign(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "recursively defined: ");
  print_node(nusmv_stderr, s);
  fprintf(nusmv_stderr,"\n");
  print_io_atom_stack_assign(nusmv_stderr);
  clear_assign_hash();
  finish_parsing_err();
}

static void error_nested_next(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "nested NEXT operators: ");
  print_node(nusmv_stderr, s);
  fprintf(nusmv_stderr,"\n");
  print_io_atom_stack_assign(nusmv_stderr);
  clear_assign_hash();
  finish_parsing_err();
}


static void error_unexpected_next(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "found unexpected next operator: ");
  print_node(nusmv_stderr, s);
  fprintf(nusmv_stderr,"\n");
  print_io_atom_stack_assign(nusmv_stderr);
  clear_assign_hash();
  finish_parsing_err();
}

/**Function********************************************************************

  Synopsis           [Checks flattened assign statement for input variables]

  Description        [If the flattened assign statement contains input
  variables then this function will print out an error message. Note that
  input variables are allowed in some parts of an assign statement. They're
  not allowed anywhere in an init section and cannot be contained within a
  next statement inside a next declaration.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void compileCheckAssignForInputVars(Encoding_ptr senc, node_ptr assign)
{
  if (assign == Nil) return;

  switch (node_get_type(assign)) {
  case ATOM:
  case NUMBER: 
    /* this case should be removed and replaced with an assert. No
       other cases are possible. */
    break;

  case DOT: 
    {
      node_ptr vname = assign;
      node_ptr lhsa = assign;
      node_ptr stored = lookup_assign_db_hash(lhsa);
      
      if (stored != (node_ptr) NULL) {
        if (car(stored) != Nil) {
          NodeList_ptr var_list = Compile_get_expression_dependencies(senc, car(stored));

	  if (Encoding_list_contains_input_vars(senc, var_list)) {
	    error_assign_exp_contains_input_vars(assign);
	  }
	  NodeList_destroy(var_list);
        }
      }
      break;
    }

  case ARRAY:
    if (Encoding_is_symbol_input_var(senc, assign)) {
      error_assign_exp_contains_input_vars(assign);
    }
    break;

  case SMALLINIT: 
    {
      node_ptr vname = car(assign);
      node_ptr lhsa = find_node(SMALLINIT, vname, Nil);
      node_ptr stored = lookup_assign_db_hash(lhsa);
      
      if (stored != (node_ptr)NULL) {
	if (car(stored) != Nil) {
	  NodeList_ptr var_list = Compile_get_expression_dependencies(senc, car(stored));
	  
	  if (Encoding_list_contains_input_vars(senc, var_list)) {
	    error_assign_exp_contains_input_vars(assign);
	  }
	  NodeList_destroy(var_list);
        }
      }
      break;
    }

  case NEXT: 
    /* We don't care about presence of inputs in next assign */
    break;
  

  case CONS:
  case AND:
    compileCheckAssignForInputVars(senc, car(assign));
    compileCheckAssignForInputVars(senc, cdr(assign));
    break;
    
  case EQDEF:
    compileCheckAssignForInputVars(senc, car(assign));
    break;      

  default:
    fprintf(nusmv_stderr, "compileCheckAssignForInputVars: unrecognized token (%d)\n", node_get_type(assign));
    nusmv_assert(0);

  } /* switch */

}


/**Function********************************************************************

  Synopsis           [Checks expression for input variables in next statements]

  Description        [Returns true if the expression contains a next statement
  which itself has an input variable in it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean compileExpressionHasNextInputs(Encoding_ptr senc, node_ptr expr)
{
  boolean res = false;

  if (expr != Nil) {
    switch (node_get_type(expr)) {
    case ATOM:
    case NUMBER: {
      break;
    }
    case NOT: {
      res = compileExpressionHasNextInputs(senc, car(expr));
      break;
    }
    case DOT: {
      res = compileExpressionHasNextInputs(senc, cdr(expr));
      break;
    }
    case NEXT: {
      NodeList_ptr var_list = Compile_get_expression_dependencies(senc, expr);
      res = Encoding_list_contains_input_vars(senc, var_list);
      NodeList_destroy(var_list);
      break;
    }
    default:
      res = (compileExpressionHasNextInputs(senc, car(expr)) ||
	     compileExpressionHasNextInputs(senc, cdr(expr)));
      break;
    }
  }
  return res;
}

/**Function********************************************************************

  Synopsis           [Checks flattened trans statement for input variables]

  Description        [If the flattened trans statement contains input
  variables within next() statements then this function will print out an
  error message.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void compileCheckTransForInputVars(Encoding_ptr senc, node_ptr trans)
{
  if (!(trans == Nil) && compileExpressionHasNextInputs(senc, trans)) {
    error_trans_exp_contains_input_vars(trans);
  }
}

/**Function********************************************************************

  Synopsis           [Checks flattened invar statement for input variables]

  Description        [If the flattened invar statement contains input
  variables then this function will print out an error message.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/

void compileCheckInvarForInputVars(Encoding_ptr senc, node_ptr invar)
{
  NodeList_ptr var_list = Compile_get_expression_dependencies(senc, invar);
  if (Encoding_list_contains_input_vars(senc, var_list)) {
    error_invar_exp_contains_input_vars(invar);
  }
  NodeList_destroy(var_list);
}

/**Function********************************************************************

  Synopsis           [Checks flattened init statement for input variables]

  Description        [If the flattened init statement contains input
  variables then this function will print out an error message.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void compileCheckInitForInputVars(Encoding_ptr senc, node_ptr init)
{
  NodeList_ptr var_list = Compile_get_expression_dependencies(senc, init);
  if (Encoding_list_contains_input_vars(senc, var_list)) {
    error_init_exp_contains_input_vars(init);
  }
  NodeList_destroy(var_list);
}
