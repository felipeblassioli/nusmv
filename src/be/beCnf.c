/**CFile***********************************************************************

  FileName    [beCnf.c]

  PackageName [be]

  Synopsis    [Conjunctive Normal Form of boolean extpressions]

  Description [This module defines the Be_Cnf structure and any related 
  method. When converting a be into cnf form the Be_ConvertToCnf function
  returns a Be_Cnf structure. The Be_Cnf structure is a base class for the 
  structure Bmc_Problem.]

  SeeAlso     [Be_ConvertToCnf, Bmc_Problem]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``be'' package of NuSMV version 2. 
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

#include "be.h"
#include "utils/NodeList.h"
#include "parser/symbols.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/**Struct**********************************************************************

  Synopsis    [Private definition for Boolean Expression in CNF form]

  Description [In order to use a Be_Cnf instance see the Be_Cnf_ptr type.]

  SeeAlso     [Be_Cnf_ptr]

******************************************************************************/
typedef struct Be_Cnf_TAG {
  be_ptr originalBe; /* the original BE problem */
  lsList cnfVars;    /* The list of CNF variables */
  lsList cnfClauses; /* The list of CNF clauses */
  int    cnfMaxVarIdx;  /* The maximum CNF variable index */

  /* literal assigned to whole CNF formula. (It may be negative)
     If the formula is a constant, see Be_Cnf_ptr. */
  int formulaLiteral;
} Be_Cnf;


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Declarations of internal functions                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Constructor for the Be_Cnf structure]

  Description [When the returned pointer is no longer used, 
  call Be_Cnf_Delete]

  SideEffects []

  SeeAlso     [Be_Cnf_Delete]

******************************************************************************/
Be_Cnf_ptr Be_Cnf_Create(const be_ptr be) 
{
  Be_Cnf_ptr self = ALLOC(Be_Cnf, 1);
  nusmv_assert(self != NULL);

  self->originalBe = be;
  self->cnfVars = lsCreate();
  self->cnfClauses = lsCreate();
  self->cnfMaxVarIdx = 0;  
  self->formulaLiteral = 0;

  return self;
}

/**Function********************************************************************

  Synopsis    [Be_Cnf structure destructor]

  Description []

  SideEffects []

  SeeAlso     [Be_Cnf_Create]

******************************************************************************/
void Be_Cnf_Delete(Be_Cnf_ptr self) 
{
  nusmv_assert(self != NULL);
  Utils_FreeListOfLists(self->cnfClauses);
  lsDestroy(self->cnfVars, NULL);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Removes any duplicate literal appearing in single clauses]

  Description [Removes any duplicate literal appearing in single clauses]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_Cnf_RemoveDuplicateLiterals(Be_Cnf_ptr self) 
{
  lsGen gen_clauses;
  lsList clause;
 
  nusmv_assert(self != NULL);
  
  gen_clauses = lsStart(Be_Cnf_GetClausesList(self));
  while (lsNext(gen_clauses, (lsGeneric*) &clause, LS_NH) == LS_OK) {
    NodeList_ptr lits; 
    lsGen gen_lit;
    int var_idx;

    lits = NodeList_create();

    gen_lit = lsStart(clause);
    while (lsNext(gen_lit, (lsGeneric*) &var_idx, LS_NH) == LS_OK) {
      node_ptr node = find_node(NUMBER, (node_ptr) var_idx, Nil);
      if (NodeList_belongs_to(lits, node)) {
	lsStatus res; 
	res = lsDelBefore(gen_lit, (lsGeneric*) &var_idx);
	nusmv_assert(res !=  LS_BADSTATE);
      }
      else {
	NodeList_append(lits, node);
      }
    }
    lsFinish(gen_lit);

    /* this frees also all created nodes */
    NodeList_destroy(lits);
  }
  lsFinish(gen_clauses);
}

/**Function********************************************************************

  Synopsis    [Returns the original BE problem this CNF was created from]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
be_ptr Be_Cnf_GetOriginalProblem(const Be_Cnf_ptr self)
{
  return self->originalBe;
}


/**Function********************************************************************

  Synopsis    [Returns the literal assigned to the whole formula.
  It may be negative. If the formula is a constant unspecified value is returned]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Be_Cnf_GetFormulaLiteral(const Be_Cnf_ptr self) 
{
  return self->formulaLiteral;
} 

/**Function********************************************************************

  Synopsis    [Returns the independent variables list in the CNF 
  representation]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
lsList Be_Cnf_GetVarsList(const Be_Cnf_ptr self) { return self->cnfVars; }


/**Function********************************************************************

  Synopsis    [Returns a list of lists which contains the CNF-ed formula]

  Description [Each list in the list is a set of integers which 
  represents a single clause. Any integer value depends on the variable 
  name and the time which the variasble is considered in, whereas the 
  integer sign is the variable polarity in the CNF-ed representation.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
lsList Be_Cnf_GetClausesList(const Be_Cnf_ptr self) 
{
  return self->cnfClauses; 
}


/**Function********************************************************************

  Synopsis    [Returns the maximum variable index in the list of clauses]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Be_Cnf_GetMaxVarIndex(const Be_Cnf_ptr self) 
{ 
  return self->cnfMaxVarIdx; 
}


/**Function********************************************************************

  Synopsis    [Returns the number of independent variables in the given 
  Be_Cnf structure]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
size_t Be_Cnf_GetVarsNumber(const Be_Cnf_ptr self) 
{
  return lsLength(Be_Cnf_GetVarsList(self));
}


/**Function********************************************************************

  Synopsis    [Returns the number of clauses in the given Be_Cnf structure]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
size_t Be_Cnf_GetClausesNumber(const Be_Cnf_ptr self) 
{
  return lsLength(Be_Cnf_GetClausesList(self));  
}


/**Function********************************************************************

  Synopsis    [Sets the literal assigned to the whole formula]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_Cnf_SetFormulaLiteral(Be_Cnf_ptr self, const int  formula_literal) 
{
  self->formulaLiteral =  formula_literal;
}

/**Function********************************************************************

  Synopsis    [Sets the maximum variable index value]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_Cnf_SetMaxVarIndex(Be_Cnf_ptr self, const int max_idx) 
{
  self->cnfMaxVarIdx = max_idx;
}


/**AutomaticEnd***************************************************************/

