/**CFile***********************************************************************

  FileName    [simTrace.c]

  PackageName [sim]

  Synopsis    [Running trace handling]

  Description [Internal procedures included in this module:
		<ul>
		<li> <b>SimDllTraceNodes()</b> Tracing search tree nodes
		     (heuristics choices)
		<li> <b>SimDllTraceLeafs()</b> Tracing search tree leafs
		     (backtracks)
		</ul>]
		
  SeeAlso     []

  Author      [Armando Tacchella]

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
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Tracing search nodes.]

  Description [Each time the function is called a counter is decremented.
               Once the number of ticks is reached, the functions outputs
               data on the look-ahead algorithms.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void SimDllTraceNodes()
{

  if (SimLapStat[SIMNODE_NUM] - 1 == 0) {

    /* If a given number of ticks elapsed, output statistics. */
    printf("Current depth %d\n", SimCnt[SIMCUR_LEVEL]);
    printf("Look-ahead statistics since last update:\n");
    printf("  Unit propagations: %d\n", 
	   SimStat[SIMUNIT_NUM] - SimLapStat[SIMUNIT_NUM]);
    SimLapStat[SIMUNIT_NUM] = SimStat[SIMUNIT_NUM];
#ifdef PURE_LITERAL
    printf("  Pure literals: %d\n", 
	   SimStat[SIMPURE_NUM] - SimLapStat[SIMPURE_NUM]);
    SimLapStat[SIMPURE_NUM] = SimStat[SIMPURE_NUM];
#endif
    printf("  Failed literals: %d\n", 
	   SimStat[SIMFAILED_NUM] - SimLapStat[SIMFAILED_NUM]);
    SimLapStat[SIMFAILED_NUM] = SimStat[SIMFAILED_NUM];
    printf("Current path (choice points only):\n  ");
    SimDllPrintPath();
#ifdef LEARNING    
    printf("Clauses (open/total/learned): %d / %d / %d\n",
	   SimCnt[SIMCUR_CL_NUM], SimCnt[SIMCL_NUM], 
	   Vsize(SimLearned));
#else
    printf("Clauses (open/total): %d / %d \n",
	   SimCnt[SIMCUR_CL_NUM], SimCnt[SIMCL_NUM]);
#endif	   
    printf("Variables (open/total): %d / %d \n", 
	   Vsize(SimProps) -  Vsize(SimStack), Vsize(SimProps));
    printf("\n");
    SimLapStat[SIMNODE_NUM] = SimParam[SIM_RUN_TRACE];

  } else if (SimLapStat[SIMNODE_NUM] > 0) {
    
    /* Decrease the number of ticks remaining. */
    SimLapStat[SIMNODE_NUM] -= 1;

  }

  return;

} /* End of SimDllTraceNodes. */


/**Function********************************************************************

  Synopsis    [Tracing search leafs.]

  Description [Each time the function is called a counter is decremented.
               Once the number of ticks is reached, the functions outputs
               data on the look-back algorithms.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void SimDllTraceLeafs()
{

  if (SimLapStat[SIMFAIL_NUM] - 1 == 0) {

    /* If a given number of ticks elapsed, output statistics. */
    printf("Current depth %d\n", SimCnt[SIMCUR_LEVEL]);
    printf("Look-back statistics since last update:\n");
    printf("  Maximum depth in the search tree: %d\n", SimStat[SIMDEPTH_MAX]);
#ifdef BACKJUMPING
    printf("  Skipped nodes: %d\n", 
	   SimStat[SIMSKIP_NUM] - SimLapStat[SIMSKIP_NUM]);
    SimLapStat[SIMSKIP_NUM] = SimStat[SIMSKIP_NUM];
#endif
#ifdef LEARNING
    printf("  Unit on learned: %d\n", 
	   SimStat[SIMULEARN_NUM] - SimLapStat[SIMULEARN_NUM]);
    SimLapStat[SIMULEARN_NUM] = SimStat[SIMULEARN_NUM];
#endif
    printf("Current path (choice points only):\n  ");
    SimDllPrintPath();
#ifdef LEARNING    
    printf("Clauses (open/total/learned): %d / %d / %d\n",
	   SimCnt[SIMCUR_CL_NUM], SimCnt[SIMCL_NUM], 
	   Vsize(SimLearned));
#else
    printf("Clauses (open/total): %d / %d \n",
	   SimCnt[SIMCUR_CL_NUM], SimCnt[SIMCL_NUM]);
#endif	   
    printf("Variables (open/total): %d / %d \n", 
	   Vsize(SimProps) -  Vsize(SimStack), Vsize(SimProps));
    printf("\n");
    SimLapStat[SIMFAIL_NUM] = SimParam[SIM_RUN_TRACE];

  } else if (SimLapStat[SIMFAIL_NUM] > 0) {
    
    /* Decrease the number of ticks remaining. */
    SimLapStat[SIMFAIL_NUM] -= 1;

  }

  return;

} /* End of SimDllTraceLeafs. */

