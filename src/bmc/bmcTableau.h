/**CHeaderFile*****************************************************************

  FileName    [bmcTableau.h]

  PackageName [bmc]

  Synopsis    [Public interface for tableau-related functionalities]

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

  Revision    [$Id: bmcTableau.h,v 1.2.4.1.2.1 2004/07/27 12:12:13 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_TABLEAU__H
#define _BMC_TABLEAU__H

#include "bmcFsm.h"

#include "util.h"
#include "be/be.h"
#include "node/node.h"


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

EXTERN be_ptr
Bmc_Tableau_GetNoLoop ARGS((const Bmc_Fsm_ptr be_fsm,
                            const node_ptr ltl_wff,
                            const int k));

EXTERN be_ptr
Bmc_Tableau_GetSingleLoop ARGS((const Bmc_Fsm_ptr be_fsm,
                                const node_ptr ltl_wff,
                                const int k, const int l));

EXTERN be_ptr
Bmc_Tableau_GetAllLoops ARGS((const Bmc_Fsm_ptr be_fsm,
                              const node_ptr ltl_wff,
                              const int k, const int l));

EXTERN be_ptr
Bmc_Tableau_GetAllLoopsDepth1 ARGS((const Bmc_Fsm_ptr be_fsm,
                                    const node_ptr ltl_wff, const int k));

EXTERN be_ptr
Bmc_Tableau_GetLtlTableau ARGS((const Bmc_Fsm_ptr be_fsm,
				const node_ptr ltl_wff,
				const int k, const int l));

/**AutomaticEnd***************************************************************/

#endif /* _BMC_TABLEAU__H */
