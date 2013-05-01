/**CFile***********************************************************************

  FileName    [optCmd.c]

  PackageName [opt]

  Synopsis    [The option command file.]

  Description [optional]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``opt'' package of NuSMV version 2. 
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

******************************************************************************/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "opt/opt.h"
#include "opt/optInt.h" 
#include "bmc/bmcUtils.h"
#include "trans/trans.h"  /* to access TransType interface */
#include "trace/pkg_trace.h"  /* to access TransType interface */

static char rcsid[] UTIL_UNUSED = "$Id: optCmd.c,v 1.48.4.15.2.11 2005/10/27 14:44:25 nusmv Exp $";

int CommandSetVariable ARGS((int  argc, char ** argv));
int CommandUnsetVariable ARGS((int  argc, char ** argv));

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void print_options ARGS((options_ptr opt, FILE * fn));
static char* remove_non_existant_pps ARGS((const char* pp_list));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initialize the NuSMV options.]

  Description        [The NuSMV options are initialized. A pointer to
  a structure containing the NuSMV options is allocated, its fields
  are initialized and the pointer is returned.]

  SideEffects        []

******************************************************************************/
options_ptr init_options()
{
  options_ptr opt;

  opt = ALLOC(Options, 1);
  opt->pgm_name = DEFAULT_PGM_NAME;
  opt->input_file = DEFAULT_INPUT_FILE;
  opt->input_order_file = DEFAULT_INPUT_ORDER_FILE;
  opt->output_order_file = DEFAULT_OUTPUT_ORDER_FILE;
  opt->output_flatten_model_file = NIL(char);
  opt->output_boolean_model_file = NIL(char);
  opt->verbose = 0;
  opt->pp_list = util_strsav("");
  opt->shown_states = DEFAULT_SHOWN_STATES;
  opt->check_fsm = false;
  opt->ignore_spec = false;
  opt->ignore_compute = false;
  opt->ignore_ltlspec = false;
  opt->ignore_pslspec = false;
  opt->ignore_invar = false;
  opt->forward_search = false;
  opt->print_reachable = false;
  opt->reorder = false;
  opt->reorder_method  = DEFAULT_REORDER;
  opt->dynamic_reorder = false;
  opt->batch = true;
  opt->partition_method = TRANS_TYPE_THRESHOLD;
  opt->conj_part_threshold = DEFAULT_CONJ_PART_THRESHOLD;
  opt->image_cluster_size = DEFAULT_IMAGE_CLUSTER_SIZE;
  opt->ignore_init_file = false;
  opt->ag_only = false;
  opt->cone_of_influence = false;
  opt->list_properties = false;
  opt->prop_no = -1;
  /* BMC Stuff */
  opt->bmc_mode = false;
  opt->bmc_dimacs_filename = DEFAULT_DIMACS_FILENAME ;
  opt->bmc_invar_dimacs_filename = DEFAULT_INVAR_DIMACS_FILENAME ;
  opt->bmc_pb_length = DEFAULT_BMC_PB_LENGTH;
  opt->bmc_pb_loop = NULL; set_bmc_pb_loop(opt, DEFAULT_BMC_PB_LOOP);
  opt->bmc_invar_alg = NULL; set_bmc_invar_alg(opt,DEFAULT_BMC_INVAR_ALG);
#if HAVE_INCREMENTAL_SAT
  opt->bmc_inc_invar_alg = NULL; set_bmc_inc_invar_alg(opt,
						       DEFAULT_BMC_INC_INVAR_ALG);
#endif

  opt->bmc_optimized_tableau = DEFAULT_BMC_OPTIMIZED_TABLEAU;
  opt->bmc_force_pltl_tableau  = DEFAULT_BMC_FORCE_PLTL_TABLEAU;

  /* SAT Stuff */
  set_sat_solver(opt, DEFAULT_SAT_SOLVER);

  opt->preorder = false;
  opt->affinity = true;
  
  /* Temporary Stuff */
  opt->use_fair_states = true;
  opt->use_reachable_states = false;

  /* others */
  opt->counter_examples = true;
  opt->on_failure_script_quits = false;
  opt->write_order_dumps_bits = false;

  /* SA Stuff */
#if HAVE_SA
  opt->sa_stsa_fm_mapping_file = DEFAULT_SA_STSA_FM_MAPPING_FILE;
#endif

  return opt;
}

/**Function********************************************************************

  Synopsis           [Initialize the NuSMV options command]

  Description        [This function declares the interactive shell
  commands necessary to manipulate NuSMV options.]

  SideEffects        []

******************************************************************************/
void init_options_cmd()
{
  char *path;
  char *lib_name;

  Cmd_CommandAdd("set", CommandSetVariable, 0);
  Cmd_CommandAdd("unset", CommandUnsetVariable, 0);
  lib_name = Sm_NuSMVObtainLibrary();
  path = ALLOC(char, strlen(lib_name) + 20);
  sprintf(path, "set open_path .:%s", lib_name);
  Cmd_CommandExecute(path);
  FREE(lib_name);
  FREE(path);
}

void set_pgm_name(options_ptr opt, char * str)
{
  nusmv_assert(opt != NULL);
  opt->pgm_name = util_strsav(str);
}
void reset_pgm_name(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->pgm_name = DEFAULT_PGM_NAME;
}
char* get_pgm_name(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->pgm_name);
}

void set_input_file(options_ptr opt, char * str)
{
  nusmv_assert(opt != NULL);
  if (util_is_string_null(str)) opt->input_file = NIL(char);
  else opt->input_file = util_strsav(str);
}
void reset_input_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->input_file = DEFAULT_INPUT_FILE;
}
char * get_input_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->input_file);
}

void set_input_order_file(options_ptr opt, char * str)
{
  nusmv_assert(opt != NULL);
  if (util_is_string_null(str)) opt->input_order_file = NIL(char);
  else opt->input_order_file = util_strsav(str);
}
void reset_input_order_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->input_order_file = DEFAULT_INPUT_ORDER_FILE;
}
char * get_input_order_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->input_order_file);
}

void set_output_order_file(options_ptr opt, char * str)
{
  nusmv_assert(opt != NULL);
  if (util_is_string_null(str)) opt->output_order_file = NIL(char);
  else opt->output_order_file = util_strsav(str);
}
void reset_output_order_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->output_order_file = DEFAULT_OUTPUT_ORDER_FILE;
}
char * get_output_order_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->output_order_file);
}
boolean is_default_order_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  if (opt->output_order_file == NIL(char)) {
    return DEFAULT_OUTPUT_ORDER_FILE == NIL(char);
  }
  if (DEFAULT_OUTPUT_ORDER_FILE == NIL(char)) return 0;
  return((strcmp(opt->output_order_file, DEFAULT_OUTPUT_ORDER_FILE) == 0));
}

void set_output_flatten_model_file(options_ptr opt, char * str)
{
  nusmv_assert(opt != NULL);
  if (util_is_string_null(str)) opt->output_flatten_model_file = NIL(char);
  else opt->output_flatten_model_file = util_strsav(str);
}
void reset_output_flatten_model_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->output_flatten_model_file = NIL(char);
}
char * get_output_flatten_model_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->output_flatten_model_file);
}

void set_output_boolean_model_file(options_ptr opt, char * str)
{
  nusmv_assert(opt != NULL);
  if (util_is_string_null(str))  opt->output_boolean_model_file = NIL(char);
  else opt->output_boolean_model_file = util_strsav(str);
}
void reset_output_boolean_model_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->output_boolean_model_file = NIL(char);
}
char * get_output_boolean_model_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->output_boolean_model_file);
}

void set_verbose_level(options_ptr opt, int level)
{
  nusmv_assert(opt != NULL);
  opt->verbose = level;
}
int get_verbose_level(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->verbose);
}
boolean opt_verbose_level_eq(options_ptr opt, int level)
{
  nusmv_assert(opt != NULL);
  return((opt->verbose == level));
}
boolean opt_verbose_level_gt(options_ptr opt, int level)
{
  nusmv_assert(opt != NULL);
  return((opt->verbose > level));
}
boolean opt_verbose_level_ge(options_ptr opt, int level)
{
  nusmv_assert(opt != NULL);
  return((opt->verbose >= level));
}
boolean opt_verbose_level_lt(options_ptr opt, int level)
{
  nusmv_assert(opt != NULL);
  return((opt->verbose < level));
}
boolean opt_verbose_level_le(options_ptr opt, int level)
{
  nusmv_assert(opt != NULL);
  return((opt->verbose <= level));
}

void set_pp_list(options_ptr opt, char * pp_list)
{
  char* new_pp_list;

  if (strcmp(pp_list,"") != 0) {
    new_pp_list = remove_non_existant_pps(pp_list);
  }
  else {
    new_pp_list = util_strsav("");
  }

  nusmv_assert(opt != NULL);
  if (opt->pp_list != (char*) NULL) {
    FREE(opt->pp_list);
  }
  opt->pp_list = new_pp_list;
}
char* get_pp_list(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return opt->pp_list;
}

void set_shown_states_level(options_ptr opt, int level)
{
  nusmv_assert(opt != NULL);
  opt->shown_states = level;
}
int opt_shown_states_level(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->shown_states));
}

void set_ignore_spec(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_spec = true;
}
void unset_ignore_spec(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_spec = false;
}
boolean opt_ignore_spec(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->ignore_spec == true));
}

void set_ignore_compute(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_compute = true;
}
void unset_ignore_compute(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_compute = false;
}
boolean opt_ignore_compute(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->ignore_compute == true));
}

void set_ignore_ltlspec(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_ltlspec = true;
}
void unset_ignore_ltlspec(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_ltlspec = false;
}
boolean opt_ignore_ltlspec(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->ignore_ltlspec == true));
}

void set_ignore_pslspec(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_pslspec = true;
}
void unset_ignore_pslspec(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_pslspec = false;
}
boolean opt_ignore_pslspec(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->ignore_pslspec == true));
}

void set_check_fsm(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->check_fsm = true;
}
void unset_check_fsm(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->check_fsm = false;
}
boolean opt_check_fsm(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->check_fsm == true));
}

void set_ignore_invar(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_invar = true;
}
void unset_ignore_invar(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_invar = false;
}
boolean opt_ignore_invar(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->ignore_invar == true));
}

void set_forward_search(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->forward_search = true;
}
void unset_forward_search(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->forward_search = false;
}
boolean opt_forward_search(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->forward_search == true));
}

void set_print_reachable(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->print_reachable = true;
}
void unset_print_reachable(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->print_reachable = false;
}
boolean opt_print_reachable(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->print_reachable == true));
}

void set_reorder(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->reorder = true;
}
void unset_reorder(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->reorder = false;
}
boolean opt_reorder(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->reorder == true));
}

void set_reorder_method(options_ptr opt, unsigned int method)
{
  nusmv_assert(opt != NULL);
  opt->reorder_method = method;
}
unsigned int get_reorder_method(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->reorder_method);
}

void set_dynamic_reorder(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->dynamic_reorder = true;
}
void unset_dynamic_reorder(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->dynamic_reorder = false;
}
boolean opt_dynamic_reorder(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->dynamic_reorder == true));
}

void set_batch(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->batch = true;
}
void unset_batch(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->batch = false;
}
boolean opt_batch(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->batch == true));
}

void set_partition_method(options_ptr opt, const TransType method)
{
  nusmv_assert(opt != NULL);
  opt->partition_method = method;
}
TransType get_partition_method(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->partition_method);
}
void set_monolithic(options_ptr opt)
{
  set_partition_method(opt, TRANS_TYPE_MONOLITHIC);
}
void set_conj_partitioning(options_ptr opt)
{
  set_partition_method(opt, TRANS_TYPE_THRESHOLD);
}
void reset_partitioning_method(options_ptr opt)
{
  set_partition_method(opt, TRANS_TYPE_MONOLITHIC);
}
void set_iwls95cp_partitioning(options_ptr opt)
{
  set_partition_method(opt, TRANS_TYPE_IWLS95);
}
boolean opt_monolithic(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->partition_method == TRANS_TYPE_MONOLITHIC));
}
boolean opt_conj_partitioning(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->partition_method == TRANS_TYPE_THRESHOLD));
}
boolean opt_iwls95cp_partitioning(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->partition_method == TRANS_TYPE_IWLS95));
}

void set_conj_part_threshold(options_ptr opt, int threshold)
{
  nusmv_assert(opt != NULL);
  opt->conj_part_threshold = threshold;
}
void reset_conj_part_threshold(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->conj_part_threshold = DEFAULT_CONJ_PART_THRESHOLD;
}
int get_conj_part_threshold(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->conj_part_threshold));
}

void set_image_cluster_size(options_ptr opt, int threshold)
{
  nusmv_assert(opt != NULL);
  opt->image_cluster_size = threshold;
}
void reset_image_cluster_size(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->image_cluster_size = DEFAULT_IMAGE_CLUSTER_SIZE;
}
int get_image_cluster_size(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->image_cluster_size));
}

void set_ignore_init_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_init_file = true;
}
void unset_ignore_init_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ignore_init_file = false;
}
boolean opt_ignore_init_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->ignore_init_file == true));
}

void set_ag_only(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ag_only = true;
}
void unset_ag_only(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->ag_only = false;
}
boolean opt_ag_only(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->ag_only == true));
}


void set_cone_of_influence(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->cone_of_influence = true;
}
void unset_cone_of_influence(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->cone_of_influence = false;
}
boolean opt_cone_of_influence(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->cone_of_influence == true));
}

void set_bmc_mode(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->bmc_mode = true;
}
void unset_bmc_mode(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->bmc_mode = false;
}
boolean opt_bmc_mode(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->bmc_mode == true));
}

void set_list_properties(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->list_properties = true;
}
void unset_list_properties(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->list_properties = false;
}
boolean opt_list_properties(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return((opt->list_properties == true));
}

void set_prop_no(options_ptr opt, int n)
{
  nusmv_assert(opt != NULL);
  nusmv_assert(n >= 0);
  opt->prop_no = n;
}
int get_prop_no(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->prop_no);
}

void print_partition_method (FILE * output_stream)
{
  fprintf(output_stream, "Monolithic, Threshold, Iwls95CP");
}

/* BMC Stuff */
char* get_bmc_dimacs_filename(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return (opt->bmc_dimacs_filename);
}
void set_bmc_dimacs_filename(options_ptr opt, char * str)
{
  nusmv_assert(opt != NULL);
  opt->bmc_dimacs_filename = util_strsav(str);
}
char* get_bmc_invar_dimacs_filename(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return (opt->bmc_invar_dimacs_filename);
}
void set_bmc_invar_dimacs_filename(options_ptr opt, char * str)
{
  nusmv_assert(opt != NULL);
  opt->bmc_invar_dimacs_filename = util_strsav(str);
}
int get_bmc_pb_length(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return opt->bmc_pb_length;
}

void set_bmc_pb_length(options_ptr opt, const int k)
{
  nusmv_assert(opt != NULL);
  opt->bmc_pb_length = k;
}

const char* get_bmc_pb_loop(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return opt->bmc_pb_loop;
}

void set_bmc_pb_loop(options_ptr opt, const char* loop)
{
  nusmv_assert(opt != NULL);

  /* Frees the previuosly allocated string (bmc_pb_loop must be 
     initially NULL): */
  if (opt->bmc_pb_loop != NULL) FREE(opt->bmc_pb_loop);
  opt->bmc_pb_loop = util_strsav((char*)loop);
}

const char* get_bmc_invar_alg(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return opt->bmc_invar_alg;
}

void set_bmc_invar_alg(options_ptr opt, const char* alg)
{
  nusmv_assert(opt != NULL);
  opt->bmc_invar_alg = alg; /* a string alg should not be deleted or 
			       modified outside */
}

#if HAVE_INCREMENTAL_SAT
const char* get_bmc_inc_invar_alg(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return opt->bmc_inc_invar_alg;
}

void set_bmc_inc_invar_alg(options_ptr opt, const char* alg)
{
  nusmv_assert(opt != NULL);
  opt->bmc_inc_invar_alg = alg;; /* a string alg should not be deleted or 
			       modified outside */
}
#endif

void set_bmc_optimized_tableau(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->bmc_optimized_tableau = 1;
}

void unset_bmc_optimized_tableau(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->bmc_optimized_tableau = 0;
}

boolean opt_bmc_optimized_tableau(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return (opt->bmc_optimized_tableau == 1);
}

void set_bmc_force_pltl_tableau(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->bmc_force_pltl_tableau = 1;
}

void unset_bmc_force_pltl_tableau(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->bmc_force_pltl_tableau = 0;
}

boolean opt_bmc_force_pltl_tableau(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return (opt->bmc_force_pltl_tableau == 1);
}

void set_sat_solver(options_ptr opt, const char* satSolver)
{
  const char* normalizedSatSolver = Sat_NormalizeSatSolverName(satSolver);
  nusmv_assert(normalizedSatSolver != NULL);
  opt->sat_solver = normalizedSatSolver;
}

const char* get_sat_solver(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return (opt->sat_solver);  
}

boolean set_default_trace_plugin(options_ptr opt, int plugin)
{
  nusmv_assert(opt != NULL);
  return TracePkg_set_default_trace_plugin(plugin);
}

int get_default_trace_plugin(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return TracePkg_get_default_trace_plugin();
}

/* Only for testing purpose right now */
void set_iwls95_preorder(options_ptr opt)
{
   opt->preorder = true;
}

void unset_iwls95_preorder(options_ptr opt)
{
   opt->preorder = false;
}

boolean opt_iwls95_preorder(options_ptr opt)
{
  return(opt->preorder);
}

void set_affinity(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->affinity = true;
}

void unset_affinity(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->affinity = false;
}

boolean opt_affinity(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->affinity);
}

void set_counter_examples(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->counter_examples = true;
}

void unset_counter_examples(options_ptr opt) 
{
  nusmv_assert(opt != NULL);
  opt->counter_examples = false;
}

boolean opt_counter_examples(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return opt->counter_examples;
}

void set_on_failure_script_quits(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->on_failure_script_quits = true;
}

void unset_on_failure_script_quits(options_ptr opt) 
{
  nusmv_assert(opt != NULL);
  opt->on_failure_script_quits = false;
}

boolean opt_on_failure_script_quits(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return opt->on_failure_script_quits;
}

void set_write_order_dumps_bits(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->write_order_dumps_bits = true;
}

void unset_write_order_dumps_bits(options_ptr opt) 
{
  nusmv_assert(opt != NULL);
  opt->write_order_dumps_bits = false;
}

boolean opt_write_order_dumps_bits(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return opt->write_order_dumps_bits;
}

void set_use_fair_states(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->use_fair_states = true;
}

void unset_use_fair_states(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->use_fair_states = false;
}

boolean opt_use_fair_states(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->use_fair_states);
}

void set_use_reachable_states(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->use_reachable_states = true;
}

void unset_use_reachable_states(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->use_reachable_states = false;
}

boolean opt_use_reachable_states(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->use_reachable_states);
}

/* SA Stuff */
#if HAVE_SA
void set_sa_stsa_fm_mapping_file(options_ptr opt, char * str)
{
  nusmv_assert(opt != NULL);
  if (util_is_string_null(str)) opt->sa_stsa_fm_mapping_file = NIL(char);
  else opt->sa_stsa_fm_mapping_file = util_strsav(str);
}
void reset_sa_stsa_fm_mapping_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  opt->sa_stsa_fm_mapping_file = DEFAULT_SA_STSA_FM_MAPPING_FILE;
}
char * get_sa_stsa_fm_mapping_file(options_ptr opt)
{
  nusmv_assert(opt != NULL);
  return(opt->sa_stsa_fm_mapping_file);
}
#endif

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis          [Sets an environment variable]

  CommandName       [set]

  CommandSynopsis   [Sets an environment variable]

  CommandArguments  [\[-h\] \[&lt;name&gt;\] \[&lt;value&gt;\]] 

  CommandDescription [ A variable environment is maintained by the command 
  interpreter.
  The "set" command sets a variable to a particular value, and the
  "unset" command removes the definition of a variable.
  If "set" is given no arguments, it prints the current value of all variables.<p>

  Command options:<p>
  <dl> <dt> -h
  <dd> Prints the command usage.
  </dl>
  <dl> <dt> &lt;name&gt;
  <dd> Variable name
  </dl>
  <dl> <dt> &lt;value&gt;
  <dd> Value to be assigned to the variable.
  </dl>

 <p>
  Interpolation of variables is allowed when using the set command. The
  variables are referred to with the prefix of '$'. So for example, 
  what follows can be done to check the value of a set variable:<br>
  <code>
  NuSMV> set foo bar<br>
  NuSMV> echo $foo<br>
  bar <br>
  </code>
  
  The last line "bar" will be the output produced by NuSMV.<p>

  Variables can be extended by using the character ':' to concatenate
  values. For example: <br>
  <code>
  NuSMV> set foo bar<br>
  NuSMV> set foo $foo:foobar<br>
  NuSMV> echo $foo<br>
  bar:foobar<br>
  </code>
  The variable <code> foo </code> is extended with the value <code>
  foobar </code>. <p>
  
  Whitespace characters may be present within quotes. However, variable
  interpolation lays the restriction that the characters ':' and '/' may
  not be used within quotes. This is to allow for recursive interpolation.
  So for example, the following is allowed<br>
  <code>
  NuSMV> set "foo bar" this<br>
  NuSMV> echo $"foo bar"<br>
  this <br>
  </code> 
  The last line will be the output produced by NuSMV. <br>
  But in the following, the  value of the variable <code> foo/bar </code>
  will not be interpreted correctly:<p>
  <code>
  NuSMV> set "foo/bar" this<br>
  NuSMV> echo $"foo/bar"<br>
  foo/bar<br>
  </code>
  If a variable is not set by the "set" command, then the variable is returned
  unchanged.
  <p>

  Different commands use environment information for different purposes.
  The command interpreter makes use of the following parameters:<p>

 <dl>
 <dt><b>autoexec</b>
 <dd>     Defines a command string to be automatically executed after every 
           command processed by the command interpreter.
           This is useful for things like timing commands, or tracing the 
           progress of optimization.
</dl>


 <dl><dt><b>open_path</b>
 <dd>      "open_path" (in analogy to the shell-variable PATH) is a list of
           colon-separated strings giving directories to be searched whenever
           a file is opened for read.  Typically the current directory (.) is
           the first item in this list. The standard system library (typically 
           $NuSMV_LIBRARY_PATH) is always implicitly appended to the current 
           path.
           This provides a convenient short-hand mechanism for reaching 
           standard library files.
 </dl>
 <dl><dt> <b>nusmv_stderr </b>
 <dd>   Standard error (normally stderr) can be re-directed to a file
           by setting the variable nusmv_stderr.
 </dl>           

 <dl><dt>  <b>nusmv_stdout</b>
 <dd>           Standard output (normally stdout) can be re-directed to a file
           by setting the variable nusmv_stdout.
 </dl>           
  ]

  SideEffects        []

  SeeAlso            [unset]

******************************************************************************/
int CommandSetVariable(int  argc, char ** argv)
{
  char *flag_value, *key, *value;
  avl_generator *gen;
  int c;
  
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch(c) {
      case 'h':
        goto usage;
        break;
      default:
        goto usage;
    }
  }
  if (argc == 0 || argc > 3) {
    goto usage ;
  }
  else if (argc == 1) {
    fprintf(nusmv_stdout, "The user variables are:\n");
    avl_foreach_item(cmdFlagTable, gen, AVL_FORWARD, &key, &value) {
      fprintf(nusmv_stdout, "%s\t\t\t\"%s\"\n", key, value);
    }
    /* fprintf(nusmv_stdout, "The internal variables are:\n"); */
    print_options(options, nusmv_stdout);
    return 0;
  }
  else {
    key = argv[1];
    if (avl_delete(cmdFlagTable, &key, &value)) {
      FREE(key);
      FREE(value);
    }
    flag_value = argc == 2 ? util_strsav("") : util_strsav(argv[2]);
    if (strcmp(argv[1], "nusmv_stdout") == 0) {
      if (nusmv_stdout != stdout) {
        (void) fclose(nusmv_stdout);
      }
      if (strcmp(flag_value, "") == 0) {
        flag_value = "-";
      }
      nusmv_stdout = Cmd_FileOpen(flag_value, "w", NIL(char *), 0);
      if (nusmv_stdout == NULL) {
        nusmv_stdout = stdout;
      }
#if HAVE_SETVBUF
# if SETVBUF_REVERSED      
      setvbuf(nusmv_stdout, _IOLBF, (char *) NULL, 0);
# else 
      setvbuf(nusmv_stdout, (char *) NULL, _IOLBF, 0);
# endif
#endif      
    }
    if (strcmp(argv[1], "nusmv_stderr") == 0) {
      if (nusmv_stderr != stderr) {
        (void) fclose(nusmv_stderr);
      }
      if (strcmp(flag_value, "") == 0) {
        flag_value = "-";
      }
      nusmv_stderr = Cmd_FileOpen(flag_value, "w", NIL(char *), 0);
      if (nusmv_stderr == NULL) {
        nusmv_stderr = stderr;
      }
#if HAVE_SETVBUF
# if SETVBUF_REVERSED
      setvbuf(nusmv_stderr, _IOLBF, (char *) NULL, 0);
# else
      setvbuf(nusmv_stderr, (char *) NULL, _IOLBF, 0);
# endif
#endif      
    }
    if (strcmp(argv[1], "history") == 0) {
      if (nusmv_historyFile != NIL(FILE)) {
        (void) fclose(nusmv_historyFile);
      }
      if (strcmp(flag_value, "") == 0) {
        nusmv_historyFile = NIL(FILE);
      }
      else {
        nusmv_historyFile = Cmd_FileOpen(flag_value, "w", NIL(char *), 0);
        if (nusmv_historyFile == NULL) {
          nusmv_historyFile = NIL(FILE);
        }
      }
    }
    if (strcmp(argv[1], PROGRAM_NAME) == 0) {
      set_pgm_name(options, flag_value);
      return 0;
    }
    if (strcmp(argv[1], INPUT_FILE) == 0) {
      if (cmp_struct_get_read_model(cmps)) {
        fprintf(nusmv_stdout, "***** Warning: a model is already loaded -- input_file not changed.\n");
        fprintf(nusmv_stdout, "***** The model should be reset (e.g., using command \"reset\")\n");
        fprintf(nusmv_stdout, "***** before the input_file can be changed.\n");
        return 1;
      }
      if (util_is_string_null(flag_value)) set_input_file(options, NULL);
      else set_input_file(options, util_strsav(flag_value));
      return 0;
    }
    if (strcmp(argv[1], INPUT_ORDER_FILE) == 0) {
      if (util_is_string_null(flag_value)) set_input_order_file(options, NULL);
      else set_input_order_file(options, util_strsav(flag_value));
      return 0;
    }
    if (strcmp(argv[1], OUTPUT_ORDER_FILE) == 0) {
      if (util_is_string_null(flag_value)) {
        set_output_order_file(options, NULL);
      }
      else { 
        set_output_order_file(options, util_strsav(flag_value));
      }
      return 0;
    }
    if (strcmp(argv[1], OUTPUT_FLATTEN_MODEL_FILE) == 0) {
      if (util_is_string_null(flag_value)) {
        set_output_flatten_model_file(options, NULL);
      }
      else { 
        set_output_flatten_model_file(options, util_strsav(flag_value));
      }
      return 0;
    }
    if (strcmp(argv[1], OUTPUT_BOOLEAN_MODEL_FILE) == 0) {
      if (util_is_string_null(flag_value)) {
        set_output_boolean_model_file(options, NULL);
      }
      else { 
        set_output_boolean_model_file(options, util_strsav(flag_value));
      }
      return 0;
    }

    if (strcmp(argv[1], VERBOSE_LEVEL) == 0) {
      char *err_occ[1];
      int cur_verbose;

      err_occ[0] = "";
      cur_verbose = strtol(flag_value,err_occ, 10);
      if (strcmp(err_occ[0], "") != 0) {
        fprintf(nusmv_stderr, "Error: \"%s\" is not a valid value for the \"%s\" option.\n", err_occ[0], VERBOSE_LEVEL);
        return 1;
      }
      set_verbose_level(options, cur_verbose);
      return 0;
    }
    if (strcmp(argv[1], RUN_CPP) == 0) {
      fprintf(nusmv_stderr, "Error: the \"%s\" option is no longer supported - use \"%s cpp\" instead.\n", RUN_CPP, PP_LIST);
      return 1;
    }
    if (strcmp(argv[1], PP_LIST) == 0) {
      if (argc == 2) {
	fprintf(nusmv_stderr, "Error: the \"%s\" option requires an argument.\n", PP_LIST);
	return 1;
      }
      set_pp_list(options, flag_value);
      return 0;
    }
    if (strcmp(argv[1], SHOWN_STATES) == 0) {
      char *err_occ[1];
      int cur_shown_states;

      err_occ[0] = "";
      cur_shown_states = strtol(flag_value, err_occ, 10);
      if (strcmp(err_occ[0], "") != 0) {
        fprintf(nusmv_stderr, "Error: \"%s\" is not a valid value for the \"%s\" option.\n", err_occ[0], SHOWN_STATES);
        return 1;
      }
      if (cur_shown_states > MAX_SHOWN_STATES) {
        fprintf(nusmv_stderr,
                "Error: \"%d\" is not a valid value for the \"%s\" option.\n", cur_shown_states, SHOWN_STATES);
        fprintf(nusmv_stderr,
                "Option \"%s\" has been set to highest possible value.\n", SHOWN_STATES);
        set_shown_states_level(options, MAX_SHOWN_STATES);
        return 1;
      }
      if (cur_shown_states < 1) {
        fprintf(nusmv_stderr, "Error: \"%d\" is not a valid value for the \"%s\" option.\n", cur_shown_states, SHOWN_STATES);
        fprintf(nusmv_stderr, "Option \"%s\" has been set to the default value.\n", SHOWN_STATES);
        set_shown_states_level(options, 25);
        return 1;
      }
      set_shown_states_level(options, cur_shown_states);
      return 0;
    }
    if (strcmp(argv[1], IGNORE_SPEC) == 0) {
      set_ignore_spec(options);
      return 0;
    }
    if (strcmp(argv[1], IGNORE_COMPUTE) == 0) {
      set_ignore_compute(options);
      return 0;
    }
    if (strcmp(argv[1], IGNORE_LTLSPEC) == 0) {
      set_ignore_ltlspec(options);
      return 0;
    }
    if (strcmp(argv[1], IGNORE_PSLSPEC) == 0) {
      set_ignore_pslspec(options);
      return 0;
    }
    if (strcmp(argv[1], OPT_CHECK_FSM) == 0) {
      set_check_fsm(options);
      return 0;
    }
    if (strcmp(argv[1], IGNORE_INVAR) == 0) {
      set_ignore_invar(options);
      return 0;
    }
    if (strcmp(argv[1], FORWARD_SEARCH) == 0) {
      set_forward_search(options);
      /* if this option is used, also their use is enabled */
      set_use_reachable_states(options);
      return 0;
    }
    if (strcmp(argv[1], PRINT_REACHABLE) == 0) {
      set_print_reachable(options);
      /* if this option is used, also their use is enabled */
      set_use_reachable_states(options);
      return 0;
    }
    if (strcmp(argv[1], ENABLE_REORDER) == 0) {
      set_reorder(options);
      return 0;
    }
    if (strcmp(argv[1], CONE_OF_INFLUENCE) == 0) {
      set_cone_of_influence(options);
      return 0;
    }
    if (strcmp(argv[1], REORDER_METHOD) == 0) {
      unsigned int method = StringConvertToDynOrderType(flag_value);

      if (method == REORDER_NONE) {
        fprintf(nusmv_stderr, "Error: \"%s\" is not a valid ordering method.\n", flag_value);
        return 1;
      }
      dd_autodyn_enable(dd_manager, method);
      if (!opt_dynamic_reorder(options)) {
        dd_autodyn_disable(dd_manager);
      }
      set_reorder_method(options, method);
      return 0;
    }
    if (strcmp(argv[1], DYNAMIC_REORDER) == 0) {
      int current_ordering_method = dd_get_ordering_method(dd_manager);

      set_dynamic_reorder(options);
      dd_autodyn_enable(dd_manager, current_ordering_method);
      return 0;
    }
    if (strcmp(argv[1], PARTITION_METHOD) == 0) {
      if (flag_value == NULL) {
        fprintf(nusmv_stderr, "The option \"%s\" requires an argument.\n", PARTITION_METHOD);
        return 1;
      }
      else {
        if (TransType_from_string(flag_value) != TRANS_TYPE_INVALID) {
          set_partition_method(options, TransType_from_string(flag_value));
          return 0;
        } else {
          fprintf(nusmv_stderr, "Error: \"%s\" is not a valid value for the \"%s\" option.\n", flag_value, PARTITION_METHOD);
	  return 1;
        }
      }
    }
    if (strcmp(argv[1], CONJ_PART_THRESHOLD) == 0) {
      char *err_occ[1];
      int cur_rt;
  
      err_occ[0] = "";
      cur_rt = strtol(flag_value, err_occ, 10);
      if (strcmp(err_occ[0], "") != 0) {
        fprintf(nusmv_stderr, "Error: \"%s\" is not a valid value for the \"%s\" option.\n", err_occ[0], CONJ_PART_THRESHOLD);
        return 1;
      }
      set_conj_part_threshold(options, cur_rt);
      return 0;
    }
    if (strcmp(argv[1], IMAGE_CLUSTER_SIZE) == 0) {
      char *err_occ[1];
      int cur_rt;
  
      err_occ[0] = "";
      cur_rt = strtol(flag_value, err_occ, 10);
      if (strcmp(err_occ[0], "") != 0) {
        fprintf(nusmv_stderr, "Error: \"%s\" is not a valid value for the \"%s\" option.\n", err_occ[0], IMAGE_CLUSTER_SIZE);
        return 1;
      }
      set_image_cluster_size(options, cur_rt);
      return 0;
    }
    if (strcmp(argv[1], IGNORE_INIT_FILE) == 0) {
      set_ignore_init_file(options);
      return 0;
    }
    if (strcmp(argv[1], AG_ONLY_SEARCH) == 0) {
      set_ag_only(options);
      return 0;
    }
    if (strcmp(argv[1], BMC_DIMACS_FILENAME) == 0) {
      set_bmc_dimacs_filename(options, util_strsav(flag_value));
      return 0;
    }
    if (strcmp(argv[1], BMC_INVAR_DIMACS_FILENAME) == 0) {
      set_bmc_invar_dimacs_filename(options, util_strsav(flag_value));
      return 0;
    }

    if (strcmp(argv[1], BMC_PB_LENGTH) == 0) {
      int length = atoi(flag_value);
      if (length < 0) {
        fprintf(nusmv_stderr, 
             "It is not possible to set the problem length to this value.\n");
        return 1;
      }
      else {
        outcome res;
        int loop = 
          Bmc_Utils_RelLoop2AbsLoop(Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), &res), 
                                    length);
        if ((res == SUCCESS) && Bmc_Utils_Check_k_l(length, loop) == SUCCESS) {
          set_bmc_pb_length(options, atoi(flag_value));
        }
        else {
          fprintf( nusmv_stderr, 
                   "Error: value %s is not compatible with current loopback value set to %s.\n", 
                   flag_value, get_bmc_pb_loop(options));
          return 1;
        }
      }
      return 0;
    }

    if (strcmp(argv[1], BMC_PB_LOOP) == 0) {
      outcome res;
      int length = get_bmc_pb_length(options);
      int loop = Bmc_Utils_RelLoop2AbsLoop(Bmc_Utils_ConvertLoopFromString(flag_value, &res), length);
      if ((res == SUCCESS) && Bmc_Utils_Check_k_l(length, loop) == SUCCESS) {
        set_bmc_pb_loop(options, flag_value);
      }
      else {
        fprintf(nusmv_stderr, 
                "\"%s\" is not a valid value for the bmc_loopback variable.\n\n",
                flag_value);
        return 1;
      }
      return 0;
    }

    if (strcmp(argv[1], BMC_INVAR_ALG) == 0) {
      if (flag_value == NULL) {
        fprintf(nusmv_stderr, "The option \"%s\" requires an argument.\n",
		BMC_INVAR_ALG);
        return 1;
      }
      /* here is a short term solution, in future proper function
         should be invoked from BMC model to get existing INVAR
         (non-incr) algorithms */

      if (strcasecmp(BMC_INVAR_ALG_CLASSIC, flag_value) == 0) {
	set_bmc_invar_alg(options, BMC_INVAR_ALG_CLASSIC);
      } else if (strcasecmp(BMC_INVAR_ALG_EEN_SORENSSON, flag_value) == 0) {
	set_bmc_invar_alg(options, BMC_INVAR_ALG_EEN_SORENSSON);
      } else {
          fprintf(nusmv_stderr, 
                  "Error: \"%s\" is not a valid value for variable \"%s\".\n", 
		  flag_value,
		  BMC_INVAR_ALG);
	  fprintf(nusmv_stderr,
		"The available INVAR solving (non-incremental) algorithms are:\n");
	  fprintf(nusmv_stderr,
		  BMC_INVAR_ALG_CLASSIC " " BMC_INVAR_ALG_EEN_SORENSSON "\n");
	  return 1;
      }
      return 0;
    }

#if HAVE_INCREMENTAL_SAT
    if (strcmp(argv[1], BMC_INC_INVAR_ALG) == 0) {
      if (flag_value == NULL) {
        fprintf(nusmv_stderr, "The option \"%s\" requires an argument.\n",
		BMC_INC_INVAR_ALG);
        return 1;
      }
      /* here is a short term solution, in future proper function should be 
	 invoked from BMC model to get existing INVAR (incremental) algorithms */

      if (strcasecmp(BMC_INC_INVAR_ALG_DUAL, flag_value) == 0) {
	set_bmc_inc_invar_alg(options, BMC_INC_INVAR_ALG_DUAL);
      } else if (strcasecmp(BMC_INC_INVAR_ALG_ZIGZAG, flag_value) == 0) {
	set_bmc_inc_invar_alg(options,BMC_INC_INVAR_ALG_ZIGZAG);
      } else {
          fprintf(nusmv_stderr, 
                  "Error: \"%s\" is not a valid value for variable \"%s\".\n", 
		  flag_value,
		  BMC_INC_INVAR_ALG);
	  fprintf(nusmv_stderr,
		"The available INVAR solving (incremental) algorithms are: \n");
	  fprintf(nusmv_stderr, 
		  BMC_INC_INVAR_ALG_DUAL " " BMC_INC_INVAR_ALG_ZIGZAG "\n");
	  return 1;
      }

      return 0;
    }
#endif

    if (strcmp(argv[1], BMC_OPTIMIZED_TABLEAU) == 0) {
      set_bmc_optimized_tableau(options);
      return 0;
    }
    if (strcmp(argv[1], BMC_FORCE_PLTL_TABLEAU) == 0) {
      set_bmc_force_pltl_tableau(options);
      return 0;
    }
    
    if (strcmp(argv[1], A_SAT_SOLVER) == 0) {
      const char* satSolver;
      if (flag_value == NULL) {
        fprintf(nusmv_stderr, "The option \"%s\" requires an argument.\n",
		A_SAT_SOLVER);
        return 1;
      }
      satSolver = Sat_NormalizeSatSolverName(flag_value);
      if (satSolver != 0) set_sat_solver(options, satSolver);
      else {
        fprintf(nusmv_stderr, "Error: supplied an invalid SAT solver.\n");
        Sat_PrintAvailableSolvers(nusmv_stderr);
        return 1;       
      }
      return 0;
    }

    if (strcmp(argv[1], IWLS95_PREORDER) == 0) {
      set_iwls95_preorder(options);
      return 0;
    }
    if (strcmp(argv[1], AFFINITY_CLUSTERING) == 0) {
      set_affinity(options);
      return 0;
    }
    if (strcmp(argv[1], COUNTER_EXAMPLES) == 0) {
      set_counter_examples(options);
      return 0;
    }
    if (strcmp(argv[1], ON_FAILURE_SCRIPT_QUITS) == 0) {
      set_on_failure_script_quits(options);
      return 0;
    }
    if (strcmp(argv[1],  WRITE_ORDER_DUMPS_BITS) == 0) {
      set_write_order_dumps_bits(options);
      return 0;
    }

    if (strcmp(argv[1], DEFAULT_TRACE_PLUGIN) == 0) {
      char *err_occ[1];
      int dp;
  
      err_occ[0] = "";
      dp = strtol(flag_value, err_occ, 10);
      if (strcmp(err_occ[0], "") != 0) {
          fprintf(nusmv_stderr, 
                  "Error: \"%s\" is not a valid value for variable"
		  "\"default_trace_plugin\".\n", 
                  err_occ[0]);
        return 1;
      }
      if (!set_default_trace_plugin(options, dp)) return 0;
      else return 1;      
    }

    /* SA Stuff */
#if HAVE_SA
    if (strcmp(argv[1], SA_STSA_FM_MAPPING_FILE) == 0) {
      if (util_is_string_null(flag_value))
	set_sa_stsa_fm_mapping_file(options, NULL);
      else
	set_sa_stsa_fm_mapping_file(options, util_strsav(flag_value));
      return 0;
    }
#endif

    /* None of the internal options have been recognized */
    (void) avl_insert(cmdFlagTable, util_strsav(argv[1]), flag_value);
    return 0;
  }

  usage:
      (void) printf("usage: set [-h] [name [value]]\n");
      fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
      return 1;
}


/**Function********************************************************************

  Synopsis          [Unsets an environment variable]

  CommandName       [unset]

  CommandSynopsis   [Unsets an environment variable]

  CommandArguments  [\[-h\] &lt;variables&gt;] 

  CommandDescription [A variable environment is maintained by the command 
  interpreter. 
  The "set" command sets a variable to a particular value, and the
  "unset" command removes the definition of a variable. <p>
  Command options:<p>
  <dl><dt> -h
  <dd> Prints the command usage.
  </dl>
  <dl><dt> &lt;variables&gt;
  <dd> Variables to be unset
  </dl>
  ]

  SideEffects        []

  SeeAlso            [set]

******************************************************************************/
int CommandUnsetVariable(int  argc, char ** argv)
{
  int i;
  char *key, *value;
  int c;
  
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch(c) {
      case 'h':
        goto usage;
        break;
      default:
        goto usage;
    }
  }
  
  if (argc < 2) {
    goto usage;
  }
  if (strcmp(argv[1], PROGRAM_NAME) == 0) {
    reset_pgm_name(options);
    return 0;
  }
  if (strcmp(argv[1], INPUT_FILE) == 0) {
    if (cmp_struct_get_read_model(cmps)) {
      fprintf(nusmv_stdout, "***** Warning: a model is already loaded -- input_file not changed.\n");
      fprintf(nusmv_stdout, "***** The model should be reset (e.g., using command \"reset\")\n");
      fprintf(nusmv_stdout, "***** before the input_file can be changed.\n");
      return 1;
    }
    reset_input_file(options);
    return 0;
  }
  if (strcmp(argv[1], INPUT_ORDER_FILE) == 0) {
    reset_input_order_file(options);
    return 0;
  }
  if (strcmp(argv[1], OUTPUT_ORDER_FILE) == 0) {
    set_output_order_file(options, NULL);
    return 0;
  }
  if (strcmp(argv[1], OUTPUT_FLATTEN_MODEL_FILE) == 0) {
    reset_output_flatten_model_file(options);
    return 0;
  }
  if (strcmp(argv[1], OUTPUT_BOOLEAN_MODEL_FILE) == 0) {
    reset_output_boolean_model_file(options);
    return 0;
  }
  if (strcmp(argv[1], VERBOSE_LEVEL) == 0) {
    set_verbose_level(options, 0);
    return 0;
  }
  if (strcmp(argv[1], RUN_CPP) == 0) {
    fprintf(nusmv_stderr, "Error: the \"unset %s\" option is no longer supported - use set %s \"\" instead.\n", RUN_CPP, PP_LIST);
    return 1;
  }
  if (strcmp(argv[1], SHOWN_STATES) == 0) {
    set_shown_states_level(options, DEFAULT_SHOWN_STATES);
    return 0;
  }
  if (strcmp(argv[1], IGNORE_SPEC) == 0) {
    unset_ignore_spec(options);
    return 0;
  }
  if (strcmp(argv[1], IGNORE_COMPUTE) == 0) {
    unset_ignore_compute(options);
    return 0;
  }
  if (strcmp(argv[1], IGNORE_LTLSPEC) == 0) {
    unset_ignore_ltlspec(options);
    return 0;
  }
  if (strcmp(argv[1], IGNORE_PSLSPEC) == 0) {
    unset_ignore_pslspec(options);
    return 0;
  }
  if (strcmp(argv[1], OPT_CHECK_FSM) == 0) {
    unset_check_fsm(options);
    return 0;
  }
  if (strcmp(argv[1], IGNORE_INVAR) == 0) {
    unset_ignore_invar(options);
    return 0;
  }
  if (strcmp(argv[1], FORWARD_SEARCH) == 0) {
    unset_forward_search(options);
    return 0;
  }
  if (strcmp(argv[1], PRINT_REACHABLE) == 0) {
    unset_print_reachable(options);
    return 0;
  }
  if (strcmp(argv[1], ENABLE_REORDER) == 0) {
    unset_reorder(options);
    return 0;
  }
  if (strcmp(argv[1], CONE_OF_INFLUENCE) == 0) {
    unset_cone_of_influence(options);
    return 0;
  }
  if (strcmp(argv[1], DYNAMIC_REORDER) == 0) {
    unset_dynamic_reorder(options);
    dd_autodyn_disable(dd_manager);
    return 0;
  }
  if (strcmp(argv[1], PARTITION_METHOD) == 0) {
    set_monolithic(options);
    return 0;
  }
  if (strcmp(argv[1], CONJ_PART_THRESHOLD) == 0) {
    reset_conj_part_threshold(options);
    return 0;
  }
  if (strcmp(argv[1], IMAGE_CLUSTER_SIZE) == 0) {
    reset_image_cluster_size(options);
    return 0;
  }
  if (strcmp(argv[1], IGNORE_INIT_FILE) == 0) {
    unset_ignore_init_file(options);
    return 0;
  }
  if (strcmp(argv[1], AG_ONLY_SEARCH) == 0) {
    unset_ag_only(options);
    return 0;
  }
  if (strcmp(argv[1], IWLS95_PREORDER) == 0) {
    unset_iwls95_preorder(options);
    return 0;
  }
  if (strcmp(argv[1], AFFINITY_CLUSTERING) == 0) {
    unset_affinity(options);
    return 0;
  }
  if (strcmp(argv[1], BMC_INVAR_ALG) == 0) {
    set_bmc_invar_alg(options, DEFAULT_BMC_INVAR_ALG);
    return 0;
  }
#if HAVE_INCREMENTAL_SAT
  if (strcmp(argv[1], BMC_INC_INVAR_ALG) == 0) {
    set_bmc_inc_invar_alg(options, DEFAULT_BMC_INC_INVAR_ALG);
    return 0;
  }
#endif
  if (strcmp(argv[1], BMC_OPTIMIZED_TABLEAU) == 0) {
    unset_bmc_optimized_tableau(options);
    return 0;
  }
  if (strcmp(argv[1], BMC_FORCE_PLTL_TABLEAU) == 0) {
    unset_bmc_force_pltl_tableau(options);
    return 0;
  }
  if (strcmp(argv[1], A_SAT_SOLVER) == 0) {
    set_sat_solver(options, DEFAULT_SAT_SOLVER);
    return 0;
  }
  if (strcmp(argv[1], COUNTER_EXAMPLES) == 0) {
    unset_counter_examples(options);
    return 0;
  }
  if (strcmp(argv[1], ON_FAILURE_SCRIPT_QUITS) == 0) {
    unset_on_failure_script_quits(options);
    return 0;
  }   
  if (strcmp(argv[1],  WRITE_ORDER_DUMPS_BITS) == 0) {
    unset_write_order_dumps_bits(options);
    return 0;
  }

    /* SA Stuff */
#if HAVE_SA
  if (strcmp(argv[1], SA_STSA_FM_MAPPING_FILE) == 0) {
    reset_sa_stsa_fm_mapping_file(options);
    return 0;
  }
#endif

  /* End of internal variables */
  for(i = 1; i < argc; i++) {
    key = argv[i];
    if (avl_delete(cmdFlagTable, &key, &value)) {
      FREE(key);
      FREE(value);
    }
  }
  return 0;


  usage:
    fprintf(nusmv_stderr, "usage: unset [-h] variables \n");
    fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
    return 1;  
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

static void print_options(options_ptr opt, FILE * fn)
{
  fprintf(fn, "%s\t\t\t\"%s\"\n", PROGRAM_NAME,
          (opt->pgm_name != (char *)NULL) 
          ? opt->pgm_name : OPT_USER_POV_NULL_STRING );
  fprintf(fn, "%s\t\t\t\"%s\"\n", INPUT_FILE,
          (opt->input_file != (char *)NULL) 
          ? opt->input_file : OPT_USER_POV_NULL_STRING );
  fprintf(fn, "%s\t\t\"%s\"\n",   INPUT_ORDER_FILE,
          (opt->input_order_file != (char *)NULL) 
          ? opt->input_order_file : OPT_USER_POV_NULL_STRING);
  fprintf(fn, "%s\t\t\"%s\"\n",   OUTPUT_ORDER_FILE, 
          (opt->output_order_file != (char*)NULL)
          ? opt->output_order_file : OPT_USER_POV_NULL_STRING);
  fprintf(fn, "%s\t\"%s\"\n",   OUTPUT_FLATTEN_MODEL_FILE, 
          (opt->output_flatten_model_file != (char*)NULL)
          ? opt->output_flatten_model_file : OPT_USER_POV_NULL_STRING);
  fprintf(fn, "%s\t\"%s\"\n",   OUTPUT_BOOLEAN_MODEL_FILE, 
          (opt->output_boolean_model_file != (char*)NULL)
          ? opt->output_boolean_model_file : OPT_USER_POV_NULL_STRING);

  fprintf(fn, "%s\t\t\t\"%d\"\n", VERBOSE_LEVEL, opt->verbose);
  fprintf(fn, "%s\t\t\t\t\"%s\"\n", PP_LIST, opt->pp_list);
  fprintf(fn, "%s\t\t\t\"%d\"\n", SHOWN_STATES, opt->shown_states);
  fprintf(fn, "%s\t\t\t\"%d\"\n", OPT_CHECK_FSM, opt->check_fsm);
  fprintf(fn, "%s\t\t\t\"%d\"\n", IGNORE_SPEC, opt->ignore_spec);
  fprintf(fn, "%s\t\t\t\"%d\"\n", IGNORE_COMPUTE, opt->ignore_compute);
  fprintf(fn, "%s\t\t\t\"%d\"\n", IGNORE_LTLSPEC, opt->ignore_ltlspec);
  fprintf(fn, "%s\t\t\t\"%d\"\n", IGNORE_PSLSPEC, opt->ignore_pslspec);
  fprintf(fn, "%s\t\t\t\"%d\"\n", IGNORE_INVAR, opt->ignore_invar);
  fprintf(fn, "%s\t\t\t\"%d\"\n", FORWARD_SEARCH, opt->forward_search);
  fprintf(fn, "%s\t\t\t\"%d\"\n", PRINT_REACHABLE, opt->print_reachable);
  fprintf(fn, "%s\t\t\t\"%d\"\n", ENABLE_REORDER, opt->reorder);
  fprintf(fn, "%s\t\t\t\"%s\"\n", REORDER_METHOD, DynOrderTypeConvertToString(opt->reorder_method));
  fprintf(fn, "%s\t\t\t\"%d\"\n", DYNAMIC_REORDER, opt->dynamic_reorder);
  fprintf(fn, "%s\t\t\"%s\"\n",   PARTITION_METHOD, TransType_to_string(opt->partition_method));
  fprintf(fn, "%s\t\t\"%d\"\n",   CONJ_PART_THRESHOLD, opt->conj_part_threshold);
  fprintf(fn, "%s\t\t\"%d\"\n",   IMAGE_CLUSTER_SIZE, opt->image_cluster_size);
  fprintf(fn, "%s\t\t\t\"%d\"\n", IWLS95_PREORDER, opt->preorder);
  fprintf(fn, "%s\t\t\t\"%d\"\n", AFFINITY_CLUSTERING, opt->affinity);
  fprintf(fn, "%s\t\t\"%d\"\n",   IGNORE_INIT_FILE, opt->ignore_init_file);
  fprintf(fn, "%s\t\t\t\"%d\"\n",  AG_ONLY_SEARCH, opt->ag_only);
  fprintf(fn, "%s\t\t\"%d\"\n",  CONE_OF_INFLUENCE, opt->cone_of_influence);
  /* BMC Stuff */
  fprintf(fn, "%s\t\t\"%s\"\n",   BMC_DIMACS_FILENAME, 
          (opt->bmc_dimacs_filename != (char*)NULL) 
          ? opt->bmc_dimacs_filename : OPT_USER_POV_NULL_STRING);
  fprintf(fn, "%s\t\"%s\"\n",   BMC_INVAR_DIMACS_FILENAME, 
          (opt->bmc_invar_dimacs_filename != (char*)NULL)
          ? opt->bmc_invar_dimacs_filename : OPT_USER_POV_NULL_STRING);
  fprintf(fn, "%s\t\t\t\"%d\"\n",   BMC_PB_LENGTH, opt->bmc_pb_length);
  fprintf(fn, "%s\t\t\t\"%s\"\n",   BMC_PB_LOOP, opt->bmc_pb_loop);

  fprintf(fn, "%s\t\t\t\"%s\"\n",   BMC_INVAR_ALG, opt->bmc_invar_alg);
#if HAVE_INCREMENTAL_SAT
  fprintf(fn, "%s\t\t\"%s\"\n",   BMC_INC_INVAR_ALG, opt->bmc_inc_invar_alg);
#endif

  fprintf(fn, "%s\t\t\"%d\"\n", BMC_OPTIMIZED_TABLEAU, 
          opt_bmc_optimized_tableau(opt));
  fprintf(fn, "%s\t\t\"%d\"\n", BMC_FORCE_PLTL_TABLEAU, 
          opt_bmc_force_pltl_tableau(opt));
  fprintf(fn, "%s\t\t\t\"%s\"\n", A_SAT_SOLVER,  
          get_sat_solver(opt));
  fprintf(fn, "%s\t\t\"%d\"\n", COUNTER_EXAMPLES,  
	  opt_counter_examples(opt));
  fprintf(fn, "%s\t\t\"%d\"\n", ON_FAILURE_SCRIPT_QUITS,  
	  opt_on_failure_script_quits(opt));
  fprintf(fn, "%s\t\t\"%d\"\n",  WRITE_ORDER_DUMPS_BITS,  
	  opt_write_order_dumps_bits(opt));
  fprintf(fn, "%s\t\t\"%d\"\n", DEFAULT_TRACE_PLUGIN,  
	  get_default_trace_plugin(opt));

#if HAVE_SA
  fprintf(fn, "%s\t\t\"%s\"\n", SA_STSA_FM_MAPPING_FILE,
          (opt->sa_stsa_fm_mapping_file != (char *)NULL) 
          ? opt->sa_stsa_fm_mapping_file : OPT_USER_POV_NULL_STRING );
#endif

}



/**Function********************************************************************

  Synopsis [Finds all preprocessor names occurring in the given string
  that are not actually available, and returns the set of the only
  available ones]

  Description        [Returned string must be freed]

  SideEffects        []

******************************************************************************/
static char* remove_non_existant_pps(const char* pp_list)
{
  char* paths;
  char* open_path = "";
  char* valid_pps;
  char* pp_list_copy;
  char* pp;
  int len;

  len = strlen(pp_list);

  valid_pps = ALLOC(char, len+1);
  nusmv_assert(valid_pps != (char*) NULL);
  valid_pps[0] = '\0';

  pp_list_copy = ALLOC(char, len+2);
  nusmv_assert(pp_list_copy != (char*) NULL);
  strncpy(pp_list_copy, pp_list, len+1); 
  pp_list_copy[len+1] = '\0'; /* additional '\0' for strtok below */

  /* gets an operating system variable PATH and NuSMV's variable open_path */
# if HAVE_GETENV
  paths = getenv("PATH");
# else
  paths = "."; /* default is the current dir */
# endif

  avl_lookup(cmdFlagTable, "open_path", &open_path);

  pp = strtok(pp_list_copy, " \t\n\r");
  while (pp != (char*) NULL) {
    char* pp_filename; 

    pp_filename = get_preprocessor_filename(pp);
    if ((pp_filename != (char*) NULL) &&
	( Utils_file_exists_in_paths(pp_filename, open_path, ":;") ||
          Utils_file_exists_in_paths(pp_filename, paths, ":;") )) {
      
      if (valid_pps[0] != '\0') strcat(valid_pps, " ");
      strcat(valid_pps, pp);
    }
    /* "strtok" is not safe and can be changed by other functions. 
       "pp + its length" is the pointer to the next string to be parsed.
    */
    pp = pp + strlen(pp) + 1; /* 1 is used to pass past the current '\0' */
    pp = strtok(pp, " \t\n\r");    
  } /* while loop */

  FREE(pp_list_copy);
  return valid_pps;
}
