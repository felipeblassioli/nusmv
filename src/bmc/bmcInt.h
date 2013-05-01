/**CHeaderFile*****************************************************************

  FileName    [bmcInt.h]

  PackageName [bmc]

  Synopsis    [The private interfaces for the <tt>bmc</tt> package]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by ITC-irst and University of Trento.

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

  Revision    [$Id: bmcInt.h,v 1.31.2.7.2.7 2005/11/16 12:09:45 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_INT_H
#define _BMC_INT_H


#include <time.h>
#include <limits.h>
#include <stdio.h>

#include "bmcVarsMgr.h"

#include "utils/utils.h"
#include "node/node.h"
#include "compile/compile.h"
#include "dd/dd.h"
#include "opt/opt.h"
#include "be/be.h"
#include "compile/compileFsmMgr.h"
#include "trace/TraceManager.h"


/* Uncomment the following line to print out benchmarking info */
/* #define BENCHMARKING */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define BMC_DUMP_FILENAME_MAXLEN 4096

#define BMC_NO_PROPERTY_INDEX -1

#define BMC_BEXP_OUTPUT_SMV 0
#define BMC_BEXP_OUTPUT_LB 1

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;
EXTERN cmp_struct_ptr cmps;

EXTERN options_ptr options;
EXTERN DdManager* dd_manager;
EXTERN FsmBuilder_ptr global_fsm_builder;
EXTERN TraceManager_ptr global_trace_manager;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#ifndef max
#define max(_a_, _b_) ((_a_ < _b_) ? _b_ : _a_)
#endif

#ifndef min
#define min(_a_, _b_) ((_a_ < _b_) ? _a_ : _b_)
#endif

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void bmc_init_wff2nnf_hash ARGS((void));

EXTERN void bmc_quit_wff2nnf_hash ARGS((void)); 

EXTERN int
Bmc_TestTableau ARGS((int argc, char ** argv));

EXTERN void Bmc_TestReset    ARGS((void));


EXTERN void
Bmc_PrintStats ARGS((Be_Manager_ptr beManager, int clustSize, FILE* outFile));


EXTERN boolean
isPureFuture ARGS((const node_ptr pltl_wff));

EXTERN be_ptr
Bmc_GetTestTableau ARGS((const BmcVarsMgr_ptr vars_mgr,
                         const node_ptr ltl_wff,
                         const int k, const int l));

EXTERN be_ptr
BmcInt_Tableau_GetAtTime ARGS((const BmcVarsMgr_ptr vars_mgr,
			       const node_ptr ltl_wff,
			       const int time, const int k, const int l));

/* ================================================== */
/* Tableaux for an LTL formula:                       */
EXTERN be_ptr
bmc_tableauGetNextAtTime ARGS((const BmcVarsMgr_ptr vars_mgr,
                               const node_ptr ltl_wff,
                               const int time, const int k, const int l));

EXTERN be_ptr
bmc_tableauGetEventuallyAtTime ARGS((const BmcVarsMgr_ptr vars_mgr,
                                     const node_ptr ltl_wff,
                                     const int intime, const int k,
                                     const int l));

EXTERN be_ptr
bmc_tableauGetGloballyAtTime ARGS((const BmcVarsMgr_ptr vars_mgr,
                                   const node_ptr ltl_wff,
                                   const int intime, const int k,
                                   const int l));

EXTERN be_ptr
bmc_tableauGetUntilAtTime ARGS((const BmcVarsMgr_ptr vars_mgr,
                                const node_ptr p, const node_ptr q,
                                const int time, const int k, const int l));

EXTERN be_ptr
bmc_tableauGetReleasesAtTime ARGS((const BmcVarsMgr_ptr vars_mgr,
                                   const node_ptr p, const node_ptr q,
                                   const int time, const int k, const int l));
/* ================================================== */


/* ================================================== */
/* Tableaux for a PLTL formula:                       */
EXTERN be_ptr
Bmc_TableauPLTL_GetTableau ARGS((const BmcVarsMgr_ptr vars_mgr,
                                 const node_ptr pltl_wff,
                                 const int k, const int l));

EXTERN be_ptr
Bmc_TableauPLTL_GetAllTimeTableau ARGS((const BmcVarsMgr_ptr vars_mgr,
                                        const node_ptr pltl_wff,
                                        const int k));
/* ================================================== */


/* ================================================== */
/* Conv module:                                       */
EXTERN void Bmc_Conv_init_cache ARGS((void));
EXTERN void Bmc_Conv_quit_cache ARGS((void));

/* ================================================== */


/* ================================================== */
/* Utils module:                                      */

EXTERN lsList 
Bmc_Utils_get_vars_list_for_uniqueness ARGS((BmcVarsMgr_ptr vars_mgr, 
					     Prop_ptr invarprop));

/* ================================================== */

/**AutomaticEnd***************************************************************/

#endif /* _BMC_INT_H */

























