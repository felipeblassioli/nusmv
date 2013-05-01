/**CFile***********************************************************************

  FileName    [TracePlugin.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TracePlugin object.]

  Description [ This file contains the definition of \"TracePlugin\" class.]
		
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

#include "TracePlugin.h"
#include "TracePlugin_private.h"

static char rcsid[] UTIL_UNUSED = "$Id: TracePlugin.c,v 1.1.2.9 2004/05/31 13:58:13 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void trace_plugin_finalize ARGS((Object_ptr object, void *dummy));

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Action associated with the Class TracePlugin.]

  Description [Executes the different action method, corresponding to which
  derived class instance belongs to, on the trace.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TracePlugin_action(const TracePlugin_ptr self, Trace_ptr trace, void* arg)
{
  TRACE_PLUGIN_CHECK_INSTANCE(self);

  return self->action(self, trace, arg);
}

/**Function********************************************************************

  Synopsis    [Returns a short description of the plugin.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
char* TracePlugin_get_desc(const TracePlugin_ptr self)
{
  TRACE_PLUGIN_CHECK_INSTANCE(self);

  return self->desc;
}

/* ---------------------------------------------------------------------- */
/*     Private Methods                                                    */
/* ---------------------------------------------------------------------- */
/**Function********************************************************************

  Synopsis    [Action associated with the Class action.]

  Description [It is a pure virtual function and TracePlugin is an abstract
  base class. Every derived class must ovewrwrite this function. It returns 1
  if operation is successful, 0 otherwise.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int trace_plugin_action(const TracePlugin_ptr self,
                        Trace_ptr trace, void* arg)
{
  nusmv_assert(false); /* Pure Virtual Member Function */
  return 0;
}

/**Function********************************************************************

  Synopsis    [This function initializes the plugin class.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_plugin_init(TracePlugin_ptr self, char* desc)
{
  object_init(OBJECT(self));

  OVERRIDE(Object, finalize) = trace_plugin_finalize;
  OVERRIDE(TracePlugin, action) = trace_plugin_action;

  self->desc = ALLOC(char, strlen(desc) + 1);
  nusmv_assert(self->desc != (char*) NULL);
  strncpy(self->desc, desc, strlen(desc) + 1);
}

/**Function********************************************************************

  Synopsis    [This function de-initializes the plugin class.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_plugin_deinit(TracePlugin_ptr self)
{
  FREE(self->desc);
  object_deinit(OBJECT(self));
}

/*---------------------------------------------------------------------------*/
/* Static functions                                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Finalize method of plugin class.]

  Description [Pure virtual function. This must be refined by derived classes.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void  trace_plugin_finalize(Object_ptr object, void* dummy)
{
  TracePlugin_ptr self = TRACE_PLUGIN(object);

  trace_plugin_deinit(self);
  nusmv_assert(false);
}

