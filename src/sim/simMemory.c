/**CFile***********************************************************************

  FileName    [simMemory.c]

  PackageName [sim]

  Synopsis    [Memory handling]

  Description [Internal procedures included in this module:
		<ul>
		<li> <b>SimDllStartMemory()</b> begin memory accounting
		<li> <b>SimDllStopMemory()</b> end memory accounting
		<li> <b>SimDllSetMemout()</b> sets the memory-out
		</ul>]
		
  SeeAlso     [sim.h]

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

  Revision    [v. 1beta]

******************************************************************************/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "simInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#if HAVE_SYS_TIME_H
#if HAVE_SYS_RESOURCE_H
#if HAVE_UNISTD_H

/* A lot of stuff, needed by getrusage, get/setrlimit to work properly. */
#define HAVE_MEMORY

#endif
#endif
#endif


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

  Synopsis    [Starts memory accounting.]

  Description [Sets the given counter to the current unshared data size.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
SimDllStartMemory(
  int             i)		  
{
  
#ifdef HAVE_MEMORY
  struct rusage t;

  getrusage(RUSAGE_SELF, &t);
  SimStat[i] = (int)t.ru_idrss;
#endif

  return;

} /* End of SimDllBeginMemory. */


/**Function********************************************************************

  Synopsis    [Stops memory accounting.]

  Description [Sets the given counter to the current unshared data size
               minus the old counter value.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
SimDllStopMemory(
  int             i)
{
  
#ifdef HAVE_MEMORY
  struct rusage t;
  long          now;

  getrusage(RUSAGE_SELF, &t);
  now = t.ru_idrss - (long)SimStat[i];
  SimStat[i] = (int)now;
#endif

  return;

} /* End of SimDllStopMemory. */


/**Function********************************************************************

  Synopsis    [Sets the memeout on the process.]

  Description [Sets the specified memeout using the CPU resource limit
               mechanism.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
SimDllSetMemout(
  int             mmout)
{
  
#ifdef HAVE_MEMORY
  struct rlimit r;          
  
  if (mmout > 0) {
    getrlimit(RLIMIT_DATA, &r);
    r.rlim_cur = mmout;       
    setrlimit(RLIMIT_DATA, &r);
  }
#endif
  
  return;
  
} /* End of SimSetMemout. */







