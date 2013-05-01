/**CFile***********************************************************************

FileName    [pslExpr.c]

PackageName [parser.psl]

Synopsis    [Implementation of the PSL parser interface]

Description []

SeeAlso     [pslExpr.h]

Author      [Roberto Cavada, Marco Roveri]

Copyright   [
This file is part of the ``parser.psl'' package of NuSMV version 2. 
Copyright (C) 2005 by ITC-irst. 

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

#include <stdio.h>

#include "pslExpr.h"
#include "pslInt.h"
#include "psl_grammar.h"

#include "parser/symbols.h"

#include "utils/utils.h"
#include "utils/ustring.h"

static char rcsid[] UTIL_UNUSED = "$Id: pslExpr.c,v 1.1.2.4 2005/11/15 09:04:17 nusmv Exp $"; 


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
extern FILE* nusmv_stderr; 
extern FILE* nusmv_stdout; 

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void psl_expr_print_klass ARGS((FILE* file, SyntaxClass type)); 
static int psl_expr_is_boolean ARGS((const PslExpr psl));

static int 
psl_expr_check_klass ARGS((const PslExpr psl, SyntaxClass expected));

static int psl_expr_base_num_to_val ARGS((char* base_num));
static PslNode_ptr psl_expr_context2maincontext ARGS((PslNode_ptr context));

static void 
psl_expr_require_klass ARGS((const PslExpr psl, SyntaxClass expected)); 



/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/* ---------- Creates: -------------------------------------------------

     OP_ID
      /  \
   right  Nil                    
---------------------------------------------------------------------- */
void psl_expr_make_unary_op(PslExpr* res, 
                            const PslExpr* right, 
                            const PslOp op_id, 
                            const SyntaxClass right_req_klass, 
                            const SyntaxClass res_klass)
{
  psl_expr_require_klass(*right, right_req_klass); 
  res->klass = res_klass; 
  
  if (psl_node_is_number(right->psl_node)) {
    int val = psl_node_number_get_value(right->psl_node); 
    if (op_id == TKMINUS) {
      res->psl_node = psl_node_make_number(-val); 
      return; 
    }
    if (op_id == TKPLUS) {
      res->psl_node = psl_node_make_number(val); 
      return; 
    }    
  }

  /* takes the action here */
  res->psl_node = psl_new_node(op_id, right->psl_node, PSL_NULL); 
}


/* ---------- Creates: -------------------------------------------------

      OP_ID
      /  \
   left   rigth                  
---------------------------------------------------------------------- */
void psl_expr_make_binary_op(PslExpr* res, 
                             const PslExpr* left, 
                             const PslOp op_id, 
                             const PslExpr* right, 
                             const SyntaxClass ops_req_klass, 
                             const SyntaxClass res_klass)
{
  psl_expr_require_klass(*left, ops_req_klass); 
  psl_expr_require_klass(*right, ops_req_klass); 
  
  /* takes the action here */
  res->klass = res_klass; 
  res->psl_node = psl_new_node(op_id, left->psl_node, right->psl_node); 
}
     

/* ---------- Creates: -------------------------------------------------

        NEXT_OP_ID
          /     \
  fl_property   TKCOLON
                 /   \
               when  bool_expr  

... where any 'when' and 'bool_expr' can be NULL (see the grammar) 
---------------------------------------------------------------------- */
void psl_expr_make_extended_next_op(PslOp op_id, 
                                    const PslExpr* fl_property, 
                                    const PslExpr* when, 
                                    const PslExpr* bool_expr, 
                                    PslExpr* res)
{
  PslExpr empty = psl_expr_make_empty(); 

  if (bool_expr != NULL) psl_expr_require_klass(*bool_expr, SC_BOOL_EXPR); 
  
  if (when == NULL) when = &empty; 
  if (bool_expr == NULL) bool_expr = &empty; 

  res->klass = fl_property->klass; 
  res->psl_node = psl_new_node(op_id, fl_property->psl_node, 
                               psl_new_node(TKCOLON, 
                                            when->psl_node, 
                                            bool_expr->psl_node)); 
}


/* ---------- Creates: -------------------------------------------------
   TKFORALL
    /    \
   id     TKIN
         /  \
   range   value_set

   ... where range might be empty, and value_set can be either a list
   of values/ranges or the boolean type. 
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_replicator(PslExpr id, PslExpr range, 
                                 PslExpr value_set)
{
  PslExpr res; 

  res.klass = SC_REPLICATOR; 
  res.psl_node = psl_new_node(TKFORALL, id.psl_node, 
                              psl_new_node(TKIN, range.psl_node, 
                                           value_set.psl_node)); 
  return res; 
}


/* ---------- Creates: -------------------------------------------------
   TKREPLPROP
   /  \
   replicator   property
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_replicated_property(PslExpr replicator, PslExpr expr)
{
  PslExpr res; 
  psl_expr_require_klass(replicator, SC_REPLICATOR); 
  psl_expr_require_klass(expr, SC_PROPERTY); 
  
  res.klass = expr.klass; 
  res.psl_node = psl_new_node(TKREPLPROP, replicator.psl_node, 
                              expr.psl_node); 
  return res; 
}


/* ---------- Creates: -------------------------------------------------
     TKATOM      
     /    \
   string   Nil
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_atom(const char* str)
{
  PslExpr res; 

  res.klass = SC_IDENTIFIER; 
  res.psl_node = psl_new_node(TKATOM, 
			      (PslNode_ptr) find_string((char*) str), NULL); 
  return res; 
}

/* ---------- Creates: -------------------------------------------------
     TKIDENTIFIER
       /    \
    left   right

   ... where left can be TKATOM or TKARRAY, 
   and right is in { TKIDENTIFIER, TKATOM, TKARRAY, TKNUMBER, Nil }
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_id(PslExpr left, PslExpr right)
{
  PslExpr res; 

  res.klass = SC_IDENTIFIER; 
  res.psl_node = psl_new_node(TKIDENTIFIER, 
                              left.psl_node, 
			      right.psl_node); 
  return res; 
}


/* ---------- Creates: -------------------------------------------------
              TKARRAY     
              /    \
   TKIDENTIFIER   TKNUMBER
   or TKATOM       or TKATOM
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_id_array(PslExpr id, PslExpr num)
{
  PslExpr res; 

  psl_expr_require_klass(id, SC_IDENTIFIER); 
  psl_expr_require_klass(num, SC_NUM_EXPR);  

  res.klass = SC_IDENTIFIER; 
  res.psl_node = psl_new_node(TKARRAY, id.psl_node, num.psl_node); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
              TKCONTEXT     
               /    \
            ctx     node    
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_context(PslExpr ctx, PslExpr node)
{
  PslExpr res; 

  psl_expr_require_klass(ctx, SC_IDENTIFIER); 

  res.klass = SC_IDENTIFIER; 
  res.psl_node = PslNode_new_context(
	     psl_expr_context2maincontext(ctx.psl_node), node.psl_node); 

  return res;    
}


/* ---------- Creates: -------------------------------------------------
   Nil
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_empty()
{
  PslExpr res; 
  res.klass = SC_NONE; 
  res.psl_node = PSL_NULL; 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
    TKTRUE
     /  \
   Nil   Nil   
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_true()
{
  PslExpr res; 
  res.klass = SC_BOOL_EXPR; 
  res.psl_node = psl_node_make_true(); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
    TKFALSE
     /  \
   Nil   Nil   
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_false()
{
  PslExpr res; 
  res.klass = SC_BOOL_EXPR; 
  res.psl_node = psl_node_make_false(); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
   TKINF
   /  \
   Nil   Nil
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_inf()
{
  PslExpr res; 
  res.klass = SC_NUM_EXPR; 
  res.psl_node = psl_new_node(TKINF, PSL_NULL, PSL_NULL); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
   TKBOOLEAN
   /  \
   Nil   Nil
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_boolean_type()
{
  PslExpr res; 
  res.klass = SC_NONE; 
  res.psl_node = psl_new_node(TKBOOLEAN, PSL_NULL, PSL_NULL); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
   TKTRUE              TKFALSE
   /  \       or       /   \
   Nil   Nil           Nil    Nil

   depending on the value of val
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_boolean_value(int val)
{
  PslExpr res; 
  PslOp op; 
  if (val == 0) op = TKFALSE; 
  else op = TKTRUE; 

  res.klass = SC_BOOL_EXPR; 
  res.psl_node = psl_new_node(op, PSL_NULL, PSL_NULL); 
  return res; 
}


/* ---------- Creates: -------------------------------------------------
   TKNUMBER
   /   \
   val    Nil
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_number(int val)
{
  PslExpr res; 

  res.klass = SC_NUM_EXPR; 
  res.psl_node = psl_node_make_number(val); 
  return res; 
}


/* ---------- Creates: -------------------------------------------------
   TKNUMBER
   /   \
   val    Nil
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_base_number(char* base_num)
{
  return psl_expr_make_number(psl_expr_base_num_to_val(base_num)); 
}


/* ---------- Creates: -------------------------------------------------
   TKREALNUMBER
   /   \
   fval    Nil
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_real_number(char* fval)
{
  PslExpr res; 
  res.klass = SC_NUM_EXPR; 
  res.psl_node = psl_new_node(TKREALNUMBER, 
                              (PslNode_ptr) find_string(fval), PSL_NULL); 
  return res; 
}


/* ---------- Creates: -------------------------------------------------
   TKDOTDOT
   /  \
   low   high
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_range(PslExpr low, PslExpr high)
{
  PslExpr res; 
  
  psl_expr_require_klass(low, SC_NUM_EXPR); 
  psl_expr_require_klass(high, SC_NUM_EXPR); 

  res.klass = SC_RANGE; 
  res.psl_node = psl_new_node(TKDOTDOT, low.psl_node, high.psl_node); 
  return res; 
}

  
/* ---------- Creates: -------------------------------------------------
      TKCASE
      /     \  
  TKCOLON    \  
    /  \      \
 cond  _then  _list               

   ... where _list is either a similar structure or a TKTRUE node 
   as a terminator.

   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_case(PslExpr cond, PslExpr _then, PslExpr _list)
{
  PslExpr res; 

  psl_expr_require_klass(cond, SC_BOOL_EXPR); 
  psl_expr_require_klass(_then, SC_NUM_BOOL_EXPR); 
  res.klass = _then.klass; 
  res.psl_node = psl_new_node(TKCASE, 
                              psl_new_node(TKCOLON, cond.psl_node, 
                                           _then.psl_node), 
                              _list.psl_node); 
  return res; 
}


/* ---------- Creates: -------------------------------------------------
                    TKCASE
                    /    \
                TKCOLON   \  
                 /  \      \
             cond  _then TKCASE                    
                          /  \
                      TKCOLON  TRUEEXPR
                       /  \
                 NUMBER  _else
                     /     \
                    1       Nil
 
   Note: Wouldn't be nice to have a specific expression to hold ITE?

   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_ite(PslExpr cond, PslExpr _then, PslExpr _else)
{
  PslExpr res; 
  PslExpr _elsecase; 

  _elsecase = psl_expr_make_case(psl_expr_make_number(1), 
                                 _else, 
                                 psl_expr_make_empty()); 

  res = psl_expr_make_case(cond, _then, _elsecase); 
  return res; 
}



/* ---------- Creates: -------------------------------------------------
       op
     /    \
  TKCOLON Nil
   /   \
 seq   expr

   ... where op must be in {TKPIPEMINUSGT, TKPIPEEQGT}
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_suffix_implication_weak(PslExpr seq, PslOp op, 
                                              PslExpr expr)
{

  PslExpr res; 

  nusmv_assert(op == TKPIPEMINUSGT || op == TKPIPEEQGT); 

  psl_expr_require_klass(seq, SC_SEQUENCE); 
  psl_expr_require_klass(expr, SC_FL_PROPERTY); 
  
  res.klass = SC_FL_PROPERTY; 

  res.psl_node = psl_new_node(op, 
                              psl_new_node(TKCOLON, seq.psl_node, 
                                           expr.psl_node), 
			      PSL_NULL); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
       op
      /  \
 TKCOLON  TKBANG
   / \
seq   expr

   ... where op must be in {TKPIPEMINUSGT, TKPIPEEQGT}
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_suffix_implication_strong(PslExpr seq, PslOp op, 
                                                PslExpr expr)
{
  PslExpr res; 
  
  nusmv_assert(op == TKPIPEMINUSGT || op == TKPIPEEQGT); 

  psl_expr_require_klass(seq, SC_SEQUENCE); 
  psl_expr_require_klass(expr, SC_FL_PROPERTY); 

  res.klass =  SC_FL_PROPERTY; 

  res.psl_node = psl_new_node(op, 
                              psl_new_node(TKCOLON, seq.psl_node, 
                                           expr.psl_node), 
			      psl_new_node(TKBANG, PSL_NULL, PSL_NULL)); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
           op
          /  \
     TKCOMMA  seq
      /  \
   begin  end
        
   ... where op must be in {TKWITHINBANG, TKWITHIN, TKWITHINBANG_, TKWITHIN_}
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_within(PslOp op, PslExpr begin, PslExpr end, 
                             PslExpr seq)
{
  PslExpr res; 

  nusmv_assert(op == TKWITHINBANG || op == TKWITHIN || 
               op == TKWITHINBANG_ || op == TKWITHIN_); 

  psl_expr_require_klass(begin, SC_SEQUENCE); 
  psl_expr_require_klass(end, SC_BOOL_EXPR); 
  psl_expr_require_klass(seq, SC_SEQUENCE); 

  res.klass = SC_FL_PROPERTY; 
  res.psl_node = psl_new_node(op, psl_new_node(TKCOMMA, begin.psl_node, 
                                               end.psl_node), 
                              seq.psl_node); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
       op
      /  \
   expr   seq

   ... where op must be in 
   {TKWHILENOTBANG, TKWHILENOT, TKWHILENOTBANG_, TKWHILENOT_}
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_whilenot(PslOp op, PslExpr expr, PslExpr seq)
{
  PslExpr res; 
  
  nusmv_assert(op == TKWHILENOTBANG || op == TKWHILENOT ||
               op == TKWHILENOTBANG_ || op == TKWHILENOT_); 
  
  psl_expr_require_klass(expr, SC_BOOL_EXPR); 
  psl_expr_require_klass(seq, SC_SEQUENCE); 
  
  res.klass = SC_FL_PROPERTY; 
  res.psl_node = psl_new_node(op, expr.psl_node, seq.psl_node); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
       TKABORT
        /   \
   fl_prop   cond
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_abort(PslExpr fl_prop, PslExpr cond)
{
  PslExpr res; 

  psl_expr_require_klass(fl_prop, SC_FL_PROPERTY); 
  psl_expr_require_klass(cond, SC_BOOL_EXPR); 
  
  res.klass = SC_FL_PROPERTY; 
  res.psl_node = psl_new_node(TKABORT, fl_prop.psl_node, cond.psl_node); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
     TKSERE
      /  \ 
   expr   Nil
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_sere(PslExpr expr)
{
  PslExpr res; 
  
  psl_expr_require_klass(expr, SC_SEQUENCE); 

  res.klass = SC_SEQUENCE; 
  res.psl_node = psl_new_node(TKSERE, expr.psl_node, PSL_NULL); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
    TKSERECONCAT
      /      \
   seq1     seq2
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_sere_concat(PslExpr seq1, PslExpr seq2)
{
  PslExpr res; 
  
  psl_expr_require_klass(seq1, SC_SEQUENCE); 
  psl_expr_require_klass(seq2, SC_SEQUENCE); 

  res.klass = SC_SEQUENCE; 
  res.psl_node = psl_new_node(TKSERECONCAT, seq1.psl_node, seq2.psl_node); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
     TKSEREFUSION
     /      \
   seq1     seq2
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_sere_fusion(PslExpr seq1, PslExpr seq2)
{
  PslExpr res; 
  
  psl_expr_require_klass(seq1, SC_SEQUENCE); 
  psl_expr_require_klass(seq2, SC_SEQUENCE); 

  res.klass = SC_SEQUENCE; 
  res.psl_node = psl_new_node(TKSEREFUSION, seq1.psl_node, seq2.psl_node); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
      TKSERECOMPOUND
         /    \
       op      Nil
      /  \
   seq1   seq2

   ... where op must be in {TKAMPERSANDAMPERSAND, TKAMPERSAND, TKPIPE}
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_sere_compound_binary_op(PslExpr seq1, PslOp op, 
                                              PslExpr seq2)
{
  PslExpr res; 
  
  nusmv_assert(op==TKAMPERSANDAMPERSAND || op==TKAMPERSAND || op==TKPIPE); 

  psl_expr_require_klass(seq1, SC_SEQUENCE); 
  psl_expr_require_klass(seq2, SC_SEQUENCE); 

  res.klass = SC_SEQUENCE; 
  res.psl_node = psl_node_make_sere_compound(seq1.psl_node, op, seq2.psl_node);
  return res;  
}


/* ---------- Creates: -------------------------------------------------
    TKSEREREPEATED
      /         \ 
     op         count
    /  \
  sere  Nil

   ... where op must be in {TKLBSPLAT, TKLBPLUSRB, TKLBEQ, TKLBMINUSGT}.
   
   count can be Nil.
   sere can be Nil.
   
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_repeated_sere(PslOp op, PslExpr sere, PslExpr count)
{
  PslExpr res; 
  
  nusmv_assert(op==TKLBSPLAT || op==TKLBPLUSRB || 
               op==TKLBEQ || op==TKLBMINUSGT); 

  if (sere.klass != SC_NONE) psl_expr_require_klass(sere, SC_SEQUENCE); 
  if (count.klass != SC_NONE) psl_expr_require_klass(count, SC_NUM_RANGE); 

  res.klass = SC_SEQUENCE; 
  res.psl_node = psl_new_node(TKSEREREPEATED, 
                              psl_new_node(op, sere.psl_node, PSL_NULL), 
                              count.psl_node); 
  return res;  

}


/* ---------- Creates: -------------------------------------------------
     TKCONS
     /    \
   a        b
 
   The resulting type is SC_LIST
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_cons(PslExpr a, PslExpr b)
{
  PslExpr res; 
  
  res.klass = SC_LIST; 
  res.psl_node = psl_node_make_cons(a.psl_node, b.psl_node); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
     TKCONS
     /    \
   a        b
 
   The resulting type is SC_LIST. A new node is created. 
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_cons_new(PslExpr a, PslExpr b)
{
  PslExpr res; 
  
  res.klass = SC_LIST; 
  res.psl_node = psl_node_make_cons_new(a.psl_node, b.psl_node); 
  return res;  
}

/* ---------- Creates: -------------------------------------------------
     TKCONCATENATION
      /         \
   expr_list     Nil

   ... where expr_list is a list in the form 
   CONS(expr1, CONS(expr2, ... , CONS(exprN, Nil)...))
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_concatenation(PslExpr expr_list)
{
  PslExpr res; 
  
  psl_expr_require_klass(expr_list, SC_LIST); 
  res.klass = SC_LIST; 
  res.psl_node = psl_new_node(TKCONCATENATION, expr_list.psl_node, PSL_NULL); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
   TKCONCATENATION
   /         \
   expr_list        expr

   ... where expr_list is a list in the form 
   CONS(expr1, CONS(expr2, ... , CONS(exprN, Nil)...))
   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_multiple_concatenation(PslExpr expr, 
                                             PslExpr expr_list)
{
  PslExpr res; 
  
  psl_expr_require_klass(expr_list, SC_LIST); 
  res.klass = SC_LIST; 
  res.psl_node = psl_new_node(TKCONCATENATION, expr_list.psl_node, 
                              expr.psl_node); 
  return res;  
}


/* ---------- Creates: -------------------------------------------------
   op
   /  \
   expr   Nil

   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_obe_unary(PslOp op, PslExpr expr)
{
  PslExpr res; 
  
  psl_expr_require_klass(expr, SC_OBE_PROPERTY); 

  res.klass = SC_OBE_PROPERTY; 
  res.psl_node = psl_new_node(op, expr.psl_node, PSL_NULL); 
  return res;  
} 


/* ---------- Creates: -------------------------------------------------
        op
       /  \
   left   right

   ---------------------------------------------------------------------- */
PslExpr psl_expr_make_obe_binary(PslExpr left, PslOp op, PslExpr right)
{
  PslExpr res; 
  
  psl_expr_require_klass(left, SC_OBE_PROPERTY); 
  psl_expr_require_klass(right, SC_OBE_PROPERTY); 

  res.klass = SC_OBE_PROPERTY; 
  res.psl_node = psl_new_node(op, left.psl_node, right.psl_node); 
  return res;  
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [required]

Description        [optional]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static void psl_expr_print_klass(FILE* file, SyntaxClass type) 
{
  char* name; 
  
  switch (type) {
  case SC_NUM_EXPR: name = "Numeric"; break; 
  case SC_BOOL_EXPR: name = "Boolean"; break; 
  case SC_IDENTIFIER: name = "Identifier"; break; 
  case SC_NUM_BOOL_EXPR: name = "Numeric or Boolean"; break; 
  case SC_PROPERTY: name = "Property"; break; 
  case SC_FL_PROPERTY: name = "FL Property"; break; 
  case SC_OBE_PROPERTY: name = "OBE Property"; break; 
  case SC_SEQUENCE: name = "Sequence"; break; 
  case SC_REPLICATOR: name = "Replicator"; break; 
  case SC_RANGE: name = "Range"; break; 
  case SC_NONE: name = "None"; break; 
  case SC_LIST: name = "List"; break; 
  case SC_NUM_RANGE: name = "Numeric or Range"; break; 
  default: name = "Unknown"; 
  }

  fprintf(file, name); 
}


/**Function********************************************************************

Synopsis [Returns 1 if the given node is boolean compatible type, 0
otherwise]

Description        [optional]

SideEffects        []

SeeAlso            [optional]

******************************************************************************/
static int psl_expr_is_boolean(const PslExpr psl)
{
  if ((psl.klass == SC_BOOL_EXPR)     || 
      (psl.klass == SC_IDENTIFIER)    || 
      (psl.klass == SC_NUM_BOOL_EXPR) ||
      (psl.klass == SC_PROPERTY)      ||
      (psl.klass == SC_OBE_PROPERTY)  ||
      (psl.klass == SC_FL_PROPERTY)) return 1; 
  
  if (psl_node_is_number(psl.psl_node)) {
    /* checks whether it is a "boolean" constant number */
    if ( (psl_node_number_get_value(psl.psl_node) == 0) || 
         (psl_node_number_get_value(psl.psl_node) == 1) ) return 1; 
  }

  return 0; 
}


/**Function********************************************************************

Synopsis [returns 0 if the given psl expr is not compatible with the
given klass]

Description        [optional]

SideEffects        []

SeeAlso            [optional]

******************************************************************************/
static int psl_expr_check_klass(const PslExpr psl, SyntaxClass expected)
{
  int res = (psl.klass == expected); 

  switch (expected) {
  case SC_NUM_EXPR:  res = res || (psl.klass == SC_IDENTIFIER); break; 
  case SC_BOOL_EXPR: res = res || psl_expr_is_boolean(psl); break; 

  case SC_IDENTIFIER: break; 

  case SC_NUM_BOOL_EXPR: 
    res = res || (psl.klass == SC_NUM_EXPR) || psl_expr_is_boolean(psl); 
    break; 
  
  case SC_FL_PROPERTY:
    res = res || (psl.klass == SC_IDENTIFIER) || 
      (psl.klass == SC_BOOL_EXPR) || (psl.klass == SC_SEQUENCE); 
    break; 

  case SC_OBE_PROPERTY:
    res = res || (psl.klass == SC_IDENTIFIER) || (psl.klass == SC_BOOL_EXPR); 
    break; 

  case SC_PROPERTY:
    res = res || (psl.klass == SC_IDENTIFIER) || (psl.klass == SC_BOOL_EXPR) 
      || (psl.klass == SC_FL_PROPERTY) || (psl.klass == SC_OBE_PROPERTY); 
    break; 

  case SC_REPLICATOR: break; 

  case SC_SEQUENCE: 
    res = res || 
      (psl_expr_is_boolean(psl) && PslNode_is_propositional(psl.psl_node)); 
    break; 
    
  case SC_LIST: break; 

  case SC_RANGE: break; 

  case SC_NUM_RANGE: 
    res = res || (psl.klass == SC_IDENTIFIER) || 
      (psl.klass == SC_NUM_EXPR) || (psl.klass == SC_RANGE); 
    break; 

  default: res = 0; 
  }

  return res; 
}


/**Function********************************************************************

Synopsis [Converts from base to number: TO BE IMPLEMENTED]

Description        [optional]

SideEffects        []

SeeAlso            [optional]

******************************************************************************/
static int psl_expr_base_num_to_val(char* base_num)
{ 
  nusmv_assert(false); /* Cannot be called */
  return 0; 
}



/**Function********************************************************************

Synopsis [Private service for make_context]

Description [This function is used to build the internal structure of
   the context (e.g. module instance name) from the parse tree. The
   function is needed since with the grammar it is not possible/simple
   to build directly the desired structure.]

SideEffects        []

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_expr_context2maincontext(PslNode_ptr context)
{
  /* assume psl_new_node invoke find_node */
  switch (psl_node_get_op(context)) {
  case TKATOM:
    return psl_new_node(TKIDENTIFIER, PSL_NULL, context); 
  case TKIDENTIFIER: {
    PslNode_ptr t = psl_expr_context2maincontext(psl_node_get_left(context)); 
    return psl_new_node(TKIDENTIFIER, t, psl_node_get_right(context)); 
  }
  case TKARRAY: {
    PslNode_ptr t = psl_expr_context2maincontext(psl_node_get_left(context)); 
    return psl_new_node(TKARRAY, t, psl_node_get_right(context)); 
  }
  default:
    fprintf(nusmv_stderr, "psl_expr_context2maincontext: undefined token \"%d\"\n", 
	    psl_node_get_op(context)); 
    nusmv_assert(false); 
  }
}


/**Function********************************************************************

Synopsis           [Checks that given expression is compatible with the 
given required syntactic class]

Description        [optional]

SideEffects        []

SeeAlso            [optional]

******************************************************************************/
static void psl_expr_require_klass(const PslExpr psl, SyntaxClass expected) 
{
  if (psl_expr_check_klass(psl, expected) == 0) {
    fprintf(stderr, "\nType error: expected '"); 
    psl_expr_print_klass(stderr, expected); 
    fprintf(stderr, "', got '");    
    psl_expr_print_klass(stderr, psl.klass); 
    fprintf(stderr, "' instead.\n");        
    psl_yyerror("wrong type."); 
    nusmv_exit(1); 
  }
}  
