/**CFile***********************************************************************

  FileName    [SatMinisat_private.c]

  PackageName [SatMinisat]

  Synopsis    [The private interface of class SatMinisat]

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

  Revision    [$Id: SatMinisat_private.h,v 1.1.2.3 2005/11/16 12:09:46 nusmv Exp $]

******************************************************************************/
#ifndef __SAT_MINISAT_PRIVATE__H
#define __SAT_MINISAT_PRIVATE__H

#include "SatMinisat.h"
#include "satMiniSatIfc.h"

#include "sat/SatIncSolver_private.h"
#include "utils/assoc.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [SatMinisat Class]

  Description [ This class defines a prototype for a generic SatMinisat. This
  class is virtual and must be specialized. ]

  SeeAlso     []   
  
*******************************************************************************/
typedef struct SatMinisat_TAG
{
  INHERITS_FROM(SatIncSolver);

  MiniSat_ptr minisatSolver; /* actual instance of minisat */
  /* All input variables are represented by the  internal ones inside the 
     SatMinisat. Bellow two hash table perform the convertion in both ways */
  hash_ptr cnfVar2minisatVar;/* converts CNF variable to internal variable */
  hash_ptr minisatVar2cnfVar;/* converts internal variable into CNF variable */
  
} SatMinisat;

/**AutomaticStart*************************************************************/ 

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
void sat_minisat_init ARGS((SatMinisat_ptr self, const char* name));
void sat_minisat_deinit ARGS((SatMinisat_ptr self));

int sat_minisat_cnfLiteral2minisatLiteral ARGS((SatMinisat_ptr self,
						int cnfLitaral));
int sat_minisat_minisatLiteral2cnfLiteral ARGS((SatMinisat_ptr self, 
						int minisatLiteral));
     

/* virtual function from SatSolver */
void sat_minisat_add ARGS((const SatSolver_ptr self,
			   const Be_Cnf_ptr cnfProb,
			   SatSolverGroup group));

void sat_minisat_set_polarity ARGS((const SatSolver_ptr self,
				    const Be_Cnf_ptr cnfProb,
				    int polarity,
				    SatSolverGroup group));

SatSolverResult sat_minisat_solve_all_groups ARGS((const SatSolver_ptr self));

lsList sat_minisat_make_model ARGS((const SatSolver_ptr self));

/* virtual functions from SatIncSolver */
SatSolverGroup 
sat_minisat_create_group ARGS((const SatIncSolver_ptr self));

void
sat_minisat_destroy_group ARGS((const SatIncSolver_ptr self,
				SatSolverGroup group));

void
sat_minisat_move_to_permanent_and_destroy_group
                                ARGS((const SatIncSolver_ptr self,
				      SatSolverGroup group));
SatSolverResult
sat_minisat_solve_groups ARGS((const SatIncSolver_ptr self,
			       const lsList groups));

SatSolverResult
sat_minisat_solve_without_groups ARGS((const SatIncSolver_ptr self,
				       const lsList groups));
/**AutomaticEnd***************************************************************/

#endif /* __SAT_MINISAT_PRIVATE__H */
