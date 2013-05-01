/**CFile***********************************************************************

  FileName    [bmcFsm.c]

  PackageName [bmc]

  Synopsis    [Members implementation of class Bmc_Fsm]

  Description []

  SeeAlso     []

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


#include "bmcFsm.h"
#include "bmcCheck.h"
#include "bmcConv.h"

#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: bmcFsm.c,v 1.4.4.3.2.1 2004/07/27 12:12:11 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct*********************************************************************

  Synopsis           [Represents a Finite State Machine which 
  collects the whole model in Boolean Expression format]
  Description        [This class definition is private. Make reference to 
  the Bmc_Fsm_ptr type in order to handle a valid instance of this class.]
  SideEffects        []
  SeeAlso            []

******************************************************************************/
typedef struct Bmc_Fsm_TAG {
  be_ptr init;
  be_ptr invar;
  be_ptr trans;
  node_ptr fairness_list;
  BmcVarsMgr_ptr vars_manager; 
} Bmc_Fsm;


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
static void Bmc_Fsm_SetInit     ARGS((Bmc_Fsm_ptr self, be_ptr init)); 
static void Bmc_Fsm_SetInvar    ARGS((Bmc_Fsm_ptr self, be_ptr invar)); 
static void Bmc_Fsm_SetTrans    ARGS((Bmc_Fsm_ptr self, be_ptr trans)); 
static void Bmc_Fsm_SetFairness ARGS((Bmc_Fsm_ptr self, node_ptr fair)); 


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Class Bmc_Fsm constructor]

  Description        [Creates a new instance of the Bmc_Fsm class, getting 
  information from an instance of Fsm_Sexp type.]

  SideEffects        []

  SeeAlso            [Bmc_Fsm_Create, Bmc_Fsm_Delete]

******************************************************************************/
Bmc_Fsm_ptr Bmc_Fsm_CreateFromSexprFSM(BmcVarsMgr_ptr vars_manager, 
				       const SexpFsm_ptr bfsm)
{
  node_ptr list_of_fairness = SexpFsm_get_justice(bfsm);
  node_ptr list_of_valid_fairness = 
    Bmc_CheckFairnessListForPropositionalFormulae(list_of_fairness);
  
  Bmc_Fsm_ptr fsm = 
    Bmc_Fsm_Create(vars_manager, 
		   Bmc_Conv_Bexp2Be(vars_manager, SexpFsm_get_init(bfsm)),
		   Bmc_Conv_Bexp2Be(vars_manager, SexpFsm_get_invar(bfsm)),
		   Bmc_Conv_Bexp2Be(vars_manager, SexpFsm_get_trans(bfsm)),
		   Bmc_Conv_BexpList2BeList(vars_manager, 
					    list_of_valid_fairness));
  
  free_list(list_of_valid_fairness);
  return fsm;
}


/**Function********************************************************************

  Synopsis           [Class Bmc-Fsm constructor]

  Description        [It gets init, invar, transition relation and the list
  of fairness in Boolean Expression format.]

  SideEffects        []

  SeeAlso            [Bmc_Fsm_Delete]

******************************************************************************/
Bmc_Fsm_ptr Bmc_Fsm_Create(BmcVarsMgr_ptr vars_manager, 
			   const be_ptr init, 
			   const be_ptr invar,
			   const be_ptr trans, 
			   const node_ptr list_of_be_fairness)
{
  Bmc_Fsm_ptr fsm = ALLOC(Bmc_Fsm, 1);
  
  if (fsm == NULL) {
    internal_error("Bmc_Fsm_Create: Unable to allocate memory.\n");
  }

  Bmc_Fsm_SetInit(fsm, init);
  Bmc_Fsm_SetInvar(fsm, invar);
  Bmc_Fsm_SetTrans(fsm, trans);
  Bmc_Fsm_SetFairness(fsm, list_of_be_fairness);
  fsm->vars_manager = vars_manager;

  return fsm;
}


/**Function********************************************************************

  Synopsis           [Class Bmc_Fsm destructor]

  Description        []

  SideEffects        [self will be invalidated]

  SeeAlso            [Bmc_Fsm_Create, Bmc_Fsm_CreateFromSexprFSM]

******************************************************************************/
void Bmc_Fsm_Delete(Bmc_Fsm_ptr* self_ref)
{
  nusmv_assert((*self_ref) != NULL);
  FREE(*self_ref); *self_ref = NULL;
}


/**Function********************************************************************

  Synopsis           [Copy constructor for class Bmc_Fsm]

  Description        [Creates a new independent copy of the given fsm instance.
  You must destroy the returned class instance by invoking the class 
  destructor when you no longer need it.]

  SideEffects        []

  SeeAlso            [Bmc_Fsm_Delete]

******************************************************************************/
Bmc_Fsm_ptr Bmc_Fsm_Dup(Bmc_Fsm_ptr fsm)
{
  /* Necessary since the master in BE is built only after bmc_setup */
  Bmc_Fsm_ptr self = Bmc_Fsm_Create(Bmc_Fsm_GetVarsManager(fsm), 
				    Bmc_Fsm_GetInit(fsm), 
				    Bmc_Fsm_GetInvar(fsm), 
				    Bmc_Fsm_GetTrans(fsm),
				    Bmc_Fsm_GetListOfFairness(fsm));
  return self;
}


/**Function********************************************************************

  Synopsis           [Returns the initial states stored in BE format into the
  given fsm instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Fsm_GetInit(const Bmc_Fsm_ptr self)
{
  return self->init;
}


/**Function********************************************************************

  Synopsis           [Returns the invariants stored in BE format into the
  given fsm instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Fsm_GetInvar(const Bmc_Fsm_ptr self)
{
  return self->invar;
}


/**Function********************************************************************

  Synopsis           [Returns the transition relation stored in BE format 
  into the given fsm instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Fsm_GetTrans(const Bmc_Fsm_ptr self)
{
  return self->trans;
}


/**Function********************************************************************

  Synopsis           [Returns the list of fairness stored in BE format 
  into the given fsm instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Fsm_GetListOfFairness(const Bmc_Fsm_ptr self)
{
  return self->fairness_list;
}


/**Function********************************************************************

  Synopsis           [Returns the variables manager associated with the 
  given fsm instance]

  Description        []

  SideEffects        []

  SeeAlso            [Vars_Mgr]

******************************************************************************/
BmcVarsMgr_ptr Bmc_Fsm_GetVarsManager(const Bmc_Fsm_ptr self)
{
  return self->vars_manager;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Private service to set init states into the given fsm]

  Description        []

  SideEffects        [fsm will change]

  SeeAlso            []

******************************************************************************/
static void Bmc_Fsm_SetInit(Bmc_Fsm_ptr self, be_ptr init)
{
  nusmv_assert(self != NULL);
  self->init = init;
}


/**Function********************************************************************

  Synopsis           [Private service to set invar states into the given fsm]

  Description        []

  SideEffects        [self will change]

  SeeAlso            []

******************************************************************************/
static void Bmc_Fsm_SetInvar(Bmc_Fsm_ptr self, be_ptr invar)
{
  nusmv_assert(self != (Bmc_Fsm_ptr)NULL);
  self->invar = invar;
}


/**Function********************************************************************

  Synopsis           [Private service to set the transition relation
  into the given fsm]

  Description        []

  SideEffects        [self will change]

  SeeAlso            []

******************************************************************************/
static void Bmc_Fsm_SetTrans(Bmc_Fsm_ptr self, be_ptr trans)
{
  nusmv_assert(self != (Bmc_Fsm_ptr)NULL);
  self->trans = trans;
}


/**Function********************************************************************

  Synopsis           [Private service to set the list of fairness into 
  the given fsm]

  Description        []

  SideEffects        [self will change]

  SeeAlso            []

******************************************************************************/
static void Bmc_Fsm_SetFairness(Bmc_Fsm_ptr self, node_ptr fair)
{
  nusmv_assert(self != (Bmc_Fsm_ptr)NULL);
  self->fairness_list = fair;
}
