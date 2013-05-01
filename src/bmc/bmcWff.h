/**CHeaderFile*****************************************************************

  FileName    [bmcWff.h]

  PackageName [bmc]

  Synopsis    [Wff related public interface]

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

  For more information of NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

  Revision    [$Id: bmcWff.h,v 1.2.4.1.2.1 2004/07/27 12:12:13 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_WFF__H
#define _BMC_WFF__H

#include "node/node.h"
#include "utils/utils.h"


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

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN node_ptr Bmc_Wff_MkNnf ARGS((node_ptr wff));

EXTERN int Bmc_Wff_GetDepth ARGS((node_ptr ltl_wff));

EXTERN node_ptr Bmc_Wff_MkTruth ARGS((void));

EXTERN node_ptr Bmc_Wff_MkFalsity ARGS((void));

EXTERN node_ptr Bmc_Wff_MkNot ARGS((node_ptr arg));

EXTERN node_ptr Bmc_Wff_MkAnd ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Bmc_Wff_MkOr ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Bmc_Wff_MkImplies ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Bmc_Wff_MkIff ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Bmc_Wff_MkNext ARGS((node_ptr arg));

EXTERN node_ptr Bmc_Wff_MkXopNext ARGS((node_ptr arg, int x));

EXTERN node_ptr Bmc_Wff_MkOpNext ARGS((node_ptr arg));

EXTERN node_ptr Bmc_Wff_MkOpPrec ARGS((node_ptr arg));

EXTERN node_ptr Bmc_Wff_MkGlobally ARGS((node_ptr arg));

EXTERN node_ptr Bmc_Wff_MkHistorically ARGS((node_ptr arg));

EXTERN node_ptr Bmc_Wff_MkEventually ARGS((node_ptr arg));

EXTERN node_ptr Bmc_Wff_MkOnce ARGS((node_ptr arg));

EXTERN node_ptr Bmc_Wff_MkUntil ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Bmc_Wff_MkSince ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Bmc_Wff_MkReleases ARGS((node_ptr arg1, node_ptr arg2));

EXTERN node_ptr Bmc_Wff_MkTriggered ARGS((node_ptr arg1, node_ptr arg2));

/**AutomaticEnd***************************************************************/

#endif /* _BMC_WFF__H */

