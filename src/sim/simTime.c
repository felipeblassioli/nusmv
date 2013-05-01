/**CFile***********************************************************************

  FileName    [simTime.c]

  PackageName [sim]

  Synopsis    [Timers handling]

  Description [Internal procedures included in this module:
		<ul>
		<li> <b>SimDllStartTimer()</b> starts a timer
		<li> <b>SimDllStopTimer()</b> stops a timer
		<li> <b>SimDllSetTimout()</b> sets the timeout
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

  Revision    [v. 1.0]

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
#define HAVE_TIMER

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

  Synopsis    [Starts a timer.]

  Description [Sets the specified timer to the current CPU resource usage.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
SimDllStartTimer(
  int             timer)
{
  
#ifdef HAVE_TIMER
  struct rusage t;

  getrusage(RUSAGE_SELF, &t);
  SimTimer[timer] = (float)t.ru_utime.tv_usec / 1.0e6;
  SimTimer[timer] += (float)t.ru_utime.tv_sec;
#endif

  return;

} /* End of SimDllStartTimer. */


/**Function********************************************************************

  Synopsis    [Stops a timer.]

  Description [Sets the specified timer to the current CPU resource usage
               minus the old timer value.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
SimDllStopTimer(
  int             timer)
{
  
#ifdef HAVE_TIMER
  struct rusage t;
  float         now;

  getrusage(RUSAGE_SELF, &t);
  now = ((float)t.ru_utime.tv_usec / 1.0e6) + (float)t.ru_utime.tv_sec;
  if (now <= SimTimer[timer]) { 
    SimTimer[timer] = 0.0;
  } else {
    SimTimer[timer] = now - SimTimer[timer];
  }
#endif

  return;

} /* End of SimDllStopTimer. */


/**Function********************************************************************

  Synopsis    [Sets the timeout on the process.]

  Description [Sets the specified timeout using the CPU resource limit
               mechanism.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
SimDllSetTimeout(
  int             tmout)
{
  
#ifdef HAVE_TIMER
  struct rlimit r;          
  
  if (tmout > 0) {
    getrlimit(RLIMIT_CPU, &r);
    r.rlim_cur = tmout;       
    setrlimit(RLIMIT_CPU, &r);
  }
#endif
  
  return;
  
} /* End of SimSetTimeout. */






