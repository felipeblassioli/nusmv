/**CFile***********************************************************************

  FileName    [bmcTableauLTLformula.c]

  PackageName [bmc]

  Synopsis    [Bmc.Tableau module]

  Description [This module contains all the operations related to the
               construction of tableaux for LTL formulas]

  SeeAlso     [bmcGen.c, bmcModel.c, bmcConv.c, bmcVarMgr.c]

  Author      [Roberto Cavada]

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

#include "bmcInt.h"
#include "bmcUtils.h"
#include "bmcModel.h"

#include "parser/symbols.h"
#include "utils/error.h"


static char rcsid[] UTIL_UNUSED = "$Id: bmcTableauLTLformula.c,v 1.3.4.5.2.2 2004/07/28 15:33:03 nusmv Exp $";

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

static be_ptr
bmc_tableauGetUntilAtTime_aux(const BmcVarsMgr_ptr vars_mgr,
			      const node_ptr p, const node_ptr q,
			      const int time, const int k, const int l,
			      const int steps);

static be_ptr
bmc_tableauGetReleasesAtTime_aux(const BmcVarsMgr_ptr vars_mgr,
				 const node_ptr p, const node_ptr q,
				 const int time, const int k, const int l,
				 const int steps);


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Given a wff expressed in ltl builds the model-independent
  tableau at 'time' of a path formula bounded by \[k, l\]]

  Description        [This function is the entry point of a mutual recursive
  calling stack. All logical connectives are resolved, excepted for NOT, which
  closes the recursive calling stack. Also variables and falsity/truth
  constants close the recursion.]

  SideEffects        []

  SeeAlso            [bmc_tableauGetNextAtTime,
  bmc_tableauGetGloballyAtTime, bmc_tableauGetEventuallyAtTime,
  bmc_tableauGetUntilAtTime, bmc_tableauGetReleasesAtTime]

******************************************************************************/
be_ptr
BmcInt_Tableau_GetAtTime(const BmcVarsMgr_ptr vars_mgr, const node_ptr ltl_wff,
			 const int time, const int k, const int l)
{
  be_ptr result=NULL;

  /* checks out the validity of [l, k] only if a loop exists: */
  nusmv_assert(Bmc_Utils_IsNoLoopback(l) || (k > l));
  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );


  switch (node_get_type(ltl_wff)) {
  case TRUEEXP:
    result = Be_Truth(BmcVarsMgr_GetBeMgr(vars_mgr));
    break;

  case FALSEEXP:
    result = Be_Falsity(BmcVarsMgr_GetBeMgr(vars_mgr));
    break;

  case BIT:
  case DOT:
    if (time == k) {
      int var_idx = BmcVarsMgr_Name2VarIndex(vars_mgr, ltl_wff);
      if (BmcVarsMgr_IsBeIndexInInputBlock(vars_mgr, var_idx)) {
	/* input vars when time == max_time evaluate to false: */
	result = Be_Falsity(BmcVarsMgr_GetBeMgr(vars_mgr));
	break;
      }
    }

    result = BmcVarsMgr_Name2Timed(vars_mgr, ltl_wff, time, k);
    break;

  case ARRAY:
    if (!Encoding_is_symbol_declared(BmcVarsMgr_GetSymbEncoding(vars_mgr),
				     ltl_wff)) {
      internal_error("Unexpected array node\n");      
    }

    if (!Encoding_is_symbol_boolean_var(BmcVarsMgr_GetSymbEncoding(vars_mgr), 
					ltl_wff)) {
      fprintf(nusmv_stderr, "Detected scalar array variable '");
      print_node(nusmv_stderr, ltl_wff);
      fprintf(nusmv_stderr, "'");
      internal_error("Scalar array variable has been found where a boolean "
		     "variable had to be used instead.\n"
		     "This might be due to a bug on your model.");
    }

    if (time == k) {
      int var_idx = BmcVarsMgr_Name2VarIndex(vars_mgr, ltl_wff);
      if (BmcVarsMgr_IsBeIndexInInputBlock(vars_mgr, var_idx)) {
	/* input vars when time == max_time evaluate to false: */
	result = Be_Falsity(BmcVarsMgr_GetBeMgr(vars_mgr));
	break;
      }
    }

    result = BmcVarsMgr_Name2Timed(vars_mgr, ltl_wff, time, k);
    break;
    

  case NOT:
    /* checks out that argument of NOT operator is actually a variable: */
    nusmv_assert( node_get_type(car(ltl_wff)) == DOT ||
		  node_get_type(car(ltl_wff)) == BIT || 
		  node_get_type(car(ltl_wff)) == ARRAY);
    
    if (!Encoding_is_symbol_declared(BmcVarsMgr_GetSymbEncoding(vars_mgr),
				     car(ltl_wff))) {
      internal_error("Unexpected scalar or undefined node\n");      
    }

    if ((node_get_type(car(ltl_wff)) == ARRAY) && 
	(!Encoding_is_symbol_boolean_var(
		 BmcVarsMgr_GetSymbEncoding(vars_mgr), car(ltl_wff)))) {
      fprintf(nusmv_stderr, "Detected scalar array variable '");
      print_node(nusmv_stderr, car(ltl_wff));
      fprintf(nusmv_stderr, "'");
      internal_error("Scalar array variable has been found where a boolean "
		     "variable had to be used instead.\n"
		     "This might be due to a bug on your model.");
    }

    if (time == k) { 
      int var_idx = BmcVarsMgr_Name2VarIndex(vars_mgr, car(ltl_wff));
      if (BmcVarsMgr_IsBeIndexInInputBlock(vars_mgr, var_idx)) {
	/* input vars when time == max_time evaluate to false: */
	result = Be_Falsity(BmcVarsMgr_GetBeMgr(vars_mgr));
	break;
      }
    }

    result = Be_Not(BmcVarsMgr_GetBeMgr(vars_mgr), 
		    BmcVarsMgr_Name2Timed(vars_mgr, car(ltl_wff), time, k));
    break;

  case AND:
    result = Be_And(BmcVarsMgr_GetBeMgr(vars_mgr),
		    BmcInt_Tableau_GetAtTime(vars_mgr,car(ltl_wff),time,k,l),
		    BmcInt_Tableau_GetAtTime(vars_mgr,cdr(ltl_wff),time,k,l));
    break;

  case OR:
    result = Be_Or(BmcVarsMgr_GetBeMgr(vars_mgr),
		   BmcInt_Tableau_GetAtTime(vars_mgr,car(ltl_wff),time,k,l),
		   BmcInt_Tableau_GetAtTime(vars_mgr,cdr(ltl_wff),time,k,l));
    break;

  case IFF:
    result = Be_Iff(BmcVarsMgr_GetBeMgr(vars_mgr),
		    BmcInt_Tableau_GetAtTime(vars_mgr,car(ltl_wff),time,k,l),
		    BmcInt_Tableau_GetAtTime(vars_mgr,cdr(ltl_wff),time,k,l));
    break;

  case OP_NEXT:
    result = bmc_tableauGetNextAtTime(vars_mgr, car(ltl_wff), time, k, l);
    break;

  case OP_GLOBAL:
    result = bmc_tableauGetGloballyAtTime(vars_mgr, car(ltl_wff), time, k, l);
    break;

  case OP_FUTURE: /* EVENTUALLY */
    result = bmc_tableauGetEventuallyAtTime(vars_mgr, car(ltl_wff), time, k, l);
              
    break;

  case UNTIL:
    result = bmc_tableauGetUntilAtTime(vars_mgr, car(ltl_wff), cdr(ltl_wff),
				       time, k, l);
    break;

  case RELEASES:
    result = bmc_tableauGetReleasesAtTime(vars_mgr, car(ltl_wff), cdr(ltl_wff),
					  time, k, l );
    break;

  case IMPLIES:
    internal_error("'Implies' should had been nnf-ed away!\n");

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
    /* no other type are available here: */
    nusmv_assert(false);
  }

  nusmv_assert(result != NULL); /*it must be assigned! */
  return result;
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Resolves the NEXT operator, building the tableau for
  its argument]

  Description        [Returns a falsity constants if the next operator leads
  out of \[l, k\] and there is no loop]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr
bmc_tableauGetNextAtTime(const BmcVarsMgr_ptr vars_mgr, const node_ptr ltl_wff,
			 const int time, const int k, const int l)
{
  int succtime;
  be_ptr tableau;

  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );

  /* checks out the validity of [l, k] only if a loop exists: */
  nusmv_assert(Bmc_Utils_IsNoLoopback(l) || (k > l) );

  succtime = Bmc_Utils_GetSuccTime(time, k, l);

  if (!Bmc_Utils_IsNoLoopback(succtime)) {
    tableau = BmcInt_Tableau_GetAtTime(vars_mgr, ltl_wff, succtime, k, l);
  }
  else {
    tableau = Be_Falsity(BmcVarsMgr_GetBeMgr(vars_mgr));
  }

  return tableau;
}


/**Function********************************************************************

  Synopsis           [Resolves the future operator, and builds a conjunctive
  expression of tableaus, by iterating intime up to k in a different manner
  depending on the \[l, k\] interval form]

  Description        [ltl_wff is the 'p' part in 'F p'.
  If intime<=k is out of \[l, k\] or if there is no loop,
  iterates from intime to k, otherwise iterates from l to k]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr
bmc_tableauGetEventuallyAtTime(const BmcVarsMgr_ptr vars_mgr,
			       const node_ptr ltl_wff,
			       const int intime, const int k, const int l)
{
  int time;
  be_ptr tableau;
  int stop_time;
  int start_time;

  nusmv_assert((intime < k) || (intime==k && Bmc_Utils_IsNoLoopback(l)) );

  /* checks out the validity of [l, k] only if a loop exists: */
  nusmv_assert(Bmc_Utils_IsNoLoopback(l) || (k > l));

  tableau = Be_Falsity(BmcVarsMgr_GetBeMgr(vars_mgr));

  /* there exist three cases:
     1) no loop: iterates from k downto intime;
     2) loop, (intime < l): iterates from k-1 downto intime;
     3) loop, (l <= intime < k) : iterates from k-1 downto l */

  if (Bmc_Utils_IsNoLoopback(l)) {
    /* The first case */
    start_time = k;
    stop_time  = intime;
  }
  else {
    /* The second and third case*/
    start_time = k-1;
    stop_time  = min(intime,l);
  }

  for (time = start_time; time>=stop_time; --time) {
    /* lazy evaluation: */
    be_ptr tableau_at_time = BmcInt_Tableau_GetAtTime(vars_mgr, ltl_wff, 
						      time, k, l);
                   
    if ( Be_IsTrue(BmcVarsMgr_GetBeMgr(vars_mgr), tableau_at_time) ) {
      tableau = tableau_at_time;
      break;
    }
    tableau = Be_Or(BmcVarsMgr_GetBeMgr(vars_mgr),
           tableau_at_time, tableau);
  } /* loop */

  return tableau;
}

/**Function********************************************************************

  Synopsis           [As bmc_tableauGetEventuallyAtTime, but builds a
  conjunctioned expression in order to be able to assure a global constraint]

  Description        [ltl_wff is the 'p' part in 'G p']

  SideEffects        []

  SeeAlso            [bmc_tableauGetEventuallyAtTime]

******************************************************************************/
be_ptr
bmc_tableauGetGloballyAtTime(const BmcVarsMgr_ptr vars_mgr,
			     const node_ptr ltl_wff,
			     const int intime, const int k, const int l)
{
  int time;
  be_ptr tableau;
  int stop_time;

  nusmv_assert((intime < k) || (intime==k && Bmc_Utils_IsNoLoopback(l)) );

  /* checks out the validity of [l, k] only if a loop exists: */
  nusmv_assert(Bmc_Utils_IsNoLoopback(l) || (k > l));

  /* there exist three cases:
     1) no loop: cannot assure nothing, so return falsity;
     2) loop, (intime < l): iterates from intime to k-1;
     3) loop, (l <= intime < k) : iterates from intime to k-1, and then from
        l to intime-1 (so more efficiently from l to k-1.)  */
  if (Bmc_Utils_IsNoLoopback(l)) {
    tableau = Be_Falsity(BmcVarsMgr_GetBeMgr(vars_mgr));
  }
  else {
    /* second and third cases */
    tableau = Be_Truth(BmcVarsMgr_GetBeMgr(vars_mgr));

    stop_time = min(intime, l);
    for (time=k-1; time >= stop_time; --time) {
      /* lazy evaluation: */
      be_ptr tableau_at_time = BmcInt_Tableau_GetAtTime(vars_mgr, ltl_wff,
							time, k, l);
      if ( Be_IsFalse(BmcVarsMgr_GetBeMgr(vars_mgr), tableau_at_time) ) {
	tableau = tableau_at_time;
	break;
      }
      tableau = Be_And(BmcVarsMgr_GetBeMgr(vars_mgr),
		       tableau_at_time, tableau);
    }
  }

  return tableau; 
}



/**Function********************************************************************

  Synopsis           [Builds an expression which evaluates the until operator]

  Description        [Carries out the steps number to be performed, depending
  on l,k and time, then calls bmc_tableauGetUntilAtTime_aux]

  SideEffects        []

  SeeAlso            [bmc_tableauGetUntilAtTime_aux]

******************************************************************************/
be_ptr
bmc_tableauGetUntilAtTime(const BmcVarsMgr_ptr vars_mgr,
			  const node_ptr p, const node_ptr q,
			  const int time, const int k, const int l)
{
  int steps;

  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );

  if (Bmc_Utils_IsNoLoopback(l)) {
    steps = k - time + 1 ; /* no loop, interval [time, k] */
  }
  else {
    steps = (k-1) - min(time,l) + 1; /* loop, full round */
  }

  return bmc_tableauGetUntilAtTime_aux(vars_mgr, p, q, time, k, l, steps);
}


/**Function********************************************************************

  Synopsis           [Builds an expression which evaluates the release
  operator]

  Description        [Carries out the steps number to be performed, depending
  on l,k and time, then calls bmc_tableauGetReleasesAtTime_aux]

  SideEffects        []

  SeeAlso            [bmc_tableauGetReleasesAtTime_aux]

******************************************************************************/
be_ptr
bmc_tableauGetReleasesAtTime(const BmcVarsMgr_ptr vars_mgr,
			     const node_ptr p, const node_ptr q,
			     const int time, const int k, const int l)
{
  int steps;

  nusmv_assert (time <= k);

  if (Bmc_Utils_IsNoLoopback(l)) {
    steps = k - time + 1 ; /* no loop, interval [time, k] */
  }
  else {
    steps = (k-1) - min(time,l) + 1; /* loop, full round */
  }

  return bmc_tableauGetReleasesAtTime_aux(vars_mgr, p, q, time, k, l, steps);
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [auxiliary part of bmc_tableauGetUntilAtTime]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
bmc_tableauGetUntilAtTime_aux(const BmcVarsMgr_ptr vars_mgr,
			      const node_ptr p, const node_ptr q,
			      const int time, const int k, const int l,
			      const int steps)
{
  be_ptr tableau_temp; /* for increasing of performances */
  be_ptr tableau_following; /* to increase readability */

  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );
  nusmv_assert (steps >= 1);

  tableau_temp = BmcInt_Tableau_GetAtTime(vars_mgr, q, time, k, l);

  if (steps > 1) {
    tableau_following =
      bmc_tableauGetUntilAtTime_aux(vars_mgr, p, q,
				    Bmc_Utils_GetSuccTime(time, k, l),
				    k, l, steps - 1);

    tableau_temp =
      Be_Or( BmcVarsMgr_GetBeMgr(vars_mgr),
	     tableau_temp,
	     Be_And(BmcVarsMgr_GetBeMgr(vars_mgr),
		    BmcInt_Tableau_GetAtTime(vars_mgr, p, time, k, l),
		    tableau_following) );
  }
  return tableau_temp;
}




/**Function********************************************************************

  Synopsis           [auxiliary part of bmc_tableauGetReleasesAtTime]

  Description        [Builds the release operator expression]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
bmc_tableauGetReleasesAtTime_aux(const BmcVarsMgr_ptr vars_mgr,
				 const node_ptr p, const node_ptr q,
				 const int time, const int k, const int l,
				 const int steps)
{
  be_ptr tableau_p;
  be_ptr tableau_q;
  be_ptr tableau_result;

  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );
  nusmv_assert (steps >= 1);

  tableau_p = BmcInt_Tableau_GetAtTime(vars_mgr, p, time, k, l);
  tableau_q = BmcInt_Tableau_GetAtTime(vars_mgr, q, time, k, l);

  if (steps == 1) {
    if (Bmc_Utils_IsNoLoopback(l)) { /* q & p */
      tableau_result = Be_And(BmcVarsMgr_GetBeMgr(vars_mgr), 
			      tableau_p, tableau_q);
    } else { /* q */
      tableau_result = tableau_q;
    }
  }
  else { /* q & ( p | X(p R q) ) */
    be_ptr tableau_following =
      bmc_tableauGetReleasesAtTime_aux(vars_mgr, p, q,
				       Bmc_Utils_GetSuccTime(time, k, l),
				       k, l, steps - 1);
    tableau_result =
      Be_And(BmcVarsMgr_GetBeMgr(vars_mgr),
	     tableau_q,
	     Be_Or(BmcVarsMgr_GetBeMgr(vars_mgr),
		   tableau_p, tableau_following));
  }

  return tableau_result;
}


