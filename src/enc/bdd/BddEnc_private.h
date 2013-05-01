/**CHeaderFile*****************************************************************

  FileName    [BddEnc_private.h]

  PackageName [enc.bdd]

  Synopsis    [The Bdd encoding private interface, and BddEnc class 
  declaration]

  Description []
                                               
  SeeAlso     [BddEnc.c, BddEnc.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.bdd'' package of NuSMV version 2. 
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

#ifndef __ENC_BDD_BDD_ENC_PRIVATE_H__
#define __ENC_BDD_BDD_ENC_PRIVATE_H__

#include "BddEnc.h"
#include "BddEncCache.h"

#include "utils/utils.h"
#include "utils/assoc.h"
#include "node/node.h"
#include "dd/dd.h"



/**Macro***********************************************************************

  Synopsis     [Return value used to indicate that the evaluation of
  an atom is not yet terminated.]

  Description  ]

  SeeAlso      [get_definition]

  SideEffects  []

******************************************************************************/
#define EVALUATING ((add_ptr)(-1))


/**Struct**********************************************************************

  Synopsis    [The BddEnc structure.]

  Description [For the time being, this is just a dummy structure]
  
******************************************************************************/
typedef struct BddEnc_TAG 
{
  Encoding_ptr senc; 
  BddEncCache_ptr cache;

  DdManager* dd;

  /* ---------------------------------------------------------------------
     Warning: all add and bdd members must be accessed via the
     provided methods! 
     --------------------------------------------------------------------- */

  boolean saved; 
  /* these members are saved by BddEnc_save */
  struct {
    /* The cube of input variables to be used in image forward and backward */
    add_ptr __input_variables_add; 
    bdd_ptr __input_variables_bdd;
    
    /* The cube of state variables to be used in image forward */
    add_ptr __state_variables_add; 
    bdd_ptr __state_variables_bdd;

    /* The cube of state variables to be used in image backward */
    add_ptr __next_state_variables_add;
    bdd_ptr __next_state_variables_bdd;

    /* the size of array minterm_vars */
    int minterm_input_vars_dim;
    int minterm_state_vars_dim;
    int minterm_next_state_vars_dim;
    int minterm_state_input_vars_dim;
  
    node_ptr state_vars_add_list; /* ADDs list of state variables */

    /* The number of boolean state variables created to encode
       symbolic state variables, current and next */
    int num_of_state_vars;
  
    /* The number of boolean input state variables created to encode
       symbolic input variables */
    int num_of_input_vars;

  } state, saved_state;  
  
    
    
  /* The array of symbolic variable names. Each element i contains the
     symbolic name associated to the variable with index i */
  node_ptr variable_names[MAX_VAR_INDEX];
  
  /* These arrays are used to maintain correspondence between current
  and next variables. Position i contains the index of the
  corresponding next state variable. They are used to perform forward
  and backward shifting respectively */
  int current2next[MAX_VAR_INDEX]; 
  int next2current[MAX_VAR_INDEX];

  /* Array used to pick up a minterm from a given BDD. This array
     should contain at least all variables in the support of the BDD
     which we want extract a minterm of */
  bdd_ptr minterm_input_vars[MAX_VAR_INDEX]; 
  bdd_ptr minterm_state_vars[MAX_VAR_INDEX]; 
  bdd_ptr minterm_next_state_vars[MAX_VAR_INDEX]; 
  bdd_ptr minterm_state_input_vars[MAX_VAR_INDEX]; 
  
  /* This is a stack of instances of class BddEncPrintInfo, used 
   to print Bdds */
  node_ptr print_stack;
  
  /* This variable is used to control the behavior of the
     method bdd_enc_eval (specifically the behavior of its subroutine
     get_definition). */
  boolean enforce_constant;

  /* Used for grouping of variables: */
  int dyn_reord_flag; 
  dd_reorderingtype dyn_reord_type;
  int group_begin_start;
  MtrNode* group_tree;

  /* masks: */
  add_ptr __state_vars_mask_add;
  add_ptr __input_vars_mask_add;
  add_ptr __state_input_vars_mask_add;
  bdd_ptr __state_vars_mask_bdd;
  bdd_ptr __input_vars_mask_bdd;
  bdd_ptr __state_input_vars_mask_bdd;
  
} BddEnc;




#endif /* __ENC_BDD_BDD_ENC_PRIVATE_H__ */
