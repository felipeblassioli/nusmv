/**CFile***********************************************************************

  FileName    [rbcSubst.c]

  PackageName [rbc]

  Synopsis    [Formula substitutions.]

  Description [External functions included in this module:
		<ul>
		<li> <b>Rbc_Subst()</b> Substitute variables with variables
		<li> <b>Rbc_Shift()</b> Shift the variables along an offset 
		<li> <b>Rbc_SubstRbc()</b> Substitute variables with formulas
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

#include "rbc/rbcInt.h"

#include "utils/error.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

static const unsigned int RBC_MAX_OUTDEGREE = 3;

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************
  Synopsis      [Data passing in substitution-DFS (variables to variables).]
  Description   [Data passing in substitution-DFS (variables to variables).]
  SeeAlso       []
******************************************************************************/
struct SubstDfsData {
  Rbc_Manager_t * rbcManager;
  int           * subst;
  Rbc_t         * result;
};

/**Struct**********************************************************************
  Synopsis      [Data passing in shift-DFS (variables to variables).]
  Description   [Data passing in shift-DFS (variables to variables).]
  SeeAlso       []
******************************************************************************/
struct ShiftDfsData {
  Rbc_Manager_t * rbcManager;
  int             shift;
  Rbc_t         * result;
};

/**Struct**********************************************************************
  Synopsis      [Data passing in substitution-DFS (variables to formula).]
  Description   [Data passing in substitution-DFS (variables to formula).]
  SeeAlso       []
******************************************************************************/
struct SubstRbcDfsData {
  Rbc_Manager_t  * rbcManager;
  Rbc_t         ** substRbc;
  Rbc_t          * result;
};

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct SubstDfsData     SubstDfsData_t;
typedef struct ShiftDfsData     ShiftDfsData_t;
typedef struct SubstRbcDfsData  SubstRbcDfsData_t;

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

static int SubstSet(Rbc_t * f, char * SubstData, int sign);
static void SubstFirst(Rbc_t * f, char * SubstData, int sign);
static void SubstBack(Rbc_t * f, char * SubstData, int sign);
static void SubstLast(Rbc_t * f, char * SubstData, int sign);
static int ShiftSet(Rbc_t * f, char * shiftData, int sign);
static void ShiftFirst(Rbc_t * f, char * shiftData, int sign);
static void ShiftBack(Rbc_t * f, char * shiftData, int sign);
static void ShiftLast(Rbc_t * f, char * shiftData, int sign);
static int SubstRbcSet(Rbc_t * f, char * SubstRbcData, int sign);
static void SubstRbcFirst(Rbc_t * f, char * SubstRbcData, int sign);
static void SubstRbcBack(Rbc_t * f, char * SubstRbcData, int sign);
static void SubstRbcLast(Rbc_t * f, char * SubstRbcData, int sign);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Creates a fresh copy G(Y) of the rbc F(X) such 
               that G(Y) = F(X)[Y/X] where X and Y are vectors of
	       variables.] 

  Description [Given `rbcManager', the rbc `f', and the array of integers
               `subst', replaces every occurence of the variable
	       x_i in in `f' with the variable x_j provided that 
	       subst[i] = j. There is no need for `subst' to contain
	       all the  variables, but it should map at least the variables
	       in `f' in order for the substitution to work properly.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_Subst(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * f,
  int           * subst)
{
  Dag_DfsFunctions_t SubstFunctions;
  SubstDfsData_t     SubstData;

  /* Cleaning the user fields. */
  Dag_Dfs(f, &dag_DfsClean, NIL(char));

  /* Setting up the DFS functions. */
  SubstFunctions.Set        = SubstSet;
  SubstFunctions.FirstVisit = SubstFirst;
  SubstFunctions.BackVisit  = SubstBack;
  SubstFunctions.LastVisit  = SubstLast;
 
  /* Setting up the DFS data. */
  SubstData.rbcManager = rbcManager; 
  SubstData.subst   = subst;
  SubstData.result     = NIL(Rbc_t);

  /* Calling DFS on f. */
  Dag_Dfs(f, &SubstFunctions, (char*)(&SubstData));

  return SubstData.result;
  
} /* End of Rbc_Subst. */


/**Function********************************************************************

  Synopsis    [Creates a fresh copy G(X') of the rbc F(X) by shifting 
               each variable index of a certain amount.]

  Description [Given `rbcManager', the rbc `f', and the integer `shift',
               replaces every occurence of the variable x_i in in `f' with 
	       the variable x_(i + shift).]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_Shift(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * f,
  int             shift)
{
  Dag_DfsFunctions_t shiftFunctions;
  ShiftDfsData_t     shiftData;

  /* Cleaning the user fields. */
  Dag_Dfs(f, &dag_DfsClean, NIL(char));

  /* Setting up the DFS. */
  shiftFunctions.Set        = ShiftSet;
  shiftFunctions.FirstVisit = ShiftFirst;
  shiftFunctions.BackVisit  = ShiftBack;
  shiftFunctions.LastVisit  = ShiftLast;

  /* Setting up the DFS data. */
  shiftData.rbcManager = rbcManager; 
  shiftData.shift      = shift;
  shiftData.result     = NIL(Rbc_t);

  /* Calling DFS on f. */
  Dag_Dfs(f, &shiftFunctions, (char*)(&shiftData));

  return shiftData.result;
  
} /* End of Rbc_Shift. */


/**Function********************************************************************

  Synopsis    [Creates a fresh copy G(S) of the rbc F(X) such 
               that G(S) = F(X)[S/X] where X is a vector of variables and
	       S is a corresponding vector of formulas.] 

  Description [Given `rbcManager', the rbc `f', and the array of rbcs
               `substRbc', replaces every occurence of the variable
	       x_i in in `f' with the rbc r_i provided that 
	       substRbc[i] = r_i. There is no need for `substRbc' to contain
	       all the  variables, but it should map at least the variables
	       in `f' in order for the substitution to work properly.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_SubstRbc(
  Rbc_Manager_t  * rbcManager,
  Rbc_t          * f,
  Rbc_t         ** substRbc)
{
  Dag_DfsFunctions_t SubstRbcFunctions;
  SubstRbcDfsData_t  SubstRbcData;

  /* Cleaning the user fields. */
  Dag_Dfs(f, &dag_DfsClean, NIL(char));

  /* Setting up the DFS functions. */
  SubstRbcFunctions.Set        = SubstRbcSet;
  SubstRbcFunctions.FirstVisit = SubstRbcFirst;
  SubstRbcFunctions.BackVisit  = SubstRbcBack;
  SubstRbcFunctions.LastVisit  = SubstRbcLast;
 
  /* Setting up the DFS data. */
  SubstRbcData.rbcManager = rbcManager; 
  SubstRbcData.substRbc   = substRbc;
  SubstRbcData.result     = NIL(Rbc_t);

  /* Calling DFS on f. */
  Dag_Dfs(f, &SubstRbcFunctions, (char*)(&SubstRbcData));

  return SubstRbcData.result;
  
} /* End of Rbc_SubstRbc. */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Dfs Set for substitution.]

  Description [Dfs Set for substitution.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int
SubstSet(
 Rbc_t  * f,
 char   * SubstData,
 int      sign)
{
  SubstDfsData_t * sd = (SubstDfsData_t*)SubstData;

  /* Set the current general reference as result. */
  sd -> result = RbcId((Rbc_t*)(f -> gRef), sign);

  /* All nodes should be visited once and only once. */
  return (0);

} /* End of SubstSet. */


/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for substitution.]

  Description [Dfs FirstVisit for substitution.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
SubstFirst(
 Rbc_t  * f,
 char   * SubstData,
 int      sign)
{

  /* Create a temporary list (use vertex own general reference). */
  if (f -> symbol != RBCVAR) {
    f -> gRef = (char*)malloc(sizeof(Rbc_t*) * RBC_MAX_OUTDEGREE);
    f -> iRef = 0;
  }

  return;

} /* End of SubstFirst. */


/**Function********************************************************************

  Synopsis    [Dfs BackVisit for substitution.]

  Description [Dfs BackVisit for substitution.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
SubstBack(
 Rbc_t  * f,
 char   * SubstData,
 int      sign)
{
  SubstDfsData_t * sd   = (SubstDfsData_t*)SubstData;

  /* Get the current result and add it to the temporary list. */
  nusmv_assert(f -> iRef < RBC_MAX_OUTDEGREE);
  ((Rbc_t**)(f -> gRef))[(f -> iRef)++] = sd -> result;

  return;

} /* End of SubstBack. */


/**Function********************************************************************

  Synopsis    [Dfs LastVisit for Substitution.]

  Description [Dfs LastVisit for Substitution.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
SubstLast(
 Rbc_t  * f,
 char   * SubstData,
 int      sign)
{
  SubstDfsData_t * sd   = (SubstDfsData_t*)SubstData;
  Rbc_t         ** sons = (Rbc_t**)(f -> gRef);

  if (f -> symbol == RBCVAR) {
    /* Variables need to be subsituted. */
    sd -> result = 
      Rbc_GetIthVar(sd -> rbcManager,
		    ((int*)sd -> subst)[(int)(f -> data)]);
  } else {
    /* Substitutions may trigger simplifications. */
    if(f -> symbol == RBCAND)
      sd->result = Rbc_MakeAnd(sd -> rbcManager, sons[0], sons[1], RBC_TRUE);
    else if(f -> symbol == RBCIFF)
      sd->result = Rbc_MakeIff(sd -> rbcManager, sons[0], sons[1], RBC_TRUE);
    else if(f -> symbol == RBCITE)
      sd->result = Rbc_MakeIte(sd -> rbcManager, sons[0], sons[1], sons[2], RBC_TRUE);
    else
      internal_error("SubstLast: unknown RBC symbol");
    /* Clean the temporary sons list. */
    FREE(sons);
  }
  
  /* Adjust the sign and leave the result in the gRef too. */
  f -> gRef = (char*)(sd -> result);
  sd -> result = RbcId(sd -> result, sign);

  return;

} /* End of SubstLast. */


/**Function********************************************************************

  Synopsis    [Dfs Set for shifting.]

  Description [Dfs Set for shifting.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int
ShiftSet(
 Rbc_t  * f,
 char   * shiftData,
 int      sign)
{
  ShiftDfsData_t * sd = (ShiftDfsData_t*)shiftData;

  /* Set the current general reference as result. */
  sd -> result = RbcId((Rbc_t*)(f -> gRef), sign);

  /* All the nodes are visited once and only once. */
  return (0);

} /* End of ShiftSet. */


/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for shifting.]

  Description [Dfs FirstVisit for shifting.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
ShiftFirst(
 Rbc_t  * f,
 char   * shiftData,
 int      sign)
{

  /* Create a temporary list (use vertex own general reference). */
  if (f -> symbol != RBCVAR) {
    f -> gRef = (char*)malloc(sizeof(Rbc_t*) * RBC_MAX_OUTDEGREE);
    f -> iRef = 0;
  }

  return;

} /* End of ShiftFirst. */


/**Function********************************************************************

  Synopsis    [Dfs BackVisit for shifting.]

  Description [Dfs BackVisit for shifting.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
ShiftBack(
 Rbc_t  * f,
 char   * shiftData,
 int      sign)
{
  ShiftDfsData_t * sd   = (ShiftDfsData_t*)shiftData;

  /* Get the current result and add it to the temporary list. */
  nusmv_assert(f -> iRef < RBC_MAX_OUTDEGREE);
  ((Rbc_t**)(f -> gRef))[(f -> iRef)++] = sd -> result;

  return;

} /* End of ShiftBack. */


/**Function********************************************************************

  Synopsis    [Dfs LastVisit for shifting.]

  Description [Dfs LastVisit for shifting.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
ShiftLast(
 Rbc_t  * f,
 char   * shiftData,
 int      sign)
{
  ShiftDfsData_t * sd   = (ShiftDfsData_t*)shiftData;
  Rbc_t         ** sons = (Rbc_t**)(f -> gRef);

  if (f -> symbol == RBCVAR) {
    /* Variables need to be shifted. */
    sd -> result = 
      Rbc_GetIthVar(sd -> rbcManager, (int)(sd -> shift) + (int)(f ->data));
  } else {
    /* Substitutions may trigger simplifications. */
    if(f -> symbol == RBCAND)
      sd->result = Rbc_MakeAnd(sd -> rbcManager, sons[0], sons[1], RBC_TRUE);
    else if(f -> symbol == RBCIFF)
      sd->result = Rbc_MakeIff(sd -> rbcManager, sons[0], sons[1], RBC_TRUE);
    else if(f -> symbol == RBCITE)
      sd->result = Rbc_MakeIte(sd->rbcManager, sons[0], sons[1], sons[2], RBC_TRUE);
    else
      internal_error("ShiftLast: unknown RBC symbol");
    /* Clean the temporary sons list. */
    FREE(sons);
  }

  /* Adjust the sign and leave the result in the gRef too. */
  f -> gRef = (char*)(sd -> result);
  sd -> result = RbcId(sd -> result, sign);

  return;

} /* End of ShiftLast. */


/**Function********************************************************************

  Synopsis    [Dfs Set for substitution (variables to formulas).]

  Description [Dfs Set for substitution (variables to formulas).]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int
SubstRbcSet(
 Rbc_t  * f,
 char   * SubstRbcData,
 int      sign)
{
  SubstRbcDfsData_t * sd = (SubstRbcDfsData_t*)SubstRbcData;

  /* Set the current general reference as result. */
  sd -> result = RbcId((Rbc_t*)(f -> gRef), sign);

  /* All nodes should be visited once and only once. */
  return (0);

} /* End of SubstRbcSet. */


/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for substitution (variables to formulas).]

  Description [Dfs FirstVisit for substitution (variables to formulas).]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
SubstRbcFirst(
 Rbc_t  * f,
 char   * SubstRbcData,
 int      sign)
{

  /* Create a temporary list (use vertex own general reference). */
  if (f -> symbol != RBCVAR) {
    f -> gRef = (char*)malloc(sizeof(Rbc_t*) * RBC_MAX_OUTDEGREE);
    f -> iRef = 0;
  }

  return;

} /* End of SubstRbcFirst. */


/**Function********************************************************************

  Synopsis    [Dfs BackVisit for substRbcitution.]

  Description [Dfs BackVisit for substRbcitution.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
SubstRbcBack(
 Rbc_t  * f,
 char   * SubstRbcData,
 int      sign)
{
  SubstRbcDfsData_t * sd   = (SubstRbcDfsData_t*)SubstRbcData;

  /* Get the current result and add it to the temporary list. */
  nusmv_assert(f -> iRef < RBC_MAX_OUTDEGREE);
  ((Rbc_t**)(f -> gRef))[(f -> iRef)++] = sd -> result;

  return;

} /* End of SubstRbcBack. */


/**Function********************************************************************

  Synopsis    [Dfs LastVisit for SubstRbcitution.]

  Description [Dfs LastVisit for SubstRbcitution.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
SubstRbcLast(
 Rbc_t  * f,
 char   * SubstRbcData,
 int      sign)
{
  SubstRbcDfsData_t * sd   = (SubstRbcDfsData_t*)SubstRbcData;
  Rbc_t         ** sons = (Rbc_t**)(f -> gRef);

  if (f -> symbol == RBCVAR) {
    /* Variables need to be subsituted by formulas. */
    sd -> result = sd -> substRbc[(int)(f -> data)];
  } else {
    /* Substitutions may trigger simplifications. */
    if(f -> symbol == RBCAND)
      sd->result = Rbc_MakeAnd(sd -> rbcManager, sons[0], sons[1], RBC_TRUE);
    else if(f -> symbol == RBCIFF)
      sd->result = Rbc_MakeIff(sd -> rbcManager, sons[0], sons[1], RBC_TRUE);
    else if(f -> symbol == RBCITE)
      sd->result = Rbc_MakeIte(sd -> rbcManager, sons[0], sons[1], sons[2], RBC_TRUE);
    else
      internal_error("SubstRbcLast: unknown RBC symbol");
    /* Clean the temporary sons list. */
    FREE(sons);
  }

  /* Adjust the sign and leave the result in the gRef too. */
  f -> gRef = (char*)(sd -> result);
  sd -> result = RbcId(sd -> result, sign);

  return;

} /* End of SubstRbcLast. */


    
	       

