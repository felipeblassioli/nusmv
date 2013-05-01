/**CHeaderFile*****************************************************************

  FileName    [ltl2smv.h]

  PackageName [ltl2smv]

  Synopsis    [A function to run ltl2smv routine.]

  Description [Here we perform a convertion from LTL formula to SMV module.
  The invoker should create a file with the LTL formula,
  the function 'ltl2smv' will read this input file and create an output file
  with smv module. Then the invoker can read this output file.

  This file provides routines for reducing LTL model
  checking to CTL model checking. This work is absed on the work
  presented in \[1\]<br>

  <ul><li> O. Grumberg E. Clarke and K. Hamaguchi.  <cite>Another Look
          at LTL Model Checking</cite>. <em>Formal Methods in System
          Design, 10(1):57--71, February 1997.</li>
  </ul>]

  SeeAlso     []

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``ltl2smv'' package of NuSMV version 2. 
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

  Revision    [$Id: ltl2smv.h,v 1.1.2.2 2005/11/16 12:09:46 nusmv Exp $]

******************************************************************************/

#ifndef __LTL_2_SMV_H__
#define __LTL_2_SMV_H__

#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void ltl2smv ARGS((int counter, const char* inFile,
			  const char* outFile));

#endif /* __LTL_2_SMV_H__ */
