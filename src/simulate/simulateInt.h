 /**CHeaderFile*****************************************************************

  FileName    [simulateInt.h]

  PackageName [simulate]

  Synopsis    [Internal Header File for the simulate package]

  Description [Internal Header File for the simulate package]

  SeeAlso     [simulate.c]

  Author      [Andrea Morichetti]

  Copyright   [
  This file is part of the ``simulate'' package of NuSMV version 2. 
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

  Revision    [$Id: simulateInt.h,v 1.4.4.12.2.1 2005/11/16 12:09:47 nusmv Exp $]

******************************************************************************/

#ifndef _SIMULATEINT
#define _SIMULATEINT

#include <stdio.h>
#include <stdlib.h>

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "simulate.h"

#include "utils/utils.h"
#include "dd/dd.h"
#include "opt/opt.h"
#include "fsm/bdd/BddFsm.h"
#include "compile/compile.h"
#include "trace/Trace.h"
#include "trace/TraceManager.h"
#include "compile/compileFsmMgr.h"


#if HAVE_SYS_SIGNAL_H
#  include <sys/signal.h>
#endif
#if HAVE_SIGNAL_H
#  include <signal.h>
#endif

#include <setjmp.h>
#include <assert.h>



/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/* maximum length of the string containing constraints */
#define CONSTR_LENGTH 256
/* Length of the string used for the choice entered in interac. sim.*/
#define CHOICE_LENGTH 8


EXTERN DdManager* dd_manager;
EXTERN options_ptr options;
EXTERN cmp_struct_ptr cmps;
EXTERN int trace_number;

EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;

EXTERN TraceManager_ptr global_trace_manager;
EXTERN FsmBuilder_ptr global_fsm_builder; 

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN int 
Simulate_CmdPickOneState ARGS((BddFsm_ptr, Simulation_Mode, int, char *));
EXTERN int Simulate_CmdShowTraces ARGS((int, int, boolean, char *));

EXTERN bdd_ptr current_state_bdd_get ARGS((void));
EXTERN void current_state_bdd_reset ARGS((void)); 
EXTERN void current_state_set ARGS((bdd_ptr state, TraceLabel label)); 
EXTERN void current_state_label_reset ARGS((void)); 
EXTERN void current_state_bdd_free ARGS((void));

EXTERN node_ptr proc_selector_internal_vname;

#endif /* _SIMULATEINT */
