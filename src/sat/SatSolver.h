/**CHeaderFile*****************************************************************

  FileName    [SatSolver.h]

  PackageName [SatSolver]

  Synopsis    [The header file for the SatSolver class.]

  Description [A non-incremental SAT solver interface]

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

  Revision    [$Id: SatSolver.h,v 1.1.2.3 2005/11/16 12:09:46 nusmv Exp $]

******************************************************************************/

#ifndef __SAT_SOLVER_SAT_SOLVER__H
#define __SAT_SOLVER_SAT_SOLVER__H

#include "utils/object.h"
#include "be/be.h"
#include "utils/list.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* a flag returned by the 'solve' methods */
typedef enum SatSolverResult_TAG
{ SAT_SOLVER_INTERNAL_ERROR=-1, 
  SAT_SOLVER_TIMEOUT, 
  SAT_SOLVER_MEMOUT, 
  SAT_SOLVER_SATISFIABLE_PROBLEM, 
  SAT_SOLVER_UNSATISFIABLE_PROBLEM, 
  SAT_SOLVER_UNAVAILABLE
} SatSolverResult;

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct SatSolver_TAG* SatSolver_ptr;
typedef int SatSolverGroup;
/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define SAT_SOLVER(x) \
	 ((SatSolver_ptr) x)

#define SAT_SOLVER_CHECK_INSTANCE(x) \
	 (nusmv_assert(SAT_SOLVER(x) != SAT_SOLVER(NULL)))

/**AutomaticStart*************************************************************/
/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/* SatSolver Destructors */
EXTERN void SatSolver_destroy ARGS((SatSolver_ptr self));

EXTERN const SatSolverGroup
SatSolver_get_permanent_group ARGS((const SatSolver_ptr self));

EXTERN VIRTUAL void
SatSolver_add ARGS((const SatSolver_ptr self,
		    const Be_Cnf_ptr cnfProb,
		    SatSolverGroup group));

EXTERN VIRTUAL void
SatSolver_set_polarity ARGS((const SatSolver_ptr self,
			     const Be_Cnf_ptr cnfProb,
			     int polarity,
			     SatSolverGroup group));

EXTERN VIRTUAL SatSolverResult
SatSolver_solve_all_groups ARGS((const SatSolver_ptr self));

EXTERN VIRTUAL const lsList
SatSolver_get_model ARGS((const SatSolver_ptr self));

EXTERN const char*
SatSolver_get_name ARGS((const SatSolver_ptr self));

EXTERN long
SatSolver_get_last_solving_time ARGS((const SatSolver_ptr self));

/**AutomaticEnd***************************************************************/

#endif /* __SAT_SOLVER_SAT_SOLVER__H  */
