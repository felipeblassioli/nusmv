/**CHeaderFile*****************************************************************

  FileName    [sim.h]

  PackageName [sim]

  Synopsis    [The Satisfiability Internal Module (SIM) package.]

  Description [External functions and data strucures of the SIM package.]

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

#ifndef _SIM
#define _SIM

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "utils/utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* The meaning of EXTERN. */
#define  EXTERN extern

/* Logical values. */
#define SIM_TT               1  /* True. */
#define SIM_DC               0  /* Don't Care. */
#define SIM_FF              -1  /* False. */

/* Return values. */
#define SIM_GEN_FAIL        -2  /* A generic failure. */
#define SIM_TIME_FAIL       -1  /* A timeout occurred. */
#define SIM_UNSAT            0  /* Unsatisfiable. */
#define SIM_SAT              1  /* Satisfiable. */
#define SIM_UNKNOWN          2  /* Unknowon. */

/* Type of learning. */
#define SIM_RELEVANCE        0  /* Relevance-bounded learning. */
#define SIM_SIZE             1  /* Size learning. */

/* Type of preprocessing. */
#define SIM_NONE             0  /* No preprocessing. */
#define SIM_LOOKAHEAD        1  /* Apply look ahead techniques. */
#define SIM_1LIT             2  /* Apply 1-literal rule. */
#define SIM_TAIL             3  /* Apply tail resolution. */
#define SIM_SUBS             4  /* Apply clause subsumption. */
#define SIM_FAILED           5  /* Apply failed literal propagation. */
#define SIM_BINARY           6  /* Apply binary clauses propagation. */
#define SIM_RESOLVE          7  /* Apply resolution to add small clauses. */

/* Parameters. */
#define  SIM_ASK_DEFAULT    -1  /* Ask the default for a parameter. */
#define  SIM_TIMEOUT         0  /* Maximum amount of time. */
#define  SIM_MEMOUT          1  /* Maximum amount of memory. */
#define  SIM_HEURISTICS      2  /* Heuristic choice. */
#define  SIM_SOL_NUM         3  /* Requested number of solutions. */
#define  SIM_LEARN_ORDER     4  /* Learn order. */
#define  SIM_LEARN_TYPE      5  /* Learning type. */
#define  SIM_INDEP_PROPS     6  /* If non-0 prefers model variables. */
#define  SIM_PPROC_STRENGTH  7  /* Preprocessing strength. */
#define  SIM_RND_SEED        8  /* Random seed. */
#define  SIM_VERBOSITY       9  /* Verbosity level of the output. */
#define  SIM_RUN_TRACE      10  /* Running trace quantization steps. */
#define  SIM_HEUR_PARAM     11  /* An optional parameter for heuristics. */
#define  SIM_MAX_VAR_NUM    12  /* Number of propositions. */
#define  SIM_MAX_CL_NUM     13  /* Number of clauses. */
#define  SIM_PARAM_NUM      14  /* How many parameters. */

/* Heuristics. */
#define SIM_USR_HEUR         0  /* Ask the user for a literal. */
#define SIM_RND_HEUR         1  /* Random literal and sign. */
#define SIM_JW_HEUR          2  /* Use Jeroslow-Wang's heuristics. */
#define SIM_JW2_HEUR         3  /* As above but scores propositions. */
#define SIM_SATO_HEUR        4  /* Use Sato 3.2  heuristics. */
#define SIM_SATZ_HEUR        5  /* Use Satz heuristics. */
#define SIM_BOEHM_HEUR       6  /* Use Boehm heuristics. */
#define SIM_MOMS_HEUR        7  /* Use M.O.M.S. heuristics. */
#define SIM_RELSAT_HEUR      8  /* Use Relsat 2.0 heuristics. */
#define SIM_UNITIE_HEUR      9  /* Use BCP-based heuristics. */
#define SIM_HEUR_NUM        10  /* How many heuristics. */


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************
  Synopsis      [A DLL module.]
  Description   [Holds the state and the control information of a DLL module.]
  SeeAlso       []
******************************************************************************/
typedef struct Sim_Dll Sim_Dll_t; 

/*---------------------------------------------------------------------------*/
/* Enumerated type declarations                                              */
/*---------------------------------------------------------------------------*/

/**Enum************************************************************************
  Synopsis    [Error codes.]
  Description [Error codes.]
  SeeAlso     []
******************************************************************************/
typedef enum {
    SIM_NO_ERROR,
    SIM_IO_ERROR,
    SIM_MEMORY_ERROR,
    SIM_INTERNAL_ERROR,
    SIM_VERIFY_ERROR
} Sim_ErrCode_c;

/**Enum************************************************************************
  Synopsis    [Error locations.]
  Description [Error locations.]
  SeeAlso     []
******************************************************************************/
typedef enum {
  SIM_NO_LOCATION,
  SIM_EP_LOCATION,
  SIM_RP_LOCATION,
  SIM_BCP_LOCATION,
  SIM_MLF_LOCATION,
  SIM_HEUR_LOCATION,
  SIM_BJ_LOCATION,
  SIM_LRN_LOCATION,
  SIM_CONS_LOCATION
} Sim_ErrLocation_c;


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************
  Synopsis     [Avalable heuristics names.]
  Description  [Strings indexed by {SIM_USR_HEUR ... SIM_UNITIE_HEUR}.]
  SeeAlso      []
******************************************************************************/
EXTERN char * Sim_heuristicsName[SIM_HEUR_NUM];

/**Variable********************************************************************
  Synopsis     [Parameter names.]
  Description  [Strings indexed by {SIM_TIMEOUT ... SIM_CL_NUM}.]
  SeeAlso      []
******************************************************************************/
EXTERN char * Sim_paramName[SIM_PARAM_NUM];


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int * Sim_ParamInit();
EXTERN int * Sim_ParamSet(int * param, int pName, int pValue);
EXTERN void Sim_DllInit(int * params);
EXTERN int Sim_DllNewCl();
EXTERN int Sim_DllNewClSize(short size);
EXTERN int Sim_DllAddLit(int clId, int lit);
EXTERN int Sim_DllCommitCl(int clId);
EXTERN void Sim_DllPropMakeIndep(int prop);
EXTERN int * Sim_DllGetSolution();
EXTERN int * Sim_DllGetStack();
EXTERN void Sim_DllClear();
EXTERN Sim_ErrCode_c Sim_GetErrorCode();
EXTERN Sim_ErrLocation_c Sim_GetErrLocation();
EXTERN void Sim_DllPrintFormulaDimacs();
EXTERN void Sim_DllPrintModel();
EXTERN void Sim_DllPrintStats();
EXTERN void Sim_DllPrintTimers();
EXTERN void Sim_DllPrintResult(int res);
EXTERN void Sim_DllPrintParams();
EXTERN int Sim_DllParseDimacs(FILE * inFile, int * params);
EXTERN int Sim_DllSolve();

/**AutomaticEnd***************************************************************/

#endif /* _SIM */


