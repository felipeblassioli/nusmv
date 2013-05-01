/**CFile***********************************************************************

  FileName    [Trace.c]

  PackageName [trace]

  Synopsis    [Routines related to Trace.]

  Description [ This file contains the definition of \"Trace\" class.]
		
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

#include "pkg_traceInt.h"
#include "Trace.h"
#include "Trace_private.h"
#include "TraceNode.h"
#include "TraceNode_private.h"
#include "node/node.h"
#include "parser/symbols.h"
#include "enc/bdd/BddEnc.h" /* For BDD Encoder */
#include "prop/prop.h" /* For Prop_master_get_bdd_fsm */

static char rcsid[] UTIL_UNUSED = "$Id: Trace.c,v 1.1.2.29.2.2 2004/11/11 12:51:45 nusmv Exp $";
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define TRACE_CHECK_OWNERSHIP(x) \
	  (nusmv_assert(x->ID <= 0 ))

#define TRACE_UNREGISTERED -1

#define TRACE_TYPE_CNTEXAMPLE_STRING "Counterexample"
#define TRACE_TYPE_SIMULATION_STRING "Simulation"
#define TRACE_TYPE_INVALID_STRING "Invalid"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Trace Class]

  Description [ This class contains informations about a Trace:<br>
  	<dl> 
        <dt><code>ID</code>
            <dd>  Unique ID of the registered traces. -1 for unregistered
            traces.
        <dt><code>desc</code>
            <dd>  Description of the trace. 
        <dt><code>dd</code>
            <dd>  The Decision Diagram Manager.
        <dt><code>enc</code>
            <dd>  The BDD Encoder Object.
        <dt><code>length</code>
            <dd>  Diameter of the trace.
        <dt><code>type</code>
            <dd>  Type of the trace.
        <dt><code>first_node</code>
            <dd>  Pointer to the first node of the doubly linked list of
            TraceNodes.
        <dt><code>last_node</code>
            <dd> Pointer to the last node of the doubly linked list of
            TraceNodes. 
	</dl>
	<br>
	]

  SeeAlso     []   
  
******************************************************************************/
typedef struct Trace_TAG
{
  int ID;
  char* desc;               
  DdManager* dd;		       
  BddEnc_ptr enc;         
  int length;			        
  TraceType type;		      
  TraceNode_ptr first_node;	
  TraceNode_ptr last_node;
} Trace;

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

  Synopsis    [\"Trace\" class Constructor.]

  Description [Allocates and initializes a trace. As arguments this function
  takes pointer to the DdManager \"dd\", description of the trace \"desc\",
  type of the Trace \"type\" and initial state of the trace \"start_state\". It
  returns a pointer to the allocated Trace object.
  /"start state/" bdd is referenced by this function.]

  SideEffects []

  SeeAlso     [Trace_destroy]

******************************************************************************/
Trace_ptr Trace_create(BddEnc_ptr enc, const char* desc, const TraceType type,
                       BddStates start_state) 
{
  Trace_ptr self = ALLOC(Trace, 1);
  TraceNode_ptr tmp;
  TRACE_CHECK_INSTANCE(self);
  nusmv_assert(type != TRACE_TYPE_INVALID);

  self->ID = TRACE_UNREGISTERED;		

  self->desc = ALLOC(char, strlen(desc) + 1);
  nusmv_assert(self->desc != (char*) NULL);
  strncpy(self->desc, desc, strlen(desc) + 1);

  self->enc = enc;
  self->dd = BddEnc_get_dd_manager(enc);
  self->type = type;
  self->length = 0; /* Length of a trace is number of inputs it has seen */

  /* A Trace contains at least one Trace Node for the initial state. */
  tmp = TraceNode_create();
  TraceNode_set_state(tmp, self->dd, start_state);
  self->first_node = tmp;
  self->last_node = tmp;

  return self;
}

/**Function********************************************************************

  Synopsis    [The \"Trace\" class destructor.]

  Description [This function destroys the trace and all the trace node
  instances inside it. In order to destroy a trace directly, it should not be
  registered with TraceManager because in that case it's ownership is with
  TraceManager and only that can destroy it. ]

  SideEffects []

  SeeAlso     [Trace_create]

******************************************************************************/
void Trace_destroy(Trace_ptr self)
{
  TraceIterator_ptr iter;

  TRACE_CHECK_INSTANCE(self);
  TRACE_CHECK_OWNERSHIP(self);	/* Can not be destroyed if TraceManager holds
                                   the ownership */ 
                                   
  iter = Trace_begin(self);

  while (!TraceIterator_is_end(iter)) {
    TraceNode_ptr trace_node = iter;

    iter = TraceIterator_get_next(iter);
    TraceNode_destroy(trace_node, self->dd);
  }

  FREE(self->desc);
  FREE(self);
}

/**Function********************************************************************

  Synopsis    [Creates the Trace from a list of states.]

  Description [ Trace constructor. It takes a list of states and construct a
  trace out of it (all the inputs are bdd_one)]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Trace_ptr Trace_create_from_state_list(BddEnc_ptr enc, const char* desc, 
				       const TraceType type, 
				       node_ptr path)
{
  Trace_ptr res;
  DdManager* dd = BddEnc_get_dd_manager(enc);
  BddInputs all_inputs = BDD_INPUTS(bdd_one(dd));

  nusmv_assert(path != Nil); 
  
  res = Trace_create(enc, desc, type, BDD_STATES(car(path)));

  path = cdr(path);
  while (path != Nil) {
    Trace_append(res, all_inputs, BDD_STATES(car(path)));
    path = cdr(path);
  } 

  bdd_free(dd, all_inputs);
  return res;
}


/**Function********************************************************************

  Synopsis    [Creates the Trace from a list of states.]

  Description [ Trace constructor. It takes a list of states and construct a
  trace out of it. path is a sequence of "states (inputs states)*"]

  SideEffects []

  SeeAlso     []

******************************************************************************/

Trace_ptr Trace_create_from_state_input_list(BddEnc_ptr enc, const char* desc, 
					     const TraceType type, 
					     node_ptr path)
{
  Trace_ptr res;
  DdManager* dd = BddEnc_get_dd_manager(enc);

  if (path == Nil) {
    /* Trivially false */
    bdd_ptr one = bdd_one(dd);
    res = Trace_create(enc, desc, type, BDD_STATES(one));
    bdd_free(dd, one);
  }
  else {
    res = Trace_create(enc, desc, type, BDD_STATES(car(path)));

    path = cdr(path);
    while (path != Nil) {
      bdd_ptr input;
      bdd_ptr state;

      input = BDD_INPUTS(car(path));
      path = cdr(path);
      state = BDD_STATES(car(path)); 

      Trace_append(res, input, state);
   
      path = cdr(path);
    } 
  }

  return res;
}


/**Function********************************************************************

  Synopsis    [Appends a new input and a new state to the trace.]

  Description [This function takes next input \"inp_i\" and the next state
  \"state_i\" and appends it to the "self". The BDDs state_i and inp_i would be
  referenced.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_append(Trace_ptr self, BddInputs inp_i, BddStates state_i)
{
  TraceNode_ptr tmp;

  TRACE_CHECK_INSTANCE(self);

  tmp = TraceNode_create();

  TraceNode_set_state(tmp, self->dd, state_i);
  trace_node_set_prev(tmp, self->last_node);

  TraceNode_set_input(self->last_node, self->dd, inp_i);
  trace_node_set_next(self->last_node, tmp);

  self->last_node = tmp;
  self->length = self->length + 1;
}

/**Function********************************************************************

  Synopsis    [Appends  to a trace from a list of states.]

  Description [All the BDDs inside the list /"path/" are referenced. So that
  caller of this function can safely destroy the list. The first element of 
  the list is supposed to be the initial state, and will be skipped since 
  it is supposed to be already set in the Trace]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_append_from_list_state(Trace_ptr self, node_ptr path)
{
  BddInputs all_inputs = BDD_INPUTS(bdd_one(self->dd));

  nusmv_assert(path != Nil);

  path = cdr(path);
  while (path != Nil) {
    Trace_append(self, all_inputs, BDD_STATES(car(path)));
    path = cdr(path);
  } 

  bdd_free(self->dd, all_inputs);
}


/**Function********************************************************************

  Synopsis    [Appends  to a trace from a list of states.]

  Description [All the BDDs inside the list /"path/" are referenced. So that
  caller of this function can safely destroy the list. The first element of 
  the list is supposed to be the initial state, and will be skipped since 
  it is supposed to be already set in the Trace]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_append_from_list_input_state(Trace_ptr self, node_ptr path)
{
  nusmv_assert(path != Nil);

  path = cdr(path);
  while (path != Nil) {
    Trace_append( self, BDD_INPUTS(car(path)), BDD_STATES(car(cdr(path))) );
    path = cdr(cdr(path));
  } 
}


/**Function********************************************************************

  Synopsis    [Checks the validity of the given trace.]

  Description [ This function checks the validity of a trace by checking if the
  starting state of the trace is initial state and consecutive states are
  related by transition relation.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Trace_is_valid(Trace_ptr self)
{
  BddStates initial_states;
  BddStates start_state;
  boolean res = true;
  BddFsm_ptr fsm = Prop_master_get_bdd_fsm();

  /* 0- Check If NULL */
  if (self == TRACE(NULL)) return false;

  start_state = Trace_get_ith_state(self, 0);
  /* 1- Check Start State */
  {
    bdd_ptr init_bdd = BddFsm_get_init(fsm);
    bdd_ptr invar_bdd = BddFsm_get_state_constraints(fsm);
    initial_states = bdd_and(self->dd, init_bdd, invar_bdd);
    bdd_free(self->dd, init_bdd);
    bdd_free(self->dd, invar_bdd);

    res = bdd_entailed(self->dd, start_state, initial_states);
    if (!res) {
      fprintf(nusmv_stderr, 
              "Warning!! starting state is not initial state \n");
    }

    bdd_free(self->dd, initial_states);
    bdd_free(self->dd, start_state);
  }

  /* 2- Check Consecutive States are related by transition relation */
  if (res) {
    TraceIterator_ptr iter = Trace_begin(self);
    BddStates curr_state, next_state, next_states;
    BddInputs input; 

    curr_state = TraceNode_get_state(Trace_get_node(self, iter));
    input = TraceNode_get_input(Trace_get_node(self, iter));

    iter = TraceIterator_get_next(iter); 
    while (iter != TRACE_ITERATOR(NULL)) {
      nusmv_assert(input != BDD_INPUTS(NULL));

      next_state = TraceNode_get_state(Trace_get_node(self, iter)); 
      next_states = BddFsm_get_constrained_forward_image(fsm, curr_state, 
							 input);

      res = bdd_entailed(self->dd, next_state, next_states);
      
      bdd_free(self->dd, curr_state);
      curr_state = bdd_dup(next_state);
      bdd_free(self->dd, next_state);
      bdd_free(self->dd, next_states);
      bdd_free(self->dd, input);

     
      if (!res) {
	fprintf(nusmv_stderr, "Warning!! consecutive states are improper \n");
	break;
      }

      input = TraceNode_get_input(Trace_get_node(self, iter));
      iter = TraceIterator_get_next(iter); 
    } /* loop on state/input airs */
  }

  return res;
}

/**Function********************************************************************

  Synopsis    [Copies "self" to "other".]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Trace_ptr Trace_copy_prefix_until_iterator(const Trace_ptr self, 
                                           const TraceIterator_ptr until_here)
{
  Trace_ptr result;
  BddStates start_state;
  TraceIterator_ptr iter;

  TRACE_CHECK_INSTANCE(self);

  start_state = Trace_get_ith_state(self, 0);
  result = Trace_create(self->enc, self->desc,
                        self->type, start_state);
  if (start_state != BDD_STATES(NULL)) bdd_free(self->dd, start_state);

  iter = Trace_begin(self);
  while (iter != until_here && iter != TRACE_ITERATOR(NULL)) {
    BddStates curr_state;
    BddInputs curr_input;

    curr_input = TraceNode_get_input(iter);
    iter = TraceIterator_get_next(iter);

    if (iter == TRACE_ITERATOR(NULL)) {
      nusmv_assert(until_here == TRACE_ITERATOR(NULL));
      if (curr_input != BDD_INPUTS(NULL)) bdd_free(self->dd, curr_input);
      break;
    }

    curr_state = TraceNode_get_state(iter);
    Trace_append(result, curr_input, curr_state);

    if (curr_state != BDD_STATES(NULL)) bdd_free(self->dd, curr_state);
    if (curr_input != BDD_INPUTS(NULL)) bdd_free(self->dd, curr_input);
  }
  
  if (iter != until_here) nusmv_assert(iter != TRACE_ITERATOR(NULL));

  return result;
}

/**Function********************************************************************

  Synopsis    [Returns a copy of the trace.]

  Description [Copy constructor.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Trace_ptr Trace_copy(const Trace_ptr self)
{
  return Trace_copy_prefix_until_iterator(self, TRACE_END_ITERATOR);
}

/**Function********************************************************************

  Synopsis    [This function returns the first node of the \"self\" having same
  state as the last node.]

  Description [It returns a pointer to the first trace node having same state
  as the last one. It returns NULL otherwise. The ownership of the returned
  traceNode object is not transferred to the caller. Caller must not free the
  returned object.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceNode_ptr Trace_get_loopback(const Trace_ptr self)
{
  BddStates last_state;
  TraceNode_ptr result = TRACE_NODE(NULL);
  TraceIterator_ptr iter;


  TRACE_CHECK_INSTANCE(self);

  last_state = TraceNode_get_state(self->last_node);

  iter = Trace_begin(self);
  while (!TraceIterator_is_end(iter)) {
    BddStates curr_state = TraceNode_get_state(iter);

    if ((curr_state == last_state) && 
        (Trace_get_node(self, iter) != self->last_node)) {
      result = Trace_get_node(self, iter);

      if (curr_state != BDD_STATES(NULL)) bdd_free(self->dd, curr_state);
      break;
    }

    if (curr_state != BDD_STATES(NULL)) bdd_free(self->dd, curr_state);
    iter = TraceIterator_get_next(iter);
  }

  if (last_state != BDD_STATES(NULL)) bdd_free(self->dd, last_state);

  return result;
}

/**Function********************************************************************

  Synopsis    [It returns all loopbacks from last state.]

  Description [This function returns an array of TraceNodes containing pointers
  to the TraceNodes which have same state as the last one.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
array_t* Trace_get_all_loopbacks(const Trace_ptr self)
{
  BddStates last_state;
  array_t* result = (array_t *) NULL;
  TraceIterator_ptr iter;

  TRACE_CHECK_INSTANCE(self);

  result = array_alloc(TraceNode_ptr, 1);
  nusmv_assert(result != (array_t *) ARRAY_OUT_OF_MEM);

  last_state = TraceNode_get_state(self->last_node);

  iter = Trace_begin(self);
  while (!TraceIterator_is_end(iter)) {
    BddStates curr_state = TraceNode_get_state(iter);

    if ((curr_state == last_state) && 
        (Trace_get_node(self, iter) != self->last_node)) {
      array_insert_last(TraceNode_ptr, result, 
                        Trace_get_node(self, iter));
    }
    if (curr_state != BDD_STATES(NULL)) bdd_free(self->dd, curr_state);

    iter = TraceIterator_get_next(iter);
  }

  if (last_state != BDD_STATES(NULL)) bdd_free(self->dd, last_state);

  return result;
}

/**Function********************************************************************

  Synopsis    [Returns the state corresponding to the ith traceNode.]

  Description [returned BDD is referenced. Please note that initial state is 0th
  state.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
BddStates Trace_get_ith_state(const Trace_ptr self, int i)
{
  TraceIterator_ptr iter;
  int cnt;

  TRACE_CHECK_INSTANCE(self);

  nusmv_assert(i >= 0);
  nusmv_assert(i <= self->length);

  iter = Trace_begin(self);
  for (cnt = 0; cnt < i; ++cnt) {
    iter = TraceIterator_get_next(iter); 
  }

  nusmv_assert(!TraceIterator_is_end(iter)); 

  return TraceNode_get_state(iter);
}

/**Function********************************************************************

  Synopsis    [Returns the ith traceNode.]

  Description [The ownership of the returned node is not transfered to the
  caller. Caller must not free the returned TraceNode object.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceNode_ptr Trace_get_ith_node(const Trace_ptr self, int i)
{
  TraceIterator_ptr iter;
  int cnt;

  TRACE_CHECK_INSTANCE(self);

  nusmv_assert(i >= 0);
  nusmv_assert(i <= self->length);

  iter = Trace_begin(self);
  for (cnt = 0; cnt < i; ++cnt) {
    iter = TraceIterator_get_next(iter); 
  }

  nusmv_assert(!TraceIterator_is_end(iter)); 

  return Trace_get_node(self, iter);
}

/**Function********************************************************************

  Synopsis    [Checks equality between \"self\" and \"other\".]

  Description [Returns true if they are equal, false otherwise.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Trace_is_equal(const Trace_ptr self, const Trace_ptr other)
{
  boolean result = true;
  TraceIterator_ptr self_iter = Trace_begin(self);
  TraceIterator_ptr other_iter = Trace_begin(other);

  if (self->length != other->length) result = false;
  if (self->dd != other->dd) result = false;

  while (!TraceIterator_is_end(self_iter) && result) {
    TraceNode_ptr self_node = Trace_get_node(self, self_iter);
    TraceNode_ptr other_node = Trace_get_node(other, other_iter);
    BddStates self_state, other_state;
    BddInputs self_input, other_input;

    self_state = TraceNode_get_state(self_node);
    other_state = TraceNode_get_state(other_node);

    if (self_state != other_state) result = false; 

    if (self_state != BDD_STATES(NULL)) bdd_free(self->dd, self_state);
    if (other_state != BDD_STATES(NULL)) bdd_free(other->dd, other_state);

    self_input = TraceNode_get_input(self_node);
    other_input = TraceNode_get_input(other_node);

    if (self_input != other_input) result = false; 

    if (self_input != BDD_INPUTS(NULL)) bdd_free(self->dd, self_input);
    if (other_input != BDD_INPUTS(NULL)) bdd_free(other->dd, other_input);

    self_iter = TraceIterator_get_next(self_iter);
    other_iter = TraceIterator_get_next(other_iter);
  }

  return result;
}

/**Function********************************************************************

  Synopsis    [Returns an iterator to iterate the \"self\".]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceIterator_ptr Trace_begin(const Trace_ptr self)
{
  TraceIterator_ptr iter;
  TRACE_CHECK_INSTANCE(self);

  iter = TRACE_ITERATOR(self->first_node);
  return iter;
}

/**Function********************************************************************

  Synopsis    [Returns an iterator pointing to the given trace-node of  the
  \"self\".]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceIterator_ptr Trace_get_iterator(const Trace_ptr self, 
                                     const TraceNode_ptr node)
{
  TraceIterator_ptr iter;
  TRACE_CHECK_INSTANCE(self);

  iter = TRACE_ITERATOR(node);
  return iter;
}

/**Function********************************************************************

  Synopsis    [ It returns the Trace node currently pointed by TraceIterator.]

  Description [The ownership of the returned node is not transfered to the
  caller. Caller must not free the returned TraceNode object.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceNode_ptr Trace_get_node(const Trace_ptr self, 
                             const TraceIterator_ptr iter)
{
  TRACE_ITERATOR_CHECK_INSTANCE(iter);

  return TRACE_NODE(iter);
}

/**Function********************************************************************

  Synopsis    [Returns the length of the trace.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Trace_get_length(const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return self->length;
}

/**Function********************************************************************

  Synopsis    [Returns the ID of the trace.]

  Description [This ID correspond to the index of the trace inside the trace
  manager. If the trace is not registeres with the trace manager, this ID would
  be TRACE_UNREGISTERED.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Trace_get_id(const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return self->ID;
}

/**Function********************************************************************

  Synopsis    [Returns the description associated with the trace.]

  Description [Returned string must not be freed.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
const char* Trace_get_desc(const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return self->desc;
}

/**Function********************************************************************

  Synopsis    [Returns the type of the trace.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceType Trace_get_type(const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return self->type;
}

/**Function********************************************************************

  Synopsis    [Returns the last node of the trace.]

  Description [The ownership of the returned object is not transfered to the
  caller of this function. It means that caller of this function must not free
  the returned traceNode. ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceNode_ptr Trace_get_last_node(const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return self->last_node;
}

/**Function********************************************************************

  Synopsis    [Returns the bdd encoder associated with the trace.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
BddEnc_ptr Trace_get_enc(Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return self->enc;
}

/**Function********************************************************************

  Synopsis    [Sets the start state of the Trace.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_set_start_state(Trace_ptr self, BddStates s)
{
  TRACE_CHECK_INSTANCE(self);

  TraceNode_set_state(self->first_node, self->dd, s);
}

/**Function********************************************************************

  Synopsis    [ Sets the description of the trace.]

  Description [It allocates the memory for the description string. So the
  ownership of the passed desc string remains with the caller.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_set_desc(Trace_ptr self, const char* desc)
{
  TRACE_CHECK_INSTANCE(self);

  FREE(self->desc);

  self->desc = ALLOC(char, strlen(desc) + 1);

  nusmv_assert(self->desc != (char*) NULL);
  strncpy(self->desc, desc, strlen(desc) + 1);
}

/**Function********************************************************************

  Synopsis    [Set the type of the trace.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_set_type(Trace_ptr self, const TraceType type)
{
  TRACE_CHECK_INSTANCE(self);

  self->type = type;
}

/**Function********************************************************************

  Synopsis    [Checks whether trace is registered with the trace manager.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Trace_is_registered(const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return (self->ID != TRACE_UNREGISTERED);
}

/**Function********************************************************************

  Synopsis    [Returns a string corresponding to a TraceType.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
const char* TraceType_to_string(const TraceType self)
{
  char* result;

  switch (self){
  case TRACE_TYPE_CNTEXAMPLE: result = TRACE_TYPE_CNTEXAMPLE_STRING; break;
  case TRACE_TYPE_SIMULATION: result = TRACE_TYPE_SIMULATION_STRING; break;

  default: result = TRACE_TYPE_INVALID_STRING; /*Invalid Type */ 
  }

  return result; 
}

/**Function********************************************************************

  Synopsis    [Returns a TraceType object from the given string.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceType TraceType_from_string(const char* str)
{
  TransType result;

  if (strncmp(str, TRACE_TYPE_CNTEXAMPLE_STRING, strlen(str)) == 0) {
    result = TRACE_TYPE_CNTEXAMPLE;
  }
  if (strncmp(str, TRACE_TYPE_SIMULATION_STRING, strlen(str)) == 0) {
    result = TRACE_TYPE_SIMULATION;
  }
  else result = TRACE_TYPE_INVALID;

  return result;
}

/**Function********************************************************************

  Synopsis    [This function takes an iterator to a trace and advances it by
  one.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceIterator_ptr TraceIterator_get_next(const TraceIterator_ptr self)
{
  TraceIterator_ptr res;

  TRACE_ITERATOR_CHECK_INSTANCE(self);
  
  res = TRACE_ITERATOR(trace_node_get_next(TRACE_NODE(self)));

  return res;
}

/**Function********************************************************************

  Synopsis    [This function checks if the iterator is at the end of the
  trace.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean TraceIterator_is_end(const TraceIterator_ptr self)
{
  return (self == TRACE_END_ITERATOR);
}

/*---------------------------------------------------------------------------*/
/* Static functions definitions                                              */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Assign an identifier to the trace.]

  Description [ TraceManager uses this function to assign a unique identifier ID
  to each registered trace. ONLY TraceManager and Trace  should use this
  function.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_set_ID(Trace_ptr self, int ID)
{
  TRACE_CHECK_INSTANCE(self);

  self->ID = ID;
}

/**Function********************************************************************

  Synopsis    [Unregisters the given trace.]

  Description [Only TraceManager should access this function. This function
  resets the Trace ID to TRACE_UNREGISTERED (-1).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_unregister(Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);
  Trace_set_ID(self, TRACE_UNREGISTERED);
  return;
}

