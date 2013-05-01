/**CFile***********************************************************************

  FileName    [pkg_trace.c]

  PackageName [trace]

  Synopsis    [Routines related to the trace Package.]

  Description [This file contains routines related to Initializing and
  Quiting trace package. ]
		
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

#include "pkg_trace.h"
#include "TraceManager.h"
#include "pkg_traceInt.h"


static char rcsid[] UTIL_UNUSED = "$Id: pkg_trace.c,v 1.1.2.16.2.1 2005/05/05 13:16:54 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
TraceManager_ptr global_trace_manager = TRACE_MANAGER(NULL);

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Initializes the Trace Package.]

  Description [TraceManager get initialized. ]

  SideEffects []

  SeeAlso     [TracePkg_quit]

******************************************************************************/
void TracePkg_init()
{
  if (global_trace_manager != TRACE_MANAGER(NULL)) {
    TraceManager_destroy(global_trace_manager); 
  }

  global_trace_manager = TraceManager_create();
  nusmv_assert(global_trace_manager != TRACE_MANAGER(NULL));

  TraceManager_init_plugins(global_trace_manager);
}

/**Function********************************************************************

  Synopsis    [Quits the Trace package.]

  Description []

  SideEffects []

  SeeAlso     [TracePkg_init]

******************************************************************************/
void TracePkg_quit()
{
  if (global_trace_manager != TRACE_MANAGER(NULL)) {
    TraceManager_destroy(global_trace_manager);
    global_trace_manager = TRACE_MANAGER(NULL);
  }
}


/**Function********************************************************************

  Synopsis    [Accessor for the global trace manager]

  Description [Can be called only after initialization of the trace package]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceManager_ptr TracePkg_get_global_trace_manager()
{
  TRACE_MANAGER_CHECK_INSTANCE(global_trace_manager);

  return global_trace_manager;
}
