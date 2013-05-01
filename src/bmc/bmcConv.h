/**CHeaderFile*****************************************************************

  FileName    [bmcConv.h]

  PackageName [bmc]

  Synopsis    [The conversion be<->bdd module interface]

  Description [This layer contains all functionalities to perform 
  conversion from Boolean Expressions (be) to BDD-based boolean expressions, 
  and vice-versa]

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

  Revision    [$Id: bmcConv.h,v 1.2.4.2.2.1 2004/07/27 12:12:11 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_CONV__H
#define _BMC_CONV__H

#include "bmcVarsMgr.h"
#include "be/be.h"
#include "node/node.h"
#include "utils/utils.h"


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN node_ptr Bmc_Conv_Be2Bexp ARGS((BmcVarsMgr_ptr vars_mgr, be_ptr be)); 
EXTERN be_ptr Bmc_Conv_Bexp2Be ARGS((BmcVarsMgr_ptr vars_mgr, node_ptr bexp)); 
EXTERN node_ptr 
Bmc_Conv_BexpList2BeList ARGS((BmcVarsMgr_ptr vars_mgr, 
			       node_ptr bexp_list)); 

/**AutomaticEnd***************************************************************/

#endif /* _BMC_CONV__H */

