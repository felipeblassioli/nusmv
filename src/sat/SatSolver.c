/**CFsaile***********************************************************************

  FileName    [SatSolver.c]

  PackageName [SatSolver]

  Synopsis    [Routines related to SatSolver object.]

  Description [ This file contains the definition of \"SatSolver\" class.]
		
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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information of NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

******************************************************************************/

#include "satInt.h" /* just for 'options' */
#include "SatSolver_private.h"



static char rcsid[] UTIL_UNUSED = "$Id: SatSolver.c,v 1.1.2.3 2004/07/28 17:19:55 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void sat_solver_finalize ARGS((Object_ptr object, void *dummy));

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Destoys an instance of a  SAT solver]

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void SatSolver_destroy(SatSolver_ptr self)
{
  nusmv_assert(SAT_SOLVER(NULL) != self);

  if (opt_verbose_level_gt(options, 0)) {    
    fprintf(nusmv_stderr, "Destroying a SAT solver instance '%s'\n",
	    SatSolver_get_name(self));
  }
  
  Object_destroy(OBJECT(self), (char*)NULL);
  
  if (opt_verbose_level_gt(options, 0)) {    
    fprintf(nusmv_stderr, "Done\n");
  }
};

/**Function********************************************************************

  Synopsis    [Returns the permanent group of this class instance.]

  Description [Every solver has one permanent group that can not be destroyed.
  This group may has more efficient representation and during invocations 
  of any 'solve' functions, the permanent group will always be
  included into the groups to be solved.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
const SatSolverGroup
SatSolver_get_permanent_group(const SatSolver_ptr self)
{
  SatSolverGroup group;
  lsStatus status;
  SAT_SOLVER_CHECK_INSTANCE(self);

  /* the first element in the list of existing group is always 
     the permanent group */
  status = lsFirstItem(self->existingGroups, (lsGeneric*)&group, 0);
  nusmv_assert(LS_OK == status);
  return group;
};

/**Function********************************************************************

  Synopsis    [Adds a CNF formula to a group ]

  Description [
  The function does not specify the polarity of the formula.
  This should be done by SatSolver_set_polarity. 
  In general, if polarity is not set any value can be assigned to the formula 
  and its variables  (this may potentially slow down the solver because 
  there is a number of variables whose value can be any and solver will try to 
  assign values to them though it is not necessary). Moreover, some solver
  (such as ZChaff) can deal with non-redundent clauses only, so the input
  clauses must be non-redundent: no variable can be in the same clause twice. 
  CNF formular may be a constant.]

  SideEffects []

  SeeAlso     [SatSolver_set_polarity]

******************************************************************************/
void
SatSolver_add(const SatSolver_ptr self, const Be_Cnf_ptr cnfProb,
	      SatSolverGroup group)
{
  SAT_SOLVER_CHECK_INSTANCE(self);
  /* the formula is a constant => do nothing (see SatSolver_set_polarity) */
  if (Be_Cnf_GetFormulaLiteral(cnfProb) == INT_MAX)
    return;

  self->add(self, cnfProb, group);
};

/**Function*********************************************************************

  Synopsis    [Sets the polarity of a CNF formula in a group]

  Description [Polarity 1 means the formula will be considered in this group
  as positive. Polarity -1 means the formula will be considered in this group
  as negative. The formula is not added to the group, just the formula's
  polarity. The formula can be added to a group with SatSolver_add.
  The formula and its polarity can be added to different groups.
  CNF formular may be a constant.]

  SideEffects []

  SeeAlso     [SatSolver_add]

******************************************************************************/
void
SatSolver_set_polarity (const SatSolver_ptr self,
			const Be_Cnf_ptr cnfProb,
			int polarity,
			SatSolverGroup group)
{
  SAT_SOLVER_CHECK_INSTANCE(self);
  nusmv_assert( -1==polarity || 1==polarity );
  nusmv_assert(sat_solver_BelongToList(self->existingGroups,
					    (lsGeneric)group));

  /* special case: the formula is a constant */
  if (Be_Cnf_GetFormulaLiteral(cnfProb) == INT_MAX)
    {
      lsList clauses = Be_Cnf_GetClausesList(cnfProb);
      int length = lsLength(clauses);
      int theConstant;
      
      /* cehck whether the constant value is true or false (see Be_Cnf_ptr) */
      if (0 == length)	
	theConstant = 1;
      else {
	/* the list must be a list with one empty clause */
	nusmv_assert(1 == length);
	nusmv_assert(LS_OK == lsFirstItem(clauses, (lsGeneric*)&clauses, 0));
	nusmv_assert(0 == lsLength(clauses));

	theConstant = -1;
      }
      theConstant *= polarity; /* consider the polarity */
      /* insert the formula */
      if (1 == theConstant) {
	if (opt_verbose_level_gt(options, 0)) {    
	  fprintf(nusmv_stderr,
		  "The true contant has been added to a solver\n");
	}
	/* the formula is true => do nothing */
      }
      else {
	if (opt_verbose_level_gt(options, 0)) {
	  fprintf(nusmv_stdout,
		  "The false contant has been added to a solver\n");
	}
	/* the formula is false. Remember this group as unsatisfiable */
	if (! sat_solver_BelongToList(self->unsatisfiableGroups,
					   (lsGeneric)group)) {
	  lsNewBegin(self->unsatisfiableGroups, (lsGeneric)group, 0);
	}
      }
      return;
    } /* if INT_MAX */
      
  self->set_polarity(self, cnfProb, polarity, group);
};

/**Function********************************************************************

  Synopsis    [Solves all groups belonging to the solver and returns the flag]

  Description []

  SideEffects []

  SeeAlso     [SatSolverResult]

******************************************************************************/
SatSolverResult
SatSolver_solve_all_groups (const SatSolver_ptr self)
{ 
  SatSolverResult result;

  SAT_SOLVER_CHECK_INSTANCE(self);
  
  /* destroy the model of previous solving */
  lsDestroy(self->model, 0);
  self->model = (lsList)NULL;

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Invoking solver '%s'...\n",
            SatSolver_get_name(self));
  }
  
  self->solvingTime = util_cpu_time(); 

  /* we have unsatisfiable formulas in some groups */
  if (0 != lsLength(self->unsatisfiableGroups)) 
    result = SAT_SOLVER_UNSATISFIABLE_PROBLEM;
  else
    result = self->solve_all_groups(self);

  self->solvingTime = util_cpu_time() - self->solvingTime;

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Solver '%s' returned after %f secs \n",
            SatSolver_get_name(self),
            SatSolver_get_last_solving_time(self)/1000.0);
  }
 
  return result;
};

/**Function********************************************************************

  Synopsis    [Returns the model (of previous solving)]

  Description [ The previous solving call should have returned SATISFIABLE.
  The returned list is a list of values in dimac form (positive literal
  is included as the variable index, negative literal as the negative 
  variable index, if a literal has not been set its value is not included). ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
const lsList
SatSolver_get_model (const SatSolver_ptr self)
{
  SAT_SOLVER_CHECK_INSTANCE(self);

  if ((lsList)NULL == self->model)
    self->model = self->make_model(self);

  return self->model;
};

/**Function********************************************************************

  Synopsis    [Returns the name of the solver]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
const char*
SatSolver_get_name (const SatSolver_ptr self)
{
  SAT_SOLVER_CHECK_INSTANCE(self);
  return self->name;
};

/**Function********************************************************************

  Synopsis    [Returns the time of last solving]

  Description []

  SideEffects []

  SeeAlso     []
******************************************************************************/
long
SatSolver_get_last_solving_time (const SatSolver_ptr self)
{
  SAT_SOLVER_CHECK_INSTANCE(self);
  return self->solvingTime;
};

/* ---------------------------------------------------------------------- */
/* Private Methods                                                        */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis    [Pure virtual function, adds a formula to a group]

  Description [It is a pure virtual function and SatSolver is an abstract
  base class. Every derived class must ovewrwrite this function.]

  SideEffects []

  SeeAlso     [SatSolver_add]

******************************************************************************/
void sat_solver_add(const SatSolver_ptr self,
		    const Be_Cnf_ptr cnfProb,
		    SatSolverGroup group)
{
  nusmv_assert(false); /* Pure Virtual Member Function */
}

/**Function********************************************************************

  Synopsis    [Pure virtual function, sets the polarity of a formula ]

  Description [It is a pure virtual function and SatSolver is an abstract
  base class. Every derived class must ovewrwrite this function.]

  SideEffects []

  SeeAlso     [SatSolver_set_polarity]

******************************************************************************/
void sat_solver_set_polarity(const SatSolver_ptr self,
			     const Be_Cnf_ptr cnfProb,
			     int polarity,
			     SatSolverGroup group)
{
  nusmv_assert(false); /* Pure Virtual Member Function */
}

/**Function********************************************************************

  Synopsis    [Pure virtual function, tries to solve formulas in the group and
  the permanent group ]

  Description [It is a pure virtual function and SatSolver is an abstract
  base class. Every derived class must ovewrwrite this function.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
SatSolverResult sat_solver_solve_all_groups(const SatSolver_ptr self)
{
  nusmv_assert(false); /* Pure Virtual Member Function */
  return SAT_SOLVER_INTERNAL_ERROR;
}


/**Function********************************************************************

  Synopsis    [Pure virtual function, creates a model for last successful
  solving]

  Description [It is a pure virtual function and SatSolver is an abstract
  base class. Every derived class must ovewrwrite this function.
  It is an error if the last solving was unsuccessful.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
lsList sat_solver_make_model(const SatSolver_ptr self)
{
  nusmv_assert(false); /* Pure Virtual Member Function */
  return (lsList)NULL;
}

/**Function********************************************************************

  Synopsis    [returns 1 if an element belongs to the list and 0 otherwise.]

  Description [just checks all elements in the list for being equal to 
  a given element]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int sat_solver_BelongToList(const lsList list, const lsGeneric element)
{
  lsGen gen = lsStart(list);
  lsGeneric data;
  while (lsNext(gen, &data, LS_NH) == LS_OK) {
    if (element == data)
      {
	lsFinish(gen);
	return 1;
      }
  }
  lsFinish(gen);
  return 0;
}

/**Function********************************************************************

  Synopsis    [removes an element from the list]

  Description [If there is no such element in the list => do nothing]

  SideEffects []

  SeeAlso     [sat_solver_BelongToList]

******************************************************************************/
void sat_solver_RemoveFromList(lsList list, const lsGeneric element)
{
  lsGen gen = lsStart(list);
  lsGeneric data;
  while (lsNext(gen, &data, LS_NH) == LS_OK) {
    if (element == data) /* delete the item */ 
      {
	lsDelBefore(gen, &data);
	nusmv_assert(element == data);
	lsFinish(gen);
	return;
      }
  }
  lsFinish(gen);
  return ;
}

/*---------------------------------------------------------------------------*/
/* Initializer, De-initializer, Finalizer                                    */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [This function initializes the SatSolver class.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void sat_solver_init(SatSolver_ptr self, const char* name)
{
  const char* defName;
  SAT_SOLVER_CHECK_INSTANCE(self);

  object_init(OBJECT(self));

  OVERRIDE(Object, finalize) = sat_solver_finalize;
  OVERRIDE(SatSolver, add) = sat_solver_add;
  OVERRIDE(SatSolver, set_polarity) = sat_solver_set_polarity;
  OVERRIDE(SatSolver, solve_all_groups) = sat_solver_solve_all_groups;
  OVERRIDE(SatSolver, make_model) = sat_solver_make_model;

  /* inits members: */
  defName = ((char*)NULL) != name ? name : "Unknown";
  self->name = ALLOC(char, strlen(defName)+1);
  nusmv_assert(self->name != (char*)NULL);
  strcpy(self->name, defName);

  self->solvingTime = 0;
  self->model = (lsList)NULL;
  self->existingGroups = lsCreate();
  /* insert the permanent group with ID '-1',
     !!! In a concrete class you should remove the inserted permanent group
     and insert a real one */
  lsNewBegin(self->existingGroups, (lsGeneric)-1, 0);
  self->unsatisfiableGroups = lsCreate();
}

/**Function********************************************************************

  Synopsis    [This function de-initializes the SatSolver class.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void sat_solver_deinit(SatSolver_ptr self)
{
  SAT_SOLVER_CHECK_INSTANCE(self);

  FREE(self->name);
  if((lsList)NULL != self->model) 
    lsDestroy(self->model, 0);

  lsDestroy(self->existingGroups, 0);
  lsDestroy(self->unsatisfiableGroups, 0);

  object_deinit(OBJECT(self));
}


/**Function********************************************************************

  Synopsis    [Finalize method of SatSolver class.]

  Description [Pure virtual function. This must be refined by derived classes.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void sat_solver_finalize(Object_ptr object, void* dummy)
{
  SatSolver_ptr self = SAT_SOLVER(object);
 
  sat_solver_deinit(self);
  nusmv_assert(false);
}

