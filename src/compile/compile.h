/**CHeaderFile*****************************************************************

  FileName    [compile.h]

  PackageName [compile]

  Synopsis    [Compilation of NuSMV input language into BDD.]

  Description [This package contains the compiler of NuSMV code into
  BDD. It works on a flattened/instantiated structure. Performs the
  checks on the parse tree and generates the encoding of scalar
  variables into BDDs. Then, the transition relation is created with
  different methods.]

  Author      [Marco Roveri, Emanuele Olivetti]

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

  Revision    [$Id: compile.h,v 1.44.4.17.2.3 2005/11/16 12:09:45 nusmv Exp $]

******************************************************************************/
#ifndef _COMPILE
#define _COMPILE

#include "utils/utils.h"
#include "node/node.h"
#include "dd/dd.h"
#include "set/set.h"
#include "fsm/sexp/SexpFsm.h"
#include "fsm/sexp/Expr.h"

#include "enc/symb/Encoding.h"
#include "enc/bdd/BddEnc.h"


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Data structure used to store the results of compilation.]

  Description [Data structure used to store the results of compilation.]

******************************************************************************/
struct cmp_struct {
  int      read_model;
  int      flatten_hierarchy;
  int      encode_variables;
  int      process_selector;
  int      build_frames;
  int      compile_check;
  int      build_init;
  int      build_model;
  int      build_flat_model;
  int      build_bool_model;
  int      bmc_setup;
  int      fairness_constraints;
  int      coi;
  int      build_model_setup;
  node_ptr init_expr;
  node_ptr invar_expr;
  node_ptr trans_expr;
  node_ptr procs_expr;
  node_ptr justice_expr;
  node_ptr compassion_expr;
  node_ptr spec_expr;
  node_ptr compute_expr;
  node_ptr ltlspec_expr;
  node_ptr pslspec_expr;
  node_ptr invar_spec_expr;
};



/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Enum************************************************************************

  Synopsis    [Enumerates the different types of a specification]

  Description [Enumerates the different types of a specification]

******************************************************************************/
typedef enum {ST_Notype, ST_Ctl, ST_Ltl, ST_Invar, ST_Compute} Spec_Type;

/**Enum************************************************************************

  Synopsis    [Enumerates the status of a specification]

  Description [Enumerates the status of a specification]

******************************************************************************/
typedef enum {SS_Nostatus, SS_Unchecked, SS_True, SS_False, SS_Wrong, SS_Number} Spec_Status;

/**Enum************************************************************************

  Synopsis    [Enumerates the different result types of a formula]

  Description [Enumerates the different result types of a formula]

******************************************************************************/
typedef enum {wff_NIL, wff_ERROR, wff_BOOLEAN, wff_NUMERIC, wff_SYMBOLIC} wff_result_type;

//typedef struct Iwls95OptionStruct Iwls95OptionStruct_t;
typedef struct cmp_struct cmp_struct_rec;
typedef struct cmp_struct * cmp_struct_ptr;

typedef struct _Fsm_SexpRec    Fsm_SexpRec;
typedef struct _Fsm_SexpRec  * Fsm_SexpPtr;




#include "be/be.h" /* the generic boolean expressions interface */

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [The symbolic name of the input process selector variable.]

  Description  [This is the internal symbolic name of process selector
  variable. The range of this variable is the set of names of the
  instantiated processes.]

  SideEffects  []

******************************************************************************/
#define PROCESS_SELECTOR_VAR_NAME "_process_selector_"

/**Macro***********************************************************************

  Synopsis     [The "running" symbol.]

  Description  [The "running" symbol used to refer the internal
  variable "running" of  processes.]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define RUNNING_SYMBOL "running"

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void Compile_Init ARGS((void));
EXTERN void Compile_End  ARGS((void));

EXTERN void Compile_CheckProgram ARGS((node_ptr, node_ptr, node_ptr, 
				       node_ptr, node_ptr, node_ptr));

EXTERN NodeList_ptr 
Compile_get_expression_dependencies ARGS((const Encoding_ptr, Expr_ptr));

EXTERN void CompileFlatten_quit_flattener ARGS((void));

EXTERN Expr_ptr 
CompileFlatten_resolve_name ARGS((Expr_ptr n, node_ptr context));


EXTERN node_ptr node_plus ARGS((node_ptr, node_ptr));
EXTERN node_ptr node_plus1 ARGS((node_ptr));

EXTERN node_ptr Compile_ReadOrder ARGS((Encoding_ptr senc, const char *));
EXTERN void 
Compile_WriteOrder ARGS((const BddEnc_ptr enc, const char *, boolean));

EXTERN node_ptr sym_intern ARGS((char *));

EXTERN void 
Compile_FlattenHierarchy ARGS((Encoding_ptr senc, 
			       node_ptr, node_ptr, node_ptr *, node_ptr *,
			       node_ptr *, node_ptr *, node_ptr *,
			       node_ptr *, node_ptr *, node_ptr *,
			       node_ptr *, node_ptr *, node_ptr *,
			       node_ptr));

EXTERN void 
Compile_FlattenTableau ARGS((Encoding_ptr senc, 
			     node_ptr, node_ptr, node_ptr *, node_ptr *,
			     node_ptr *, node_ptr *, node_ptr *,
			     node_ptr *, node_ptr *, node_ptr *,
			     node_ptr *, node_ptr *, node_ptr *,
			     node_ptr));

EXTERN void Compile_BuildVarsBdd ARGS((void));

EXTERN void build_proc_selector ARGS((node_ptr));
EXTERN void Compile_CompileInit ARGS((node_ptr, node_ptr));
EXTERN void Compile_CompileModel ARGS((node_ptr, node_ptr, node_ptr, add_ptr));


void init_param_hash ARGS((void));
void insert_param_hash ARGS((node_ptr, node_ptr));
node_ptr lookup_param_hash ARGS((node_ptr));
void clear_param_hash ARGS((void));

void init_flatten_constant_hash ARGS((void));
void insert_flatten_constant_hash ARGS((node_ptr, node_ptr));
node_ptr lookup_flatten_constant_hash ARGS((node_ptr));
void clear_flatten_constant_hash ARGS((void));

void init_flatten_def__hash ARGS((void));
void insert_flatten_def_hash ARGS((node_ptr, node_ptr));
node_ptr lookup_flatten_def_hash ARGS((node_ptr));
void clear_flatten_def_hash ARGS((void));


void init_check_constant_hash ARGS((void));
EXTERN void insert_check_constant_hash ARGS((node_ptr, node_ptr));
node_ptr lookup_check_constant_hash ARGS((node_ptr));
void clear_check_constant_hash ARGS((void));

void init_flatten_def_hash ARGS((void));
void insert_flatten_def_hash ARGS((node_ptr, node_ptr));
node_ptr lookup_flatten_def_hash ARGS((node_ptr));
void clear_flatten_def_hash ARGS((void));

void init_wfftype_hash ARGS((void));
void insert_wfftype_hash ARGS((node_ptr, wff_result_type));
wff_result_type lookup_wfftype_hash ARGS((node_ptr));
void clear_wfftype_hash ARGS((void));

void init_assign_db_hash ARGS((void));
void insert_assign_db_hash ARGS((node_ptr, node_ptr));
node_ptr lookup_assign_db_hash ARGS((node_ptr));
void clear_assign_db_hash ARGS(());

void compileCheckForInputVars ARGS((Encoding_ptr, node_ptr, node_ptr,
				    node_ptr, node_ptr));
void compileCheckInvarForInputVars ARGS((Encoding_ptr, node_ptr));
void compileCheckInitForInputVars ARGS((Encoding_ptr, node_ptr));
void compileCheckTransForInputVars ARGS((Encoding_ptr, node_ptr));
void compileCheckAssignForInputVars ARGS((Encoding_ptr, node_ptr));
boolean compileExpressionHasNextInputs ARGS((Encoding_ptr, node_ptr));

EXTERN cmp_struct_ptr cmp_struct_init ARGS((void));
EXTERN int cmp_struct_get_read_model ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_read_model ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_flatten_hrc ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_flatten_hrc ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_encode_variables ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_encode_variables ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_process_selector ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_process_selector ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_build_frames ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_build_frames ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_compile_check ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_compile_check ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_build_init ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_build_init ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_build_model ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_build_model ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_build_flat_model ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_build_flat_model ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_build_bool_model ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_build_bool_model ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_fairness ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_fairness ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_coi ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_coi ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_build_model_setup ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_build_model_setup ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_bmc_setup ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_bmc_setup ARGS((cmp_struct_ptr cmp));
EXTERN node_ptr cmp_struct_get_init ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_init ARGS((cmp_struct_ptr cmp, node_ptr n));
EXTERN node_ptr cmp_struct_get_invar ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_invar ARGS((cmp_struct_ptr cmp, node_ptr n));
EXTERN node_ptr cmp_struct_get_trans ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_trans ARGS((cmp_struct_ptr cmp, node_ptr n));
EXTERN node_ptr cmp_struct_get_procs ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_procs ARGS((cmp_struct_ptr cmp, node_ptr n));
EXTERN node_ptr cmp_struct_get_justice ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_justice ARGS((cmp_struct_ptr cmp, node_ptr n));
EXTERN node_ptr cmp_struct_get_compassion ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_compassion ARGS((cmp_struct_ptr cmp, node_ptr n));
EXTERN node_ptr cmp_struct_get_spec ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_spec ARGS((cmp_struct_ptr cmp, node_ptr n));
EXTERN node_ptr cmp_struct_get_compute ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_compute ARGS((cmp_struct_ptr cmp, node_ptr n));
EXTERN node_ptr cmp_struct_get_ltlspec ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_ltlspec ARGS((cmp_struct_ptr cmp, node_ptr n));
EXTERN node_ptr cmp_struct_get_pslspec ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_pslspec ARGS((cmp_struct_ptr cmp, node_ptr n));
EXTERN node_ptr cmp_struct_get_invar_spec ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_invar_spec ARGS((cmp_struct_ptr cmp, node_ptr n));

EXTERN void init_module_hash ARGS((void));
EXTERN void insert_module_hash ARGS((node_ptr, node_ptr));
EXTERN node_ptr lookup_module_hash ARGS((node_ptr));
EXTERN void clear_module_hash ARGS((void));

EXTERN void init_expr2bexp_hash ARGS((void)); 
EXTERN void quit_expr2bexpr_hash ARGS((void));


EXTERN node_ptr Compile_FlattenSexp ARGS((const Encoding_ptr, node_ptr, 
					  node_ptr));

EXTERN node_ptr 
Compile_FlattenSexpExpandDefine ARGS((const Encoding_ptr senc, node_ptr, 
				      node_ptr));

EXTERN node_ptr Compile_FlattenProcess ARGS((node_ptr));

EXTERN int Compile_WriteFlatten ARGS((const Encoding_ptr, FILE*, 
				      cmp_struct_ptr));

EXTERN int 
Compile_WriteFlattenBool ARGS((const Encoding_ptr, FILE*, SexpFsm_ptr, 
			       cmp_struct_ptr));

EXTERN Expr_ptr expr2bexpr ARGS((Expr_ptr));
EXTERN Expr_ptr detexpr2bexpr ARGS((Expr_ptr));

EXTERN void init_coi_hash ARGS((void));
EXTERN void clear_coi_hash ARGS((void));
EXTERN void init_define_dep_hash ARGS((void));
EXTERN void clear_define_dep_hash ARGS((void));
EXTERN void init_dependencies_hash ARGS((void));
EXTERN void clear_dependencies_hash ARGS((void));
EXTERN Set_t Formula_GetDependencies ARGS((const Encoding_ptr senc, node_ptr, 
					   node_ptr));

EXTERN Set_t 
Formulae_GetDependencies ARGS((const Encoding_ptr senc, node_ptr, node_ptr, 
			       node_ptr));

EXTERN Set_t ComputeCOI ARGS((const Encoding_ptr senc, Set_t));

EXTERN void print_conjunctive_partition_info ARGS((FILE *, node_ptr));
EXTERN bdd_ptr Compile_BuildInitBdd ARGS((node_ptr));
EXTERN bdd_ptr Compile_BuildInvarBdd ARGS((node_ptr, add_ptr));

EXTERN void Compile_InitializeBuildModel ARGS((void));

EXTERN void start_test ARGS((char *));
EXTERN void end_test ARGS((char *));

EXTERN int Compile_CheckWff ARGS((node_ptr, node_ptr));

EXTERN boolean 
Compile_is_input_variable_formula ARGS((const Encoding_ptr senc, node_ptr n));

void set_definition_mode_to_get ARGS((void));
void set_definition_mode_to_expand ARGS((void));
int definition_mode_is_expand ARGS((void));

void 
instantiate_var ARGS((Encoding_ptr senc, node_ptr, node_ptr, node_ptr *, 
		      node_ptr *, node_ptr *, node_ptr *, node_ptr *, 
		      node_ptr *, 
		      node_ptr *, node_ptr *, node_ptr *, node_ptr *, 
		      node_ptr *, node_ptr *, node_ptr));
void 
instantiate_vars ARGS((Encoding_ptr senc, node_ptr, node_ptr, node_ptr *, 
		       node_ptr *, node_ptr *, node_ptr *, node_ptr *, 
		       node_ptr *, 
		       node_ptr *, node_ptr *, node_ptr *, node_ptr *, 
		       node_ptr *, node_ptr *));

node_ptr Flatten_GetDefinition ARGS((const Encoding_ptr senc, node_ptr atom));

#endif /* _COMPILE */
