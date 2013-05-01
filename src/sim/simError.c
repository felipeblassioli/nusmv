/**CFile***********************************************************************

  FileName    [simError.c]

  PackageName [sim]

  Synopsis    [Error handling]

  Description [Internal procedures included in this module:
		<ul>
		<li> <b>SimDllThrow()</b> raises a SIG_USR1 to signal
		     that something was wrong
		</ul>]
		
  SeeAlso     []

  Author      [Armando Tacchella Davide Zambonin]

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
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Returns the error code (if any).]

  Description [Returns the error code (if any).]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Sim_ErrCode_c 
Sim_GetErrorCode()
{
  return SimErrCode;
} /* End of Sim_GetErrorCode(). */


/**Function********************************************************************

  Synopsis    [Returns the error location (if any).]

  Description [Returns the error location (if any).]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Sim_ErrLocation_c 
Sim_GetErrorLocation()
{
  return SimErrLocation;
} /* End of Sim_GetErrorLocation(). */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Prompts an error to the user and raises a SIGUSR1.]

  Description [Takes an error type and the error location in the program.
               Prompts the user on stderr and teminates raising the user 
	       defined signal SIGUSR1.] 

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
SimDllThrow(
  Sim_ErrCode_c        errCode,
  Sim_ErrLocation_c    errLoc,
  char               * message)
{
  
  /* Establish what kind of error occurred and prompt it on stderr. */
  switch (errCode) {
  case SIM_NO_ERROR:
    return;
  case SIM_INTERNAL_ERROR: 
    fprintf(stderr, "SIM_INTERNAL_ERROR!\n");
    break;
  case SIM_IO_ERROR: 
    fprintf(stderr, "SIM_IO_ERROR!\n");
    break;
  case SIM_MEMORY_ERROR:
    fprintf(stderr, "SIM_MEMORY_OUT!\n");
    break;
  case SIM_VERIFY_ERROR:
    fprintf(stderr, "SIM_VERIFY_FAILED!\n");
    break;
  }
  
  fprintf(stderr, "%s\n", message);

  /* Update the information about the failure. */
  SimErrCode = errCode;
  SimErrLocation = errLoc;

#if defined HAVE_SIGNAL_H && ! defined __MINGW32__
  /* Raise the signal. */
  raise(SIGUSR1);
#else
  /* Abort. */
  exit((int) errCode);
#endif
  
} /* End of SimDllThrow. */


