/**CHeaderFile*****************************************************************

  FileName    [BddFsm.h]

  PackageName [fsm.bdd]

  Synopsis    [Declares the interface of the class BddFsm]

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


#ifndef __FSM_BDD_BDD_FSM_H__
#define __FSM_BDD_BDD_FSM_H__

#include "bdd.h"
#include "FairnessList.h"

#include "utils/utils.h"  /* for EXTERN and ARGS */
#include "dd/dd.h"
#include "trans/bdd/BddTrans.h"
#include "enc/bdd/BddEnc.h" /* Encoding */
#include "fsm/sexp/sexp.h" /* VarSet_ptr */



/**Type************************************************************************

  Synopsis     []

  Description  []

  Notes        []

******************************************************************************/
typedef struct BddFsm_TAG*  BddFsm_ptr;

#define BDD_FSM(x) \
         ((BddFsm_ptr) x)

#define BDD_FSM_CHECK_INSTANCE(x) \
         (nusmv_assert( BDD_FSM(x) != BDD_FSM(NULL) ))



/* ---------------------------------------------------------------------- */
/* public interface                                                       */
/* ---------------------------------------------------------------------- */

EXTERN BddFsm_ptr 
BddFsm_create ARGS((BddEnc_ptr encoding, BddStates init, 
		    BddInvarStates invar_states, BddInvarInputs invar_inputs, 
		    BddTrans_ptr trans, 
		    JusticeList_ptr justice, CompassionList_ptr compassion));

EXTERN void BddFsm_destroy ARGS((BddFsm_ptr self));

EXTERN BddFsm_ptr BddFsm_copy ARGS((const BddFsm_ptr self));

EXTERN JusticeList_ptr BddFsm_get_justice ARGS((const BddFsm_ptr self));

EXTERN CompassionList_ptr BddFsm_get_compassion ARGS((const BddFsm_ptr self)); 

EXTERN BddStates BddFsm_get_init ARGS((const BddFsm_ptr self));

EXTERN BddInvarStates 
BddFsm_get_state_constraints ARGS((const BddFsm_ptr self));

EXTERN BddInvarInputs 
BddFsm_get_input_constraints ARGS((const BddFsm_ptr self));

EXTERN BddTrans_ptr BddFsm_get_trans ARGS((const BddFsm_ptr self));

EXTERN BddStates BddFsm_get_fair_states ARGS((BddFsm_ptr self));
EXTERN BddStatesInputs BddFsm_get_fair_states_inputs ARGS((BddFsm_ptr self));

EXTERN BddStatesInputs BddFsm_get_monolithic_trans_bdd ARGS((BddFsm_ptr self));

EXTERN BddStates BddFsm_get_reachable_states ARGS((BddFsm_ptr self));

EXTERN BddStates 
BddFsm_get_reachable_states_at_distance ARGS((BddFsm_ptr self, 
					      int distance));

EXTERN int 
BddFsm_get_distance_of_states ARGS((BddFsm_ptr self, 
				    BddStates states));

EXTERN int BddFsm_get_diameter ARGS((BddFsm_ptr self));

EXTERN BddStates 
BddFsm_get_not_successor_states ARGS((BddFsm_ptr self));

EXTERN BddStates BddFsm_get_deadlock_states ARGS((BddFsm_ptr self));

EXTERN boolean BddFsm_is_total ARGS((BddFsm_ptr self));

EXTERN boolean BddFsm_is_deadlock_free ARGS((BddFsm_ptr self));

EXTERN BddStates
BddFsm_states_inputs_to_states ARGS((const BddFsm_ptr self,
				     BddStatesInputs si));

EXTERN BddInputs
BddFsm_states_inputs_to_inputs ARGS((const BddFsm_ptr self,
				     BddStatesInputs si));

EXTERN BddStates 
BddFsm_get_forward_image ARGS((const BddFsm_ptr self, BddStates states));

EXTERN BddStates 
BddFsm_get_constrained_forward_image ARGS((const BddFsm_ptr self, 
					   BddStates states, 
					   BddStatesInputs constraints));

EXTERN BddStates 
BddFsm_get_backward_image ARGS((const BddFsm_ptr self, BddStates states));

EXTERN BddStates 
BddFsm_get_constrained_backward_image ARGS((const BddFsm_ptr self, 
					    BddStates states, 
					    BddStatesInputs constraints));

EXTERN BddStatesInputs 
BddFsm_get_weak_backward_image ARGS((const BddFsm_ptr self, 
				     BddStates states));

EXTERN BddStatesInputs 
BddFsm_get_k_backward_image ARGS((const BddFsm_ptr self, 
				  BddStates states,
				  int k));

EXTERN BddStatesInputs 
BddFsm_get_strong_backward_image ARGS((const BddFsm_ptr self, 
				       BddStates states));


EXTERN void BddFsm_print_info ARGS((const BddFsm_ptr self, FILE* file));

EXTERN void 
BddFsm_print_reachable_states_info ARGS((const BddFsm_ptr self, 
					 const boolean print_states, 
					 FILE* file));

EXTERN void 
BddFsm_print_fair_states_info ARGS((const BddFsm_ptr self, 
				    const boolean print_states, 
				    FILE* file));

EXTERN void 
BddFsm_print_fair_transitions_info ARGS((const BddFsm_ptr self, 
					 const boolean print_states, 
					 FILE* file));

EXTERN void BddFsm_check_machine ARGS((const BddFsm_ptr self));

EXTERN void 
BddFsm_apply_synchronous_product ARGS((BddFsm_ptr self, 
				       const BddFsm_ptr other));

/* temporary per il momento */
EXTERN boolean BddFsm_is_fair_states ARGS((const BddFsm_ptr self,
					   BddStates states));

EXTERN BddInputs
BddFsm_states_to_states_get_inputs ARGS((const BddFsm_ptr self,
					 BddStates cur_states,
					 BddStates next_states));

EXTERN BddStatesInputs 
BddFsm_get_states_inputs_constraints ARGS((const BddFsm_ptr self));

EXTERN BddStates 
BddFsm_get_fair_states_subset ARGS((const BddFsm_ptr self,
                                    BddStates subspace));

EXTERN BddStatesInputs
BddFsm_get_fair_states_inputs ARGS((BddFsm_ptr self));

EXTERN BddStates BddFsm_states_inputs_to_states ARGS((const BddFsm_ptr self,
                                                      BddStatesInputs si));

EXTERN BddStates BddFsm_states_inputs_to_inputs ARGS((const BddFsm_ptr self,
                                                      BddStatesInputs si));

EXTERN BddStatesInputs BddFsm_get_fair_states_inputs ARGS((BddFsm_ptr self));


/* temporary per il momento */

#endif /* __FSM_BDD_BDD_FSM_H__ */
