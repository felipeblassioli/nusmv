/**CFile***********************************************************************

  FileName    [simOutput.c]

  PackageName [sim]

  Synopsis    [Output routines.]

  Description [External procedures included in this module:
		<ul>
		<li> <b>Sim_DllPrintFormulaDimacs()</b> prints out the
    		     internal formula in DIMACS syntax; 
		<li> <b>Sim_DllPrintModel()</b> prints out the current
		     truth assignment;
		<li> <b>Sim_DllPrintStats()</b> prints out statistics.
		<li> <b>Sim_DllPrintTimers()</b> prints out timers.
		<li> <b>Sim_DllPrintResult()</b> prints out a formatted result
		<li> <b>Sim_DllPrintParam()</b> prints out parameters.
                </ul>
               Internal procedures included in this module:
                <ul>
                <li> <b>SimDllPrintStack()</b> prints out the search stack;
                <li> <b>SimDllPrintPath()</b> prints out the search path;
                <li> <b>SimDllPrintOpen()</b> prints out the open variables.
                <li> <b>SimDllPrintStruct()</b> prints formula structure;
                <li> <b>SimDllPrintClause</b> prints out a clause;
		<li> <b>SimDllPrintLearnedFW</b> prints learned clauses (FIFO);
		<li> <b>SimDllPrintLearnedBW</b> prints learned clauses (LIFO).
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


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


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

  Synopsis    [Print the formula in the DLL module using DIMACS format.]

  Description [Print the formula in the DLL module using DIMACS format.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void 
Sim_DllPrintFormulaDimacs()
{

  SimClause_t  * cl;
  SimProp_t    * p;
  SimProp_t   ** genp;
  int            i, varNum = 0;

  /* Understand the maximum variable index. */
  Vforeach0(SimProps, genp, p) {
    if (p -> prop > varNum) {
      varNum = p -> prop;
    }
  }

  /* Print the DIMACS header. */
  printf("c Generated by Sim_DllPrintFormula\n");
  printf("p cnf %d %d\n", varNum, SimCnt[SIMCUR_CL_NUM]);

  /* Print the clauses in DIMACS format. */
  for (i = 0; i < Vsize(SimClauses); i++) {
    cl = V(SimClauses)[i];
    if (SimClauseIsOpen(cl)) {
      Vforeach0(cl -> lits, genp, p) {
	if (SimRef(p) -> teta == SIM_DC) {
	  if (SimIsPos(p)) {
	    printf("%d ", p -> prop);
	  } else {
	    printf("-%d ", SimRef(p) -> prop);
	  }
	}
      }
      printf("0\n");
    }
  }

  fflush(stdout);
  return;

} /* End of Sim_DllPrintFormulaDimacs. */


/**Function********************************************************************

  Synopsis    [Print the model.]

  Description [Print the model.]

  SideEffects [None]

  SeeAlso     [Sim_DllPrintStack]

******************************************************************************/
void 
Sim_DllPrintModel()
{
  
  SimProp_t  * p;
  SimProp_t ** genp;
  
  Vforeach0(SimProps, genp, p) {
    if (p -> teta != SIM_DC) {
      printf("%d ", (p -> teta) * (p -> prop));
   }
 }
 printf("\n");
 
 fflush(stdout);
 return;

} /* End of Sim_DllPrintModel */


/**Function********************************************************************

  Synopsis    [Prints internal statistics.]

  Description [Prints internal statistics.]

  SideEffects [None]

  SeeAlso     [Sim_DllPrintModel]

******************************************************************************/
void 
Sim_DllPrintStats()
{
  
 int i, j, len, max;

 if (SimParam[SIM_VERBOSITY] == 0) {

   /* Machine-readable output is requested. */
   for (i = 0; i < SIMSTAT_NUM; i++) {
     switch (i) {
     case SIMLEARN_AVE:
       if (SimStat[SIMLEARN_NUM] != 0) {
	 printf("%d ", SimStat[i] / SimStat[SIMLEARN_NUM]);
       }
       break;
     default:
       printf("%d ", SimStat[i]);
       break;
     }
   }

 } else {

   /* Human-readable output is requested. */
   /* Find the longest name. */
   max = 0;
   for (i = 0; i < SIMSTAT_NUM; i++) {
     if ((len = strlen(SimstatName[i])) > max) {
       max = len;
     }
   }
   
   /* Print out statistics names and values. */
   for (i = 0; i < SIMSTAT_NUM; i++) {
     switch (i) {
     case SIMUNIT_NUM :
       printf("\nDLL algorithm:\n");
       break;
     case SIMSKIP_NUM :
       printf("\nBackjumping:\n");
       break;
     case SIMLEARN_NUM :
       printf("\nLearning:\n");
       break;
     case SIMPUNIT_NUM :
       printf("\nPreprocessing:\n");
       break;
     case SIMUSD_MEM :
       printf("\nMemory:\n");
       break;
     }
     printf("%s", SimstatName[i]);
     /* Pad with spaces to align numbers. */
     for (j = 0; j < (max - strlen(SimstatName[i])); printf(" "), j++);
     switch (i) {
     case SIMLEARN_AVE:
       if (SimStat[SIMLEARN_NUM] != 0) {
	 printf("%10d\n", SimStat[i] / SimStat[SIMLEARN_NUM]);
       } else {
	 printf("%10s\n", "N/A");
       }
       break;
     default:
       printf("%10d\n", SimStat[i]);
       break;
     }
   }
   
 }

 printf("\n");
 
 fflush(stdout);
 return;

} /* End of Sim_DllPrintStats. */


/**Function********************************************************************

  Synopsis    [Prints internal timers.]

  Description [Prints internal timers.]

  SideEffects [None]

  SeeAlso     [Sim_DllPrintModel]

******************************************************************************/
void 
Sim_DllPrintTimers()
{
  
 int i, j, len, max;

 if (SimParam[SIM_VERBOSITY] == 0) {

   /* Machine-readable output is requested. */
   for (i = 0; i < SIMTIMER_NUM; i++) {
     printf("%.3f ", SimTimer[i]);
   }

 } else {

   /* Human-readable output is requested. */
   /* Find the longest name. */
   max = 0;
   for (i = 0; i < SIMTIMER_NUM; i++) {
     if ((len = strlen(SimtimerName[i])) > max) {
       max = len;
     }
   }
   
   /* Print out timers names and values. */
   for (i = 0; i < SIMTIMER_NUM; i++) {
     printf("%s", SimtimerName[i]);
     /* Pad with spaces to align numbers. */
     for (j = 0; j < (max - strlen(SimtimerName[i])); printf(" "), j++);
     printf("%10.3f\n", SimTimer[i]);
   }
   
 }
 
 fflush(stdout);
 return;

} /* End of Sim_DllPrintTimers. */


/**Function********************************************************************

  Synopsis    [Prints the result (formatted).]

  Description [Prints the result (formatted).]

  SideEffects [None]

  SeeAlso     [Sim_DllPrintModel]

******************************************************************************/
void 
Sim_DllPrintResult(
  int         res)
{
  char chRes;

  /* Output the result as a string or as a number. */
  if (SimParam[SIM_VERBOSITY] != 0) {
    switch (res) {
    case SIM_GEN_FAIL : chRes = 'F'; break;
    case SIM_TIME_FAIL : chRes = 'T'; break;
    case SIM_UNSAT : chRes = 'U'; break;
    case SIM_SAT : chRes = 'S'; break;
    default : chRes = '?';
    }
    printf("Result: %c\n", chRes); 
  } else {
    printf("%d ", res); 
  }

  /* Handle the special case of timeout. */
  if (res == SIM_TIME_FAIL) {
    SimDllStopTimer(SIMSEARCH_TIME);
  }
  SimDllStopMemory(SIMUSD_MEM);

  /* Print timers and statistics. */
  Sim_DllPrintTimers();
  Sim_DllPrintStats();

  return;
}


/**Function********************************************************************

  Synopsis    [Prints parameters (configuration).]

  Description [Prints parameters (configuration).]

  SideEffects [None]

  SeeAlso     [Sim_DllPrintModel]

******************************************************************************/
void 
Sim_DllPrintParams()
{

  int i, j, len, max;

  printf("COMPILE TIME OPTIONS:\n");
  
#ifdef PURE_LITERAL
  printf("Pure literal rule is enabled, ");
#else
  printf("Pure literal rule is disabled, ");
#endif

#ifdef HORN_RELAXATION
  printf("horn relaxation is enabled,\n");
#else
  printf("horn relaxation is disabled,\n");
#endif

#ifdef BACKJUMPING
#ifdef LEARNING
  printf("backjumping and learning are enabled.\n");
#else
  printf("backjumping is enabled.\n");
#endif
#else
  printf("backjumping is disabled.\n");
#endif

  printf("\nRUNTIME OPTIONS:\n");

 /* Find the longest name. */
 max = 0;
 for (i = 0; i < SIM_PARAM_NUM; i++) {
   if ((len = strlen(Sim_paramName[i])) > max) {
     max = len;
   }
 }

 for (i = 0; i < SIM_PARAM_NUM; i++) {
   printf("%s", Sim_paramName[i]);
   for (j = 0; j < (max - strlen(Sim_paramName[i])); printf(" "), j++);
   switch (i) {
   case SIM_MEMOUT :
     printf("%10d\n", SimParam[i] >> 20);
     break;
   case SIM_HEURISTICS :
     printf("%10s\n", Sim_heuristicsName[SimParam[i]]);
     break;
   case SIM_LEARN_TYPE :
     printf("%10s\n", (SimParam[i] == SIM_RELEVANCE ? "relevance" : "size"));
     break;
   default :
     printf("%10d\n", SimParam[i]);
   }
 }
 printf("\n");
 
 fflush(stdout);
 return;

} /* End of Sim_DllPrintParams. */
 

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Print the search stack.]

  Description [Print the search stack.]

  SideEffects [None]

  SeeAlso     [Sim_DllPrintModel]

******************************************************************************/void 
SimDllPrintStack()
{
 
  char * stackVal[6] = {"UN","PP","PN","LS","RS","FL"};

  int i;
  for (i = 0; i < Vsize(SimStack); i++) {
    printf("%d%s ", 
	   V(SimStack)[i] -> teta * V(SimStack)[i] -> prop, 
	   stackVal[(int)V(SimStack)[i] -> mode]);
  }
  printf("\n");

  fflush(stdout);
  return;

} /* End of SimDllPrintStack. */


/**Function********************************************************************

  Synopsis    [Print the search path.]

  Description [Print the search path.]

  SideEffects [None]

  SeeAlso     [Sim_DllPrintModel]

******************************************************************************/void 
SimDllPrintPath()
{
 
  int    i;

  for (i = 0; i < Vsize(SimStack); i++) {
    if (V(SimStack)[i] -> mode == SIMLSPLIT) {
      printf("%d ", V(SimStack)[i] -> teta * V(SimStack)[i] -> prop);
    }
  }
  printf("\n");

  return;

} /* End of SimDllPrintPath. */


/**Function********************************************************************

  Synopsis    [Print the open propositions.]

  Description [Print the open propositions.]

  SideEffects [None]

  SeeAlso     [SimDllUsrHeur]

******************************************************************************/
void 
SimDllPrintOpen()
{
 
 SimProp_t  * p;
 SimProp_t ** genp;
  
 Vforeach0(SimProps, genp, p) {
   if (p -> teta == SIM_DC) {
     printf("%d ", p -> prop);
   }
 }
 printf("\n");
 
 fflush(stdout);
 return;

} /* End of SimDllPrintOpen. */


/**Function********************************************************************

  Synopsis [Print the formula in the DLL module showing internal status.]

  Description [Print the formula in the DLL module using a special
               format:
	       <ul>
	       <li> each clause sits in a single line;
	       <li> the clause is headed by the number of open literals
	            in angle brackets;
	       <li> if the clause was simplified then a pair of square
	            brackets surrounds it; 
	       <li> if a literal was resolved it appears surrounded by
	            normal parentheses.
               </ul>		    
	       Also the following internal state information is
	       printed in a trailer:
	       <ul>
	       <li> search stack contents
	       <li> maximum varibale index and currently open variables
	       <li> total number of clauses, currently open clauses
       	            and, if HORN_RELAXATION is enabled, currently open
		    non-horn clauses.   
	       </ul>]

  SideEffects [None]

  SeeAlso     [Sim_DllPrintFormulaDimacs]

******************************************************************************/
void 
SimDllPrintFormulaStruct()
{

  int         i;
  SimProp_t ** genp;
  SimProp_t  * p;

  int          varNum = 0;
  int          varCur = 0;

  /* Print clauses */
  for (i = 0; i < Vsize(SimClauses); i++) {
    SimDllPrintClause(V(SimClauses)[i]);
    printf("\n");
  }

  /* Understand the current maximum number of variables */
  Vforeach0(SimProps, genp, p) {
    if (p -> prop > varNum) {
      varNum = p -> prop;
    }
    if (p -> teta == SIM_DC) {
      varCur += 1;
    }
  }

  /* Print trailer */
  printf("\nThe search stack is:\n");
  SimDllPrintStack();

  printf("\nVariables (init. max, init. open, curr. max., curr. open):\n");
  printf("          %10d, %10d, %10d, %10d\n",
	 SimParam[SIM_MAX_VAR_NUM], Vsize(SimProps), varNum, varCur);
#ifdef HORN_RELAXATION
  printf("Clauses (initial,   current, init. non-Horn, curr. non-Horn):\n");
  printf("      %10d,%10d,     %10d,     %10d\n",
	 SimCnt[SIMCL_NUM], SimCnt[SIMCUR_CL_NUM], 
	 SimCnt[SIMNHCL_NUM], SimCnt[SIMCUR_NHCL_NUM]);
#else
  printf("Clauses (initial,   current):");
  printf("      %10d,%10d\n",
	 SimCnt[SIMCL_NUM], SimCnt[SIMCUR_CL_NUM]);
#endif

  return;

} /* End of Sim_DllPrintFormulaStruct. */


/**Function********************************************************************

  Synopsis    [Prints learned clauses (first learned, first printed).]

  Description [Prints learned clauses (first learned, first printed).
               If n is less than the number of learned clauses prints n
               learned clauses from the top.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void 
SimDllPrintLearnedFW(
  int         n)
{

#ifdef LEARNING
  int            i;

  /* Possibly print only some clauses. */
  if (n > Vsize(SimLearned)) {
    n = Vsize(SimLearned);
  }
  /* Print clauses */
  for (i = 0; i < n; i++) {
    SimDllPrintClause(V(SimLearned)[i]);
    if (V(SimLearned)[i] -> backUnitLearned >= 0) {
      printf("*");
    }
    printf("\n");
  }
#endif

  return;

} /* End of Sim_DllPrintLearnedFW. */


/**Function********************************************************************

  Synopsis    [Prints learned clauses (last learned, first printed).]

  Description [Prints learned clauses (last learned, first printed).
               if n is less than the number of learned clauses, prints n
               clauses from the bottom.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void 
SimDllPrintLearnedBW(
  int         n)
{

#ifdef LEARNING
  int            i;

  /* Possibly print only some clauses. */
  if (n > Vsize(SimLearned)) {
    n = Vsize(SimLearned);
  }

  /* Print clauses */
  for (i = Vsize(SimLearned) - 1; i >= (Vsize(SimLearned) - n); i--) {
    SimDllPrintClause(V(SimLearned)[i]);
    if (V(SimLearned)[i] -> backUnitLearned >= 0) {
      printf("*");
    }
    printf("\n");
  }
#endif

  return;

} /* End of Sim_DllPrintLearnedBW. */


/**Function********************************************************************

  Synopsis    [Prints a clause structure.]

  Description [Prints a clause structure.]

  SideEffects [None]

  SeeAlso     [Sim_DllUsrHeur]

******************************************************************************/
void SimDllPrintClause(
  SimClause_t * cl)
{
  SimProp_t  * p;
  SimProp_t ** genp;

  if (!SimClauseIsOpen(cl)) {
    printf("[ ");
  }
  
  printf("<%d> ",cl -> openLitNum);
  Vforeach0(cl -> lits, genp, p) {
    if (SimIsPos(p)) {
      if (SimRef(p) -> teta == SIM_FF) {
	printf("(%d) ", p -> prop);
      } else {
	printf("%d ", p -> prop);
      }
    } else {
      if (SimRef(p) -> teta == SIM_TT) {
	printf("(-%d) ", SimRef(p) -> prop);
      } else {
	printf("-%d ", SimRef(p) -> prop);
      }
    }
  }
  
  if (!SimClauseIsOpen(cl)) {
    printf("]");
  }
  
  return;

} /* End of SimDllPrintClause. */
  


