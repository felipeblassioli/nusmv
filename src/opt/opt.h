/**CHeaderFile*****************************************************************

  FileName    [opt.h]

  PackageName [opt]

  Synopsis    [The option header file.]

  Description [This file conatins a data structure to manage all the
  command line options of the NuSMV system.]

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

  For more information on NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

  Revision    [$Id: opt.h,v 1.36.4.10.2.7 2005/11/15 15:02:02 nusmv Exp $]

******************************************************************************/

#ifndef __OPT_H__
#define __OPT_H__

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "utils/utils.h"
#include "sat/sat.h" /* for SAT Solver */
#include "trans/trans.h" /* for TransType */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define DEFAULT_PGM_NAME   (char *)NULL
#define DEFAULT_INPUT_FILE (char *)NULL
#define DEFAULT_INPUT_ORDER_FILE (char *)NULL
#define DEFAULT_OUTPUT_ORDER_FILE "temp.ord"
#define DEFAULT_CONJ_PART_THRESHOLD 1000
#define DEFAULT_IMAGE_CLUSTER_SIZE 1000
#define DEFAULT_SHOWN_STATES 25
/* maximum number of states shown during an interactive simulation step*/
#define MAX_SHOWN_STATES 65535
/* BMC stuff */
#define DEFAULT_DIMACS_FILENAME        "@f_k@k_l@l_n@n"
#define DEFAULT_INVAR_DIMACS_FILENAME  "@f_invar_n@n"
#define DEFAULT_BMC_PB_LENGTH     10
#define DEFAULT_BMC_PB_LOOP         Bmc_Utils_GetAllLoopbacksString()
#define DEFAULT_BMC_INVAR_ALG       "classic"
#define DEFAULT_BMC_INC_INVAR_ALG   "dual"
#define DEFAULT_BMC_OPTIMIZED_TABLEAU 1

#if HAVE_SOLVER_MINISAT
#define DEFAULT_SAT_SOLVER        "MiniSat"
#else
#define DEFAULT_SAT_SOLVER        "Sim"
#endif

#define DEFAULT_BMC_FORCE_PLTL_TABLEAU 0

#define OPT_USER_POV_NULL_STRING  "" /* user pov of the null string */

#if HAVE_SA
#define DEFAULT_SA_STSA_FM_MAPPING_FILE (char *)NULL
#endif


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct options_TAG*  options_ptr;


EXTERN void set_use_reachable_states ARGS((options_ptr));
EXTERN void unset_use_reachable_states ARGS((options_ptr));
EXTERN boolean opt_use_reachable_states ARGS((options_ptr));

EXTERN void set_use_fair_states ARGS((options_ptr));
EXTERN void unset_use_fair_states ARGS((options_ptr));
EXTERN boolean opt_use_fair_states ARGS((options_ptr));


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN options_ptr init_options ARGS((void));
EXTERN void init_options_cmd ARGS((void));
EXTERN void    set_pgm_name ARGS((options_ptr, char *));
EXTERN void    reset_pgm_name ARGS((options_ptr));
EXTERN char *  get_pgm_name ARGS((options_ptr));
EXTERN void    set_input_file ARGS((options_ptr, char *));
EXTERN void    reset_input_file ARGS((options_ptr));
EXTERN char *  get_input_file ARGS((options_ptr));
EXTERN void    set_input_order_file ARGS((options_ptr, char *));
EXTERN void    reset_input_order_file ARGS((options_ptr));
EXTERN char *  get_input_order_file ARGS((options_ptr));
EXTERN void    set_output_order_file ARGS((options_ptr, char *));
EXTERN void    reset_output_order_file ARGS((options_ptr));
EXTERN char *  get_output_order_file ARGS((options_ptr));
EXTERN boolean is_default_order_file ARGS((options_ptr opt));
EXTERN void    set_output_flatten_model_file ARGS((options_ptr, char *));
EXTERN void    reset_output_flatten_model_file ARGS((options_ptr));
EXTERN char *  get_output_flatten_model_file ARGS((options_ptr));
EXTERN void    set_output_boolean_model_file ARGS((options_ptr, char *));
EXTERN void    reset_output_boolean_model_file ARGS((options_ptr));
EXTERN char *  get_output_boolean_model_file ARGS((options_ptr));
EXTERN void    set_verbose_level ARGS((options_ptr, int));
EXTERN int     get_verbose_level ARGS((options_ptr));
EXTERN boolean opt_verbose_level_eq ARGS((options_ptr, int));
EXTERN boolean opt_verbose_level_gt ARGS((options_ptr, int));
EXTERN boolean opt_verbose_level_ge ARGS((options_ptr, int));
EXTERN boolean opt_verbose_level_lt ARGS((options_ptr, int));
EXTERN boolean opt_verbose_level_le ARGS((options_ptr, int));
EXTERN void    set_pp_list ARGS((options_ptr, char *));
EXTERN char *  get_pp_list ARGS((options_ptr));
EXTERN void    set_shown_states_level ARGS((options_ptr, int));
EXTERN int     opt_shown_states_level ARGS((options_ptr));
EXTERN void    set_ignore_spec ARGS((options_ptr));
EXTERN void    unset_ignore_spec ARGS((options_ptr));
EXTERN boolean opt_ignore_spec ARGS((options_ptr));
EXTERN void    set_ignore_compute ARGS((options_ptr));
EXTERN void    unset_ignore_compute ARGS((options_ptr));
EXTERN boolean opt_ignore_compute ARGS((options_ptr));
EXTERN void    set_ignore_ltlspec ARGS((options_ptr));
EXTERN void    unset_ignore_ltlspec ARGS((options_ptr));
EXTERN boolean opt_ignore_ltlspec ARGS((options_ptr));
EXTERN void    set_ignore_pslspec ARGS((options_ptr));
EXTERN void    unset_ignore_pslspec ARGS((options_ptr));
EXTERN boolean opt_ignore_pslspec ARGS((options_ptr));
EXTERN void    set_check_fsm ARGS((options_ptr));
EXTERN void    unset_check_fsm ARGS((options_ptr));
EXTERN boolean opt_check_fsm ARGS((options_ptr));
EXTERN void    set_ignore_invar ARGS((options_ptr));
EXTERN void    unset_ignore_invar ARGS((options_ptr));
EXTERN boolean opt_ignore_invar ARGS((options_ptr));
EXTERN void    set_forward_search ARGS((options_ptr));
EXTERN void    unset_forward_search ARGS((options_ptr));
EXTERN boolean opt_forward_search ARGS((options_ptr));
EXTERN void    set_print_reachable ARGS((options_ptr));
EXTERN void    unset_print_reachable ARGS((options_ptr));
EXTERN boolean opt_print_reachable ARGS((options_ptr));
EXTERN void    set_reorder ARGS((options_ptr));
EXTERN void    unset_reorder ARGS((options_ptr));
EXTERN boolean opt_reorder ARGS((options_ptr));
EXTERN void    set_reorder_method ARGS((options_ptr, unsigned int));
EXTERN unsigned int get_reorder_method ARGS((options_ptr));
EXTERN void    set_dynamic_reorder ARGS((options_ptr));
EXTERN void    unset_dynamic_reorder ARGS((options_ptr));
EXTERN boolean opt_dynamic_reorder ARGS((options_ptr));
EXTERN void    set_batch ARGS((options_ptr));
EXTERN void    unset_batch ARGS((options_ptr));
EXTERN boolean opt_batch ARGS((options_ptr));
EXTERN void set_partition_method ARGS((options_ptr, const TransType)); 
EXTERN TransType get_partition_method ARGS((options_ptr opt));
EXTERN void    reset_partitioning_method ARGS((options_ptr));
EXTERN void    set_monolithic ARGS((options_ptr));
EXTERN void    set_conj_partitioning ARGS((options_ptr));
EXTERN void    set_iwls95cp_partitioning ARGS((options_ptr));
EXTERN boolean opt_monolithic ARGS((options_ptr));
EXTERN boolean opt_conj_partitioning ARGS((options_ptr));
EXTERN boolean opt_iwls95cp_partitioning ARGS((options_ptr));
EXTERN void    set_conj_part_threshold ARGS((options_ptr, int));
EXTERN void    reset_conj_part_threshold ARGS((options_ptr));
EXTERN int     get_conj_part_threshold ARGS((options_ptr));
EXTERN void set_image_cluster_size ARGS((options_ptr, int));
EXTERN void reset_image_cluster_size ARGS((options_ptr));
EXTERN int get_image_cluster_size ARGS((options_ptr));
EXTERN void    set_ignore_init_file ARGS((options_ptr));
EXTERN void    unset_ignore_init_file ARGS((options_ptr));
EXTERN boolean opt_ignore_init_file ARGS((options_ptr));
EXTERN void    set_ag_only ARGS((options_ptr));
EXTERN void    unset_ag_only ARGS((options_ptr));
EXTERN boolean opt_ag_only ARGS((options_ptr));
EXTERN void    set_cone_of_influence ARGS((options_ptr));
EXTERN void    unset_cone_of_influence ARGS((options_ptr));
EXTERN boolean opt_cone_of_influence ARGS((options_ptr));
EXTERN void    set_bmc_mode ARGS((options_ptr));
EXTERN void    unset_bmc_mode ARGS((options_ptr));
EXTERN boolean opt_bmc_mode ARGS((options_ptr));
EXTERN void    set_list_properties ARGS((options_ptr));
EXTERN void    unset_list_properties ARGS((options_ptr));
EXTERN boolean opt_list_properties ARGS((options_ptr));
EXTERN void    set_prop_no ARGS((options_ptr, int n));
EXTERN int     get_prop_no ARGS((options_ptr));
EXTERN void print_partition_method  ARGS((FILE *));
EXTERN char* get_bmc_dimacs_filename ARGS((options_ptr));
EXTERN void set_bmc_dimacs_filename ARGS((options_ptr, char *));
EXTERN char* get_bmc_invar_dimacs_filename ARGS((options_ptr));
EXTERN void set_bmc_invar_dimacs_filename ARGS((options_ptr, char *));
EXTERN void set_bmc_pb_length ARGS((options_ptr opt, const int k));
EXTERN int get_bmc_pb_length ARGS((options_ptr));
EXTERN void set_bmc_pb_loop  ARGS((options_ptr opt, const char* loop));
EXTERN const char* get_bmc_pb_loop ARGS((options_ptr));
EXTERN void set_bmc_invar_alg ARGS((options_ptr opt, const char* loop));
EXTERN const char* get_bmc_invar_alg ARGS((options_ptr));
#if HAVE_INCREMENTAL_SAT
EXTERN void set_bmc_inc_invar_alg ARGS((options_ptr opt, const char* loop));
EXTERN const char* get_bmc_inc_invar_alg ARGS((options_ptr));
#endif
EXTERN void set_bmc_optimized_tableau ARGS((options_ptr));
EXTERN void unset_bmc_optimized_tableau ARGS((options_ptr));
EXTERN boolean opt_bmc_optimized_tableau ARGS((options_ptr));
EXTERN void    set_bmc_force_pltl_tableau   ARGS((options_ptr));
EXTERN void    unset_bmc_force_pltl_tableau ARGS((options_ptr));
EXTERN boolean opt_bmc_force_pltl_tableau   ARGS((options_ptr));
EXTERN void set_sat_solver ARGS((options_ptr, const char*));
EXTERN const char* get_sat_solver ARGS((options_ptr));
EXTERN void set_iwls95_preorder ARGS((options_ptr opt));
EXTERN void unset_iwls95_preorder ARGS((options_ptr opt));
EXTERN boolean opt_iwls95_preorder ARGS((options_ptr opt));
EXTERN void set_affinity ARGS((options_ptr));
EXTERN void unset_affinity ARGS((options_ptr));
EXTERN boolean opt_affinity ARGS((options_ptr));
EXTERN void set_counter_examples ARGS((options_ptr));
EXTERN void unset_counter_examples ARGS((options_ptr));
EXTERN boolean opt_counter_examples ARGS((options_ptr));
EXTERN void set_on_failure_script_quits ARGS((options_ptr));
EXTERN void unset_on_failure_script_quits ARGS((options_ptr));
EXTERN boolean opt_on_failure_script_quits ARGS((options_ptr));
EXTERN void set_write_order_dumps_bits ARGS((options_ptr));
EXTERN void unset_write_order_dumps_bits ARGS((options_ptr));
EXTERN boolean opt_write_order_dumps_bits ARGS((options_ptr));
/* SA Stuff */
#if HAVE_SA
EXTERN void    set_sa_stsa_fm_mapping_file ARGS((options_ptr, char*));
EXTERN void    reset_sa_stsa_fm_mapping_file ARGS((options_ptr));
EXTERN char *  get_sa_stsa_fm_mapping_file ARGS((options_ptr opt));
#endif

#endif /* __OPT_H__ */
