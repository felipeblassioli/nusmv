%{
/**CFile***********************************************************************

  FileName    [grammar.y]

  PackageName [parser]

  Synopsis    [Yacc for NuSMV input language parser]

  SeeAlso     [input.l]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``parser'' package of NuSMV version 2. 
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

  For more information on NuSMV see <http://nusmv.irst.itc.it>
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

#include <limits.h>

#include "parserInt.h"
#include "utils/utils.h"
#include "utils/ustring.h"
#include "node/node.h"
#include "opt/opt.h"
#include "utils/error.h"

/* [FB] For parse tree labelling */
#include "symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: grammar.y,v 1.19.4.10.2.3 2005/07/07 16:45:28 nusmv Exp $";


#define YYMAXDEPTH INT_MAX


node_ptr parse_tree;
node_ptr parse_tree_int;
int parse_command_flag;

extern FILE * nusmv_stderr;
    
void yyerror ARGS((char *s));
static node_ptr context2maincontext ARGS((node_ptr context));
static int nusmv_parse_psl ARGS((void));

%}
/*
  The number of conflicts: shift/reduce expected.
  If more, than a warning message is printed out.
*/
%expect 1

%union {
  node_ptr node;
}

/*
  All of the terminal grammar symbols (tokens recognized by the lexical analyzer) 
  Note: all binary operators associate from left to right,  operators are
        listed from lowest to highest priority 

  Note: The following token are not used inside the grammar, but are
  used by other modules inside the system (i.e. the compiler, mc).
  TOK_STEP TOK_RESET TOK_ASYNC TOK_MODTYPE TOK_LAMBDA TOK_CONTEXT TOK_EU TOK_AU TOK_EBU TOK_ABU TOK_MINU TOK_MAXU
  TOK_FORMAT TOK_CONSTANT TOK_SCALAR TOK_CONS TOK_OVER TOK_BDD TOK_ATLINE TOK_APROPOS TOK_IFTHENELSE
  TOK_QUOTE TOK_DL_ATOM TOK_APATH TOK_EPATH TOK_BIT
*/

%left TOK_GOTO TOK_LET TOK_STEP TOK_EVAL TOK_RESET TOK_CONSTRAINT
%left TOK_ASYNC TOK_MODULE TOK_PROCESS TOK_MODTYPE TOK_LAMBDA TOK_CONTEXT TOK_EU TOK_AU TOK_EBU TOK_ABU TOK_MINU TOK_MAXU
%left TOK_VAR TOK_IVAR TOK_DEFINE TOK_INIT TOK_TRANS TOK_INVAR TOK_FORMAT TOK_SPEC TOK_LTLSPEC TOK_COMPUTE 
%left TOK_PSLSPEC
%left TOK_INVARSPEC TOK_FAIRNESS TOK_COMPASSION TOK_JUSTICE
%left TOK_ISA TOK_CONSTANT TOK_ASSIGN TOK_INPUT TOK_OUTPUT TOK_IMPLEMENTS
%left TOK_BOOLEAN TOK_ARRAY TOK_OF TOK_SCALAR TOK_CONS TOK_OVER TOK_BDD
%left TOK_SEMI TOK_LP TOK_RP TOK_LB TOK_RB TOK_LCB TOK_RCB TOK_LLCB TOK_RRCB
%left TOK_EQDEF TOK_TWODOTS TOK_ATLINE
%left <node> TOK_FALSEEXP TOK_TRUEEXP
%left TOK_APROPOS TOK_SELF TOK_SIGMA
%left TOK_CASE TOK_ESAC TOK_COLON
%left TOK_IF TOK_THEN TOK_ELSE TOK_IFTHENELSE
%left TOK_INCONTEXT TOK_SIMPWFF TOK_LTLWFF TOK_CTLWFF TOK_COMPWFF

%left <node> TOK_ATOM
%left <node> TOK_NUMBER
%left <node> TOK_QUOTE
%left <node> TOK_DL_ATOM

%left  TOK_COMMA
%right TOK_IMPLIES
%left  TOK_IFF
%left  TOK_OR TOK_XOR TOK_XNOR
%left  TOK_AND
%left  TOK_NOT
%left  TOK_EX TOK_AX TOK_EF TOK_AF TOK_EG TOK_AG TOK_EE TOK_AA TOK_SINCE TOK_UNTIL TOK_TRIGGERED TOK_RELEASES TOK_EBF TOK_EBG TOK_ABF TOK_ABG TOK_BUNTIL TOK_MMIN TOK_MMAX
%left  TOK_OP_NEXT TOK_OP_GLOBAL TOK_OP_FUTURE
%left  TOK_OP_PREC TOK_OP_NOTPRECNOT TOK_OP_HISTORICAL TOK_OP_ONCE
%left  TOK_APATH TOK_EPATH
%left  TOK_EQUAL TOK_NOTEQUAL TOK_LT TOK_GT TOK_LE TOK_GE
%left  TOK_UNION
%left  TOK_SETIN
%left  TOK_MOD
%left  TOK_PLUS TOK_MINUS
%left  TOK_TIMES TOK_DIVIDE
%left  TOK_UMINUS		/* supplies precedence for unary minus */
%left  TOK_NEXT TOK_SMALLINIT
%left  TOK_DOT
%left  TOK_BIT

/* all nonterminals return a parse tree node */
%type <node> module_list module declarations declaration var input_var var_list
%type <node> ivar_list type itype isa init trans invar define define_list spec
%type <node> ltlspec pslspec compute fairness justice compassion invarspec 
%type <node> atom_list decl_var_id var_id subrange number simple_expr 
%type <node> assign_expr init_expr next_expr justice_expr compassion_expr 
%type <node> fair_expr invar_expr trans_expr ctl_expr compute_expr s_case_list 
%type <node> n_case_list constant ltl_expr ltl_orexpr ltl_andexpr define_expr
%type <node> ltl_binary_expr ltl_atomexpr ltl_case_expr ltl_simple_expr 
%type <node> module_sign module_type simple_expr_list assign assign_list
%type <node> assign_type simple_expr_set next_expr_set ctl_expr_set 
%type <node> ltl_expr_set input output implements term_list
%type <node> constant_list actual_params command command_case trace state
%type <node> context ctl_case_list

/* %type <node> trace state */

/* currently not used: SEE GRAMMAR RULES 
%type next_expr_list
*/

%start begin
%%
begin         : { 
                  if(parse_command_flag) {
		    yyerror("MODULE definition when command expected");
		    YYERROR;
		  }
                } 
                module_list {parse_tree = $2;}
              | { 
                  if(! parse_command_flag) {
		    yyerror("command found in an SMV file");
		    YYERROR;
		  }
                }
                command {parse_tree_int = $2;}
              ;

/*
 An NuSMV program is a repetition of modules.
 Each module has a signature and a body.
*/
module_list   : module {$$ = cons($1, Nil);}
              | module_list module {$$ = cons($2, $1);}
              ;
module        : TOK_MODULE module_sign declarations {$$ = new_node(MODULE, $2, $3);}
              ;
module_sign   : TOK_ATOM {$$ = new_node(MODTYPE, $1, Nil);}
              | TOK_ATOM TOK_LP TOK_RP {$$ = new_node(MODTYPE, $1, Nil);}
              | TOK_ATOM TOK_LP atom_list TOK_RP {$$ = new_node(MODTYPE, $1, $3);}
              ;
atom_list     : TOK_ATOM {$$ = cons(find_atom($1), Nil);}
              | atom_list TOK_COMMA TOK_ATOM {$$ = cons(find_atom($3), $1);}
              ;


/* The body of a module */
declarations  : {$$ = Nil;}
              | declarations declaration {$$ = cons($2, $1);}
              ;
declaration   : isa
              | var
              | input_var
              | assign 
              | init
              | invar
              | trans
              | define
              | fairness
              | justice
              | compassion
              | invarspec
              | spec
              | ltlspec
              | pslspec
              | compute
              | implements
              | input
              | output
              ;

/* Module macro-expansion */
isa           : TOK_ISA TOK_ATOM {$$ = new_node(ISA, $2, Nil);}
              ;

/* parse an optional semicolon */
optsemi      : | TOK_SEMI;

/*
 Variable declarations:
 This includes also the instantiation of module
 (in synchronous and asynchronous product).
*/
var           : TOK_VAR var_list {$$ = new_node(VAR, $2, Nil);}
              ;
input_var     : TOK_IVAR ivar_list {$$ = new_node(IVAR, $2, Nil);}
              ;
var_list      : {$$ = Nil;}
              | var_list decl_var_id TOK_COLON type TOK_SEMI {$$ = cons(new_node(COLON, $2, $4), $1);}
              ;
ivar_list     : {$$ = Nil;}
              | ivar_list decl_var_id TOK_COLON itype TOK_SEMI {$$ = cons(new_node(COLON, $2, $4), $1);}
              ;
type          : TOK_BOOLEAN {$$ = new_node(BOOLEAN, Nil, Nil);}
              | subrange
              | TOK_LCB constant_list TOK_RCB {$$ = new_node(SCALAR, $2, Nil);}
              | TOK_ARRAY subrange TOK_OF type {$$ = new_node(ARRAY, $2, $4);}
              | module_type
              | TOK_PROCESS module_type {$$ = new_node(PROCESS, $2, Nil);}
              ;
itype         : TOK_BOOLEAN {$$ = new_node(BOOLEAN, Nil, Nil);}
              | subrange
              | TOK_LCB constant_list TOK_RCB {$$ = new_node(SCALAR, $2, Nil);}
              | TOK_ARRAY subrange TOK_OF itype {$$ = new_node(ARRAY, $2, $4);}
              ;
constant_list : constant {$$ = cons(find_atom($1), Nil);}
              | constant_list TOK_COMMA constant {$$ = cons(find_atom($3), $1);}
              ;
constant      : TOK_ATOM
              | number
	      | TOK_FALSEEXP
	      | TOK_TRUEEXP
              ;
subrange      : number TOK_TWODOTS number {$$ = new_node(TWODOTS, $1, $3);}
              ;
number        : TOK_NUMBER
	      | TOK_PLUS TOK_NUMBER %prec TOK_UMINUS { $$ = $2; }
	      | TOK_MINUS TOK_NUMBER %prec TOK_UMINUS {node_int_setcar($2, -(node_get_int($2))); $$ = $2;}
              ;
module_type   : TOK_ATOM {$$ = new_node(MODTYPE, $1, Nil);}
              | TOK_ATOM TOK_LP TOK_RP {$$ = new_node(MODTYPE, $1, Nil);}
              | TOK_ATOM TOK_LP actual_params TOK_RP {$$ = new_node(MODTYPE, $1, $3);}
              ;
actual_params : simple_expr_list
              ;

/* Assignments of initial, current or next value of variables */
assign        : TOK_ASSIGN assign_list {$$ = new_node(ASSIGN, $2, Nil);}
              ;
assign_list   : {$$ = Nil;}
              | assign_list assign_type {$$ = new_node(AND, $1, $2);}
              ;
assign_type   : var_id TOK_EQDEF simple_expr TOK_SEMI
                 {$$ = new_node(EQDEF, $1, $3);} 
              | TOK_SMALLINIT TOK_LP var_id TOK_RP TOK_EQDEF simple_expr TOK_SEMI
                { $$ = new_node(EQDEF, new_node(SMALLINIT, $3, Nil), $6);}
              | TOK_NEXT TOK_LP var_id TOK_RP TOK_EQDEF assign_expr TOK_SEMI
                { $$ = new_node(EQDEF, new_node(NEXT, $3, Nil), $6);}
              ;
assign_expr   : next_expr
              ;

context       : TOK_ATOM
              | context TOK_DOT TOK_ATOM         { $$ = find_node(DOT, $1, $3); }
              | context TOK_LB TOK_NUMBER TOK_RB { $$ = find_node(ARRAY, $1, $3); }
              ;    

/* Direct finite state machine definition (init, invar, trans) */
init          : TOK_INIT init_expr optsemi {$$ = new_node(INIT, $2, Nil);}
              ;
init_expr     : simple_expr
              ;
invar         : TOK_INVAR invar_expr optsemi {$$ = new_node(INVAR, $2, Nil);}
              ;
invar_expr    : simple_expr
              ;
trans         : TOK_TRANS trans_expr optsemi {$$ = new_node(TRANS, $2, Nil);}
              ;
trans_expr    : next_expr
              ;

/* Definitions */
define        : TOK_DEFINE define_list {$$ = new_node(DEFINE, $2, Nil);}
              ;
define_list   : {$$ = Nil;}
              | define_list decl_var_id TOK_EQDEF define_expr TOK_SEMI {$$ = cons(new_node(EQDEF, $2, $4), $1);}
              ;
define_expr   : simple_expr
              ;

/* Fairness declarations */
fairness      : TOK_FAIRNESS fair_expr optsemi {$$ = new_node(JUSTICE, $2, Nil);}
              ;
fair_expr     : simple_expr
              ;

justice       : TOK_JUSTICE  justice_expr optsemi {$$ = new_node(JUSTICE, $2, Nil);}
              ;
justice_expr  : simple_expr
              ;

compassion    : TOK_COMPASSION compassion_expr optsemi {$$ = new_node(COMPASSION, $2, Nil);}
              ;
compassion_expr: TOK_LP simple_expr TOK_COMMA simple_expr TOK_RP {$$ = cons($2,$4);} 
               ;

/* Specifications and computation of min and max distance */
spec          : TOK_SPEC ctl_expr optsemi {$$ = new_node(SPEC, $2, Nil);}
              ;

ltlspec       : TOK_LTLSPEC ltl_expr optsemi {$$ = new_node(LTLSPEC, $2, Nil);}
              ;

pslspec       : TOK_PSLSPEC 
{ 
  if (nusmv_parse_psl() != 0) {
    YYABORT;
  }
  $$ = new_node(PSLSPEC, psl_parsed_tree, Nil);
}
;

invarspec     : TOK_INVARSPEC invar_expr optsemi {$$ = new_node(INVARSPEC, $2, Nil);}
              ;
 
compute       : TOK_COMPUTE compute_expr optsemi {$$ = new_node(COMPUTE, $2, Nil);}
              ;
compute_expr  : TOK_MMIN TOK_LB ctl_expr TOK_COMMA ctl_expr TOK_RB { $$ = new_node(MINU, $3, $5); }
              | TOK_MMAX TOK_LB ctl_expr TOK_COMMA ctl_expr TOK_RB { $$ = new_node(MAXU, $3, $5); }
              ;

/* Variable identifiers.
   decl_var_id is used for declarations; self not allowed.
   var_id is used to reference variables, includes self.
 */

decl_var_id   : TOK_ATOM
              | decl_var_id TOK_DOT TOK_ATOM {$$ = new_node(DOT, $1, $3);}
              | decl_var_id TOK_DOT TOK_NUMBER {$$ = new_node(DOT, $1, $3);}
              | decl_var_id TOK_LB TOK_NUMBER TOK_RB {$$ = new_node(ARRAY, $1, $3);}
              ;
var_id        : TOK_ATOM
              | TOK_SELF {$$ = new_node(SELF,Nil,Nil);}
              | var_id TOK_DOT TOK_ATOM {$$ = new_node(DOT, $1, $3);}
              | var_id TOK_DOT TOK_NUMBER {$$ = new_node(DOT, $1, $3);}
              | var_id TOK_LB simple_expr TOK_RB {$$ = new_node(ARRAY, $1, $3);}
              ;


/* Simple expressions. Do not involve next state variables or CLT. */

simple_expr_list : simple_expr {$$ = cons($1,Nil);}
	         | simple_expr_list TOK_COMMA simple_expr {$$ = cons($3, $1);}
                 ;
simple_expr   : var_id
              | number
              | subrange
              | TOK_FALSEEXP
              | TOK_TRUEEXP
              | TOK_LP simple_expr TOK_RP { $$ = $2; }
	      | simple_expr TOK_IMPLIES simple_expr { $$ = new_node(IMPLIES, $1, $3); }
	      | simple_expr TOK_IFF simple_expr { $$ = new_node(IFF, $1, $3); }
	      | simple_expr TOK_OR simple_expr { $$ = new_node(OR,$1, $3); }
              | simple_expr TOK_XOR simple_expr { $$ = new_node(XOR,$1, $3); }
              | simple_expr TOK_XNOR simple_expr { $$ = new_node(XNOR,$1, $3); }
	      | simple_expr TOK_AND simple_expr { $$ = new_node(AND, $1, $3); }
	      | TOK_NOT simple_expr { $$ = new_node(NOT, $2, Nil); }
              | simple_expr TOK_PLUS simple_expr { $$ = new_node(PLUS, $1, $3); }
              | simple_expr TOK_MINUS simple_expr { $$ = new_node(MINUS, $1, $3); }
              | simple_expr TOK_TIMES simple_expr { $$ = new_node(TIMES, $1, $3); }
              | simple_expr TOK_DIVIDE simple_expr { $$ = new_node(DIVIDE, $1, $3); }
              | simple_expr TOK_MOD simple_expr { $$ = new_node(MOD, $1, $3); }
              | simple_expr TOK_EQUAL simple_expr { $$ = new_node(EQUAL, $1, $3); }
              | simple_expr TOK_NOTEQUAL simple_expr { $$ = new_node(NOTEQUAL, $1, $3); }
              | simple_expr TOK_LT simple_expr { $$ = new_node(LT, $1, $3); }
              | simple_expr TOK_GT simple_expr { $$ = new_node(GT, $1, $3); }
              | simple_expr TOK_LE simple_expr { $$ = new_node(LE, $1, $3); }
              | simple_expr TOK_GE simple_expr { $$ = new_node(GE, $1, $3); }
              | TOK_LCB simple_expr_set TOK_RCB { $$ = $2; } 
              | simple_expr TOK_UNION simple_expr { $$ = new_node(UNION, $1, $3); }
              | simple_expr TOK_SETIN simple_expr { $$ = new_node(SETIN, $1, $3); }
              | TOK_CASE s_case_list TOK_ESAC { $$ = $2; }
              | TOK_SIGMA TOK_LB TOK_ATOM TOK_EQUAL subrange TOK_RB simple_expr
                { $$ = new_node(SIGMA, new_node(EQUAL, $3, $5), $7); }
	      ;
simple_expr_set : simple_expr {$$ = $1;}
                | simple_expr_set TOK_COMMA simple_expr {$$ = new_node(UNION, $1, $3);}
                ;
s_case_list   : {$$=new_node(TRUEEXP, Nil, Nil);}
              | simple_expr TOK_COLON simple_expr TOK_SEMI s_case_list
                {
	          $$ = new_node(CASE, new_node(COLON, $1, $3), $5);
	        }
              ;



/* Next expressions. Contain next state variables, but no CTL. */
next_expr     : var_id
              | number
              | subrange
              | TOK_FALSEEXP
              | TOK_TRUEEXP
/*              | TOK_NEXT TOK_LP var_id TOK_RP { $$ = new_node(NEXT, $3, Nil); } */
/* We dont want expression of this kind: next(alpha & next(beta))   */
              | TOK_NEXT TOK_LP simple_expr TOK_RP { $$ = new_node(NEXT, $3, Nil); } 
              | TOK_LP next_expr TOK_RP { $$ = $2; }
	      | next_expr TOK_IMPLIES next_expr { $$ = new_node(IMPLIES, $1, $3); }
	      | next_expr TOK_IFF next_expr { $$ = new_node(IFF, $1, $3); }
	      | next_expr TOK_OR next_expr { $$ = new_node(OR, $1, $3); }
	      | next_expr TOK_XOR next_expr { $$ = new_node(XOR, $1, $3); }
	      | next_expr TOK_XNOR next_expr { $$ = new_node(XNOR, $1, $3); }
	      | next_expr TOK_AND next_expr { $$ = new_node(AND, $1, $3); }
	      | TOK_NOT next_expr { $$ = new_node(NOT, $2, Nil); }
              | next_expr TOK_PLUS next_expr { $$ = new_node(PLUS, $1, $3); }
              | next_expr TOK_MINUS next_expr { $$ = new_node(MINUS, $1, $3); }
              | next_expr TOK_TIMES next_expr { $$ = new_node(TIMES, $1, $3); }
              | next_expr TOK_DIVIDE next_expr { $$ = new_node(DIVIDE, $1, $3); }
              | next_expr TOK_MOD next_expr { $$ = new_node(MOD, $1, $3); }
              | next_expr TOK_EQUAL next_expr { $$ = new_node(EQUAL, $1, $3); }
              | next_expr TOK_NOTEQUAL next_expr { $$ = new_node(NOTEQUAL, $1, $3); }
              | next_expr TOK_LT next_expr { $$ = new_node(LT, $1, $3); }
              | next_expr TOK_GT next_expr { $$ = new_node(GT, $1, $3); }
              | next_expr TOK_LE next_expr { $$ = new_node(LE, $1, $3); }
              | next_expr TOK_GE next_expr { $$ = new_node(GE, $1, $3); }
              | TOK_LCB next_expr_set TOK_RCB { $$ = $2; } 
              | next_expr TOK_UNION next_expr { $$ = new_node(UNION, $1, $3); }
              | next_expr TOK_SETIN next_expr { $$ = new_node(SETIN, $1, $3); }
              | TOK_CASE n_case_list TOK_ESAC { $$ = $2; }
              | TOK_SIGMA TOK_LB TOK_ATOM TOK_EQUAL subrange TOK_RB next_expr
                { $$ = new_node(SIGMA, new_node(EQUAL, $3, $5), $7); }
              ;
next_expr_set : next_expr {$$ = $1;}
              | next_expr_set TOK_COMMA next_expr {$$ = new_node(UNION, $1, $3);}
              ;
n_case_list   : {$$=new_node(TRUEEXP, Nil, Nil);}
              | next_expr TOK_COLON next_expr TOK_SEMI n_case_list
                {
	          $$ = new_node(CASE, new_node(COLON, $1, $3), $5);
	        }
              ;

/* currently not used 
next_expr_list : next_expr {$$ = cons($1,Nil);}
	         | next_expr_list TOK_COMMA next_expr {$$ = cons($3, $1);}
                 ;
*/

/* (Bounded) CTL formulas. Contain CTL operators, but no next variables. */

ctl_expr      : var_id
              | number
              | subrange
              | TOK_FALSEEXP
              | TOK_TRUEEXP
              | TOK_LP ctl_expr TOK_RP { $$ = $2; }
	      | ctl_expr TOK_IMPLIES ctl_expr { $$ = new_node(IMPLIES, $1, $3); }
	      | ctl_expr TOK_IFF ctl_expr { $$ = new_node(IFF, $1, $3); }
	      | ctl_expr TOK_OR ctl_expr { $$ = new_node(OR, $1, $3); }
	      | ctl_expr TOK_XOR ctl_expr { $$ = new_node(XOR, $1, $3); }
	      | ctl_expr TOK_XNOR ctl_expr { $$ = new_node(XNOR, $1, $3); }
	      | ctl_expr TOK_AND ctl_expr { $$ = new_node(AND, $1, $3); }
	      | TOK_NOT ctl_expr { $$ = new_node(NOT, $2, Nil); }
              | ctl_expr TOK_PLUS ctl_expr { $$ = new_node(PLUS, $1, $3); }
              | ctl_expr TOK_MINUS ctl_expr { $$ = new_node(MINUS, $1, $3); }
              | ctl_expr TOK_TIMES ctl_expr { $$ = new_node(TIMES, $1, $3); }
              | ctl_expr TOK_DIVIDE ctl_expr { $$ = new_node(DIVIDE, $1, $3); }
              | ctl_expr TOK_MOD ctl_expr { $$ = new_node(MOD, $1, $3); }
              | ctl_expr TOK_EQUAL ctl_expr { $$ = new_node(EQUAL, $1, $3); }
              | ctl_expr TOK_NOTEQUAL ctl_expr { $$ = new_node(NOTEQUAL, $1, $3); }
              | ctl_expr TOK_LT ctl_expr { $$ = new_node(LT, $1, $3); }
              | ctl_expr TOK_GT ctl_expr { $$ = new_node(GT, $1, $3); }
              | ctl_expr TOK_LE ctl_expr { $$ = new_node(LE, $1, $3); }
              | ctl_expr TOK_GE ctl_expr { $$ = new_node(GE, $1, $3); }
              | TOK_LCB ctl_expr_set TOK_RCB { $$ = $2; } 
              | ctl_expr TOK_UNION ctl_expr { $$ = new_node(UNION, $1, $3); }
              | ctl_expr TOK_SETIN ctl_expr { $$ = new_node(SETIN, $1, $3); }
              | TOK_CASE ctl_case_list TOK_ESAC { $$ = $2; }
/*            | TOK_SIGMA TOK_LB TOK_ATOM TOK_EQUAL subrange TOK_RB ctl_expr */
/*                { $$ = new_node(SIGMA, new_node(EQUAL, $3, $5), $7); } */
              | TOK_EX ctl_expr { $$ = new_node(EX, $2, Nil); }
              | TOK_AX ctl_expr { $$ = new_node(AX, $2, Nil); }
              | TOK_EF ctl_expr { $$ = new_node(EF, $2, Nil); }
              | TOK_AF ctl_expr { $$ = new_node(AF, $2, Nil); }
              | TOK_EG ctl_expr { $$ = new_node(EG, $2, Nil); }
              | TOK_AG ctl_expr { $$ = new_node(AG, $2, Nil); }
	      | TOK_AA TOK_LB ctl_expr TOK_UNTIL ctl_expr TOK_RB { $$ = new_node(AU, $3, $5); }
	      | TOK_EE TOK_LB ctl_expr TOK_UNTIL ctl_expr TOK_RB { $$ = new_node(EU, $3, $5); }
	      | TOK_AA TOK_LB ctl_expr TOK_BUNTIL subrange ctl_expr TOK_RB
                       { $$ = new_node(ABU, new_node(AU, $3, $6), $5); }
	      | TOK_EE TOK_LB ctl_expr TOK_BUNTIL subrange ctl_expr TOK_RB
                       { $$ = new_node(EBU, new_node(EU, $3, $6), $5); }
              | TOK_EBF subrange ctl_expr { $$ = new_node(EBF, $3, $2); }
              | TOK_ABF subrange ctl_expr { $$ = new_node(ABF, $3, $2); }
              | TOK_EBG subrange ctl_expr { $$ = new_node(EBG, $3, $2); }
              | TOK_ABG subrange ctl_expr { $$ = new_node(ABG, $3, $2); }
	      ;
ctl_expr_set  : ctl_expr {$$ = $1;}
              | ctl_expr_set TOK_COMMA ctl_expr {$$ = new_node(UNION, $1, $3);}
              ;
ctl_case_list : {$$=new_node(TRUEEXP, Nil, Nil);}
              | ctl_expr TOK_COLON ctl_expr TOK_SEMI ctl_case_list
                {
	          $$ = new_node(CASE, new_node(COLON, $1, $3), $5);
	        }
              ;

/* TOK_LTL grammar */
ltl_expr      : ltl_orexpr {$$ = $1;}
              | ltl_orexpr TOK_IMPLIES ltl_expr {$$ = new_node(IMPLIES, $1, $3);}
              | ltl_orexpr TOK_IFF ltl_orexpr {$$ = new_node(IFF, $1, $3);}
              | ltl_orexpr TOK_XOR ltl_orexpr {$$ = new_node(XOR, $1, $3);}
              | ltl_orexpr TOK_XNOR ltl_orexpr {$$ = new_node(XNOR, $1, $3);}
              ;
ltl_orexpr    : ltl_andexpr {$$ = $1;}
              | ltl_orexpr TOK_OR ltl_andexpr {$$ = new_node(OR, $1, $3);}
              ;
ltl_andexpr   : ltl_binary_expr {$$ = $1;}
              | ltl_andexpr TOK_AND ltl_binary_expr {$$ = new_node(AND, $1, $3);}
              ;
ltl_binary_expr : ltl_atomexpr {$$ = $1;}
              | ltl_binary_expr TOK_UNTIL ltl_atomexpr {$$ = new_node(UNTIL, $1, $3);}
              | ltl_binary_expr TOK_SINCE ltl_atomexpr {$$ = new_node(SINCE, $1, $3);}
              | ltl_binary_expr TOK_RELEASES ltl_atomexpr 
                {$$ = new_node(NOT, new_node(UNTIL, new_node(NOT, $1, Nil), 
                new_node(NOT, $3, Nil)), Nil);}
              | ltl_binary_expr TOK_TRIGGERED ltl_atomexpr 
                {$$ = new_node(NOT, new_node(SINCE, new_node(NOT, $1, Nil), 
                new_node(NOT, $3, Nil)), Nil);}
              ;
ltl_atomexpr  : TOK_NOT ltl_atomexpr {$$ = new_node(NOT, $2, Nil);}
              | TOK_OP_NEXT       ltl_atomexpr {$$ = new_node(OP_NEXT, $2, Nil);} 
              | TOK_OP_PREC       ltl_atomexpr {$$ = new_node(OP_PREC, $2, Nil);} 
              | TOK_OP_NOTPRECNOT ltl_atomexpr {$$ = new_node(OP_NOTPRECNOT, $2, Nil);}               
              | TOK_OP_GLOBAL     ltl_atomexpr {$$ = new_node(OP_GLOBAL, $2, Nil);}
              | TOK_OP_HISTORICAL ltl_atomexpr {$$ = new_node(OP_HISTORICAL, $2, Nil);}
              | TOK_OP_FUTURE     ltl_atomexpr {$$ = new_node(OP_FUTURE, $2, Nil);}
              | TOK_OP_ONCE       ltl_atomexpr {$$ = new_node(OP_ONCE, $2, Nil);}
              | TOK_LP ltl_expr TOK_RP {$$ = $2;}
              | TOK_CASE ltl_case_expr TOK_ESAC {$$ = $2;}
              | ltl_simple_expr
              ;

ltl_case_expr : {$$=new_node(TRUEEXP, Nil, Nil);}
              | ltl_expr TOK_COLON ltl_expr TOK_SEMI ltl_case_expr
                {
	          $$ = new_node(CASE, new_node(COLON, $1, $3), $5);
	        }
              ;

ltl_simple_expr : var_id
                | number
                | subrange
                | TOK_FALSEEXP
                | TOK_TRUEEXP
                | TOK_LP ltl_simple_expr TOK_RP {$$ = $2;}
                | ltl_simple_expr TOK_PLUS ltl_simple_expr {$$ = new_node(PLUS, $1, $3);}
                | ltl_simple_expr TOK_MINUS ltl_simple_expr {$$ = new_node(MINUS, $1, $3);}
                | ltl_simple_expr TOK_TIMES ltl_simple_expr {$$ = new_node(TIMES, $1, $3);}
                | ltl_simple_expr TOK_DIVIDE ltl_simple_expr {$$ = new_node(DIVIDE, $1, $3);}
                | ltl_simple_expr TOK_MOD ltl_simple_expr {$$ = new_node(MOD, $1, $3);}
                | ltl_simple_expr TOK_EQUAL ltl_simple_expr {$$ = new_node(EQUAL, $1, $3);}
                | ltl_simple_expr TOK_NOTEQUAL ltl_simple_expr {$$ = new_node(NOTEQUAL, $1, $3);}
                | ltl_simple_expr TOK_LT ltl_simple_expr {$$ = new_node(LT, $1, $3);}
                | ltl_simple_expr TOK_GT ltl_simple_expr {$$ = new_node(GT, $1, $3);}
                | ltl_simple_expr TOK_LE ltl_simple_expr {$$ = new_node(LE, $1, $3);}
                | ltl_simple_expr TOK_GE ltl_simple_expr {$$ = new_node(GE, $1, $3);}
                | TOK_LCB ltl_expr_set TOK_RCB { $$ = $2; } 
                | ltl_simple_expr TOK_UNION ltl_simple_expr { $$ = new_node(UNION, $1, $3);}
                | ltl_simple_expr TOK_SETIN ltl_simple_expr { $$ = new_node(SETIN, $1, $3);}
                ;
ltl_expr_set    : ltl_expr {$$ = $1;}
                | ltl_expr_set TOK_COMMA ltl_expr {$$ = new_node(UNION, $1, $3);}
                ;
/*
   Only for homomorphism checking (currently disabled).
 */
implements    : TOK_IMPLEMENTS TOK_ATOM {$$ = new_node(IMPLEMENTS, $2, Nil);}
              ;
input         : TOK_INPUT var_list {$$ = new_node(INPUT, $2, Nil);}
              ;
output        : TOK_OUTPUT term_list TOK_SEMI {$$ = new_node(OUTPUT, $2, Nil);}
              ;
term_list     : var_id {$$ = cons($1, Nil);}
              | term_list TOK_COMMA var_id {$$ = cons($3, $1);}
              ;

command       : command_case {$$ = $1;}
              | error TOK_SEMI {return(1);}
              | error {return(1);}
              ;

command_case  : TOK_GOTO state TOK_SEMI
                 {parse_tree_int = new_node(GOTO, $2, Nil); return(0);}
              | TOK_LET var_id TOK_EQDEF simple_expr TOK_SEMI
                 {parse_tree_int = new_node(LET, new_node(EQDEF, $2, $4), Nil); return(0);}
              | TOK_EVAL ctl_expr TOK_SEMI
                 {parse_tree_int = new_node(EVAL, $2, Nil); return(0);}
              | TOK_INIT init_expr TOK_SEMI
                 {parse_tree_int = new_node(INIT, $2, Nil); return(0);}
              | TOK_FAIRNESS fair_expr TOK_SEMI
                 {parse_tree_int = new_node(FAIRNESS, $2, Nil); return(0);}
              | TOK_TRANS trans_expr TOK_SEMI
                 {parse_tree_int = new_node(TRANS, $2, Nil); return(0);}
              | TOK_CONSTRAINT simple_expr TOK_SEMI
                 {parse_tree_int = new_node(CONSTRAINT, $2, Nil); return(0);}
              | TOK_SIMPWFF simple_expr TOK_SEMI
                 {parse_tree_int =
                    new_node(SIMPWFF,
                             new_node(CONTEXT, Nil, $2), Nil); return(0);}
              | TOK_SIMPWFF simple_expr TOK_INCONTEXT context TOK_SEMI
                 {parse_tree_int =
                    new_node(SIMPWFF,
                             new_node(CONTEXT, context2maincontext($4), $2), Nil);
                 return(0);}
              | TOK_CTLWFF ctl_expr TOK_SEMI
                 {parse_tree_int =
                    new_node(CTLWFF,
                             new_node(CONTEXT, Nil, $2), Nil); return(0);}
              | TOK_CTLWFF ctl_expr TOK_INCONTEXT context TOK_SEMI
                 {parse_tree_int =
                    new_node(CTLWFF,
                             new_node(CONTEXT, context2maincontext($4), $2), Nil);
                 return(0);}
              | TOK_LTLWFF ltl_expr TOK_SEMI
                 {parse_tree_int =
                    new_node(LTLWFF,
                             new_node(CONTEXT, Nil, $2), Nil); return(0);}
              | TOK_LTLWFF ltl_expr TOK_INCONTEXT context TOK_SEMI
                 {parse_tree_int =
                    new_node(LTLWFF,
                             new_node(CONTEXT, context2maincontext($4), $2), Nil);
                 return(0);}
              | TOK_COMPWFF compute_expr TOK_SEMI
                 {parse_tree_int =
                    new_node(COMPWFF,
                             new_node(CONTEXT, Nil, $2), Nil); return(0);}
              | TOK_COMPWFF compute_expr TOK_INCONTEXT context TOK_SEMI
                 {parse_tree_int =
                    new_node(COMPWFF,
                             new_node(CONTEXT, context2maincontext($4), $2), Nil);
                 return(0);}
              ;

trace         : TOK_NUMBER {$$ = (node_ptr)find_atom($1);}
              | state TOK_DOT TOK_NUMBER {$$ = find_node(DOT, $1, (node_ptr)find_atom($3));}
              ;

state         : trace TOK_DOT TOK_NUMBER {$$ = find_node(DOT, $1, (node_ptr)find_atom($3));}
              ;


%%

/* Additional source code */
void yyerror(char *s)
{
    extern options_ptr options;
    extern char yytext[];
    
    start_parsing_err();
    fprintf(nusmv_stderr, "at token \"%s\": %s\n", yytext, s);
    if (opt_batch(options)) finish_parsing_err();
}

int yywrap()
{
  return(1);
}

/* This function is used to build the internal structure of the
   context (e.g. module instance name) from the parse tree. The
   function is needed since with the grammar it is not possible/simple
   to build directly the desired structure. */
static node_ptr context2maincontext(node_ptr context) {
  switch(node_get_type(context)) {
  case ATOM:
    return find_node(DOT, Nil, find_atom(context));
  case DOT: {
    node_ptr t = context2maincontext(car(context));
    return find_node(DOT, t, find_atom(cdr(context)));
  }
  case ARRAY: {
    node_ptr t = context2maincontext(car(context));
    return find_node(ARRAY, t, find_atom(cdr(context)));
  }
  default:
    fprintf(nusmv_stderr, "context2maincontext: undefined token \"%d\"\n");
    nusmv_assert(false);
  }
}

static int nusmv_parse_psl() 
{
  int res;
  res = psl_yyparse();
  return res;  
}    
