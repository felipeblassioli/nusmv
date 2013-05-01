/**CHeaderFile*****************************************************************

  FileName    [bmcSatTrace.h]

  PackageName [bmc]

  Synopsis    [Interface for class BmcSatTrace]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2004 by ITC-irst and University of Trento.

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

  Revision    [$Id: bmcSatTrace.h,v 1.1.2.2 2004/07/29 09:27:04 nusmv Exp $]

******************************************************************************/

#ifndef __BMC_SAT_TRACE_H__
#define __BMC_SAT_TRACE_H__


#include "bmcVarsMgr.h"

#include "be/be.h"
#include "utils/utils.h"
#include "utils/list.h"
#include "utils/NodeList.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct*********************************************************************

  Synopsis           [This is the BmcSatTrace cass accessor type]
  Description        []
  SeeAlso            []

******************************************************************************/
typedef struct BmcSatTrace_TAG* BmcSatTrace_ptr;


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

EXTERN BmcSatTrace_ptr 
BmcSatTrace_create ARGS((be_ptr beProb, lsList beModel));

EXTERN void
BmcSatTrace_destroy ARGS((BmcSatTrace_ptr* self_ref));

EXTERN NodeList_ptr 
BmcSatTrace_get_symbolic_model ARGS((const BmcSatTrace_ptr self,  
				     const BmcVarsMgr_ptr vars_mgr, 
				     const int k));


/**AutomaticEnd***************************************************************/

#endif /* __BMC_SAT_TRACE_H__ */
