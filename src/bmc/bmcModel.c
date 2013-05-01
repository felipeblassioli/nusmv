/**CFile***********************************************************************

  FileName    [bmcModel.c]

  PackageName [bmc]

  Synopsis    [Bmc.Model module]

  Description [This module contains all the model-related operations]

  SeeAlso     [bmcGen.c, bmcTableau.c, bmcConv.c, bmcVarMgr.c]

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

#include "bmcModel.h"
#include "bmcUtils.h"


static char rcsid[] UTIL_UNUSED = "$Id: bmcModel.c,v 1.30.2.1.2.1 2004/07/27 12:12:12 nusmv Exp $";

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
bmc_model_getSingleFairness ARGS((const BmcVarsMgr_ptr vars_mgr,
				  const be_ptr one_fairness,
				  const int k, const int l));


static be_ptr
bmc_model_getFairness_aux ARGS((const BmcVarsMgr_ptr vars_mgr,
				const node_ptr list,
				const int k, const int l));


/**AutomaticEnd***************************************************************/



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Retrieves the init states from the given fsm, and
  compiles them into a BE at time 0]

  Description        [Use this function instead of explicitly get the init
  from the fsm and shift them at time 0 using the vars manager layer.]

  SideEffects        []

  SeeAlso            [Bmc_Model_GetInvarAtTime]

******************************************************************************/
be_ptr Bmc_Model_GetInit0(const Bmc_Fsm_ptr be_fsm)
{
  BmcVarsMgr_ptr vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);
  Be_Manager_ptr be_mgr = BmcVarsMgr_GetBeMgr(vars_mgr);
  be_ptr init0 = BmcVarsMgr_ShiftCurrNext2Time(vars_mgr,
			       Be_And(be_mgr, Bmc_Fsm_GetInit(be_fsm),
				      Bmc_Fsm_GetInvar(be_fsm)),
			       0);
  return init0;
}


/**Function********************************************************************

  Synopsis           [Retrieves the invars from the given fsm, and
  compiles them into a BE at the given time]

  Description        [Use this function instead of explicitly get the invar
  from the fsm and shift them at the requested time using the vars
  manager layer.]

  SideEffects        []

  SeeAlso            [Bmc_Model_GetInit0]

******************************************************************************/
be_ptr Bmc_Model_GetInvarAtTime(const Bmc_Fsm_ptr be_fsm, const int time)
{
  return  BmcVarsMgr_ShiftCurrNext2Time(Bmc_Fsm_GetVarsManager(be_fsm),
					 Bmc_Fsm_GetInvar(be_fsm),
					 time);
}



/**Function********************************************************************

  Synopsis           [Unrolls the transition relation from j to k, taking
  into account of invars]

  Description        [Using of invars over next variables instead of the
  previuos variables is a specific implementation aspect]

  SideEffects        []

  SeeAlso            [Bmc_Model_GetPathWithInit, Bmc_Model_GetPathNoInit]

******************************************************************************/
be_ptr
Bmc_Model_GetUnrolling(const Bmc_Fsm_ptr be_fsm, const int j, const int k)
{
  BmcVarsMgr_ptr vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);
  Be_Manager_ptr be_mgr = BmcVarsMgr_GetBeMgr(vars_mgr);
  be_ptr invar = Bmc_Fsm_GetInvar(be_fsm);
  
  be_ptr trans_invar_j = Be_And(be_mgr, Bmc_Fsm_GetTrans(be_fsm), invar);
  be_ptr invar_next = BmcVarsMgr_ShiftCurrState2Next(vars_mgr, invar);						  
  be_ptr trans_invar = Be_And(be_mgr, trans_invar_j, invar_next);

  return BmcVarsMgr_MkAndCurrNextInterval(vars_mgr, trans_invar, j, k - 1);
}


/**Function********************************************************************

  Synopsis           [Returns the path for the model from 0 to k,
  taking into account the invariants (and no init)]

  Description        []

  SideEffects        []

  SeeAlso            [Bmc_Model_GetPathWithInit]

******************************************************************************/
be_ptr Bmc_Model_GetPathNoInit(const Bmc_Fsm_ptr be_fsm, const int k)
{
  return Bmc_Model_GetUnrolling(be_fsm, 0, k);
}


/**Function********************************************************************

  Synopsis           [Returns the path for the model from 0 to k,
  taking into account initial conditions and invariants]

  Description        []

  SideEffects        []

  SeeAlso            [Bmc_Model_GetPathNoInit]

******************************************************************************/
be_ptr Bmc_Model_GetPathWithInit(const Bmc_Fsm_ptr be_fsm, const int k)
{
  return Be_And(BmcVarsMgr_GetBeMgr(Bmc_Fsm_GetVarsManager(be_fsm)),
		Bmc_Model_GetPathNoInit(be_fsm, k),
		Bmc_Model_GetInit0(be_fsm));
}


/**Function********************************************************************

  Synopsis           [Generates and returns an expression representing
  all fairnesses in a conjunctioned form]

  Description        [Uses bmc_model_getFairness_aux which recursively calls
  itself to conjuctive all fairnesses by constructing a top-level 'and'
  operation.
  Moreover bmc_model_getFairness_aux calls the recursive function
  bmc_model_getSingleFairness, which resolves a single fairness as
  a disjunctioned expression in which each ORed element is a shifting of
  the single fairness across \[l, k\] if a loop exists.
  If no loop exists, nothing can be issued, so a falsity value is returned]

  SideEffects        []

  SeeAlso            [bmc_model_getFairness_aux, bmc_model_getSingleFairness]

******************************************************************************/
be_ptr Bmc_Model_GetFairness(const Bmc_Fsm_ptr be_fsm, 
			     const int k, const int l)
{
  node_ptr list = Bmc_Fsm_GetListOfFairness(be_fsm);
  return bmc_model_getFairness_aux(Bmc_Fsm_GetVarsManager(be_fsm), list, k, l);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

static be_ptr bmc_model_getFairness_aux(const BmcVarsMgr_ptr vars_mgr,
					const node_ptr list,
					const int k, const int l)
{
  be_ptr res = NULL;
  Be_Manager_ptr be_mgr = BmcVarsMgr_GetBeMgr(vars_mgr);

  if (list == LS_NIL)  res = Be_Truth(be_mgr);
  else if (Bmc_Utils_IsNoLoopback(l))  res = Be_Falsity(be_mgr);
  else {
    be_ptr singleFairness =
      bmc_model_getSingleFairness(vars_mgr, (be_ptr)car(list), k, l);

    res = Be_And( be_mgr, singleFairness,
		  bmc_model_getFairness_aux(vars_mgr, cdr(list), k, l) );
  }

  return res;
}


static be_ptr
bmc_model_getSingleFairness(const BmcVarsMgr_ptr vars_mgr,
			    const be_ptr one_fairness, 
			    const int k, const int l)
{
  nusmv_assert(l < k);
  nusmv_assert(Bmc_Utils_IsSingleLoopback(l));

  return BmcVarsMgr_MkOrCurrNextInterval(vars_mgr, one_fairness, l, k-1);
}


