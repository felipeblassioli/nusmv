/**CFile***********************************************************************

  FileName    [compileWriteBool.c]

  PackageName [compile]

  Synopsis [Creation of an SMV file containing the flattened
  booleanized model.]

  Description [Creation of an SMV file containing the booleanized and
  flattened model, processes will be removed by explicitly introducing
  a process variable and modifying assignments to take care of
  inertia.]

  SeeAlso     []

  Author      [Marco Roveri]

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
#include "parser/symbols.h"
#include "fsm/sexp/SexpFsm.h"

static char rcsid[] UTIL_UNUSED = "$Id: compileWriteBool.c,v 1.6.4.6.2.1 2005/06/27 14:46:38 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int 
write_flatten_vars ARGS((const Encoding_ptr senc, FILE*, node_ptr, char*));

static int 
write_flatten_expr ARGS((const Encoding_ptr senc, FILE*, node_ptr, char*));

static int 
write_flatten_expr_pair ARGS((const Encoding_ptr senc, FILE*, 
			      node_ptr, char*));

static int 
write_flatten_bfexpr ARGS((const Encoding_ptr senc, FILE*, node_ptr, char*));

static int compile_write_flatten_psl 
ARGS((const Encoding_ptr senc, FILE* out, node_ptr n));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Prints the flatten version of an SMV model.]

  Description        [[Prints on the specified file the flatten
  version of an SMV model.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Compile_WriteFlattenBool(const Encoding_ptr senc, FILE* out, 
			     SexpFsm_ptr fsm, cmp_struct_ptr s)
{
  boolean psl_warning_already_printed = false;

  fprintf(out, "-- Flattened Boolean model generated from %s\n\n", 
	  get_input_file(options)); 
  fprintf(out, "MODULE main\n");

  if (write_flatten_expr(senc, out, SexpFsm_get_init(fsm), "INIT\n"))
    fprintf(out, "\n");

  if (write_flatten_expr(senc, out, SexpFsm_get_invar(fsm), "INVAR\n"))
    fprintf(out, "\n");

  if (write_flatten_expr(senc, out, SexpFsm_get_trans(fsm), "TRANS\n"))
    fprintf(out, "\n");

  if (cmp_struct_get_compassion(s) != Nil) {
    if (write_flatten_expr(senc, out, SexpFsm_get_justice(fsm), "JUSTICE\n"))
      fprintf(out, "\n\n");
    if (write_flatten_expr_pair(senc, out, SexpFsm_get_compassion(fsm),
				"COMPASSION\n"))
      fprintf(out, "\n\n");
  } 
  else { /* For backward compatibility */
    if (write_flatten_expr(senc, out, SexpFsm_get_compassion(fsm), "FAIRNESS\n"))
      fprintf(out, "\n\n");
  }

  if (write_flatten_bfexpr(senc, out, cmp_struct_get_spec(s), "SPEC\n"))
      fprintf(out, "\n");

  if (write_flatten_bfexpr(senc, out, cmp_struct_get_compute(s), "COMPUTE\n"))
      fprintf(out, "\n");

  if (write_flatten_bfexpr(senc, out, cmp_struct_get_ltlspec(s), "LTLSPEC\n"))
      fprintf(out, "\n");

  if (write_flatten_bfexpr(senc, out, cmp_struct_get_invar_spec(s), "INVARSPEC\n"))
      fprintf(out, "\n");

  if (cmp_struct_get_pslspec(s) != Nil && !psl_warning_already_printed) {
    fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
    fprintf(nusmv_stderr,  
	    "This version does not support the booleanization of PSL properties.\n"\
	    "However, for user's convenience all the PSL properties will be dumped\n"\
	    "as comments in the output file.\n");
    fprintf(nusmv_stderr, "******** END WARNING ********\n\n");

    fprintf(out, 
	    "--- Dumping of PSL properties is not supported by this version of the system.\n"\
	    "--- However, the PSL properties had been dumped here for user's convenience,\n"\
	    "--- as the occurred in the original model. \n");
    compile_write_flatten_psl(senc, out, cmp_struct_get_pslspec(s));
    psl_warning_already_printed = true;
  }


  write_flatten_vars(senc, out, 
		     NodeList_to_node_ptr(Encoding_get_input_vars_list(senc)), 
		     "IVAR");
  write_flatten_vars(senc, out, 
		     NodeList_to_node_ptr(Encoding_get_state_vars_list(senc)), 
		     "VAR");

  return 1;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Writes VAR declarations in SMV format on a file.]

  Description        [Writes VAR declarations in SMV format on a file.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int 
write_flatten_vars(const Encoding_ptr senc, FILE* out, node_ptr lov, char* str)
{
  if (lov != Nil) fprintf(out, "%s\n", str);
  else return 0;

  while (lov != Nil) {
    node_ptr name = car(lov);
    
    if (Encoding_is_symbol_var(senc, name)) {
      node_ptr range = Encoding_get_var_range(senc, name);
      print_node(out, name);
      if (range == boolean_range) { fprintf(out, " : boolean;\n"); } 
      else {
	node_ptr l = range;
        fprintf(out, " : {");

        while (l != Nil) {
          print_node(out, car(l));
          l = cdr(l);
          if (l != Nil) { fprintf(out, ", "); }
        }
        fprintf(out, "};\n");
      }
    }
    lov = cdr(lov);
  }

  fprintf(out, "\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Writes flattened expression in SMV format on a file.]

  Description        [Writes a generic expression prefixed by a given
  string in SMV format on a file.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int 
write_flatten_expr(const Encoding_ptr senc, FILE* out, node_ptr n, char* s)
{
  int res = 0;

  if (n == Nil) return 0;
  switch(node_get_type(n)) {
  case CONS:
  case AND:
    res = write_flatten_expr(senc, out, car(n), s);
    res = write_flatten_expr(senc, out, cdr(n), s);
    break;
  default:
    res = 1;
    if (n != find_node(TRUEEXP, Nil, Nil)) {
      fprintf(out, "%s", s);
      if (node_get_type(n) == CONTEXT) {
      /* We are dealing with a property that has not yet been
         flattened before */
        node_ptr fn = Compile_FlattenSexp(senc, n, Nil);

        print_node(out, fn);
      } else {
        print_node(out, n);
      }
      fprintf(out, "\n\n");
    }
    break;
  }
  return res;
}

/**Function********************************************************************

  Synopsis           [Writes flattened expression pairs in SMV format on a file.]

  Description        [Writes a list of flattened expression pairs 
  prefixed by a given string in SMV format on a file.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int 
write_flatten_expr_pair(const Encoding_ptr senc, FILE* out, 
			node_ptr l, char* s)
{
  if (l == Nil) return 0;
  while (l) {
    node_ptr n = car(l);
    l = cdr(l);
    nusmv_assert(node_get_type(n) == CONS);
    fprintf(out, "%s (", s);
    if (node_get_type(n) == CONTEXT) {
      node_ptr fn = Compile_FlattenSexp(senc, car(n), Nil);
      print_node(out, fn);
    } else {
      print_node(out, car(n));
    }
    fprintf(out, ", ");
    if (node_get_type(n) == CONTEXT) {
      node_ptr fn = Compile_FlattenSexp(senc, cdr(n), Nil);
      print_node(out, fn);
    } else {
      print_node(out, cdr(n));
    }
    fprintf(out, ")\n");
  }
  return 1;
}

/**Function********************************************************************

  Synopsis           [Writes flattened expression in SMV format on a file.]

  Description        [Writes a generic expression prefixed by a given
  string in SMV format on a file.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int 
write_flatten_bfexpr(const Encoding_ptr senc, FILE* out, node_ptr n, char* s)
{
  int res = 0;

  if (n == Nil) return 0;

  switch (node_get_type(n)) {
  case CONS:
  case AND:
    res = write_flatten_bfexpr(senc, out, car(n), s);
    res = write_flatten_bfexpr(senc, out, cdr(n), s);
    break;

  default:
    res = 1;
    if (n != find_node(TRUEEXP, Nil, Nil)) {
      fprintf(out, "%s", s);
      if (node_get_type(n) == CONTEXT) {
      /* We are dealing with a property that has not yet been
         flattened and booleanized before */
        node_ptr fn = Compile_FlattenSexp(senc, n, Nil);
        node_ptr bfn = expr2bexpr(fn);
        print_node(out, bfn);
      } else { print_node(out, n); }

      fprintf(out, "\n\n");
    }
    break;
  }
  return res;
}


/**Function********************************************************************

  Synopsis           [Writes PSL properties as they are.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int compile_write_flatten_psl(const Encoding_ptr senc, 
				     FILE* out, node_ptr n)
{
  if (n == Nil) return 0;

  switch (node_get_type(n)) {
  case CONS:
  case AND:
    compile_write_flatten_psl(senc, out, car(n));
    compile_write_flatten_psl(senc, out, cdr(n));
    break;
    
  default:
    fprintf(out, "-- PSLSPEC\n--   ");
    PslNode_print(out, n);
    fprintf(out, "\n\n");
  } /* switch */

  return 1;
}

