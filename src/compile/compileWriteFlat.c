/**CFile***********************************************************************

  FileName    [compileWriteFlat.c]

  PackageName [compile]

  Synopsis [Creation of an SMV file containing the flattened model.]

  Description [Creation of an SMV file containing the flattened model,
  processes will be removed by explicitly introducing a process
  variable and modifying assignments to take care of inertia.]

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


static char rcsid[] UTIL_UNUSED = "$Id: compileWriteFlat.c,v 1.6.4.7.2.2 2005/07/14 15:58:25 nusmv Exp $";

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
static void print_assign ARGS((FILE *, node_ptr, node_ptr));

static int 
write_flatten_vars ARGS((const Encoding_ptr senc, FILE*, node_ptr lov, char*));

static int write_flatten_define ARGS((const Encoding_ptr senc, FILE*));
static int write_flatten_assign ARGS((const Encoding_ptr senc, FILE*));

static int 
write_flatten_expr ARGS((const Encoding_ptr senc, FILE*, node_ptr, char*));

static int 
write_flatten_expr_pair ARGS((const Encoding_ptr senc, 
			      FILE*, node_ptr, char*));

static int write_process_selector_define ARGS((const Encoding_ptr senc, 
					       FILE*));

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
int Compile_WriteFlatten(const Encoding_ptr senc, FILE* out, cmp_struct_ptr s)
{
  boolean psl_warning_already_printed = false;

  fprintf(out, "-- Flattened model generated from %s\n\n", 
	  get_input_file(options)); 
  fprintf(out, "MODULE main\n");
  write_flatten_vars(senc, out, 
		     NodeList_to_node_ptr(Encoding_get_input_vars_list(senc)), 
		     "IVAR");
  write_flatten_vars(senc, out, 
		     NodeList_to_node_ptr(Encoding_get_state_vars_list(senc)), 
		     "VAR");
  /* write_process_selector_define(senc, out); */
  write_flatten_define(senc, out);
  if (write_flatten_assign(senc, out)) { fprintf(out, "\n"); }
  if (write_flatten_expr(senc, out, cmp_struct_get_init(s), "INIT\n"))
    fprintf(out, "\n");
  if (write_flatten_expr(senc, out, cmp_struct_get_invar(s), "INVAR\n"))
    fprintf(out, "\n");
  if (write_flatten_expr(senc, out, cmp_struct_get_trans(s), "TRANS\n"))
    fprintf(out, "\n");

  if (cmp_struct_get_compassion(s) != Nil) {
    if (write_flatten_expr(senc, out, cmp_struct_get_justice(s), "JUSTICE\n"))
      fprintf(out, "\n\n");
    if (write_flatten_expr_pair(senc, out, cmp_struct_get_compassion(s), 
				"COMPASSION\n"))
      fprintf(out, "\n\n");
  } 
  else { /* For backward compatibility */
    if (write_flatten_expr(senc, out, cmp_struct_get_justice(s), "FAIRNESS\n"))
      fprintf(out, "\n\n");
  }

  if (write_flatten_expr(senc, out, cmp_struct_get_spec(s), "SPEC\n"))
      fprintf(out, "\n\n");
  if (write_flatten_expr(senc, out, cmp_struct_get_compute(s), "COMPUTE\n"))
      fprintf(out, "\n\n");
  if (write_flatten_expr(senc, out, cmp_struct_get_ltlspec(s), "LTLSPEC\n"))
      fprintf(out, "\n\n");
  if (write_flatten_expr(senc, out, cmp_struct_get_invar_spec(s), "INVARSPEC\n"))
      fprintf(out, "\n\n");

  if (cmp_struct_get_pslspec(s) != Nil && !psl_warning_already_printed) {
    fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
    fprintf(nusmv_stderr,  
	    "This version does not support the flattening of PSL properties.\n"
	    "However, for user's convenience all the PSL properties will be dumped\n"
	    "as comments in the output file.\n");
    fprintf(nusmv_stderr, "******** END WARNING ********\n\n");    

    fprintf(out, 
	    "--- Dumping of PSL properties is not supported by this version of the system.\n"\
	    "--- However, the PSL properties had been dumped here for user's convenience,\n"\
	    "--- as the occurred in the original model. \n");
    compile_write_flatten_psl(senc, out, cmp_struct_get_pslspec(s));
    psl_warning_already_printed = true;
  }

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
  else return(0);
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
          if (l != Nil) fprintf(out, ", ");
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

  Synopsis           [Writes DEFINE declarations in SMV format on a
  file.]

  Description        [Writes DEFINE declarations in SMV format on a
  file.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int write_process_selector_define(const Encoding_ptr senc, FILE* out) 
{
  node_ptr n;

  fprintf(out, "DEFINE\n");

  for(n = process_running_symbols; n != Nil; n = cdr(n)){
    node_ptr name = car(n);

    if (Encoding_is_symbol_define(senc, name)) { 
      node_ptr fdef; 
      fdef = Compile_FlattenSexp(senc, 
				 Encoding_lookup_symbol(senc, name), Nil);

      print_node(out, name);
      fprintf(out, " := ");
      print_node(out, fdef);
      fprintf(out, ";\n");
    }
    else {
      fprintf(nusmv_stderr, "write_process_selector_define:\n");
      fprintf(nusmv_stderr, "Undefined symbol: \"");
      print_node(nusmv_stderr, name);
      fprintf(nusmv_stderr, "\"\n");
    }
  }

  fprintf(out, "\n");

  return 1;
}

      

/**Function********************************************************************

  Synopsis           [Writes DEFINE declarations in SMV format on a
  file.]

  Description        [Writes DEFINE declarations in SMV format on a
  file.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int write_flatten_define(const Encoding_ptr senc, FILE* out)
{
  NodeList_ptr names = Encoding_get_defines_list(senc);
  ListIter_ptr iter;

  if (Encoding_get_defines_num(senc) > 0) { fprintf(out, "DEFINE\n"); }

  iter = NodeList_get_first_iter(names);
  while (! ListIter_is_end(iter)) {
    node_ptr name = NodeList_get_elem_at(names, iter);
    node_ptr fdef = 
      Compile_FlattenSexp(senc, Encoding_lookup_symbol(senc, name), Nil);
    
    if (fdef != Nil) {
      print_node(out, name);
      fprintf(out, " := ");
      print_node(out, fdef);
      fprintf(out, ";\n");
    }
    else { 
      fprintf(nusmv_stderr, "write_flatten_define: Flattening failed\n"); 
    }
    
    iter = ListIter_get_next(iter);
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

  if (n == Nil) return(0);
  switch(node_get_type(n)) {
  case CONS:
  case AND:
    res = write_flatten_expr(senc, out, car(n), s);
    res = write_flatten_expr(senc, out, cdr(n), s);
    break;
  default:
    res = 1;
    fprintf(out, "%s ", s);
    if (node_get_type(n) == CONTEXT) {
      /* We are dealing with a property that has not yet been 
	 flattened before */
      node_ptr fn = Compile_FlattenSexp(senc, n, Nil);

      print_node(out, fn);
    } else {
      print_node(out, n);
    }
    fprintf(out, "\n");
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
write_flatten_expr_pair(const Encoding_ptr senc, 
			FILE* out, node_ptr l, char* s)
{
  if (l == Nil) return 0;

  while (l != Nil) {
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

  Synopsis           [Writes flattened ASSIGN declarations in SMV format on a
  file.]

  Description        [Writes flattened ASSIGN declarations in SMV format on a
  file.]

  SideEffects        []

  SeeAlso            [write_flatten_assign_recur]

******************************************************************************/
static int write_flatten_assign(const Encoding_ptr senc, FILE* out)
{
  NodeList_ptr vars = Encoding_get_all_model_vars_list(senc);
  ListIter_ptr iter; 

  iter = NodeList_get_first_iter(vars);
  while (! ListIter_is_end(iter)) {
    node_ptr name = NodeList_get_elem_at(vars, iter);
    node_ptr init_name = find_node(SMALLINIT, name, Nil);
    node_ptr next_name = find_node(NEXT, name, Nil);
    node_ptr invar_expr = lookup_assign_db_hash(name);
    node_ptr init_expr = lookup_assign_db_hash(init_name);
    node_ptr next_expr = lookup_assign_db_hash(next_name);

    if (invar_expr != (node_ptr)NULL) {
      invar_expr = car(invar_expr);
    }
    if (init_expr != (node_ptr)NULL) {
      init_expr = car(init_expr);
    }
    if (next_expr != (node_ptr)NULL) {
      next_expr = car(next_expr);
    }

    if ((init_expr != (node_ptr)NULL) ||
        (next_expr != (node_ptr)NULL) ||
        (invar_expr != (node_ptr)NULL)) {
      fprintf(out, "ASSIGN\n");
    }
    if (init_expr != (node_ptr)NULL) {
      print_assign(out, init_name, init_expr);
    }
    if (invar_expr != (node_ptr)NULL) {
      print_assign(out, name, invar_expr);
    }
    if (next_expr != (node_ptr)NULL) {
      print_assign(out, next_name, next_expr);
    }
    if ((init_expr != (node_ptr)NULL) ||
        (next_expr != (node_ptr)NULL) ||
        (invar_expr != (node_ptr)NULL)) {
      fprintf(out, "\n");
    }
    
    iter = ListIter_get_next(iter);
  }

  return 1;
}

/**Function********************************************************************

  Synopsis           [Prints an assignement statement]

  Description        [[Prints an assignement statement]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void print_assign(FILE * out, node_ptr lhs, node_ptr rhs)
{
  print_node(out, lhs);
  fprintf(out, " := ");
  print_node(out, rhs);
  fprintf(out, ";\n");
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
