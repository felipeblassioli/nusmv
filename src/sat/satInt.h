/**CHeaderFile*****************************************************************

  FileName    [satInt.h]

  PackageName [sat]

  Synopsis    [The internal header file for the <tt>sat</tt> package]

  Description [At the momemnt this file just provide the facilities to 
  access 'options' unitilities]

  SeeAlso     []

  Author      [Andrei Tchaltsev, Roberto Cavada]]

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

  Revision    [$Id: satInt.h,v 1.3.4.2.2.1 2004/07/27 12:12:15 nusmv Exp $]

******************************************************************************/ 

#ifndef _SAT_INT_H
#define _SAT_INT_H


#include "sat.h"
#include "opt/opt.h"


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

EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;
EXTERN options_ptr options;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/


#endif /* _SAT_INT_H */
