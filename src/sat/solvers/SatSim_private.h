/**CHeaderFile*****************************************************************

  FileName    [SatSim_private.h]

  PackageName [sat.plugins]

  Synopsis    [The private header file for the SatSim class.]

  Description [Sim  (non-incremental) solver implementation of the generic 
  SatSolver class ]

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

  Revision    [$Id: SatSim_private.h,v 1.1.2.3 2005/11/16 12:09:46 nusmv Exp $]

******************************************************************************/
#ifndef __SAT_SIM_PRIVATE__H
#define __SAT_SIM_PRIVATE__H

#include "SatSim.h"

#include "sat/SatSolver_private.h"
#include "sim/sim.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [SatSim Class]

  Description [ This class contains (non-incremental) SatSim -- 
  an implementation of generic SatSolver.
  This Class inherits from SatSolver class.
  ]

  SeeAlso     []   
  
******************************************************************************/
typedef struct SatSim_TAG 
{ 
  INHERITS_FROM(SatSolver);

  lsList clauses; /* all clause submitted to the solver */
  lsList independentVars; /* a list of independent varibales in the
                             CNF formulas, all duplicates are removed */
  int maxVar; /* maximal variable in all submitted clauses */
  int* simParameters; /* parameters supplied to Sim */
} SatSim;

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
void 
sat_sim_init ARGS((SatSim_ptr self, const char* name)); 

void sat_sim_deinit ARGS((SatSim_ptr self));

/* virtual function from SatSolver */
static void sat_sim_add (const SatSolver_ptr self,
		  const Be_Cnf_ptr cnfProb,
		  SatSolverGroup group);

static void sat_sim_set_polarity ARGS((const SatSolver_ptr self,
					const Be_Cnf_ptr cnfProb,
					int polarity,
					SatSolverGroup group));

static SatSolverResult 
sat_sim_solve_all_groups ARGS((const SatSolver_ptr self));

static lsList sat_sim_make_model ARGS((const SatSolver_ptr self));

/**AutomaticEnd***************************************************************/

#endif /* __SAT_SIM_PRIVATE__H */

