/**CHeaderFile*****************************************************************

  FileName    [bmc.h]

  PackageName [bmc]

  Synopsis    [The header file for the <tt>bmc</tt> package]

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

  Revision    [$Id: bmc.h,v 1.22.4.2.2.2 2005/11/16 12:09:44 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_H
#define _BMC_H

#include "utils/utils.h" /* for EXTERN and ARGS */

/* all BMC modules: */
#include "bmcCmd.h"
#include "bmcBmc.h"
#include "bmcPkg.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Constant********************************************************************

  Synopsis [The names for INVAR solving algorithms (incremental and
  non-incremental).]

  Description        []

  SeeAlso            []

******************************************************************************/
#define BMC_INVAR_ALG_CLASSIC       "classic"
#define BMC_INVAR_ALG_EEN_SORENSSON "een-sorensson"
#define BMC_INC_INVAR_ALG_DUAL      "dual"
#define BMC_INC_INVAR_ALG_ZIGZAG    "zigzag"


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


/**AutomaticEnd***************************************************************/

#endif /* _BMC_H */
