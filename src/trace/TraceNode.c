/**CFile***********************************************************************

  FileName    [TraceNode.c]

  PackageName [trace]

  Synopsis    [Routines related to functionality related to a node of a trace.]

  Description [This file contains the definition of the \"TraceNode\" class.]
		
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

#include "TraceNode.h"
#include "TraceNode_private.h"

static char rcsid[] UTIL_UNUSED = "$Id: TraceNode.c,v 1.1.2.6 2004/05/31 13:58:13 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    ["TraceNode" Class]

  Description [ This class defines a node of the trace, a doubly linked list of
  the TraceNodes, and provides functionality for manipulating it.<br>
  <dl> 
      <dt><code>state</code>
          <dd> The BDD cube representing a state. 	
      <dt><code>input</code>
          <dd> The BDD cude representing an  input.
      <dt><code>next</code>
      	   <dd> Pointer to the next TraceNode. 
      <dt><code>prev</code>
           <dd> Pointer to the previous TraceNode.
  </dl>]

  SeeAlso     []   
  
******************************************************************************/
typedef struct TraceNode_TAG {
  BddStates state;		
  BddInputs input;		
  TraceNode_ptr next;	
  TraceNode_ptr prev;	
} TraceNode;

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [TraceNode Class constructor.]

  Description [Allocates and initializes a TraceNode.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceNode_ptr TraceNode_create(void)
{
  TraceNode_ptr self = ALLOC(TraceNode, 1);

  TRACE_NODE_CHECK_INSTANCE(self);

  self->input = BDD_INPUTS(NULL);
  self->state = BDD_STATES(NULL);
  self->next = TRACE_NODE(NULL);
  self->prev = TRACE_NODE(NULL);

  return self;
}

/**Function********************************************************************

  Synopsis    [The "TraceNode" class dectructor.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TraceNode_destroy(TraceNode_ptr self, DdManager* dd)
{
  TRACE_NODE_CHECK_INSTANCE(self);

  if (self->input != BDD_INPUTS(NULL)) bdd_free(dd, self->input);
  if (self->state != BDD_STATES(NULL)) bdd_free(dd, self->state);

  FREE(self);
}

/**Function********************************************************************

  Synopsis    [Sets the value of the input of the traceNode to "inp_i"]

  Description [Given BDD will be referenced.]

  SideEffects [Given BDD will be referenced.]

  SeeAlso     []

******************************************************************************/
void TraceNode_set_input(TraceNode_ptr self, DdManager* dd, BddInputs inp_i)
{
  TRACE_NODE_CHECK_INSTANCE(self);

  if (self->input != BDD_INPUTS(NULL)) {
    bdd_free(dd, self->input);
    self->input = BDD_INPUTS(NULL);
  }
  if (inp_i != BDD_INPUTS(NULL)) self->input = bdd_dup(inp_i);
}

/**Function********************************************************************

  Synopsis    [Sets the value of the state of the traceNode to "trace_i".]

  Description [Given BDD will be referenced.]

  SideEffects [Given BDD will be referenced.]

  SeeAlso     []

******************************************************************************/
void TraceNode_set_state(TraceNode_ptr self, DdManager* dd, BddStates state_i)
{
  TRACE_NODE_CHECK_INSTANCE(self);

  if (self->state != BDD_STATES(NULL)) {
    bdd_free(dd, self->state);
    self->state = BDD_STATES(NULL);
  }

  if (state_i != BDD_STATES(NULL)) self->state = bdd_dup(state_i);
}

/**Function********************************************************************

  Synopsis    [returns the value of the input of the TraceNode.]

  Description [ returned BDD will be referenced]

  SideEffects [ Returned BDD will be referenced.]

  SeeAlso     []

******************************************************************************/
BddInputs TraceNode_get_input(const TraceNode_ptr self)
{
  bdd_ptr tmp = BDD_INPUTS(NULL);

  TRACE_NODE_CHECK_INSTANCE(self);

  if (self->input != BDD_INPUTS(NULL)) {
    tmp = bdd_dup(self->input);
  }

  return tmp;
}

/**Function********************************************************************

  Synopsis    [ returns the value of the state of the TraceNode.]

  Description [ returned BDD will be referenced]

  SideEffects [ returned BDD will be referenced]

  SeeAlso     []

******************************************************************************/
BddStates TraceNode_get_state(const TraceNode_ptr self)
{
  bdd_ptr tmp = BDD_STATES(NULL);

  TRACE_NODE_CHECK_INSTANCE(self);

  if (self->state != BDD_STATES(NULL)) tmp = bdd_dup(self->state);
  return tmp;
}

/* ---------------------------------------------------------------------- */
/*     Private Methods                                                    */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis    [Assigns the node "next" to be the successor of the node "self".]

  Description [Sets the value of the next pointer to "next".]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_node_set_next(TraceNode_ptr self, TraceNode_ptr next)
{
  TRACE_NODE_CHECK_INSTANCE(self);

  self->next = next;
}

/**Function********************************************************************

  Synopsis    [Assigns the node "prev" to be the predecessor of the node "self".]

  Description [Sets the value of the prev pointer to "prev"]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_node_set_prev(TraceNode_ptr self, TraceNode_ptr prev)
{
  TRACE_NODE_CHECK_INSTANCE(self);

  self->prev = prev;
}

/**Function********************************************************************

  Synopsis    [Returns the trace node that follows "self" node.]

  Description [returns the value of the next pointer.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceNode_ptr trace_node_get_next(const TraceNode_ptr self)
{
  TRACE_NODE_CHECK_INSTANCE(self);

  return self->next;
}

/**Function********************************************************************

  Synopsis    [returns the trace node that precedes "self" node. ]

  Description [returns the value of the prev pointer.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceNode_ptr trace_node_get_prev(const TraceNode_ptr self)
{
  TRACE_NODE_CHECK_INSTANCE(self);
  
  return self->prev;
}

/**Function********************************************************************

  Synopsis    [Checks whether it is the last node.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean trace_node_is_last(const TraceNode_ptr self)
{
  TRACE_NODE_CHECK_INSTANCE(self);

  return (self->next == TRACE_NODE(NULL));
}

/**Function********************************************************************

  Synopsis    [Checks whether it is the first node.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean trace_node_is_first(const TraceNode_ptr self)
{
  TRACE_NODE_CHECK_INSTANCE(self);

  return (self->prev == TRACE_NODE(NULL));
}
