/**CHeaderFile*****************************************************************

  FileName    [simInt.h]

  PackageName [sim]

  Synopsis    [The SIM library of efficient SAT algorithms.]

  Description [Internal functions and data strucures of the SIM package.]

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


#ifndef _SIMINT
#define _SIMINT

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "sim.h"
#include "simVector.h"

#include "utils/utils.h"



/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* Memory allocation. */
#define SIMCL_SIZE        4 /* Default number of literals in a clause. */
#define SIMOCC_SIZE       4 /* Defaul positive and negative occurrences. */
#define SIMCL_GROW        2 /* Growth rate of clauses. */
#define SIMOCC_GROW       2 /* Growth rate of occurrences. */
#define SIMMODEL_GROW     2 /* Growth rate of model variables. */
#define SIMLEARNED_GROW 1.2 /* Growth rate of learned clauses. */

/* Parsing and stuff. */
#define SIMMAX_CL_CH     5000  /* Maximum number of characters per line. */
#define SIMMAX_LIT_CH      15  /* Maximum number of characters per literal. */ 

/* Type of assignment. */
#define SIMUNIT             0  /* Unit propagation. */ 
#define SIMPUREPOS          1  /* Pure literal (positive). */
#define SIMPURENEG          2  /* Pure literal (negative). */
#define SIMLSPLIT           3  /* Left split. */
#define SIMRSPLIT           4  /* Right split. */
#define SIMFAILED           5  /* Failed literal. */

/* Status of a clause (to handle unit learned clauses). */
#define SIMALLOW_UNIT      -2
#define SIMFORBID_UNIT     -1

/* Counters. */
#define SIMCL_NUM           0  /* Clauses before the search. */
#define SIMNHCL_NUM         1  /* Non-Horn clauses before the search. */
#define SIMCUR_CL_NUM       2  /* Open clauses. */
#define SIMCUR_NHCL_NUM     3  /* Open non-Horn clauses. */
#define SIMCUR_LEVEL        4  /* Current depth in the search tree. */
#define SIMCNT_NUM          5  /* How many counters. */

/* Statistics. */
#define SIMUNIT_NUM         0  /* Unit propagations. */
#define SIMPURE_NUM         1  /* Pure literal propagations. */
#define SIMFAILED_NUM       2  /* Failed literal propagations. */
#define SIMNODE_NUM         3  /* Search tree nodes (heuristics choices). */
#define SIMFAIL_NUM         4  /* Contradictions found. */
#define SIMFDA_NUM          5  /* Failure driven assignments. */
#define SIMDEPTH_MAX        6  /* Maximum depth of the search tree. */
#define SIMDEPTH_MIN        7  /* Shallowest point of backtracking. */
#define SIMSOL_NODE_NUM     8  /* Depth of the solution. */
#define SIMCYCLES_NUM       9  /* Number of cycles in the main loop. */
#define SIMSKIP_NUM        10  /* How many skipped nodes. */
#define SIMSKIP_MAX        11  /* The highest backjump (number of nodes) .*/
#define SIMLEARN_NUM       12  /* Learned clauses. */
#define SIMSLEARN_NUM      13  /* Persistently learned clauses. */
#define SIMULEARN_NUM      14  /* Unit propagations on unit learned clauses. */
#define SIMLEARN_MAX       15  /* Biggest learned clause. */
#define SIMLEARN_MIN       16  /* Smallest learned clause. */    
#define SIMLEARN_AVE       17  /* Average size of learned clauses. */
#define SIMCLASH_NUM       18  /* Number of conflicts on learned clauses. */

#define SIMPUNIT_NUM       19  /* Unit propagations (preproc.). */
#define SIMPPURE_NUM       20  /* Pure literal propagations (preproc.).*/
#define SIMPFAIL_NUM       21  /* Failed literal propagations (preproc.).*/
#define SIMPBIN_NUM        22  /* Binary clauses propagations (preproc.).*/
#define SIMP1LIT_NUM       23  /* Number of 1-literals (preproc.).  */
#define SIMPTAIL_NUM       24  /* Number of tail resolutions (preproc.). */
#define SIMPSUBS_NUM       25  /* Number of clause subsumptions (preproc.). */
#define SIMPRES_NUM        26  /* Number of added implicates (preproc.). */
#define SIMPDEL_CL_NUM     27  /* Number of discarded clauses (preproc.). */
#define SIMPENL_CL_NUM     28  /* Number of enlarged clauses (preproc.). */

#define SIMUSD_MEM         29  /* Used memory. */
#define SIMSTAT_NUM        30  /* How many statistics. */

/* Timers */
#define SIMPARSE_TIME       0  /* Time to parse the formula. */
#define SIMBUILD_TIME       1  /* Time to build the internal structure. */
#define SIMPREPROC_TIME     2  /* Time to preprocess the formula. */
#define SIMSEARCH_TIME      3  /* Time to search for an assignment. */
#define SIMTIMER_NUM        4  /* How many timers. */


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct SimProp   SimProp_t;        /* Propositions. */
typedef struct SimClause SimClause_t;      /* Clauses. */ 

typedef int         (*SimIntFunPtr_t)();   /* Returning int. */
typedef SimProp_t * (*SimPropFunPtr_t)();  /* Returning Sim_Prop_t. */
typedef void        (*SimVoidFunPtr_t)();  /* Procedures. */

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************
  Synopsis      [Holds data about a clause. ]
  Description   [Holds the number of open literals (openLitNum), and 
                 the number of positive open literals (posLitNum), 
		 a reference to the subsumer (sub) and the literals 
		 in the clause (lits).    
		 Holds a back reference to the array of clauses for deletion.
		 If HORN_RELAXATION is defined, holds a back reference to
		 the index of non-horn clauses.
		 If LEARNING is defined, holds the stack index of the unit 
		 learned clauses (unitIdx) and a flag (learned) initilized
		 to 0 and stamped with the current level whenever the
		 clause is learned.] 
  SeeAlso       [SimProp_t Sim_Dll_t]
******************************************************************************/
struct SimClause {
  short         openLitNum;
  short         posLitNum;
  SimProp_t   * sub;
  int           backClauses;

  VdeclareShort(SimProp_t*, lits);

#ifdef HORN_RELAXATION
  int           backNhClauses;
#endif
#ifdef LEARNING
  short         backUnitLearned;
  short         learned;
#endif
};

/**Struct**********************************************************************
  Synopsis      [Holds data about a proposition.]
  Description   [Holds the name of the proposition (prop), the current 
                 valuation (teta), the propagation type (mode), the level 
		 of assignment (level), and pointers to positive (posLits) 
		 and negative (negLits) occurrences of the proposition.
		 Holds a back reference to the proposition index and to
		 the model proposition index for deletion.
		 If BACKJUMPING is defined, it holds a pointer to the
		 reason (reason) that caused the assignment of 
		 the proposition. If LEARNING is defined it holds the
		 number of the occurrences in the original clauses, 
		 both positive (origPosSize) and negative (origNegSize).]
  SeeAlso       [SimClause_t, Sim_Dll_t]
******************************************************************************/
struct SimProp {
  int            prop; 
  char           teta;
  char           mode;
  short          level;
  int            backProps;
  int            backModelProps;

  Vdeclare(SimClause_t*, posLits);
  Vdeclare(SimClause_t*, negLits);

#ifdef BACKJUMPING
  SimClause_t  * reason;
#endif
};


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************
  Synopsis      [DLL data structure.]

  Description   [Holds the state and the control information of a DLL
                 module:
		 <ul>
		 <li> Indexes to propositions:
		     <ul>
		     <li> all the propositions (props);
		     <li> the search stack (stack);
		     <li> the proposition-to-pointer look up table (prop2ref)
		     <li> the original (modelProps) propopsitions;
		     <li> with PURE_LITERAL, the unpropagated pure literals;
		     </ul>
		 <li> Indexes to clauses:
		     <ul>
		     <li> all the clauses (clauses);
		     <li> unpropagated unit clauses (bcpStack);
		     <li> with HORN_RELAXATION, the non-Horn clauses;
		     <li> with LEARNING, learned and unit learned clauses
		     </ul>
		 <li> Parameters, counters and statistics: 
		     <ul>
		     <li> Counters array (cnt); 
		     <li> Statistics array (stat);
		     <li> Parameters array (param);
		     <li> Timers array (timer);
		     </ul>
		 <li> Additional components:
		     <ul>
		     <li> with BACKJUMPING, the working reason (wrLits),
		          its membership flags (litInWr) and the selected
			  conflict clause (conflictCl);
		     </ul>
		 <li> Auxiliary data:
		     <ul>
		     <li> A reference to auxiliary data (auxData); 
		     <li> Exit condition (failCond).
		     </ul>
		 </ul>

  SeeAlso       [SimProp_t  SimClause_t]
******************************************************************************/
VdeclareExt(SimProp_t*, SimProps);
VdeclareExt(SimProp_t*, SimStack);
VdeclareExt(SimProp_t*, SimProp2ref);
VdeclareExt(SimProp_t*, SimModelProps);
#ifdef PURE_LITERAL
VdeclareExt(SimProp_t*, SimMlfStack);
#endif
VdeclareExt(SimClause_t*, SimClauses);
VdeclareExt(SimClause_t*, SimBcpStack);
#ifdef HORN_RELAXATION
VdeclareExt(SimClause_t*, SimNhClauses);
#endif
#ifdef LEARNING
VdeclareExt(SimClause_t*, SimLearned);
VdeclareExt(SimClause_t*, SimUnitLearned);
#endif
extern int              SimParam[SIM_PARAM_NUM];
extern int              SimCnt[SIMCNT_NUM];
extern int              SimStat[SIMSTAT_NUM];
extern int              SimLapStat[SIMSTAT_NUM];
extern float            SimTimer[SIMTIMER_NUM];
#ifdef BACKJUMPING
VdeclareExt(SimProp_t*, SimWrLits);
VdeclareExt(int, SimLitInWr);
extern SimClause_t    * SimConflictCl;
#endif  
extern Sim_ErrCode_c     SimErrCode;
extern Sim_ErrLocation_c SimErrLocation;

/**Variable********************************************************************
  Synopsis     [Available heuristics.]
  Description  [Available heuristics.]
  SeeAlso      [Sim_Dll_t]
******************************************************************************/
extern SimPropFunPtr_t Simheuristics[SIM_HEUR_NUM];

/**Variable********************************************************************
  Synopsis     [Default parameters.]
  Description  [Default parameters.]
  SeeAlso      [Sim_Dll_t]
******************************************************************************/
extern int SimdefaultParam[SIM_PARAM_NUM];

/**Variable********************************************************************
  Synopsis     [Names of the statistics.]
  Description  [Names of the statistics.]
  SeeAlso      [Sim_Dll_t]
******************************************************************************/
extern char * SimstatName[SIMSTAT_NUM];

/**Variable********************************************************************
  Synopsis     [Names of the timers.]
  Description  [Names of the timers.]
  SeeAlso      [Sim_Dll_t]
******************************************************************************/
extern char * SimtimerName[SIMTIMER_NUM];


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* CONTROL MACROS. */

/**Macro***********************************************************************
  Synopsis     [Decides what kind of consistency check is to be performed.]
  Description  [Calls the function that checks if the formula is empty, i.e., 
                if the number of open clauses is 0. If HORN_RELAXATION is 
		defined, calls the function that checks if the number of 
		open non-Horn clauses is 0.]
  SideEffects  [The function assigns a value to s]
  SeeAlso      []
******************************************************************************/
#ifdef HORN_RELAXATION
#define SimDllCheckConsistency(s) SimDllGetModelsHorn(s)
#else
#define SimDllCheckConsistency(s) SimDllGetModels(s)
#endif

/**Macro***********************************************************************
  Synopsis     [Selects how to propagate the proposition.]
  Description  [Calls SimDllExtendPropTT or SimDllExtendPropFF, according
                to the value of sign.]
  SideEffects  []
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimDllExtendProp(p, teta, mode)					   \
(teta == SIM_TT ? SimDllExtendPropTT(p, mode) : SimDllExtendPropFF(p, mode))

/**Macro***********************************************************************
  Synopsis     [Selects how to propagate the proposition.]
  Description  [Calls SimDllExtendPropTT or SimDllExtendPropFF, according
                to the value of sign.]
  SideEffects  []
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimDllRetractProp(p)				   	\
(p -> teta == SIM_TT ? 						\
 SimDllRetractPropTT(p) : SimDllRetractPropFF(p))

/**Macro***********************************************************************
  Synopsis     [Calls the selected heuristics.]
  Description  [Call the selected heuristics.]
  SideEffects  [The heuristics assigns a value to s and m.]
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimDllChooseLiteral(s, m)                    \
Simheuristics[SimParam[SIM_HEURISTICS]](s, m)

/**Macro***********************************************************************
  Synopsis     [Selects the backtracking function.]
  Description  [If BACKJUMPING is defined calls Backjumping instead of
                Chronological backtracking.]
  SideEffects  [The function assigns a value to s and m.]
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#ifdef BACKJUMPING
#define SimDllBacktrack(s, m) SimDllBackjumping(s, m)
#else
#define SimDllBacktrack(s, m) SimDllChronoBt(s, m)
#endif

/**Macro***********************************************************************
  Synopsis     [When to call MLF.]
  Description  [If PURE_LITERAL is defined calls SimDllMlf.]
  SideEffects  []
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#ifdef PURE_LITERAL
#define SimDllDoMlf SimDllMlf()
#else
#define SimDllDoMlf
#endif


/* UTILITY MACROS. */

/**Macro***********************************************************************
  Synopsis     [Tests if the clause is open.]
  Description  [Tests if the clause is open.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define SimClauseIsOpen(c) (c -> sub == 0)

/**Macro***********************************************************************
  Synopsis     [Checks if a solution is found.]
  Description  [Checks if the number of open clauses is greater than 0.
                If HORN_RELAXATION is defined, checks if the number of
		open non-Horn clauses is greater than 0.]
  SideEffects  [none]
  SeeAlso      [SimDllMlf]
******************************************************************************/
#ifdef HORN_RELAXATION
#define SimDllFormulaIsEmpty \
(SimCnt[SIMCUR_NHCL_NUM] == 0)
#else
#define SimDllFormulaIsEmpty \
(SimCnt[SIMCUR_CL_NUM] == 0)
#endif

/**Macro***********************************************************************
  Synopsis     [Swaps two scalars.]
  Description  [Swaps two scalars.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define SimSwap(a, b, s)\
s = a; a = b; b = s

/**Macro***********************************************************************
  Synopsis     [Return the absolute value of an integer.]
  Description  [Return the absolute value of an integer.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define SimAbs(x)\
( x >= 0 ? x : ~(x)+1)

/**Macro***********************************************************************
  Synopsis     [Bit annotarion in proposition pointers.]
  Description  [Bit annotation in proposition pointers.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define SimIsPos(p) (((int)p & 0x80000000) == 0)
#define SimRef(p)   ((SimProp_t*)((int)p & 0x7fffffff))
#define SimMkNeg(p) ((SimProp_t*)((int)p | 0x80000000))


/* MEMORY RELATED MACROS */

/**Macro***********************************************************************
  Synopsis     [Remove a clause. ]
  Description  [Remove a clause.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define SimDllFreeClause(c)                     \
SimDllFree(c -> lits, (c -> lits[0] + 1) * sizeof(int));\
SimDllFree(c, sizeof(SimClause_t))

/**Macro***********************************************************************
  Synopsis     [Sim memory checking macro.]
  Description  [If the second argument is NULL raises the exception
                SIM_MEMORY_ERROR using the library own exception handling.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define SimDllMemCheck(ptr, message)					\
if ((ptr) == NULL) SimDllThrow(SIM_MEMORY_ERROR, SIM_NO_LOCATION, message)


/* RANDOM NUMBER GENERATION MACROS. */

/**Macro***********************************************************************
  Synopsis     [Returns a long integer in the range [0, x-1].]
  Description  [Returns a long integer in the range [0, x-1].]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#if HAVE_RANDOM
#define SimRndNumber(x)\
  (random() % x)
#else
#define SimRndNumber(x)\
  (rand() % x)
#endif


/**Macro***********************************************************************
  Synopsis     [Stores the random generation seed.]
  Description  [Stores the random generation seed.]
  SideEffects  [If `x' is zero and `getpid()' is available, x is set
                to the return value of `getpid()'.]
  SeeAlso      []
******************************************************************************/
#if HAVE_SRANDOM
# if HAVE_GETPID
#   define SimRndSeed(x)\
      srandom((unsigned int) (x = (x == 0 ? getpid() : x)))
# else 
#  define SimRndSeed(x)\
      srandom((unsigned int) (x = (x == 0 ? 1 : x)))
# endif 

#else /* ! HAVE_SRANDOM */
# if HAVE_GETPID
#   define SimRndSeed(x)\
      srand((unsigned int) (x = (x == 0 ? getpid() : x)))
# else 
#  define SimRndSeed(x)\
      srand((unsigned int) (x = (x == 0 ? 1 : x)))
# endif 
#endif

/* BENCHMARK MACROS */
#ifdef BENCHMARK

/**Macro***********************************************************************
  Synopsis     [Increments a statistic.]
  Description  [Increments a statistic.]
  SideEffects  [none]
  SeeAlso      [SimDllStatDec]
******************************************************************************/
#define SimDllStatInc(stat)		\
SimStat[stat]++

/**Macro***********************************************************************
  Synopsis     [Adds a given number to a statistic.]
  Description  [Adds a given number to a statistic.]
  SideEffects  [none]
  SeeAlso      [SimDllStatDec]
******************************************************************************/
#define SimDllStatAdd(stat, num)		\
SimStat[stat] += num

/**Macro***********************************************************************
  Synopsis     [Decrements a statistic.]
  Description  [Decrements a statistic.]
  SideEffects  [none]
  SeeAlso      [SimDllStatInc]
******************************************************************************/
#define SimDllStatDec(stat)		\
SimStat[stat]--

/**Macro***********************************************************************
  Synopsis     [Updates a (maximum) statistic.]
  Description  [Compares the current value with the previous value
                and updates the information when the current value is bigger.]
  SideEffects  [none]
  SeeAlso      [SimDllStatInc]
******************************************************************************/
#define SimDllStatUpdateMax(stat, cur)		\
if (cur > SimStat[stat]) SimStat[stat] = cur

/**Macro***********************************************************************
  Synopsis     [Updates a (minimum) statistic.]
  Description  [Compares the current value with the previous value
                and updates the information when the current value is smaller.]
  SideEffects  [none]
  SeeAlso      [SimDllStatInc]
******************************************************************************/
#define SimDllStatUpdateMin(stat, cur)		\
if (cur < SimStat[stat]) SimStat[stat] = cur

#else

/* If BENCHMARK is undefined, these macros are empty. */
#define SimDllStatInc(stat)		
#define SimDllStatAdd(stat, num)
#define SimDllStatDec(stat)	
#define SimDllStatUpdateMax(stat, cur)
#define SimDllStatUpdateMin(stat, cur)

#endif
	

/* CHECK MACROS */
#ifdef CHECK

/**Macro***********************************************************************
  Synopsis     [Sim `assert' macro.]
  Description  [If the condition is not fullfilled raises the exception
                SIM_INTERNAL_ERROR using the library own exception handling.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define SimDllAssert(cond, location, message)				\
if (!(cond)) SimDllThrow(SIM_INTERNAL_ERROR, location, message)

#else 

/* If CHECK is undefined, these macros are empty. */
#define SimDllAssert(cond, location, where)

#endif


/* TRACE MACROS */
#ifdef TRACE

/**Macro***********************************************************************
  Synopsis     [Sim trace enabler.]
  Description  [Calls the function only if TRACE is defined.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define SimOnTraceDo(funCall) funCall()

#else 

/* If TRACE is undefined, the macros is empty. */
#define SimOnTraceDo(funCall)

#endif


/* DEBUG MACROS. */
#ifdef DEBUG

/**Macro***********************************************************************
  Synopsis     [Prints a message about a split.]
  Description  [Takes a proposition, a value and a level.]
  SideEffects  [none]
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimLetPropHaveValue(p, v, l)                    \
printf("Now let [%d] have value %s at level {%d}\n",    \
       p -> prop, (v > 0 ? "TT" : "FF"), l)

/**Macro***********************************************************************
  Synopsis     [Prints a message about an heuristics choice.]
  Description  [Takes a proposition, the heuristics name and a weight.]
  SideEffects  [none]
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimChosenWithWeight(p, n, w)                                    \
printf("%s heuristics chooses %d with weight %d", n, p -> prop, w)

/**Macro***********************************************************************
  Synopsis     [Prints a message about an implied assignment.]
  Description  [Takes a proposition, a value, a level and the reason for the
                implied assignment (e.g., unit, pure literal, etc.).]
  SideEffects  [none]
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimPropHasValueBy(p, l, s)                \
printf("  [%d] has value %s by %s at level {%d}\n",   \
       p -> prop, (p -> teta == SIM_TT ? "TT" : "FF"), s, l)

/**Macro***********************************************************************
  Synopsis     [Prints a message about retracting a valuation.]
  Description  [Takes a proposition, a value and a level.]
  SideEffects  [none]
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimRetractingProp(p, l)                      \
printf("  Retracting [%d] at level {%d}\n", p -> prop, l)

/**Macro***********************************************************************
  Synopsis     [Prints a message about a skipped choice point.]
  Description  [Takes a proposition and a level.]
  SideEffects  [none]
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimSkippingProp(p, l)                   		\
if (p -> mode == SIMLSPLIT) 					\
  printf("  Skipping [%d] at level {%d}\n", p -> prop, l)

/**Macro***********************************************************************
  Synopsis     [Prints a message about a contradiction.]
  Description  [Takes a proposition.]
  SideEffects  [none]
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimContradictionFoundOn(p, l)                                   \
printf("  Contradiction found on [%d] at level {%d}\n", p -> prop, l)

/**Macro***********************************************************************
  Synopsis     [Prints a message about the creation of a new working reason.]
  Description  [Takes a list of literals and a temporary index.]
  SideEffects  [none]
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimNewWrFrom1(cl)						\
{									\
  int i;								\
  printf("  Resolving the working reason with:\n    ");			\
  for(i = 0; i < Vsize(cl -> lits); i++) {				\
    if (SimIsPos(V(cl -> lits)[i])) {					\
      printf("%d ", V(cl -> lits)[i] -> prop);		         	\
    } else {								\
      printf("-%d ", SimRef(V(cl -> lits)[i]) -> prop);	        	\
    }									\
  }									\
  printf("\n");								\
}

/**Macro***********************************************************************
  Synopsis     [Prints a message about the current working reason.]
  Description  [Takes a list of literals and a temporary index.]
  SideEffects  [none]
  SeeAlso      [Sim_DllSolve]
******************************************************************************/
#define SimWrIs(lits)						\
{								\
  int i;							\
  printf("  The current working reason is:\n    ");		\
  for(i = 0; i < Vsize(lits); i++) {				\
    if (SimIsPos(V(lits)[i])) {	             			\
      printf("%d ", V(lits)[i] -> prop);         		\
    } else {							\
      printf("-%d ", SimRef(V(lits)[i]) -> prop);	        \
    }								\
  }								\
  printf("\n");							\
}

/**Macro***********************************************************************
  Synopsis     [Prints a message about learning a clause.]
  Description  [Takes parameters for SimDllPrintClause.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define SimDllLearningClause(c)                    \
printf("  Learning the clause:\n    ");   \
SimDllPrintClause(c);                              \
printf("\n")

#else

/* If DEBUG is undefined, these macros are empty. */
#define SimLetPropHaveValue(p, v, l)
#define SimChosenWithWeight(p, n, w)                                    
#define SimPropHasValueBy(p, l, s)                
#define SimRetractingProp(p, l)                      
#define SimSkippingProp(p, l)                   
#define SimContradictionFoundOn(p, l)                                   
#define SimNewWrFrom2(lits1, lits2)                             
#define SimNewWrFrom1(lits)                                             
#define SimWrIs(lits)                                                   
#define SimDllLearningClause(c)                    

#endif

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN SimClause_t* SimClauseInit(int clId, short clSize);
EXTERN void SimClauseClear(SimClause_t * cl);
EXTERN int SimDllGetModels(int * stop);
EXTERN int SimDllGetModelsHorn(int * stop);
EXTERN void SimDllBuild();
EXTERN SimProp_t * SimDllInsProp(int lit, SimClause_t * cl);
EXTERN void SimDllDelProp(SimProp_t * p);
EXTERN void SimDllThrow(Sim_ErrCode_c errCode, Sim_ErrLocation_c errLoc, char * message);
EXTERN SimProp_t * SimDllUsrHeur(int * sign, int * mode);
EXTERN SimProp_t * SimDllRndHeur(int * sign, int * mode);
EXTERN SimProp_t * SimDllJWHeur(int * sign, int * mode);
EXTERN SimProp_t * SimDll2JWHeur(int * sign, int * mode);
EXTERN SimProp_t * SimDllBoehmHeur(int * sign, int * mode);
EXTERN SimProp_t * SimDllMomsHeur(int * sign, int * mode);
EXTERN void SimDllInitSatzHeur();
EXTERN void SimDllEndSatzHeur();
EXTERN SimProp_t * SimDllSatzHeur(int * sign, int * mode);
EXTERN void SimDllInitSatoHeur();
EXTERN void SimDllEndSatoHeur();
EXTERN SimProp_t * SimDllSatoHeur(int * sign, int * mode);
EXTERN void SimDllInitRelsatHeur();
EXTERN void SimDllEndRelsatHeur();
EXTERN SimProp_t * SimDllRelsatHeur(int * sign, int * mode);
EXTERN void SimDllInitUnitieHeur();
EXTERN void SimDllEndUnitieHeur();
EXTERN SimProp_t * SimDllUnitieHeur(int * sign, int * mode);
EXTERN int SimDllBcp();
EXTERN void SimDllMlf();
EXTERN SimProp_t * SimDllChronoBt(int * sign, int * mode);
EXTERN SimProp_t * SimDllBackjumping(int * sign, int * mode);
EXTERN void SimDllInitWr(SimClause_t * cl);
EXTERN void SimDllResolveWithWr(SimClause_t * cl, SimProp_t * pRes);
EXTERN SimClause_t * SimDllMakeClauseFromWr(int optimize);
EXTERN void SimDllLearnClause(SimClause_t * cl);
EXTERN void SimDllUnlearnClause(SimClause_t * cl);
EXTERN void SimDllStartMemory(int i);
EXTERN void SimDllStopMemory(int i);
EXTERN void SimDllSetMemout(int mmout);
EXTERN void SimDllPrintStack();
EXTERN void SimDllPrintPath();
EXTERN void SimDllPrintOpen();
EXTERN void SimDllPrintFormulaStruct();
EXTERN void SimDllPrintLearnedFW(int n);
EXTERN void SimDllPrintLearnedBW(int n);
EXTERN void SimDllPrintClause(SimClause_t * cl);
EXTERN void SimDllParseDimacsCls(FILE * inFile);
EXTERN SimProp_t* SimPropInit(int prop, short litSize);
EXTERN void SimPropClear(SimProp_t * p);
EXTERN int SimDllExtendPropTT(SimProp_t * p, int mode);
EXTERN int SimDllExtendPropFF(SimProp_t * p, int mode);
EXTERN void SimDllRetractPropTT(SimProp_t * p);
EXTERN void SimDllRetractPropFF(SimProp_t * p);
EXTERN int SimDllCheck(int * redundant);
EXTERN void SimDllStartTimer(int timer);
EXTERN void SimDllStopTimer(int timer);
EXTERN void SimDllSetTimeout(int tmout);
EXTERN void SimDllTraceNodes();
EXTERN void SimDllTraceLeafs();

/**AutomaticEnd***************************************************************/

#endif /* _SIMINT */
