/**CFile***********************************************************************

  FileName    [rbcCnf.c]

  PackageName [rbc]

  Synopsis    [Conjunctive Normal Form (CNF) conversions.]

  Description [External functions included in this module:
		<ul>
		<li> <b>Rbc_Convert2Cnf()</b>  
		<li> <b>Rbc_CnfVar2RbcIndex()</b>  
		</ul>]
		
  SeeAlso     []

  Author      [Armando Tacchella and Tommi Junttila]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by University of Genova. 

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

  Revision    [v. 1.0]

******************************************************************************/

#include <limits.h>

#include "rbc/rbcInt.h"
#include "utils/error.h" /* for internal_error */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************
  Synopsis      [Data passing in cnf-DFS.]
  Description   [Data passing in cnf-DFS.]
  SeeAlso       []
******************************************************************************/
struct CnfDfsData {
  Rbc_Manager_t* rbcManager;
  lsList clauses;
  lsList vars;
  int result;
};


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct CnfDfsData CnfDfsData_t;


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

static int CnfSet(Rbc_t * f, char * cnfData, int sign);
static void CnfFirst(Rbc_t * f, char * cnfData, int sign);
static void CnfBack(Rbc_t * f, char * cnfData, int sign);
static void CnfLast(Rbc_t * f, char * cnfData, int sign);
static lsGeneric SwapSign(lsGeneric data);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Translates the rbc into the corresponding (equisatisfiable)
               set of clauses.]

  Description [Given `rbcManager' and `f', `clauses' is filled with the
               disjunctions corresponding to the rbc nodes according to 
	       the rules:
	       
	       f = A & B => -f A              f = A <-> B =>  f  A  B
	                    -f B                              f -A -B
			     f -A -B                         -f -A  B
                                                             -f  A -B

	       `vars' is filled with the variables that occurred in `f' 
	       (original or model variables converted into corresponding CNF 
               variables). It is user's responsibility 
	       to create `clauses' and `vars' *before* calling the function.
               New variables are added by the conversion: the maximum
	       index is returned by the function.
               The literal associated to 'f' is assigned to parameter 
	       *literalAssignedToWholeFormula (it may be negative). 
	       Special case - A CONSTANT (this is consistent with description
                 of Be_Cnf_ptr): if the formula is a constant
                 then *literalAssignedToWholeFormula will be INT_MAX
		 and the return value will 0.
		 if formula is true, `clauses' is the empty list,
                 if formula is false, `clauses' contains a single empty clause.]
							     
  SideEffects [`clauses', `vars' and '*literalAssignedToWholeFormula'
              are filled up. Fields inside rbcManager might change ]

  SeeAlso     []

******************************************************************************/
int
Rbc_Convert2Cnf(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * f,
  lsList          clauses,
  lsList          vars,
  int* literalAssignedToWholeFormula)
{
  int                i, maxVar;
  lsList             fClause;
  Dag_DfsFunctions_t cnfFunctions;
  CnfDfsData_t       cnfData;

  *literalAssignedToWholeFormula = INT_MAX;
  
  /* Handling special cases: f is a constant true or false. */
  if (f == Rbc_GetOne(rbcManager)) {
    return 0;
  }
  if (f == Rbc_GetZero(rbcManager)) {
    fClause = lsCreate();
    (void) lsNewBegin(clauses, (lsGeneric) fClause, LS_NH);
    return 0;
  }

  /* Determine the current maximum variable index. */
  maxVar = 0;
  for (i = 0; i < rbcManager->varCapacity; i++) {
    if (rbcManager->varTable[i] != NIL(Rbc_t)) {
      maxVar = i;
    }
  }

  /* check whether maxUnchangedRbcVariable can be extended
     (or whether indexes above the maxUnchangedRbcVariable have not been used) */
  if ((rbcManager->maxUnchangedRbcVariable == rbcManager->maxCnfVariable) &&
      (rbcManager->maxUnchangedRbcVariable < maxVar)) {
    rbcManager->maxUnchangedRbcVariable = maxVar;
    rbcManager->maxCnfVariable = maxVar;
  }

  /* Cleaning the user fields. */
  Dag_Dfs(f, &dag_DfsClean, NIL(char));

  /* Setting up the DFS functions. */
  cnfFunctions.Set        = CnfSet;
  cnfFunctions.FirstVisit = CnfFirst;
  cnfFunctions.BackVisit  = CnfBack;
  cnfFunctions.LastVisit  = CnfLast;
 
  /* Setting up the DFS data. */
  cnfData.rbcManager = rbcManager; 
  cnfData.clauses     = clauses;
  cnfData.vars        = vars;
  cnfData.result      = 0;

  /* Calling DFS on f. */
  Dag_Dfs(f, &cnfFunctions, (char*)(&cnfData));

  /* Remember the unit clause (literal) that stands for the whole formula f */
  *literalAssignedToWholeFormula = cnfData.result;

  return rbcManager->maxCnfVariable;
  
} /* End of Rbc_Convert2Cnf. */


/**Function********************************************************************

  Synopsis    [Returns the RBC index corresponding to a particular CNF var]

  Description [Returns -1, if there is no original RBC variable
  corresponding to CNF variable, this may be the case if CNF variable
  corresponds to an internal node (not leaf) of RBC tree. Input CNF
  variable should be a correct variable generated by RBC manager.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int
Rbc_CnfVar2RbcIndex(Rbc_Manager_t* rbcManager, int cnfVar)
{
  Rbc_t* rbcNode;

  rbcNode = (Rbc_t*) find_assoc(rbcManager->cnfVar2rbcNode, (node_ptr) cnfVar);

  /* there is always a corresponding RBC node for every (valid) CNF variable */
  nusmv_assert((Rbc_t*) NULL != rbcNode);

  return Rbc_GetVarIndex(rbcNode);
}


/**Function********************************************************************

  Synopsis    [Returns the associated CNF variable of a given RBC index]

  Description [Returns 0, if there is no original RBC variable
  corresponding to CNF variable. This may be the case if particular RBC
  node (of the given variable) has never been converted into CNF]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int
Rbc_RbcIndex2CnfVar(Rbc_Manager_t* rbcManager, int rbcIndex)
{
  Rbc_t* rbcNode;
  int cnfVar;

  /* BE index is > 0 and always != 0 (in current implementation) */
  nusmv_assert(0 < rbcIndex); 
  
  rbcNode = Rbc_GetIthVar(rbcManager, rbcIndex);
  cnfVar = (int) find_assoc(rbcManager->rbcNode2cnfVar,
			    (node_ptr) Dag_VertexGetRef(rbcNode));
  
  /* if there is no associated cnf var => 0 is returned automatically */
  return cnfVar;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Dfs Set for CNF conversion.]

  Description [Dfs Set for CNF conversion.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int CnfSet(Rbc_t* f, char* cnfData, int sign)
{
  CnfDfsData_t* cd = (CnfDfsData_t*)cnfData;

  /* Set the current integer reference as result. */
  cd->result = (sign != 0 ? -1 * (f->iRef) : f->iRef);

  /* All nodes should be visited once and only once. */
  return 0;
} /* End of CnfSet. */


/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for CNF conversion.]

  Description [Dfs FirstVisit for CNF conversion.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void CnfFirst(Rbc_t* f, char* cnfData, int sign)
{
  if (f->symbol != RBCVAR) { 
    /* Create a temporary list (use vertex own general reference). */
    f->gRef = (char*) lsCreate();
  }
} /* End of CnfFirst. */


/**Function********************************************************************

  Synopsis    [Dfs BackVisit for CNF conversion.]

  Description [Dfs BackVisit for CNF conversion.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
CnfBack(Rbc_t* f, char* cnfData, int sign)
{
  CnfDfsData_t * cd   = (CnfDfsData_t*)cnfData;

  /* Get the current result and add it (negated) to the temporary list. */
  (void) lsNewEnd((lsList)(f->gRef),
		  (lsGeneric)( -1 * (cd->result)), 
		  LS_NH);

  return;

} /* End of CnfBack. */


/**Function********************************************************************

  Synopsis    [Dfs LastVisit for CNF conversion.]

  Description [Dfs LastVisit for CNF conversion.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void CnfLast(Rbc_t* f, char* cnfData, int sign)
{
  lsList    tmpCl;
  lsGen     gen;
  int       s, pol;

  CnfDfsData_t* cd = (CnfDfsData_t*) cnfData;
  lsList sons = (lsList) (f->gRef);

  int cnfVar = (int) find_assoc( cd->rbcManager->rbcNode2cnfVar,
				 (node_ptr) Dag_VertexGetRef(f) );
  
  /* if there is no associated cnf var => create it and associate with rbc node*/
  if (0 == cnfVar) {
    /* special case: if this is a var node and rbc var is less then
       'max unchanged rbc var' then its cnf index will be the same as 
       rbc index */
    if ((f->symbol == RBCVAR) &&
	(((int) f->data) <=  cd->rbcManager->maxUnchangedRbcVariable)) {
      /* in current implementation BE indexes cannot be 0.
      This info is used in Be_CnfLiteral2BeLiteral */
      nusmv_assert(0 != (int) f->data);
      cnfVar = (int) f->data;
    } 
    else { /* otherwise just generate a new cnf var */
      cnfVar = ++(cd->rbcManager->maxCnfVariable);
    }

    /* make the association: RBC node -> CNF var */
    insert_assoc(cd->rbcManager->rbcNode2cnfVar,
		 (node_ptr) Dag_VertexGetRef(f),
		 (node_ptr) cnfVar);
    /* make the association: CNF var -> RBC node */
    insert_assoc(cd->rbcManager->cnfVar2rbcNode,
		 (node_ptr) cnfVar,
		 (node_ptr) Dag_VertexGetRef(f));
  }

  f->iRef = cnfVar;

  if (f->symbol == RBCVAR) {

    /* Fill in the list of 'original' model vars converted into CNF vars */
    (void) lsNewEnd(cd->vars, (lsGeneric) cnfVar, LS_NH);

  } 
  else {
    
    /* Generate and append clauses. */
    if (f->symbol == RBCAND) {

      /* Add the binary clauses {-f s_i} */
      gen = lsStart(sons);
      while (lsNext(gen, (lsGeneric*) &s, LS_NH) == LS_OK) {
	tmpCl = lsCreate();
	(void) lsNewEnd(tmpCl, (lsGeneric)(-1 * cnfVar), LS_NH);
	(void) lsNewEnd(tmpCl, (lsGeneric)(-1 * s), LS_NH);
	(void) lsNewBegin(cd->clauses, (lsGeneric)tmpCl, LS_NH);
      }
      lsFinish(gen);
      
      /* Add the clause {f -s_1 -s_2} */
      (void) lsNewBegin(sons, (lsGeneric)cnfVar, LS_NH);
      (void) lsNewBegin(cd->clauses, (lsGeneric)sons, LS_NH);

    } else if (f->symbol == RBCIFF) {

      /* Add the clause {-f s_1 -s_2} */
      tmpCl = lsCreate();
      (void) lsNewEnd(tmpCl, (lsGeneric)(-1 * cnfVar), LS_NH);
      gen = lsStart(sons);
      pol = -1;
      while (lsNext(gen, (lsGeneric*) &s, LS_NH) == LS_OK) {
	(void) lsNewEnd(tmpCl, (lsGeneric)(s * pol), LS_NH);
	pol *= -1;
      }
      lsFinish(gen);
      (void) lsNewBegin(cd->clauses, (lsGeneric)tmpCl, LS_NH);

      /* Add the clause {-f -s_1 s_2} */
      tmpCl = lsCreate();
      (void) lsNewEnd(tmpCl, (lsGeneric)(-1 * cnfVar), LS_NH);
      gen = lsStart(sons);
      pol = 1;
      while (lsNext(gen, (lsGeneric*) &s, LS_NH) == LS_OK) {
	(void) lsNewEnd(tmpCl, (lsGeneric)(s * pol), LS_NH);
	pol *= -1;
      }
      lsFinish(gen);
      (void) lsNewBegin(cd->clauses, (lsGeneric)tmpCl, LS_NH);

      /* Add the clause {f s_1 s_2} */
      tmpCl = lsCopy(sons, SwapSign);
      (void) lsNewBegin(tmpCl, (lsGeneric)cnfVar, LS_NH);
      (void) lsNewBegin(cd->clauses, (lsGeneric)tmpCl, LS_NH);

      /* Add the clause {f -s_1 -s_2} */
      (void) lsNewBegin(sons, (lsGeneric)cnfVar, LS_NH);
      (void) lsNewBegin(cd->clauses, (lsGeneric)sons, LS_NH);

    } else if (f->symbol == RBCITE) {
      int i, t, e;

      gen = lsStart(sons);

      /* Should have three children */
      if(lsNext(gen, (lsGeneric*) &i, LS_NH) != LS_OK) nusmv_assert(false);
      if(lsNext(gen, (lsGeneric*) &t, LS_NH) != LS_OK) nusmv_assert(false);
      if(lsNext(gen, (lsGeneric*) &e, LS_NH) != LS_OK) nusmv_assert(false);
      lsFinish(gen);
      
      /* Add the clause {-f -i t} */
      tmpCl = lsCreate();
      (void) lsNewEnd(tmpCl, (lsGeneric)(-1 * cnfVar), LS_NH);
      (void) lsNewEnd(tmpCl, (lsGeneric)(i), LS_NH);
      (void) lsNewEnd(tmpCl, (lsGeneric)(-1 * t), LS_NH);
      (void) lsNewBegin(cd->clauses, (lsGeneric)tmpCl, LS_NH);

      /* Add the clause {-f i e} */
      tmpCl = lsCreate();
      (void) lsNewEnd(tmpCl, (lsGeneric)(-1 * cnfVar), LS_NH);
      (void) lsNewEnd(tmpCl, (lsGeneric)(-1 * i), LS_NH);
      (void) lsNewEnd(tmpCl, (lsGeneric)(-1 * e), LS_NH);
      (void) lsNewBegin(cd->clauses, (lsGeneric)tmpCl, LS_NH);

      /* Add the clause {f -i -t} */
      tmpCl = lsCreate();
      (void) lsNewEnd(tmpCl, (lsGeneric)(cnfVar), LS_NH);
      (void) lsNewEnd(tmpCl, (lsGeneric)(i), LS_NH);
      (void) lsNewEnd(tmpCl, (lsGeneric)(t), LS_NH);
      (void) lsNewBegin(cd->clauses, (lsGeneric)tmpCl, LS_NH);

      /* Add the clause {f i -e} */
      tmpCl = lsCreate();
      (void) lsNewEnd(tmpCl, (lsGeneric)(cnfVar), LS_NH);
      (void) lsNewEnd(tmpCl, (lsGeneric)(-1 * i), LS_NH);
      (void) lsNewEnd(tmpCl, (lsGeneric)(e), LS_NH);
      (void) lsNewBegin(cd->clauses, (lsGeneric)tmpCl, LS_NH);

      lsDestroy(sons, 0);
    }
    else
      internal_error("CnfLast: unkown RBC symbol");

  }
  
  /* Adjust the sign of the result. */
  cd->result = (sign != 0 ? -1 * (f->iRef) : f->iRef);

  return;

} /* End of CnfLast. */


/**Function********************************************************************

  Synopsis    [Swaps the sign of the argument.]

  Description [Swaps the sign of the argument.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static lsGeneric
SwapSign(
  lsGeneric data)	    
{
 
  return (lsGeneric)(-1 * (int)data);

} /* End of CnfSwapSign. */
    
	       



