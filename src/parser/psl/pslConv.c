/**CFile***********************************************************************

FileName    [psl_conv.c]

PackageName [parser.psl]

Synopsis    [Algorithms and conversions on PslNode structure]

Description []

SeeAlso     [psl_conv.h]

Author      [Fabio Barbon, Roberto Cavada, Simone Semprini]

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

#include "pslNode.h"
#include "pslExpr.h"
#include "psl_grammar.h"
#include "pslInt.h"

#include "parser/symbols.h"
#include "utils/NodeList.h"


static char rcsid[] UTIL_UNUSED = "$Id: pslConv.c,v 1.1.2.18 2005/11/16 11:44:40 nusmv Exp $"; 

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

/**Macro**********************************************************************
  Synopsis     [Define to optimize the convertion of next]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define PSL_CONV_DISTRIB_NEXT


/**Macro**********************************************************************
  Synopsis     [Enable for debugging of fix point]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define PSL_VERBOSE_TRANSLATE 0 


/**Macro**********************************************************************
  Synopsis     [This was implemented for the sake of readability]
  Description  [This is used by the function that converts the operators]
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define PSL_OP_CONV2(psl, smv)                 \
   if (op == ((type == SMV2PSL)? smv : psl)) { \
      switch (type) {                          \
      case PSL2SMV: return smv;                \
      case PSL2PSL: return psl;                \
      case SMV2PSL: return psl;                \
      default:                                 \
	internal_error("PSL_OP_CONV2: invalid convertsion type.\n"); \
      }                                        \
   } 


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static PslNode_ptr 
psl_node_pslobe2ctl ARGS((PslNode_ptr expr, PslOpConvType type, 
			  NodeList_ptr replicator_id_stack));

static PslNode_ptr 
psl_node_pslltl2ltl ARGS((PslNode_ptr expr, PslOpConvType type, 
			  NodeList_ptr replicator_id_stack));

static PslNode_ptr 
psl_node_expand_next_event ARGS((PslOp op, PslNode_ptr f, PslNode_ptr b, 
			   PslOpConvType type));

static PslNode_ptr 
psl_node_subst_id ARGS((PslNode_ptr expr, PslNode_ptr id, PslNode_ptr v, 
			boolean is_top_level));

static PslNode_ptr 
psl_node_range_expand ARGS((PslNode_ptr range, PslNode_ptr vset)); 

static PslNode_ptr 
psl_node_expand_replicator ARGS((PslNode_ptr rep, PslNode_ptr wff));

static PslNode_ptr psl_node_sere_remove_disj ARGS((PslNode_ptr e));

static PslNode_ptr 
psl_node_insert_inside_holes ARGS((PslNode_ptr e, PslNode_ptr to_be_inserted,
				   boolean* inserted));

static PslNode_ptr 
psl_node_sere_concat_fusion2ltl ARGS((PslNode_ptr e, PslNode_ptr phi));

static PslNode_ptr psl_node_sere_translate ARGS((PslNode_ptr e));
static boolean psl_node_sere_is_disj ARGS((PslNode_ptr e));

static PslNode_ptr 
psl_node_sere_distrib_disj ARGS((PslNode_ptr e, boolean *modified));

static PslNode_ptr psl_node_sere_remove_star_count ARGS((PslNode_ptr e));

static PslNode_ptr 
psl_node_sere_remove_trailing_star ARGS((PslNode_ptr e, boolean* modified));

static boolean psl_node_sere_is_ampersand ARGS((PslNode_ptr e));
static PslNode_ptr psl_node_sere_get_leftmost ARGS((PslNode_ptr e));
static PslNode_ptr psl_node_sere_get_rightmost ARGS((PslNode_ptr e));

static PslNode_ptr 
psl_node_sere_remove_plus ARGS((PslNode_ptr e, boolean toplevel));

static PslNode_ptr psl_node_sere_remove_trailing_plus ARGS((PslNode_ptr e));
static PslNode_ptr psl_node_remove_suffix_implication ARGS((PslNode_ptr e));

static PslNode_ptr 
psl_node_sere_remove_star ARGS((PslNode_ptr e, boolean toplevel, 
				boolean* modified));

static PslNode_ptr 
psl_node_sere_remove_ampersand ARGS((PslNode_ptr e, boolean* modified));

static PslNode_ptr 
psl_node_sere_remove_2ampersand ARGS((PslNode_ptr e, boolean *modified));

static PslNode_ptr 
psl_node_sere_remove_fusion ARGS((PslNode_ptr e, boolean *modified));

static PslNode_ptr 
psl_node_remove_forall_replicators ARGS((PslNode_ptr expr, 
					 NodeList_ptr replicator_id_stack));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/



/**Function********************************************************************

Synopsis [Converts an id to a different id type, for example a PSL id
to a SMV id]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr PslNode_convert_id(PslNode_ptr id, PslOpConvType type)
{
  PslNode_ptr result; 
  PslOp op, op_psl; 
  
  if (id == PSL_NULL) return PSL_NULL; 
  
  op = psl_node_get_op(id); 

  if (type == SMV2PSL) op_psl = psl_conv_op(op, SMV2PSL); 
  else op_psl = op; 

  switch (op_psl) {
    /* leaves: */
  case TKATOM: 
  case TKNUMBER:
    result = psl_conv_new_node(type, op, 
			       psl_node_get_left(id), 
			       psl_node_get_right(id)); 
    break; 

  case TKARRAY:
  case TKIDENTIFIER:
    result = psl_conv_new_node(type, op, 
		       PslNode_convert_id(psl_node_get_left(id), type), 
		       PslNode_convert_id(psl_node_get_right(id), type)); 
    break; 

  default:
    fprintf(nusmv_stderr, 
	    "PslNode_convert_id: operator type not supported \"%d\"\n", 
	    op_psl); 
    nusmv_assert(false); 
  }

  return result; 
}


/**Function********************************************************************

Synopsis [Takes a PSL OBE expression and builds the corresponding
CTL expression ]

Description [Takes a PSL OBE expression and builds the corresponding
CTL expression.]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
PslNode_ptr PslNode_pslobe2ctl(PslNode_ptr expr, PslOpConvType type) 
{
  NodeList_ptr repl_stack = NodeList_create();
  PslNode_ptr res = psl_node_pslobe2ctl(expr, type, repl_stack);

  NodeList_destroy(repl_stack);
  return res;
}


/**Function********************************************************************

Synopsis    [Private service for high level function PslNode_pslobe2ctl]

Description [Private service for high level function PslNode_pslobe2ctl]

SideEffects        [required]

SeeAlso            [PslNode_pslobe2ctl]

******************************************************************************/
static PslNode_ptr psl_node_pslobe2ctl(PslNode_ptr expr, PslOpConvType type, 
				       NodeList_ptr replicator_id_stack) 
{
  PslNode_ptr result; 
  PslOp op; 

  if (expr == PSL_NULL) return PSL_NULL; 

  op = psl_node_get_op(expr); 
  
  if (psl_node_is_leaf(expr)) {
    return psl_conv_new_node(type, op, 
			     psl_node_get_left(expr), 
			     psl_node_get_right(expr)); 
  }
  
  switch (op) {
  case TKCONTEXT: 

  case TKCASE:
  case TKCOLON:

    /* id */ 
  case TKARRAY:
  case TKIDENTIFIER:

    /* primary */
  case TKMINUS: 
  case TKPLUS:

    /* binary operators */
  case TKUNION:
  case TKIN:
  case TKSPLAT:
  case TKSLASH:
  case TKPERCENT:
  case TKEQEQ:
  case TKEQ:
  case TKBANGEQ:
  case TKLT:
  case TKLE:
  case TKGT:
  case TKGE:
  case TKBANG:
  case TKTILDE:

  case TKAMPERSAND:
  case TKPIPE:
  case TKCARET:
  case TKLTMINUSGT:
  case TKMINUSGT:

    /* next operators */
  case TKAX:
  case TKEX:
    /* globally operators */
  case TKAG:
  case TKEG:
    /* eventually operators */
  case TKAF:
  case TKEF:
    /* AU, EU */
  case TKE:
  case TKA:
    result = psl_conv_new_node(type, op, 
		       psl_node_pslobe2ctl(psl_node_get_left(expr), type, 
					   replicator_id_stack), 
		       psl_node_pslobe2ctl(psl_node_get_right(expr), type, 
					   replicator_id_stack)); 
    break; 

  case TKREPLPROP: 
    {
      PslNode_ptr rep = psl_node_repl_prop_get_replicator(expr); 
      PslNode_ptr wff = psl_node_repl_prop_get_property(expr); 
      PslNode_ptr id = psl_node_get_replicator_id(rep); 

      /* checks if the forall ID has been already used by an outer forall */
      if (NodeList_belongs_to(replicator_id_stack, 
			      PslNode_convert_to_node_ptr(id))) {
	error_psl_repeated_replicator_id();
      }
      NodeList_prepend(replicator_id_stack, PslNode_convert_to_node_ptr(id));

      result = psl_node_expand_replicator(rep, wff); 
      result = psl_node_pslobe2ctl(result, type, replicator_id_stack); 
      
      /* finally pops the forall ID from the stack: */
      NodeList_remove_elem_at(replicator_id_stack, 
			      NodeList_get_first_iter(replicator_id_stack));
      break; 
    }

  default:
    fprintf(nusmv_stderr, 
	    "psl_node_pslobe2ctl: operator type not supported \"%d\"\n", op); 
    nusmv_assert(false); 
  }

  return result; 
}

/**Function********************************************************************

Synopsis [Takes a PSL expression and expands all forall constructs
contained in the expression]

Description [Takes a PSL expression and expands all forall constructs
contained in the expression. Visits the syntax tree of the expressions
and whenever it finds a forall construct it expands the expression in
its scope.]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
PslNode_ptr PslNode_remove_forall_replicators(PslNode_ptr expr) 
{
  NodeList_ptr repl_stack = NodeList_create();
  PslNode_ptr res = psl_node_remove_forall_replicators(expr, repl_stack);

  NodeList_destroy(repl_stack);
  return res;
}



/**Function********************************************************************

Synopsis [Private service for high level function
          PslNode_remove_forall_replicators]

Description [Private service for high level function
             PslNode_remove_forall_replicators. In removing nested
             forall it takes into accaount possible clashes on the
             names of the bounded variables.]

SideEffects        [required]

SeeAlso            [PslNode_remove_forall_replicators]

******************************************************************************/
static PslNode_ptr psl_node_remove_forall_replicators(PslNode_ptr expr, 
						      NodeList_ptr replicator_id_stack) {
  PslNode_ptr result;
  PslOp op; 

  if (expr == PSL_NULL) return PSL_NULL; 
  if (psl_node_is_leaf(expr)) return expr; 

  op = psl_node_get_op(expr); 

  switch(op) {
  case TKREPLPROP: 
    {
      PslNode_ptr rep = psl_node_repl_prop_get_replicator(expr); 
      PslNode_ptr wff = psl_node_repl_prop_get_property(expr); 
      PslNode_ptr id = psl_node_get_replicator_id(rep); 

      /* checks if the forall ID has been already used by an outer forall */
      if (NodeList_belongs_to(replicator_id_stack, 
			      PslNode_convert_to_node_ptr(id))) {
        error_psl_repeated_replicator_id();
      }
      NodeList_prepend(replicator_id_stack, PslNode_convert_to_node_ptr(id));

      result = psl_node_expand_replicator(rep, wff); 
      result = psl_node_remove_forall_replicators(result, replicator_id_stack); 
      
      /* finally pops the forall ID from the stack: */
      NodeList_remove_elem_at(replicator_id_stack, 
			      NodeList_get_first_iter(replicator_id_stack));
      break; 
    }
  default:
    result = psl_new_node(psl_node_get_op(expr), 
                          psl_node_remove_forall_replicators(psl_node_get_left(expr),
                                                             replicator_id_stack),
                          psl_node_remove_forall_replicators(psl_node_get_right(expr),
                                                             replicator_id_stack));
    break;
  }
  return result;
}

/**Function********************************************************************

Synopsis           [Takes a PSL LTL expression and builds the 
corresponding LTL expression ]


Description [Takes a PSL LTL expression and builds the corresponding
LTL expression. This ignores SERE that can be easily mapped to the
corresponding LTL expression.  The parameter replicator_id_stack is
used to prevent ID duplication of nested forall (replicators).]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
PslNode_ptr PslNode_pslltl2ltl(PslNode_ptr expr, PslOpConvType type) 
{
  NodeList_ptr repl_stack = NodeList_create();
  PslNode_ptr res = psl_node_pslltl2ltl(expr, type, repl_stack);

  NodeList_destroy(repl_stack);
  return res;
}


/**Function********************************************************************

Synopsis           [Takes a PSL LTL expression and builds the 
corresponding LTL expression ]


Description [Takes a PSL LTL expression and builds the corresponding
LTL expression. This ignores SERE that can be easily mapped to the
corresponding LTL expression.  The parameter replicator_id_stack is
used to prevent ID duplication of nested forall (replicators).]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_pslltl2ltl(PslNode_ptr expr, PslOpConvType type, 
				       NodeList_ptr replicator_id_stack)
{
  PslNode_ptr result; 
  PslOp op; 

  if (expr == PSL_NULL) return PSL_NULL; 

  op = psl_node_get_op(expr); 
  if (psl_node_is_leaf(expr)) {
    return psl_conv_new_node(type, op, 
			     psl_node_get_left(expr), 
			     psl_node_get_right(expr)); 
  }

  switch (op) {

  case TKCASE:

  case TKCONTEXT:

  case TKCOLON:

    /* id */
  case TKARRAY:
  case TKIDENTIFIER:

    /* primary */
  case TKMINUS:
  case TKPLUS:

    /* binary operators */
  case TKUNION:
  case TKIN:
  case TKSPLAT:
  case TKSLASH:
  case TKPERCENT:
  case TKEQEQ:
  case TKEQ:
  case TKBANGEQ:
  case TKLT:
  case TKLE:
  case TKGT:
  case TKGE:
  case TKBANG:
  case TKTILDE:

  case TKAMPERSAND:
  case TKPIPE:
  case TKCARET:
  case TKLTMINUSGT:
  case TKMINUSGT:

    /* unary temporal ops */
  case TKEVENTUALLYBANG:
  case TKF:
  case TKG:
  case TKALWAYS:

    /* binary temporal ops */
  case TKU:
  case TKUNTILBANG: 
    result = psl_conv_new_node(type, op, 
		       psl_node_pslltl2ltl(psl_node_get_left(expr), type, 
					   replicator_id_stack), 
		       psl_node_pslltl2ltl(psl_node_get_right(expr), type, 
					   replicator_id_stack)); 
    break; 


    /* next* operators */
  case TKX:
  case TKXBANG:
  case TKNEXT:
  case TKNEXTBANG:
    {
      PslNode_ptr l = psl_node_pslltl2ltl(psl_node_get_left(expr), type, 
					  replicator_id_stack); 
      PslNode_ptr r = psl_node_get_right(expr); 

      /* Developer's note: we might think to use specific top level
	 ops for extended next operators, instead of reusing NEXT and X */
      if (r != PSL_NULL) {
	/* Extended next (event) operator */
        int lim; 
        PslNode_ptr lim_expr; 

        nusmv_assert(psl_node_get_op(r) == TKCOLON); 
        nusmv_assert(psl_node_get_right(r) == PSL_NULL); 

        lim_expr = psl_node_get_left(r); 
	if (!psl_node_is_number(lim_expr)) {
	  fprintf(nusmv_stderr, "In PSL expression '");
	  PslNode_print(nusmv_stderr, expr); 
	  fprintf(nusmv_stderr, "'\n");
	  error_psl_not_supported_feature_next_number();
	}
        lim = psl_node_number_get_value(lim_expr); 
          
        /* inf must be greater or equal to 0 */
        nusmv_assert(lim >=0 ); 
        for (; lim > 0; --lim) {
	  l = psl_conv_new_node(type, op, l, PSL_NULL); 
	}
      }
      else {
	/* when is not specified */
	l = psl_conv_new_node(type, op, l, PSL_NULL); 
      }

      /* At the moment we do not distinguish among weak and strong next */
      result = l; 
    }
    break; 

  case TKNEXT_E:
  case TKNEXT_EBANG:
  case TKNEXT_A:
  case TKNEXT_ABANG:
    {
      PslNode_ptr l = psl_node_get_left(expr); 
      PslNode_ptr r = psl_node_get_right(expr); 

      /* we do not distinguish among weak and strong next */
      nusmv_assert(r != PSL_NULL); 
      nusmv_assert(psl_node_get_op(r) == TKCOLON); 
      nusmv_assert(psl_node_get_right(r) == PSL_NULL); 

      result = psl_node_pslltl2ltl(l, type, replicator_id_stack); 

      {      
        PslNode_ptr rr; 
        int inf, sup; 
        PslNode_ptr lim_expr = psl_node_get_left(r); 

        nusmv_assert(psl_node_get_op(lim_expr) == TKDOTDOT); 

	if (!psl_node_is_number(psl_node_get_left(lim_expr))) {
	  fprintf(nusmv_stderr, "In PSL expression '");
	  PslNode_print(nusmv_stderr, expr); 
	  fprintf(nusmv_stderr, "'\n");
	  error_psl_not_supported_feature_next_number();
	}
        inf = psl_node_number_get_value(psl_node_get_left(lim_expr)); 

	if (!psl_node_is_number(psl_node_get_right(lim_expr))) {
	  fprintf(nusmv_stderr, "In PSL expression '");
	  PslNode_print(nusmv_stderr, expr); 
	  fprintf(nusmv_stderr, "'\n");
	  error_psl_not_supported_feature_next_number();
	}          
        sup = psl_node_number_get_value(psl_node_get_right(lim_expr)); 

	if (inf > sup) {
	  fprintf(nusmv_stderr, "Error in: ");
	  PslNode_print(nusmv_stderr, expr);
	  fprintf(nusmv_stderr, "\n");

	  error_invalid_numeric_value(sup, "Next operators expect"\
				      " ranges with high bound greater than"\
				      " low bound.");
	}

        rr = result; 
       
#ifdef PSL_CONV_DISTRIB_NEXT
        /* 
           X^{inf}(phi & X (phi & .. X(phi & X phi)....)) 
	   \----------------- sup - inf -------/
        */
        {
          int k; 

          for (k = (sup - inf); k > 0; --k) {
            if ((op == TKNEXT_E) || (op == TKNEXT_EBANG)) {
              /* result' = result | rr */
              rr = psl_conv_new_node(type, TKPIPE, result, 
			     psl_conv_new_node(type, TKXBANG, rr, PSL_NULL)); 
            }
            else {
	      rr = psl_conv_new_node(type, TKAMPERSAND, 
				     result, 
			     psl_conv_new_node(type, TKXBANG, rr, PSL_NULL)); 
            }
          }
          for (k = inf; k > 0; --k) {
            rr = psl_conv_new_node(type, TKXBANG, rr, PSL_NULL); 
          }
        }
        result = rr; 
#else        
        /* X^{inf}(phi) & X^{inf+1}(phi) & ... & X^{sup}(phi) */
	
        for (i = inf; i > 0; --i) {
          result = psl_conv_new_node(type, TKXBANG, result, PSL_NULL); 
        }

	rr = result; 

        for (i = inf + 1 ; i <= sup; ++i) {
          rr = psl_conv_new_node(type, TKXBANG, rr, PSL_NULL); 
          
          if ((op == TKNEXT_E) || (op == TKNEXT_EBANG)) {
            /* result' = result | rr */
            result = psl_conv_new_node(type, TKPIPE, result, rr); 
          }
          else {
            /* result' = result & rr */
            result = psl_conv_new_node(type, TKAMPERSAND, result, rr); 
          }
        }
#endif
      }
    }
    break; 

  case TKNEXT_EVENT:
  case TKNEXT_EVENTBANG:
    {
      PslNode_ptr b; 
      PslNode_ptr l = psl_node_get_left(expr); 
      PslNode_ptr r = psl_node_get_right(expr); 

      nusmv_assert(r != PSL_NULL); 
      nusmv_assert(psl_node_get_op(r) == TKCOLON); 

      b = psl_node_pslltl2ltl(psl_node_get_right(r), type, 
			      replicator_id_stack); 
      nusmv_assert(b != PSL_NULL); 

      /* !b {U|W} (b & l) */
      result = psl_node_expand_next_event(op, 
			  psl_node_pslltl2ltl(l, type, replicator_id_stack), 
					  b, type); 

      {
        PslNode_ptr lim_expr = psl_node_get_left(r); 

        if (lim_expr != PSL_NULL) {
          /* the next event is iterated */
          int lim;

	  if (!psl_node_is_number(lim_expr)) {
	    fprintf(nusmv_stderr, "In PSL expression '");
	    PslNode_print(nusmv_stderr, expr); 
	    fprintf(nusmv_stderr, "'\n");
	    error_psl_not_supported_feature_next_number();
	  }
	  lim  = psl_node_number_get_value(lim_expr); 
          if (lim <= 0) {
	    fprintf(nusmv_stderr, "Error in: ");
	    PslNode_print(nusmv_stderr, expr);
	    fprintf(nusmv_stderr, "\n");

	    error_invalid_numeric_value(lim, "Next event operators expect"\
					" a positive integer."); 
	  }

          for (; lim > 1; --lim) {
            /* We assume strong next */
            result = psl_conv_new_node(type, TKXBANG, result, PSL_NULL); 
            result = psl_node_expand_next_event(op, result, b, type); 
          }
        }
      }
    }
    break; 

  case TKNEXT_EVENT_E:
  case TKNEXT_EVENT_EBANG:
  case TKNEXT_EVENT_A:
  case TKNEXT_EVENT_ABANG:
    {
      PslNode_ptr b; 
      PslNode_ptr l = psl_node_get_left(expr); 
      PslNode_ptr r = psl_node_get_right(expr); 

      nusmv_assert(r != PSL_NULL); 
      nusmv_assert(psl_node_get_op(r) == TKCOLON); 

      b = psl_node_pslltl2ltl(psl_node_get_right(r), type, 
			      replicator_id_stack);
      nusmv_assert(b != PSL_NULL); 

      {
        int i, inf, sup; 
        PslNode_ptr rr; 
        PslNode_ptr lim_expr = psl_node_get_left(r); 
        
        /* !b {U|W} (b & l) */
        rr = psl_node_expand_next_event(op, 
			psl_node_pslltl2ltl(l, type, replicator_id_stack), 
					b, type); 

	if (!psl_node_is_number(psl_node_get_left(lim_expr))) {
	  fprintf(nusmv_stderr, "In PSL expression '");
	  PslNode_print(nusmv_stderr, expr); 
	  fprintf(nusmv_stderr, "'\n");
	  error_psl_not_supported_feature_next_number();  
	}
        inf = psl_node_number_get_value(psl_node_get_left(lim_expr)); 

	if (!psl_node_is_number(psl_node_get_right(lim_expr))) {
	  fprintf(nusmv_stderr, "In PSL expression '");
	  PslNode_print(nusmv_stderr, expr); 
	  fprintf(nusmv_stderr, "'\n");
	  error_psl_not_supported_feature_next_number();  
	}
        sup = psl_node_number_get_value(psl_node_get_right(lim_expr)); 

        /* inf, sup must be greater than 0 */
        if (inf <= 0) {
	  fprintf(nusmv_stderr, "Error in: ");
	  PslNode_print(nusmv_stderr, expr);
	  fprintf(nusmv_stderr, "\n");

	  error_invalid_numeric_value(inf, "Next event operators expect"\
				      " a positive range."); 
	}
	
        if (sup <= 0) {
	  fprintf(nusmv_stderr, "Error in: ");
	  PslNode_print(nusmv_stderr, expr);
	  fprintf(nusmv_stderr, "\n");

	  error_invalid_numeric_value(sup, "Next event operators expect"\
				      " a positive range."); 
	}

	if (inf > sup) {
	  fprintf(nusmv_stderr, "Error in: ");
	  PslNode_print(nusmv_stderr, expr);
	  fprintf(nusmv_stderr, "\n");

	  error_invalid_numeric_value(sup, "Next event operators expect"\
				      " ranges with high bound greater than"\
				      " low bound.");
	}

        for (i = 2; i <= inf; ++i) {
          rr = psl_conv_new_node(type, TKXBANG, rr, PSL_NULL); 
          rr = psl_node_expand_next_event(op, rr, b, type); 
        }

        result = rr; 

        for (i = inf + 1; i <= sup; ++i) {
          rr = psl_conv_new_node(type, TKXBANG, rr, PSL_NULL); 
          rr = psl_node_expand_next_event(op, rr, b, type); 

	  if (op == TKNEXT_EVENT_A || op == TKNEXT_EVENT_ABANG) {
            result = psl_conv_new_node(type, TKAMPERSAND, result, rr); 
	  } 
	  else result = psl_conv_new_node(type, TKPIPE, result, rr); 
        }
      }
      break; 
    }

    /* never */
  case TKNEVER:
    {
      /* globally */
      PslNode_ptr not_l; 
      PslNode_ptr l = psl_node_get_left(expr); 
      PslNode_ptr r = psl_node_get_right(expr); 

      nusmv_assert(r == PSL_NULL); 

      l = psl_node_pslltl2ltl(l, type, replicator_id_stack); 
      not_l = psl_conv_new_node(type, TKBANG, l, PSL_NULL); 
      result = psl_conv_new_node(type, TKG, not_l, PSL_NULL); 
      break; 
    }

    /* weak until operators */
  case TKW:
  case TKUNTIL:
    {
      PslNode_ptr f1 = psl_node_get_left(expr); 
      PslNode_ptr f2 = psl_node_get_right(expr); 

      f1 = psl_node_pslltl2ltl(f1, type, replicator_id_stack); 
      f2 = psl_node_pslltl2ltl(f2, type, replicator_id_stack); 

      result = psl_conv_new_node(type, TKU, f1, f2); 
      result = psl_conv_new_node(type, TKPIPE, result, 
				 psl_conv_new_node(type, TKG, f1, PSL_NULL)); 
      break; 
    }

  case TKUNTILBANG_:
    {
      PslNode_ptr f1 = psl_node_get_left(expr); 
      PslNode_ptr f2 = psl_node_get_right(expr); 

      f1 = psl_node_pslltl2ltl(f1, type, replicator_id_stack); 
      f2 = psl_node_pslltl2ltl(f2, type, replicator_id_stack); 

      f2 = psl_conv_new_node(type, TKAMPERSAND, f1, f2); 
      result = psl_conv_new_node(type, TKU, f1, f2); 
      break; 
    }

  case TKUNTIL_:
    {
      PslNode_ptr f1 = psl_node_get_left(expr); 
      PslNode_ptr f2 = psl_node_get_right(expr); 

      f1 = psl_node_pslltl2ltl(f1, type, replicator_id_stack); 
      f2 = psl_node_pslltl2ltl(f2, type, replicator_id_stack); 

      f2 = psl_conv_new_node(type, TKAMPERSAND, f1, f2); 
      result = psl_conv_new_node(type, TKU, f1, f2); 
      result = psl_conv_new_node(type, TKPIPE, result, 
				 psl_conv_new_node(type, TKG, f1, PSL_NULL)); 
      break; 
    }

  case TKBEFOREBANG:
    {
      PslNode_ptr f1 = psl_node_get_left(expr); 
      PslNode_ptr f2 = psl_node_get_right(expr); 

      f1 = psl_node_pslltl2ltl(f1, type, replicator_id_stack); 
      f2 = psl_node_pslltl2ltl(f2, type, replicator_id_stack); 

      f2 = psl_conv_new_node(type, TKBANG, f2, PSL_NULL); 
      f1 = psl_conv_new_node(type, TKAMPERSAND, f1, f2); 
      result = psl_conv_new_node(type, TKU, f2, f1); 
      break; 
    }

  case TKBEFORE:
    {
      PslNode_ptr f1 = psl_node_get_left(expr); 
      PslNode_ptr f2 = psl_node_get_right(expr); 

      f1 = psl_node_pslltl2ltl(f1, type, replicator_id_stack); 
      f2 = psl_node_pslltl2ltl(f2, type, replicator_id_stack); 

      f2 = psl_conv_new_node(type, TKBANG, f2, PSL_NULL); 
      f1 = psl_conv_new_node(type, TKAMPERSAND, f1, f2); 
      result = psl_conv_new_node(type, TKU, f2, f1); 
      result = psl_conv_new_node(type, TKPIPE, result, 
				 psl_conv_new_node(type, TKG, f2, PSL_NULL)); 
      break; 
    }
    
  case TKBEFOREBANG_:
    {
      PslNode_ptr f1 = psl_node_get_left(expr); 
      PslNode_ptr f2 = psl_node_get_right(expr); 

      f1 = psl_node_pslltl2ltl(f1, type, replicator_id_stack); 
      f2 = psl_node_pslltl2ltl(f2, type, replicator_id_stack); 

      f2 = psl_conv_new_node(type, TKBANG, f2, PSL_NULL); 
      result = psl_conv_new_node(type, TKU, f2, f1); 
      break; 
    }

  case TKBEFORE_:
    {
      PslNode_ptr f1 = psl_node_get_left(expr); 
      PslNode_ptr f2 = psl_node_get_right(expr); 

      f1 = psl_node_pslltl2ltl(f1, type, replicator_id_stack); 
      f2 = psl_node_pslltl2ltl(f2, type, replicator_id_stack); 

      f2 = psl_conv_new_node(type, TKBANG, f2, PSL_NULL); 
      result = psl_conv_new_node(type, TKU, f2, f1); 
      result = psl_conv_new_node(type, TKPIPE, result, 
				 psl_conv_new_node(type, TKG, f2, PSL_NULL)); 
      break;      
    }

  case TKREPLPROP: 
    {
      PslNode_ptr rep = psl_node_repl_prop_get_replicator(expr); 
      PslNode_ptr wff = psl_node_repl_prop_get_property(expr); 
      PslNode_ptr id = psl_node_get_replicator_id(rep); 

      /* checks if the forall ID has been already used by an outer forall */
      if (NodeList_belongs_to(replicator_id_stack, 
			      PslNode_convert_to_node_ptr(id))) {
	error_psl_repeated_replicator_id();
      }
      NodeList_prepend(replicator_id_stack, PslNode_convert_to_node_ptr(id));

      result = psl_node_expand_replicator(rep, wff); 
      result = psl_node_pslltl2ltl(result, type, replicator_id_stack); 

      /* finally pops the forall ID from the stack: */
      NodeList_remove_elem_at(replicator_id_stack, 
			      NodeList_get_first_iter(replicator_id_stack));
      break; 
    }

  default:
    fprintf(nusmv_stderr, 
	    "psl_node_pslltl2ltl: operator type not supported \"%d\"\n", 
	    op); 
    nusmv_assert(false); 
  }

  return result; 
}


/**Function********************************************************************

Synopsis           [Converts the given expression (possibly containing sere)
into an equivalent LTL formula]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
PslNode_ptr PslNode_remove_sere(PslNode_ptr e)
{
  PslNode_ptr r = psl_node_sere_translate(e); 
  /* Here e is a disjunction of concat/fusion */
  PslNode_ptr m = psl_node_sere_remove_disj(r); 
  return m; 
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [Converts the given operator into either a PSL operator, or 
a SMV operator, depending on the value of 'type']

Description        []

SideEffects        [required]

SeeAlso            [PSL_OP_CONV2]

******************************************************************************/
int psl_conv_op(int op, PslOpConvType type)
{
  PSL_OP_CONV2(TKCONTEXT, CONTEXT); 
  PSL_OP_CONV2(TKIDENTIFIER, DOT); 
  PSL_OP_CONV2(TKATOM, ATOM); 
  PSL_OP_CONV2(TKARRAY, ARRAY); 
  PSL_OP_CONV2(TKTRUE, TRUEEXP); 
  PSL_OP_CONV2(TKFALSE, FALSEEXP); 
  PSL_OP_CONV2(TKNUMBER, NUMBER); 
  PSL_OP_CONV2(TKMINUS, MINUS); 
  PSL_OP_CONV2(TKPLUS, PLUS); 
  PSL_OP_CONV2(TKUNION, UNION); 
  PSL_OP_CONV2(TKIN, SETIN); 
  PSL_OP_CONV2(TKSPLAT, TIMES); 
  PSL_OP_CONV2(TKSLASH, DIVIDE); 
  PSL_OP_CONV2(TKPERCENT, MOD); 
  PSL_OP_CONV2(TKEQEQ, EQUAL); 
  PSL_OP_CONV2(TKEQ, EQUAL); 
  PSL_OP_CONV2(TKBANGEQ, NOTEQUAL); 
  PSL_OP_CONV2(TKLT, LT); 
  PSL_OP_CONV2(TKLE, LE); 
  PSL_OP_CONV2(TKGT, GT); 
  PSL_OP_CONV2(TKGE, GE); 
  
  PSL_OP_CONV2(TKCASE, CASE); 
  PSL_OP_CONV2(TKCOLON, COLON); 

  PSL_OP_CONV2(TKBANG, NOT); 
  PSL_OP_CONV2(TKTILDE, NOT); 
  PSL_OP_CONV2(TKAMPERSAND, AND); 
  PSL_OP_CONV2(TKPIPE, OR); 
  PSL_OP_CONV2(TKCARET, XOR); 
  PSL_OP_CONV2(TKLTMINUSGT, IFF); 
  PSL_OP_CONV2(TKMINUSGT, IMPLIES); 
  
  PSL_OP_CONV2(TKX, OP_NEXT); 
  PSL_OP_CONV2(TKXBANG, OP_NEXT); 
  PSL_OP_CONV2(TKNEXT, OP_NEXT); 
  PSL_OP_CONV2(TKNEXTBANG, OP_NEXT); 
  
  PSL_OP_CONV2(TKEVENTUALLYBANG, OP_FUTURE); 
  PSL_OP_CONV2(TKF, OP_FUTURE); 
  
  PSL_OP_CONV2(TKG, OP_GLOBAL); 
  PSL_OP_CONV2(TKALWAYS, OP_GLOBAL); 
  
  PSL_OP_CONV2(TKU, UNTIL); 
  PSL_OP_CONV2(TKUNTILBANG, UNTIL); 
  
  /* CTL operators */
  PSL_OP_CONV2(TKAX, AX); 
  PSL_OP_CONV2(TKEX, EX); 
  PSL_OP_CONV2(TKAG, AG); 
  PSL_OP_CONV2(TKEG, EG); 
  PSL_OP_CONV2(TKAF, AF); 
  PSL_OP_CONV2(TKEF, EF); 
  
  PSL_OP_CONV2(TKE, EU); 
  PSL_OP_CONV2(TKA, AU); 
  
  fprintf(nusmv_stderr, "psl_conv_op: operator type not supported \"%d\"\n", op); 
  nusmv_assert(false);    
  return -1; 
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [During the conversion to LTL, this function is invoked 
when the expansion of next_event family is required.]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr 
psl_node_expand_next_event(PslOp op, PslNode_ptr f, PslNode_ptr b, 
			   PslOpConvType type)
{
  PslNode_ptr result, notb; 

  nusmv_assert((op == TKNEXT_EVENT) || (op == TKNEXT_EVENTBANG) || 
               (op == TKNEXT_EVENT_E) || (op == TKNEXT_EVENT_EBANG) ||
               (op == TKNEXT_EVENT_A) || (op == TKNEXT_EVENT_ABANG)); 

  notb = psl_conv_new_node(type, TKBANG, b, PSL_NULL); 
  result = psl_conv_new_node(type, TKAMPERSAND, b, f); 

  if ((op == TKNEXT_EVENT) || 
      (op == TKNEXT_EVENT_E) ||
      (op == TKNEXT_EVENT_A)) {
    /* result = psl_new_node(TKW, notb, result); */
    result = psl_conv_new_node(type, TKPIPE, 
			       psl_conv_new_node(type, TKU, notb, result), 
			       psl_conv_new_node(type, TKG, notb, PSL_NULL)); 
  }
  else {
    result = psl_conv_new_node(type, TKU, notb, result); 
  }
  return result; 
}


/**Function********************************************************************

Synopsis           [This is used to rename IDs occurring in the tree, when 
the replicator 'foreach' statement is resolved]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr 
psl_node_subst_id(PslNode_ptr expr, PslNode_ptr id, PslNode_ptr v, 
		  boolean is_top_level)
{
  if (expr == PSL_NULL) return expr; 

  /* Does not substitute inner replicators */
  if (psl_node_is_replicator(expr)) return expr; 

  if (psl_node_is_id(expr)) {
    if (psl_node_is_id_equal(expr, id) && is_top_level) return v; /* substitute */
    else {
      switch (psl_node_get_op(expr)) {
      case TKARRAY:
	/* Developers' note: second operand allows to write expressions like:
           forall i in {0,1,2}: forall arr in {a,b,c}: arr[i];
           ... will be expanded in:
	   ((((c[2] & b[2]) & a[2]) & ((c[1] & b[1]) & a[1])) & ((c[0] & b[0]) & a[0]))
	*/
	expr = psl_new_node(psl_node_get_op(expr), 
			    psl_node_subst_id(psl_node_get_left(expr), id, v, true), 
			    psl_node_subst_id(psl_node_get_right(expr), id, v, true)); 
	break; 

      case TKIDENTIFIER:
	expr = psl_new_node(psl_node_get_op(expr), 
			    psl_node_subst_id(psl_node_get_left(expr), id, v, false), 
			    psl_node_subst_id(psl_node_get_right(expr), id, v, false)); 
	break; 
	
      }
      return expr; 
    }
  }
  
  if (psl_node_is_leaf(expr)) return expr; 
  return psl_new_node(psl_node_get_op(expr), 
                      psl_node_subst_id(psl_node_get_left(expr), id, v, true), 
                      psl_node_subst_id(psl_node_get_right(expr), id, v, true)); 
}


/**Function********************************************************************

Synopsis           [Expansion of range for the replicator resolution]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_range_expand(PslNode_ptr range, PslNode_ptr vset) 
{
  PslNode_ptr result; 
  
  if (range != PSL_NULL) {
    fprintf(nusmv_stderr, "psl_node_range_expand: Replicator range " \
	    "not yet handled.\n"); 
    error_psl_not_supported_feature();
  }

  result = PSL_NULL; 

  if (psl_node_is_boolean_type(vset) == true) {
    result = psl_node_make_cons_new(psl_node_make_false(), 
				psl_node_make_cons_new(psl_node_make_true(), 
						   result)); 
  }
  else {
    nusmv_assert(psl_node_is_cons(vset)); 

    for (; vset != PSL_NULL; vset = psl_node_cons_get_next(vset)) {
      PslNode_ptr v = psl_node_cons_get_element(vset); 

      switch (psl_node_get_op(v)) {
      case TKDOTDOT: 
        {
          int i, inf, sup; 

	  if (!psl_node_is_number(psl_node_range_get_low(v))) {
	    error_psl_not_supported_feature_next_number();  
	  }
          inf = psl_node_number_get_value(psl_node_range_get_low(v)); 

	  if (!psl_node_is_number(psl_node_range_get_high(v))) {
	    error_psl_not_supported_feature_next_number();  
	  }
          sup = psl_node_number_get_value(psl_node_range_get_high(v)); 

	  if (inf > sup) {
	    fprintf(nusmv_stderr, "Error in: ");
	    PslNode_print(nusmv_stderr, v);
	    fprintf(nusmv_stderr, "\n");
	    
	    error_invalid_numeric_value(sup, 
			"Range with high bound greater than low bound.");
	  }
                                
          for (i = inf; i <= sup; i++) {
            result = psl_node_make_cons_new(psl_node_make_number(i), result); 
          }
          break; 
        }
      case TKBOOLEAN:
        result = psl_node_make_cons_new(psl_node_make_false(), 
				    psl_node_make_cons_new(psl_node_make_true(), 
						       result)); 
        break; 
      case TKNUMBER:
        result = psl_node_make_cons_new(v, result);  
        break; 

      case TKATOM:
      case TKIDENTIFIER:
        result = psl_node_make_cons_new(v, result); 
        break; 

      default:
        fprintf(nusmv_stderr, 
		"psl_node_range_expand: expression not supported \"%d\"\n", 
                psl_node_get_op(v)); 
        nusmv_assert(false); 
        break; 
      }
    }
        result = psl_node_cons_reverse(result);  /* We must reverse the list */
  }

  return result; 
}


/**Function********************************************************************

Synopsis           [Expansion of a replicator 'forall' statement]

Description [Wff must not have been converted to smv yet when this
   function is called.  The result still contains only psl tokens.]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr 
psl_node_expand_replicator(PslNode_ptr rep, PslNode_ptr wff) 
{
  PslNode_ptr result; 
  PslNode_ptr id, range, vset; 
  PslNode_ptr erange; 

  nusmv_assert(psl_node_is_replicator(rep)); 

  id = psl_node_get_replicator_id(rep); 
  range = psl_node_get_replicator_range(rep); 
  vset = psl_node_get_replicator_value_set(rep); 

  erange = psl_node_range_expand(range, vset); 

  result = PSL_NULL; 

  {
    PslNode_ptr r; 
    
    for (r = erange ; r != PSL_NULL; r = psl_node_cons_get_next(r)) {
      PslNode_ptr rwff; 
      PslNode_ptr rt; 
      PslNode_ptr v = psl_node_cons_get_element(r); 

      rt = r; 
      rwff = psl_node_subst_id(wff, id, v, true); 
      if (result == PSL_NULL) result = rwff; 
      else result = psl_new_node(TKAMPERSAND, result, rwff); 
      
      free_node(rt); 
    }
  }
  return result; 
}


/**Function********************************************************************

Synopsis           [Removes the disjunction among SERE, by distributing it]

Description [ This function assumes that expression is a disjunction
of concat/fusion]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_remove_disj(PslNode_ptr e)
{
  if (e == PSL_NULL) return PSL_NULL; 

  /* absorb sere parenteses */
  if (psl_node_get_op(e)==TKSERE) {
    return psl_node_sere_remove_disj(psl_node_get_left(e)); 
  }

  if (psl_node_is_id(e) || psl_node_is_leaf(e) || 
      PslNode_is_propositional(e)) {
    return e; 
  }
  
  if (psl_node_sere_is_concat_fusion(e)) {
    return psl_node_sere_concat_fusion2ltl(e, PSL_NULL);
  }

  /* if the top level operator is a | */
  if (psl_node_get_op(e)==TKSERECOMPOUND && psl_node_get_left(e) &&
      psl_node_get_op(psl_node_get_left(e))==TKPIPE) {
    PslNode_ptr l, r; 
    e = psl_node_get_left(e); /* gets r1 | r2 */
    l = psl_node_sere_remove_disj(psl_node_get_left(e)); /* gets r1 */
    r = psl_node_sere_remove_disj(psl_node_get_right(e)); /* gets r2 */
    return psl_new_node(TKPIPE /* here is logical or */, l, r); 
  }

  return psl_new_node(psl_node_get_op(e), 
		      psl_node_sere_remove_disj(psl_node_get_left(e)), 
		      psl_node_sere_remove_disj(psl_node_get_right(e)));  
}


/**Function********************************************************************

Synopsis           [Service due to way concat_fusion expansion is implemented]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr 
psl_node_insert_inside_holes(PslNode_ptr e, PslNode_ptr to_be_inserted,
			     boolean* inserted)
{

  if (e == PSL_NULL) return PSL_NULL; 

  if (to_be_inserted == PSL_NULL) return e; 

  if (psl_node_is_id(e) || psl_node_is_leaf(e) || 
      PslNode_is_propositional(e)) {
    return e;
  }

  /* visits e and insert to_be_inserted into the right argument of
     each hole contained in e */
  
  /* holes inside until */
  if (psl_node_get_op(e) == TKUNTILBANG) {
    *inserted = true;
    return psl_new_node(TKUNTILBANG, 
			psl_node_get_left(e), 
			psl_new_node(TKAMPERSAND, 
				     psl_node_get_right(e), 
				     to_be_inserted));
  }

  /* holes inside X of XF */

  if (psl_node_get_op(e) == TKEVENTUALLYBANG &&
      psl_node_get_left(e) == PSL_NULL) {
    /* inside F */
    *inserted = true;
    return psl_new_node(TKEVENTUALLYBANG, to_be_inserted, PSL_NULL);    
    
  }
  
  {
    PslNode_ptr l = psl_node_insert_inside_holes(psl_node_get_left(e),
						 to_be_inserted, 
						 inserted);
    PslNode_ptr r = psl_node_insert_inside_holes(psl_node_get_right(e), 
						 to_be_inserted,
						 inserted);

    return psl_new_node(psl_node_get_op(e), l, r);	    
  }
}


/**Function********************************************************************

Synopsis [Resolves concat/fusion and converts it to an equivalent LTL
expression]

Description        [This function assumes that expression is a concat/fusion]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr 
psl_node_sere_concat_fusion2ltl(PslNode_ptr e, PslNode_ptr phi)
{
  if (e == PSL_NULL) return PSL_NULL; 

  /* recursion on atomic  */
  if (psl_node_get_op(e)==TKSERE) {
    return psl_node_sere_concat_fusion2ltl(psl_node_get_left(e), phi); 
  }
  
  /* rec rule: f({r1; r2})=f(r1, next! f(r2)) */
  if (psl_node_get_op(e) == TKSERECONCAT || psl_node_get_op(e) == TKSEREFUSION) {
    PslNode_ptr r1 = psl_node_get_left(e); 
    PslNode_ptr r2 = psl_node_get_right(e); 
    PslNode_ptr next_phi;

    if (psl_node_get_op(e) == TKSERECONCAT) {
      next_phi =
	psl_new_node(TKXBANG, 
		     psl_node_sere_concat_fusion2ltl(r2, phi), 
		     PSL_NULL); 
    }

    if (psl_node_get_op(e) == TKSEREFUSION) {
      next_phi = psl_node_sere_concat_fusion2ltl(r2, phi);
    }    
    
    return psl_node_sere_concat_fusion2ltl(r1, next_phi); 
  }

  /* atomic */
  {
    PslNode_ptr rec;
    boolean inserted = false;
    rec = psl_node_insert_inside_holes(e, phi, &inserted);
    if (inserted) phi = PSL_NULL;
    rec = psl_node_sere_remove_disj(rec);
    return (phi != PSL_NULL) ? psl_new_node(TKAMPERSAND, rec, phi) : rec;
  }
}


/**Function********************************************************************

Synopsis [High-level service of exported function PslNode_remove_sere]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_translate(PslNode_ptr e)
{
  PslNode_ptr m=e; 
  boolean mod = false; 
  boolean mod1 = false;

  /* no need for fixpoint here (every possible star count is expanded 
     one shoot). */
  m = psl_node_sere_remove_star_count(m); 
#if PSL_VERBOSE_TRANSLATE
  fprintf(stdout, "multiplication<"); 
  PslNode_print(nusmv_stdout, m); 
  fprintf(nusmv_stdout, ">\n"); 
#endif

  /* a first call to remove_star is needed to remove possible trailing 
     starts and allow the treatmeant of possible trailing pluses, like 
     in {a;[+];[*]} */
  /* here mod1 is passed as a dummy parameter and has no consequencese
     on the following code */
  m = psl_node_sere_remove_star(m, true, &mod1);
#if PSL_VERBOSE_TRANSLATE 
  fprintf(stdout, "rem[*]<"); 
  PslNode_print(nusmv_stdout, m); 
  fprintf(nusmv_stdout, ">\n"); 
#endif
  
  m = psl_node_sere_remove_plus(m, true); 
#if PSL_VERBOSE_TRANSLATE
  fprintf(stdout, "rem[+]<"); 
  PslNode_print(nusmv_stdout, m); 
  fprintf(nusmv_stdout, ">\n"); 
#endif
  
  m = psl_node_remove_suffix_implication(m);
#if PSL_VERBOSE_TRANSLATE
  fprintf(stdout, "rem[si]<"); 
  PslNode_print(nusmv_stdout, m); 
  fprintf(nusmv_stdout, ">\n"); 
#endif
  
  do {
    mod = false; 
    
    m = psl_node_sere_remove_star(m, true, &mod1); mod |= mod1;  
#if PSL_VERBOSE_TRANSLATE
    fprintf(stdout, "rem[*]<"); 
    PslNode_print(nusmv_stdout, m); 
    fprintf(nusmv_stdout, ">\n"); 
#endif
    
    m = psl_node_sere_remove_ampersand(m, &mod1); mod |= mod1; 
#if PSL_VERBOSE_TRANSLATE
    if (mod1) {
      fprintf(stdout, "rem&<"); 
      PslNode_print(nusmv_stdout, m); 
      fprintf(nusmv_stdout, ">\n"); 
    }
#endif

    m = psl_node_sere_remove_2ampersand(m, &mod1); mod=mod || mod1; 
#if PSL_VERBOSE_TRANSLATE
    if (mod1) {
      fprintf(stdout, "rem&&<"); 
      PslNode_print(nusmv_stdout, m); 
      fprintf(nusmv_stdout, ">\n"); 
    }
#endif

    m = psl_node_sere_remove_fusion(m, &mod1); mod=mod || mod1; 
#if PSL_VERBOSE_TRANSLATE
    if (mod1) {
      fprintf(stdout, "rem:<"); 
      PslNode_print(nusmv_stdout, m); 
      fprintf(nusmv_stdout, ">\n"); 
    }
#endif

    m = psl_node_sere_distrib_disj(m, &mod1); mod=mod || mod1; 
#if PSL_VERBOSE_TRANSLATE
    if (mod1) {
      fprintf(stdout, "dist|<"); 
      PslNode_print(nusmv_stdout, m); 
      fprintf(nusmv_stdout, ">\n"); 
    }
#endif

#if PSL_VERBOSE_TRANSLATE
    if (mod) {
      fprintf(nusmv_stdout, "MOD: <"); 
      PslNode_print(nusmv_stdout, m); 
      fprintf(nusmv_stdout, ">\n"); 
    }
#endif
  } while (mod); 
  
  return m; 
}


/**Function********************************************************************

Synopsis [Returns true if the given expression is a disjunction of SEREs.]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static boolean psl_node_sere_is_disj(PslNode_ptr e)
{
  if (e == PSL_NULL) return false; 
  if (psl_node_get_left(e) == PSL_NULL) return false; 
  if (psl_node_get_op(e)==TKSERE) {
    return psl_node_sere_is_disj(psl_node_get_left(e)); 
  }
  return (psl_node_get_op(e)==TKSERECOMPOUND &&
	  psl_node_get_op(psl_node_get_left(e))==TKPIPE); 
}


/**Function********************************************************************

Synopsis           [Distributes the disjunction among SEREs]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_distrib_disj(PslNode_ptr e, boolean *modified)
{
  PslOp op; 
  PslOp real_op;

  *modified=false; 

  if (e == PSL_NULL) return PSL_NULL; 

  if (psl_node_is_leaf(e) || psl_node_is_id(e)) return e; 

  op = psl_node_get_op(e);
  real_op = (op==TKSERECOMPOUND) ? psl_node_get_op(psl_node_get_left(e)) : op;

  /* distributive rule (lookahead on parse-tree is used ) */
  if (real_op==TKSERECONCAT || real_op==TKSEREFUSION || 
      real_op==TKAMPERSAND || real_op==TKAMPERSANDAMPERSAND) {    
    PslNode_ptr l, r, disj; 

    if (op==TKSERECOMPOUND) {
      l = psl_node_get_left(psl_node_get_left(e)); 
      r = psl_node_get_right(psl_node_get_left(e)); 
    } 
    else {
      l = psl_node_get_left(e); 
      r = psl_node_get_right(e); 
    }

    if (psl_node_sere_is_disj(l)) {
      /* left node is a disjunction */
      PslNode_ptr r1, r2, r3, r1_op_r3, r2_op_r3; 

      *modified=true; 
      while (psl_node_get_op(l)==TKSERE) l = psl_node_get_left(l); 

      disj = psl_node_get_left(l); /* to remove TKSERECOMPOUND */
      nusmv_assert(psl_node_get_op(disj)==TKPIPE); 

      r1 = psl_node_get_left(disj); 
      r2 = psl_node_get_right(disj); 
      r3 = r; 

      if (real_op==TKSERECONCAT || real_op==TKSEREFUSION) {
	r1_op_r3 = psl_new_node(real_op, r1, r3); 
	r2_op_r3 = psl_new_node(real_op, r2, r3); 
      } 
      else {
	r1_op_r3 = psl_node_make_sere_compound(r1, real_op, r3); 
	r2_op_r3 = psl_node_make_sere_compound(r2, real_op, r3); 
      }

      return psl_node_make_sere_compound(r1_op_r3, TKPIPE, r2_op_r3); 
    } 
    else if (psl_node_sere_is_disj(r)) {
      /* right node is a disjunction (no matter if both are) */
      PslNode_ptr r1, r2, r3, r3_op_r1, r3_op_r2; 

      *modified=true; 

      while (psl_node_get_op(r)==TKSERE) r = psl_node_get_left(r); 

      disj = psl_node_get_left(r); /* to remove TKSERECOMPOUND */
      nusmv_assert(psl_node_get_op(disj)==TKPIPE); 

      r1 = psl_node_get_left(disj); 
      r2 = psl_node_get_right(disj); 
      r3 = l; 
      
      if (real_op==TKSERECONCAT || real_op==TKSEREFUSION) {
	r3_op_r1 = psl_new_node(real_op, r3, r1); 
	r3_op_r2 = psl_new_node(real_op, r3, r2); 
      } 
      else {
	r3_op_r1 = psl_node_make_sere_compound(r3, real_op, r1); 
	r3_op_r2 = psl_node_make_sere_compound(r3, real_op, r2); 
      }

      return psl_node_make_sere_compound(r3_op_r1,TKPIPE, r3_op_r2); 
    }
  }

  /* fallback rule */
  {
    boolean lm, rm; 
    PslNode_ptr l=psl_node_sere_distrib_disj(psl_node_get_left(e), &lm); 
    PslNode_ptr r=psl_node_sere_distrib_disj(psl_node_get_right(e), &rm); 
    *modified = (lm||rm); 
    return psl_new_node(psl_node_get_op(e), l, r); 
  }
}


/**Function********************************************************************

Synopsis           [Resolves starred SEREs]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_remove_star_count(PslNode_ptr e)
{
  if (e == PSL_NULL) return PSL_NULL; 

  if (psl_node_is_leaf(e) || psl_node_is_id(e)) return e; 

  if (psl_node_sere_is_propositional(e)) return e; 

  if (psl_node_sere_is_repeated(e) && !psl_node_sere_is_star_count(e)) return e;

  if (psl_node_get_op(e)==TKSERE) {
    return psl_node_sere_remove_star_count(psl_node_get_left(e)); 
  }

  if (psl_node_sere_is_star_count(e)) {
    /* assumes the counter is a number */
    PslNode_ptr count_range = psl_node_sere_star_get_count(e);
    int count;

    nusmv_assert(psl_node_is_number(count_range));
    count = psl_node_number_get_value(count_range); 
    if (count>0) {
      /* a [* count] possibly applied to a sere */
      PslNode_ptr mul = psl_node_sere_remove_star_count(psl_node_get_left(psl_node_get_left(e))); 
      if (mul ==  PSL_NULL) {
	/* a [* count] stand-alone, i.e. not applied to a sere, can be
	   treated as if applied to the sere {True} */
	mul = psl_new_node(TKSERE, psl_node_make_true(), PSL_NULL);
      }
      {
      PslNode_ptr acc = mul; 
      for (count--; count>0; count--) acc = psl_new_node(TKSERECONCAT, mul, acc); 
      
      return acc; 
      }
    } /* else empty star or plus on propositionals */
  }
  
  {
    PslNode_ptr l=psl_node_sere_remove_star_count(psl_node_get_left(e)); 
    PslNode_ptr r=psl_node_sere_remove_star_count(psl_node_get_right(e)); 
    return psl_new_node(psl_node_get_op(e), l, r); 
  }
}


/**Function********************************************************************

Synopsis           [Returns true if the given SERE is in the form {a} & {b}]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static boolean psl_node_sere_is_ampersand(PslNode_ptr e)
{
  return psl_node_is_sere_compound_binary(e) && 
    (psl_node_get_left(e) != PSL_NULL) &&
    (psl_node_get_op(psl_node_get_left(e))==TKAMPERSAND);
}


/**Function********************************************************************

Synopsis           [Returns the leftmost element of e that is not a SERE]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_get_leftmost(PslNode_ptr e)
{
  if (psl_node_get_op(e)==TKSERE && ! psl_node_is_sere(psl_node_get_left(e))) {
    return e; 
  }

  nusmv_assert(psl_node_sere_is_concat_fusion(e)); 
  nusmv_assert(psl_node_get_left(e) != PSL_NULL); 
  return psl_node_sere_get_leftmost(psl_node_get_left(e)); 
}


/**Function********************************************************************

Synopsis           [Returns the rightmost element of e that is not a SERE]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_get_rightmost(PslNode_ptr e)
{
  if (psl_node_get_op(e)==TKSERE) {
    if (!psl_node_is_sere(psl_node_get_left(e))) return e;
    else return psl_node_sere_get_rightmost(psl_node_get_left(e));
  }

  nusmv_assert(psl_node_sere_is_concat_fusion(e)); 
  nusmv_assert(psl_node_get_right(e) != PSL_NULL); 
  return psl_node_sere_get_rightmost(psl_node_get_right(e)); 
}


/**Function********************************************************************

Synopsis           [Resolve SERE \[+\]]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_remove_plus(PslNode_ptr e, boolean toplevel)
{
  boolean toplevel_l = toplevel;
  boolean toplevel_r = toplevel;

  if (e == PSL_NULL) return PSL_NULL; 
  if (psl_node_is_leaf(e) || psl_node_is_id(e)) return e; 

  /* toplevel trailing plus in a concatenations is simplified
     e.g. r;[+] --> r;TRUE */
  if (psl_node_get_op(e)==TKSERECONCAT && toplevel) {  
    e = psl_node_sere_remove_trailing_plus(e);
  }

  if (toplevel && psl_node_sere_is_standalone_plus(e)) {
    return psl_new_node(TKSERE, psl_node_make_true(), PSL_NULL);
  }

  if (psl_node_sere_is_plus(e)) {
    PslNode_ptr expr;

    /* since the possible toplevel trailing plus has been already simplified,
       here we are NOT toplevel */
    /* {r1;[+]} --> {r1;{F}} */   
    if(psl_node_sere_is_standalone_plus(e)) {
      return psl_new_node(TKSERE,
			  psl_new_node(TKEVENTUALLYBANG, PSL_NULL, PSL_NULL),
			  PSL_NULL);
    }
    
    expr = psl_node_sere_repeated_get_expr(e); 

    return psl_new_node(TKSERE,
			psl_new_node(TKUNTILBANG, expr, expr), 
			PSL_NULL); 
  }
  
  if (psl_node_get_op(e)==TKSERECONCAT || psl_node_get_op(e)==TKSEREFUSION) {
    toplevel_l = false;     
  }

/*   if (psl_node_get_op(e)==TKSERECONCAT || psl_node_get_op(e)==TKSEREFUSION) { */
/*     toplevel = false; */
/*   } */

  {
    PslNode_ptr l = psl_node_sere_remove_plus(psl_node_get_left(e), toplevel_l); 
    PslNode_ptr r = psl_node_sere_remove_plus(psl_node_get_right(e), toplevel_r); 

    return psl_new_node(psl_node_get_op(e), l, r); 
  }
}


/**Function********************************************************************

Synopsis           [Resolves suffix implication]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_remove_suffix_implication(PslNode_ptr e)
{
  if (e == PSL_NULL) return PSL_NULL; 

  if (psl_node_is_leaf(e) || psl_node_is_id(e) || 
      psl_node_sere_is_propositional(e)) {
    return e;
  }

  if (psl_node_is_suffix_implication(e)) {
    PslNode_ptr pre, con;
    PslNode_ptr npre, sere_con;  
    PslOp op = psl_node_get_op(e);

    pre = psl_node_suffix_implication_get_premise(e);
    con = psl_node_suffix_implication_get_consequence(e);
    npre = psl_new_node(TKBANG, pre, PSL_NULL);

    /* makes 'con' a sere if needed */
    if (!psl_node_is_sere(con)) {
      sere_con = psl_new_node(TKSERE, con, PSL_NULL);
    }
    else sere_con = con;

    /* resolves possible nested suffix implications within con: */
    sere_con = psl_node_remove_suffix_implication(sere_con);
    
    if (op == TKPIPEMINUSGT) {
      sere_con = psl_new_node(TKSEREFUSION, pre, sere_con);
    }
    else {
      nusmv_assert(op == TKPIPEEQGT);
      sere_con = psl_new_node(TKSERECONCAT, pre, sere_con);
    }

    return psl_new_node(TKPIPE, npre, sere_con);
  }
  else {
    PslNode_ptr l, r;
    l = psl_node_remove_suffix_implication(psl_node_get_left(e)); 
    r = psl_node_remove_suffix_implication(psl_node_get_right(e)); 
    return psl_new_node(psl_node_get_op(e), l, r); 
  }
}


/**Function********************************************************************

Synopsis           [Resolves starred SEREs]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_remove_star(PslNode_ptr e, boolean toplevel, 
					     boolean* modified)
{

  *modified = false;

  if (e == PSL_NULL) return PSL_NULL; 
  if (psl_node_is_leaf(e) || psl_node_is_id(e)) return e; 

  /* toplevel trailing stars in a concatenations are simplified
     e.g. r;[*];[*] --> r */
  if (toplevel) {
    e = psl_node_sere_remove_trailing_star(e, modified);
    if (e == PSL_NULL) return PSL_NULL;
  }

  if (psl_node_get_op(e)==TKSERECONCAT) {

    if (psl_node_sere_is_standalone_star(psl_node_get_right(e))) {
      PslNode_ptr l;

      /* since toplevel trailing stars have been already simplified,
	 here we are NOT toplevel */
      /* {r1;[*]} --> {r1 | {r1;{F}}} */
      nusmv_assert(!toplevel);

      l = psl_node_sere_remove_star(psl_node_get_left(e), false, modified); 

      /* regardless the effects of the recursive call on tail, the
         expression has been modified */
      *modified = true; 
      return psl_new_node(TKSERECOMPOUND,
	        psl_new_node(TKPIPE, l,
		   psl_new_node(TKSERECONCAT, l,
		      psl_new_node(TKSERE,
			 psl_new_node(TKEVENTUALLYBANG, PSL_NULL, PSL_NULL),
			    PSL_NULL))),
			  PSL_NULL);
    }

    if (psl_node_is_propstar(psl_node_get_right(e))) {
      PslNode_ptr l;
      PslNode_ptr p_star;
      PslNode_ptr p;
      
      /* {r1;b[*]} --> {r1 | {r1;{bUb}}} */

      l = psl_node_sere_remove_star(psl_node_get_left(e), false, modified); 
      p_star = psl_node_get_right(e);
      while (psl_node_get_op(p_star)==TKSERE) p_star = psl_node_get_left(p_star);
      /* gets the expression that is argument of [*] */
      p = psl_node_sere_repeated_get_expr(p_star);

      /* regardless the effects of the recursive call on tail, the
         expression has been modified */
      *modified = true;
      return psl_new_node(TKSERECOMPOUND,
	        psl_new_node(TKPIPE, l,
		   psl_new_node(TKSERECONCAT, l,
				psl_new_node(TKSERE,
					     psl_new_node(TKUNTILBANG, p, p),
					     PSL_NULL))),
			  PSL_NULL);
    }

    if (psl_node_sere_is_standalone_star(psl_node_get_left(e))) {
      PslNode_ptr r;

      /* {[*];r1} --> {r1 | {{F};r1}} */

      r = psl_node_sere_remove_star(psl_node_get_right(e), false, modified); 

      /* regardless the effects of the recursive call on tail, the
         expression has been modified */
      *modified = true;
      return psl_new_node(TKSERECOMPOUND,
	        psl_new_node(TKPIPE, r,
		   psl_new_node(TKSERECONCAT, 
		      psl_new_node(TKSERE,
			 psl_new_node(TKEVENTUALLYBANG, PSL_NULL, PSL_NULL),
			    PSL_NULL), r)),			  
			  PSL_NULL);

    }

    if (psl_node_is_propstar(psl_node_get_left(e))) {
      PslNode_ptr r;
      PslNode_ptr p_star;
      PslNode_ptr p;

      /* b[*];r2 --> r2 | {bUb};r2 */

      r = psl_node_sere_remove_star(psl_node_get_right(e), false, modified); 
      p_star = psl_node_get_left(e);
      while (psl_node_get_op(p_star)==TKSERE) p_star = psl_node_get_left(p_star); 
      /* gets the expression that is argument of [*] */
      p = psl_node_sere_repeated_get_expr(p_star);
      
      /* regardless the effects of the recursive call on tail, the
         expression has been modified */
      *modified = true;
      return psl_new_node(TKSERECOMPOUND,
		psl_new_node(TKPIPE, r,
		   psl_new_node(TKSERECONCAT,
				psl_new_node(TKSERE,
					     psl_new_node(TKUNTILBANG, p, p),
					     PSL_NULL), r)),	      	       
			  PSL_NULL);
    }    

  }

  /* either not TKSERECONCAT, or TKSERECONCAT but no star on left and right */

  /* fallback rule */
  {

    PslNode_ptr l;
    PslNode_ptr r;
    boolean toplevel_l = toplevel;
    boolean toplevel_r = toplevel;
    boolean rec_modified;

    if (psl_node_get_op(e)==TKSERECONCAT || psl_node_get_op(e)==TKSEREFUSION) {
      toplevel_l = false;     
    }


    if (psl_node_is_sere_compound_binary(e)) {

      /* gets the operands */
      l = psl_node_get_left(psl_node_get_left(e));
      r = psl_node_get_right(psl_node_get_left(e));

      /* recursive calls on operands */
      l = psl_node_sere_remove_star(l, toplevel_l, &rec_modified);
      *modified |= rec_modified;
 
      r = psl_node_sere_remove_star(r, toplevel_r, &rec_modified); 
      *modified |= rec_modified;

      if (l == PSL_NULL) return r;
      if (r == PSL_NULL) return l;
      
      return psl_node_make_sere_compound(l, 
                psl_node_get_op(psl_node_get_left(e)), r);
    }

    l = psl_node_sere_remove_star(psl_node_get_left(e), toplevel_l, 
				  &rec_modified);
    *modified |= rec_modified;
    
    r = psl_node_sere_remove_star(psl_node_get_right(e), toplevel_r, 
				  &rec_modified);
    *modified |= rec_modified;

    return psl_new_node(psl_node_get_op(e), l, r); 
  }
}


/**Function********************************************************************

Synopsis           [Resolves trailing standalone stars]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_remove_trailing_star(PslNode_ptr e, 
						      boolean* modified)
{
  PslNode_ptr head;
  PslNode_ptr tail;
  PslNode_ptr tail_rec;

  if (e == PSL_NULL) return PSL_NULL;
  if (psl_node_is_leaf(e) || psl_node_is_id(e)) return e; 

  if (psl_node_sere_is_star(e)) {
    *modified = true;
    return PSL_NULL;  
  }

  if (!(psl_node_get_op(e) == TKSERECONCAT)) return e;

  head = psl_node_get_left(e);
  tail = psl_node_get_right(e);
  tail_rec = psl_node_sere_remove_trailing_star(tail, modified);

  if (tail_rec == PSL_NULL) {
    return psl_node_sere_remove_trailing_star(head, modified);    
  }

  return psl_new_node(TKSERECONCAT, head, tail_rec);
}

/**Function********************************************************************

Synopsis           [Resolves the last trailing standalone plus]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_remove_trailing_plus(PslNode_ptr e)
{
  PslNode_ptr head;
  PslNode_ptr tail;
  PslNode_ptr tail_rec;

  if (e == PSL_NULL) return PSL_NULL;
  if (psl_node_is_leaf(e) || psl_node_is_id(e)) return e; 

  if (psl_node_sere_is_standalone_plus(e)) {
    return psl_new_node(TKSERE, psl_node_make_true(), PSL_NULL);
  }

  if (!(psl_node_get_op(e) == TKSERECONCAT)) return e;

  head = psl_node_get_left(e);
  tail = psl_node_get_right(e);
  tail_rec = psl_node_sere_remove_trailing_plus(tail);

  return psl_new_node(TKSERECONCAT, head, tail_rec);
}

/**Function********************************************************************

Synopsis           [Resolves {a}&{a}]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr 
psl_node_sere_remove_ampersand(PslNode_ptr e, boolean* modified)
{
  *modified=false; 

  if (e == PSL_NULL) return PSL_NULL; 
  if (psl_node_is_leaf(e) || psl_node_is_id(e)) return e; 
  if (psl_node_sere_is_propositional(e)) return e; 

  /* recursion step */
  if (psl_node_sere_is_ampersand(e)) {

    /* found an ampersand */
    PslNode_ptr exp = psl_node_get_left(e); /* gets r1 & r2 */
    PslNode_ptr l=psl_node_get_left(exp); /* gets r1 */
    PslNode_ptr r=psl_node_get_right(exp); /* gets r2 */
    boolean lb=psl_node_sere_is_propositional(l); 
    boolean rb=psl_node_sere_is_propositional(r); 
    boolean lc=psl_node_sere_is_concat_holes_free(l); 
    boolean rc=psl_node_sere_is_concat_holes_free(r); 
    PslNode_ptr head_l; 
    PslNode_ptr head_r; 
    PslNode_ptr heads; 
    
    /* if at least one ampersand argument is boolean (base
       expression), apply base rule: merge&(a, {b; r})={(a\and b); r} */

    /* gets the first element of both operands */
    if ((lb || lc) && (rb || rc)) {
      head_l = psl_node_sere_get_leftmost(l); 
      head_r = psl_node_sere_get_leftmost(r); 
      heads = psl_new_node(TKSERE, 
			   psl_new_node(TKAMPERSAND /* here is logical "and" */, 
					psl_node_get_left(head_l), /* extracts proposition from atomic sere */
					psl_node_get_left(head_r) /* extracts proposition from atomic sere */
					), 
			   PSL_NULL);          

      if (lb && rb) {
	*modified = true; 
	return heads; 
      }

      if ((lc && !lb) && (rc && !rb)) { 
	/* are both concatenations, we need to keep the tails; since
	   they are bot concatenations we are guaranteed the tails ar
	   not null */
	PslNode_ptr tails_rec;
	PslNode_ptr tails = psl_new_node(TKSERECOMPOUND, 
					 psl_new_node(TKAMPERSAND, 
						      psl_node_prune(l, head_l), 
						      psl_node_prune(r, head_r)), 
					 PSL_NULL); 
	
	tails_rec = psl_node_sere_remove_ampersand(tails, modified);
	/* regardless the effects of the recursive call on tail, the
	 *expression has been modified */
	*modified = true; 
	return psl_new_node(TKSERECONCAT, heads, tails_rec); 
      }

      /* here one is propositional and the other is "non atomic" concatenation */

      if (rc && !rb) { /* iff is_propositional(l) */
	*modified = true;      
	return psl_new_node(TKSERECONCAT, heads, psl_node_prune(r, head_r)); 
      }
      
      if (lc && !lb) { /* iff is_propositional(r) */
	*modified = true; 
	return psl_new_node(TKSERECONCAT, heads, psl_node_prune(l, head_l)); 
      }      
    }
  }

  /* fallback rec rule */
  /* either the top level is not a &, or the two arguments are not concatenations */
  {    
    boolean lm, rm; 

    if (psl_node_sere_is_ampersand(e)) {
      /* special handling of & */
      PslNode_ptr exp = psl_node_get_left(e); /* gets r1 & r2 */
      PslNode_ptr lexp=psl_node_get_left(exp); /* gets r1 */
      PslNode_ptr rexp=psl_node_get_right(exp); /* gets r2 */
      PslNode_ptr lrec = psl_node_sere_remove_ampersand(lexp, &lm); 
      PslNode_ptr rrec = psl_node_sere_remove_ampersand(rexp, &rm); 
      
      if (lm||rm) {
	boolean m; 
	PslNode_ptr rec = psl_node_sere_remove_ampersand(
				 psl_new_node(TKSERECOMPOUND, 
					      psl_new_node(TKAMPERSAND, lrec, rrec), 
					      PSL_NULL), 
				 &m); 
	*modified = m; 
	return rec; 
      } 
      else return e; 
      
    }
    else { 
      PslNode_ptr l = psl_node_sere_remove_ampersand(psl_node_get_left(e), &lm); 
      PslNode_ptr r = psl_node_sere_remove_ampersand(psl_node_get_right(e), &rm); 

      *modified=(lm || rm); 
      return psl_new_node(psl_node_get_op(e), l, r); 
    }
  }
}


/**Function********************************************************************

Synopsis           [Resolves {a} && {a}]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr psl_node_sere_remove_2ampersand(PslNode_ptr e, boolean *modified)
{
  *modified=false; 

  if (e == PSL_NULL) return PSL_NULL; 

  if (psl_node_is_leaf(e) || psl_node_is_id(e)) return e; 

  if (psl_node_get_op(e)==TKSERE && 
      PslNode_is_propositional(psl_node_get_left(e))) {
    return e; 
  }

  /* recursion step */
  if (psl_node_sere_is_2ampersand(e)) {

    /* found an ampersand */
    PslNode_ptr exp = psl_node_get_left(e); /* gets r1 && r2 */
    PslNode_ptr l=psl_node_get_left(exp); /* gets r1 */
    PslNode_ptr r=psl_node_get_right(exp); /* gets r2 */
    boolean lb=psl_node_sere_is_propositional(l); 
    boolean rb=psl_node_sere_is_propositional(r); 
    boolean lc=psl_node_sere_is_concat_holes_free(l); 
    boolean rc=psl_node_sere_is_concat_holes_free(r); 

    /* it could be the case that one of the two arguments is TKFALSE
       (as a result of a recursion) */
    if ((psl_node_get_op(l) == TKFALSE) || (psl_node_get_op(r) == TKFALSE)) {
      nusmv_assert(false);
    }

    /* if both the two 2ampersand argument are boolean (base expression), 
       apply base rule: merge&&(a, {b; r})=merge&&({a; r}, b)=false , 
       merge&&(a, b) = {a\and b} */
    if (lb && rb) {
      PslNode_ptr head_l = psl_node_sere_get_leftmost(l); 
      PslNode_ptr head_r = psl_node_sere_get_leftmost(r); 
      PslNode_ptr heads = 
	psl_new_node(TKSERE, 
		     psl_new_node(TKAMPERSAND, /* here is logical "and" */
				  psl_node_get_left(head_l), /* extracts proposition from atomic sere */
				  psl_node_get_left(head_r) /* extracts proposition from atomic sere */
				  ), 
		     PSL_NULL); 
      *modified=true; 
      return heads; 
    }

    /* if one of the two arguments is propositional and the other 
       is a concatenation there is no use in proceeding */
    if ((lb && rc) || (lc && rb)) {
      *modified=true; 
      return psl_new_node(TKSERE, psl_node_make_false(), PSL_NULL); 
    }
    
    /* this function assumes both arguments are concatenations */
    if (lc && rc) { 
      /* if both ampersand arguments are non atomic concatenations ("; ") , apply
	 recursive rule: merge&&({a; r_1}, {b; r_2})={(a\and b); merge&&(r_1, r_2)*/
           
      PslNode_ptr head_l = psl_node_sere_get_leftmost(l); 
      PslNode_ptr head_r = psl_node_sere_get_leftmost(r); 
      PslNode_ptr heads = 
	psl_new_node(TKSERE, 
		     psl_new_node(TKAMPERSAND, /* here is logical "and" */
				  psl_node_get_left(head_l), /* extracts proposition from atomic sere */
				  psl_node_get_left(head_r) /* extracts proposition from atomic sere */
				  ), 
		     PSL_NULL); 

      /* since arguments are bot concatenations we are guaranteed the tails ar not null */
      PslNode_ptr tails = 
	psl_new_node(TKSERECOMPOUND, 
		     psl_new_node(TKAMPERSANDAMPERSAND, 
				  psl_node_prune(l, head_l), 
				  psl_node_prune(r, head_r)), 
		     PSL_NULL); 
      
      *modified=true; 
      return psl_new_node(TKSERECONCAT, 
			  heads, 
			  psl_node_sere_remove_2ampersand(tails, modified)); 
      
      
    }
  } 
    
  /* fallback rec rule */
  /* either the top level is not a &&, or the two arguments are not concatenations */
  {    
    boolean lm, rm; 

    if (psl_node_sere_is_2ampersand(e)) {
      /* special handling of && */
      PslNode_ptr exp = psl_node_get_left(e); /* gets r1 && r2 */
      PslNode_ptr lexp=psl_node_get_left(exp); /* gets r1 */
      PslNode_ptr rexp=psl_node_get_right(exp); /* gets r2 */
      PslNode_ptr lrec = psl_node_sere_remove_2ampersand(lexp, &lm); 
      PslNode_ptr rrec = psl_node_sere_remove_2ampersand(rexp, &rm); 
      
      if (lm || rm) {
	boolean m; 
	PslNode_ptr rec = psl_node_sere_remove_2ampersand(
				  psl_new_node(TKSERECOMPOUND, 
					       psl_new_node(TKAMPERSANDAMPERSAND, lrec, rrec), 
					       PSL_NULL), 
				  &m); 
	*modified = *modified || m; 
	return rec; 
      } 
      else return e; 
    }
    else {  
      PslNode_ptr l = psl_node_sere_remove_2ampersand(psl_node_get_left(e), &lm); 
      PslNode_ptr r = psl_node_sere_remove_2ampersand(psl_node_get_right(e), &rm); 

      *modified = (lm || rm); 
      return psl_new_node(psl_node_get_op(e), l, r); 
    }
  }
}


/**Function********************************************************************

Synopsis           [Resolves {a}:{a}]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static PslNode_ptr 
psl_node_sere_remove_fusion(PslNode_ptr e, boolean *modified)
{
  PslNode_ptr l; 
  PslNode_ptr r; 

  *modified=false; 

  if (e == PSL_NULL) return PSL_NULL; 

  if (psl_node_is_leaf(e) || psl_node_is_id(e)) return e; 

  if (psl_node_sere_is_propositional(e)) return e; 

  /* if the top level operator is a : and the arguments are
     concatenation_or_fusion, then merges the arguments by conjoyning
     the righmost element of the left operand and the left most
     element of the right operand */
  l = psl_node_get_left(e); 
  r = psl_node_get_right(e); 
  if (psl_node_get_op(e)==TKSEREFUSION &&  
      psl_node_sere_is_concat_fusion_holes_free(l) && 
      psl_node_sere_is_concat_fusion_holes_free(r)) {    
    boolean lb = psl_node_sere_is_propositional(l); 
    boolean rb = psl_node_sere_is_propositional(r); 

    /* if both arguments are propositional, returns their conjunction */
    if (lb && rb) {    
      *modified=true; 
      return psl_new_node(TKSERE, 
			  psl_new_node(TKAMPERSAND, /* here is logical "and" */
				       psl_node_get_left(psl_node_sere_get_leftmost(l)), 
				       psl_node_get_left(psl_node_sere_get_leftmost(r))), 
			  PSL_NULL); 
      
    }

    /* if only one argument is propositional, recurs on the other
       argument and then joins the first argument to the left most
       element of the result of the recursion */ 
    if (lb) { 
      /* left is propositional, iff the other is not, i.e. is
	 concat_or_fusion */
      boolean m; 

      /* proposition a: */
      PslNode_ptr a = psl_node_sere_get_leftmost(l); 
      PslNode_ptr rec = psl_node_sere_remove_fusion(r, &m); 
      PslNode_ptr b = psl_node_sere_get_leftmost(rec); /* r head */
      PslNode_ptr head = 
	psl_new_node(TKSERE, 
		     psl_new_node(TKAMPERSAND, /* here is logical "and" */
				  psl_node_get_left(a), psl_node_get_left(b)), 
		     PSL_NULL); 

      PslNode_ptr tail = psl_node_prune(rec, b); 

      *modified=true; 

      /* recurring on one argument could make it propositional (e.g. {a:b} -> {a&b}), 
	 and its tail could be null */
      if (tail == PSL_NULL) return head; 
      else return psl_new_node(TKSERECONCAT, head, tail); 
    }

    if (rb) { 
      /* right is propositional, iff the other is not, i.e. is
	 concat_or_fusion */
      boolean m; 

      /* proposition a: */
      PslNode_ptr a = psl_node_sere_get_leftmost(r); 
      PslNode_ptr rec = psl_node_sere_remove_fusion(l, &m); 
      PslNode_ptr b = psl_node_sere_get_rightmost(rec); /* r tail */
      PslNode_ptr tail = 
	psl_new_node(TKSERE, 
		     psl_new_node(TKAMPERSAND, /* here is logical "and" */
				  psl_node_get_left(b), psl_node_get_left(a)), 
		     PSL_NULL); 

      PslNode_ptr head = psl_node_prune(rec, b); 

      *modified=true; 

      /* recurring on one argument could make it propositional (e.g. {a:b} -> {a&b}), 
	 and its tail could be null */
      if (head == PSL_NULL) return tail; 
      else return psl_new_node(TKSERECONCAT, head, tail); 
    }

    /* both arguments are concat_or_fusion */
    {
      boolean m; 
      
      /* recursive calls : the results do NOT contains : */
      PslNode_ptr l_rec = psl_node_sere_remove_fusion(l, &m); 
      PslNode_ptr r_rec = psl_node_sere_remove_fusion(r, &m); 
      
      /* the two elements to be merged */
      PslNode_ptr l_tail = psl_node_sere_get_rightmost(l_rec); 
      PslNode_ptr r_head = psl_node_sere_get_leftmost(r_rec); 

      /* the rest of the arguments */
      PslNode_ptr l_rest = psl_node_prune(l_rec, l_tail); 
      PslNode_ptr r_rest = psl_node_prune(r_rec, r_head); 

      PslNode_ptr merge_point =
	psl_new_node(TKSERE, 
		     psl_new_node(TKAMPERSAND, /* here is logical "and" */
				  psl_node_get_left(l_tail), 
				  psl_node_get_left(r_head)), 
		     PSL_NULL); 
      if ((l_rest == PSL_NULL) && (r_rest == PSL_NULL)) {
	*modified=true; 
	return merge_point; 
      }

      if (l_rest == PSL_NULL) {
	*modified=true; 
	return psl_new_node(TKSERECONCAT, 
			    merge_point, 
			    r_rest); 
      }

      if (r_rest == PSL_NULL) {
	*modified=true; 
	return psl_new_node(TKSERECONCAT, 
			    l_rest, 
			    merge_point); 
      }

      *modified=m; 
      return psl_new_node(TKSERECONCAT, 
			  l_rest, 
			  psl_new_node(TKSERECONCAT, 
				       merge_point, 
				       r_rest)); 
    }
  }
				      
  /* fall back rule: if the top level operator is not a :, then
     continue on its arguments */
  {
    boolean lm, rm; 

    PslNode_ptr l = psl_node_sere_remove_fusion(psl_node_get_left(e), &lm); 
    PslNode_ptr r = psl_node_sere_remove_fusion(psl_node_get_right(e), &rm); 
    PslNode_ptr result; 

    *modified = lm || rm; 
    result = psl_new_node(psl_node_get_op(e), l, r); 

    return result; 
  }
}

