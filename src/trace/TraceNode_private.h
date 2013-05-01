/**CHeaderFile*****************************************************************

  FileName    [TraceNode_private.h]

  PackageName [trace]

  Synopsis    [The private header file for the TraceNode class]

  Description []

  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2. 
  Copyright (C) 2003 by ITC-irst.

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

******************************************************************************/

#ifndef __TRACE_NODE_PRIVATE__H
#define __TRACE_NODE_PRIVATE__H

#include "TraceNode.h"
#include "utils/utils.h"
#include "dd/dd.h" 
#include "fsm/bdd/BddFsm.h"

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
/* Function prototype                                                        */
/*---------------------------------------------------------------------------*/
/* TraceNode Getters */
EXTERN void trace_node_set_next ARGS((TraceNode_ptr self, TraceNode_ptr next));
EXTERN void trace_node_set_prev ARGS((TraceNode_ptr self, TraceNode_ptr prev));

/* TraceNode setters */
EXTERN TraceNode_ptr trace_node_get_next ARGS((const TraceNode_ptr self));
EXTERN TraceNode_ptr trace_node_get_prev ARGS((const TraceNode_ptr self));

/* Other Functions */
EXTERN boolean trace_node_is_last ARGS((const TraceNode_ptr self));
EXTERN boolean trace_node_is_first ARGS((const TraceNode_ptr self));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_NODE_PRIVATE__H */
