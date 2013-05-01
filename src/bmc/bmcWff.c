/**CFile***********************************************************************

  FileName    [bmcWff.c]

  PackageName [bmc]

  Synopsis    [Well Formed Formula manipulation routines]

  Description []

  SeeAlso     []

  Author      [Alessandro Cimatti and Lorenzo Delana]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by ITC-irst and University of Trento.

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

#include "bmcWff.h"
#include "bmcInt.h" /* for max and min */

#include "cudd.h" /* for FALSE */
#include "parser/symbols.h" /* for constants */
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: bmcWff.c,v 1.12.4.5.2.3 2004/08/25 09:03:35 uid500 Exp $";

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

static hash_ptr wff2nnf_hash = (hash_ptr) NULL;

void bmc_init_wff2nnf_hash() 
{
  nusmv_assert(wff2nnf_hash == (hash_ptr) NULL);
  wff2nnf_hash = new_assoc();
}

void bmc_quit_wff2nnf_hash() 
{
  if (wff2nnf_hash != (hash_ptr) NULL) {
    free_assoc(wff2nnf_hash);
    wff2nnf_hash = (hash_ptr) NULL;
  }
}

static void bmc_wff2nnf_hash_insert_entry(node_ptr wff, boolean polarity, 
					  node_ptr nnf)
{
  nusmv_assert(wff2nnf_hash != (hash_ptr) NULL);
  insert_assoc(wff2nnf_hash, find_node(CONS, wff, (node_ptr) polarity), nnf);
}

static node_ptr bmc_wff2nnf_hash_lookup_entry(node_ptr wff, boolean polarity)
{
  nusmv_assert(wff2nnf_hash != (hash_ptr) NULL);
  return find_assoc(wff2nnf_hash, find_node(CONS, wff, (node_ptr) polarity));
}


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static node_ptr bmc_wff_expand_case_aux ARGS((node_ptr wff));
static node_ptr bmc_wff_expand_case     ARGS((node_ptr wff));
static node_ptr bmc_wff_mk_nnf ARGS((node_ptr wff, boolean pol));

static node_ptr bmc_wff_mkBinary ARGS((int type, node_ptr arg1, 
				       node_ptr arg2));
static node_ptr bmc_wff_mkUnary ARGS((int type, node_ptr arg));
static node_ptr bmc_wff_mkConst ARGS((int type));


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Makes a <i>truth</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkTruth(void)
{
  return bmc_wff_mkConst(TRUEEXP);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>false</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkFalsity(void)
{
  return bmc_wff_mkConst(FALSEEXP);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>not</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkNot(node_ptr arg)
{
  return bmc_wff_mkUnary(NOT, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>and</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkAnd(node_ptr arg1, node_ptr arg2)
{
  node_ptr falsity;
  node_ptr truth;

  falsity = Bmc_Wff_MkFalsity(); 
  if ((arg1 == falsity) || (arg2 == falsity)) return falsity; 

  truth = Bmc_Wff_MkTruth();
  if (arg1 == truth) return arg2;
  if (arg2 == truth) return arg1;

  return bmc_wff_mkBinary(AND, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>or</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkOr(node_ptr arg1, node_ptr arg2)
{  
  node_ptr falsity;
  node_ptr truth;

  truth = Bmc_Wff_MkTruth();
  if ((arg1 == truth) || (arg2 == truth)) return truth;

  falsity = Bmc_Wff_MkFalsity(); 
  if (arg1 == falsity) return arg2;
  if (arg2 == falsity) return arg1;

  return bmc_wff_mkBinary(OR, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>implies</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkImplies(node_ptr arg1, node_ptr arg2)
{
  return bmc_wff_mkBinary(IMPLIES, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>iff</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkIff(node_ptr arg1, node_ptr arg2)
{
  return bmc_wff_mkBinary(IFF, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>next</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkNext(node_ptr arg)
{
  return bmc_wff_mkUnary(NEXT, arg);
}


/**Function********************************************************************

  Synopsis           [Applies <i>op_next</i> x times]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkXopNext(node_ptr arg, int x)
{
  nusmv_assert(x >= 0);

  if (x == 0)
    return arg;
  else
    return Bmc_Wff_MkOpNext(Bmc_Wff_MkXopNext(arg, x - 1));
}

/**Function********************************************************************

  Synopsis           [Makes an <i>op_next</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkOpNext(node_ptr arg)
{
  return bmc_wff_mkUnary(OP_NEXT, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>op_next</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkOpPrec(node_ptr arg)
{
  return bmc_wff_mkUnary(OP_PREC, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>op_next</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkOpNotPrecNot(node_ptr arg)
{
  return bmc_wff_mkUnary(OP_NOTPRECNOT, arg);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>globally</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkGlobally(node_ptr arg)
{
  return bmc_wff_mkUnary(OP_GLOBAL, arg);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>historically</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkHistorically(node_ptr arg)
{
  return bmc_wff_mkUnary(OP_HISTORICAL, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>eventually</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkEventually(node_ptr arg)
{
  return bmc_wff_mkUnary(OP_FUTURE, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>once</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkOnce(node_ptr arg)
{
  return bmc_wff_mkUnary(OP_ONCE, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>until</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkUntil(node_ptr arg1, node_ptr arg2)
{
  return bmc_wff_mkBinary(UNTIL, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>since</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkSince(node_ptr arg1, node_ptr arg2)
{
  return bmc_wff_mkBinary(SINCE, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>releases</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkReleases(node_ptr arg1, node_ptr arg2)
{
  return bmc_wff_mkBinary(RELEASES, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>triggered</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkTriggered(node_ptr arg1, node_ptr arg2)
{
  return bmc_wff_mkBinary(TRIGGERED, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes the <b>negative normal form</b> of given WFF]

  Description        [A positive (1) polarity will not negate entire formula]

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Wff_MkNnf(node_ptr wff) 
{
  return bmc_wff_mk_nnf(wff, true);
}


/**Function********************************************************************

  Synopsis           [Returns the modal depth of the given formula]

  Description        [Returns 0 for propositional formulae, 1 or more for
  temporal formulae]

  SideEffects        [none]

  SeeAlso            []

******************************************************************************/
int Bmc_Wff_GetDepth(node_ptr ltl_wff)
{
  int depth = -1;
  int d1,d2;

  switch (node_get_type(ltl_wff)) {
  case TRUEEXP:                           /* TRUEEXP   */
  case FALSEEXP:                          /* FALSEEXP  */
    depth = 0;
    break;

  case NOT:                             /* NOT       */
    depth = Bmc_Wff_GetDepth(car(ltl_wff));
    break;

  case AND:                             /* AND       */
  case OR:                              /* OR        */
  case IFF:                             /* IFF        */
    d1 = Bmc_Wff_GetDepth(car(ltl_wff));
    d2 = Bmc_Wff_GetDepth(cdr(ltl_wff));
    depth = max(d1,d2);
    break;

  case OP_NEXT:                         /* OP_NEXT   */
  case OP_PREC:                         /* OP_PREC   */
  case OP_NOTPRECNOT:                   /* OP_NOTPRECNOT */
  case OP_GLOBAL:                       /* OP_GLOBAL */
  case OP_HISTORICAL:                   /* OP_HISTORICAL */
  case OP_FUTURE:                       /* OP_FUTURE */
  case OP_ONCE:                         /* OP_ONCE */
    depth = 1 + Bmc_Wff_GetDepth(car(ltl_wff));
    break;

  case UNTIL:                           /* UNTIL     */
  case SINCE:                           /* SINCE     */
  case RELEASES:                        /* RELEASES  */
  case TRIGGERED:                       /* TRIGGERED */
    d1 = Bmc_Wff_GetDepth(car(ltl_wff));
    d2 = Bmc_Wff_GetDepth(cdr(ltl_wff));
    depth = 1 + max(d1,d2);
    break;

  case IMPLIES:
    internal_error("implies should have been nnf-ef away!\n");
    break;

  case BIT:
  case DOT:
  case ARRAY:
    depth = 0;
    break;

  case ATOM:
  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
  case EE:
  case AA:
  case EBF:
  case EBG:
  case ABF:
  case ABG:
  case BUNTIL:
  case MMIN:
  case MMAX:
  case APATH:
  case EPATH:
    internal_error( "Unexpected CTL operator, node type %d\n",
        node_get_type(ltl_wff) );

  default:
    /* no other cases are currently allowed */
    nusmv_assert(FALSE);
  }

  return depth;
}




/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

static node_ptr bmc_wff_expand_case(node_ptr wff) 
{
  node_ptr res;

  nusmv_assert(node_get_type(wff) == CASE);
  res = bmc_wff_expand_case_aux(wff);

  return res;
}


static node_ptr bmc_wff_expand_case_aux(node_ptr wff) 
{
  if (node_get_type(wff) == CASE) {
    node_ptr cur_cond  = car(car(wff));
    node_ptr cur_rslt  = cdr(car(wff));
    node_ptr case_rest = cdr(wff);
    node_ptr res;

    nusmv_assert(node_get_type(car(wff)) == COLON);

    res = Bmc_Wff_MkOr( Bmc_Wff_MkAnd(cur_cond, cur_rslt),
      Bmc_Wff_MkAnd(Bmc_Wff_MkNot(cur_cond),
              bmc_wff_expand_case_aux(case_rest))
         );
    return res;

  }
  else {
    nusmv_assert((node_get_type(wff) == TRUEEXP) ||
     (node_get_type(wff) == FALSEEXP));

    return wff;
  }
}


static node_ptr bmc_wff_mk_nnf(node_ptr wff, boolean pol)
{
  node_ptr res;
  
  /* if reached a Nil branch then end recursion with a Nil node */
  if (wff == Nil) return Nil;

  /* Only temporal operator X (node type OP_NEXT) is legal in LTL wffs.
     The operator NEXT used in model definition is trapped. */
  nusmv_assert(node_get_type(wff) != NEXT);

  res = bmc_wff2nnf_hash_lookup_entry(wff, pol);
  if (res != (node_ptr) NULL) return res;

  switch (node_get_type(wff)) {
  case TRUEEXP:                        
    if (pol) res = Bmc_Wff_MkTruth();
    else res = Bmc_Wff_MkFalsity();  /* !1 <-> 0 */
    break;

  case FALSEEXP:                       
    if (pol) res = Bmc_Wff_MkFalsity();
    else res = Bmc_Wff_MkTruth();    /* !0 <-> 1 */
    break;

  case NOT:                            
    /* !(a) <-> (!a) */
    /* !(!a) <-> a */
    res = bmc_wff_mk_nnf(car(wff), !pol);
    break;

  case AND:                            
    if (pol) res = Bmc_Wff_MkAnd(bmc_wff_mk_nnf(car(wff), true),
				 bmc_wff_mk_nnf(cdr(wff), true));
    else { 
      /* !(a & b) <-> (!a | !b) */
      res = Bmc_Wff_MkOr(bmc_wff_mk_nnf(car(wff), false),
			 bmc_wff_mk_nnf(cdr(wff), false));
    }
    break;
    
  case OR:
    if (pol) res = Bmc_Wff_MkOr(bmc_wff_mk_nnf(car(wff), true),
				bmc_wff_mk_nnf(cdr(wff), true));
    else {
      /* !(a | b) <-> (!a & !b) */
      res = Bmc_Wff_MkAnd(bmc_wff_mk_nnf(car(wff), false),
			  bmc_wff_mk_nnf(cdr(wff), false));
    }
    break;

  case IMPLIES:                        
    if (pol) {
      /* (a -> b) <-> !(a & !b) <-> (!a | b) */
      res = Bmc_Wff_MkOr(bmc_wff_mk_nnf(car(wff), false),
			 bmc_wff_mk_nnf(cdr(wff), true));
    }
    else {
      /* !(a -> b) <-> (a & !b) */
      res = Bmc_Wff_MkAnd(bmc_wff_mk_nnf(car(wff), true),
			  bmc_wff_mk_nnf(cdr(wff), false));
    }
    break;

  case IFF:                          
    if (pol) {
      /* (a <-> b) <->
	 !(a & !b) & !(b & !a) <->
	 (!a | b) & (!b | a) */
      res = Bmc_Wff_MkAnd( Bmc_Wff_MkOr(bmc_wff_mk_nnf(car(wff), false),
					bmc_wff_mk_nnf(cdr(wff), true)),
			   Bmc_Wff_MkOr(bmc_wff_mk_nnf(car(wff), true),
					bmc_wff_mk_nnf(cdr(wff), false)) );
    }
    else {
      /* !(a <-> b) <->
	 !(!(a & !b) & !(b & !a)) <->
	 (a & !b) | (b & !a) */
      res = Bmc_Wff_MkOr( Bmc_Wff_MkAnd(bmc_wff_mk_nnf(car(wff), true),
					bmc_wff_mk_nnf(cdr(wff), false)),
			  Bmc_Wff_MkAnd(bmc_wff_mk_nnf(car(wff), false),
					bmc_wff_mk_nnf(cdr(wff), true)) );
    }
    break;

  case XOR:
    if (pol) {
      /* (a xor b) <-> (a & !b) | (!a & b) */
      res = Bmc_Wff_MkOr( Bmc_Wff_MkAnd(bmc_wff_mk_nnf(car(wff), true),
					bmc_wff_mk_nnf(cdr(wff), false)),
			  Bmc_Wff_MkAnd(bmc_wff_mk_nnf(car(wff), false),
					bmc_wff_mk_nnf(cdr(wff), true)) );
    }
    else {
      /* !(a xnor b) <-> (a | !b) & (!a | b) */
      res = Bmc_Wff_MkAnd( Bmc_Wff_MkOr(bmc_wff_mk_nnf(car(wff), true),
					bmc_wff_mk_nnf(cdr(wff), false)),
			   Bmc_Wff_MkOr(bmc_wff_mk_nnf(car(wff), false),
					bmc_wff_mk_nnf(cdr(wff), true)) );
    }
    break;
      
  case XNOR:
    if (pol) {
      /* (a xnor b) <-> (!a | b) & (!b | a) */
      res = Bmc_Wff_MkAnd( Bmc_Wff_MkOr(bmc_wff_mk_nnf(car(wff), false),
                                        bmc_wff_mk_nnf(cdr(wff), true)),
                           Bmc_Wff_MkOr(bmc_wff_mk_nnf(car(wff), true),
                                        bmc_wff_mk_nnf(cdr(wff), false)) );
    }
    else {
      /* !(a xnor b) <-> (a & !b) | (!a & b) */
      res = Bmc_Wff_MkOr( Bmc_Wff_MkAnd(bmc_wff_mk_nnf(car(wff), true),
					bmc_wff_mk_nnf(cdr(wff), false)),
			  Bmc_Wff_MkAnd(bmc_wff_mk_nnf(car(wff), false),
					bmc_wff_mk_nnf(cdr(wff), true)) );
    }
    break;

  case OP_NEXT:     
    /* !X(a) <-> X(!a) */
    res = Bmc_Wff_MkOpNext(bmc_wff_mk_nnf(car(wff), pol));
    break;

  case OP_PREC:    
    /* !Y(a) <-> Z(!a) */
    if (pol) res = Bmc_Wff_MkOpPrec(bmc_wff_mk_nnf(car(wff), pol));
    else res = Bmc_Wff_MkOpNotPrecNot(bmc_wff_mk_nnf(car(wff), pol));
    break;

  case OP_NOTPRECNOT:
    /* !Z(a) <-> Y(!a) */
    if (pol) res = Bmc_Wff_MkOpNotPrecNot(bmc_wff_mk_nnf(car(wff), pol));
    else res = Bmc_Wff_MkOpPrec(bmc_wff_mk_nnf(car(wff), pol));
    break;

  case OP_GLOBAL:   
    if (pol) res = Bmc_Wff_MkGlobally(bmc_wff_mk_nnf(car(wff), pol));
    else {
      /* !G(a) <-> F(!a) */
      res = Bmc_Wff_MkEventually(bmc_wff_mk_nnf(car(wff), pol));
    }
    break;

  case OP_HISTORICAL:   
    if (pol) res = Bmc_Wff_MkHistorically(bmc_wff_mk_nnf(car(wff), pol));
    else {
      /* !H(a) <-> O(!a) */
      res = Bmc_Wff_MkOnce(bmc_wff_mk_nnf(car(wff), pol));
    }
    break;

  case OP_FUTURE:               
    if (pol) res = Bmc_Wff_MkEventually(bmc_wff_mk_nnf(car(wff), pol));
    else {
      /* !F(a) <-> G(!a) */
      res = Bmc_Wff_MkGlobally(bmc_wff_mk_nnf(car(wff), pol));
    }
    break;

  case OP_ONCE:                       
    if (pol) res = Bmc_Wff_MkOnce(bmc_wff_mk_nnf(car(wff), pol));
    else {
      /* !O(a) <-> H(!a) */
      res = Bmc_Wff_MkHistorically(bmc_wff_mk_nnf(car(wff), pol));
    }
    break;

  case UNTIL:                          
    if (pol) res = Bmc_Wff_MkUntil(bmc_wff_mk_nnf(car(wff), pol),
				   bmc_wff_mk_nnf(cdr(wff), pol));
    else {
      /* !(a U b) <-> (!a V !b) */
      res = Bmc_Wff_MkReleases(bmc_wff_mk_nnf(car(wff), pol),
			       bmc_wff_mk_nnf(cdr(wff), pol));
    }
    break;

  case SINCE: 
    if (pol) res = Bmc_Wff_MkSince(bmc_wff_mk_nnf(car(wff), pol),
				   bmc_wff_mk_nnf(cdr(wff), pol));
    else {
      /* !(a S b) <-> (!a T !b) */
      res = Bmc_Wff_MkTriggered(bmc_wff_mk_nnf(car(wff), pol),
				bmc_wff_mk_nnf(cdr(wff), pol));
    }
    break;

  case RELEASES: 
    if (pol) res = Bmc_Wff_MkReleases(bmc_wff_mk_nnf(car(wff), pol),
				      bmc_wff_mk_nnf(cdr(wff), pol));
    else {
      /* !(a V b) <-> (!a U !b) */
      res = Bmc_Wff_MkUntil(bmc_wff_mk_nnf(car(wff), pol),
			    bmc_wff_mk_nnf(cdr(wff), pol));
    }
    break;

  case TRIGGERED:                      
    if (pol) res = Bmc_Wff_MkTriggered(bmc_wff_mk_nnf(car(wff), pol),
				       bmc_wff_mk_nnf(cdr(wff), pol));
    else {
      /* !(a T b) <-> (!a S !b) */
      res = Bmc_Wff_MkSince(bmc_wff_mk_nnf(car(wff), pol),
			    bmc_wff_mk_nnf(cdr(wff), pol));
    }
    break;

  case CASE:
    {
      node_ptr nocase_wff = bmc_wff_expand_case(wff);
      res = bmc_wff_mk_nnf(nocase_wff, pol);
      break;
    }

  case BIT:
  case DOT: 
  case ARRAY:  
    /* it is a bexp var */
    if (pol) res = wff;
    else res = Bmc_Wff_MkNot(wff);
    break;

  case NUMBER: 
  case ATOM:
    /* internal format for atoms that should have been previously
     hidden within DOT and ARRAY */
    internal_error("bmc_wff_mk_nnf: unexpected node %s\n",
		   (node_get_type(wff) == NUMBER) ? "NUMBER" : "ATOM");
    res = (node_ptr) NULL;
    break;

  default: 
    internal_error("bmc_wff_mk_nnf: unexpected TOKEN %d\n",
		   node_get_type(wff));
    
  }

  if (res != (node_ptr) NULL) {
    bmc_wff2nnf_hash_insert_entry(wff, pol, res);
  }

  return res;  
}



/**Function********************************************************************

  Synopsis           [Makes a <b>binary</b> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
static node_ptr bmc_wff_mkBinary(int type, node_ptr arg1, node_ptr arg2)
{
  return find_node(type, arg1, arg2);
}


/**Function********************************************************************

  Synopsis           [Makes a <b>unary</b> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
static node_ptr bmc_wff_mkUnary(int type, node_ptr arg)
{
  return find_node(type, arg, Nil);
}

/**Function********************************************************************

  Synopsis           [Makes a <b>constant</b> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
static node_ptr bmc_wff_mkConst(int type)
{
  return find_node(type, Nil, Nil);
}

