/**CFile***********************************************************************

  FileName    [SatSolver_private.c]

  PackageName [SatSolver]

  Synopsis    [The private interface of class SatSolver]

  Description [Private definition to be used by derived classes]
		
  SeeAlso     []

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``sat'' package of NuSMV version 2. 
  Copyright (C) 2004 by ITC-irst.

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

  Revision    [$Id: SatSolver_private.h,v 1.1.2.3 2005/11/16 12:09:46 nusmv Exp $]

******************************************************************************/
#ifndef __SAT_SOLVER_PRIVATE__H
#define __SAT_SOLVER_PRIVATE__H

#include "SatSolver.h"
#include "utils/object_private.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [SatSolver Class]

  Description [ This class defines a prototype for a generic SatSolver. This
  class is virtual and must be specialized. ]

  SeeAlso     []   
  
*******************************************************************************/
typedef struct SatSolver_TAG
{
  INHERITS_FROM(Object);

  char* name; /* name of the solver */
  long solvingTime; /* time of the last solving */
  lsList model; /* the model created during last successful solving */
  /* groups belonging to the solver. The permanent group is always the first 
     in the list!
  */
  lsList existingGroups;
  lsList unsatisfiableGroups; /* groups which contains unsatisfiable formulas */
  /* ---------------------------------------------------------------------- */ 
  /* Virtual Methods                                                        */
  /* ---------------------------------------------------------------------- */ 
  /* adds a set of CNF clauses in the solver, but not specifies the polarity 
     of formula */
  VIRTUAL void (*add) (const SatSolver_ptr self,
		       const Be_Cnf_ptr cnfProb,
		       SatSolverGroup group);

  /* sets the polarity of a formula in a group */
  VIRTUAL void (*set_polarity) (const SatSolver_ptr self,
				const Be_Cnf_ptr cnfProb,
				int polarity,
				SatSolverGroup group);

  /* solves formulas in the groups 'groupList' beloning to the solver */
  VIRTUAL SatSolverResult (*solve_all_groups) (const SatSolver_ptr self);

  /* creates the model. The previous call to 'solve' must return SATISFIABLE */
  VIRTUAL lsList (*make_model) (const SatSolver_ptr self);

} SatSolver;

/**AutomaticStart*************************************************************/ 

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
void sat_solver_init ARGS((SatSolver_ptr self, const char* name));
void sat_solver_deinit ARGS((SatSolver_ptr self));

/* pure virtual functions */
void sat_solver_add ARGS((const SatSolver_ptr self,
			  const Be_Cnf_ptr cnfClause,
			  SatSolverGroup group));

void sat_solver_set_polarity ARGS((const SatSolver_ptr self,
				   const Be_Cnf_ptr cnfClause,
				   int polarity,
				   SatSolverGroup group));

SatSolverResult sat_solver_solve_all_groups ARGS((const SatSolver_ptr self));

lsList sat_solver_make_model ARGS((const SatSolver_ptr self));

/* general functions */
int
sat_solver_BelongToList ARGS((const lsList list, const lsGeneric element));
void 
sat_solver_RemoveFromList ARGS((lsList list, const lsGeneric element));

/**AutomaticEnd***************************************************************/

#endif /* __SAT_SOLVER_PRIVATE__H */
