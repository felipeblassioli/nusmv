/**CFile***********************************************************************

  FileName    [TracePlugin_private.c]

  PackageName [trace.plugins]

  Synopsis    [The private interface of class TracePlugin]

  Description [Private definition to be used by derived classes]
		
  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace.plugins'' package of NuSMV version 2. 
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
#ifndef __TRACE_PLUGIN_PRIVATE__H
#define __TRACE_PLUGIN_PRIVATE__H

#include "TracePlugin.h"
#include "utils/utils.h"
#include "utils/object.h"
#include "utils/object_private.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [TracePlugin Class]

  Description [This class defines a prototype for a generic TracePlugin. This
  class is virtual and must be specialized. ]

  SeeAlso     []   
  
******************************************************************************/
typedef struct TracePlugin_TAG
{
  INHERITS_FROM(Object);
  char* desc; /* Short description of the plugin */
  /* ---------------------------------------------------------------------- */ 
  /*     Virtual Methods                                                    */
  /* ---------------------------------------------------------------------- */   
  VIRTUAL int (*action)(const TracePlugin_ptr self, Trace_ptr trace, 
                        void* arg); /*action associated with the trace */
} TracePlugin;

/**AutomaticStart*************************************************************/ 

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
void trace_plugin_init ARGS((TracePlugin_ptr self, char* desc));
void trace_plugin_deinit ARGS((TracePlugin_ptr self));

int trace_plugin_action 
ARGS((const TracePlugin_ptr self, Trace_ptr trace, void* arg));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_PLUGIN_PRIVATE__H */
