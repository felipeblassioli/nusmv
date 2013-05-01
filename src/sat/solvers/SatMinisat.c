/**CFsaile***********************************************************************

  FileName    [SatMinisat.c]

  PackageName [SatMinisat]

  Synopsis    [Routines related to SatMinisat object.]

  Description [ This file contains the definition of \"SatMinisat\" class.
  The solver contains its own coding of varibales, so input variables may
  by in any range from 1 .. INT_MAX, with possible holes in the range.

  Group Control:
   To control groups, every group has its ID, which is an usual internal 
   variable. If a formula is added to a permanent group, then literals are just
   converted into the internal literals and the clauses are permamently 
   added to the solver.
   If a formula is added to non-permanent group, then after convertion of 
   literals, every clause in the  group will additionally obtain one 
   literal which is just group id, and then the clauses are added permanently
   to solver.
   Then if a group is turn on, then just its negated ID is added temporary to 
   the solver. If we want to turn the group off, the just its ID
   is added temporary to the solver.
  ]
		
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

******************************************************************************/

#include "SatMinisat_private.h"

static char rcsid[] UTIL_UNUSED = "$Id: SatMinisat.c,v 1.1.2.5 2004/08/26 08:51:57 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void sat_minisat_finalize ARGS((Object_ptr object, void *dummy));

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates a Minisat SAT solver and initializes it.]

  Description [The first parameter is the name of the solver.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
SatMinisat_ptr SatMinisat_create(const char* name)
{
  SatMinisat_ptr self = ALLOC(SatMinisat, 1);

  SAT_MINISAT_CHECK_INSTANCE(self);

  sat_minisat_init(self, name);
  return self;
}

/**Function********************************************************************

  Synopsis    [Destroys an instance of a MiniSat SAT solver]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void SatMinisat_destroy (SatMinisat_ptr self)
{
  SatSolver_destroy(SAT_SOLVER(self));
}

/* ---------------------------------------------------------------------- */
/* Private Methods                                                        */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis    [Convert a cnf literal into an internal literal used by minisat]

  Description [The literal may not be 0 (because 0 cannot have sign).
  First, the function obtains the cnf variable (removes the sign),
  obtains associated internal var through hash table(creates if necessary
  an internal variable) 
  and then converts it in minisat literal (just adjust the sign). 
  If necessary a new minisat variable is created.]

  SideEffects []

  SeeAlso     [sat_minisat_minisatLiteral2cnfLiteral]

******************************************************************************/
int sat_minisat_cnfLiteral2minisatLiteral(SatMinisat_ptr self, int cnfLiteral)
{
  int cnfVar = abs(cnfLiteral);
  int minisatVar;
  
  SAT_MINISAT_CHECK_INSTANCE(self);
  nusmv_assert(cnfVar > 0);
  /* it is important for sat_minisat_minisatLiteral2cnfLiteral */
  nusmv_assert( (int)Nil != cnfVar );
  
  minisatVar = (int)find_assoc(self->cnfVar2minisatVar, (node_ptr)cnfVar);
 
  if ((int)Nil == minisatVar) {
    /* create a new internal var and associate with cnf */
    minisatVar = MiniSat_New_Variable(self->minisatSolver);
    insert_assoc(self->cnfVar2minisatVar, (node_ptr)cnfVar, (node_ptr)minisatVar);
    insert_assoc(self->minisatVar2cnfVar, (node_ptr)minisatVar, (node_ptr)cnfVar);
  }

  return cnfLiteral > 0 ? minisatVar : - minisatVar;
};

/**Function********************************************************************

  Synopsis    [Convert an internal minisat literal into a cnf literal]

  Description [The variable in the literal has to be created by 
   sat_minisat_cnfLiteral2minisatLiteral only.
  First, the function obtains the minisat variable from the literal,
  obtains associated cnf variable (there must already be the association),
  and then converts it in cnf literal (adjust the sign)]

  SideEffects []

  SeeAlso     [sat_minisat_cnfLiteral2minisatLiteral]

******************************************************************************/
int sat_minisat_minisatLiteral2cnfLiteral(SatMinisat_ptr self, int minisatLiteral)
{
  int minisatVar = abs(minisatLiteral);
  int cnfVar = (int)find_assoc(self->minisatVar2cnfVar, (node_ptr)minisatVar);

#if 0 
  We cannot check that cnfVar != Nil, since some internal variables 
  can be used as group id-s.
  We cannnot check that internal variable is a group id, because 
  some groups may be deleted and their id-s are removed from the list
  'existing group'.

  /* cnf var is Nill only if the corresponding internal var represents 
     a group id, otherwise is always greater then 0 */
  nusmv_assert( ((int) Nil != cnfVar) ||
		sat_solver_BelongToList(SAT_SOLVER(self)->existingGroups,
					(lsGeneric)minisatVar) );
#endif

  return minisatLiteral > 0 ? cnfVar : - cnfVar;
};

/**Function********************************************************************

  Synopsis    [Adds a clause to the solver database.]

  Description [converts all CNF literals into the internal literals,
  adds a group id to every clause (if group is not permament) and then add
  obtained clauses to actual Minisat]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void sat_minisat_add(const SatSolver_ptr solver,
		     const Be_Cnf_ptr cnfProb,
		     SatSolverGroup group)
{
  SatMinisat_ptr self = SAT_MINISAT(solver);
  
  lsList clause;
  lsGen genClause;
  /* buffer to hold minisat's clauses. I think a usual clause will be 2-4 literal
   and in any case it will not be more then 1000 literal (there is an
   assertion to check it) */
  static int minisatClause[1000];
  /* just for efficiency */
  const int groupIsNotPermanent =
    SatSolver_get_permanent_group(SAT_SOLVER(self)) != group;


  SAT_MINISAT_CHECK_INSTANCE(self);

  lsForEachItem (Be_Cnf_GetClausesList(cnfProb), genClause, clause) {
    lsGen genLiteral;
    int literal;
    int literalNumber = 0;
    nusmv_assert(1000-4>lsLength(clause)); /* see minisatClause above */

    lsForEachItem(clause, genLiteral, literal) {
      minisatClause[literalNumber]
	= sat_minisat_cnfLiteral2minisatLiteral(self, literal);
      ++literalNumber;
    }

    if (groupIsNotPermanent) { /* add group id to the clause */
      minisatClause[literalNumber] = group;
      ++literalNumber;
    }
    /* add to real minisat */ 
    MiniSat_Add_Clause(self->minisatSolver, minisatClause, literalNumber);
    /* with the new interface of minisat there is not reason to remember 
       that an unsatisfiable clause has been added to the solver */

  } /* while() */
}

/**Function********************************************************************

  Synopsis    [Sets the polarity of the formula.]

  Description [Sets the polarity of the formula.
  Polarity 1 means the formula is considered as positive, and -1 means
  the negation of the formula will be solved.
  A unit clause of the literal (with sign equal to polarity)
  corresponding to the given CNF formula is added to the solve.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void sat_minisat_set_polarity(const SatSolver_ptr solver,
			     const Be_Cnf_ptr cnfProb,
			     int polarity,
			     SatSolverGroup group)
{
  SatMinisat_ptr self = SAT_MINISAT(solver);
  
  int cnfLiteral;
  int minisatLiteral;
  int minisatClause[5]; /* only one or two literals may be in the clause */

  SAT_MINISAT_CHECK_INSTANCE(self);

  cnfLiteral = polarity * Be_Cnf_GetFormulaLiteral(cnfProb);
  minisatLiteral = sat_minisat_cnfLiteral2minisatLiteral ( self,
							   cnfLiteral);
  minisatClause[0] = minisatLiteral;
  
  if ( SatSolver_get_permanent_group(SAT_SOLVER(self)) == group ) {
    MiniSat_Add_Clause(self->minisatSolver, minisatClause, 1);
  }
  else { /* add group id to clause to controle the CNF formula */
    minisatClause[1] = group;
    MiniSat_Add_Clause (self->minisatSolver, minisatClause, 2);
  }
}

/**Function********************************************************************

  Synopsis    [Tries to solve all added formulas]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
SatSolverResult sat_minisat_solve_all_groups(const SatSolver_ptr solver)
{
  SatMinisat_ptr self = SAT_MINISAT(solver);
  SAT_MINISAT_CHECK_INSTANCE(self);
  
  return sat_minisat_solve_groups(SAT_INC_SOLVER(self),
				  SAT_SOLVER(self)->existingGroups);
}

/**Function********************************************************************

  Synopsis    [This function creates a model (in the original CNF variables)]

  Description [The previous invocation of SAT_Solve should have been successful]

  SideEffects []

  SeeAlso     []

******************************************************************************/
lsList sat_minisat_make_model (const SatSolver_ptr solver)
{
  SatMinisat_ptr self = SAT_MINISAT(solver);
  int index;
  lsList model;
  int varNumber;
  
  SAT_MINISAT_CHECK_INSTANCE(self);
  /* a model is created only if there is no model */
  nusmv_assert((lsList)NULL == SAT_SOLVER(self)->model);

  model = lsCreate();
  varNumber = MiniSat_Nof_Variables(self->minisatSolver);

  for (index = 1; index <= varNumber; ++index) {
    int cnfLiteral = sat_minisat_minisatLiteral2cnfLiteral(self, index);
									  
    if (cnfLiteral > 0) {/* it is a real variable */ 
      switch (MiniSat_Get_Value(self->minisatSolver, index)) {

      case 0: /* negative polarity => change the polarity of CNF var */
	cnfLiteral = -cnfLiteral;
      case 1:  /* positive polarity => do nothing */
	lsNewEnd(model, (lsGeneric)cnfLiteral, LS_NH); /* appends the model */
	break;
	
      case -1: break; /* does not store unassigned vars */
      default:
	nusmv_assert(false); /* no other values should be provided */
      }
    }
    else {   /* just debugging */
#if 0 
  We cannot check that cnfVar != Nil, since some internal variables 
  can be used as group id-s.
  We cannnot check that internal variable is a group id, because 
  some groups may be deleted and their id-s are removed from the list
  'existing group'.
#endif
    }
  } /* for() */
  return model;
};
  
/**Function********************************************************************

  Synopsis    [Creates a new group and returns its ID ]

  Description [Adds the group at the END of the existing groups list]

  SideEffects []

  SeeAlso     [SatIncSolver_destroy_group,
  SatIncSolver_move_to_permanent_and_destroy_group]

******************************************************************************/
SatSolverGroup
sat_minisat_create_group(const SatIncSolver_ptr solver)
{
  SatMinisat_ptr self = SAT_MINISAT(solver);
  int newGroup;
  SAT_MINISAT_CHECK_INSTANCE(self);
  newGroup = MiniSat_New_Variable(self->minisatSolver);
  lsNewEnd(SAT_SOLVER(self)->existingGroups, (lsGeneric)newGroup, 0);
  return newGroup;
};


/**Function********************************************************************

  Synopsis    [Destroy an existing group (which has been returned by
  SatIncSolver_create_group) and all formulas in it. ]

  Description [Just adds to the solver a unit clause with positive literal 
  of a variable with index  equal to group id ]

  SideEffects []

  SeeAlso     [SatIncSolver_create_group]

******************************************************************************/
void
sat_minisat_destroy_group(const SatIncSolver_ptr solver, SatSolverGroup group)
{
  SatMinisat_ptr self = SAT_MINISAT(solver);
  int minisatClause[2];


  SAT_MINISAT_CHECK_INSTANCE(self);
  /* it should not be a permanent group */
  nusmv_assert(SatSolver_get_permanent_group(SAT_SOLVER(self)) != group);
  /* the group should exist */
  nusmv_assert(sat_solver_BelongToList(SAT_SOLVER(self)->existingGroups,
				       (lsGeneric)group));


  /* delete the group from the lists */
  sat_solver_RemoveFromList(SAT_SOLVER(self)->existingGroups,
			    (lsGeneric)group);
  sat_solver_RemoveFromList(SAT_SOLVER(self)->unsatisfiableGroups,
			    (lsGeneric)group);

  /* add literal corresponding to group id to the solver (to
     make all clauses contaning it true, so useless */
  minisatClause[0] = group;
  MiniSat_Add_Clause(self->minisatSolver, minisatClause, 1);
  /* with new minisat interface it is not necessary to check
     the successfulness of adding a clause */
};

/**Function********************************************************************

  Synopsis    [Moves all formulas from a group into the permanent group of
  the solver and then destroy the given group.]
  

  Description [just adds  to minisat a unit clause with negative literal 
  of a variable with index equal to group id]

  SideEffects []

  SeeAlso     [SatIncSolver_create_group, SatSolver_get_permanent_group,
  ]

******************************************************************************/
void
sat_minisat_move_to_permanent_and_destroy_group(const SatIncSolver_ptr solver,
						SatSolverGroup group)
{
  SatMinisat_ptr self = SAT_MINISAT(solver);
  int minisatClause[2];
  SatSolverGroup permamentGroup;


  SAT_MINISAT_CHECK_INSTANCE(self);

  permamentGroup = SatSolver_get_permanent_group(SAT_SOLVER(self));

  /* it should not be a permanent group */
  nusmv_assert( permamentGroup != group);
  /* the group should exist */
  nusmv_assert(sat_solver_BelongToList(SAT_SOLVER(self)->existingGroups,
				       (lsGeneric)group));

  /* if the group is unsatisfiable, make the permanent group unsatisfiable */
  if (sat_solver_BelongToList(SAT_SOLVER(self)->unsatisfiableGroups,
			      (lsGeneric)group) &&
      ! sat_solver_BelongToList(SAT_SOLVER(self)->unsatisfiableGroups,
				(lsGeneric)permamentGroup) ) {
    lsNewBegin(SAT_SOLVER(self)->unsatisfiableGroups,
	       (lsGeneric)permamentGroup,
	       0);
  }


  /* delete the group from the lists */
  sat_solver_RemoveFromList(SAT_SOLVER(self)->existingGroups,
			    (lsGeneric)group);
  sat_solver_RemoveFromList(SAT_SOLVER(self)->unsatisfiableGroups,
			    (lsGeneric)group);

  /* add negated literal corresponding to group id to the solver (to
     remove the group id literal from all the clauses belonding to the group */
  minisatClause[0] = -group;
  MiniSat_Add_Clause(self->minisatSolver, minisatClause, 1);
  /* with new minisat interface it is not necessary to check
     the successfulness of adding a clause */
  MiniSat_simplifyDB(self->minisatSolver);
};

/**Function********************************************************************

  Synopsis    [Tries to solve formulas from the groups in the list.]

  Description [The permanent group is automatically added to the list.
  Returns a flag whether the solving was successful. If it was successful only
  then SatSolver_get_model may be invoked to obtain the model ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
SatSolverResult
sat_minisat_solve_groups(const SatIncSolver_ptr solver, const lsList groups)
{
  SatMinisat_ptr self = SAT_MINISAT(solver);
  SatSolverGroup permanentGroup;
  int minisatResult;
  int* assumptions;
  int numberOfGroups;
  lsGen gen;
  SatSolverGroup aGroup;

  
  
  SAT_MINISAT_CHECK_INSTANCE(self);

  permanentGroup = SatSolver_get_permanent_group(SAT_SOLVER(self));

  /* if the permanent group is unsatisfiable => return.
   We check it here because the input list may not contain permanent group */
  if (sat_solver_BelongToList(SAT_SOLVER(self)->unsatisfiableGroups,
			      (lsGeneric)permanentGroup) ) {
    return SAT_SOLVER_UNSATISFIABLE_PROBLEM;
  }
  
  numberOfGroups = lsLength(groups);
  nusmv_assert( numberOfGroups>= 0 );
  assumptions = ALLOC(int, numberOfGroups);
  
  numberOfGroups = 0;
  lsForEachItem (groups, gen, aGroup) {
    /* the group existins */
    nusmv_assert(sat_solver_BelongToList(SAT_SOLVER(self)->existingGroups,
					 (lsGeneric)aGroup));
    /* the group is unsatisfiable => exit */
    if (sat_solver_BelongToList(SAT_SOLVER(self)->unsatisfiableGroups,
				(lsGeneric)aGroup)) {
      lsFinish(gen);
      FREE(assumptions);
      return SAT_SOLVER_UNSATISFIABLE_PROBLEM;
    }
    
    /* add negated literal of group id to the assumptions (if this is not
       a permanent group) */
    if (permanentGroup != aGroup) {
      assumptions[numberOfGroups] = -aGroup;
      ++numberOfGroups;
    }
  }; /* end of lsForEachItem */

  /* try to solver (MiniSat_Solve will invoke internal simplifyDB) */
  minisatResult = MiniSat_Solve_Assume(self->minisatSolver,
				       numberOfGroups, assumptions);
  FREE(assumptions);
  if (1 == minisatResult) {
    return SAT_SOLVER_SATISFIABLE_PROBLEM;
  }
  else
    return SAT_SOLVER_UNSATISFIABLE_PROBLEM;
};


/**Function********************************************************************

  Synopsis    [Tries to solve formulas in groups belonging to the solver
  except the groups in the list.]

  Description [The permanent group must not be in the list.
  Returns a flag whether the solving was successful. If it was successful only
  then SatSolver_get_model may be invoked to obtain the model ]

  SideEffects []

  SeeAlso     [SatSolverResult,SatSolver_get_permanent_group,
  SatIncSolver_create_group, SatSolver_get_model]

******************************************************************************/
SatSolverResult
sat_minisat_solve_without_groups(const SatIncSolver_ptr solver,
				 const lsList groups)
{
  SatMinisat_ptr self = SAT_MINISAT(solver);

  SatSolverResult result;
  lsList includeGroups;
  lsGen gen;
  SatSolverGroup aGroup;

  SAT_MINISAT_CHECK_INSTANCE(self);
  nusmv_assert(!sat_solver_BelongToList (groups,
		(lsGeneric)SatSolver_get_permanent_group(SAT_SOLVER(self))));

  /* create a list of all groups except the groups in the list */
  includeGroups = lsCreate();
  lsForEachItem(SAT_SOLVER(self)->existingGroups, gen, aGroup)
    {
      if(!sat_solver_BelongToList(groups, (lsGeneric)aGroup)) {
	lsNewEnd(includeGroups, (lsGeneric)aGroup, 0);
      }
    }
  result = sat_minisat_solve_groups(solver, includeGroups);
  lsDestroy(includeGroups, 0);
  return result;
};



/*---------------------------------------------------------------------------*/
/* Initializer, De-initializer, Finalizer                                    */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Initializes Sat Minisat object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void sat_minisat_init(SatMinisat_ptr self, const char* name) 
{
  SAT_MINISAT_CHECK_INSTANCE(self);

  sat_inc_solver_init(SAT_INC_SOLVER(self), name);

  OVERRIDE(Object, finalize) = sat_minisat_finalize;

  OVERRIDE(SatSolver, add) = sat_minisat_add;
  OVERRIDE(SatSolver, set_polarity) = sat_minisat_set_polarity;
  OVERRIDE(SatSolver, solve_all_groups) = sat_minisat_solve_all_groups;
  OVERRIDE(SatSolver, make_model) = sat_minisat_make_model;

  OVERRIDE(SatIncSolver, create_group) = sat_minisat_create_group;
  OVERRIDE(SatIncSolver, destroy_group) = sat_minisat_destroy_group;
  OVERRIDE(SatIncSolver, move_to_permanent_and_destroy_group) 
    = sat_minisat_move_to_permanent_and_destroy_group;
  OVERRIDE(SatIncSolver, solve_groups) = sat_minisat_solve_groups;
  OVERRIDE(SatIncSolver, solve_without_groups)
    = sat_minisat_solve_without_groups;

  self->minisatSolver = MiniSat_Create();

  /* the exisiting (-1) permanent group is OK, since minisat always 
     deals with variables greater then 0 */

  self->cnfVar2minisatVar = new_assoc();
  self->minisatVar2cnfVar = new_assoc();
}

/**Function********************************************************************

  Synopsis    [Deinitializes SatMinisat object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void sat_minisat_deinit(SatMinisat_ptr self)
{ 
  SAT_MINISAT_CHECK_INSTANCE(self);

  free_assoc(self->cnfVar2minisatVar);
  free_assoc(self->minisatVar2cnfVar);
  
  MiniSat_Delete(self->minisatSolver);

  sat_solver_deinit(SAT_SOLVER(self));
}
/**Function********************************************************************

  Synopsis    [Finalize method of SatMinisat class.]

  Description [Pure virtual function. This must be refined by derived classes.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void  sat_minisat_finalize(Object_ptr object, void* dummy)
{
  SatMinisat_ptr self = SAT_MINISAT(object);
  sat_minisat_deinit(self);
  FREE(self);
}

