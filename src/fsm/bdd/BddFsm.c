/**CFile***********************************************************************

  FileName    [BddFsm.c]

  PackageName [fsm.bdd]

  Synopsis    [Defines the public interface for the class BddFsm]

  Description [A BddFsm is a Finite State Machine (FSM) whose building blocks 
               (the set of initial state, the transition relation, the set of
               constraints on inputs and so on) are represented by means of
               BDD data structures, and whose capabilities are based on 
               operations upon and between BDDs as well.]
  
  Author      [Roberto Cavada, Marco Benedetti]

  Copyright   [
  This file is part of the ``fsm.bdd'' package of NuSMV version 2. 
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

#include "bddInt.h"
#include "BddFsm.h"
#include "FairnessList.h"
#include "enc/enc.h"
#include "utils/utils_io.h"
#include "utils/error.h"
#include <math.h>

static char rcsid[] UTIL_UNUSED = "$Id: BddFsm.c,v 1.1.2.44.2.3 2005/08/22 08:29:06 nusmv Exp $";


typedef struct BddFsm_TAG 
{
  /* private members */
  DdManager*  dd;
  Encoding_ptr senc;
  BddEnc_ptr  enc;

  BddStates      init;

  BddInvarStates invar_states;
  BddInvarInputs invar_inputs;

  BddTrans_ptr trans; 

  JusticeList_ptr    justice;
  CompassionList_ptr compassion;  
  
  BddFsmCache_ptr cache;
} BddFsm; 


/* ---------------------------------------------------------------------- */
/*                     Static functions prototypes                        */
/* ---------------------------------------------------------------------- */

static void bdd_fsm_init ARGS((BddFsm_ptr self, BddEnc_ptr encoding, 
			       BddStates init, BddInvarStates invar_states, 
			       BddInvarInputs invar_inputs, 
			       BddTrans_ptr trans, 
			       JusticeList_ptr justice, 
			       CompassionList_ptr compassion));

static void bdd_fsm_copy ARGS((const BddFsm_ptr self, BddFsm_ptr copy));

static void bdd_fsm_deinit ARGS((BddFsm_ptr self));

static void bdd_fsm_compute_reachable_states ARGS((BddFsm_ptr self));

static BddStatesInputs 
bdd_fsm_get_legal_state_input ARGS((BddFsm_ptr self));

/* The new code for fairness */
static BddStatesInputs
bdd_fsm_get_fair_states_inputs_in_subspace ARGS((const BddFsm_ptr self, 
						 BddStatesInputs subspace));

static BddStatesInputs 
bdd_fsm_fair_SI_subset_aux ARGS((const BddFsm_ptr self,
				 BddStatesInputs states, 
				 BddStatesInputs subspace));

static BddStatesInputs
bdd_fsm_get_fair_SI_subset ARGS((const BddFsm_ptr self, 
				 BddStatesInputs subspace));


static void bdd_fsm_check_init_state_invar_emptyness ARGS((const BddFsm_ptr self));
static void bdd_fsm_check_fairness_emptyness ARGS((const BddFsm_ptr self));


/* ---------------------------------------------------------------------- */
/*                          public methods                                */
/* ---------------------------------------------------------------------- */


/**Function********************************************************************

  Synopsis           [Constructor for BddFsm]

  Description        [All given bdd are referenced. 
  self becomes the owner of given trans, justice and compassion objects, 
  whereas the encoding is owned by the caller]

  SideEffects        []
  
  SeeAlso            []

******************************************************************************/
BddFsm_ptr BddFsm_create(BddEnc_ptr encoding, 
			 BddStates init, 
			 BddInvarStates invar_states, 
			 BddInvarInputs invar_inputs, 
			 BddTrans_ptr trans, 
			 JusticeList_ptr justice, 
			 CompassionList_ptr compassion)
{
  BddFsm_ptr self = ALLOC( BddFsm, 1 ); 
  BDD_FSM_CHECK_INSTANCE(self);

  bdd_fsm_init(self, encoding, init, invar_states, invar_inputs, 
	       trans, justice, compassion);
  
  return self;
}



/**Function********************************************************************

  Synopsis           [Destructor of class BddFsm]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddFsm_destroy(BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);
  
  bdd_fsm_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Copy constructor for BddFsm]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddFsm_ptr BddFsm_copy(const BddFsm_ptr self)
{
  BddFsm_ptr copy;
  
  BDD_FSM_CHECK_INSTANCE(self);

  copy = ALLOC( BddFsm, 1 ); 
  BDD_FSM_CHECK_INSTANCE(copy);
  
  bdd_fsm_copy(self, copy);
  
  return copy;  
}


/**Function********************************************************************

  Synopsis           [Getter for justice list]

  Description        [self keeps the ownership of the returned object]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
JusticeList_ptr BddFsm_get_justice(const BddFsm_ptr self) 
{
  BDD_FSM_CHECK_INSTANCE(self);
  return self->justice;
}


/**Function********************************************************************

  Synopsis           [Getter for compassion list]

  Description        [self keeps the ownership of the returned object]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
CompassionList_ptr BddFsm_get_compassion(const BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);
  return self->compassion;
}

/**Function********************************************************************

  Synopsis           [Getter for init]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddStates BddFsm_get_init(const BddFsm_ptr self)
{
  bdd_ptr res; 
  
  BDD_FSM_CHECK_INSTANCE(self);

  res = bdd_dup((bdd_ptr) self->init);
  return BDD_STATES(res);
}


/**Function********************************************************************

  Synopsis           [Getter for state constraints]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddInvarStates BddFsm_get_state_constraints(const BddFsm_ptr self)
{
  bdd_ptr res; 
  
  BDD_FSM_CHECK_INSTANCE(self);
  
  res = bdd_dup( (bdd_ptr) self->invar_states );
  return BDD_INVAR_STATES(res);
}


/**Function********************************************************************

  Synopsis           [Getter for input constraints]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddInvarInputs BddFsm_get_input_constraints(const BddFsm_ptr self)
{
  bdd_ptr res; 
  
  BDD_FSM_CHECK_INSTANCE(self);
  
  res = bdd_dup( (bdd_ptr) self->invar_inputs );
  return BDD_INVAR_INPUTS(res);
}


/**Function********************************************************************

  Synopsis           [Getter for the trans]

  Description        [Returned Trans instance is not copied, do not destroy
  it, since self keeps the ownership.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddTrans_ptr BddFsm_get_trans(const BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);
  
  return self->trans;
}


/**Function********************************************************************

  Synopsis     [Gets the set of reachable states of this machine]

  Description  [Returned bdd is referenced. 
               
                This method returns the set R of reachable states,
                i.e.  those states that can be actually reached
                starting from one of the initial state.
                
                R is the set of states such that "i TRC s" holds for
                some state i in the set of initial states, where TRC
                is the transitive closure of the conjunction of the
                transition relation of the machine with the set of
                invar states, the set of constraints on inputs and the
                set of state/input constraints.
                                                                                               
                R is computed by this method in a forward manner by
                exploiting the "BddFsm_get_forward_image" method
                during a fixpoint calculation. In particular, R is
                computed by reaching the fixpoint on the functional
                that maps S onto the forward image
                BddFsm_get_forward_image(S) of S, where the
                computation is started from the set of initial states.
                Notice that the set of invar states, the set of
                constraints on inputs and the set of state/input
                constrains are implicitly taken into account by
                BddFsm_get_forward_image(S).]

  SideEffects  [Internal cache could change]

  SeeAlso      []

******************************************************************************/
BddStates BddFsm_get_reachable_states(BddFsm_ptr self)
{ 
  BddStates res; 
  
  BDD_FSM_CHECK_INSTANCE(self);
 
  if ( CACHE_IS_EQUAL(reachable.computed, false) ) {
    bdd_fsm_compute_reachable_states(self); 
  }     
  
  res = CACHE_GET(reachable.states);
  bdd_ref((bdd_ptr) res);

  return res;
}


/**Function********************************************************************

  Synopsis     [Returns the set of reachable states at a given distance]

  Description  [Computes the set of reachable states if not previously, 
                cached. Returned bdd is referenced. 
		
		If distance is greater than the diameter, an assertion
		is fired.

                This method returns the set R of states of this
                machine which can be reached in exactly "distance"
                steps by applying the "BddFsm_get_forward_image"
                method ("distance" times) starting from one of
                the initial states (and cannot be reached with less
                than "distance" steps).

                In the case that the distance is less than 0, the
                empty-set is returned.

                These states are computed as intermediate steps of the
                fixpoint characterization given in the
                "BddFsm_get_reachable_states" method.]

  SideEffects  [Internal cache could change]

******************************************************************************/
BddStates BddFsm_get_reachable_states_at_distance(BddFsm_ptr self, 
						  int distance)
{
  BddStates res;

  BDD_FSM_CHECK_INSTANCE(self);  
  
  res = BDD_STATES(NULL);

  if (distance >= 0) {
    int diameter;

    if (CACHE_IS_EQUAL(reachable.computed, false)) {
      bdd_fsm_compute_reachable_states(self);
    }
 
    diameter = CACHE_GET(reachable.diameter);

    /* checks distance */
    nusmv_assert(distance <= diameter); 

    res = CACHE_GET_BDD(reachable.layers[distance]);
  }

  /* checks if assigned: */ 
  if (res == BDD_STATES(NULL))  res = BDD_STATES( bdd_zero(self->dd) );
  return res;
}

/**Function********************************************************************

  Synopsis     [Returns a bdd that represents the monolithic 
  transition relation]

  Description  [This method returns a monolithic representation of
                the transition relation, which is computed on the
                basis of the internal partitioned representation by
                composing all the element of the partition.

		Returned bdd is referenced.]

  SideEffects  [Internal cache could change]

  SeeAlso      []

******************************************************************************/
bdd_ptr BddFsm_get_monolithic_trans_bdd(BddFsm_ptr self)
{
  bdd_ptr res; 
  ClusterList_ptr clusters;

  BDD_FSM_CHECK_INSTANCE(self);    
  
  if ( CACHE_IS_EQUAL(monolithic_trans, (bdd_ptr) NULL) ) {
    clusters = BddTrans_get_forward(self->trans);
    res = ClusterList_get_monolithic_bdd(clusters);

    CACHE_SET_BDD(monolithic_trans, res);
    bdd_free(self->dd, res);
  }

  res = CACHE_GET_BDD(monolithic_trans);
  return res;
}


/**Function********************************************************************

  Synopsis     [Returns the distance of a given set of states from initial
                states]

  Description  [Computes the set of reachable states if not previously cached. 
                Returns -1 if given states set is not reachable.

                This method returns an integer which represents the
                distance of the farthest state in "states". The
                distance of one single state "s" is the number of
                applications of the "BddFsm_get_forward_image"
                method (starting from the initial set of states)
                which is necessary and sufficient to end up with a set
                of states containing "s". The distance of a *set* of
                states "set" is the maximal distance of states in
                "set", i.e. the number of applications of the
                "BddFsm_get_forward_image" method (starting from the
                initial set of states) which is necessary and
                sufficient to reach at least once (not necessarily
                during the last application, but somewhere along the
                way) each state in "set".
                
                So, the distance of a set of states is a max-min
		function.
		Could update the cache.]

  SideEffects  [Internal cache could change]

******************************************************************************/
int BddFsm_get_distance_of_states(BddFsm_ptr self, BddStates states)
{
  bdd_ptr constr_states; 
  int i;
  int diameter;
  int result = -1;

  BDD_FSM_CHECK_INSTANCE(self);  

  if (CACHE_IS_EQUAL(reachable.computed, false)) {
    bdd_fsm_compute_reachable_states(self);
  }

  /* applies state constraints: */
  constr_states = bdd_and(self->dd, 
			  (bdd_ptr) states, 
			  (bdd_ptr) self->invar_states);
  
  diameter = CACHE_GET(reachable.diameter);

  for (i=0; i<diameter; ++i) {
    bdd_ptr Ri = (bdd_ptr) BddFsm_get_reachable_states_at_distance(self, i);
    int entailed = bdd_entailed(self->dd, constr_states, Ri);

    if (entailed == 1) {
      bdd_free(self->dd, Ri);      
      result = i;
      break;
    } 
    
    bdd_free(self->dd, Ri);      
  }

  bdd_free(self->dd, constr_states);
  return result; 
}


/**Function********************************************************************

  Synopsis     [Returns the diameter of the machine from the inital state]

  Description  [This method returns an integer which represents the
                diameter of the machine with respect to the set of
                initial states, i.e.  the distance of the fatherst
                state in the machine (starting from the initial
                states), i.e. the maximal value among the lengths of
                shortest paths to each reachable state.  The initial
                diameter is computed as the number of iteration the
                fixpoint procedure described above (see
                "BddFsm_get_reachable_states") does before reaching
                the fixpoint.  It can also be seen as the maximal
                value the "BddFsm_get_distance_of_states" can return
                (which is returned when the argument "states" is set
                to "all the states").

		Could update the cache.]
  
  SideEffects  [Internal cache could change]

******************************************************************************/
int BddFsm_get_diameter(BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);  
  
  if (CACHE_IS_EQUAL(reachable.computed, false)) {
    bdd_fsm_compute_reachable_states(self);
  }

  return CACHE_GET(reachable.diameter);
}


/**Function********************************************************************

  Synopsis     [Returns the set of states without subsequents]

  Description  [This method returns the set of states with no
                successor.  A state "ds" has no successor when all the
                following conditions hold:
                
                1) ds is a state satisfying stateConstr.
                2) no transition from ds exists which is consistent
                   with input and state/input constraint and leads to
                   a state satisfying stateConstr. 

		Could update the cache.]
  
  SideEffects  [Internal cache could change]

******************************************************************************/
BddStates BddFsm_get_not_successor_states(BddFsm_ptr self)
{
  BddStates res; 

  BDD_FSM_CHECK_INSTANCE(self);  
  
  if ( CACHE_IS_EQUAL(not_successor_states, BDD_STATES(NULL)) ) {
    bdd_ptr all_states = bdd_one(self->dd);
    bdd_ptr succ       = BddFsm_get_backward_image(self, all_states);
    bdd_ptr not_succ   = bdd_not(self->dd, succ);
    bdd_ptr no_succ_constr = bdd_and(self->dd, not_succ, self->invar_states);

    bdd_free(self->dd, not_succ);
    bdd_free(self->dd, succ);
    bdd_free(self->dd, all_states);
  
    CACHE_SET_BDD(not_successor_states, no_succ_constr);
    bdd_free(self->dd, no_succ_constr);
  }     
  
  res = CACHE_GET_BDD(not_successor_states);
  return res;
}


/**Function********************************************************************

  Synopsis     [Returns the set of deadlock states]

  Description  [This method returns the set of deadlock states.  A
                state ds is said to be a deadlock state when all the
                following conditions hold:
          
                1) ds is a state satisfying stateConstr;
		2) no transition from ds exists which is consistent
                   with input and state/input constraint and leads to
                   a state satisfying stateConstr; 
		3) s is rechable.

		Could update the cache. May trigger the computation of
		reachable states and states without successors.
		Returned bdd is referenced.]
  
  SideEffects  [Cache can change]

******************************************************************************/
BddStates BddFsm_get_deadlock_states(BddFsm_ptr self)
{
  BddStates res; 
  
  BDD_FSM_CHECK_INSTANCE(self);  
  
  if ( CACHE_IS_EQUAL(deadlock_states, BDD_STATES(NULL)) ) {
    BddStates no_succ = BddFsm_get_not_successor_states(self);
    BddStates reachable = BddFsm_get_reachable_states(self);

    bdd_ptr deadlock = bdd_and(self->dd, reachable, no_succ);
    bdd_free(self->dd, reachable);
    bdd_free(self->dd, no_succ);    
    
    CACHE_SET_BDD(deadlock_states, BDD_STATES(deadlock));
    bdd_free(self->dd, deadlock);
  }
 
  res = CACHE_GET_BDD(deadlock_states);  
  return res;  
}


/**Function********************************************************************

  Synopsis     [Returns true if this machine is total]

  Description  [This method checks wether this machine is total, in
                the sense that each INVAR state has at least one INVAR
                successor state given the constraints on the inputs
                and the state/input.
                                                                
                This is done by checking that the BddFsm_ImageBwd
                image of the set of all the states is the set of all
                the INVAR states.  This way, the INVAR constraints
                together with the set of constraints on both input and
                state/input are implicitly taken into account by
                BddFsm_get_forward_image.
                
                The answer "false" is produced when states exist that
                admit no INVAR successor, given the sets of input and
                state/input constraints. However, all these "dead"
                states may be non-reachable, so the machine can still
                be "deadlock free".  See the "BddFsm_is_deadlock_free"
                method.

		Could update the cache. May trigger the computation of
		states without successors.]
  
  SideEffects  [Cache can change]

******************************************************************************/
boolean BddFsm_is_total(BddFsm_ptr self) 
{
  BddStates no_succ;
  boolean res;

  BDD_FSM_CHECK_INSTANCE(self);  
  
  no_succ = BddFsm_get_not_successor_states(self);

  res = bdd_is_zero(self->dd, (bdd_ptr) no_succ);                  
  bdd_free(self->dd, no_succ);

  return res;
}


/**Function********************************************************************

  Synopsis     [Returns true if this machine is deadlock free]

  Description  [This method checks wether this machine is deadlock
                free, i.e.  wether it is impossible to reach an INVAR
                state with no admittable INVAR successor moving from
                the initial condition.
  
                This happens when the machine is total. If it is not,
		each INVAR state from which no transition to another
		INVAR state can be made according to the input and
		state/input constraints is non-reachable.
                                                
                This method checks deadlock freeness by checking
                that the intersection between the set of reachable
                states and the set of INVAR states with no admittable
                INVAR successor is empty.

		Could update the cache. May trigger the computation of
		deadlock states.]
  
  SideEffects  [Cache can change]

******************************************************************************/
boolean BddFsm_is_deadlock_free(BddFsm_ptr self) 
{
  BddStates deadlock;
  boolean res;

  BDD_FSM_CHECK_INSTANCE(self);  
  
  deadlock = BddFsm_get_deadlock_states(self);

  res = bdd_is_zero(self->dd, (bdd_ptr) deadlock);                  
  bdd_free(self->dd, deadlock);

  return res;
}


/**Function********************************************************************

  Synopsis     [Returns the forward image of a set of states]

  Description  [This method computes the forward image of a set of
                states S, i.e. the set of INVAR states which are
                reachable from one of the INVAR states in S by means
                of one single machine transition among those
                consistent with both the input constraints and the
                state/input constraints.
                
                The forward image of S(X) is computed as follows.
 
                a. S1(X,I)   := S(X) and Invar(X) and InputConst(I)
                b. S2(X')    := { x' | <x,i,x'> in Tr(X,I,X') for 
                                  some <x,i> in S1(X,I) }
                c. S3(X)     := S2(X')[x/x']
                d. FwdImg(X) := S3(X) and Invar(X)

		Returned bdd is referenced.]
  
  SideEffects  []

******************************************************************************/
BddStates BddFsm_get_forward_image(const BddFsm_ptr self, BddStates states)
{
  BddStatesInputs one = bdd_one(self->dd);
  BddStates res; 

  BDD_FSM_CHECK_INSTANCE(self);
  
  res = BddFsm_get_constrained_forward_image(self, states, one);
  bdd_free(self->dd, one);

  return res;
}


/**Function********************************************************************

  Synopsis     [Returns the constrained forward image of a set of states]

  Description  [This method computes the forward image of a set of
                states S, given a set C(X,I) of contraints on STATE
                and INPUT vars which are meant to represent a
                restriction on allowed transitions and inputs.
                
                The constrained image is the set of INVAR states which
                are reachable from one of the INVAR states in S by
                means of one single machine transition among those
                consistent with both the constraints defined within
                the machine and the additional constraint C(X,I).
                
                The forward image of S(X,IC) is computed as follows.
 
                a. S1(X,I)   := S(X) and Invar(X) and InputConst(I) and C(X,I)
                b. S2(X')    := { x' | <x,i,x'> in Tr(X,I,X') for 
                                  some <x,i> in S1(X,I) }
                c. S3(X)     := S2(X')[x/x']
                d. FwdImg(X,IC) := S3(X) and Invar(X)

		To apply no contraints, parameter constraints must be the 
		true bdd. Returned bdd is referenced]
  
  SideEffects  []

******************************************************************************/
BddStates BddFsm_get_constrained_forward_image(const BddFsm_ptr self, 
					       BddStates states, 
					       BddStatesInputs constraints)
{
  BddStates res;
  bdd_ptr constr_trans;
  bdd_ptr tmp;

  BDD_FSM_CHECK_INSTANCE(self);

  /* ------------------------------------------------------------ */
  /* Apply invariant contraints: */
  constr_trans = bdd_and(self->dd, states, self->invar_states);
  bdd_and_accumulate(self->dd, &constr_trans, self->invar_inputs);
  bdd_and_accumulate(self->dd, &constr_trans, constraints);
  /* ------------------------------------------------------------ */

  tmp = BddTrans_get_forward_image_state(self->trans, constr_trans);  
  bdd_free(self->dd, constr_trans);
  
  res = BDD_STATES( BddEnc_next_state_var_to_state_var(self->enc, tmp) );
  bdd_free(self->dd, tmp);
  
  bdd_and_accumulate(self->dd, (bdd_ptr*) &res, self->invar_states);
  return res;  
}


/**Function********************************************************************

  Synopsis     [Returns the backward image of a set of states]

  Description  [This method computes the backward image of a set S of
                states, i.e. the set of INVAR states from which some
                of the INVAR states in S is reachable by means of one
                single machine transition among those consistent with
                both the input constraints and the state/input
                constraints.
                               
                The backward image of S(X) is computed as follows.
  
                a. S1(X)     := S(X) and Invar(X)
                b. S2(X')    := S1(X)[x'/x]
                c. S3(X,I)   := Invar(X) and InputConst(I)
                c. BwdImg(X) := { x | <x,i,x'> in Tr(X,I,X') for 
                                  some <x,i> in S3(X,I) and some x' in S2(X') } 
  
	       Returned bdd is referenced.]
  
  SideEffects  []

******************************************************************************/
BddStates BddFsm_get_backward_image(const BddFsm_ptr self, BddStates states)
{
  BddStates res;
  BddStatesInputs one;

  BDD_FSM_CHECK_INSTANCE(self);
  
  one = bdd_one(self->dd);

  res = BddFsm_get_constrained_backward_image(self, states, one);
  bdd_free(self->dd, one);

  return res;  
}


/**Function********************************************************************

  Synopsis     [Returns the constrained backward image of a set of states]

  Description  [This method computes the backward image of a set of
                states S, given a set C(X,I) of contraints on STATE
                and INPUT vars which are meant to represent a
                restriction on allowed transitions and inputs.

                The constrained image is the set of INVAR states from
                which some of the INVAR states in S is reachable by
                means of one single machine transition among those
                consistent with both the machine constraints and the
                given additional constraint C(X,I).
                                
                The backward image of S(X,IC) is computed as follows.
 
                a. S1(X)     := S(X) and Invar(X)
                b. S2(X')    := S1(X)[x'/x]
                c. S3(X,I)   := Invar(X) and InputConst(I) and IC(I) and C(X,I)
                c. BwdImg(X) := { x | <x,i,x'> in Tr(X,I,X') for 
                                  some <x,i> in S3(X,I) and some x' in S2(X') }
  
	        To apply no contraints, parameter constraints must be
                the true bdd. Returned bdd is referenced.]
  
  SideEffects  []

******************************************************************************/
BddStates BddFsm_get_constrained_backward_image(const BddFsm_ptr self, 
						BddStates states, 
						BddStatesInputs constraints)
{
  bdd_ptr constr_trans;
  bdd_ptr tmp, result;
  
  BDD_FSM_CHECK_INSTANCE(self);
    
  tmp = bdd_and(self->dd, states, self->invar_states);
  constr_trans = BddEnc_state_var_to_next_state_var(self->enc, tmp);  
  bdd_free(self->dd, tmp);  
  
  bdd_and_accumulate(self->dd, &constr_trans, self->invar_inputs);
  bdd_and_accumulate(self->dd, &constr_trans, constraints);

  result = BddTrans_get_backward_image_state(self->trans, constr_trans);
  
  bdd_and_accumulate(self->dd, &result, self->invar_states);

  bdd_free(self->dd, constr_trans);
         
  return BDD_STATES(result);
}

/**Function********************************************************************

  Synopsis     [Returns the k-backward image of a set of states]

  Description  [This method computes the set of <state,input> couples
                that lead into at least k distinct states of the set 
                of states given as input. The returned couples 
                and the states in the set given in input are restricted
                                                 
                The k-backward image of S(X) is computed as follows.
  
                a. S1(X)     := S(X) and Invar(X)
                b. S2(X')    := S1(X)[X'/X]
                c. S3(X,I,k) := {<x,i> | exists x'[1..k] : S2(x'[m]) and
                                  x'[m] != x'[n] if m != n and
                                  <x,i,x'[m]> in Tr }
                d. KBwdImg(X,I,k) := S3(X,I,k) and Invar(X) and 
                                     InputConst(I)
               
 	        The returned bdd is referenced.]
  
  SideEffects  []

******************************************************************************/
BddStatesInputs BddFsm_get_k_backward_image(const BddFsm_ptr self, 
				            BddStates states,
					    int k)
{
  bdd_ptr tmp, tmp1, result; 

  BDD_FSM_CHECK_INSTANCE(self);
    
  tmp = bdd_and(self->dd, states, self->invar_states);

  /* We need to apply the mask, otherwise the count is not correct! */
  tmp1 = BddEnc_apply_state_vars_mask_bdd(self->enc, tmp);
  bdd_free(self->dd, tmp);

  tmp = BddEnc_state_var_to_next_state_var(self->enc, tmp1);
  bdd_free(self->dd, tmp1);
    
  result = BddTrans_get_k_backward_image_state_input(self->trans, tmp, k);
  bdd_free(self->dd, tmp);

  bdd_and_accumulate(self->dd, &result, self->invar_inputs);
  bdd_and_accumulate(self->dd, &result, self->invar_states);
  
  return BDD_STATES(result);
}

/**Function********************************************************************

  Synopsis     [Returns the weak backward image of a set of states]

  Description  [This method computes the set of <state,input> couples
                that leads into the set of states given as input.
                i.e. the set of <s,i> couples such that <s,i> is
                consistent with both the input constraints and the
                state/input constraints, s is INVAR, and a transition
                from s to s' labelled by i exists for some INVAR s' in
                S.
                                                 
                The weak backward image of S(X) is computed as follows.
  
                a. S1(X)     := S(X) and Invar(X)
                b. S2(X')    := S1(X)[x'/x]
                c. S3(X,I)   := Invar(X) and InputConst(I)
                c. WeakBwdImg(X,I) := {<x,i> | <x,i,x'> in Tr(X,I,X') for some 
                                       <x,i> in S3(X,I) and some x' in S2(X') } 
   
               
	       Returned bdd is referenced.]
  
  SideEffects  []

******************************************************************************/
BddStatesInputs BddFsm_get_weak_backward_image(const BddFsm_ptr self, 
					       BddStates states)
{
  bdd_ptr constr_trans;
  bdd_ptr tmp, result; 

  BDD_FSM_CHECK_INSTANCE(self);
    
  tmp = bdd_and(self->dd, states, self->invar_states);
  constr_trans = BddEnc_state_var_to_next_state_var(self->enc, tmp);
  bdd_free(self->dd, tmp);
    
  bdd_and_accumulate(self->dd, &constr_trans, self->invar_inputs);
      
  result = BddTrans_get_backward_image_state_input(self->trans, constr_trans);
  bdd_and_accumulate(self->dd, &result, self->invar_states);
  
  bdd_free(self->dd, constr_trans);
     
  return BDD_STATES(result);
}


/**Function********************************************************************

  Synopsis     [Returns the strong backward image of a set of states]

  Description  [This method computes the set of <state,input>
                transitions that have at least one successor and are
                such that all the successors lay inside the INVAR
                subset of the set of states given as input.
                                                   
                The strong backward image of S(X, I) is computed as
                follows.
  
                a. S1(X,I) := WeakBwdImg(not S(X))
                b. S2(X,I) := (not S1(X,I)) and StateConstr(X) and 
                              InputConst(I)
                c. Tr(X,I) := {<x,i> | <x,i,x'> in Tr(X,I,X') for some x'}
                d. StrongBwdImg(X,I) := S2(X,I) and Tr(X,I)
   
		Returned bdd is referenced.]
  
  SideEffects  []

******************************************************************************/
BddStatesInputs BddFsm_get_strong_backward_image(const BddFsm_ptr self, 
						 BddStates states)
{
  bdd_ptr not_states;
  bdd_ptr tmp, result; 

  BDD_FSM_CHECK_INSTANCE(self);

  not_states = bdd_not(self->dd, states);  
  /* there is no need to add state or input invariants because
     there are added in BddFsm_get_weak_backward_image
  */
  tmp = BddFsm_get_weak_backward_image(self, not_states); 
  bdd_free(self->dd, not_states);

  /* Here tmp is the set of state/input transitions that can be
     actually made and that lead outside of the set defined by the
     input parameter 'states'
  */  

  result = bdd_not(self->dd, tmp);
  bdd_free(self->dd, tmp);

  /* result is the set of state/input transitions that either
     originate from 'non-real states' or - in case there is at least
     one successor - are such that all the successors lay inside the
     set defined by the input parameter 'states' or or one of state,
     input or all successors belong to sets not satisfying invariants.
  */     

  /* Obtain all the legal transitions, i.e. such state/input
     which have have at least one successor  and all state/input/successor
     satisty the invariants.
  */
  tmp = (bdd_ptr) bdd_fsm_get_legal_state_input(self);
       
  bdd_and_accumulate(self->dd, &result, tmp);
  bdd_free(self->dd, tmp);

  /* At this point, result is the set of state/input transitions that
     have at least one successor and are such that all 
     the successors lay inside the set defined by the 
     input parameter 'states' */

  return BDD_STATES_INPUTS(result);
}

/**Function********************************************************************

  Synopsis           [Prints some information about this BddFsm.]

  Description        [Prints some information about this BddFsm.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void BddFsm_print_info(const BddFsm_ptr self, FILE* file)
{
  BddStates init_bdd = BddFsm_get_init(self);
  BddInvarStates state_bdd = BddFsm_get_state_constraints(self);
  BddInvarInputs input_bdd = BddFsm_get_input_constraints(self);

  if ((init_bdd != NULL)) {
    fprintf(file, "BDD nodes representing init set of states: %d\n",
            bdd_size(self->dd, init_bdd));
    bdd_free(self->dd, init_bdd);
  }            
  
  if ((state_bdd != NULL)) {
    fprintf(file, "BDD nodes representing state constraints: %d\n",
            bdd_size(self->dd, state_bdd));
    bdd_free(self->dd, state_bdd);
  }            

  if ((input_bdd != NULL)) {
    fprintf(file, "BDD nodes representing input constraints: %d\n",
            bdd_size(self->dd, input_bdd));
    bdd_free(self->dd, input_bdd);
  } 
           
  BddTrans_print_short_info(BddFsm_get_trans(self), file);
}


/**Function********************************************************************

  Synopsis           [Prints statistical information about reachable states.]

  Description        [Prints statistical information about reachable
  states, i.e. the real number of reachable states. It is computed
  taking care of the encoding and of the indifferent variables in the
  encoding.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddFsm_print_reachable_states_info(const BddFsm_ptr self, 
					const boolean print_states, 
					FILE* file)
{
  bdd_ptr reachable;
  bdd_ptr mask; 
  double reached_cardinality;
  double search_space_cardinality;

  mask = BddEnc_get_state_vars_mask_bdd(self->enc);  
  reachable = BddFsm_get_reachable_states(self);
  bdd_and_accumulate(self->dd, &reachable, mask);

  reached_cardinality = BddEnc_count_states_of_bdd(self->enc, reachable);
  search_space_cardinality = BddEnc_count_states_of_bdd(self->enc, mask);
  bdd_free(self->dd, mask);

  fprintf(file, "system diameter: %d\n", BddFsm_get_diameter(self));

  fprintf(file, "reachable states: %g (2^%g) out of %g (2^%g)\n",
	  reached_cardinality, log(reached_cardinality)/log(2.0),
	  search_space_cardinality, log(search_space_cardinality)/log(2.0));

  if (print_states) {
    BddEnc_print_set_of_states(self->enc, reachable, false, file); 
  }

  bdd_free(self->dd, reachable);
}

/**Function********************************************************************

  Synopsis     [Returns the set of fair state-input pairs of the machine.]

  Description  [ ] 
                                

  SideEffects  [Internal cache could change]

  SeeAlso      []

******************************************************************************/
BddStatesInputs BddFsm_get_fair_states_inputs(BddFsm_ptr self)
{
  BddStatesInputs res; 

  BDD_FSM_CHECK_INSTANCE(self);

  if ( CACHE_IS_EQUAL(fair_states_inputs, BDD_STATES(NULL)) ) {
    BddStatesInputs si;
    BddStates fair_si;

    si = BddFsm_get_states_inputs_constraints(self);

    if (opt_use_reachable_states(options)) {
      bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(self);
      
      bdd_and_accumulate(self->dd, &si, reachable_states_bdd);
      bdd_free(self->dd, reachable_states_bdd);
    }

    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stderr, "Computing the set of fair <state>x<input> pairs\n");
    }

    fair_si = bdd_fsm_get_fair_states_inputs_in_subspace(self, si);

    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stderr,"done\n");
    }
  
    CACHE_SET_BDD(fair_states_inputs, fair_si);

    bdd_free(self->dd, fair_si);
    bdd_free(self->dd, si);

  }

  res = CACHE_GET_BDD(fair_states_inputs);

  return res;
}


/**Function********************************************************************

  Synopsis     [Returns the set of fair states of a fsm.]

  Description  [ ]
                                

  SideEffects        [Internal cache could change]

  SeeAlso            []

******************************************************************************/
BddStates BddFsm_get_fair_states(BddFsm_ptr self)
{
  BddStates res; 

  BDD_FSM_CHECK_INSTANCE(self);

  if ( CACHE_IS_EQUAL(fair_states, BDD_STATES(NULL)) ) {
    BddStatesInputs si = BddFsm_get_fair_states_inputs(self);
    BddStates fs = BddFsm_states_inputs_to_states(self, si);

    CACHE_SET_BDD(fair_states, fs);

    bdd_free(self->dd, fs);
  }

  res = CACHE_GET_BDD(fair_states);
  return res;
}

/**Function********************************************************************

  Synopsis [Given two sets of states, returns the set of inputs
  labeling any transition from a state in the first set to a state in
  the second set.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/

BddInputs BddFsm_states_to_states_get_inputs(const BddFsm_ptr self,
					     BddStates cur_states,
					     BddStates next_states)
{
  BddStates bwd_image_si;
  BddInputs inputs;
  
  bwd_image_si = BddFsm_get_weak_backward_image(self, next_states);
  bdd_and_accumulate(self->dd, &bwd_image_si, cur_states);

  inputs = BddFsm_states_inputs_to_inputs(self, bwd_image_si);

  bdd_free(self->dd, bwd_image_si);
  return inputs;
}

/**Function********************************************************************

  Synopsis           [Checks if a set of states is fair.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/

boolean BddFsm_is_fair_states(const BddFsm_ptr self, BddStates states)
{
  BddStates fair_states;
  boolean res;

  BDD_FSM_CHECK_INSTANCE(self);

  fair_states = BddFsm_get_fair_states(self);

  res = (bdd_entailed(self->dd, states, fair_states) == 1);

  bdd_free(self->dd, fair_states);
  return res;
}


/**Function********************************************************************

  Synopsis           [Getter for compassion list]

  Description        [self keeps the ownership of the returned object]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddStatesInputs BddFsm_get_states_inputs_constraints(const BddFsm_ptr self)
{
  BddStatesInputs result;
  BDD_FSM_CHECK_INSTANCE(self);
  /* it would be a better idea to cache it */
  result = BddTrans_get_backward_image_state_input(self->trans, bdd_one(self->dd));
  return result;
}


/**Function********************************************************************

  Synopsis     [Computes the states occurring in a set of states-inputs pairs.]

  Description  [Quantifies away the input variables.]  

  SeeAlso      []
  
  SideEffects  []

******************************************************************************/
BddStates BddFsm_states_inputs_to_states(const BddFsm_ptr self,
					 BddStatesInputs si)
{
  BddStates states;
  bdd_ptr input_vars_cube = BddEnc_get_input_vars_support(self->enc);

  states = bdd_forsome(self->dd, si, input_vars_cube);

  bdd_free(self->dd, input_vars_cube);

  return states;
}


/**Function********************************************************************

  Synopsis     [Computes the inputs occurring in a set of states-inputs pairs.]

  Description  [Quantifies away the input variables.]  

  SeeAlso      []
  
  SideEffects  []

******************************************************************************/
BddStates BddFsm_states_inputs_to_inputs(const BddFsm_ptr self,
					 BddStatesInputs si)
{
  BddStates inputs;
  bdd_ptr state_vars_cube = BddEnc_get_state_vars_support(self->enc);

  inputs = bdd_forsome(self->dd, si, state_vars_cube);

  bdd_free(self->dd, state_vars_cube);

  return inputs;
}


/**Function********************************************************************

  Synopsis           [Prints statistical information about fair states.]

  Description [Prints the number of fair states, taking care of the
		     encoding and of the indifferent variables in the
		     encoding. In verbose mode also prints states.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddFsm_print_fair_states_info(const BddFsm_ptr self, 
				   const boolean print_states, FILE* file)
{
  bdd_ptr fair_states;
  bdd_ptr mask; 
  double reached_cardinality;
  double search_space_cardinality;

  fair_states = BddFsm_get_fair_states(self);
  mask = BddEnc_get_state_vars_mask_bdd(self->enc);  
  bdd_and_accumulate(self->dd, &fair_states, mask);
 
  reached_cardinality = BddEnc_count_states_of_bdd(self->enc, fair_states);
  search_space_cardinality = BddEnc_count_states_of_bdd(self->enc, mask);
  bdd_free(self->dd, mask);

  fprintf(file, "system diameter: %d\n", BddFsm_get_diameter(self));

  fprintf(file, "fair states: %g (2^%g) out of %g (2^%g)\n",
	  reached_cardinality, log(reached_cardinality)/log(2.0),
	  search_space_cardinality, log(search_space_cardinality)/log(2.0));

  if (print_states) {
    BddEnc_print_set_of_states(self->enc, fair_states, false, file); 
  }
  bdd_free(self->dd, fair_states);
}


/**Function********************************************************************

  Synopsis           [Prints statistical information about fair states and transitions.]

  Description        [Prints the number of fair states, taking care of
		     the encoding and of the indifferent variables in the encoding.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddFsm_print_fair_transitions_info(const BddFsm_ptr self, 
					const boolean print_transitions, 
					FILE* file)
{
  bdd_ptr fair_trans;
  bdd_ptr mask;
  double fairstates_cardinality;
  double search_space_cardinality;
  Encoding_ptr symenc = BddEnc_get_symbolic_encoding(self->enc);

  fair_trans = BddFsm_get_fair_states_inputs(self);
  mask = BddEnc_get_state_input_vars_mask_bdd(self->enc);
  bdd_and_accumulate(self->dd, &fair_trans, mask);
 
  fairstates_cardinality = BddEnc_get_minterms_of_bdd(self->enc, fair_trans);
  search_space_cardinality = BddEnc_get_minterms_of_bdd(self->enc, mask);
  bdd_free(self->dd, mask);

  fprintf(file, "fair states: %g (2^%g) out of %g (2^%g)\n",
	  fairstates_cardinality, log(fairstates_cardinality)/log(2.0),
	  search_space_cardinality, log(search_space_cardinality)/log(2.0));

  if (print_transitions) {
    BddEnc_print_set_of_state_input_pairs(self->enc, fair_trans, false, file); 
  }

  bdd_free(self->dd, fair_trans);
}


/**Function********************************************************************

  Synopsis    [Check that the transition relation is total]

  Description [Check that the transition relation is total. If not the
  case than a deadlock state is printed out.  May trigger the
  computation of reachable states and states without successors.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void BddFsm_check_machine(const BddFsm_ptr self)
{  
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Checking totality and deadlock states.\n");
  }
  
  bdd_fsm_check_init_state_invar_emptyness(self);
  bdd_fsm_check_fairness_emptyness(self);

  if (! BddFsm_is_total(self)) {
    bdd_ptr noSuccStates = BddFsm_get_not_successor_states(self);
    bdd_ptr ds = BddEnc_pick_one_state(self->enc, noSuccStates);
    bdd_free(self->dd, noSuccStates);
    
    fprintf(nusmv_stdout, "\n##########################################################\n");
    fprintf(nusmv_stdout, "The transition relation is not total. A state without\n");
    fprintf(nusmv_stdout, "successors is:\n");

    BddEnc_print_bdd_begin(self->enc, 
       NodeList_to_node_ptr(Encoding_get_state_vars_list(self->senc)), 
       false);

    BddEnc_print_bdd(self->enc, ds, nusmv_stdout);

    if (CACHE_IS_EQUAL(reachable.computed, true) || 
	opt_use_reachable_states(options)) {
      /* here the reachable states calculation has been done or requested. */
      if (! BddFsm_is_deadlock_free(self)) {
	
	bdd_ptr deadlockStates = BddFsm_get_deadlock_states(self);
	bdd_ptr ds = BddEnc_pick_one_state(self->enc, deadlockStates);
	bdd_free(self->dd, deadlockStates);

	fprintf(nusmv_stdout, "The transition relation is not deadlock-free.\n");
	fprintf(nusmv_stdout, "A deadlock state is:\n");        
	BddEnc_print_bdd(self->enc, ds, nusmv_stdout);
      } 
      else {
	fprintf(nusmv_stdout, "However, all the states without successors are\n");        
	fprintf(nusmv_stdout, "non-reachable, so the machine is deadlock-free.\n");                
      } 
    } 
    else {
      /* reachables states should be calculated */
      fprintf(nusmv_stdout, "NOTE: No-successor states could be non-reachable, so\n");
      fprintf(nusmv_stdout, "      the machine could still be deadlock-free.\n");
      fprintf(nusmv_stdout, "      Reachable states have to be computed to check this.\n");
    }
    
    BddEnc_print_bdd_end(self->enc);

    fprintf(nusmv_stdout, "##########################################################\n");
    bdd_free(self->dd, ds);
  } 
  else {
    fprintf(nusmv_stdout, "\n##########################################################\n");
    fprintf(nusmv_stdout, "The transition relation is total: No deadlock state exists\n");
    fprintf(nusmv_stdout, "##########################################################\n");
  } 
}


/**Function********************************************************************

  Synopsis    [Performs the synchronous product of two fsm]

  Description [The result goes into self, no changes on other. 
  Both the two FSMs must be based on the same dd manager. 
  The cache will change, since a new separated family will be created for 
  the internal cache, and it will not be shared anymore with previous family. 
  From the old cache will be reused as much as possible]

  SideEffects [self will change]

  SeeAlso     [BddFsmCache_reset_not_reusable_fields_after_product]

******************************************************************************/
void BddFsm_apply_synchronous_product(BddFsm_ptr self, const BddFsm_ptr other)
{
  BddFsmCache_ptr new_cache;

  BDD_FSM_CHECK_INSTANCE(self);

  /* check for the same dd manager */
  nusmv_assert(self->dd == other->dd);

  /* check for the same dd manager, in the future we will probably
     relax this constraint  */
  nusmv_assert(self->enc == other->enc);
  
  /* init */
  bdd_and_accumulate(self->dd, &(self->init), other->init);
  
  /* invars */
  bdd_and_accumulate(self->dd, &(self->invar_states), other->invar_states);

  /* trans */
  {
    bdd_ptr state_vars_cube = BddEnc_get_state_vars_support(self->enc);
    bdd_ptr input_vars_cube = BddEnc_get_input_vars_support(self->enc);
    bdd_ptr next_vars_cube = BddEnc_get_next_state_vars_support(self->enc);

    BddTrans_apply_synchronous_product(self->trans, other->trans, 
				       state_vars_cube, 
				       input_vars_cube, 
				       next_vars_cube);
    
    bdd_free(self->dd, (bdd_ptr) state_vars_cube);
    bdd_free(self->dd, (bdd_ptr) input_vars_cube);
    bdd_free(self->dd, (bdd_ptr) next_vars_cube);
  }  

  /* fairness constraints */
  JusticeList_apply_synchronous_product(self->justice, other->justice);
  CompassionList_apply_synchronous_product(self->compassion, other->compassion);

  /* cache substitution */
  new_cache = BddFsmCache_hard_copy(self->cache);
  BddFsmCache_reset_not_reusable_fields_after_product(new_cache);
  BddFsmCache_destroy(self->cache);
  self->cache = new_cache;
}


/* ---------------------------------------------------------------------- */
/*                         Static functions                               */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis    [Private initializer]

  Description []

  SideEffects []

******************************************************************************/
static void bdd_fsm_init(BddFsm_ptr self, 
			 BddEnc_ptr encoding, 
			 BddStates init, 
			 BddInvarStates invar_states, 
			 BddInvarInputs invar_inputs, 
			 BddTrans_ptr trans, 
			 JusticeList_ptr justice, 
			 CompassionList_ptr compassion)
{
  self->enc = encoding;
  self->dd = BddEnc_get_dd_manager(encoding);
  self->senc = BddEnc_get_symbolic_encoding(encoding);

  nusmv_assert(init != NULL);

  self->init = BDD_STATES( bdd_dup((bdd_ptr) init) );
  self->invar_states = BDD_INVAR_STATES( bdd_dup((bdd_ptr) invar_states) );
  self->invar_inputs = BDD_INVAR_INPUTS( bdd_dup((bdd_ptr) invar_inputs) );
  self->trans = trans;
  self->justice = justice;
  self->compassion = compassion;

  self->cache = BddFsmCache_create(self->dd);

  /* check inits and invars for emptiness */
  bdd_fsm_check_init_state_invar_emptyness(self);
}


/**Function********************************************************************

  Synopsis    [private copy constructor]

  Description []

  SideEffects []

******************************************************************************/
static void bdd_fsm_copy(const BddFsm_ptr self, BddFsm_ptr copy)
{
  copy->dd = self->dd;
  copy->enc = self->enc;

  copy->init = BDD_STATES( bdd_dup((bdd_ptr) self->init ) );
  copy->invar_states = 
    BDD_INVAR_STATES( bdd_dup((bdd_ptr) self->invar_states ) );
  copy->invar_inputs  = 
    BDD_INVAR_INPUTS( bdd_dup((bdd_ptr) self->invar_inputs ) );

  copy->trans = BDD_TRANS( Object_copy(OBJECT(self->trans)) );
  copy->justice = JUSTICE_LIST( Object_copy(OBJECT(self->justice)));
  copy->compassion = COMPASSION_LIST( Object_copy(OBJECT(self->compassion)));

  copy->cache = BddFsmCache_soft_copy(self->cache);
}


/**Function********************************************************************

  Synopsis    [private member called by the destructor]

  Description []

  SideEffects []

******************************************************************************/
static void bdd_fsm_deinit(BddFsm_ptr self)
{
  bdd_free(self->dd, (bdd_ptr) self->init);
  bdd_free(self->dd, (bdd_ptr) self->invar_states);
  bdd_free(self->dd, (bdd_ptr) self->invar_inputs);

  Object_destroy(OBJECT(self->trans), NULL);
  Object_destroy(OBJECT(self->justice), NULL);
  Object_destroy(OBJECT(self->compassion), NULL);

  BddFsmCache_destroy(self->cache);  
}


/**Function********************************************************************

  Synopsis     [Calculates the set of reachable states of this machine]

  Description  []

  SideEffects  [Changes the internal cache]

  SeeAlso      []

******************************************************************************/
static void bdd_fsm_compute_reachable_states(BddFsm_ptr self)
{
  bdd_ptr reachable_states_bdd;
  bdd_ptr from_lower_bound;   /* the frontier */
  bdd_ptr invars;
  node_ptr reachable_states_layers;
  int diameter = 0;
  
  reachable_states_bdd = BddFsm_get_init(self); 
  invars = BddFsm_get_state_constraints(self);
  bdd_and_accumulate(self->dd, &reachable_states_bdd, invars);
  bdd_free(self->dd, invars);

  from_lower_bound = BddFsm_get_init(self); 
  reachable_states_layers = Nil; 

  if (opt_verbose_level_gt(options, 1)) {
    fprintf(nusmv_stderr, "\ncomputing reachable state space\n");
  }
  
  while (bdd_isnot_zero(self->dd, from_lower_bound)) {
    
    bdd_ptr from_upper_bound = bdd_dup(reachable_states_bdd);
    reachable_states_layers = 
      cons((node_ptr) bdd_dup(reachable_states_bdd), reachable_states_layers);
    
    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stderr, 
	      "  iteration %d: BDD size = %d, frontier size = %d, states = %g\n",
	      diameter, bdd_size(self->dd, reachable_states_bdd),
	      bdd_size(self->dd, from_lower_bound),
	      BddEnc_count_states_of_bdd(self->enc, reachable_states_bdd));
    }
    
    {
      BddStates img = 
	BddFsm_get_forward_image(self, BDD_STATES(from_lower_bound));
      
      bdd_or_accumulate(self->dd, &reachable_states_bdd, img);
      bdd_free(self->dd, (bdd_ptr) img);
    }

    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stderr, "  forward step done, size = %d\n",
	      bdd_size(self->dd, reachable_states_bdd)); 
    }
    
    /* updates from_lower_bound */
    {
      bdd_ptr not_from_upper_bound = bdd_not(self->dd, from_upper_bound);
       
      bdd_free(self->dd, from_lower_bound);
      from_lower_bound = bdd_and(self->dd, 
				 reachable_states_bdd, 
				 not_from_upper_bound);
        
      bdd_free(self->dd, not_from_upper_bound);
    }

    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stderr, "  new frontier computed, size = %d\n",
	      bdd_size(self->dd, from_lower_bound));
    }
    
    bdd_free(self->dd, from_upper_bound);
    ++diameter;

  } /* while loop */
  
  bdd_free(self->dd, from_lower_bound);  
   
  /* 
     BddFsmCache_set_reachables is responsible of the free of the list
     reachable_states_layers
  */
  BddFsmCache_set_reachables(self->cache, 
			     BDD_STATES(reachable_states_bdd), 
			     reachable_states_layers, 
			     diameter);

  bdd_free(self->dd, reachable_states_bdd);

  if (opt_verbose_level_gt(options, 1)) {
    fprintf(nusmv_stderr, "done\n");
  }

}


/**Function********************************************************************

  Synopsis     [Returns the set of states and inputs,
  for which a legal transition can be made.]

  Description  [A legal transition is a transition which satisfy the
  transition relation, and the state, input and next-state satisfy the
  invariants.  So the image S(X, I) is computed as follows:
     S(X,I) = StateConstr(X) & InputConstr(i) & StateConstr(X') &
              Tr(X,I,X') for some X'

  Used for planning in strong backward image computation. 
  Could update the cache. 
  Returned bdd is referenced.
  ]
  
  SideEffects  [Cache can change]

******************************************************************************/
static BddStatesInputs bdd_fsm_get_legal_state_input(BddFsm_ptr self)
{
  BddStatesInputs res;
  
  if ( CACHE_IS_EQUAL(legal_state_input, BDD_STATES_INPUTS(NULL)) ) {
    bdd_ptr one = bdd_one(self->dd);
    /* Here we use weak-backward-image because
       it automatically applies invariant contrains on state, input, next-state.
    */
    bdd_ptr tmp = BddFsm_get_weak_backward_image(self, one);

    CACHE_SET_BDD(legal_state_input, tmp);
    bdd_free(self->dd, one);
    bdd_free(self->dd, tmp);
  } 

  res = CACHE_GET_BDD(legal_state_input);
  return res;
}

/**Function********************************************************************

  Synopsis     [Computes the preimage of a set of states-inputs pairs.]

  Description  [Quantifies away the inputs, and computes the (states-inputs)
                preimage of the resulting set of states.]  

  SeeAlso      []
  
  SideEffects  []

******************************************************************************/
static BddStatesInputs bdd_fsm_EX_SI(const BddFsm_ptr self, BddStatesInputs si)
{
  BddStates states;  
  BddStatesInputs si_preimage;

  states = BddFsm_states_inputs_to_states(self, si);
  si_preimage = BddFsm_get_weak_backward_image(self, states);

  if (opt_use_reachable_states(options)) {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(self);

    bdd_and_accumulate(self->dd, &si_preimage, reachable_states_bdd);
    bdd_free(self->dd, reachable_states_bdd);
  }

  bdd_free(self->dd, states);

  return si_preimage;
}

/**Function********************************************************************

  Synopsis     [Computes the set of state-input pairs that satisfy E(f U g), with f and g
                sets of state-input pairs.]

  Description  [] 

  SeeAlso      []
  
  SideEffects  []

******************************************************************************/
static BddStatesInputs bdd_fsm_EU_SI(const BddFsm_ptr self, 
				     BddStatesInputs f, BddStatesInputs g)
{
  int i;
  BddStatesInputs oldY;
  BddStatesInputs resY;
  BddStatesInputs newY;
  BddStatesInputs rg = bdd_dup(g);

  if (opt_use_reachable_states(options)) {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(self);

    bdd_and_accumulate(self->dd, &rg, reachable_states_bdd);
    bdd_free(self->dd, reachable_states_bdd);
  }

  oldY = bdd_dup(rg);
  resY = bdd_dup(rg);
  newY = bdd_dup(rg);
  bdd_free(self->dd, rg);
  i = 0;

  while (bdd_isnot_zero(self->dd, newY)) {
    BddStatesInputs tmp_1, tmp_2;

    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stderr,
              "    size of Y%d = %g <states>x<inputs>, %d BDD nodes\n",
              i, BddEnc_count_states_inputs_of_bdd(self->enc, resY),
              bdd_size(self->dd, resY));
    }
    
    bdd_free(self->dd, oldY);

    oldY = bdd_dup(resY);
    tmp_1 = bdd_fsm_EX_SI(self, newY);
    tmp_2 = bdd_and(self->dd, f, tmp_1);
    bdd_free(self->dd, tmp_1);

    bdd_or_accumulate(self->dd, &resY, tmp_2);
    bdd_free(self->dd, tmp_2);

    tmp_1 = bdd_not(self->dd, oldY);
    bdd_free(self->dd, newY);

    newY = bdd_and(self->dd, resY, tmp_1);
    bdd_free(self->dd, tmp_1);

    i++;
  }

  bdd_free(self->dd, newY);
  bdd_free(self->dd, oldY);

  /* We do not free resY since it is rersposibility of the caller to
     free it. Functions always return a referenced bdd. */
  return BDD_STATES_INPUTS( resY );
}



/**Function********************************************************************

  Synopsis     []

  Description  [] 

  SeeAlso      []
  
  SideEffects  []

******************************************************************************/
static BddStatesInputs bdd_fsm_get_fair_SI_subset(const BddFsm_ptr self, 
						  BddStatesInputs subspace)
{
  BddStatesInputs res;
  BddStatesInputs old;
  int i = 0;

  BDD_FSM_CHECK_INSTANCE(self);

  res = bdd_one(self->dd);
  old = bdd_zero(self->dd);

  /* GFP computation */
  while (res != old) { 
    BddStatesInputs new;

    if (opt_verbose_level_gt(options, 1)) {
      fprintf(nusmv_stderr, "  size of res%d = %g <states>x<input>, %d BDD nodes\n",
              i++, BddEnc_count_states_inputs_of_bdd(self->enc, res),
              bdd_size(self->dd, res));
    }

    bdd_free(self->dd, old);                             
    old = bdd_dup(res);                                    

    /* MAP( ApplicableStatesInputs) over Fairness constraints */
    /* return GFP Z. (Q /\ EX_SI ( Z /\ AND_i EU_SI(Z, (Z/\ StatesInputFC_i)))) */
    new = bdd_fsm_fair_SI_subset_aux(self, BDD_STATES_INPUTS(res), subspace);

    bdd_and_accumulate(self->dd, &res, (bdd_ptr) new);
    bdd_and_accumulate(self->dd, &res, (bdd_ptr) subspace);

    bdd_free(self->dd, (bdd_ptr) new); 
  }                                                             
  bdd_free(self->dd, old);
  
  return BDD_STATES_INPUTS(res);
}

/**Function********************************************************************

  Synopsis     []

  Description  [] 

  SeeAlso      []
  
  SideEffects  []

******************************************************************************/

static BddStatesInputs bdd_fsm_fair_SI_subset_aux(const BddFsm_ptr self,
						  BddStatesInputs states, 
						  BddStatesInputs subspace)
{
  BddStatesInputs res;
  FairnessListIterator_ptr iter;  
  BddStatesInputs partial_result;
  int i;

  res = bdd_one(self->dd);
  partial_result = bdd_dup(states);
  i = 0;

  /* Accumulates justice constraints: */
  iter = FairnessList_begin( FAIRNESS_LIST(self->justice) );
  while ( ! FairnessListIterator_is_end(iter) ) {
    BddStatesInputs p;
    BddStatesInputs constrained_state;
    BddStatesInputs temp;

    p = JusticeList_get_p(self->justice, iter);
    constrained_state = bdd_and(self->dd, states, p);
    temp = bdd_fsm_EU_SI(self, subspace, constrained_state);

    bdd_free(self->dd, constrained_state);
    bdd_free(self->dd, p);

    bdd_and_accumulate(self->dd, &partial_result, temp); 
    bdd_free(self->dd, temp);

    iter = FairnessListIterator_next(iter);      
    i++;
  } /* outer while loop */


  res = bdd_fsm_EX_SI(self, partial_result);
  bdd_free(self->dd, partial_result);

  return res;  
}


/**Function********************************************************************

  Synopsis     []

  Description  []
  
  SideEffects  []

******************************************************************************/
static BddStatesInputs
bdd_fsm_get_fair_states_inputs_in_subspace(const BddFsm_ptr self, 
					   BddStatesInputs subspace)
{
  BddStates fair_states;
  BddStatesInputs fair_states_inputs;
  BddStatesInputs applicable_states_inputs;

  applicable_states_inputs = BddFsm_get_states_inputs_constraints(self);

  bdd_and_accumulate(self->dd, &applicable_states_inputs, subspace);

  fair_states_inputs = bdd_fsm_get_fair_SI_subset(self, 
						  applicable_states_inputs);

  bdd_free(self->dd, applicable_states_inputs);

  return fair_states_inputs;
}


/**Function********************************************************************

  Synopsis    [Check inits for emptyness, and prints a warning if needed]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void bdd_fsm_check_init_state_invar_emptyness(const BddFsm_ptr self)
{
  /* checks for emptyness of inits: */
  if (bdd_is_zero(self->dd, self->init)) {
    warning_fsm_init_empty();
  }
  if (bdd_is_zero(self->dd, self->invar_states)) {
    warning_fsm_invar_empty();
  }
}


/**Function********************************************************************

  Synopsis    [Checks fair states for emptyness, as well as fot the 
  intersaction of fair states and inits. Prints a warning if needed ]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void bdd_fsm_check_fairness_emptyness(const BddFsm_ptr self)
{
  bdd_ptr zero;
  bdd_ptr fair; 

  fair = BddFsm_get_fair_states_inputs(self);

  if (bdd_is_zero(self->dd, fair)) {
    warning_fsm_fairness_empty();
  }
  else if (bdd_isnot_zero(self->dd, self->init)) {
    bdd_ptr fair_init = bdd_and(self->dd, self->init, fair);

    if (bdd_is_zero(self->dd, fair_init)) {
      warning_fsm_init_and_fairness_empty();
    }
    bdd_free(self->dd, fair_init);
  }

  bdd_free(self->dd, fair);
}

