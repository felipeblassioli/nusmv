/**CFile***********************************************************************

FileName    [pslPrint.c]

PackageName [parser.psl]

Synopsis    [Routines for printing of PslNode]

Description []

SeeAlso     [psl_conv.h]

Author      [Fabio Barbon, Marco Roveri]

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

#include "pslInt.h"
#include "pslExpr.h"
#include "psl_grammar.h"

#include "utils/utils.h"
#include "utils/ustring.h"


static char rcsid[] UTIL_UNUSED = "$Id: pslPrint.c,v 1.1.2.5 2005/11/16 11:44:40 nusmv Exp $"; 


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
extern FILE * nusmv_stderr; 
extern FILE * nusmv_stdout; 


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int psl_node_print ARGS((FILE* f, PslNode_ptr expr));
static int psl_node_print_id ARGS((FILE* f, PslNode_ptr id));
static int psl_print_op ARGS((FILE* f, PslOp op, boolean wrap_with_spaces));
static int psl_print_next_op ARGS((FILE * f, PslOp op));
static int psl_node_print_array ARGS((FILE* f, PslNode_ptr expr));



/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [Generic psl expression printing routine]

Description        [optional]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
int PslNode_print(FILE* f, PslNode_ptr expr)
{
  PslOp op; 

  if (expr == PSL_NULL) return 1; 

  op = psl_node_get_op(expr); 

  /*   fprintf(stderr, "OP:%d\n", op); */

  switch (op) {
  case TKCONTEXT:
    return PslNode_print(f, psl_node_get_right(expr)) && 
      (psl_node_get_left(expr) != PSL_NULL) && fprintf(f, " IN ") &&
      PslNode_print(f, psl_node_get_left(expr)); 

  case TKTRUE:
    return fprintf(f, "TRUE"); 
  case TKFALSE:
    return fprintf(f, "FALSE"); 

  case TKATOM:
  case TKIDENTIFIER:
  case TKARRAY:
    return psl_node_print_id(f, expr); 

    /* numerical expressions operators */
  case TKNUMBER:
    return psl_node_print(f, expr); 

  case TKMINUS:
  case TKPLUS:
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
  case TKCARET:
    return fprintf(f, "(") && 
      psl_node_print(f, expr) &&
      fprintf(f, ")"); 

  case TKDOTDOT:
    return
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ":") &&
      PslNode_print(f, psl_node_get_right(expr)); 


  case TKCASE:
    {
      /* "case" block is fully exploited here (handles also "ite") */
      int ret; 
      PslNode_ptr i; 
      ret = fprintf(f, "(case "); 
      for (i=expr; psl_node_is_case(i); i=psl_node_get_case_next(i))
        ret = ret && 
          PslNode_print(f, psl_node_get_case_cond(i)) &&
          fprintf(f, ":") &&
          PslNode_print(f, psl_node_get_case_then(i)) &&
          fprintf(f, "; "); 
      return ret && fprintf(f, " endcase)"); 
    }

  case TKREPLPROP:
    return fprintf(f, "( ") &&
      PslNode_print(f, psl_node_get_left(expr)) && /* replicator */
      fprintf(f, " : ") &&
      PslNode_print(f, psl_node_get_right(expr)) && /* property */
      fprintf(f, " )"); 

  case TKFORALL: 
    {
      PslNode_ptr rvs = psl_node_get_replicator_value_set(expr); 
      PslNode_ptr rr = psl_node_get_replicator_range(expr); 
      int res; 
      
      res = fprintf(f, "forall ") && 
	PslNode_print(f, psl_node_get_replicator_id(expr)); 
      
      if (rr != PSL_NULL) {
	res = res && fprintf(f, " [") && PslNode_print(f, rr) && 
	  fprintf(f, "]"); 
      }
      
      res = res && fprintf(f, " in "); 
      if (psl_node_is_boolean_type(rvs)) res = res && PslNode_print(f, rvs); 
      else {
	res = res && fprintf(f, "{") && PslNode_print(f, rvs) && 
	  fprintf(f, "}"); 
      }
      return res; 
    }     
             
  case TKCONS:
    if (psl_node_get_right(expr)) {
      return PslNode_print(f, psl_node_get_right(expr)) &&
        fprintf(f, ", ") &&
        PslNode_print(f, psl_node_get_left(expr)); 
    }
    else return PslNode_print(f, psl_node_get_left(expr)); 

  case TKBOOLEAN:
    return fprintf(f, "boolean"); 

    /* logical operators */
  case TKBANG:
    return fprintf(f, "!(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKTILDE:
    return fprintf(f, "~(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 

  case TKAMPERSAND:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " & ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKAMPERSANDAMPERSAND:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " && ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKPIPE:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " | ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKPIPEPIPE:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " || ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKMINUSGT:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " -> ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKLTMINUSGT:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " <-> ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 

  case TKABORT:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " abort ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 


    /* temporal operators */
  case TKF:
    return fprintf(f, "F (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKG:
    return fprintf(f, "G (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKU:
    return fprintf(f, "[") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " U ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, "]"); 
  case TKW:
    return fprintf(f, "[") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " W ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, "]"); 
  case TKEG:
    return fprintf(f, "EG (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKEX:
    return fprintf(f, "EX (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKEF:
    return fprintf(f, "EF (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKAG:
    return fprintf(f, "AG (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKAX:
    return fprintf(f, "AX (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKAF:
    return fprintf(f, "AF (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKA:
    return fprintf(f, "A [") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " U ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, "]"); 
  case TKE:
    return fprintf(f, "E [") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " U ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, "]"); 
    
  case TKBEFOREBANG:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " before! ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKBEFORE:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " before ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKBEFOREBANG_:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " before!_ ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKBEFORE_:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " before_ ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 

  case TKUNTILBANG:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " until! ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKUNTIL:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " until ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKUNTILBANG_:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " until!_ ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 
  case TKUNTIL_:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " until_ ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, ")"); 


  case TKALWAYS:
    return fprintf(f, "always (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKNEVER:
    return fprintf(f, "never (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 
  case TKEVENTUALLYBANG:
    return fprintf(f, "eventually! (") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ")"); 

  case TKWITHINBANG:
  case TKWITHIN:
  case TKWITHINBANG_:
  case TKWITHIN_:
    
    switch (op) {
    case TKWITHINBANG:fprintf(f, "within! ("); break; 
    case TKWITHIN:fprintf(f, "within ("); break; 
    case TKWITHINBANG_:fprintf(f, "within!_ ("); break; 
    case TKWITHIN_:fprintf(f, "within_ ("); break; 
    default: abort(); 
    }
    
    return
      PslNode_print(f, psl_node_get_left(psl_node_get_left(expr))) &&
      fprintf(f, ", ") &&
      PslNode_print(f, psl_node_get_right(psl_node_get_left(expr))) &&
      fprintf(f, ") ") &&
      PslNode_print(f, psl_node_get_right(expr)); 

  case TKWHILENOT:
  case TKWHILENOTBANG:
  case TKWHILENOT_:
  case TKWHILENOTBANG_:

    switch (op) {
    case TKWHILENOT: fprintf(f, "whilenot ("); break; 
    case TKWHILENOTBANG: fprintf(f, "whilenot! ("); break; 
    case TKWHILENOT_: fprintf(f, "whilenot_ ("); break; 
    case TKWHILENOTBANG_: fprintf(f, "whilenot!_ ("); break; 
    default: abort(); 
    }

    return 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, ") ") &&
      PslNode_print(f, psl_node_get_right(expr)); 
    

  case TKX:
  case TKXBANG:
  case TKNEXT:
  case TKNEXTBANG:
    /* simple or extended version? */
    if (!psl_node_get_right(expr)) /* iff simple next expression */
      return psl_print_next_op(f, op) && fprintf(f, "(") &&
        PslNode_print(f, psl_node_get_left(expr)) &&
        fprintf(f, ")"); 
  case TKNEXT_A:
  case TKNEXT_ABANG:
  case TKNEXT_E:
  case TKNEXT_EBANG:
  case TKNEXT_EVENTBANG:
  case TKNEXT_EVENT:
  case TKNEXT_EVENT_ABANG:
  case TKNEXT_EVENT_A:
  case TKNEXT_EVENT_EBANG:
  case TKNEXT_EVENT_E:
    {
      int ret; 

      /* assert: extended next expression */
      ret = psl_print_next_op(f, op); 
      if (op==TKX || op==TKXBANG || op==TKNEXT || op==TKNEXTBANG || 
         op==TKNEXT_A || op==TKNEXT_ABANG ||  op==TKNEXT_E || 
         op==TKNEXT_EBANG) {
        return ret && fprintf(f, "[") &&
          PslNode_print(f, psl_node_get_left(psl_node_get_right(expr))) &&
          fprintf(f, "] (") &&
          PslNode_print(f, psl_node_get_left(expr)) &&
          fprintf(f, ")"); 
        
      } else {
        /* "next_event" type next expressions */
        ret = ret && fprintf(f, "(") &&
          PslNode_print(f, psl_node_get_right(psl_node_get_right(expr))) &&
          fprintf(f, ")"); 

        if (psl_node_get_left(psl_node_get_right(expr)))
          ret = ret && fprintf(f, " [") &&
            PslNode_print(f, psl_node_get_left(psl_node_get_right(expr))) &&
            fprintf(f, "]"); 

        return ret && fprintf(f, " (") &&
          PslNode_print(f, psl_node_get_left(expr)) &&
          fprintf(f, ")"); 
        
      }
    }

    /* sere operators */
  case TKPIPEMINUSGT:
  case TKPIPEEQGT:
    return fprintf(f, "(") && 
      PslNode_print(f, psl_node_get_left(psl_node_get_left(expr))) &&
      fprintf(f, " |%c> ", (op==TKPIPEEQGT)?'=':'-') &&
      PslNode_print(f, psl_node_get_right(psl_node_get_left(expr))) &&
      (psl_node_get_right(expr))?fprintf(f, "!)"):fprintf(f, ")"); 

  case TKSERECOMPOUND:
  case TKSERE:
    nusmv_assert(!psl_node_get_right(expr)); 
    return fprintf(f, "{") &&
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, "}"); 
    /*     return PslNode_print(f, psl_node_get_left(expr)); */

  case TKSERECONCAT:
    return fprintf(f, "{") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, "; ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, "}"); 

  case TKSEREREPEATED:
    {
      int retval=1; 

      nusmv_assert(psl_node_get_left(expr)); 

      retval = retval && fprintf(f, "{"); 

      /* see psl_expr_make_repeated_sere() above for parse-tree structure */
      if (psl_node_get_left(psl_node_get_left(expr))) /* iff sere != nil */
        retval = retval &&
	  PslNode_print(f, psl_node_get_left(psl_node_get_left(expr))); 
      retval = retval && fprintf(f, "["); 

      {
        PslOp sop = psl_node_get_op(psl_node_get_left(expr)); 
        switch (sop) {
        case TKLBSPLAT: retval = retval && fprintf(f, "*"); break; 
        case TKLBPLUSRB: retval = retval && fprintf(f, "+"); break; 
        case TKLBEQ: retval = retval && fprintf(f, "="); break; 
        case TKLBMINUSGT: retval = retval && fprintf(f, "->"); break; 
        default: 
	  fprintf(nusmv_stderr, "PslNode_print: operator type not supported " \
		  "in repeated sere\"%d\"\n", op); 
	  nusmv_exit(1); 
        }
      }

      if (psl_node_get_right(expr)) {
        retval = retval && PslNode_print(f, psl_node_get_right(expr));      
      }
      retval = retval && fprintf(f, "]}"); 
      return retval; 
    }

  case TKSEREFUSION:
    return fprintf(f, "{") && 
      PslNode_print(f, psl_node_get_left(expr)) &&
      fprintf(f, " : ") &&
      PslNode_print(f, psl_node_get_right(expr)) &&
      fprintf(f, " }"); 

  default:
    fprintf(nusmv_stderr, 
	    "PslNode_print: operator type not supported \"%d\"\n", op); 
    nusmv_assert(false); 
    break; 
  }
  
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [Handles dumping of numerical psl expressions]

Description        [optional]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static int psl_node_print(FILE* f, PslNode_ptr expr)
{
  PslOp op; 

  nusmv_assert(expr != PSL_NULL); 

  op = psl_node_get_op(expr); 

  switch (op) {
  case TKTRUE:  return fprintf(f, "TRUE"); 
  case TKFALSE: return fprintf(f, "FALSE"); 

  case TKNUMBER: return fprintf(f, "%d", (int) psl_node_get_left(expr)); 
    
  case TKATOM:
  case TKIDENTIFIER:
  case TKARRAY:
    return psl_node_print_id(f, expr); 

  case TKMINUS:
  case TKPLUS:
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
  case TKCARET:
    if (psl_node_get_right(expr) != PSL_NULL) {
      /* Binary form */
      return fprintf(f, "(") && 
        PslNode_print(f, psl_node_get_left(expr)) &&
        psl_print_op(f, op, true) &&
        PslNode_print(f, psl_node_get_right(expr)) &&
        fprintf(f, ")"); 
    }
    else {
      /* Unary form */
      return psl_print_op(f, op, false) && 
	PslNode_print(f, psl_node_get_left(expr));
    }

  default:
    fprintf(nusmv_stderr, 
	    "psl_node_print: operator type not supported \"%d\"\n", 
	    op); 
    nusmv_assert(false); 
  }

  return -1; 
}


/**Function********************************************************************

Synopsis           [required]

Description        [optional]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static int psl_node_print_array(FILE* f, PslNode_ptr expr)
{
  int res; 
  nusmv_assert(psl_node_get_op(expr) == TKARRAY); 

  res = psl_node_print_id(f, psl_node_get_left(expr)); 
  if (res >= 0 && fprintf(f, "[") >= 0) {
    
    res = psl_node_print(f, psl_node_get_right(expr)) >= 0 
      && fprintf(f, "]"); 
  }

  return res; 
}


/**Function********************************************************************

Synopsis           [required]

Description        [optional]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static int psl_print_op(FILE* f, PslOp op, boolean wrap_with_spaces) 
{
  char* op_str = NULL;
  int res = 0;

  switch (op) {
  case TKMINUS: op_str = "-"; break; 
  case TKPLUS: op_str = "+"; break; 
  case TKUNION: op_str = "union"; break; 
  case TKIN: op_str = "in"; break; 

  case TKSPLAT: op_str = "*"; break; 
  case TKSLASH: op_str = "/"; break; 
  case TKPERCENT: op_str = "%"; break; 
  case TKEQEQ: op_str = "=="; break; 
  case TKEQ: op_str = "="; break; 
  case TKBANGEQ: op_str = "!="; break; 
  case TKLT: op_str = "<"; break; 
  case TKLE: op_str = "<="; break; 
  case TKGT: op_str = ">"; break; 
  case TKGE: op_str = ">="; break; 
  case TKCARET: op_str = "xor"; break;

  default:
    fprintf(nusmv_stderr, 
	    "psl_print_op: operator type not supported \"%d\"\n", op); 
    nusmv_assert(false); 
  }
  
  if (wrap_with_spaces) res |= fprintf(f, " ");
  res |= fprintf(f, op_str);
  if (wrap_with_spaces) res |= fprintf(f, " ");
  return res;
}


/**Function********************************************************************

Synopsis           [required]

Description        [optional]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static int psl_node_print_id(FILE* f, PslNode_ptr id)
{
  int res; 

  switch (psl_node_get_op(id)) {
  case TKATOM:
    res = fprintf(f, get_text((string_ptr) psl_node_get_left(id))); 
    break; 

  case TKIDENTIFIER:
    if (psl_node_get_left(id) != PSL_NULL) {
      res = psl_node_print(f, psl_node_get_left(id)) && 
	fprintf(f, "."); 
    }
    if (res >= 0 && psl_node_get_right(id) != PSL_NULL) {
      res = res >= 0 && psl_node_print(f, psl_node_get_right(id)); 
    }
    break; 
    
  case TKARRAY:
    res = psl_node_print_array(f, id); 
    break; 

  default:    
    fprintf(nusmv_stderr, 
	    "psl_node_print_id: operator type not supported \"%d\"\n", 
	    psl_node_get_op(id)); 
    nusmv_assert(false); 
  }

  return res; 
}



/**Function********************************************************************

Synopsis           [required]

Description        [optional]

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
static int psl_print_next_op(FILE * f, PslOp op) 
{
  switch (op) {
  case TKX: return fprintf(f, "X "); 
  case TKXBANG: return fprintf(f, "X! "); 
  case TKNEXT: return fprintf(f, "next "); 
  case TKNEXTBANG: return fprintf(f, "next! "); 
  case TKNEXT_A: return fprintf(f, "next_a "); 
  case TKNEXT_ABANG: return fprintf(f, "next_a! "); 
  case TKNEXT_E: return fprintf(f, "next_e "); 
  case TKNEXT_EBANG: return fprintf(f, "next_e! "); 
  case TKNEXT_EVENTBANG: return fprintf(f, "next_event! "); 
  case TKNEXT_EVENT: return fprintf(f, "next_event "); 
  case TKNEXT_EVENT_ABANG: return fprintf(f, "next_event_a! "); 
  case TKNEXT_EVENT_A: return fprintf(f, "next_event_a "); 
  case TKNEXT_EVENT_EBANG: return fprintf(f, "next_event_e! "); 
  case TKNEXT_EVENT_E: return fprintf(f, "next_event_e "); 
  default:
    fprintf(nusmv_stderr, 
	    "psl_print_next_op: operator type not supported \"%d\"\n", op); 
    nusmv_assert(false); 
    break; 
  }
}
