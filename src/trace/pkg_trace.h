/**CHeaderFile*****************************************************************

  FileName    [pkg_trace.h]

  PackageName [trace]

  Synopsis    [The header file for the trace package.]

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
#ifndef __PKG_TRACE__H
#define __PKG_TRACE__H

#include "TraceManager.h"
#include "utils/utils.h" /* For EXTERN and ARGS */

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
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void TracePkg_init ARGS((void));
EXTERN void TracePkg_quit ARGS((void));
EXTERN void traceCmd_init ARGS((void));
EXTERN int CommandShowTraces ARGS((int argc, char** argv));
EXTERN int CommandShowPlugins ARGS((int argc, char** argv));
EXTERN int CommandReadTrace ARGS((int argc, char** argv));
EXTERN TraceManager_ptr TracePkg_get_global_trace_manager ARGS((void));

EXTERN int TracePkg_get_default_trace_plugin ARGS((void));
EXTERN boolean TracePkg_set_default_trace_plugin ARGS((int dp));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE__H  */
