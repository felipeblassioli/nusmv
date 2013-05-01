/**CFile***********************************************************************

  FileName    [simHeur.c]

  PackageName [sim]

  Synopsis    [Search heuristics.]

  Description [External procedures included in this module:
		<ul>
		<li> <b>SimDllUsrHeur()</b> prompts the user
		<li> <b>SimDllRndHeur()</b> chooses a literal randomly;

		<li> <b>SimDllJWHeur()</b>         classical Jeroslow-Wang; 
		<li> <b>SimDll2JWHeur()</b>        two-sided Jeroslow-Wang;
        	<li> <b>SimDllBoehmHeur()</b>      Boehm heuristics; 
		<li> <b>SimDllMomsHeur()</b>       Moms heuristics;

		<li> <b>SimDllInitSatzHeur()</b>
		<li> <b>SimDllSatzHeur()</b>       Satz heuristics;
		<li> <b>SimDllEndSatzHeur()</b> 

                <li> <b>SimDllInitSatoHeur()</b> 
		<li> <b>SimDllSatoHeur()</b>       Sato 3.2 heuristics;
                <li> <b>SimDllEndSatoHeur()</b> 

		<li> <b>SimDllInitRelsatHeur()</b>
		<li> <b>SimDllRelsatHeur()</b>     Relsat 2.0 heuristics;
		<li> <b>SimDllEndRelsatHeur()</b> 

		<li> <b>SimDllInitUnitieHeur()</b>     
		<li> <b>SimDllUnitieHeur()</b>     Simple unit based     
		<li> <b>SimDllEndUnitieHeur()</b>     
		</ul>]
		
  SeeAlso     [simSolve.c]

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

/*  #define LOG   */
/*  #define LOG2  */
/*  #define LOG3 */

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* User heuristics. */
#define USR_BUF_MAX       15  /* Max characters in a literal. */ 

/* JW heuristics. */
#define JW_MAX             6  /* 6 literals or more have weight 1.*/

/* Sato heuristics. */
#define SATO_MAGIC         7  /* Maximum number of open non horn clauses (and\
			         open literals) to consider. */

/* Satz heuristics. */
#define SATZ_T            10  /* Minimum number of proposition to consider at\
		                 each node. */
#define SATZ_WEIGTH        5  /* Weight of occurrences in binary clauses. */

/* Boehm's heuristics. */
#define BOEHM_ALPHA        1  
#define BOEHM_BETA         2  

/* Relsat heuristics. */
#define RELSAT_FUDGEFACT 0.9  /* 90% of the best variable is best. */

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************
  Synopsis      [Data structure for Satz heuristics.]
  Description   [Holds state and control information used by Satz heuristics. 
                 For each proposition: 
		 <ul>
		 <li> the number of binary and ternary clauses where it 
		      occurs negatively (negClLength2, negClLength3);
		 <li> the number of binary and ternary clauses where it 
		      occurs positively (posClLength2, posClLength3);
	         <li> the number of clauses decreasing in size when assigning 
		      the variable (reduceIfPos and reduceIfNeg);
                 </ul>
		 plus the following data:
		 <ul>
		 <li> changed clause references (managedCls); 
		 <li> changed propositions (changedProps)
		 <li> selected propositions and index (chosenProps)
		 </ul>]
******************************************************************************/
static int           * NegClLength2;
static int           * PosClLength2;
static int           * NegClLength3;
static int           * PosClLength3;
static int           * ReducedIfPos;
static int           * ReducedIfNeg;
Vdeclare(SimClause_t*, ManagedCls);
Vdeclare(SimProp_t*,   ChangedProps);
Vdeclare(SimProp_t*,   ChosenProps);


/**Variable********************************************************************
  Synopsis      [Data structure for Sato heuristics.]
  Description   [Holds some control information used by Sato heuristics:
		 <ul>
	  	         <li> a field that indicates the
			      kind of heuristics used (useMomsHeur);
			 <li> a stack of selected propositions (selectedProps);
                         <li> a field that indicates the sign assigned to
			      the selected proposition when there are more
			      positive occurrences than negative ones.
		 </ul>]
******************************************************************************/
static int           UseMomsHeur;
static int           SignPos;
#ifdef HORN_RELAXATION
static SimProp_t   * SelectedProps[SATO_MAGIC + 1];
#endif

/**Variable********************************************************************
  Synopsis      [Data structure for Relsat's heuristics.]
  Description   [Holds some control information used by Relsat's heuristics:
		 <ul>
         	 <li> scores of the propositions (scorePos, scoreNeg);
		 <li> combined scores (score); 
		 <li> positive and negative binary occ. (Pos/NegBinOcc)
		 </ul>]
******************************************************************************/
static int           * ScorePos;
static int           * ScoreNeg;
static int           * Score;
static int           * PosBinOcc;
static int           * NegBinOcc;

/**Variable********************************************************************
  Synopsis      [Data structure for Unitie heuristics.]
  Description   [Holds some control information used by Unitie heuristics:
		 <ul>
         	 <li> the list of best variables (bestProps);
		 </ul>]
******************************************************************************/
Vdeclare(SimProp_t*, bestProps);

/**Variable********************************************************************
  Synopsis     [Available heuristics.]
  Description  [Available heuristics.]
  SideEffects  [none]
  SeeAlso      [Sim_Dll_t]
******************************************************************************/
SimPropFunPtr_t Simheuristics[SIM_HEUR_NUM] = {
  SimDllUsrHeur,
  SimDllRndHeur,
  SimDllJWHeur,
  SimDll2JWHeur,
  SimDllSatoHeur,
  SimDllSatzHeur,
  SimDllBoehmHeur,
  SimDllMomsHeur,
  SimDllRelsatHeur,
  SimDllUnitieHeur
};


/**Variable********************************************************************
  Synopsis     [Available heuristics names.]
  Description  [Available heuristics names.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
char * Sim_heuristicsName[SIM_HEUR_NUM] = {
  "UsrHeur",
  "RndHeur",
  "JWHeur",
  "2JWHeur",
  "SatoHeur",
  "SatzHeur",
  "BoehmHeur",
  "MomsHeur",
  "RelsatHeur",
  "UnitieHeur"
};

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**Macro***********************************************************************
  Synopsis     [Assigns a truth value to a proposition and propagates it
                throughout the formula doing unit resolutions only.]
  Description  [Decrements the number of open literals (openLitNum) in
                each clause. If the clause does not become unary, pushes 
		its pointer into managedCls. If the clause becomes unary, 
		pushes it in the BCP stack.]
  SideEffects  [Every times that a unit clause is found, it is pushed into 
                Bcp stack, and localBcpIdx is incremented. If a contradiction
		is found, the variable localBcpIdx is set to -1, and the 
		propagation is stopped. Otherwise, score is
		incremented. ]
  SeeAlso      [LeanBcp]
******************************************************************************/
#define LeanExtendPropTT(p, extScoringOp)			\
VforceBack(ChangedProps, p);					\
p -> teta = SIM_TT;						\
Vforeach0(p -> negLits, gencl, cl) {				\
  if (SimClauseIsOpen(cl)) {					\
    VpushBackGrow(ManagedCls, cl, SIMLEARNED_GROW);		\
    length = --(cl -> openLitNum);				\
    extScoringOp(p, cl);					\
    if (length == 1) {						\
      V(SimBcpStack)[localBcpIdx++] = cl;			\
    } else  if (length == 0) {					\
      localBcpIdx = -1;						\
      break;							\
    }								\
  }								\
}								\
if ((localBcpIdx != -1) && ((gencl += 1) != 0)) {		\
  VforeachGen0(gencl, cl) {					\
    VpushBackGrow(ManagedCls, cl, SIMLEARNED_GROW);		\
    length = --(cl -> openLitNum);				\
    extScoringOp(p, cl);					\
    if (length == 1) {						\
      V(SimBcpStack)[localBcpIdx++] = cl;			\
    } else  if (length == 0) {					\
      localBcpIdx = -1;						\
      break;							\
    }								\
  }								\
}

#define LeanExtendPropFF(p, extScoringOp)			\
VforceBack(ChangedProps, p);					\
p -> teta = SIM_FF;						\
Vforeach0(p -> posLits, gencl, cl) {				\
  if (SimClauseIsOpen(cl)) {					\
    VpushBackGrow(ManagedCls, cl, SIMLEARNED_GROW);		\
    length = --(cl -> openLitNum);				\
    extScoringOp(p, cl);					\
    if (length == 1) {						\
      V(SimBcpStack)[localBcpIdx++] = cl;			\
    } else  if (length == 0) {					\
      localBcpIdx = -1;						\
      break;							\
    }								\
  }								\
}								\
if ((localBcpIdx != -1) && ((gencl += 1) != 0)) {		\
  VforeachGen0(gencl, cl) {					\
    VpushBackGrow(ManagedCls, cl, SIMLEARNED_GROW);		\
    length = --(cl -> openLitNum);				\
    extScoringOp(p, cl);					\
    if (length == 1) {						\
      V(SimBcpStack)[localBcpIdx++] = cl;			\
    } else  if (length == 0) {					\
      localBcpIdx = -1;						\
      break;							\
    }								\
  }								\
}

/**Macro***********************************************************************
  Synopsis     [Empty macro.]
  Description  [Empty macro.]
  SideEffects  [none]
  SeeAlso      [Examine Examine0]
******************************************************************************/
#define DoNothing1(arg)

/**Macro***********************************************************************
  Synopsis     [Empty macro.]
  Description  [Empty macro.]
  SideEffects  [none]
  SeeAlso      [Examine Examine0]
******************************************************************************/
#define DoNothing2(arg1, arg2)

/**Macro***********************************************************************
  Synopsis     [Increments the unit counter (unit).]
  Description  [Increments the unit counter (unit).]
  SideEffects  [none]
  SeeAlso      [Propagate]
******************************************************************************/
#define bcpScoringRelsat(q) unit += 1              

/**Macro***********************************************************************
  Synopsis     [Increments the unit counter (unit).]
  Description  [Increments the unit counter (unit).]
  SideEffects  [none]
  SeeAlso      [Propagate]
******************************************************************************/
#define bcpScoringUnitie(q) pos += 1              

/**Macro***********************************************************************
  Synopsis     [Increments the unit counter (unit).]
  Description  [Increments the unit counter (unit).]
  SideEffects  [none]
  SeeAlso      [Propagate]
******************************************************************************/
#define extScoringUnitie(q, cl)			\
if (cl -> openLitNum == 2) {			\
  if (q -> teta == SIM_TT) {			\
    posBin += 1;				\
  } else {					\
    negBin += 1;				\
  }						\
}

/**Macro***********************************************************************
  Synopsis     [Store q reason.]
  Description  [Store q reason.]
  SideEffects  [none]
  SeeAlso      [Propagate]
******************************************************************************/
#define StoreReason(q)                                         \
q -> reason = V(SimBcpStack)[localBcpIdx]


/**Macro***********************************************************************
  Synopsis     [Performs unit propagation.]
  Description  [Keeps propagating the unit clauses until an empty
                clause is found or there are no more unit clauses.
		ScoringOp and BacktrackOp are in the form 
		`void xxxOp(SimProp_t * p)' and define additional 
		operations to be done with q. ]
  SideEffects  [The function performing ExtendProp puts -1 into the index 
                of BCP stack when an empty clause is found. ]
  SeeAlso      [LeanExtendPropTT LeanExtendPropFF]
******************************************************************************/
#define LeanBcp(q, cl, bcpScoringOp, extScoringOp, BacktrackOp)	          \
while (localBcpIdx > 0) {                                     		  \
  cl = V(SimBcpStack)[--localBcpIdx];					  \
  Vforeach0(cl -> lits, genq, q) {					  \
    if (SimRef(q) -> teta == SIM_DC) {					  \
      break;								  \
    }									  \
  }									  \
  if (q == 0) {				        		 	  \
    continue;								  \
  }                                                           		  \
  if (SimIsPos(q)) {                                        		  \
    BacktrackOp(q);                                                       \
    bcpScoringOp(q);                                                      \
    LeanExtendPropTT(q, extScoringOp);				 	  \
  } else {   								  \
    q = SimRef(q);							  \
    BacktrackOp(q);                                                       \
    bcpScoringOp(q);                                                      \
    LeanExtendPropFF(q, extScoringOp);				 	  \
  }                                                           		  \
}                                                              

/**Macro***********************************************************************
  Synopsis     [Set the flags that witness an assignment.]
  Description  [Set the flags that witness an assignment.]
  SideEffects  [none]
  SeeAlso      [Propagate]
******************************************************************************/
#define ResetOpRelsat(q)			\
if (q -> teta == SIM_TT) {			\
   ScoreNeg[q -> prop] = -1;			\
} else {					\
   ScorePos[q -> prop] = -1;			\
}

/**Macro***********************************************************************
  Synopsis     [Resets propagations.]
  Description  [Restores the number of open propositions
		to the old value in the changed clauses.
		Unassigns all the propositions in changedProps.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define ResetProps(ResetOp)				\
while (Vsize(ManagedCls) > 0) {				\
  (VextractBack(ManagedCls) -> openLitNum) += 1;	\
}							\
while (Vsize(ChangedProps) > 0) {			\
  ResetOp(Vback(ChangedProps));				\
  Vback(ChangedProps) -> teta = SIM_DC;			\
  VdropBack(ChangedProps);				\
}

/**Macro***********************************************************************
  Synopsis     [Unassigns all the propositions in changedProps.
                Restores the number of open propositions
		to the old value in the changed clauses. Computes
		working reason while restoring the counters.]
  Description  [Unassigns all the propositions in changedProps.
                Restores the number of open propositions
		to the old value in the changed clauses Computes
		working reason while restoring the counters.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#ifdef BACKJUMPING
#define ResetPropsBj					\
while (Vsize(ManagedCls) > 0) {				\
  (VextractBack(ManagedCls) -> openLitNum) += 1;	\
}							\
while (Vsize(ChangedProps) > 1) {			\
  SimDllResolveWithWr(Vback(ChangedProps) -> reason,	\
		      Vback(ChangedProps));		\
  VextractBack(ChangedProps) -> teta = SIM_DC;		\
}							\
VextractBack(ChangedProps) -> teta = SIM_DC
#endif

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static SimProp_t * DllMomsHeur(int * sign, int * mode, SimProp_t ** propList, int signPos);
static int Examine(SimProp_t * p, int sign, int * reduced);
static int Examine0(SimProp_t * p, int sign, int * reduced);
static int Propagate(SimProp_t * p, int sign, int * score);
static int CountBinaryPos(SimProp_t * p);
static int CountBinaryNeg(SimProp_t * p);
static int CountNoBinaryPos(SimProp_t * p);
static int CountNoBinaryNeg(SimProp_t * p);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Asks the user for a literal.]

  Description [Displays all the open literals and asks the user to choose among
               them (mainly for debugging purposes).]

  SideEffects [none]

  SeeAlso     [Sim_DllSolve]

******************************************************************************/
SimProp_t *
SimDllUsrHeur(
  int       * sign,
  int       * mode)
{

  char litStr[USR_BUF_MAX];
  int  lit;

  lit = 0;
  printf("Please choose one of the following propositions:\n");
  SimDllPrintOpen();
  printf("i assigns the proposition to true, -i to false: ");
  fgets(litStr, USR_BUF_MAX, stdin); 
  if (sscanf(litStr,"%d",&lit) == 0) {
    SimDllThrow(SIM_INTERNAL_ERROR, SIM_HEUR_LOCATION, "SimDllUsrHeur");
  }  

  SimDllStatInc(SIMNODE_NUM);
  SimCnt[SIMCUR_LEVEL] += 1;

  /* Set sign and mode. */
  *sign = (lit > 0 ? SIM_TT : SIM_FF);
  *mode = SIMLSPLIT;

  return V(SimProp2ref)[SimAbs(lit)];

} /* End of SimDllUsrHeur. */


/**Function********************************************************************

  Synopsis    [Selects an open literal randomly]

  Description [Draws a number r from 0 to the number of current
               variables, i.e., open propositions. Then chooses the
	       r-th open proposition with a random sign.]

  SideEffects [none]

  SeeAlso     [Sim_DllSolve]

******************************************************************************/
SimProp_t * 
SimDllRndHeur(
  int        * sign,
  int        * mode)
{

  int          r;
  SimProp_t ** genp;
  SimProp_t  * p = 0;

  /* Draw a number [0:varNum - 1] */
  r = SimRndNumber(Vsize(SimProps));

  /* Search for the r-th open proposition (modulo the open propositions). */
  while (r) {
    Vforeach0(SimProps, genp, p) {
      if (p -> teta == SIM_DC) {
	r -= 1;
      }
    }
  }

  SimDllStatInc(SIMNODE_NUM);
  SimCnt[SIMCUR_LEVEL] += 1;

  /* Set sign and mode. */
  *sign = (SimRndNumber(2) ? SIM_TT : SIM_FF);
  *mode = SIMLSPLIT;

  return p;

} /* End of SimDllRndHeur. */


/**Function********************************************************************

  Synopsis    [Chooses the best literal with Jeroslow-Wang's  heuristics.]

  Description [For each open literal l compute the sum J(l)
               of the quantities 2^(JW_MAX - |C|) (which defaults to 1
	       if |C| > JW_MAX) for each clause C where l occurs. 
	       Branch to the literal l having the highest weight J(l).
	       Break ties using a position argument: 
	       the first literal that scores highest is the best
	       (From "Branching Rules for Satisfiability, by
	       J. N. Hooker and V. Vinay, Journal of Automated
	       Reasoning 15(3), 1995").] 

  SideEffects [None]

  SeeAlso     [Sim_DllSolve]

******************************************************************************/
SimProp_t * 
SimDllJWHeur(
  int        * sign,
  int        * mode)
{
 
  int            weight, posWeight, negWeight, bestWeight;
  SimProp_t    * bestProp;
  SimClause_t  * cl,  ** gencl; 
  SimProp_t    * p,   ** genp;
  
  SimOnTraceDo(SimDllTraceNodes);

  SimDllAssert((Vsize(SimBcpStack) == 0), SIM_HEUR_LOCATION,
	       "SimDllJWHeur: Bcp stack is not empty!");
  
  /* Initialize best weight and best proposition. */
  bestWeight = 0;
  bestProp = 0;

  /* Decide the index: model propositions or all the propositions. */
  if (SimParam[SIM_INDEP_PROPS]) {
    genp = V(SimModelProps);
  } else {
    genp = V(SimProps);
  }

  /* Cycle through open propositions to calculate scores. */
  VforeachGen0(genp, p) {
    if (p -> teta == SIM_DC) {
      /* Initialize positive and negative weight. */
      posWeight = negWeight = 0;
      /* Compute weight for positive literal. */
      Vforeach0(p -> posLits, gencl, cl) {
	if (SimClauseIsOpen(cl)) {
	  posWeight +=  
	    1 << (cl -> openLitNum > JW_MAX ? 
		  0 : JW_MAX - cl -> openLitNum);
	}
      }
      /* Compute weight for negative literal. */
      Vforeach0(p -> negLits, gencl, cl) {
	if (SimClauseIsOpen(cl)) {
	  negWeight +=  
	    1 << (cl -> openLitNum > JW_MAX ? 
		  0 : JW_MAX - cl -> openLitNum);
	}
      }
#ifdef PURE_LITERAL
      /* Detect pure literals. */
      if (((posWeight == 0) && (negWeight != 0)) ||
	  ((posWeight != 0) && (negWeight == 0))) {
	VforceBack(SimMlfStack, p);
	p -> mode = (negWeight != 0 ? SIMPURENEG : SIMPUREPOS);
      }
#endif
      /* Choose weight and sign. */
      weight = (posWeight > negWeight ? posWeight : negWeight);
      /* If the weight is better than the current best, update the
	 information. */
      if (weight > bestWeight) {
	bestWeight = weight;
	bestProp = p;
	*sign = (posWeight > negWeight ? SIM_TT : SIM_FF);
      } 
    }
  } /* VforeachGen0(genp, p) */
  
  SimCnt[SIMCUR_LEVEL] += 1;
  SimDllStatInc(SIMNODE_NUM);

  /* Set the mode. */
  *mode = SIMLSPLIT;

  return bestProp;

} /* End of SimDllJWHeur. */


/**Function********************************************************************

  Synopsis    [Chooses the best literal according to 2-sided JW.]

  Description [Assign weight to propositions according to the
               2 sided Jeroslow and Wang heuristics: JW(x) + JX(-x). 
	       Suggest positive sign if JW(x) >= JW(-x), 
	       negative sign otherwise.
	       Break ties using a position argument: the first
               proposition that scores highest is the best.  
	       (From "Branching Rules for
	       Satisfiability, by J. N. Hooker and V. Vinay, Journal
	       of Automated Reasoning 15(3), 1995").]

  SideEffects [none]

  SeeAlso     [Sim_DllSolve]

******************************************************************************/
SimProp_t *
SimDll2JWHeur(
  int        * sign,
  int        * mode)
{

  int            weight, posWeight, negWeight, bestWeight;
  SimProp_t    * bestProp;
  SimClause_t  * cl,  ** gencl; 
  SimProp_t    * p,   ** genp;

  SimOnTraceDo(SimDllTraceNodes);

  /* Initialize best weight and proposition. */
  bestWeight = 0;
  bestProp = 0;

  /* Decide the index: model propositions or all the propositions. */
  if (SimParam[SIM_INDEP_PROPS]) {
    genp = V(SimModelProps);
  } else {
    genp = V(SimProps);
  }

  /* Cycle through open propositions to calculate scores. */
  VforeachGen0(genp, p) {
    if (p -> teta == SIM_DC) {
      /* Initialize positive and negative weight. */
      posWeight = negWeight = 0;
      /* Compute weight for positive literal. */
      Vforeach0(p -> posLits, gencl, cl) {
	if (SimClauseIsOpen(cl)) {
	  posWeight +=  
	    1 << (cl -> openLitNum > JW_MAX ? 
		  0 : JW_MAX - cl -> openLitNum);
	}
      }
      /* Compute weight for negative literal. */
      Vforeach0(p -> negLits, gencl, cl) {
	if (SimClauseIsOpen(cl)) {
	  negWeight +=  
	    1 << (cl -> openLitNum > JW_MAX ? 
		  0 : JW_MAX - cl -> openLitNum);
	}
      }    
#ifdef PURE_LITERAL
      /* Detect pure literals. */
      if (((posWeight == 0) && (negWeight != 0)) ||
	  ((posWeight != 0) && (negWeight == 0))) {
	VforceBack(SimMlfStack, p);
	p -> mode = (negWeight != 0 ? SIMPURENEG : SIMPUREPOS);
      }
#endif
      /* Compute weight. */
      weight = posWeight + negWeight;
      /* If the weight is better than the current best, update the
	 information. */
      if (weight > bestWeight) {
	bestWeight = weight;
	bestProp = p;
	*sign = (posWeight >= negWeight ? SIM_TT : SIM_FF);
      } 
    }
  } /*  VforeachGen0(genp, p) */

  SimCnt[SIMCUR_LEVEL] += 1;
  SimDllStatInc(SIMNODE_NUM);

  /* Set the mode. */
  *mode = SIMLSPLIT;

  return bestProp;

} /* End of SimDll2JWHeur. */


/**Function********************************************************************

  Synopsis    [Choose the best literal according to Boehm's heuristics.]  

  Description [Select the proposition with the biggest weight vector
               in lexicopgraphic order. For each proposition x the
	       weight vector is (H(x),H'(x)), where 
	       <ul>
	       <li>H(x) = ALPHA * max(h(x), h(-x)) + 
	                  BETA * min(h(x), h(-x))
       	       <li>H'(x) = ALPHA * max(h'(x), h'(-x)) + 
	                   BETA * min(h'(x), h'(-x))
   	       </ul>
               and h(x) is the number of occurrences of x in the
	       shortest clauses, while h'(x) is the number of
	       occurrences in all the clauses. From "Report on a SAT
	       competition, by M.Buro and H. Kleine Buning, Bericht Nr.
	       110, Reihe Informatik, November 1992", in the spirit of
	       Boehm's solver version A 31/3/92.]

  SideEffects [None]

  SeeAlso     [Sim_DllSolve]

******************************************************************************/
SimProp_t *
SimDllBoehmHeur(
  int        * sign,
  int        * mode)
{

  SimClause_t  * cl, ** gencl; 
  SimProp_t    * p,  ** genp;
  int            length, weightAll, weight;
  int            posAll, negAll, pos, neg;


  SimProp_t * bestProp      = 0;
  int         minLength     = Vsize(SimProps);
  int         bestWeightAll = 0;
  int         bestWeight    = 0;

  SimOnTraceDo(SimDllTraceNodes);

  /* Decide the index: model propositions or all the propositions. */
  if (SimParam[SIM_INDEP_PROPS]) {
    genp = V(SimModelProps);
  } else {
    genp = V(SimProps);
  }

  /* Examine open propositions. */
  VforeachGen0(genp, p) {

    if (p -> teta == SIM_DC) {

      /* Check positive occurrences of the proposition. */
      pos = posAll = 0;
      Vforeach0(p -> posLits, gencl, cl) {
	/* Check if the clause is open. */
	if (SimClauseIsOpen(cl)) {
	  length = cl -> openLitNum;
	  posAll++;
	  if (length < minLength) {
	    /* If the current clause is shorter than the shortest one
	       then p is definitely the best proposition so far. */
	    bestProp = p;
	    minLength = length;
	    pos = 1;
	  } else if (length == minLength) {
	    /* If the current clause length is equal to the length of the
	       shortest one, increase pos. */
	    pos++;
	  }
	}
      } /* Vforeach0(p -> posLits, gencl, cl) */

      /* Check negative occurrences of the proposition. */
      neg = negAll = 0;
      Vforeach0(p -> negLits, gencl, cl) {
	/* Check if the clause is open. */
	if (SimClauseIsOpen(cl)) {
	  length = cl -> openLitNum;
	  negAll++;
	  if (length < minLength) {
	    /* If the current clause is shorter than the shortest one
	       then p is definitely the best proposition so far. */
	    bestProp = p;
	    minLength = length;
	    pos = 0;
	    neg = 1;
	  } else if (length == minLength) {
	    /* If the current clause length is equal to the length of the
	       shortest one, increase neg. */ 
	    neg++;
	  }
	}
      } /* Vforeach0(p -> negLits, gencl, cl) */
	
#ifdef PURE_LITERAL
      /* Detect pure literals. */
      if (((posAll == 0) && (negAll != 0)) ||
	  ((posAll != 0) && (negAll == 0))) {
	VforceBack(SimMlfStack, p);
	p -> mode = (negAll != 0 ? SIMPURENEG : SIMPUREPOS);
      }
#endif

      /* Compute the weights for the current proposition. */
      weight = 
	BOEHM_ALPHA * (pos > neg ? pos : neg) + 
	BOEHM_BETA * (pos > neg ? neg : pos);
      weightAll = 
	BOEHM_ALPHA * (posAll > negAll ? posAll : negAll) + 
	BOEHM_BETA * (posAll > negAll ? negAll : posAll);

      if (bestProp == p) {
	/* The current proposition is the best one since it occurred at
	   least in a clause shorter than the previous shortest clause. */
	*sign = (pos > neg ? SIM_TT : SIM_FF);
	bestWeight = weight; 
	bestWeightAll = weightAll;
      } else if ((pos > 0) || (neg > 0)) {
	if (weight > bestWeight) {
	  /* The current proposition is the best one since it occurred
	     at least in a clause as short as the previous shortest one
	     (but not in any shorter clause) and its weight is better
	     than the previous best weight. */  
	  bestProp = p;
	  *sign = (pos > neg ? SIM_TT : SIM_FF);
	  bestWeight = weight; 
	  bestWeightAll = weightAll;
	} else if ((weight == bestWeight) &&
		   (weightAll > bestWeightAll)) {
	  /* The current proposition is the best one since it occurred
	     at least in a clause as short as the previous shortest one
	     (but not in any shorter clause), its weight coincides with 
	     the previous best weight, and weightAll is bigger than the
	     previous best weightAll. */  
	  bestProp = p;
	  bestWeightAll = weightAll;
	  *sign = (pos > neg ? SIM_TT : SIM_FF);
	}
      }

    } /*  if (p -> teta == SIM_DC) */

  }  /*  VforeachGen0(genp, p) */

  SimCnt[SIMCUR_LEVEL] += 1;
  SimDllStatInc(SIMNODE_NUM);

  /* Set the mode. */
  *mode = SIMLSPLIT;

  return bestProp;

} /* End of SimDllBoehmHeur. */


/**Function********************************************************************

  Synopsis    [Chooses the best literal according to M.O.M.S. heuristics.]

  Description [An interface to DllMomsHeur.]

  SideEffects [None]

  SeeAlso     [SimDllSatoHeur]

******************************************************************************/
SimProp_t * 
SimDllMomsHeur(
  int               * sign,
  int               * mode)
{

  SimOnTraceDo(SimDllTraceNodes);

  return DllMomsHeur(sign, mode, 0, SIM_TT);

} /* End of SimDllMomsHeur. */


/* SATZ **********************************************************************/

/**Function********************************************************************

  Synopsis    [Allocate auxiliary data for Chu Min Li heuristics. ]  

  Description [Allocate auxiliary data for Chu Min Li heuristics. ]

  SideEffects [None]

  SeeAlso     [SimDllSatzHeur SimDllEndSatzHeur ]

******************************************************************************/
void 
SimDllInitSatzHeur()
{

  int          varNum;
  int        * base;

  /* Allocate all the integer vectors needed in one shot. */
  base = (int*) calloc(Vsize(SimProp2ref) * 6, sizeof(int));
  SimDllMemCheck(base, "SimDllInitSatzHeur");
  
  /* Assign memory to the integer vectors. */
  NegClLength2 = base; 
  NegClLength3 = NegClLength2 + Vsize(SimProp2ref);  
  PosClLength2 = NegClLength3 + Vsize(SimProp2ref);  
  PosClLength3 = PosClLength2 + Vsize(SimProp2ref);  
  ReducedIfPos = PosClLength3 + Vsize(SimProp2ref);  
  ReducedIfNeg = ReducedIfPos + Vsize(SimProp2ref);  
  
  VinitReserve(ManagedCls, SimCnt[SIMCL_NUM]);
  SimDllMemCheck(V(ManagedCls), "SimDllInitSatzHeur");

  varNum = Vsize(SimProps);
  VinitReserve(ChangedProps, varNum);
  SimDllMemCheck(V(ChangedProps), "SimDllInitSatzHeur");
  
  VinitReserve(ChosenProps, (varNum + 1));
  SimDllMemCheck(V(ChosenProps), "SimDllInitSatzHeur");
  
  return;
  
} /* End of SimDllInitSatzHeur. */


/**Function********************************************************************

  Synopsis    [Free auxiliary data for Chu Min Li heuristics. ]  

  Description [Free auxiliary data for Chu Min Li heuristics. ]

  SideEffects [None]

  SeeAlso     [SimDllInitSatzHeur SimDllSatzHeur]

******************************************************************************/
void 
SimDllEndSatzHeur()
{
  
  free(NegClLength2);
  Vclear(ManagedCls);
  Vclear(ChangedProps);
  Vclear(ChosenProps);

  return;

} /* End of SimDllEndSatzHeur */


/**Function********************************************************************

  Synopsis    [Choose the best literal according to Satz heuristics.]  

  Description [Compute the number of positive and
               negative occurrences for each open proposition. 
	       Then select all the propositions that have at least 
               4 binary occurrences, of which at least 1 positive and 1
               negative (PROP41). If there are less than T propositions 
	       that satisfy this condition, then select also the 
	       propositions that have at least 3 occurrences in binary
	       clauses, of which at least 1 positive  and 1 negative
	       (PROP31). If less than T propositions are collected,
	       reset what has been done so far and compute weights for
	       each open variable using BCP and a modified
	       Jeroslow-Wang weighting function. (From "Heuristics
	       Based on Unit Propagation for Satisfiability Problems,
	       by Chu Min Li and Anbulagan, proceedings of AAAI, 1999"
	       and in the spirit of SATZ, release September 1996).]

  SideEffects [None]

  SeeAlso     [Sim_DllSolve SimDllInitSatzHeur SimDllEndSatzHeur
               Examine Examine0]

******************************************************************************/
SimProp_t *
SimDllSatzHeur(
  int        * sign,
  int        * mode)
{

  int               pos3, pos2;
  int               neg3, neg2;
  SimProp_t       * p, ** genp;
  SimClause_t     * cl, ** gencl;
  SimProp_t       * bestProp;

  unsigned int      weight, bestWeight;

  SimOnTraceDo(SimDllTraceNodes);

  do{

    Vflush(ChosenProps);  
    bestWeight = 0;
    bestProp = 0;
    *sign = 0;


    /* Fill in statistics and check for PROP41. */
    Vforeach0(SimProps, genp, p) {

      if (p -> teta == SIM_DC) {

	/* Initialize the scores. */
	neg2 = neg3 = ReducedIfNeg[p -> prop] = 0; 
	pos2 = pos3 = ReducedIfPos[p -> prop] = 0; 

	/* Check positive occurrences of the proposition. */
	Vforeach0(p -> posLits, gencl, cl) {
	  if (SimClauseIsOpen(cl)) {
	    if (cl -> openLitNum > 2) {
	      pos3 += 1;
	    } else {
	      pos2 += 1;
	    }
	  }
	}
	PosClLength2[p -> prop] = pos2; 
	PosClLength3[p -> prop] = pos3; 
#ifdef PURE_LITERAL
	/* Eventualy propagate pure literals. */
	if ((pos3 + pos2) == 0) {
	  SimDllExtendProp(p, SIM_FF, SIMPURENEG);
	  SimDllStatInc(SIMPURE_NUM);
	  SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL],"pure literal");
	  /* Go to the next open proposition. */
	  continue;
	} 
#endif
	
	/* Check negative occurrences of the proposition. */
	Vforeach0(p -> negLits, gencl, cl) {
	  if (SimClauseIsOpen(cl)) {
	    if (cl -> openLitNum > 2) {
	      neg3 += 1;
	    } else {
	      neg2 += 1;
	    }
	  }
	}
	NegClLength2[p -> prop] = neg2; 
	NegClLength3[p -> prop] = neg3; 
#ifdef PURE_LITERAL
	/* Eventualy propagate pure literals. */
	if ((neg3 + neg2) == 0) {
	  SimDllExtendPropTT(p, SIMPUREPOS);
	  SimDllStatInc(SIMPURE_NUM);
	  SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL], "pure literal");
	  /* Go to the next open proposition. */
	  continue;
	} 
#endif

	/* Check if prop satisfies PROP41. */
	if ((neg2 > 0) && (pos2 > 0) && (neg2 + pos2) > 3) {
	  if (Examine(p, SIM_TT, ReducedIfPos) == SIM_UNSAT) {
	    /* If a contradiction is found, try the other assignment. */
	    SimDllExtendPropFF(p, SIMFAILED);
	    SimDllStatInc(SIMFAILED_NUM);
	    SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL], 
			      "failed literal in PROP41");
	    if (SimDllBcp() == SIM_UNSAT) {
	      /* If a second contradiction is found, stop and backtrack. */
	      return SimDllBacktrack(sign, mode);
	    }
	    SimDllAssert(Vsize(SimBcpStack) == 0, SIM_HEUR_LOCATION,  
			 "SimDllSatzHeur (PROP41): dirty stack on f.l.");
	    /* Go to the next open proposition. */
	    continue; 
	  } else if (Examine(p, SIM_FF, ReducedIfNeg) == SIM_UNSAT) {
	    /* If a contradiction is found, try the other assignment. */
	    SimDllExtendPropTT(p, SIMFAILED);
	    SimDllStatInc(SIMFAILED_NUM);
	    SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL],
			      "failed literal in PROP41");
	    /* The other side is already checked. */
	    SimDllBcp();
	    SimDllAssert(Vsize(SimBcpStack) == 0, SIM_HEUR_LOCATION, 
			 "SimDllSatzHeur (PROP41): dirty stack on f.l.");
	    /* Go to the next open proposition. */
	    continue; 	      
	  }
	  VforceBack(ChosenProps, p);
	} /* ((neg2 > 0) && (pos2 > 0) && (neg2 + pos2) > 3) */
     
      }  /* if (p -> teta == SIM_DC) */
 
    } /*   VforeachGen0(genp, p) */


    /* If less than SATZ_T variables were collected, try with PROP31. */
    if (Vsize(ChosenProps) < SATZ_T) {

      Vforeach0(SimProps, genp, p) {

	if (p -> teta == SIM_DC) {

	  /* Check that prop does not satisfy PROP41. */
	  if (ReducedIfNeg[p -> prop] == 0) {
	    pos2 = PosClLength2[p -> prop];
	    neg2 = NegClLength2[p -> prop];
	    /* Check that prop satisfies PROP31. */
	    if ((neg2 > 0) && (pos2 > 0) && ((neg2 > 1) || (pos2 > 1))) {
	      if (Examine(p, SIM_TT, ReducedIfPos) == SIM_UNSAT) {
		/* If a contradiction is found, try the other assingment. */
		SimDllExtendPropFF(p, SIMFAILED);
		SimDllStatInc(SIMFAILED_NUM);
		SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL],
                                "failed literal in PROP31");
		if (SimDllBcp() == SIM_UNSAT) {
		  /* If a contradiction is found, stop and backtrack. */
		  return SimDllBacktrack(sign, mode);
		}
		SimDllAssert(Vsize(SimBcpStack) == 0, SIM_HEUR_LOCATION, 
			     "SimDllSatzHeur (PROP31): dirty stack on f.l.");
		/* Go to the next open proposition. */
		continue; 	      
	      } else if (Examine(p, SIM_FF, ReducedIfNeg) == SIM_UNSAT) {
		/* If a contradiction is found, try the other assingment. */
		SimDllExtendPropTT(p, SIMFAILED);
		SimDllStatInc(SIMFAILED_NUM);
		SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL],
				  "failed literal in PROP31");
		/* The other side is already checked. */
		SimDllBcp();
		SimDllAssert(Vsize(SimBcpStack) == 0, SIM_HEUR_LOCATION, 
			     "SimDllSatzHeur (PROP31): dirty stack on f.l.");
		/* Go to the next open proposition. */
		continue; 	      
	      }
	      VforceBack(ChosenProps, p);
	    } /* ((neg2 > 0) && (pos2 > 0) && ((neg2 > 1) || (pos2 > 1))) */
	  } /* (ReducedIfNeg[p -> prop] == 0) */

	} /*  if (p -> teta == SIM_DC) */
      
      } /* Vforeach0(SimProps, genp, p) */
    
    }  /* if (Vsize(ChosenProps) < SATZ_T) */


    /* If less than SATZ_T variables were collected, try with PROP0. */
    if (Vsize(ChosenProps) < SATZ_T) {

      Vflush(ChosenProps);
      
      /* Decide the index: model propositions or all the propositions. */
      if (SimParam[SIM_INDEP_PROPS]) {
	genp = V(SimModelProps);
      } else {
	genp = V(SimProps);
      }

      /* Apply BCP to all the variables. */
      VforeachGen0(genp, p) {

	if (p -> teta == SIM_DC) {
      
	  if (Examine0(p, SIM_TT, ReducedIfPos) == SIM_UNSAT) {
	    /* If a contradiction is found, try the other assignment. */
	    SimDllExtendPropFF(p, SIMFAILED);
	    SimDllStatInc(SIMFAILED_NUM);
	    SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL],
			      "failed literal in PROP0");
	    if (SimDllBcp() == SIM_UNSAT) {
	      /* If a contradiction is found, stop and backtrack. */
	      return SimDllBacktrack(sign, mode);
	    }
	    /* Go to the next open proposition. */
	    SimDllAssert(Vsize(SimBcpStack) == 0, SIM_HEUR_LOCATION, 
			 "SimDllSatzHeur (PROP0): dirty stack on f.l.");
	    continue; 
	  } else if (Examine0(p, SIM_FF, ReducedIfNeg) == SIM_UNSAT) {
	    /* If a contradiction is found, try the other assignment. */
	    SimDllExtendPropTT(p, SIMFAILED);
	    SimDllStatInc(SIMFAILED_NUM);
	    SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL],
			      "failed literal in PROP0");
	    /* The other side is already checked. */
	    SimDllBcp();
	    /* Go to the next open proposition. */
	    SimDllAssert(Vsize(SimBcpStack) == 0, SIM_HEUR_LOCATION, 
			 "SimDllSatzHeur (PROP0): dirty stack on f.l.");
	    continue; 	      
	  }
	  VforceBack(ChosenProps, p);
	  
	}  /* if (p -> teta == SIM_DC) */

      } /* VforeachGen0(genp, p) */
      
    } /* if (Vsize(ChosenProps) < SATZ_T) */
	
    /* Examine chosen propositions and select one. */
    VforceBack(ChosenProps, 0);
    Vforeach0(ChosenProps, genp, p) {
      /* ReducedIfNeg * ReducedIfPos * 1024 + 
	 ReducedIfPos + ReducedIfNeg + 1 */
      neg2 = ReducedIfNeg[p -> prop];
      weight = ReducedIfPos[p -> prop] * ((neg2 << 10) + 1) + neg2 + 1;
      if (weight > bestWeight) {
	bestProp = p;
	bestWeight = weight;
      }
    } 

 
  } while((bestProp != 0) && (bestProp -> teta != SIM_DC));

  if (bestProp != 0) {
    SimDllStatInc(SIMNODE_NUM);
    SimCnt[SIMCUR_LEVEL] += 1;
    /* Fix the sign and the mode. */
    *sign = SIM_TT;
    *mode = SIMLSPLIT;
    return bestProp;
  }

  return bestProp;

} /* End of SimDllSatzHeur. */


/* SATO **********************************************************************/

/**Function********************************************************************

  Synopsis    [Allocate auxiliary data for Sato 3.2 heuristics.]  

  Description [Allocate auxiliary data for Sato 3.2 heuristics.]

  SideEffects [none]

  SeeAlso     [SimDllSatoHeur SimDllEndSatoHeur]

******************************************************************************/
void 
SimDllInitSatoHeur()
{

  int          rate;

  /* Select the heuristics looking at the percentage of non-Horn clauses. */
  if (SimCnt[SIMNHCL_NUM] > 2) {
    rate = (100 * SimCnt[SIMCL_NUM]) / (SimCnt[SIMNHCL_NUM] - 2);
    if (rate < 350) {
      /* If the percentage of non-Horn clauses is greater than 28.54% use
	 Moms heuristics. */
      UseMomsHeur = 1;
      SignPos = SIM_FF;
    } else {
      /* If the percentage of non-Horn clauses is smaller than 28.54% use
	 shortest non-Horn first heuristics. */
      UseMomsHeur = 0;
      if (rate >= 4242) {
      /* If the percentage of non-Horn clauses is smaller than 2.36%
	 invert the sign. */
	SignPos = SIM_TT;
      } else {
	SignPos = SIM_FF;
      }
    }
  } else {
    /* If there are less than 2 non-Horn clauses use Moms heuristics. */
    UseMomsHeur = 1;
    SignPos = SIM_FF;
  }

  return;

} /* End of SimDllInitSatoHeur */


/**Function********************************************************************

  Synopsis    [Free auxiliary data for Sato heuristics.]  

  Description [Free auxiliary data for Sato heuristics.]

  SideEffects [none]

  SeeAlso     [SimDllInitSatoHeur SimDllSatoHeur]

******************************************************************************/
void 
SimDllEndSatoHeur()
{

  return;

}/* End of SimDllEndSatoHeur */


/**Function********************************************************************

  Synopsis    [Choose the best literal according to Sato 3.2 heuristics. ]  

  Description [If the percentage of non-Horn clauses is less than
               certain rate: get the propositions that occur in the 
	       shortest non-horn clauses, and select the best literal 
	       according to MOMS heuristics. Select SATO_MAGIC open non-Horn
	       clauses and SATO_MAGIC open propositions at most.
	       If the percentage of non-Horn clauses is bigger
	       than the above rate or if there are less than two
	       non-Horn clauses use MOMS heuristics only. (From
	       "SATO: an Efficient Propostional Prover, by Hantao
	       Zhang, proceedings of CADE, 1996" and in the spirit of 
	       SATO version 3.2).]

  SideEffects [none]

  SeeAlso     [Sim_DllSolve MomsHeur SimDllInitSatoHeur SimDllEndSatoHeur]

******************************************************************************/
SimProp_t *
SimDllSatoHeur(
  int        * sign,
  int        * mode)
{

  SimOnTraceDo(SimDllTraceNodes);

#ifdef HORN_RELAXATION
  SimClause_t       * cls[SATO_MAGIC];
  SimClause_t       * cl;
  SimProp_t         * p, ** genp;
  int                 i;

  int                 minLength  = Vsize(SimProps);
  int                 totalProps = 0;
  int                 totalCls   = 0;

  if (UseMomsHeur == 0) {
    
    /* Cycle on the non-horn clauses and select the shortest ones. */
    for (i = 0; i < Vsize(SimNhClauses); i++) {
      cl = V(SimNhClauses)[i];
      /* If the clause is not open, skip it. */
      if (SimClauseIsOpen(cl)) {
	if (cl -> openLitNum < minLength) {
	  /* The current clause is shorter than the current shortest clause. */
	  /* Get this clause length as the current shortest length. */
	  minLength = cl -> openLitNum;
	  /* Discard all the clauses collected up to now, but this one. */
	  cls[0] = cl;
	  totalCls = 1;
	} else {
	  /* If the current clause has the same length of the shortest one */
	  /* add it to the set. */
	  if ((totalCls < SATO_MAGIC) && ( cl -> openLitNum == minLength)) {
	    cls[totalCls++] = cl;
	  }
	}
      }
    }/* for (i = 0; i < Vsize(SimNhClauses); i++) */
    
    /* Get the propositions in the selected clauses. */
    for (i = totalCls - 1; (i >= 0) && (totalProps < SATO_MAGIC); i--) {
      /* Get the pointer to the clause and the number of literals. */
      cl = cls[i];
      /* Collect SATO_MAGIC propositions at most. */
      Vforeach0(cl -> lits, genp, p) {
	/* If the current proposition was not selected before, */
	/* put it in the proposition set, and set the flag.  */
	p = SimRef(p);
	if (p -> teta == SIM_DC) {
	  p -> teta = -2;
	  SelectedProps[totalProps++] = p;
	  if  (totalProps == SATO_MAGIC) {
	    break;
	  }
	}
      } 
    } /* (i < totalCls) && (totalProps < MAGIC) */
    
    /* Terminate the selected propositions. */
    SelectedProps[totalProps] = 0;
    
    /* Restore the proposition flags. */
    for (genp = SelectedProps, p = *genp; p != 0; p = *(++genp)) {
      p -> teta = SIM_DC;
    }

    return DllMomsHeur(sign, mode, SelectedProps, SignPos);

  } else 
#endif
    /* If the selected heuristics is Moms, call MomsHeur (0 tells MomsHeur
       to collect all open variables). */
    return DllMomsHeur(sign, mode, 0, SignPos);

} /* End of SimDllSatoHeur. */


/* RELSAT ********************************************************************/

/**Function********************************************************************

  Synopsis    [Allocate auxiliary data for Relsat heuristics.]  

  Description [Allocate auxiliary data for Relsat heuristics.]

  SideEffects [none]

  SeeAlso     [SimDllRelsatHeur SimDllEndRelsatHeur]

******************************************************************************/
void 
SimDllInitRelsatHeur()
{

  int            varNum;
  int          * base;

  /* Get the number of variables and the maximum index. */

  /* Allocate all the integer vectors needed in one shot. */
  base = (int*) calloc(Vsize(SimProp2ref) * 5, sizeof(int));
  SimDllMemCheck(base, "SimDllInitRelsatHeur");
  
  /* Assign memory to the integer vectors. */
  ScorePos  = base;
  ScoreNeg  = ScorePos + Vsize(SimProp2ref);
  Score     = ScoreNeg + Vsize(SimProp2ref);
  PosBinOcc = Score + Vsize(SimProp2ref);
  NegBinOcc = PosBinOcc + Vsize(SimProp2ref);

  VinitReserve(ManagedCls, SimCnt[SIMCL_NUM]);
  SimDllMemCheck(V(ManagedCls), "SimDllInitRelsatHeur");

  varNum = Vsize(SimProps);
  VinitReserve(ChangedProps, varNum);
  SimDllMemCheck(V(ChangedProps), "SimDllInitRelsatHeur");

  VinitReserve(ChosenProps, (varNum + 1));
  SimDllMemCheck(V(ChosenProps), "SimDllInitRelsatHeur");

  return;
  
} /* End of SimDllInitRelsatHeur */


/**Function********************************************************************

  Synopsis    [Free auxiliary data for Relsat heuristics.]  

  Description [Free auxiliary data for Relsat heuristics.]

  SideEffects [None]

  SeeAlso     [SimDllInitRelsatHeur, SimDllRelsatHeur]

******************************************************************************/
void 
SimDllEndRelsatHeur()
{

  free(ScorePos);
  Vclear(ManagedCls);
  Vclear(ChangedProps);
  Vclear(ChosenProps);

  return;

} /* End of SimDllEndRelsatHeur. */


/**Function********************************************************************

  Synopsis    [Chooses the best literal according to Relsat 2.0 heuristics. ] 

  Description [Scores the open propositions according to the number of their
               binary occurrences. If a proposition fullfills some additional
	       requirements then it is propagated and failed assignments are 
	       detected. If no failed literal is detected, a pool of variables
	       that scores in within RELSAT_FUDGEFACT of the best weight is 
	       collected. A variable and a sign are then chosen randomly. (In 
	       the spirit of Relsat 2.0 code)]

  SideEffects [none]

  SeeAlso     [Sim_DllSolve]

******************************************************************************/
SimProp_t *
SimDllRelsatHeur(
  int        * sign,
  int        * mode)
{
  int            scoreTT, scoreFF;
  float          fMin;
  SimProp_t    * p, ** genp;
  SimProp_t    * q, ** genq;
  
  float          fBest    = -1.0;
  int            noBinary = 1;
  SimProp_t    * bestProp = 0;

  SimOnTraceDo(SimDllTraceNodes);

  /* Initialize data. */
  Vflush(ChosenProps);

  /* Analyze all open propositions. */
  Vforeach0(SimProps, genp, p) {

    if (p -> teta == SIM_DC) {
      /* Collect the number of positive occurrences in binary clauses. */
      PosBinOcc[p -> prop] = CountBinaryPos(p); 
      NegBinOcc[p -> prop] = CountBinaryNeg(p);  
      
      if (PosBinOcc[p -> prop] || NegBinOcc[p -> prop]) {
        noBinary = 0;
      }

      /* Compute ScoreFF. */

      /* If p satisfies the conditions:
       - the number of binary occurrences is greater than zero;
       - the proposition was not assigned previously during this loop;
       - the current binary occurrences are greater than the former ones;
       try to propagate it. */
      if ((NegBinOcc[p -> prop] > 0) && 
          (ScoreNeg[p -> prop] != -1) && 
          (NegBinOcc[p -> prop] > ScoreNeg[p -> prop])) { 

        /* Propagate p and do BCP. */
        if (Propagate(p, SIM_TT, &scoreFF) == SIM_UNSAT) {
          bestProp = p;
          /* Save the number of binary occurrences. */
          for (genq = V(SimProps), q = *genq; q != p; q = *(++genq)) {
            if (q -> teta == SIM_DC) {
              ScoreNeg[q -> prop] = NegBinOcc[q -> prop];
              ScorePos[q -> prop] = PosBinOcc[q -> prop];
            }
          }
          VforeachGen0(genq, q) {
            if (q -> teta == SIM_DC) {
	      NegBinOcc[q -> prop] = CountBinaryNeg(q);
              if (ScoreNeg[q -> prop] == -1) {
                ScoreNeg[q -> prop] = NegBinOcc[q -> prop]; 
              }
	      PosBinOcc[q -> prop] = CountBinaryPos(q); 
              if (ScorePos[q -> prop] == -1) {
                ScorePos[q -> prop] = PosBinOcc[q -> prop];
              }
            }
          }
          /* Set sign and mode. */
          *sign = SIM_TT;
          *mode = SIMLSPLIT;
	  SimCnt[SIMCUR_LEVEL] += 1;
          SimDllStatInc(SIMFAILED_NUM);
          return bestProp;
        }
      } else { 
        scoreFF = NegBinOcc[p -> prop];
      } 
      
      /* Compute score TT. */
      
      /* If p satisfies the conditions:
       - the number of binary occurrences is greater than zero;
       - the proposition was not assigned previously during this loop;
       - the current binary occurrences are greater than the former ones;
       try to propagate it. */
      if ((PosBinOcc[p -> prop] > 0) && 
          (ScorePos[p -> prop] != -1) && 
          (PosBinOcc[p -> prop] > ScorePos[p -> prop])) { 

        /* Propagate p and do BCP. */
        if (Propagate(p, SIM_FF, &scoreTT) == SIM_UNSAT) {
          bestProp = p;
          /* Save the number of binary occurrences. */
          for (genq = V(SimProps), q = *genq; q != p; q = *(++genq)) {
            if (q -> teta == SIM_DC) {
              ScoreNeg[q -> prop] = NegBinOcc[q -> prop];
              ScorePos[q -> prop] = PosBinOcc[q -> prop];
            }
          }
          VforeachGen0(genq, q) {
            if (q -> teta == SIM_DC) {
	      NegBinOcc[q -> prop] = CountBinaryNeg(q);
              if (ScoreNeg[q -> prop] == -1) {
                ScoreNeg[q -> prop] = NegBinOcc[q -> prop]; 
              }
	      PosBinOcc[q -> prop] = CountBinaryPos(q);
              if (ScorePos[q -> prop] == -1) {
                ScorePos[q -> prop] = PosBinOcc[q -> prop];
              }
            }
          }
          /* Set sign and mode. */
          *sign = SIM_FF;
          *mode = SIMLSPLIT;
	  SimCnt[SIMCUR_LEVEL] += 1;
          SimDllStatInc(SIMFAILED_NUM);
          return bestProp;
        }
      } else { 
        scoreTT = PosBinOcc[p -> prop];
      } 

      /* Calculate the score for the proposition. */
      Score[p -> prop] = (scoreTT * scoreFF << 1) + scoreTT + scoreFF + 1;

      /* Get the best score. */
      if (fBest < Score[p -> prop]) {
        fBest = Score[p -> prop];
      }

    } /* if (p -> teta == SIM_DC) */ 

  } /* Vforeach0(SimProps, genp, p) */
  
  /* If there are no binary clauses, use a JW weight to score propositions. */
  if (noBinary) {
    Vforeach0(SimProps, genp, p) {
      scoreTT = CountNoBinaryPos(p);
      scoreFF = CountNoBinaryNeg(p);
      Score[p -> prop] = (scoreTT * scoreFF << 1) + scoreTT + scoreFF + 1;
      if (fBest < Score[p -> prop]) {
	fBest = Score[p -> prop];
      }
    }
  }
  
  /* Could not find any proposition. */
  if (fBest == -1.0) {
    return bestProp;
  }

  /* Calculate threshold starting from any weight which is considered best. */
  fMin = fBest * RELSAT_FUDGEFACT;
  /* Select a pool of propositions. */
  Vforeach0(SimProps, genp, p) {
    ScoreNeg[p -> prop] = NegBinOcc[p -> prop]; 
    ScorePos[p -> prop] = PosBinOcc[p -> prop]; 
    if ((Score[p -> prop] >= fMin) && (p -> teta == SIM_DC)) {
      VforceBack(ChosenProps, p);
    }
  }

  /* Select a proposition from the pool randomly. */
  SimDllAssert((Vsize(ChosenProps) != 0), SIM_HEUR_LOCATION, 
	       "SimDllRelsatHeur: no best candidates!"); 
  bestProp = V(ChosenProps)[SimRndNumber(Vsize(ChosenProps))];

  SimCnt[SIMCUR_LEVEL] += 1;
  SimDllStatInc(SIMNODE_NUM);

  /* Set sign and mode. */
  *mode = SIMLSPLIT;
  *sign =  (SimRndNumber(2) ? SIM_TT : SIM_FF);

  return bestProp;
  
} /* End of SimDllRelsatHeur. */


/* UNITIE ********************************************************************/

/**Function********************************************************************

  Synopsis    [Allocate auxiliary data for Unitie heuristics. ]  

  Description [Allocate auxiliary data for Uniti heuristics. ]

  SideEffects [None]

  SeeAlso     [SimDllUnitieHeur SimDllEndUnitieHeur ]

******************************************************************************/
void 
SimDllInitUnitieHeur()
{

  int          varNum;

  varNum = Vsize(SimProps);
  VinitReserve(bestProps, (varNum + 1));

  VinitReserve(ManagedCls, SimCnt[SIMCL_NUM]);
  SimDllMemCheck(V(ManagedCls), "SimDllInitUnitieHeur");

  VinitReserve(ChangedProps, varNum);
  SimDllMemCheck(V(ChangedProps), "SimDllInitUnitieHeur");
  
  VinitReserve(ChosenProps, (varNum + 1));
  SimDllMemCheck(V(ChosenProps), "SimDllInitUnitieHeur");
  
  return;
  
} /* End of SimDllInitUnitieHeur. */


/**Function********************************************************************

  Synopsis    [Free auxiliary data for Unitie heuristics. ]  

  Description [Free auxiliary data for Unitie heuristics. ]

  SideEffects [None]

  SeeAlso     [SimDllInitUnitieHeur SimDllUnitieHeur]

******************************************************************************/
void 
SimDllEndUnitieHeur()
{
  
  Vclear(bestProps);
  Vclear(ManagedCls);
  Vclear(ChangedProps);
  Vclear(ChosenProps);

  return;

} /* End of SimDllEndUnitieHeur */


/**Function********************************************************************

  Synopsis    [A simple unit based heuristics with tie breaking. ] 

  Description [Selects all the variables that maximize the score:
               binIfpos * binIfneg << 1 + unitIfpos + unitIfneg + 1
	       and breaks ties selecting the variable that subsumes the
	       largest number of clauses.]

  SideEffects [none]

  SeeAlso     [Sim_DllSolve]

******************************************************************************/
SimProp_t * 
SimDllUnitieHeur(
  int * sign,
  int * mode)
{
  /* Variables are needed for LeanXXX macros. */
  SimProp_t   * p, ** genp;
  SimProp_t   * q = 0, ** genq;
  SimClause_t * cl, **gencl;
  int           length;

  int           pos, neg;
  int           posBin, negBin;
  int           weight, bestWeight;
  int           localBcpIdx;
  SimProp_t   * bestProp;

  do {

    /*  Initializations. */
    weight = bestWeight = 0;
    bestProp = 0;
    Vflush(bestProps);
    
    /* Decide the index: model propositions or all the propositions. */
    if (SimParam[SIM_INDEP_PROPS]) {
      genp = V(SimModelProps);
    } else {
      genp = V(SimProps);
    }

    /* Apply BCP to all the propositions. */
    VforeachGen0(genp, p) {

      if (p -> teta == SIM_DC) {
	
	/* Assign p to SIM_TT and do unit propagation. */
        localBcpIdx = posBin = pos = 0;
	LeanExtendPropTT(p, extScoringUnitie);  
#ifdef BACKJUMPING
	LeanBcp(q, cl, bcpScoringUnitie, extScoringUnitie, StoreReason);
#else
	LeanBcp(q, cl, bcpScoringUnitie, extScoringUnitie, DoNothing1);
#endif
	if (localBcpIdx == 0) {
	  /* On success, bactrack one-node  */
	  ResetProps(DoNothing1);
	} else {
	  /* On failure, bactrack and try the other assignment (SIM_FF) */
#ifdef BACKJUMPING
	  /* Initialize the working reason with the empty clause and 
	     add the reason of the proposition that caused the conflict. */
	  SimDllInitWr(cl);
	  SimDllResolveWithWr(q -> reason, q);
	  ResetPropsBj;
	  p -> reason = SimDllMakeClauseFromWr(0);
#else
	  ResetProps(DoNothing1);
#endif
	  (void) SimDllExtendPropFF(p, SIMFAILED);
	  SimDllStatInc(SIMFAILED_NUM);
	  SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL], "failed literal");
	  if (SimDllBcp() == SIM_UNSAT) {
	    /*  If it fails, call Backtracking. */
	    return SimDllBacktrack(sign, mode);
	  } else {
	    /*  If it succeeds, go to the next proposition. */
	    continue; 
	  }
	}
	
	/* Assign var to FF and do unit propagation.  */
	localBcpIdx = negBin = neg = 0;
	LeanExtendPropFF(p, extScoringUnitie);  
#ifdef BACKJUMPING
	LeanBcp(q, cl, bcpScoringUnitie, extScoringUnitie, StoreReason);
#else
	LeanBcp(q, cl, bcpScoringUnitie, extScoringUnitie, DoNothing1);
#endif
	if (localBcpIdx == 0) {
	  /* On success, bactrack one-node  */
	  ResetProps(DoNothing1);
	} else {
	  /* On failure, bactrack and try the other assignment (SIM_TT) */
#ifdef BACKJUMPING
	  /* Initialize the working reason with the empty clause and 
	     add the reason of the proposition that caused the conflict. */
	  SimDllInitWr(cl);
	  SimDllResolveWithWr(q -> reason, q);
	  ResetPropsBj;
	  p -> reason = SimDllMakeClauseFromWr(0);
#else
	  ResetProps(DoNothing1);
#endif
	  (void) SimDllExtendPropTT(p, SIMFAILED);
	  SimDllStatInc(SIMFAILED_NUM);
	  SimPropHasValueBy(p, SimCnt[SIMCUR_LEVEL], "failed literal");
	  /*  The assignment was tested above and cannot fail! */
	  (void) SimDllBcp();
	  continue; 
	}      
	
	/* Compute the weight for the current proposition. */
	switch (SimParam[SIM_HEUR_PARAM]) {
	case 0:
	  weight = pos * neg * 1024 + pos + neg + 1;   
	  break;
	case 1:
	  weight = posBin * negBin * 1024 + pos + neg + 1;  
	  break;
	case 2:
	  weight = ((posBin * negBin) << 1) + pos + neg + 1;  
	  break;
	}
	if (weight > bestWeight) {
	  bestWeight = weight;
	  Vflush(bestProps);
	  VforceBack(bestProps, p);
	} else if (weight == bestWeight) {
	  VforceBack(bestProps, p);
	}
	
      } /* if (p -> teta == SIM_DC) */

    } /* VforeachGen0(genp, p) */
    
    /* Selecting one proposition out of the pool of best propositions. */
    if (Vsize(bestProps) > 0) {
      /* 0-terminate bestProps and save the first proposition as best. */
      VforceBack(bestProps, 0);
      bestProp = V(bestProps)[0];
      if (SimParam[SIM_HEUR_PARAM] > 0){
	/* Selecting the one that subsumes a lot of clauses */
	bestWeight = 0;
	Vforeach0(bestProps, genp, p) {
	  if (p -> teta == SIM_DC) {
	    pos = neg = 0;
	    Vforeach0(p -> posLits, gencl, cl) {
	      if (cl -> sub == 0) {
		pos += 1;
	      }
	    }
	    Vforeach0(p -> negLits, gencl, cl) {
	      if (cl -> sub == 0) {
		neg += 1;
	      }
	    }
	    if ((pos > bestWeight) || neg > bestWeight) {
	      bestProp = p;
	      if (pos > neg) {
		bestWeight = pos; 
		*sign = SIM_TT;
	      } else {
		bestWeight = neg; 
		*sign = SIM_FF;
	      }
	    }
	  }
	}
      } else {
	/* Selecting the first open one (if any). */
	Vforeach0(bestProps, genp, p) {
	  if (p -> teta == SIM_DC) {
	    bestProp = p;
	    break;
	  }
	}
      }
    } /* if (Vsize(bestProps) > 0) */

  } while ((bestProp != 0) && (bestProp -> teta != SIM_DC));
  /* This is needed because we might have assigned all our best propositions */
  /* during a failed literal propagation: scoring must restart! */

  SimCnt[SIMCUR_LEVEL] += 1;
  SimDllStatInc(SIMNODE_NUM);

  *mode = SIMLSPLIT;

  return bestProp;

} /* End of SimDllUnitieHeur */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Chooses the best literal according to M.O.M.S. heuristics.]

  Description [For each open proposition x computes the score f(x) * f(-x),
               where f is the number of occurrence in binary clauses.
	       Chooses the proposition with the highest score. 
	       The sign is `signPos' if f(x) > f(-x), and -`signPos' otherwise.
	       If `propList' (a list of positive integers) is non-0,
	       the heuristics looks at the contents of `propList' instead of 
	       looking at the open propositions (in the spirit of SATO 3.2).] 

  SideEffects [None]

  SeeAlso     [SimDllSatoHeur]

******************************************************************************/
static SimProp_t * 
DllMomsHeur(
  int               * sign,
  int               * mode,
  SimProp_t        ** propList, 
  int                 signPos)
{

  int             i, pos, neg, weight;
#ifdef PURE_LITERAL
  int             posAll, negAll;
#endif
  SimClause_t  * cl, ** gencl;
  SimProp_t    * p, ** genp;

  int            bestWeight = -1;
  SimProp_t *    bestProp   = 0;


  i = 1;
  /* If propList is non-0, override the general behaviour. */
  if (propList != 0) {
    /* Start from the first selected proposition. */
    genp = propList;
  } else {
    /* Start from the first open proposition. */
    if (SimParam[SIM_INDEP_PROPS]) {
      genp = V(SimModelProps);
    } else {
      genp = V(SimProps);
    }
  }

  /* Cycle propositions. */
  VforeachGen0(genp, p) {

    if (p -> teta == SIM_DC) {

      /* Initialize scores. */
      pos = neg = 1;
#ifdef PURE_LITERAL
      posAll = negAll = 0;
#endif
      /* Check the positive occurrences of the proposition. */
      Vforeach0(p -> posLits, gencl, cl) {
	/* Consider binary open clauses only. */ 
	if (SimClauseIsOpen(cl)) {
	  if (cl -> openLitNum == 2) {
	    pos += 1;
	  } 
#ifdef PURE_LITERAL	
	  posAll += 1;
#endif
	}
      }
      /* Check the negative occurrences of the proposition. */
      Vforeach0(p -> negLits, gencl, cl) {
	/* Consider binary open clauses only. */ 
	if (SimClauseIsOpen(cl)) {
	  if (cl -> openLitNum == 2) {
	    neg += 1;
	  } 
#ifdef PURE_LITERAL
	  negAll += 1;
#endif
	}
      }
#ifdef PURE_LITERAL
      /* Detect pure literals. */
      if (((posAll == 0) && (negAll != 0)) || 
	  ((posAll != 0) && (negAll == 0))) {
	VforceBack(SimMlfStack, p);
	p -> mode = (negAll != 0 ? SIMPURENEG : SIMPUREPOS);
      }
#endif
      
      /* Calculate curWeight and check if it is the best weight. */
      weight = pos * neg;
      if (weight > bestWeight) {
	bestWeight = weight;
	bestProp = p;
	*sign = (neg > pos ? -1 * signPos : signPos);
      }

    } /* if (p -> teta == SIM_DC) */
      
  } /* VforeachGen0(genp, p) */

  SimCnt[SIMCUR_LEVEL] += 1;
  SimDllStatInc(SIMNODE_NUM);

  /* Set the mode. */
  *mode = SIMLSPLIT;

  return bestProp; 

} /* End of SimDllMomsHeur. */


/**Function********************************************************************

  Synopsis    [Evaluates how much an assignment reduces the formula.]

  Description [Assigns the proposition p with a certain sign and then
               performs unit propagation collecting, scores; at the end
	       all the changes are undone. Returns SIM_UNSAT if p is a
	       failed literal and SIM_SAT otherwise.]

  SideEffects [None]

  SeeAlso     [SimDllSatzHeur Examine0 ExtendProp Bcp ResetProps]

******************************************************************************/
static int 
Examine(
  SimProp_t       * p,
  int               sign,
  int             * reduced)
{
  /* Variables are needed for LeanXXX macros. */
  SimProp_t   * q = 0, ** genq;
  SimClause_t * cl, **gencl;
  int           length;

  int           localBcpIdx = 0;

  /* Initialize managed clauses stack. */
  Vflush(ManagedCls);

  /* Propagate p. */
  if (sign == SIM_TT) {
    LeanExtendPropTT(p, DoNothing2);
  } else {
    LeanExtendPropFF(p, DoNothing2);
  }
  
  /* Do unit propagation. */
#ifdef BACKJUMPING
  LeanBcp(q, cl, DoNothing1, DoNothing2, StoreReason);
#else
  LeanBcp(q, cl, DoNothing1, DoNothing2, DoNothing1);
#endif

  /* If this is not a failed literal, save the score of the proposition. */
  if (localBcpIdx == 0) {
    reduced[p -> prop] = Vsize(ManagedCls); 
    ResetProps(DoNothing1);
    return SIM_SAT;
  } else { 
#ifdef BACKJUMPING
    /* Initialize the working reason with the empty clause and 
       add the reason of the proposition that caused the conflict. */
    SimDllInitWr(cl);
    SimDllResolveWithWr(q -> reason, q);
    /* Reset all the propagations. */
    ResetPropsBj;
    /* Get the working reason as a reason for p. */
    p -> reason = SimDllMakeClauseFromWr(0);
#else
    ResetProps(DoNothing1);
#endif
    return SIM_UNSAT;
  }

} /* End of Examine. */


/**Function********************************************************************

  Synopsis    [Evaluates how much an assignment reduces the formula.]

  Description [Assigns the proposition p with a certain sign and then
               performs unit propagation, collecting scores; at the end
	       all the changes are undone. Returns SIM_UNSAT if p is a
	       failed literal and SIM_SAT otherwise.]

  SideEffects []

  SeeAlso     [SimDllSatzHeur Examine ExtendProp Bcp ResetProps]

******************************************************************************/
static int Examine0( 
   SimProp_t       * p,   
   int               sign,
   int             * reduced)
{
  /* Variables are needed for LeanXXX macros. */
  SimProp_t   * q = 0, ** genq;
  SimClause_t * cl, **gencl;
  int           length;

  int           i;
  SimProp_t   * mp, **mgenp;
  SimClause_t * mcl;

  int           score       = 0;
  int           localBcpIdx = 0;

  /* Initialize managed clauses stack. */
  Vflush(ManagedCls);

  /* Propagate  p. */
  if (sign == SIM_TT) {
    LeanExtendPropTT(p, DoNothing2);
  } else {
    LeanExtendPropFF(p, DoNothing2);
  }

  /* Do unit propagation. */
#ifdef BACKJUMPING
  LeanBcp(q, cl, DoNothing1, DoNothing2, StoreReason); 
#else
  LeanBcp(q, cl, DoNothing1, DoNothing2, DoNothing1); 
#endif

  /* Evaluate the score for the proposition according to a modified JW
     heuristics : do it only if the proposition is not a failed literal. */ 
  if (localBcpIdx == 0) {
    /* Consider each clause that was shrinked and became binary. */
    for (i = 0; i < Vsize(ManagedCls); i++) {
      if (V(ManagedCls)[i] -> openLitNum == 2) {
	mcl = V(ManagedCls)[i];
	/* Increase the scores of the current proposition according to
	   the scores of the propositions occurring in such  clauses. */
	Vforeach0(mcl -> lits, mgenp, mp) {
	  if (SimRef(mp) -> teta == SIM_DC) {
	    if (SimIsPos(mp)) {
	      score += (NegClLength2[mp -> prop] * SATZ_WEIGTH +
			NegClLength3[mp -> prop]);  
	    } else {
	      mp = SimRef(mp);
	      score += (PosClLength2[mp -> prop] * SATZ_WEIGTH + 
			PosClLength3[mp -> prop]);
	    }
	  }
	} /*  Vforeach0(mcl -> lits, mgenp, mp) */
      }
    } /* i < Vsize(ManagedCls) */
    
    /* Save the score of proposition. */
    reduced[p -> prop] += score; 
    
    /* Reset all the propagations. */
    ResetProps(DoNothing1);
    return SIM_SAT;

  } else {
  
#ifdef BACKJUMPING  
    /* Initialize the working reason with the empty clause and 
       add the reason of the proposition that caused the conflict. */
    SimDllInitWr(cl);
    SimDllResolveWithWr(q -> reason, q);
    /* Reset all the propagations. */
    ResetPropsBj;
    /* Get the working reason as a reason for p. */
    p -> reason = SimDllMakeClauseFromWr(0);
#else
    ResetProps(DoNothing1);
#endif  
    return SIM_UNSAT;

  }

} /* End of Examine0 */


/**Function********************************************************************

  Synopsis    [Evaluates how much an assignment reduces the formula.]

  Description [Assigns the proposition p with a certain sign and then
               performs unit propagation collecting scores; at the end
	       all the changes are undone. Returns SIM_UNSAT if p is a
	       failed literal and SIM_SAT otherwise.]

  SideEffects [None]

  SeeAlso     [SimDllRelsatHeur]

******************************************************************************/
static int Propagate(
  SimProp_t     * p,
  int             sign,
  int           * score)
{

  /* Variables are needed for LeanXXX macros. */
  SimProp_t  ** genq;
  SimClause_t * cl, **gencl;
  int           length;

  int           localBcpIdx = 0;
  int           unit = 1;
  SimProp_t   * q = 0;
  /* Assign proposition. */
  if (sign == SIM_TT) {
    LeanExtendPropTT(p, DoNothing2);
  } else {
    LeanExtendPropFF(p, DoNothing2);
  }
  
  /* Propagate unit clauses. */
#ifdef INACTIVE
  LeanBcp(q, cl, bcpScoringRelsat, DoNothing2, StoreReason);
#else
  LeanBcp(q, cl, bcpScoringRelsat, DoNothing2, DoNothing1);
#endif

  if (localBcpIdx == 0) {
    *score = unit;
    /* Undo the assignments. */
    ResetProps(ResetOpRelsat); 
    return SIM_SAT;
  } else {
#ifdef INACTIVE
    SimDllInitWr(cl);
    SimDllResolveWithWr(q -> reason, q);
    ResetPropsBj;
    /* Get the working reason as a reason for p. */
    p -> reason = SimDllMakeClauseFromWr(0);
#else
    ResetProps(DoNothing1);
#endif
    return SIM_UNSAT;
  }

} /* End of Propagate. */


/**Function********************************************************************

  Synopsis    [Evaluates how many times a propositions occurres in a
               binary clause.]

  Description [Evaluates how many times a propositions occurres in a
               binary clause. If PURE LITERAL is defined 
	       returns the number of occurrences in
	       clauses of any length.]

  SideEffects [none]

  SeeAlso     [SimDllRelsatHeur]

******************************************************************************/
static int CountBinaryPos(
  SimProp_t     * p)
{
  SimClause_t * cl, ** gencl;
  int           binOcc = 0;

  Vforeach0(p -> posLits, gencl, cl) {
    if (SimClauseIsOpen(cl)) {
      if (cl -> openLitNum == 2) {
	binOcc += 1;
      }
    }
  }
  return binOcc;

}/* End of CountBinaryPos. */

/**Function********************************************************************

  Synopsis    [Evaluates how many times a propositions occurres in a
               binary clause.]

  Description [Evaluates how many times a propositions occurres in a
               binary clause. If PURE LITERAL is defined 
	       returns the number of occurrences in
	       clauses of any length.]

  SideEffects [none]

  SeeAlso     [SimDllRelsatHeur]

******************************************************************************/
static int CountBinaryNeg(
  SimProp_t     * p)
{
  SimClause_t * cl, ** gencl;
  int           binOcc = 0;

  Vforeach0(p -> negLits, gencl, cl) {
    if (SimClauseIsOpen(cl)) {
      if (cl -> openLitNum == 2) {
	binOcc += 1;
      }
    }
  }
  return binOcc;

}/* End of CountBinaryNeg. */


/**Function********************************************************************

  Synopsis    [Calculate a score for a proposition.]

  Description [Uses a modified JW weighting function H(x):
               for each open proposition x compute the sum H(x)
               of the quantities 2^(2^(JW_MAX - |C|)) (which defaults to 1
	       if |C| >= JW_MAX) for each clause C where l occurs.]

  SideEffects [none]

  SeeAlso     [SimDllRelsatHeur]

******************************************************************************/
static int CountNoBinaryPos(
  SimProp_t * p)
{
  SimClause_t  * cl, ** gencl;
  int            score = 0;

  Vforeach0(p -> posLits, gencl, cl) {
    if (SimClauseIsOpen(cl)) {
      score += 
	1 << (cl -> openLitNum >= JW_MAX ? 0 : 
	      (1 << (JW_MAX - cl -> openLitNum)));
    }
  }
  return score;

} /* End of ComputeNoBinaryScore. */


/**Function********************************************************************

  Synopsis    [Calculate a score for a proposition.]

  Description [Uses a modified JW weighting function H(x):
               for each open proposition x compute the sum H(x)
               of the quantities 2^(2^(JW_MAX - |C|)) (which defaults to 1
	       if |C| >= JW_MAX) for each clause C where l occurs.]

  SideEffects [none]

  SeeAlso     [SimDllRelsatHeur]

******************************************************************************/
static int CountNoBinaryNeg(
  SimProp_t * p)
{
  SimClause_t  * cl, ** gencl; 
  int            score = 0;

  Vforeach0(p -> negLits, gencl, cl) {
    if (SimClauseIsOpen(cl)) {
      score += 
	1 << (cl -> openLitNum >= JW_MAX ? 0 : 
	      (1 << (JW_MAX - cl -> openLitNum)));
    }
  }
  
  return score;

} /* End of ComputeNoBinaryScore. */


