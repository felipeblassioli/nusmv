/**CFile***********************************************************************

  FileName    [simData.c]

  PackageName [sim]

  Synopsis    [Data structure handling.]

  Description [External procedures included in this module:
		<ul>
		<li> constructors, destructors, etc.
		     <ul>
		     <li> <b>Sim_DllInit()</b> initializes the dll module;
		     <li> <b>Sim_DllClear()</b> shuts down the dll module;
		     </ul>
		<li> adding clauses (tautologies are *always* discarded, 
		     duplicated literals are *always* removed):
		     <ul>
		     <li> <b>Sim_DllNewCl()</b> adds a new (empty) clause;
		     <li> <b>Sim_DllNewClSize()</b> sets the size
		     <li> <b>Sim_DllAddLit()</b> adds literals to the clause;
		     <li> <b>Sim_DllCommitCl()</b> commits the clause;
		     </ul>
		<li> other interface elements:
		     <ul> 
		     <li> <b>Sim_DllPropIsModel()</b> states model membership
		     <li> <b>Sim_DllGetSolution()</b> gets the solution
		     <li> <b>Sim_DllGetStack()</b> gets the stack contents
		     </ul>
		</ul>
	       Internal procedures included in this module:
		<ul>
		<li> <b>SimDllBuild()</b> completes the initialization;
		<li> <b>SimDllInsProp()</b> adds a proposition;
                <li> <b>SimDllDelProp()</b> removes a proposition. 
		</ul>]
		
  SeeAlso     []

  Author      [Armando Tacchella Davide Zambonin]

  Copyright   [
  This file is part of the ``sim'' package of NuSMV version 2. 
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

#include "simInt.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

Vdeclare(SimProp_t*, SimProps);
Vdeclare(SimProp_t*, SimStack);
Vdeclare(SimProp_t*, SimProp2ref);
Vdeclare(SimProp_t*, SimModelProps);
#ifdef PURE_LITERAL
Vdeclare(SimProp_t*, SimMlfStack);
#endif
Vdeclare(SimClause_t*, SimClauses);
Vdeclare(SimClause_t*, SimBcpStack);
#ifdef HORN_RELAXATION
Vdeclare(SimClause_t*, SimNhClauses);
#endif
#ifdef LEARNING
Vdeclare(SimClause_t*, SimLearned);
Vdeclare(SimClause_t*, SimUnitLearned);
#endif
int         SimParam[SIM_PARAM_NUM];
int         SimCnt[SIMCNT_NUM];
int         SimStat[SIMSTAT_NUM];
int         SimLapStat[SIMSTAT_NUM];
float       SimTimer[SIMTIMER_NUM];
#ifdef BACKJUMPING
Vdeclare(SimProp_t*, SimWrLits);
Vdeclare(int, SimLitInWr);
SimClause_t     * SimConflictCl;
#endif  
Sim_ErrCode_c      SimErrCode;
Sim_ErrLocation_c  SimErrLocation;

/**Variable********************************************************************
  Synopsis     [Parameter names.]
  Description  [Parameter names.]
  SeeAlso      [Sim_Dll_t]
******************************************************************************/
char * Sim_paramName[SIM_PARAM_NUM] = {
  "Time limit (sec)",
  "Memory limit (Mb)",
  "Heuristics",
  "Requested solutions",
  "Learn order",
  "Learning type",
  "Independent propositions",
  "Preprocessing strength",
  "Random seed",
  "Verbosity",
  "Running trace",
  "Heuristics optional param.", 
  "Maximum number of input variables",
  "Maximum number of input clauses"
};

/**Variable********************************************************************
  Synopsis     [Default parameters.]
  Description  [Default parameters.]
  SeeAlso      [Sim_Dll_t]
******************************************************************************/
int SimdefaultParam[SIM_PARAM_NUM] = {
  0,                 /* SIM_TIMEOUT. */
  0,                 /* SIM_MEMOUT. */
  SIM_BOEHM_HEUR,    /* SIM_HEURISTICS. */
  1,                 /* SIM_SOL_NUM. */
  3,                 /* SIM_LEARN_ORDER. */
  SIM_RELEVANCE,     /* SIM_LEARN_TYPE. */
  0,                 /* SIM_MODEL_PROPS. */ 
  SIM_NONE,          /* SIM_PPROC_STRENGTH. */
  0,                 /* SIM_RND_SEED */
  0,                 /* SIM_VERBOSITY */ 
  0,                 /* SIM_RUN_TRACE */
  0,                 /* SIM_HEUR_PARAM */
  100,               /* SIM_MAX_VAR_NUM. */
  1000               /* SIM_MAX_CL_NUM. */
};

/**Variable********************************************************************
  Synopsis     [Statistic names.]
  Description  [Statistic names.
  SeeAlso      [Sim_Dll_t]
******************************************************************************/
char * SimstatName[SIMSTAT_NUM] = {
  "Unit clauses",
  "Pure literals",
  "Failed literals",
  "Search tree nodes",
  "Contradictions found",
  "Failure driven assignments",
  "Deepest level",
  "Shallowest backtrack",
  "Solution depth (if any)",
  "Cycles in the main loop",
  "Skipped nodes",
  "Highest backjump",
  "Learned clauses",
  "Persistently learned clauses",
  "Unit learned clauses",
  "Biggest learned clause",
  "Smallest learned clause",    
  "Average learned clause",
  "Conflicts on learned clauses",
  "Unit clauses (preproc.)",
  "Pure literals (preproc.)",
  "Failed literals (preproc.)",
  "Binary clauses (preproc.)",
  "1-literals (preproc.)",
  "Tail resolutions (preproc.)",
  "Clause subsumptions (preproc.)",
  "Added implicates (preproc.)",
  "Discarded clauses (preproc.)",
  "Enlarged clauses (preproc.)",
  "Used memory"
};

/**Variable********************************************************************
  Synopsis     [Names of the timers.]
  Description  [Names of the timers.]
  SeeAlso      [Sim_Dll_t]
******************************************************************************/
char * SimtimerName[SIMTIMER_NUM] = {
  "Parse time",
  "Build time",
  "Preprocessing time",
  "Search time"
};


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates a default parameter setup as follows:<br>
	       <tt>
                Idx  Default value  Parameter           Notes 
		--------------------------------------------------------------
                 0       unlimited  SIM_TIMEOUT         seconds
		 1       unlimited  SIM_MEMOUT          megabytes
		 2  SIM_BOEHM_HEUR  SIM_HEURISTICS      0..SIM_HEUR_NUM
		 3               1  SIM_SOL_NUM         N >= 1
		 4               3  SIM_LEARN_ORDER     N >= 2
		 5   SIM_RELEVANCE  SIM_LEARN_TYPE      SIM_RELEVANCE/SIM_SIZE
		 6               0  SIM_MODEL_PROPS     0/1
		 7        SIM_NONE  SIM_PPROC_STRENGTH  0..SIM_RESOLVE
		 8               0  SIM_RND_SEED        N >= 0
		 9               0  SIM_VERBOSITY       0/1  
		10               0  SIM_HEUR_PARAM      N >= 0
	        11             100  SIM_MAX_VAR_NUM     N > 1
		12            1000  SIM_MAX_CL_NUM      N > 1
		--------------------------------------------------------------
	       </tt>
	       Notice that the actual number of variables and clauses in the
	       formula *cannot* be greater than SIM_MAX_VAR_NUM and
	       SIM_MAX_CL_NUM respectively. Failing to dimension these
	       parameters correctly will cause the solver to crash.]

  Description []

  SideEffects [none]

  SeeAlso     [Sim_ParamSet]

******************************************************************************/
int * 
Sim_ParamInit()
{
  int i;
  int * param = (int*)calloc(SIM_PARAM_NUM, sizeof(int));
  
  SimDllMemCheck(param, "SimParamInit: not enough memory!");
  for (i = 0; i < SIM_PARAM_NUM; i++) {
    param[i] = SIM_ASK_DEFAULT;
  }

  return param;
} /* End of Sim_ParamInit. */ 


/**Function********************************************************************

  Synopsis    [Changes the value of a specific parameter.]

  Description []

  SideEffects [none]

  SeeAlso     [Sim_ParamInit]

******************************************************************************/
int * 
Sim_ParamSet(int * param, int pName, int pValue)
{
  param[pName] = pValue;

  return param;
} /* End of Sim_ParamSet. */ 


/**Function********************************************************************

  Synopsis    [Initialize the DLL module. The array `params' must
               be suitably initialized.]

  Description [Perform several initializations.
               <ul>
	       <li> the solver working parameters are set using the values 
	            in the  input array `params'; 
	       <li> timeout and memory out are fixed;
	       <li> counters, statistics and timers are resetted;
	       <li> the random seed is initialized;
	       <li> the main propositions index and the propositions
	            look-up table are initialized 
		    (the look-up table is to accomodate up to the
		    biggest variable index);  
	       <li> the model propositions index is initialized to a 
	            fraction (25%) of the number of propositions;
	       <li> if PURE_LITERAL is defined, the pure literal stack
	            is initialized to the number of propositions;
	       <li> the main clause index and the unit clauses stack
	            are initialized to the number of clauses;
	       <li> if HORN_RELAXATION is defined, the non-Horn
	            clauses index is initialized to the number of clauses.
	       </ul>]

  SideEffects [none]

  SeeAlso     [Sim_DllClear]

******************************************************************************/
void
Sim_DllInit(
  int * params)
{
 
  int              i;

  /* Initialize the parameters. */
  for (i = 0; i < SIM_PARAM_NUM; i++) {
    if (params[i] != SIM_ASK_DEFAULT) {
      SimParam[i] = params[i];
    } else {
      SimParam[i] = SimdefaultParam[i];
    }
  }

  /* Set the timeout and the memory out. */
  if (SimParam[SIM_TIMEOUT] != 0) {
    SimDllSetTimeout(SimParam[SIM_TIMEOUT]);
  }
  if (SimParam[SIM_MEMOUT] != 0) {
    SimDllSetMemout(SimParam[SIM_MEMOUT]);
  }

  /* Initialize counters, statistics, timers. */
  for (i = 0; i < SIMCNT_NUM; i++) {
    SimCnt[i] = 0;
  }
  for (i = 0; i < SIMSTAT_NUM; i++) {
    SimStat[i] = 0;
    SimLapStat[i] = 0;
  }
  for (i = 0; i < SIMTIMER_NUM; i++) {
    SimTimer[i] = 0.0;
  }
  /* Special purpose initializations (these are "minimum" statistics). */
  SimStat[SIMDEPTH_MIN] = 
    SimStat[SIMLEARN_MIN] = 1 << ((sizeof(int) * 8) - 2);
  /* Initializations for running trace statistics. */
  SimLapStat[SIMNODE_NUM] =   
    SimLapStat[SIMFAIL_NUM] = SimParam[SIM_RUN_TRACE];

  /* Initialize the random seed. */
  SimRndSeed(SimParam[SIM_RND_SEED]);

  /* Main propositions index. */
  VinitReserve0(SimProps, SimParam[SIM_MAX_VAR_NUM]);
  SimDllMemCheck(V(SimProps), "Sim_DllInit");
  /* Propositions-to-pointers look up table. */
  VinitResize(SimProp2ref, (SimParam[SIM_MAX_VAR_NUM] + 1));
  SimDllMemCheck(V(SimProps), "Sim_DllInit");
  /* Assume that model props are 25% of the maximum number of variables. */
  i = SimParam[SIM_MAX_VAR_NUM] >> 2;
  VinitReserve0(SimModelProps, i);
  SimDllMemCheck(V(SimModelProps), "Sim_DllInit");
#ifdef PURE_LITERAL
  /* Allocate the MLF stack. */
  VinitReserve(SimMlfStack, SimParam[SIM_MAX_VAR_NUM]);
  SimDllMemCheck(V(SimMlfStack), "Sim_DllInit");
#endif

  /* Main clauses index. */
  VinitReserve(SimClauses, (SimParam[SIM_MAX_CL_NUM] + 1));
  SimDllMemCheck(V(SimClauses), "Sim_DllInit");
  V(SimClauses)[0] = 0;
  /* Unit propagation stack. */
  VinitReserve(SimBcpStack, SimParam[SIM_MAX_CL_NUM]);
  SimDllMemCheck(V(SimBcpStack), "Sim_DllInit");
#ifdef HORN_RELAXATION
  /* Non-Horn clauses index. */
  VinitReserve(SimNhClauses, SimParam[SIM_MAX_CL_NUM]);
  SimDllMemCheck(V(SimNhClauses), "Sim_DllInit");
#endif

  return;

} /* End of Sim_DllInit. */


/**Function********************************************************************

  Synopsis    [Create a new (empty) clause and return its unique id
               (an integer code greater than or equal to 0). Trying to
	       add a new clause without committing any pending one
	       yields -1 as a return value.]

  Description [If there is a pending clause in SimClauses,
               return -1, a failure code.
               Otherwise, create a new empty one and reference it
	       with the last available index in SimClauses. The size
	       of the clause will be the default SIMCL_SIZE.
	       Return the clause index (to be used in future operations with
	       the clause).] 

  SideEffects [none]

  SeeAlso     [Sim_DllAddLit Sim_DllCommitCl]

******************************************************************************/
int
Sim_DllNewCl()
{

  SimClause_t * cl;

  /* It is not possible to add two clauses contemporarily!! */
  if (V(SimClauses)[Vsize(SimClauses)] != 0) {
    return -1;
  }

  /* Create a new clause, using default size and put it in the last position
     of SimClauses (but do not change the size!!). */
  cl = V(SimClauses)[Vsize(SimClauses)] = 
    SimClauseInit(Vsize(SimClauses), SIMCL_SIZE);

  return Vsize(SimClauses);

} /* End of Sim_DllNewCl. */


/**Function********************************************************************

  Synopsis    [Create a new (empty) clause and return its unique id,
               (an integer code greater than or equal to 0).
	       The clause is dimensioned to accomodate `size' literals
	       without reallocations. 
	       Trying to add a new clause without committing any pending
	       one yields -1 as a return value.]

  Description [If there is a pending clause in SimClauses,
               return -1, a failure code.
               Otherwise, create a new empty clause and reference it
	       with the last available index in SimClauses. The size
	       of the clause will be `size'.
	       Return the clause index (to be used in future operations with
	       the clause).] 

  SideEffects [none]

  SeeAlso     [Sim_DllAddLit Sim_DllCommitCl]

******************************************************************************/
int
Sim_DllNewClSize(
  short       size)
{

  SimClause_t * cl;

  /* It is not possible to add two clauses contemporarily!! */
  if (V(SimClauses)[Vsize(SimClauses)] != 0) {
    return -1;
  }

  /* Create a new clause, using default size and put it in the last position
     of SimClauses (but do not change the size!!). */
  cl = V(SimClauses)[Vsize(SimClauses)] = 
    SimClauseInit(Vsize(SimClauses), size);

  return Vsize(SimClauses);

} /* End of Sim_DllNewClSize. */


/**Function********************************************************************

  Synopsis    [Add a literal `lit' to the current clause. `lit' must
               be an integer such that its absolute value is greater
	       than 0 and smaller than or equal to the biggest allowed
	       variable index. A positive (negative) integer denotes a
	       positive (negative) occurrence of the corresponding variable.
	       If `lit' does not fulfill the above conditions, or if
	       there is no pending clause to add `lit' to, -1 is
	       returned. The function returns 0 if the pending clause
	       becomes tautologous, and 1 otherwise. Redundant
	       literals are always discarded.]

  Description [If there is no pending clause in SimClauses,
               or `lit' is not within reasonable bounds, exit
	       with error condition -1.
	       Check for tautologies and duplications using the list of
	       occurrences for the proposition corresponding to `lit'.
	       Return 0 on tautologies, 1 on duplications.
	       If the literal is ok, add it to the clause and update
	       the lists of occurrences of the corresponding proposition.]

  SideEffects [none]

  SeeAlso     [Sim_DllNewCl Sim_DllNewClSize]

******************************************************************************/
int Sim_DllAddLit(
  int         clId,		  
  int         lit)
{
  SimProp_t    * p;
  SimProp_t   ** genp;
  SimClause_t  * cl   = V(SimClauses)[Vsize(SimClauses)];
  int            prop = SimAbs(lit);

  /* There must be a clause and the literal must be a sensible value. */
  if ((cl == 0) || (clId != cl -> backClauses) ||
      (prop >= Vsize(SimProp2ref)) || (prop == 0)) {
    return -1;
  } 

  /* If the proposition corresponding to `lit' is already there, check
     for tautologies and duplications. */
  if ((p = V(SimProp2ref)[prop]) != 0) {
    if ((Vsize(p -> posLits) > 0) && (Vback(p -> posLits) == cl)) {
      if (lit > 0) {
	/* A duplication, return without doing nothing. */
	return 1;
      } else {
	/* A tautology: the clause is to be cleared away. */
	/* Remove the last occurrence of the propositions in cl. */
	Vforeach0(cl -> lits, genp, p) {
	  if (SimIsPos(p)) {
	    VpopBack0(p -> posLits);
	  } else {
	    p = SimRef(p);
	    VpopBack0(p -> negLits);
	  }
	  if ((Vsize(p -> posLits) == 0) && (Vsize(p -> negLits) == 0)) {
	    SimDllDelProp(p);
	  }
	}
	/* The clause goes away. */
	V(SimClauses)[Vsize(SimClauses)] = 0;
	SimClauseClear(cl);
	return 0;
      }
    } else if ((Vsize(p -> negLits) > 0) && (Vback(p -> negLits) == cl)) {
      if (lit < 0) {
	/* A duplication, return without doing nothing. */
	return 1;
      } else {
	/* A tautology: the clause is to be cleared away. */
	/* Remove the last occurrence of the propositions in cl. */
	Vforeach0(cl -> lits, genp, p) {
	  if (SimIsPos(p)) {
	    VpopBack0(p -> posLits);
	  } else {
	    p = SimRef(p);
	    VpopBack0(p -> negLits);
	  }
	  if ((Vsize(p -> posLits) == 0) && (Vsize(p -> negLits) == 0)) {
	    SimDllDelProp(p);
	  }
	}
	/* The clause goes away. */
	V(SimClauses)[Vsize(SimClauses)] = 0;
	SimClauseClear(cl);
	return 0;
      }
    }
  }

  /* Now, insert the proposition and add the occurrence. */
  p = SimDllInsProp(lit, cl);
  if (lit > 0) {
    cl -> posLitNum += 1;
    VpushBackGrow0(cl -> lits, p, SIMCL_GROW);
  } else {
    p = SimMkNeg(p);
    VpushBackGrow0(cl -> lits, p, SIMCL_GROW);
  }
  cl -> openLitNum += 1;

  return 1;

} /* End of Sim_DllAddLit. */


/**Function********************************************************************

   Synopsis    [Commit the pending clause. If there is no such clause,
                or the given `clId' is differente from the current
		one, -1 is returned. If the committed clause is empty, 
		0 is returned and the function resets the state (no
		pending clause). On success, `clId' is returned.]

  Description [If there is no pending clause in SimClauses or it has 
               not the same `clId', return -1.
	       If the current clause is empty, reset the state and
	       return 0.
	       If the pending clause is ok, it is confirmed in the
	       main clause index: if unary, it is pushed in the bcp
	       stack. If HORN_RELAXATION is defined, non-Horn clauses
	       are detected and pushed in the non-Horn index.]

  SideEffects [none]

  SeeAlso     [Sim_DllInit]

******************************************************************************/
int
Sim_DllCommitCl(
  int clId)
{
  SimClause_t * cl   = V(SimClauses)[Vsize(SimClauses)];

  if ((cl == 0) || (cl -> backClauses != clId)) {
    return -1;
  } else if (cl -> openLitNum == 0) {
    /* Restore the initial condition (no pending clause). */
    SimClauseClear(cl);
    V(SimClauses)[Vsize(SimClauses)] = 0;
    return 0;
  }
  
  /* In practice, this is just to increment the internal size. */
  VforceBack(SimClauses, cl);
  /* Restore the initial condition (no pending clause). */
  V(SimClauses)[Vsize(SimClauses)] = 0;
  /* If this is a unit clause, push it in the BCP stack. */
  if (cl -> openLitNum == 1) {
    VforceBack(SimBcpStack, cl);
  }
#ifdef HORN_RELAXATION
  /* If the clause is non-Horn, push it in the non-Horn index. */
  if (cl -> posLitNum > 1) {
    cl -> backNhClauses = Vsize(SimNhClauses);
    VforceBack(SimNhClauses, cl);
  }
#endif

  return (cl -> backClauses);

} /* End of Sim_DllCommitCl. */


/**Function********************************************************************

  Synopsis    [State model membership, i.e., a proposition that is not
               dependent or defined. 'prop` must be a positive
	       non-zero integer smaller than or equal to the biggest
	       allowed variable index.]

  Description [If `prop' is a sensible value, then if the
               corresponding proposition is not there, it is created and
               inserted in the main proposition index. If the
	       proposition was not declared as a model proposition, 
	       set its back reference and push it in the model
	       propositions index.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
Sim_DllPropMakeIndep(
  int prop)
{
  SimProp_t * p;

  if ((prop > 0) && (prop < Vsize(SimProp2ref))) {
    if ((p = V(SimProp2ref)[prop]) == 0) {
      p = SimDllInsProp(prop, 0);
    }
    if (p -> backModelProps == -1) { 
      p -> backModelProps = Vsize(SimModelProps);
      VpushBackGrow0(SimModelProps, p, SIMMODEL_GROW);
    }
  }

  return;

} /* End of Sim_DllPropMakeIndep. */
  

/**Function********************************************************************

  Synopsis    [Get the current assignment to propositions. The returned
               object is an array of integers indexed on
	       propositions. For each proposition, the values  SIM_TT,
	       SIM_FF, SIM_DC are given, meaning that the proposition
	       was assigned to true, false or not assigned at all (Don't
	       Care). Propositions that did not occur in the formula
	       receive a default value of SIM_DC. The first location
	       of the returned array contains the biggest proposition index.]

  Description [Allocate `result' to contain all propositions
               indexes; scan the main propositions index and store in
	       `result' the value of the corresponing literals.]

  SideEffects [none]

  SeeAlso     [Sim_DllGetStack]

******************************************************************************/
int *
Sim_DllGetSolution()
{
  SimProp_t  * p;
  SimProp_t ** genp;
  
  int        * result = calloc(Vsize(SimProp2ref), sizeof(int));
  
  Vforeach0(SimProps, genp, p) {
    result[p -> prop] = p -> teta;
  }
  result[0] = Vsize(SimProp2ref);
 
  return result;

} /* End of Sim_DllGetSolution. */


/**Function********************************************************************

  Synopsis    [Get the current search stack contents. The returned
               object is an array representing truth assignment 
	       to propositions encoded as literals (negative
	       value if the proposition was assigned to false and vice versa).
	       The first location of the returned array is the cardinality
	       of the truth assignment.]

  Description [Allocate `result' to contain all the propositions; scan
               the stack and store in `result' the id 
	       of the literal  with the sign of propagation.]

  SideEffects [none]

  SeeAlso     [Sim_DllGetStack]

******************************************************************************/
int *
Sim_DllGetStack()
{
  SimProp_t  * p;
  int          i;
  
  int        * result = calloc(Vsize(SimProps) + 1, sizeof(int));

  for (i = 0; i < Vsize(SimStack); i++) {
    p = V(SimStack)[i];
    result[i + 1] = (p -> prop) * (p -> teta);
  }
  result[0] = Vsize(SimStack) + 1;

  return result;

} /* End of Sim_DllGetStack. */


/**Function********************************************************************

  Synopsis    [Destroys the internal data structure.]

  Description [Destroys the internal data structure.]

  SideEffects [none]

  SeeAlso     [Sim_DllInit]

******************************************************************************/
void
Sim_DllClear()
{
  int           i;
  SimProp_t   * p;
  SimProp_t  ** genp;

  /* For each clause, free the literals: then free the indexes. */
  for (i = 0; i < Vsize(SimClauses); i++) {
    SimClauseClear(V(SimClauses)[i]);
  }
  Vclear(SimClauses);
  Vclear(SimBcpStack);
#ifdef HORN_RELAXATION
  Vclear(SimNhClauses);
#endif

  /* For proposition, free the occurences: then free the indexes. */
  Vforeach0(SimProps, genp, p) {
    SimPropClear(p);
  }
  Vclear(SimProp2ref);
  Vclear(SimModelProps);

  /* Free the search stack,  and MLF stack. */
  Vclear(SimStack);
#ifdef PURE_LITERAL
  Vclear(SimMlfStack);
#endif

#ifdef BACKJUMPING
  /* Free the working reason literals and membership flags. */
  Vclear(SimWrLits);
  Vclear(SimLitInWr);
#endif
  
#ifdef LEARNING
  /* For each learned clause, free the literals: 
     then free the clauses. */
  for (i = 0; i < Vsize(SimLearned); i++) {
    SimClauseClear(V(SimLearned)[i]);
  }
  Vclear(SimLearned);
  Vclear(SimUnitLearned);
#endif

  /* Some heuristics need special purpose deallocations. */
  switch (SimParam[SIM_HEURISTICS]) { 
  case SIM_SATO_HEUR: 
    SimDllEndSatoHeur(); 
    break; 
  case SIM_SATZ_HEUR: 
    SimDllEndSatzHeur(); 
    break; 
  case SIM_RELSAT_HEUR: 
    SimDllEndRelsatHeur(); 
    break; 
  case SIM_UNITIE_HEUR: 
    SimDllEndUnitieHeur(); 
    break; 
  default: 
    /* Heuristics that do not require deallocations. */
    break; 
  } 

  return;

} /* End of Sim_DllDestroy. */
  

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Final build of the data structure.]

  Description [Completes the initialization of the internal data structure
               before the search begins.]

  SideEffects [none]

  SeeAlso     [Sim_DllInit Sim_DllClear]

******************************************************************************/
void 
SimDllBuild()
{
  int            i, j, clNum, varNum;
  SimProp_t    * p;

  /* Start the timer. */
  SimDllStartTimer(SIMBUILD_TIME);

  /* Initialize open clauses (total and non-Horn). */
  SimCnt[SIMCUR_CL_NUM] = SimCnt[SIMCL_NUM] = Vsize(SimClauses);
#ifdef HORN_RELAXATION
  SimCnt[SIMCUR_NHCL_NUM] = SimCnt[SIMNHCL_NUM] = Vsize(SimNhClauses);
#endif

  /* Check that the propositions indeed occur in the formula: if not
     remove them from the main index and the model index. 
     Reverse iteration is to avoid problems when deleting
     propositions. */
  for (j = Vsize(SimProps) - 1; j >= 0; j--) {
    p = V(SimProps)[j];
    if ((Vsize(p -> posLits) == 0) && (Vsize(p -> negLits) == 0)) {
      /* Remove from model index. */
      if (p -> backModelProps != -1) {
	i = p -> backModelProps;
	V(SimModelProps)[i] = Vback(SimModelProps);
	V(SimModelProps)[i] -> backModelProps = i;
	VpopBack0(SimModelProps);
      }
      /* Delete the proposition (also remove it from the main index). */ 
      SimDllDelProp(p);
    } else {
      /* Put the separator between static and learned occurrences. */
      VpushBackGrow0(p -> posLits, 0, SIMOCC_GROW);
      VpushBackGrow0(p -> negLits, 0, SIMOCC_GROW);
#ifdef PURE_LITERAL
      /* Check for initial pure literals. */
      if (Vsize(p -> posLits) == 0) {
        p -> mode = SIMPURENEG;
        VforceBack(SimMlfStack, p);
      } else if (Vsize(p -> negLits) == 0) {
        p -> mode = SIMPUREPOS;
        VforceBack(SimMlfStack, p);
      }
#endif
    }
  }

  varNum = Vsize(SimProps);
  /* Allocate the search stack. */
  VinitReserve(SimStack,varNum);
  SimDllMemCheck(V(SimStack), "Sim_DllBuild");
#ifdef BACKJUMPING
  /* Initialize the working reason literals (initially empty). */
  VinitReserve0(SimWrLits, varNum);
  SimDllMemCheck(V(SimWrLits), "Sim_DllBuild");

  /* Initialize the working reason membership flags. */
  i = Vsize(SimProp2ref);
  VinitResize(SimLitInWr, i);
  SimDllMemCheck(V(SimLitInWr), "Sim_DllBuild");
#endif

  clNum = Vsize(SimClauses);
#ifdef LEARNING
  /* Initialize the learned clauses array (space for clNum clauses). */
  VinitReserve(SimLearned, clNum);
  SimDllMemCheck(V(SimLearned), "Sim_DllBuild");

  /* Initialize the unit learned clauses stack. */
  VinitReserve(SimUnitLearned, clNum);
  SimDllMemCheck(V(SimUnitLearned), "Sim_DllBuild");
#endif

  /* Some heuristics need special purpose inizialitazions. */
  switch (SimParam[SIM_HEURISTICS]) { 
  case SIM_SATO_HEUR: 
    SimDllInitSatoHeur(); 
    break; 
  case SIM_SATZ_HEUR: 
    SimDllInitSatzHeur(); 
    break; 
  case SIM_RELSAT_HEUR: 
    SimDllInitRelsatHeur(); 
    break; 
  case SIM_UNITIE_HEUR: 
    SimDllInitUnitieHeur(); 
    break; 
  default: 
    /* Heuristics that do not require initialization. */
    break;  
  } 

  /* Stop the timer. */
  SimDllStopTimer(SIMBUILD_TIME);

  return;

} /* End of SimDllBuild. */


/**Function********************************************************************

  Synopsis    [Inserts a new proposition in the DLL data structure.]

  Description [Inserts a new proposition in the DLL data structure.]
	       
  SideEffects [none]

  SeeAlso     [Sim_DllAddClause]

******************************************************************************/
SimProp_t *
SimDllInsProp(
  int           lit,
  SimClause_t * cl) 
{

  SimProp_t  * p;
  int          prop = SimAbs(lit);

  /* Check if the proposition is already there or not. */
  if ((p = V(SimProp2ref)[prop]) == 0) {
    p = SimPropInit(prop, SIMOCC_SIZE);
    V(SimProp2ref)[prop] = p;
    p -> backProps = Vsize(SimProps);
    VforceBack0(SimProps, p);
  }
  /* Add the occurrence (if any) */
  if (cl != 0) {
    if (lit > 0) {
      VpushBackGrow0(p -> posLits, cl, SIMOCC_GROW);
    } else {
      VpushBackGrow0(p -> negLits, cl, SIMOCC_GROW);
    }
  }
  
  return p;

} /* End of SimDllInsProp. */


/**Function********************************************************************

  Synopsis    [Removes a  literal from the DLL data structure.]

  Description [Removes a  literal from the DLL data structure.]
	       
  SideEffects [none]

  SeeAlso     [Sim_DllAddClause]

******************************************************************************/
void 
SimDllDelProp(
  SimProp_t * p) 
{
  int i;

  i =  p -> backProps;
  V(SimProps)[i] = Vback(SimProps);
  V(SimProps)[i] -> backProps = i;
  VpopBack0(SimProps);
  V(SimProp2ref)[p -> prop] = 0;
  SimPropClear(p);
  
  return;

}/* SimDllDelProp */


