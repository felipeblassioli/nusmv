/**CHeaderFile*****************************************************************

  FileName    [ltlInt.h]

  PackageName [ltl]

  Synopsis    [Internal header of the <tt>ltl</tt> package.]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``ltl'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and ITC-irst. 

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

  Revision    [$Id: ltlInt.h,v 1.3.4.12.2.1 2005/11/16 12:09:46 nusmv Exp $]

******************************************************************************/

#ifndef __LTL_INT_H__
#define __LTL_INT_H__

#include "utils/utils.h"
#include "node/node.h"
#include "dd/dd.h"
#include "compile/compile.h"
#include "opt/opt.h"
#include "fsm/bdd/BddFsm.h"
#include "compile/compileFsmMgr.h"
#include "trace/TraceManager.h"


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern options_ptr options;
extern cmp_struct_ptr cmps;
extern bdd_ptr trans_bdd;
extern bdd_ptr fair_states_bdd;
extern node_ptr fairness_constraints_bdd;
extern FILE* nusmv_stdout;
extern FILE* nusmv_stderr;
extern DdManager* dd_manager;
extern bdd_ptr invar_bdd;
extern bdd_ptr init_bdd;
extern node_ptr parse_tree;

EXTERN FsmBuilder_ptr global_fsm_builder;
EXTERN TraceManager_ptr global_trace_manager;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define WFF_ASSOC_TYPE node_ptr
#define WFF_MKASSOC(a,b) cons((node_ptr)a, (node_ptr)b)
#define WFF_ASSOC_GET_ASSOC(a) (hash_ptr)cdr(a)
#define WFF_ASSOC_GET_INPUTS(a) car(a)

#define WFFR_TYPE node_ptr
#define WFFR_NOTREWRITTEN(wff) cons(wff,Nil)
#define WFFR_REWRITTEN(wff,rew_map) cons(wff,(node_ptr)rew_map)
#define WFFR_ISNOT_REWRITTEN(WFFR) cdr(WFFR) == Nil
#define WFFR_IS_REWRITTEN(WFFR) cdr(WFFR) != Nil
#define WFFR_IS_REWRITTEN(WFFR) cdr(WFFR) != Nil
#define WFFR_GET_WFF(WFFR) car(WFFR)
#define WFFR_GET_ASSOC(WFFR) (WFF_ASSOC_TYPE)cdr(WFFR)
#define WFFR_FREE(wff) free_node(wff)

#define ASSOC_DATA_TYPE node_ptr
#define assoc_mk_data(a,b,c) cons(cons(a, b), c)
#define assoc_get_var_name(a) car(car(a))
#define assoc_get_var_range(a) cdr(car(a))
#define assoc_get_trans(a) cdr(a)
#define free_assoc_data(a) free_node(car(a));free_node(a)

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN node_ptr 
witness ARGS((BddFsm_ptr fsm, BddEnc_ptr enc, bdd_ptr feasible));

EXTERN bdd_ptr feasible ARGS((BddFsm_ptr fsm, BddEnc_ptr enc));

EXTERN WFFR_TYPE ltlRewriteWffIfInputPresent ARGS((Encoding_ptr senc, node_ptr expr));

EXTERN void ltlHandleInputsVars ARGS((Encoding_ptr senc, WFFR_TYPE wr, node_ptr * init, node_ptr * invar, node_ptr * trans));
EXTERN void ltlFreeWFFR_TYPE ARGS((WFFR_TYPE rw));

#endif /* __LTL_INT_H__ */
