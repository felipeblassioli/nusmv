/**CFile***********************************************************************

  FileName    [bmcGen.c]

  PackageName [bmc]

  Synopsis    [Bmc.Gen module]

  Description [This module contains all the problems generation functions]

  SeeAlso     [bmcBmc.c, bmcTableau.c, bmcModel.c]

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

#include "bmcGen.h"
#include "bmcInt.h"
#include "bmcModel.h"
#include "bmcTableau.h"
#include "bmcWff.h"
#include "bmcUtils.h"
#include "bmcConv.h"

static char rcsid[] UTIL_UNUSED = "$Id: bmcGen.c,v 1.3.4.1.2.1 2004/07/27 12:12:12 nusmv Exp $";

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

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Builds and returns the invariant problem of the
  given propositional formula]

  Description        [Builds the negation of
                     (I0 imp P0) and ((P0 and R01) imp P1)
		     that must be unsatisfiable.]

  SideEffects        []

  SeeAlso            [Bmc_Gen_InvarBaseStep, Bmc_Gen_InvarInductStep]

******************************************************************************/
be_ptr Bmc_Gen_InvarProblem(const Bmc_Fsm_ptr be_fsm, const node_ptr wff)
{
  BmcVarsMgr_ptr vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);
  be_ptr base = Bmc_Gen_InvarBaseStep(be_fsm, wff);
  be_ptr induct = Bmc_Gen_InvarInductStep(be_fsm, wff);
  be_ptr res = Be_Not( BmcVarsMgr_GetBeMgr(vars_mgr),
		       Be_And(BmcVarsMgr_GetBeMgr(vars_mgr), base, induct) );
  
  return res;
}



/**Function********************************************************************

  Synopsis           [Returns the LTL problem at length k with loopback l
  (single loop, no loop and all loopbacks are allowed)]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Gen_LtlProblem(const Bmc_Fsm_ptr be_fsm,
			  const node_ptr ltl_wff,
			  const int k, const int l)
{
  Be_Manager_ptr be_mgr = BmcVarsMgr_GetBeMgr(Bmc_Fsm_GetVarsManager(be_fsm));
  be_ptr path_k = Bmc_Model_GetPathWithInit(be_fsm, k);
  be_ptr tableau = Bmc_Tableau_GetLtlTableau(be_fsm, ltl_wff, k, l);
  be_ptr res = Be_And(be_mgr, path_k, tableau);
  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the base step of the invariant construction]

  Description        [Returns I0 -> P0, where I0 is the init and
  invar at time 0, and P0 is the given formula at time 0]

  SideEffects        []

  SeeAlso            [Bmc_Gen_InvarInductStep]

******************************************************************************/
be_ptr Bmc_Gen_InvarBaseStep(const Bmc_Fsm_ptr be_fsm, const node_ptr wff)
{
  BmcVarsMgr_ptr vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);
  Be_Manager_ptr be_mgr = BmcVarsMgr_GetBeMgr(vars_mgr);

  be_ptr P_0 = BmcVarsMgr_ShiftCurrNext2Time(vars_mgr,
					      Bmc_Conv_Bexp2Be(vars_mgr, wff), 0);

  return Be_Implies( be_mgr, Be_And(be_mgr,
				    Bmc_Model_GetInit0(be_fsm),
				    Bmc_Model_GetInvarAtTime(be_fsm, 0)),
		     P_0 );
}


/**Function********************************************************************

  Synopsis           [Returns the induction step of the invariant construction]

  Description        [Returns (P0 and R01) -> P1, where P0 is the formula
  at time 0, R01 is the transition (without init) from time 0 to 1,
  and P1 is the formula at time 1]

  SideEffects        []

  SeeAlso            [Bmc_Gen_InvarBaseStep]

******************************************************************************/
be_ptr Bmc_Gen_InvarInductStep(const Bmc_Fsm_ptr be_fsm,
			       const node_ptr wff)
{
  BmcVarsMgr_ptr vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);
  Be_Manager_ptr be_mgr = BmcVarsMgr_GetBeMgr(vars_mgr);

  be_ptr P = Bmc_Conv_Bexp2Be(vars_mgr, wff);
  
  be_ptr trans_01_invar_01 = Bmc_Model_GetPathNoInit(be_fsm, 1);

  be_ptr trans_01_invar_01_P0 = 
    Be_And(be_mgr,
	   trans_01_invar_01,
	   BmcVarsMgr_ShiftCurrNext2Time(vars_mgr, P, 0));

  return Be_Implies(be_mgr, trans_01_invar_01_P0,
		    BmcVarsMgr_ShiftCurrNext2Time(vars_mgr, P, 1));
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

