/**CHeaderFile*****************************************************************

  FileName    [Encoding_private.h]

  PackageName [enc.symb]

  Synopsis    [The generic encoding private interface, and Encoding class 
  declaration]

  Description []
                                               
  SeeAlso     [Encoding.c, Encoding.h]

  Author      [Roberto Cavada, Daniel Sheridan]

  Copyright   [
  This file is part of the ``enc.symb'' package of NuSMV version 2. 
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

#ifndef __ENC_SYMB__ENCODING_PRIVATE_H__
#define __ENC_SYMB__ENCODING_PRIVATE_H__

#include "Encoding.h"
#include "GroupSet.h"
#include "EncCache.h"

#include "utils/utils.h"
#include "utils/NodeList.h"


/**Struct**********************************************************************

  Synopsis    [The Encoding structure.]

  Description []
  
******************************************************************************/
typedef struct Encoding_TAG 
{  
  /* for model symbols */
  boolean refill_symbols_lists;
  NodeList_ptr model_state_symbols;
  NodeList_ptr model_input_symbols;  
  NodeList_ptr model_state_input_symbols;

  /* for input variables: */
  NodeList_ptr input_variables;
  NodeList_ptr model_input_variables;
  NodeList_ptr input_boolean_variables;
  GroupSet_ptr input_boolean_variables_group;

  /* for state variables: */
  NodeList_ptr state_variables;
  NodeList_ptr state_boolean_variables;
  GroupSet_ptr state_boolean_variables_group;

  /* for determination variables */
  NodeList_ptr det_boolean_variables;

  /* for DEFINEs */
  NodeList_ptr define_symbols;

  /* for constants: */
  NodeList_ptr constants_list;

  /* other symbols and variables in the smv model: */
  NodeList_ptr all_symbols;
  NodeList_ptr all_variables;

  EncCache_ptr cache;

  /* tricky code used in ltl mc */
  boolean saved;
  struct {
    boolean refill_symbols_lists;
    NodeList_ptr model_state_symbols;
    NodeList_ptr model_input_symbols;  
    NodeList_ptr model_state_input_symbols;

    NodeList_ptr input_variables;
    NodeList_ptr model_input_variables;
    NodeList_ptr state_variables;
    NodeList_ptr all_symbols;
    NodeList_ptr all_variables;
    NodeList_ptr define_symbols;

    GroupSet_ptr input_boolean_variables_group;
    GroupSet_ptr state_boolean_variables_group;
  } saved_lists;
  
} Encoding;






#endif /* __ENC_SYMB__ENCODING_PRIVATE_H__ */ 
