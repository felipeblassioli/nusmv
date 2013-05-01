/**CFile***********************************************************************

  FileName    [compileBEval.c]

  PackageName [compile]

  Synopsis    []

  Description [Conversion from scalar expressions to boolean expressions.]

  SeeAlso     [optional]

  Author      [Alessandro Cimatti and Marco Roveri]

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

#include "compile/compile.h"
#include "compileInt.h"
#include "utils/assoc.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "utils/utils_io.h" /* for indent_node */
#include "utils/range.h"
#include "enc/enc.h"

static char rcsid[] UTIL_UNUSED = "$Id: compileBEval.c,v 1.13.4.12.2.3 2004/10/27 17:00:15 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define INT_VAR_PREFIX "__det_"


#define NODE_PTR(x)  \
         ((node_ptr) x)

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [Counter used to create determinization variables.]

  Description [Each time a new determinization variable is introduced
  by the <tt>mk_new_var</tt> primitive, the current value of
  <tt>det_counter</tt> is used, and then it is incremented.]

  SeeAlso     [mk_new_var]

******************************************************************************/
static int det_counter = 0;

static hash_ptr expr2bexpr_hash = (hash_ptr) NULL;

void init_expr2bexp_hash() 
{
  nusmv_assert(expr2bexpr_hash == (hash_ptr) NULL);
  expr2bexpr_hash = new_assoc();
}

void quit_expr2bexpr_hash()
{
  if (expr2bexpr_hash != (hash_ptr) NULL) {
    free_assoc(expr2bexpr_hash);
    expr2bexpr_hash = (hash_ptr) NULL;
  }
}

static node_ptr make_key(node_ptr expr, boolean a, boolean b) 
{
  int j = 0;  

  j += a? 1 : 0;
  j += b? 2 : 0;

  return find_node(CONTEXT, expr, (node_ptr) j);
}

static void expr2bexpr_hash_insert_entry(node_ptr expr, node_ptr bexpr, 
					 boolean a, boolean b)
{
  nusmv_assert(expr2bexpr_hash != (hash_ptr) NULL);
  insert_assoc(expr2bexpr_hash, make_key(expr, a, b), bexpr);
}

static node_ptr 
expr2bexpr_hash_lookup_entry(node_ptr expr, boolean a, boolean b)
{
  nusmv_assert(expr2bexpr_hash != (hash_ptr) NULL);
  return find_assoc(expr2bexpr_hash, make_key(expr, a, b));
}


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static node_ptr expr2bexpr_recur ARGS((BddEnc_ptr enc, node_ptr, 
				       boolean, boolean));

static node_ptr 
scalar_atom2bexpr ARGS((BddEnc_ptr enc, node_ptr, boolean, boolean));

static node_ptr add2bexpr ARGS((add_ptr, boolean, boolean));

static node_ptr mk_new_var ARGS((Encoding_ptr senc));

static node_ptr 
add2bexpr_recur ARGS((BddEnc_ptr enc, add_ptr, boolean, boolean, hash_ptr));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Converts a scalar expression into a boolean expression.]

  Description        [Takes an scalar expression intended to evaluate
  to boolean, maps through booleans down to the atomic scalar
  propositions, builds the corresponding boolean function, and returns
  the resulting boolean expression.

  The conversion of atomic scalar proposition is currently performed
  by generating the corresponding ADD, and then printing it in terms
  of binary variables.]

  SideEffects        [None]

  SeeAlso            [detexpr2bexpr, expr2bexpr_recur]

******************************************************************************/
Expr_ptr expr2bexpr(Expr_ptr expr)
{
  node_ptr res;
  int temp = yylineno;
  BddEnc_ptr enc;

  if (expr == EXPR(NULL)) return expr;

  enc = Enc_get_bdd_encoding();

  yylineno = node_get_lineno( NODE_PTR(expr) );
  res = expr2bexpr_recur(enc, NODE_PTR(expr), false, true);
  yylineno = temp;
  return EXPR(res);
}


/**Function********************************************************************

  Synopsis           [Converts a scalar expression into a boolean expression.]

  Description        [Takes an scalar expression intended to evaluate
  to boolean, maps through booleans down to the atomic scalar
  propositions, builds the corresponding boolean function, and returns
  the resulting boolean expression.

  The conversion of atomic scalar proposition is currently performed
  by generating the corresponding ADD, and then printing it in terms
  of binary variables.

  An error is returned is determinization variables are introduced in
  the booleanization process.]

  SideEffects        [None]

  SeeAlso            [expr2bexpr, expr2bexpr_recur]

******************************************************************************/
Expr_ptr detexpr2bexpr(Expr_ptr expr)
{
  node_ptr res;
  int temp = yylineno;
  BddEnc_ptr enc;

  if (expr == EXPR(NULL)) return expr;
  
  enc = Enc_get_bdd_encoding();
  yylineno = node_get_lineno(expr);
  res = expr2bexpr_recur(enc, (node_ptr) expr, false, false);

  yylineno = temp;
  return EXPR(res);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Converts a generic expression into a boolean expression.]

  Description        [Takes an expression intended to evaluate to boolean,
   maps through booleans down to the atomic scalar propositions,
   builds the corresponding boolean function,
   and returns the resulting boolean expression.

   The conversion of atomic scalar proposition is currently performed
   by generating the corresponding ADD, and then printing it in terms
   of binary variables.

   The introduction of determinization variables is allowed only if flag
   <tt>allow_nondet</tt> is set to true.]

  SideEffects        [None]

  SeeAlso            [expr2bexpr, detexpr2bexpr]

******************************************************************************/
static node_ptr expr2bexpr_recur(BddEnc_ptr enc, node_ptr expr, 
				 boolean in_next, boolean allow_nondet)
{
  node_ptr res; 

  if (expr == Nil) return Nil;

  res = expr2bexpr_hash_lookup_entry(expr, in_next, allow_nondet);
  if (res != (node_ptr) NULL) return res;

  switch (node_get_type(expr)) {

  case TRUEEXP:
  case FALSEEXP:
    res = expr;
    break;

  case NUMBER: 
    {
      if (find_atom(expr) == one_number) {
	res = find_node(TRUEEXP, Nil, Nil);
      } else if (find_atom(expr) == zero_number) {
	res = find_node(FALSEEXP, Nil, Nil);
      } else {
	rpterr("Number that can not be casted to boolean");
      }
      break;
    }

  case BIT:
    {
      node_ptr name = CompileFlatten_resolve_name(expr, Nil);
      Encoding_ptr senc = BddEnc_get_symbolic_encoding(enc);
      nusmv_assert(Encoding_is_symbol_var(senc, name));
      
      res = name;
      break;
    }

  case DOT:
  case ATOM:
  case ARRAY: 
    {
      node_ptr name = CompileFlatten_resolve_name(expr, Nil);
      Encoding_ptr senc = BddEnc_get_symbolic_encoding(enc);

      if (Encoding_is_symbol_declared(senc, name)) { 
	node_ptr name_info;
	
	name_info = Encoding_lookup_symbol(senc, name);
	if (Encoding_is_symbol_define(senc, name)) {
	  /* define (rhs must be boolean, recur to check) */
	  node_ptr body = Encoding_get_define_flatten_body(senc, name);
	  if (body == (node_ptr) NULL) { error_undefined(name); }
	  res = expr2bexpr_recur(enc, body, in_next, allow_nondet);
	}
	else {
	  if (Encoding_is_symbol_var(senc, name)) {
	    /* variable, must be boolean */
	    if ( ! Utils_is_in_range(cdr(name_info), boolean_range) ) {
	      rpterr("Unexpected non boolean variable");
	    } 
	    
	    if (in_next) res = find_node(NEXT, name, Nil);
	    else res = name;
	  }
	  else { rpterr("Unexpected data structure"); }
	}
      }
      else { error_undefined(name); } 

      break;
    }
    
  case NOT:
    res= find_node(NOT,
		   expr2bexpr_recur(enc, car(expr), in_next, allow_nondet),
		   Nil);
    break;
  
  case CONS:
  case AND:
  case OR:
  case XOR:
  case XNOR:
  case IMPLIES:
  case IFF:
    res = find_node(node_get_type(expr),
		    expr2bexpr_recur(enc, car(expr), in_next, allow_nondet),
		    expr2bexpr_recur(enc, cdr(expr), in_next, allow_nondet));
    break;

  case CASE:
    res = find_node(CASE,
		    find_node(COLON,
	      expr2bexpr_recur(enc, car(car(expr)), in_next, allow_nondet),
	      expr2bexpr_recur(enc, cdr(car(expr)), in_next, allow_nondet)),
		    expr2bexpr_recur(enc, cdr(expr), in_next, allow_nondet));
    break;
    
  case NEXT: 
    nusmv_assert(!in_next);
    res = expr2bexpr_recur(enc, car(expr), true, allow_nondet);
    break;
    
  case EQDEF:
    {
      node_ptr var_name;
      node_ptr var;
      node_ptr lhs = car(expr);
      node_ptr rhs = cdr(expr);

      switch (node_get_type(lhs)) {
      case SMALLINIT:
        var_name = car(lhs);
        var = car(lhs);
	break;
	
      case NEXT:
        var_name = car(lhs);
        var = lhs;
	break;
	
      default:
        var_name = lhs;
        var = lhs;
      }

      {
        node_ptr name = CompileFlatten_resolve_name(var_name, Nil);
	Encoding_ptr senc = BddEnc_get_symbolic_encoding(enc);

        if (!Encoding_is_symbol_declared(senc, name)) {
	  error_undefined(name);
        } 
	
	if (Encoding_is_symbol_var(senc, name)) {
          if (Encoding_get_var_range(senc, name) == boolean_range) {
            /* boolean variable, rhs will be determinized */
            if (node_get_type(lhs) == NEXT) var = find_node(NEXT, name, Nil);
            else var = name;
            
            res = find_node(IFF, var, expr2bexpr_recur(enc, rhs, in_next, 
						       allow_nondet));
          } 
	  else {
            /* scalar variable */
            nusmv_assert(!in_next);
            res = scalar_atom2bexpr(enc, expr, in_next, allow_nondet);
          }
	} 
	else { rpterr("Unexpected data structure, variable was expected"); }
        
      }
      break;
    } /* end of case EQDEF */

  /* Predicates over scalar terms (guaranteed to return boolean) */
  case LT:
  case GT:
  case LE:
  case GE:

  /* Predicates over possibly scalar terms (guaranteed to return boolean) */
  case EQUAL:
  case NOTEQUAL:
  
  /* Function symbols over scalar terms Might return boolean, but
    check is needed Assumption: if boolean is returned, then the
    function is determinized by introducing a variable on the {0,1}
    leaves. */
  case PLUS:
  case MINUS:
  case TIMES:
  case DIVIDE:
  case MOD:
  case UNION:
  case SETIN: 
    res = scalar_atom2bexpr(enc, expr, in_next, allow_nondet);
    break;
  

  /* UNARY CTL/LTL OPERATORS */
  case EX:
  case AX:
  case EG:
  case AG:
  case EF:
  case AF:
  case OP_NEXT:
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_ONCE:
  case OP_GLOBAL:
  case OP_HISTORICAL:
    res = find_node(node_get_type(expr),
		    expr2bexpr_recur(enc, car(expr), in_next, allow_nondet),
		    Nil);
    break;
  

  /* BINARY CTL/LTL OPERATORS */
  case EU:
  case AU:
  case MINU:
  case MAXU:
  case UNTIL:
  case RELEASES:
  case SINCE: 
  case TRIGGERED: 
    res = find_node(node_get_type(expr),
		    expr2bexpr_recur(enc, car(expr), in_next, allow_nondet),
		    expr2bexpr_recur(enc, cdr(expr), in_next, allow_nondet));
    break;
    

  /* BOUNDED TEMPORAL OPERATORS (cdr is range, no recursion needed) */
  case EBF:
  case ABF:
  case EBG:
  case ABG:
  case EBU:
  case ABU: 
    res = find_node(node_get_type(expr),
		    expr2bexpr_recur(enc, car(expr), in_next, allow_nondet),
		    cdr(expr));
    break;
  

  case CONTEXT:
    res = expr2bexpr_recur(enc, cdr(expr), in_next, allow_nondet);
    break;
  

  case TWODOTS: 
    rpterr("Unexpected TWODOTS node");
    res = (node_ptr) NULL;
    break;
  
  default: 
    internal_error("expr2bexpr_recur: type = %d\n", node_get_type(expr));
    res = (node_ptr) NULL;
  }

  /* updates the results hash  */
  if (res != (node_ptr) NULL) {
    expr2bexpr_hash_insert_entry(expr, res, in_next, allow_nondet);
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Converts an atomic expression into the corresponding (boolean)
  expression.]

  Description        [Takes an atomic expression and converts it into
  a cooresponding boolean expression.

  The introduction of determinization variables is allowed only if flag
  <tt>allow_nondet</tt> is set to true.]

  SideEffects        [A new boolean variable can be introduced.]

  SeeAlso            [add2bexpr_recur, mk_new_var]

******************************************************************************/
static node_ptr scalar_atom2bexpr(BddEnc_ptr enc, node_ptr expr, 
				  boolean in_next, boolean allow_nondet)
{
  node_ptr res;
  int temp = yylineno;
  add_ptr bool_add = BddEnc_expr_to_add(enc, expr, Nil);

  yylineno = node_get_lineno(expr);
  res = add2bexpr(bool_add, in_next, allow_nondet);
  add_free(dd_manager, bool_add);
  yylineno = temp;

  return res;
}


/**Function********************************************************************

  Synopsis           [Converts a ADD into the corresponding (boolean)
  expression.]

  Description        [Takes an ADD with leaves 0, 1, or {0,1}.

   The case of {0,1} leaves is allowed only if flag
   <tt>allow_nondet</tt> is set to true.]

   Recurs down on the structure of the ADD, and maps each non terminal
   node into an if-then-else expression, maps 0 and 1 terminal nodes
   into true and false expressions, and maps {0,1} into a newly
   introduced variable to determinize the expression.]

  SideEffects        [A new boolean variable can be introduced.]

  SeeAlso            [add2bexpr_recur, mk_new_var]

******************************************************************************/
static node_ptr add2bexpr(add_ptr bool_add, boolean in_next, boolean allow_nondet)
{
  node_ptr result;
  int reordering = 0;
  dd_reorderingtype rt;
  BddEnc_ptr enc = Enc_get_bdd_encoding();

  hash_ptr lc = st_init_table(st_ptrcmp, st_ptrhash);
  if (lc == (hash_ptr) NULL) {
    fprintf(nusmv_stderr, "add2bexpr: unable to allocate local hash.\n");
    nusmv_exit(1);
  }

  /* If dynamic reordering is enabled, it is temporarily disabled */
  if ((reordering = dd_reordering_status(dd_manager, &rt))) {
    dd_autodyn_disable(dd_manager);
  }

  result = add2bexpr_recur(enc, bool_add, in_next, allow_nondet, lc);

  /* If dynamic reordering was enabled, then it is enabled */
  if (reordering == 1) {
    dd_autodyn_enable(dd_manager, rt);
  }
  st_free_table(lc);

  return(result);
}

/**Function********************************************************************

  Synopsis           [Converts a ADD into the corresponding (boolean)
  expression.]

  Description        [Auxiliary function for add2bexpr.]

  SideEffects        [A new boolean variable can be introduced.]

  SeeAlso            [add2bexpr, Utils_is_in_range]

******************************************************************************/
static node_ptr add2bexpr_recur(BddEnc_ptr enc,
				add_ptr bool_add,
				boolean in_next,
				boolean allow_nondet,
				hash_ptr lc)
{
  DdManager* dd = BddEnc_get_dd_manager(enc);
  node_ptr result = (node_ptr) NULL;

  nusmv_assert((lc != (hash_ptr) NULL));
  nusmv_assert((dd != (DdManager*) NULL));
  nusmv_assert((bool_add != (add_ptr) NULL));

  /* base case */
  if (add_isleaf(bool_add)) {
    node_ptr leaf = add_get_leaf(dd, bool_add);
    if (add_is_one(dd, bool_add)) return NODE_PTR(Expr_true());

    if (add_is_zero(dd, bool_add)) return NODE_PTR(Expr_false());

    /* It something different from true or false */
    if (Utils_is_in_range(leaf, boolean_range)) {
      if (llength(leaf) == 1) {
	/* it is (cons (number 0) Nil) or (cons (number 1) Nil) and
	   nothing must be created */
	if (car(leaf) == one_number) {
	  return NODE_PTR(Expr_true());
	}
	else if (car(leaf) == zero_number) {
	  return NODE_PTR(Expr_false());
	}
	else {
	  fprintf(nusmv_stderr,
		  "add2bexpr_recur: Attempting to convert a non boolean set.\n");
	  indent_node(nusmv_stderr, "value = ",
		      add_get_leaf(dd, bool_add), "\n");
	  nusmv_exit(1);
	}
      }
      else {
	/* a new fresh variable is needed to determinize the result */
	if (! allow_nondet) {
	  fprintf(nusmv_stderr,
		  "add2bexpr_recur: Attempting to convert a nondeterministic expression.\n");
	  nusmv_exit(1);
	}
	result = NODE_PTR( 
	   Expr_ite(EXPR(mk_new_var(BddEnc_get_symbolic_encoding(enc))), 
		    Expr_true(), Expr_false()) );
      }
    }
    else {
      fprintf(nusmv_stderr,
	      "add2bexpr_recur: Attempting to convert a non boolean set.\n");
      indent_node(nusmv_stderr, "value = ", add_get_leaf(dd, bool_add), "\n");
      nusmv_exit(1);
    }
  }
  
  else {
    /* step case */
    node_ptr t, e, var;
    int index;

    if (st_lookup(lc, (char *)bool_add, (char **)&result)) {
      return result;
    }

    index = add_index(dd, bool_add);

    t = add2bexpr_recur(enc, add_then(dd, bool_add), in_next, 
			allow_nondet, lc);
    if (t == (node_ptr) NULL) return (node_ptr) NULL;

    e = add2bexpr_recur(enc, add_else(dd, bool_add), 
			in_next, allow_nondet, lc);
    if (e == (node_ptr) NULL) return (node_ptr) NULL;

    var = BddEnc_get_var_name_from_dd_index(enc, index);
    if (var == Nil) {
      fprintf(nusmv_stderr,
              "add2bexpr_recur: No variable associated to BDD variable %d\n",
              index);

      return (node_ptr)NULL;
    }

    if (in_next == true) {
      var = NODE_PTR(Expr_next(var));
    }

    result = NODE_PTR( Expr_ite(EXPR(var), EXPR(t), EXPR(e)) );
    if (result == (node_ptr)NULL) return (node_ptr) NULL;

    if (st_add_direct(lc, (char *) bool_add, 
		      (char *)result) == ST_OUT_OF_MEM) {
      fprintf(nusmv_stderr,
              "add2bexpr_recur: Unable to insert result in local hash.\n");
      return (node_ptr) NULL;
    }
  }

  return result;
}

/**Function********************************************************************

  Synopsis           [Create a new (boolean) variable.]

  Description        [Creates a fresh variable name, creates the
  corresponding name expressions, stores it as input variable, creates
  the corresponding encoding, builds and stores the resulting variable
  node. A side note, in order to submit a booleanized formula to the
  evaluator to obtain the corresponding BDD, a new BddEnc has to be
  created after the booleanization.] 

  SideEffects        [A new dd variable is created.]

  SeeAlso            [INT_VAR_PREFIX, det_counter, Compile_EncodeVar]

******************************************************************************/
static node_ptr mk_new_var(Encoding_ptr senc) 
{
  char name[20];
  node_ptr vname; 

  /* searches for a not already declared symbol for the new determ var: */
  while (true) {
    sprintf(name, "%s%d", INT_VAR_PREFIX, det_counter++);
    vname = find_node(DOT, Nil, sym_intern(name));
  
    if (!Encoding_is_symbol_declared(senc, vname)) {
      Encoding_declare_determ_var(senc, vname);
      break; 
    }
  }
  return expr2bexpr(vname);
}


