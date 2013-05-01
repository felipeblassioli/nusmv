/**CHeaderFile*****************************************************************

  FileName    [ltl.h]

  PackageName [ltl]

  Synopsis    [Routines to handle with LTL model checking.]

  Description [Here we perform the reduction of LTL model checking to
  CTL model checking. The technique adopted has been taken from \[1\].
  <ol>
    <li>O. Grumberg E. Clarke and K. Hamaguchi. "Another Look at LTL Model Checking".
       <em>Formal Methods in System Design</em>, 10(1):57--71, February 1997.</li>
  </ol>
  ]

  SeeAlso     [mc]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``ltl'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and ITC-irst. 

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

  Revision    [$Id: ltl.h,v 1.4.6.2.2.2 2005/11/16 12:09:46 nusmv Exp $]

******************************************************************************/

#ifndef __LTL_H__
#define  __LTL_H__

#include "utils/utils.h"
#include "node/node.h"
#include "prop/prop.h"


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void print_ltlspec ARGS((FILE*, Prop_ptr));
EXTERN void Ltl_Init ARGS((void));
EXTERN void Ltl_CheckLtlSpec ARGS((Prop_ptr prop));

#endif /*  __LTL_H__ */
