/**CFile***********************************************************************

  FileName    [sat.c]

  PackageName [sat]

  Synopsis    [Sat module]

  Description []

  SeeAlso     []

  Author      [Andrei Tchaltsev, Roberto Cavada]]

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

******************************************************************************/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include "utils/utils.h"
#include "satInt.h"
#include "solvers/SatSim.h"

#if HAVE_SOLVER_ZCHAFF
#include "solvers/SatZchaff.h"
#endif

#if HAVE_SOLVER_MINISAT
#include "solvers/SatMinisat.h"
#endif

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define SIM_NAME     "Sim"
#define ZCHAFF_NAME  "ZChaff"
#define MINISAT_NAME "MiniSat"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable*******************************************************************
  Synopsis           [The sat solvers names NuSMV provides]
  Description        []
  SideEffects        []
  SeeAlso            []
******************************************************************************/
static const char* sat_solver_names[] = {
  SIM_NAME
# if HAVE_SOLVER_ZCHAFF
  ,ZCHAFF_NAME
# endif
# if HAVE_SOLVER_MINISAT
  ,MINISAT_NAME
# endif
};

static char rcsid[] UTIL_UNUSED = "$Id: satUtils.c,v 1.3.4.2.2.6 2005/11/16 12:09:46 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro**********************************************************************

  Synopsis    [Returns the number of element in a statically allocated array]

  Description []

  SeeAlso     []

******************************************************************************/
#define GET_ARRAY_LENGTH(array) \
  (sizeof(array)/sizeof(array[0]))


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Creates a SAT solver (non-incremental) of a given name.]

  Description        [The name of a solver is case-insensitive. Returns NULL 
  if requested sat solver is not available.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
SatSolver_ptr Sat_CreateNonIncSolver(const char* satSolver)
{
  SatSolver_ptr solver = SAT_SOLVER(NULL);
  nusmv_assert(satSolver != (char*) NULL);

  if (opt_verbose_level_gt(options, 0)) {    
    fprintf(nusmv_stderr, "Creating a SAT solver instance '%s' ...\n",
	    satSolver);
  }

  if (strcasecmp(SIM_NAME, satSolver) == 0) {
    solver = SAT_SOLVER(SatSim_create(SIM_NAME)); 

  } else if (strcasecmp(ZCHAFF_NAME, satSolver) == 0) {
# if HAVE_SOLVER_ZCHAFF
    solver = SAT_SOLVER(SatZchaff_create(ZCHAFF_NAME));
# endif

  } else if (strcasecmp(MINISAT_NAME, satSolver) == 0) {
# if HAVE_SOLVER_MINISAT
    solver = SAT_SOLVER(SatMinisat_create(MINISAT_NAME));
# endif
  } 

  if (opt_verbose_level_gt(options, 0)) {    
    if (solver != SAT_SOLVER(NULL)) {
      fprintf(nusmv_stderr, "Created an SAT solver instance '%s'\n", 
	      satSolver);
    }
    else {
      fprintf(nusmv_stderr, "Failed: '%s' is not available\n", satSolver);      
    }
  }

  return solver;
}


/**Function********************************************************************

  Synopsis           [Creates an incremental SAT solver instance of a given 
  name.]

  Description        [The name of a solver is case-insensitive. Returns NULL 
  if requested sat solver is not available.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
SatIncSolver_ptr Sat_CreateIncSolver(const char* satSolver)
{
  SatIncSolver_ptr solver = SAT_INC_SOLVER(NULL);
  nusmv_assert(satSolver != (char*) NULL);

  if (opt_verbose_level_gt(options, 0)) {    
    fprintf(nusmv_stderr,
	    "Creating an incremental SAT solver instance '%s'...\n",
	    satSolver);
  }

  if (strcasecmp(ZCHAFF_NAME, satSolver) == 0) {
# if HAVE_SOLVER_ZCHAFF
    solver = SAT_INC_SOLVER(SatZchaff_create(ZCHAFF_NAME));
# endif
  } else if (strcasecmp(MINISAT_NAME, satSolver) == 0) {
# if HAVE_SOLVER_MINISAT
    solver = SAT_INC_SOLVER(SatMinisat_create(MINISAT_NAME));
# endif
  } 

  if (opt_verbose_level_gt(options, 0)) {    
    if (solver != SAT_INC_SOLVER(NULL)) {
      fprintf(nusmv_stderr, "Created an incremental SAT solver instance '%s'\n", 
	      satSolver);
    }
    else {
      fprintf(nusmv_stderr, "Failed: '%s' is not available\n", satSolver);      
    }
  }
  
  return solver;
}


/**Function********************************************************************

  Synopsis           [Given a string representing the name of a sat solver,
  returns a normalized solver name -- just potential changes in character cases
  ]

  Description [In case of an error, if an input string does not
  represented any solver, returns (const char*) NULL. Returned string
  must not be freed.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
const char* Sat_NormalizeSatSolverName(const char* solverName)
{
  int i;
  for (i=0; i<GET_ARRAY_LENGTH(sat_solver_names); ++i) {
    if (strcasecmp(solverName, sat_solver_names[i]) == 0) {
      return sat_solver_names[i];
    }
  }

  return (const char*)NULL;
}

/**Function********************************************************************

  Synopsis           [Prints out the sat solvers names the system currently 
  supplies]

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Sat_PrintAvailableSolvers(FILE* file)
{
  int i;
  fprintf(file, "The available SAT solvers are: ");
  for (i=0; i<GET_ARRAY_LENGTH(sat_solver_names); ++i) {
    fprintf(file, "%s ", sat_solver_names[i]);
  }
  fprintf(file, "\n");
}

/**AutomaticEnd***************************************************************/
