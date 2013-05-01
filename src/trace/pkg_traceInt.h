/**CHeaderFile*****************************************************************

  FileName    [pkg_traceInt.h]

  PackageName [trace]

  Synopsis    [The internal header file for the <tt>trace</tt> package]

  Description []

  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2. 
  Copyright (C) 2003 by ITC-irst.

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
#ifndef __PKG_TRACE_INT__H
#define __PKG_TRACE_INT__H

#include "utils/utils.h" /* For EXTERN */ 
#include "node/node.h"   
#include "TraceManager.h" 
#include "compile/compile.h" 
#include "compile/compileFsmMgr.h" 
#include "opt/opt.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

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
EXTERN TraceManager_ptr global_trace_manager;
EXTERN FsmBuilder_ptr global_fsm_builder;
EXTERN options_ptr options; 
EXTERN cmp_struct_ptr cmps;

EXTERN node_ptr proc_selector_internal_vname;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/
/*---------------------------------------------------------------------------*/
/* Function prototype                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_INT__H */
