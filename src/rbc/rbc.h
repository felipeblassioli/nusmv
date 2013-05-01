/**CHeaderFile*****************************************************************

  FileName    [rbc.h]

  PackageName [rbc]

  Synopsis    [Formula handling with Reduced Boolean Circuits (RBCs).]

  Description [External functions and data structures of the rbc package.]

  SeeAlso     []

  Author      [Armando Tacchella]

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

#ifndef _RBC
#define _RBC


#if HAVE_CONFIG_H
# include "config.h"
#endif

/* Standard includes. */
#if HAVE_MALLOC_H
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif  
# include <malloc.h>
#elif HAVE_SYS_MALLOC_H
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif  
# include <sys/malloc.h>
#elif HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <stdio.h>

/* Submodule includes. */
#include "utils/utils.h"
#include "dag/dag.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/**Enum**********************************************************************
  Synopsis      [RBC boolean values.]
  Description   [RBC boolean values.]
  SeeAlso       []
******************************************************************************/
typedef enum Rbc_Bool {
  RBC_FALSE = DAG_BIT_SET, 
  RBC_TRUE = 0
} Rbc_Bool_c;                            

typedef struct RbcManager Rbc_Manager_t; /* The rbc manager. */
typedef Dag_Vertex_t      Rbc_t;         /* An rbc is also a dag vertex. */  

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int Rbc_Convert2Cnf(Rbc_Manager_t * rbcManager, Rbc_t * f, lsList clauses,
			   lsList vars, int* literalAssignedToWholeFormula);
EXTERN int Rbc_CnfVar2RbcIndex(Rbc_Manager_t * rbcManager, int cnfVar);
EXTERN int Rbc_RbcIndex2CnfVar(Rbc_Manager_t * rbcManager, int rbcIndex);

EXTERN Rbc_t * Rbc_GetOne(Rbc_Manager_t * rbcManager);
EXTERN Rbc_t * Rbc_GetZero(Rbc_Manager_t * rbcManager);
EXTERN Rbc_t * Rbc_GetIthVar(Rbc_Manager_t * rbcManager, int varIndex);
EXTERN Rbc_t * Rbc_MakeNot(Rbc_Manager_t * rbcManager, Rbc_t * left);
EXTERN Rbc_t * Rbc_MakeAnd(Rbc_Manager_t * rbcManager, Rbc_t * left, Rbc_t * right, Rbc_Bool_c sign);
EXTERN Rbc_t * Rbc_MakeOr(Rbc_Manager_t * rbcManager, Rbc_t * left, Rbc_t * right, Rbc_Bool_c sign);
EXTERN Rbc_t * Rbc_MakeIff(Rbc_Manager_t * rbcManager, Rbc_t * left, Rbc_t * right, Rbc_Bool_c sign);
EXTERN Rbc_t * Rbc_MakeXor(Rbc_Manager_t * rbcManager, Rbc_t * left, Rbc_t * right, Rbc_Bool_c sign);
EXTERN Rbc_t * Rbc_MakeIte(Rbc_Manager_t * rbcManager, Rbc_t * c, Rbc_t * t, Rbc_t * e, Rbc_Bool_c sign);
EXTERN Rbc_t * Rbc_GetLeftOpnd(Rbc_t * f);
EXTERN Rbc_t * Rbc_GetRightOpnd(Rbc_t * f);
EXTERN int Rbc_GetVarIndex(Rbc_t * f);
EXTERN void Rbc_Mark(Rbc_Manager_t * rbc, Rbc_t * f);
EXTERN void Rbc_Unmark(Rbc_Manager_t * rbc, Rbc_t * f);
EXTERN Rbc_Manager_t * Rbc_ManagerAlloc(int varCapacity);
EXTERN void Rbc_ManagerReserve(Rbc_Manager_t * rbcManager, int newVarCapacity);
EXTERN int Rbc_ManagerCapacity(Rbc_Manager_t * rbcManager);
EXTERN void Rbc_ManagerFree(Rbc_Manager_t * rbcManager);
EXTERN void Rbc_ManagerGC(Rbc_Manager_t * rbcManager);
EXTERN void Rbc_OutputDaVinci(Rbc_Manager_t * rbcManager, Rbc_t * f, FILE * outFile);
EXTERN void Rbc_OutputSexpr(Rbc_Manager_t * rbcManager, Rbc_t * f, FILE * outFile);
EXTERN void Rbc_OutputGdl(Rbc_Manager_t * rbcManager, Rbc_t * f, FILE * outFile);
EXTERN Rbc_t * Rbc_Subst(Rbc_Manager_t * rbcManager, Rbc_t * f, int * subst);
EXTERN Rbc_t * Rbc_Shift(Rbc_Manager_t * rbcManager, Rbc_t * f, int shift);
EXTERN Rbc_t * Rbc_SubstRbc(Rbc_Manager_t * rbcManager, Rbc_t * f, Rbc_t ** substRbc);
EXTERN void Rbc_PrintStats(Rbc_Manager_t * rbcManager, int clustSz, FILE * outFile);

/**AutomaticEnd***************************************************************/

#endif /* _RBC */

