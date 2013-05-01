/**CFile***********************************************************************

  FileName    [SatSim.c]

  PackageName [sat.plugins]

  Synopsis    [Routines related to SatSim object.]

  Description [ This file contains the definition of \"SatSim\" 
  class. SatSim  (non-incremental)  solver implementation of the generic 
  SatSolver clas]
		
  SeeAlso     []

  Author      [Andrei Tchaltsev, Roberto Cavada]

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

#include "SatSim_private.h"
#include "sat/satInt.h"

static char rcsid[] UTIL_UNUSED = "$Id: SatSim.c,v 1.1.2.4 2004/07/29 17:53:02 nusmv Exp $";
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void sat_sim_finalize ARGS((Object_ptr object, void* dummy)); 

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Creates a Sim SAT solver and initializes it.]

  Description [The first parameter is the name of the solver.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
SatSim_ptr SatSim_create(const char* name)
{
  SatSim_ptr self = ALLOC(SatSim, 1);

  SAT_SIM_CHECK_INSTANCE(self);

  sat_sim_init(self, name);
  return self;
}

/**Function********************************************************************

  Synopsis    [Destroys an instance of a Sim SAT solver]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void SatSim_destroy(SatSim_ptr self)
{
  SatSolver_destroy(SAT_SOLVER(self));
}


/* ---------------------------------------------------------------------- */
/* Private Methods                                                        */
/* ---------------------------------------------------------------------- */

/*---------------------------------------------------------------------------*/
/* Static functions                                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Compares two variables indexes]

  Description [Used to sort list of variable. Used in sat_sim_add only]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static int sat_sim_compare(lsGeneric item1, lsGeneric item2)
{
  const int i1 = (int) item1;
  const int i2 = (int) item2;
  if (i1 > i2)
    return 1;
  else if (i1 == i2)
    return 0;
  else 
    return -1;
}


/**Function********************************************************************

  Synopsis    [Adds a clause to the solver database.]

  Description [Just stores all the clauses for later solving. This enables the use
  of 'add' and 'solve' interchangeably. Though this solution is not efficient.
  CNF formula must not be a constant!]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void sat_sim_add(const SatSolver_ptr solver,
			const Be_Cnf_ptr cnfProb,
			SatSolverGroup group)
{
  SatSim_ptr self = SAT_SIM(solver);

  lsList clauses;
  lsStatus lsState; /* used just for debugging */

  SAT_SIM_CHECK_INSTANCE(self);
  /* there is only one group and it is permanent. This is not incremental SAT */
  nusmv_assert(SatSolver_get_permanent_group(SAT_SOLVER(self)) == group);

  /* copy all clauses to the internal database */
  clauses = Be_Cnf_GetClausesList(cnfProb);
  clauses = lsCopyListList(clauses);
  lsJoin(self->clauses, clauses, 0);
  lsDestroy(clauses, 0);

  /* copy all independent variables, then remove duplicates */
  lsJoin(self->independentVars, Be_Cnf_GetVarsList(cnfProb), 0);
  lsState = lsSort(self->independentVars, sat_sim_compare);
  nusmv_assert(LS_OK == lsState);
  lsState = lsUniq(self->independentVars, sat_sim_compare, 0);
  nusmv_assert(LS_OK == lsState);

  /* set the maximal variable of all clauses */
  self->maxVar = self->maxVar > Be_Cnf_GetMaxVarIndex(cnfProb)
    ? self->maxVar : Be_Cnf_GetMaxVarIndex(cnfProb);
}

/**Function********************************************************************

  Synopsis    [Sets the polarity of the formula.]

  Description [Sets the polarity of the formula.
  Polarity 1 means the formula is considered as positive, and -1 means
  the negation of the formula will be solved.
  CNF formula must not be a constant!]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void sat_sim_set_polarity(const SatSolver_ptr solver,
				 const Be_Cnf_ptr cnfProb,
				 int polarity,
				 SatSolverGroup group)
{
  SatSim_ptr self = SAT_SIM(solver);

  int literal;
  lsList clause;

  SAT_SIM_CHECK_INSTANCE(self);
  /* there is only one group and it is permanent. This is not incremental SAT */
  nusmv_assert(SatSolver_get_permanent_group(SAT_SOLVER(self)) == group);

  /* create a new clause and add there the literal with correct polarity */
  literal = polarity * Be_Cnf_GetFormulaLiteral(cnfProb);
  clause = lsCreate();
  lsNewBegin(clause, (lsGeneric)literal, 0);

  /* add the clause to the clause database */
  lsNewEnd(self->clauses, (lsGeneric) clause, 0);
  
  /* the maximal variable of all clauses does not change */
}

/**Function********************************************************************

  Synopsis    [Tries to solve all added formulas]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static SatSolverResult sat_sim_solve_all_groups(const SatSolver_ptr solver)
{
  SatSim_ptr self = SAT_SIM(solver);

  int* pars;
  
  lsGen    genCl, genLit;
  lsList   cl;
  int      lit;
  int group;

  SatSolverResult  result;

  SAT_SIM_CHECK_INSTANCE(self);

  /* --- check whether there are unsatisfiable groups */
  if (0 != lsLength(SAT_SOLVER(self)->unsatisfiableGroups)){
    /* only a permanent group can be in the list,
       since there is no other groups */
    nusmv_assert(1 == lsLength(SAT_SOLVER(self)->unsatisfiableGroups));
    lsFirstItem(SAT_SOLVER(self)->unsatisfiableGroups, (lsGeneric*)&group, 0);
    nusmv_assert(SatSolver_get_permanent_group(SAT_SOLVER(self)) == group);

    return  SAT_SOLVER_UNSATISFIABLE_PROBLEM;
  }
  

  /* ---- Sets the correct number of variables and clauses! */
  pars = self->simParameters;
  pars = Sim_ParamSet(pars, SIM_MAX_VAR_NUM, self->maxVar);
  pars = Sim_ParamSet(pars, SIM_MAX_CL_NUM, lsLength(self->clauses));
    
  /* Initializes Sim. */
  Sim_DllInit(pars);
  

  
  /* ---  Upload the clauses. */
  genCl = lsStart(self->clauses);
  while (lsNext(genCl, (lsGeneric*) &cl, LS_NH) == LS_OK) {
    int clId = Sim_DllNewCl();
    
    genLit = lsStart(cl);    
    while (lsNext(genLit, (lsGeneric*) &lit, LS_NH) == LS_OK) {
      int res = Sim_DllAddLit(clId, lit);
      switch (res) {
      case 0 : 
	/* A tautology: go to the end of the clause */
	while (lsNext(genLit, (lsGeneric*) &lit, LS_NH) == LS_OK);
	/* Initiate a new clause. */
	clId = Sim_DllNewCl();
	break;
	
      case -1 :
	/* The program is gone berserk... */
	return SAT_SOLVER_INTERNAL_ERROR;
	
      default :
	/* Everything is ok */
	break;
      }
    }
    lsFinish(genLit);
    /* Commits the clause */
    Sim_DllCommitCl(clId);
  }
  lsFinish(genCl);


  /* ----- Uploads the independent variables 
     (all duplicates have already been removed from the list) */
  genLit = lsStart(self->independentVars);
  while (lsNext(genLit, (lsGeneric*) &lit, LS_NH) == LS_OK) {
    Sim_DllPropMakeIndep(lit);
  }
  lsFinish(genLit);

  /* ---- Have Sim computed a solution? */
  if (Sim_DllSolve() == SIM_SAT) {
    /* obtain the model */
    int i;
    int* m;
    lsList model = lsCreate();
    m = Sim_DllGetStack();
    for (i = 1; i < m[0]; ++i) lsNewEnd(model, (lsGeneric)m[i], LS_NH);

    /* previous model was cleaned in SatSolver's solve. */
    nusmv_assert((lsList)NULL == SAT_SOLVER(self)->model); 
    SAT_SOLVER(self)->model = model;

    result = SAT_SOLVER_SATISFIABLE_PROBLEM;
  } 
  else {
    result = SAT_SOLVER_UNSATISFIABLE_PROBLEM;
  }   
  
  /* ----- Clean up SIM. */
  Sim_DllClear();

  return result;
}

/**Function********************************************************************

  Synopsis    [This function should never be invoked since
  the actual model is generated in 'solve' method]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static lsList sat_sim_make_model (const SatSolver_ptr self)
{
  nusmv_assert(false);
  return (lsList)NULL;
};

/*---------------------------------------------------------------------------*/
/* Initializer, De-initializer, Finalizer                                    */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Initializes Sat Sim object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void sat_sim_init(SatSim_ptr self, const char* name) 
{
  SAT_SIM_CHECK_INSTANCE(self);
  sat_solver_init(SAT_SOLVER(self), name);

  OVERRIDE(Object, finalize) = sat_sim_finalize;
  OVERRIDE(SatSolver, add) = sat_sim_add;
  OVERRIDE(SatSolver, set_polarity) = sat_sim_set_polarity;
  OVERRIDE(SatSolver, solve_all_groups) = sat_sim_solve_all_groups;
  OVERRIDE(SatSolver, make_model) = sat_sim_make_model;

  self->clauses = lsCreate();
  self->independentVars = lsCreate();
  self->maxVar = 0;

  /* prepares the parameters set: */
  self->simParameters = Sim_ParamInit();
  nusmv_assert( self->simParameters != (int*)NULL);
  /* :TODO: Should this code be substituted by a configuration file, 
     or by a set of cmd-line options? */
  self->simParameters = Sim_ParamSet(self->simParameters,
				     SIM_HEURISTICS, 
				     SIM_UNITIE_HEUR);
  self->simParameters = Sim_ParamSet(self->simParameters, SIM_RUN_TRACE, 0);
  self->simParameters = Sim_ParamSet(self->simParameters, SIM_INDEP_PROPS, 1);  
}


/**Function********************************************************************

  Synopsis    [Deinitializes SatSim object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void sat_sim_deinit(SatSim_ptr self)
{ 
  SAT_SIM_CHECK_INSTANCE(self);
  Utils_FreeListOfLists(self->clauses);
  lsDestroy(self->independentVars, 0);
  sat_solver_deinit(SAT_SOLVER(self));
}

/**Function********************************************************************

  Synopsis    [Sat Sim finalize method.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void sat_sim_finalize(Object_ptr object, void* dummy) 
{
  SatSim_ptr self = SAT_SIM(object);
  sat_sim_deinit(self);
  FREE(self);
}
