/**CFile***********************************************************************

  FileName    [compileCheckWff.c]

  PackageName [compile]

  Synopsis    [Checks for potential formula errors.]

  Description [Routines to check for potential formula errors. A potential
  error may arise when arguments do not match the formula operator domain,
  e.g. <tt>a AND b</tt>, where <tt>a</tt> or <tt>b</tt> are not booleans.]

  Author      [Andrea Morichetti and Marco Roveri]

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
#include "utils/assoc.h"
#include "parser/parser.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "enc/enc.h"

static char rcsid[] UTIL_UNUSED = "$Id: compileCheckWff.c,v 1.9.4.10 2004/05/27 13:06:06 nusmv Exp $";

/**Variable********************************************************************

  Synopsis    [The hash used to take care of already checked variables type.]

  Description [The hash used to take care of already checked variables type.
  It is used by <tt>check_wff_recur</tt>.]

  SeeAlso     [check_wff_recur]

******************************************************************************/
static hash_ptr wfftype_hash;
void init_wfftype_hash() { wfftype_hash = new_assoc(); }
void insert_wfftype_hash(node_ptr x, wff_result_type t)
{
  insert_assoc(wfftype_hash, x, (node_ptr)t);
}
wff_result_type lookup_wfftype_hash(node_ptr x)
{
  return((wff_result_type)find_assoc(wfftype_hash, x));
}
void clear_wfftype_hash() {clear_assoc(wfftype_hash);}

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static wff_result_type check_wff_recur(BddEnc_ptr enc, node_ptr, node_ptr);
static boolean wff_is_numeric(node_ptr);
static boolean wff_is_boolean(node_ptr);
static wff_result_type check_bool_num_sym(BddEnc_ptr enc, node_ptr);
static wff_result_type check_definition(BddEnc_ptr enc, node_ptr);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Checking formula potential errors and results.]

  Description        [It analizes a formula on the basis of its arguments and
  operators and returns 0 if the formula is ok, 1 otherwise]

  SideEffects        []

  SeeAlso            [check_wff_recur]
******************************************************************************/
int Compile_CheckWff(node_ptr wff, node_ptr context)
{
  int ret = 0;
  BddEnc_ptr enc = Enc_get_bdd_encoding();

  wff_result_type res = check_wff_recur(enc, wff, context);
  switch (res) {
  case wff_ERROR:
  case wff_NIL:
    ret = 1;

  default:
    ret = 0;
  }
  
  return ret;
}


/**Function********************************************************************

  Synopsis           [Checking formula potential errors and results. It is a
  front end of Compile_CheckWff]

  Description        [It analizes a formula on the basis of its arguments and
  operators and returns the domain type of the result that the formula will
  generate. Possible results are:
  <ul>
  <li> <b>BOOLEAN</b>
  <li> <b>SYMBOLIC</b>
  <li> <b>NUMERIC</b>
  <li> <b>ERROR</b> (erroneous mismatch between arguments)
  <li> <b>NIL</b> (internal error)
  </ul>
  ]

  SideEffects        []

  SeeAlso            [Compile_CheckWff, check_wff_recur]

******************************************************************************/
void check_wff(node_ptr wff, node_ptr context)
{
  wff_result_type res;
  int temp = yylineno;
  BddEnc_ptr enc = Enc_get_bdd_encoding();

  if (wff == Nil) {
    fprintf(nusmv_stderr,
      "Error: Compile_CheckWff. There is no wff to check\n");
  }
  else {
    if (context == Nil && node_get_type(wff) == CONTEXT) {
      context = car(wff);
      wff = cdr(wff);
    }
    yylineno = node_get_lineno(wff);
    res = check_wff_recur(enc, wff, context);
    yylineno = temp;
    switch (res) {
    case wff_NIL:
      fprintf(nusmv_stderr, "********** Something wrong in formula \"");
      fprintf(nusmv_stderr, "           ");
      print_node(nusmv_stderr, wff);
      if(context) {
  fprintf(nusmv_stdout, " IN ");
  print_node(nusmv_stdout, context);
      }
      fprintf(nusmv_stderr, "\"\n");
      break;;

    case wff_BOOLEAN:
    case wff_NUMERIC:
    case wff_SYMBOLIC:
      break;

    case wff_ERROR:
      {
  fprintf(nusmv_stdout, "WARNING: The evaluation of formula \"");
  print_node(nusmv_stdout, wff);
  if(context) {
    fprintf(nusmv_stdout, " IN ");
    print_node(nusmv_stdout, context);
  }
  fprintf(nusmv_stdout, "\" will produce an error.\n");
      }
      break;

    default:
      fprintf(nusmv_stderr, "Compile_CheckWff: unknown type\n");
      nusmv_exit(1);
    }
  } /* wff != Nil */
}

/**Function********************************************************************

  Synopsis           [Parses string looking for a simple/ctl/ltl/compute expression.]

  Description        [Parses string looking for a SIMPLE / CTL / LTL/
  COMPUTE expression. Return <code>0</code> if an expected expression
  has been parsed, <code>1</code> otherwise. The parsed expression is
  stored in <code>pc</code>.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int parsecheckwffcmd(int argc, char **argv, node_ptr * pc)
{
  int result = 0;
  char * inittokens[] =
    {
      "CHECKSIMPWFF ",
      "CHECKCTLWFF ",
      "CHECKLTLWFF ",
      "CHECKCOMPWFF "
    };
  int j = 0;
  int size=sizeof(inittokens)/sizeof(char*);

  /* when the first parsing succesfully terminates we exit from iteration */
  for(j = 0; result == 0 && j < size; j++)
    result = Parser_ReadCmdFromString(argc, argv, inittokens[j], ";\n", pc);
  return(result);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Checks if an atom is boolean or symbolic.]

  Description        [Checks if an atom is boolean or symbolic.]

  SideEffects        []

  SeeAlso            [wff_is_numeric]

******************************************************************************/
static boolean wff_is_boolean(node_ptr l)
{

  while (l != Nil) {
    if (!((car(l) == one_number) || (car(l) == zero_number)))
      return(false);
    l = cdr(l);
  }
  return(true);
}
/**Function********************************************************************

  Synopsis           [Checks if an atom is numeric or symbolic.]

  Description        [Checks if an atom is numeric or symbolic.]

  SideEffects        []

  SeeAlso            [wff_is_boolean]

******************************************************************************/
static boolean wff_is_numeric(node_ptr l)
{
  while (l != Nil) {
    if (car(l) != Nil) {
      if (node_get_type(car(l)) != NUMBER)
  return(false);
    }
    else fprintf(nusmv_stderr, "******** wff_is_numeric: car(l) is Nil.\n");
    l = cdr(l);
  }
  return(true);
}

static wff_result_type check_bool_num_sym(BddEnc_ptr enc, node_ptr l)
{
  if (l == Nil) return(wff_NIL);
  if (node_get_type(l) != VAR) return(check_wff_recur(enc, l, Nil));
  if (node_get_type(l) == VAR && cdr(l) != Nil) {
    l = cdr(l);
  }
  if (wff_is_boolean(l) == true)
    return(wff_BOOLEAN);
  else if (wff_is_numeric(l) == true)
    return(wff_NUMERIC);
  else return(wff_SYMBOLIC);
}

/**Function********************************************************************

  Synopsis           [Performs the recursive step of the
  <code>Compile_CheckWff</code> function.]

  Description        [Performs the recursive step of the
  <code>Compile_CheckWff</code> function.]

  SideEffects        [wfftype_hash]

  SeeAlso            [Compile_CheckWff]

******************************************************************************/

static wff_result_type check_wff_recur(BddEnc_ptr enc, node_ptr wff, 
				       node_ptr context)
{
  if (wff == Nil) return(wff_NIL);
  switch(node_get_type(wff)) {
    /* Base cases */
  case SETIN:
  case BDD:
  case TRUEEXP:
  case FALSEEXP: return(wff_BOOLEAN);
  case NUMBER:
    if (car(wff) == (node_ptr)0 || car(wff) == (node_ptr)1)
      return(wff_BOOLEAN);
    else return(wff_NUMERIC);
  case TWODOTS:  return(wff_NUMERIC);
  case DOT:
  case ARRAY: {
    node_ptr tmp = CompileFlatten_resolve_name(wff, context);

    if (tmp) return(check_definition(enc, tmp));
    else return(wff_ERROR);
  }
  case CONTEXT:
    return(check_wff_recur(enc, cdr(wff), car(wff)));

  case ATOM: {
    node_ptr name  = find_node(DOT, context, find_atom(wff));
    node_ptr par = lookup_param_hash(name);
    node_ptr symb = Encoding_lookup_symbol(BddEnc_get_symbolic_encoding(enc), 
					   name);
    add_ptr const_add = BddEnc_constant_to_add(enc, find_atom(wff));
    wff_result_type atom_type = wff_NIL;

    if (const_add != (add_ptr) NULL) {
      add_free(BddEnc_get_dd_manager(enc), const_add);
      return wff_SYMBOLIC;
    }

    atom_type = lookup_wfftype_hash(name);

    if (atom_type == wff_NIL) {
      if (par != Nil) atom_type = check_wff_recur(enc, par, context);
      if (symb != Nil) atom_type = check_bool_num_sym(enc, symb);
      if (atom_type == wff_NIL) {
	fprintf(nusmv_stderr, "ATOM expr: atom \"");
	print_node(nusmv_stderr, name);
	fprintf(nusmv_stderr, "\" not found\n");
	return wff_ERROR;
      }
      insert_wfftype_hash(name, atom_type);
    }
    return atom_type;
  }

  case IMPLIES:
  case IFF:
  case OR:
  case AND:
    if (check_wff_recur(enc, car(wff), context) != wff_BOOLEAN ||
  check_wff_recur(enc, cdr(wff), context) != wff_BOOLEAN) return(wff_ERROR);
    else return(wff_BOOLEAN);
  case NOT:
    if (check_wff_recur(enc, car(wff), context) != wff_BOOLEAN) return(wff_ERROR);
    else return(wff_BOOLEAN);
  case PLUS:
  case MINUS:
  case TIMES:
  case DIVIDE:
  case MOD:
    if (check_wff_recur(enc, car(wff), context) != wff_NUMERIC ||
  check_wff_recur(enc, cdr(wff), context) != wff_NUMERIC) return(wff_ERROR);
    else return(wff_NUMERIC);
  case EQUAL: return(wff_BOOLEAN);
  case LT:
  case GT:
  case LE:
  case GE:
    if (check_wff_recur(enc, car(wff), context) == wff_NUMERIC ||
  check_wff_recur(enc, cdr(wff), context) == wff_NUMERIC ||
  check_wff_recur(enc, car(wff), context) == wff_BOOLEAN ||
  check_wff_recur(enc, cdr(wff), context) == wff_BOOLEAN) return(wff_BOOLEAN);
    else return(wff_ERROR);
  case UNTIL:
  case RELEASES:
  case SINCE:
  case TRIGGERED:
    if ((check_wff_recur(enc, car(wff), context) != wff_BOOLEAN) ||
  (check_wff_recur(enc, cdr(wff), context) != wff_BOOLEAN)) return(wff_ERROR);
    else return(wff_BOOLEAN);
  case OP_NEXT:
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_ONCE:
  case OP_GLOBAL:
  case OP_HISTORICAL:
  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
    if (check_wff_recur(enc, car(wff), context) != wff_BOOLEAN) return(wff_ERROR);
    else return(wff_BOOLEAN);
  case MINU:
  case MAXU:
  case EU:
  case AU:
    if (check_wff_recur(enc, car(wff), context) != wff_BOOLEAN ||
  check_wff_recur(enc, cdr(wff), context) != wff_BOOLEAN) return(wff_ERROR);
    else return(wff_BOOLEAN);
  case EBF:
  case ABF:
  case EBG:
  case ABG:
  case EBU:
  case ABU:
    if (check_wff_recur(enc, car(wff), context) != wff_BOOLEAN) return(wff_ERROR);
    else return(wff_BOOLEAN);
  case CASE:
    {
      node_ptr ifarg = car(car(wff));
      node_ptr thenarg = cdr(car(wff));
      node_ptr elsearg = cdr(wff);
      if (check_wff_recur(enc, ifarg, context) != wff_BOOLEAN) {
  fprintf(nusmv_stderr, "******** CASE expr: ifarg is not boolean ********\n");
  return(wff_ERROR);
      }
      if ((check_wff_recur(enc, thenarg, context) == wff_BOOLEAN &&
     check_wff_recur(enc, elsearg, context) == wff_BOOLEAN))
        return(wff_BOOLEAN);
      if ((check_wff_recur(enc, thenarg, context) == wff_SYMBOLIC) ||
    (check_wff_recur(enc, elsearg, context) == wff_SYMBOLIC))
        return(wff_SYMBOLIC);
      if ((check_wff_recur(enc, thenarg, context) == wff_BOOLEAN &&
     check_wff_recur(enc, elsearg, context) == wff_NUMERIC) ||
    (check_wff_recur(enc, elsearg, context) == wff_BOOLEAN &&
     check_wff_recur(enc, thenarg, context) == wff_NUMERIC))
        return(wff_NUMERIC);
      fprintf(nusmv_stderr, "******** CASE expr: all the if clauses failed\n");
      return(wff_ERROR);
    }
  case CONS:
  default:
    internal_error("check_wff_recur: type = %d\n", node_get_type(wff));
    return(wff_NIL);
  }
}

/**Function********************************************************************

  Synopsis           [Returns the definition of a symbol.]

  Description        [Given the symbol represented by <code>n</code>, this
  function returns the its definition as a wff result.]

  SideEffects        []

  SeeAlso            [Compile_CheckWff]

******************************************************************************/
static wff_result_type check_definition(BddEnc_ptr enc, node_ptr n)
{
  node_ptr def = Encoding_lookup_symbol(BddEnc_get_symbolic_encoding(enc), n);

  if (def == (node_ptr) NULL) return wff_NIL;
  switch (node_get_type(def)) {
  case VAR: return check_bool_num_sym(enc, def);
  case CONTEXT: return check_wff_recur(enc, def, Nil);
  default:
    internal_error("check_definition: type = %d\n", node_get_type(def));
  }

  return wff_NIL;
}

/**Function********************************************************************

  Synopsis           [Checks whether given  formula contains input
  variables. ]

  Description        [Returns true if the given formula contains input
  variable, false otherwise.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/

boolean Compile_is_input_variable_formula(const Encoding_ptr senc, node_ptr n)
{
  Set_t dep_set =  Formula_GetDependencies(senc, n, Nil);  
  Set_t iter_set;  
  boolean result = false;

  iter_set = dep_set;
  while (Set_IsEmpty(iter_set) == false) {
    node_ptr curr_var = (node_ptr) Set_GetFirst(iter_set);

    if (memberp(curr_var, 
		NodeList_to_node_ptr(Encoding_get_input_vars_list(senc)))) {
      result = true;
    }
    iter_set = Set_GetRest(iter_set);
  }

  Set_ReleaseSet(dep_set);
  return result;
}
