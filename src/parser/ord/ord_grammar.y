%{
/**CFile***********************************************************************

  FileName    [grammar.y]

  PackageName [parser.ord]

  Synopsis    [Yacc for variable ordering parser]

  SeeAlso     [input.l]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``parser.ord'' package of NuSMV version 2. 
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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <setjmp.h>

#if HAVE_MALLOC_H
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif  
# include <malloc.h>
#elif HAVE_SYS_MALLOC_H
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif  
# include <sys/malloc.h>
#elif HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include "ParserOrd.h"
#include "ParserOrd_private.h"
#include "ordInt.h"

#include "node/node.h"
#include "utils/error.h"
#include "utils/utils.h"

static char rcsid[] UTIL_UNUSED = "$Id: ";

extern FILE* nusmv_stderr;

   
void parser_ord_error(char *s);

%}

%union {
  node_ptr node;
}

/*
  All of the terminal grammar symbols (tokens recognized by the lexical analyzer) 
  Note: all binary operators associate from left to right,  operators are
        listed from lowest to highest priority 

  Note: The following token are not used inside the grammar, but are
  used by other modules inside the system (i.e. the compiler, mc).
  STEP RESET ASYNC MODTYPE LAMBDA CONTEXT EU AU EBU ABU MINU MAXU
  FORMAT CONSTANT SCALAR CONS OVER BDD ATLINE APROPOS IFTHENELSE
  QUOTE DL_ATOM APATH EPATH BIT
*/

%left LB RB
%left <node> ATOM NUMBER 
%left DOT

/* all nonterminals return a parse tree node */
%type <node> var_id  var_main_id vars_list_item  vars_list


%start begin
%%
begin         : vars_list { }
              ;

/* Repetition of variables */
vars_list     : {}
              | vars_list_item vars_list 
                {
		  parser_ord_add_var(parser_ord_get_global_parser(), $1);
		}
              ;

vars_list_item : var_main_id
                 | var_main_id DOT NUMBER 
                   { 
                     $$ = parser_ord_mk_bit(parser_ord_get_global_parser(), 
					    $1, (int) car($3)); 
                   }
               ;



var_main_id : ATOM 
                   {
		     $$ = parser_ord_mk_dot(parser_ord_get_global_parser(),
					    Nil, $1);
		   }

            | var_main_id LB NUMBER RB 
                   {
		     $$ = parser_ord_mk_array(parser_ord_get_global_parser(), 
					      $1, $3);
                   }
            | var_main_id DOT var_id 
                   {
		     $$ = parser_ord_mk_dot(parser_ord_get_global_parser(), 
					    $1, $3);
		   }		     
            ;


var_id      : ATOM
            | var_id LB NUMBER RB 
                   {
		     $$ = parser_ord_mk_array(parser_ord_get_global_parser(), 
					      $1, $3);
                   }
            | var_id DOT var_id
                   {
		     $$ = parser_ord_mk_dot(parser_ord_get_global_parser(), 
					    $1, $3);
		   }
              ;

%%


/* Additional source code */
void parser_ord_error(char *s) 
{
    extern char parser_ord_text[];
    
    fprintf(nusmv_stderr,"\n");
    if (get_output_order_file(options)) {
      fprintf(nusmv_stderr, "file %s: ", get_output_order_file(options));
    }
    else {
      fprintf(nusmv_stderr, "file stdin: ");
    }

    if (parser_ord_lineno) {
      fprintf(nusmv_stderr, "line %d: ", parser_ord_lineno);
    }

    fprintf(nusmv_stderr, "at token \"%s\": %s\n", parser_ord_text, s);
    if (opt_batch(options)) {
      /* exits the execution */
      fprintf(nusmv_stderr, "\n");
      print_io_atom_stack(nusmv_stderr);
      nusmv_exit(1);
    }
}

int parser_ord_wrap()  { return 1; }


