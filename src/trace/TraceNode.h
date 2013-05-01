/**CHeaderFile*****************************************************************

  FileName    [TraceNode.h]

  PackageName [trace]

  Synopsis    [The header file for the TraceNode class]

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

#ifndef __TRACE_NODE__H
#define __TRACE_NODE__H

#include "utils/utils.h"
#include "dd/dd.h" 
#include "fsm/bdd/BddFsm.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct TraceNode_TAG* TraceNode_ptr;

#define TRACE_NODE(x) \
          ((TraceNode_ptr) x)
	
#define TRACE_NODE_CHECK_INSTANCE(x) \
	   (nusmv_assert(TRACE_NODE(x) != TRACE_NODE(NULL)))

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

/* TraceNode Constructor/ Destructor */
EXTERN TraceNode_ptr TraceNode_create ARGS((void));
EXTERN void TraceNode_destroy ARGS((TraceNode_ptr self, DdManager* dd));

/* TraceNode Getters */
EXTERN void TraceNode_set_input ARGS((TraceNode_ptr self, DdManager* dd,
                                      BddInputs inp_i));

EXTERN void TraceNode_set_state ARGS((TraceNode_ptr self, DdManager* dd,
                                      BddStates state_i));

/* TraceNode setters */
EXTERN BddInputs TraceNode_get_input ARGS((const TraceNode_ptr self));
EXTERN BddStates TraceNode_get_state ARGS((const TraceNode_ptr self));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_NODE__H */
