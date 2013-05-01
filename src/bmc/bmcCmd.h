/**CHeaderFile*****************************************************************

  FileName    [bmcCmd.h]

  PackageName [bmc]

  Synopsis    [The header file for the <tt>cmd</tt> module, the user 
  commands handling layer.]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by ITC-irst and University of Trento.

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

  For more information on NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

  Revision    [$Id: bmcCmd.h,v 1.2.4.1.2.4 2005/11/16 12:09:45 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_CMD_H
#define _BMC_CMD_H

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "utils/utils.h"

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int Bmc_CommandBmcSetup ARGS((const int argc, const char** argv)); 
EXTERN int Bmc_CommandBmcSimulate ARGS((const int argc, const char** argv)); 

EXTERN int Bmc_CommandGenLtlSpecBmc ARGS((const int argc, const char** argv));
EXTERN int 
Bmc_CommandGenLtlSpecBmcOnePb ARGS((const int argc, const char** argv)); 

EXTERN int 
Bmc_CommandCheckLtlSpecBmc ARGS((const int argc, const char** argv)); 
EXTERN int 
Bmc_CommandCheckLtlSpecBmcOnePb ARGS((const int argc, const char** argv)); 

EXTERN int Bmc_CommandGenInvarBmc ARGS((const int argc, const char** argv));

EXTERN int Bmc_CommandCheckInvarBmc ARGS((const int argc, const char** argv));


#if HAVE_INCREMENTAL_SAT
EXTERN int 
Bmc_CommandCheckLtlSpecBmcInc ARGS((const int argc, const char** argv)); 

EXTERN int 
Bmc_CommandCheckInvarBmcInc ARGS((const int argc, const char** argv));
#endif

/**AutomaticEnd***************************************************************/

#endif /* _BMC_CMD_H */
