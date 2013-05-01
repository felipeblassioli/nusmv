/**CHeaderFile*****************************************************************

  FileName    [compileInt.h]

  PackageName [compile]

  Synopsis    [Internal declaration needed for the compilation.]

  Description [This file provides the user routines to perform
  compilation of the read model into BDD.]

  Author      [Marco Roveri, Roberto Cavada]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and ITC-irst. 

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

  Revision    [$Id: compileInt.h,v 1.29.4.13.2.1 2005/11/16 12:09:45 nusmv Exp $]

******************************************************************************/

#ifndef __COMPILE_INT_H__
#define __COMPILE_INT_H__

#include "utils/utils.h"
#include "opt/opt.h"
#include "dd/dd.h"
#include "node/node.h"
#include "compile/compile.h"
#include "set/set.h"
#include "compile/compileFsmMgr.h"

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN FsmBuilder_ptr global_fsm_builder; 

EXTERN options_ptr options;

EXTERN FILE * nusmv_stderr;
EXTERN FILE * nusmv_stdout;

EXTERN int yylineno;

EXTERN DdManager * dd_manager;

EXTERN cmp_struct_ptr cmps;
EXTERN node_ptr parse_tree;

EXTERN node_ptr zero_number;
EXTERN node_ptr one_number;
EXTERN node_ptr boolean_range; 

EXTERN node_ptr fairness_constraints_bdd;
EXTERN bdd_ptr fair_states_bdd;

EXTERN add_ptr trans_add;
EXTERN bdd_ptr trans_bdd;

EXTERN node_ptr cp_trans_add;
EXTERN node_ptr cp_trans_bdd;
EXTERN node_ptr dp_trans_add;
EXTERN node_ptr dp_trans_bdd;
EXTERN node_ptr forward_quantifiers_add;
EXTERN node_ptr forward_quantifiers_bdd;
EXTERN node_ptr reverse_quantifiers_add;
EXTERN node_ptr reverse_quantifiers_bdd;

EXTERN add_ptr invar_add;
EXTERN bdd_ptr invar_bdd;

EXTERN add_ptr init_add;
EXTERN bdd_ptr init_bdd;

EXTERN node_ptr running_atom;
EXTERN bdd_ptr running_add;

EXTERN node_ptr proc_selector_internal_vname;
EXTERN add_ptr process_selector_add;
EXTERN node_ptr process_running_symbols;

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void build_model_monolithic ARGS((node_ptr, node_ptr, node_ptr, add_ptr));

EXTERN void Compile_CompileModelIwls95 ARGS((node_ptr, node_ptr, node_ptr, add_ptr));

EXTERN void     init_check_constant_hash    ARGS((void));
EXTERN node_ptr lookup_check_constant_hash  ARGS((node_ptr));
EXTERN void     clear_check_constant_hash   ARGS((void));

EXTERN void     init_value_hash       ARGS((void));
EXTERN void     insert_value_hash     ARGS((node_ptr, node_ptr));
EXTERN node_ptr lookup_value_hash     ARGS((node_ptr));
EXTERN void     clear_value_hash      ARGS((void));

EXTERN void     init_module_hash      ARGS((void));
EXTERN void     insert_module_hash    ARGS((node_ptr, node_ptr));
EXTERN node_ptr lookup_module_hash    ARGS((node_ptr));
EXTERN void     clear_module_hash     ARGS((void));

EXTERN void     init_constant_hash    ARGS((void));
EXTERN void     insert_constant_hash  ARGS((node_ptr, node_ptr));
EXTERN node_ptr lookup_constant_hash  ARGS((node_ptr));
EXTERN void     clear_constant_hash   ARGS((void));

EXTERN void     init_assign_db_hash       ARGS((void));
EXTERN void     insert_assign_db_hash     ARGS((node_ptr, node_ptr));
EXTERN void     clear_assign_db_hash      ARGS((void));


EXTERN int Iwls95PrintOption ARGS((FILE * fp));
EXTERN int CommandIwls95PrintOption ARGS((int argc, char ** argv));
EXTERN int CommandCPPrintClusterInfo ARGS((int argc, char ** argv));

EXTERN void 
compileFlattenSexpModel ARGS((Encoding_ptr, node_ptr, node_ptr, node_ptr));

EXTERN node_ptr compileFlattenProcess ARGS((Encoding_ptr, node_ptr));
EXTERN node_ptr get_bool_variable_name ARGS((int));

EXTERN void free_variable_sexp_model_hash_free ARGS((void));
EXTERN node_ptr var_model_sexp_build ARGS((node_ptr, node_ptr, node_ptr));
EXTERN node_ptr var_model_sexp_get_init ARGS((node_ptr));
EXTERN node_ptr var_model_sexp_get_invar ARGS((node_ptr));
EXTERN node_ptr var_model_sexp_get_next ARGS((node_ptr vm));

EXTERN void insert_coi_hash ARGS((node_ptr, Set_t));
EXTERN Set_t lookup_coi_hash ARGS((node_ptr));

EXTERN void insert_define_dep_hash ARGS((node_ptr, Set_t));
EXTERN Set_t lookup_define_dep_hash ARGS((node_ptr));

EXTERN void insert_dependencies_hash ARGS((node_ptr, Set_t));
EXTERN Set_t lookup_dependencies_hash ARGS((node_ptr));

EXTERN void check_wff ARGS((node_ptr, node_ptr));


#endif /* __COMPILE_INT_H__ */

