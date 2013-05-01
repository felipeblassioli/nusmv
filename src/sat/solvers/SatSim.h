/**CHeaderFile*****************************************************************

  FileName    [SatSim.h]

  PackageName [sat]

  Synopsis    [The header file for the SatSim class.]

  Description [Sim (non-incremental) solver implementation of the generic 
  SatSolver class ]

  SeeAlso     []

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``sat'' package of NuSMV version 2. 
  Copyright (C) 2004 by ITC-irst.

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

  Revision    [$Id: SatSim.h,v 1.1.2.3 2005/11/16 12:09:46 nusmv Exp $]

******************************************************************************/
#ifndef __SAT_SIM__H
#define __SAT_SIM__H

#include "sat/SatSolver.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct SatSim_TAG* SatSim_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define SAT_SIM(x) \
	 ((SatSim_ptr) x)

#define SAT_SIM_CHECK_INSTANCE(x) \
	 (nusmv_assert(SAT_SIM(x) != SAT_SIM(NULL)))

/**AutomaticStart*************************************************************/ 
/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN SatSim_ptr SatSim_create ARGS((const char* name));
EXTERN void SatSim_destroy ARGS((SatSim_ptr self));
/**AutomaticEnd***************************************************************/

#endif /* __SAT_SIM__H */
