/**CHeaderFile*****************************************************************

  FileName    [Trace.h]

  PackageName [trace]

  Synopsis    [The header file for the trace class.]

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

#ifndef __TRACE_TRACE__H
#define __TRACE_TRACE__H

#include "TraceNode.h" 
#include "utils/utils.h"
#include "utils/array.h"
#include "utils/error.h"
#include "utils/utils_io.h"
#include "dd/dd.h"
#include "fsm/bdd/BddFsm.h" 
#include "node/node.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*///
typedef enum TraceType_TAG {
  TRACE_TYPE_INVALID = -1,
  TRACE_TYPE_CNTEXAMPLE = 0,
  TRACE_TYPE_SIMULATION
} TraceType;

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct Trace_TAG* Trace_ptr;
typedef struct TraceNode_TAG* TraceIterator_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define TRACE(x) \
	  ((Trace_ptr) x)
	
#define TRACE_CHECK_INSTANCE(x) \
	  (nusmv_assert(TRACE(x) != TRACE(NULL)))

#define TRACE_ITERATOR(x) \
	  ((TraceIterator_ptr) x)
	
#define TRACE_ITERATOR_CHECK_INSTANCE(x) \
	  (nusmv_assert(TRACE_ITERATOR(x) != TRACE_ITERATOR(NULL)))

#define TRACE_END_ITERATOR TRACE_ITERATOR(NULL)

/**AutomaticStart*************************************************************/ 
/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
/* Trace Constructor/Destructors */
EXTERN Trace_ptr Trace_create ARGS((BddEnc_ptr enc,
                                    const char* desc,
                                    const TraceType type,
                                    BddStates start_state));

EXTERN void Trace_destroy ARGS((Trace_ptr self));

EXTERN Trace_ptr
Trace_create_from_state_list ARGS((BddEnc_ptr enc, 
				   const char* desc, 
				   const TraceType type, 
				   node_ptr path));

EXTERN Trace_ptr
Trace_create_from_state_input_list ARGS((BddEnc_ptr enc, 
					 const char* desc, 
					 const TraceType type, 
					 node_ptr path));

EXTERN Trace_ptr Trace_copy ARGS((const Trace_ptr self));
EXTERN Trace_ptr 
Trace_copy_prefix_until_iterator ARGS((const Trace_ptr self, 
                                 const TraceIterator_ptr until_here));

/* Trace Setters */
EXTERN void Trace_set_start_state ARGS((Trace_ptr self, BddStates s));
EXTERN void Trace_set_desc ARGS((Trace_ptr self, const char* desc));
EXTERN void Trace_set_type ARGS((Trace_ptr self, const TraceType type));

/* Trace Getters */
EXTERN TraceIterator_ptr Trace_begin ARGS((const Trace_ptr self));

EXTERN TraceIterator_ptr 
Trace_get_iterator ARGS((const Trace_ptr self, const TraceNode_ptr node));

EXTERN TraceNode_ptr Trace_get_loopback ARGS((const Trace_ptr self));
EXTERN array_t* Trace_get_all_loopbacks ARGS((const Trace_ptr self));
EXTERN TraceNode_ptr Trace_get_ith_node ARGS((const Trace_ptr self, int i));
EXTERN BddStates Trace_get_ith_state ARGS((const Trace_ptr self, int i));
EXTERN int Trace_get_length ARGS((const Trace_ptr self));
EXTERN int Trace_get_id ARGS((const Trace_ptr self));
EXTERN BddEnc_ptr Trace_get_enc ARGS((const Trace_ptr self)); 
EXTERN const char* Trace_get_desc ARGS((const Trace_ptr self));
EXTERN TraceType Trace_get_type ARGS((const Trace_ptr self));
EXTERN TraceNode_ptr Trace_get_last_node ARGS((const Trace_ptr self));

EXTERN TraceNode_ptr 
Trace_get_node ARGS((const Trace_ptr self, const TraceIterator_ptr iter));

/* TraceIterator functions */
EXTERN TraceIterator_ptr 
TraceIterator_get_next ARGS((const TraceIterator_ptr iter));

EXTERN boolean TraceIterator_is_end ARGS((const TraceIterator_ptr iter));

/* Other Functions */
EXTERN void 
Trace_append ARGS((Trace_ptr self, BddInputs inp_i, BddStates state_i)); 

EXTERN void 
Trace_append_from_list_state ARGS((Trace_ptr self, node_ptr path));

EXTERN void 
Trace_append_from_list_input_state ARGS((Trace_ptr self, node_ptr path));

EXTERN boolean Trace_is_registered ARGS((const Trace_ptr self));
EXTERN boolean Trace_is_valid ARGS((Trace_ptr self));

/* TraceType Methods */
const char* TraceType_to_string ARGS((const TraceType self));
TraceType TraceType_from_string ARGS((const char* str));
/**AutomaticEnd***************************************************************/

#endif /* __TRACE_TRACE__H  */
