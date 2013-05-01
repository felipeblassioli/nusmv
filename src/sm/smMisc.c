/**CFile***********************************************************************

  FileName    [smMisc.c]

  PackageName [sm]

  Synopsis    [This file contain the main routine for the batch use of NuSMV.]

  Description [This file contain the main routine for the batch use of
  NuSMV. The batch main executes the various model checking steps in a
  predefined order. After the processing of the input file than it
  return to the calling shell.]

  SeeAlso     []

  Author      [Adapted to NuSMV by Marco Roveri]

  Copyright   [
  This file is part of the ``sm'' package of NuSMV version 2. 
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

******************************************************************************/

#include "smInt.h"
#include "set/set.h"
#include "fsm/bdd/BddFsm.h"

#include "utils/error.h"
#include "mc/mc.h"
#include "enc/enc.h"
#include "bmc/bmcUtils.h"

static char rcsid[] UTIL_UNUSED = "$Id: smMisc.c,v 1.26.2.26.2.6 2005/06/29 09:51:54 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The batch main.]

  Description [The batch main. It first read the input file, than
  flatten the hierarchy. Aftre this preliminar phase it creates the
  boolean variables necessary for the encoding and then start
  compiling the read model into BDD. Now computes the reachable states
  depending if the flag has been set. before starting verifying if the
  properties specified in the model hold or not it computes the
  fairness constraints. You can also activate the reordering and
  also choose to avoid the verification of the properties.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void smBatchMain()
{
  node_ptr init_expr         = Nil;
  node_ptr invar_expr        = Nil;
  node_ptr trans_expr        = Nil;
  node_ptr procs_expr        = Nil;
  node_ptr justice_expr      = Nil;
  node_ptr compassion_expr   = Nil;
  node_ptr spec_expr         = Nil;
  node_ptr compute_expr      = Nil;
  node_ptr ltlspec_expr      = Nil;
  node_ptr pslspec_expr      = Nil;
  node_ptr invar_spec_expr   = Nil;
  int err;

  Encoding_ptr senc;
  SexpFsm_ptr scalar_fsm;

  /* Necessary to have standard behavior in the batch mode */
  util_resetlongjmp();

  /* Parse the input file */
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Parsing file %s ...", get_input_file(options));
    fflush(nusmv_stderr);
  }

  /* 1: read model */
  if (Parser_ReadSMVFromFile(get_input_file(options))) nusmv_exit(1);

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "...done.\n");
    fflush(nusmv_stderr);
  }
  
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Flattening hierarchy ...");
  }
  
  /* 2: flatten hierarchy */
  /* Initializes the module hash */
  init_module_hash();
  /* check_all_implements(parse_tree); */

  /* Initializes the encodings before the flattening phase.  Encodings
     will be re-initialized after the cretion of the boolean
     SexpFsm */
  Enc_init_symbolic_encoding();  
  senc = Enc_get_symb_encoding();

  /*
    Processing of the parse tree and constructions of all the
    expressions for the state machine(s). Here the expansions are
    performed so that modules and processes are created. The expansion
    of modules is such that the formal parameters (if any) are
    replaced by the actual ones and the machine is replicated.
  */
  Compile_FlattenHierarchy(senc, sym_intern("main"), Nil, &trans_expr, 
			   &init_expr,
                           &invar_expr, &spec_expr, &compute_expr,
                           &ltlspec_expr, &pslspec_expr, 
			   &invar_spec_expr, &justice_expr,
                           &compassion_expr, &procs_expr, Nil);

  cmp_struct_set_init(cmps, init_expr);
  cmp_struct_set_invar(cmps, invar_expr);
  cmp_struct_set_trans(cmps, trans_expr);
  cmp_struct_set_procs(cmps, procs_expr);
  cmp_struct_set_justice(cmps, reverse(justice_expr));
  cmp_struct_set_compassion(cmps, reverse(compassion_expr));
  cmp_struct_set_spec(cmps, reverse(spec_expr));
  cmp_struct_set_compute(cmps, reverse(compute_expr));
  cmp_struct_set_ltlspec(cmps, reverse(ltlspec_expr));
  cmp_struct_set_pslspec(cmps, reverse(pslspec_expr));
  cmp_struct_set_invar_spec(cmps, reverse(invar_spec_expr));

  /* We store properties in the DB of properties */
  err = PropDb_fill(senc, cmp_struct_get_spec(cmps), 
		    cmp_struct_get_compute(cmps),
		    cmp_struct_get_ltlspec(cmps), 
		    cmp_struct_get_pslspec(cmps),
		    cmp_struct_get_invar_spec(cmps));

  if (err != 0) return;

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "...done.\n");
  }

  /* If the -lp option is used, list the properties and exit */
  if (opt_list_properties(options) == true) {
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Listing the properties in the database...\n");
    }
    PropDb_print_all(nusmv_stdout);

    return;
  }


  /* 3: build variables */
  /* Initializes the build_vars routines */
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Encoding variables ...");
  }

  /* RC: the global fsm builder creation */
  nusmv_assert(global_fsm_builder == FSM_BUILDER(NULL));
  global_fsm_builder = FsmBuilder_create(dd_manager);
  Encoding_encode_vars(senc);

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "... done.\n");
  }

  /* Initializes the bdd encoding: */
  Enc_init_bdd_encoding();

  /* 4: build model */
  /*
    Checking for the correctness of the state machines which have
    been declared. It should not mention DD manipulation.
  */
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Building Model ...");
  }

  /* We check that the program is a correct NuSMV program and we
     initialize all the DS necessary for the FSM model construction */
  Compile_InitializeBuildModel();

  /* Builds the sexp FSM of the whole read model */
  scalar_fsm = FsmBuilder_create_sexp_fsm(global_fsm_builder, 
         senc, 
	 NodeList_to_node_ptr(Encoding_get_all_model_vars_list(senc)), 
         SEXP_FSM_TYPE_SCALAR);

  Prop_master_set_scalar_sexp_fsm(scalar_fsm);

  /* Write flat scalar model */
  if (get_output_flatten_model_file(options) != NIL(char)) {
    FILE* ofileid = fopen(get_output_flatten_model_file(options), "w");
    if (ofileid == (FILE*) NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n",
              get_output_flatten_model_file(options));
      nusmv_exit(1);
    }
    
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Writing whole flat model into file \"%s\"...",
              get_output_flatten_model_file(options));
    }
    Compile_WriteFlatten(senc, ofileid, cmps);
    if (opt_verbose_level_gt(options, 0)) fprintf(nusmv_stderr, "... done.\n");
    fflush(ofileid);
    fclose(ofileid);
  }

  /* Write flat boolean model */
  if (get_output_boolean_model_file(options) != NIL(char)) {
    FILE* ofileid;
    SexpFsm_ptr bool_fsm; 

    ofileid = fopen(get_output_boolean_model_file(options), "w");
    if (ofileid == (FILE*)NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n",
              get_output_flatten_model_file(options));
      nusmv_exit(1);
    }
    
    /* Creates the boolean scalar fsm if needed: */
    bool_fsm = Prop_master_get_bool_sexp_fsm();
    if (bool_fsm == SEXP_FSM(NULL)) {      
      bool_fsm = SexpFsm_scalar_to_boolean(scalar_fsm);
      Prop_master_set_bool_sexp_fsm(bool_fsm);
    }

    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Writing whole boolean model into file \"%s\"...",
              get_output_boolean_model_file(options));
    }
    Compile_WriteFlattenBool(senc, ofileid, bool_fsm, cmps);
    
    if (opt_verbose_level_gt(options, 0)) fprintf(nusmv_stderr, "... done.\n");
    fflush(ofileid);
    fclose(ofileid);
  }

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "... done.\n");
  }

  /* BMC starts ***********/
  if (opt_bmc_mode(options) == true) {
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Entering BMC mode...\n");
    }
    
    /* Creates the boolean scalar fsm if needed: */
    if (Prop_master_get_bool_sexp_fsm() == SEXP_FSM(NULL)) {      
      Prop_master_set_bool_sexp_fsm(SexpFsm_scalar_to_boolean(scalar_fsm));
    }

    /* Initializes the bmc package: */
    Bmc_Init();

    if (get_prop_no(options) != -1) {
      int prop_no = get_prop_no(options);
      Prop_ptr prop;

      if (opt_verbose_level_gt(options, 0)) {
        fprintf(nusmv_stderr, "Verifying property %d...\n", prop_no);
      }
      
      if ( (prop_no < 0) || (prop_no >= PropDb_get_size()) ) {
        fprintf(nusmv_stderr, 
                "Error: \"%d\" is not a valid property index\n", prop_no);
        nusmv_exit(1);
      }

      prop = PropDb_get_prop_at_index(prop_no);
      
      switch (Prop_get_type(prop)) {
      case Prop_Ltl:
	{
	  int rel_loop; 
	  
	  /* skip if -ils option is given */
	  if (opt_ignore_ltlspec(options)) break;

	  rel_loop = Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), 
						     NULL);
	  Bmc_GenSolveLtl(prop, get_bmc_pb_length(options), rel_loop, 
			  /*increasing length*/ TRUE , TRUE, BMC_DUMP_NONE, NULL);
	  break;
	} 

      case Prop_Psl:
	{
	  int rel_loop = Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), 
							 NULL);

	  /* skip if -ips option is given */
	  if (opt_ignore_pslspec(options)) break;

	  Bmc_check_psl_property(prop, false, false, false, 
				 get_bmc_pb_length(options), rel_loop);
	  break;
	}
		
      case Prop_Invar: 
	/* skip if -ii option is given */
	if (opt_ignore_invar(options)) break;
	
        Bmc_GenSolveInvar(prop, TRUE, BMC_DUMP_NONE, NULL);
	break;
	
      default:
        fprintf(nusmv_stderr, 
                "Error: only LTL and INVAR properties can be checked in BMC mode\n");
        nusmv_exit(1);
      } /* switch on type */

    }
    else {
      /* Checks all ltlspecs, invarspecs and pslspecs */
      
      if (! opt_ignore_ltlspec(options)) {
	lsList props; 
	lsGen  iterator; 
	Prop_ptr prop;
	int rel_loop;

	if (opt_verbose_level_gt(options, 0)) {
	  fprintf(nusmv_stderr, "Verifying the LTL properties...\n");
	}

	
	props = PropDb_get_props_of_type(Prop_Ltl);	  
        nusmv_assert(props != LS_NIL);
	
        lsForEachItem(props, iterator, prop) {
          rel_loop = Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), 
						     NULL);

          Bmc_GenSolveLtl(prop, get_bmc_pb_length(options), rel_loop, 
                          /*increasing length*/ TRUE, TRUE, BMC_DUMP_NONE, NULL);
        }

        lsDestroy(props, NULL); /* the list is no longer needed */
      }

      if (! opt_ignore_pslspec(options)) {
	lsList props; 
	lsGen  iterator; 
	Prop_ptr prop;
	int rel_loop = 
	  Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), NULL);

	if (opt_verbose_level_gt(options, 0)) {
	  fprintf(nusmv_stderr, "Verifying the PSL properties...\n");
	}
	
	props = PropDb_get_props_of_type(Prop_Psl);	  
        nusmv_assert(props != LS_NIL);
	
        lsForEachItem(props, iterator, prop) {
	  if (Prop_is_psl_ltl(prop)) {
	    Bmc_check_psl_property(prop, false, false, false, 
				   get_bmc_pb_length(options), rel_loop);
	  }
        }

        lsDestroy(props, NULL); /* the list is no longer needed */
      }

      if (! opt_ignore_invar(options)) {
        lsList props;
        lsGen  iterator; 
        Prop_ptr prop;
	
	if (opt_verbose_level_gt(options, 0)) {
	  fprintf(nusmv_stderr, "Verifying the INVAR properties...\n");
	}
      
	props = PropDb_get_props_of_type(Prop_Invar);
        nusmv_assert(props != LS_NIL);
        
        lsForEachItem(props, iterator, prop) {
          Bmc_GenSolveInvar(prop, TRUE, BMC_DUMP_NONE, NULL);
        }

        lsDestroy(props, NULL); /* the list is no longer needed */
      }
    }

    return;
  } /* end of BMC */

  /* Builds the BDD FSM of the whole read model */
  if (opt_cone_of_influence(options) == false) {
    BddFsm_ptr bdd_fsm;

    bdd_fsm = FsmBuilder_create_bdd_fsm(global_fsm_builder, 
					Enc_get_bdd_encoding(), 
					scalar_fsm, 
					get_partition_method(options));
    
    Prop_master_set_bdd_fsm(bdd_fsm);
    
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "... done.\n");
    }
  }

  if (opt_cone_of_influence(options) == false) {
    /* Reachability */
    /* Builds the OBDD representing Reachable States Set */
    if (opt_forward_search(options)) {

      if (opt_verbose_level_gt(options, 0)) {
        fprintf(nusmv_stderr,"Setting reachable states on...");
      }

      set_use_reachable_states(options);
      
      if (opt_verbose_level_gt(options, 0)) fprintf(nusmv_stderr,"...done\n");
    }
  }
  
  /* checks the fsm if required */
  if (opt_check_fsm(options) == true) {
    if (opt_cone_of_influence(options) == false) {
      BddFsm_check_machine(Prop_master_get_bdd_fsm());
    }
    else {
      fprintf(nusmv_stderr, "WARNING: Check for totality of the transition relation cannot currently\n");
      fprintf(nusmv_stderr, "performed in batch mode if the cone of influence reduction has been enabled.\n");
    }
  }

  /* Ordering file dump and exit only if ordering is disabled: */
  if ( !opt_reorder(options) 
       && !is_default_order_file(options)
       && !util_is_string_null(get_output_order_file(options)))  {
    VarOrderingType dump_type;
    if (opt_write_order_dumps_bits(options)) dump_type = DUMP_BITS;
    else dump_type = DUMP_DEFAULT;

    BddEnc_write_order(Enc_get_bdd_encoding(), get_output_order_file(options), 
		       dump_type);
    return;
  }

  if (get_prop_no(options) != -1) {
    int prop_no = get_prop_no(options);

    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Verifying property %d...\n", prop_no);
    }
      
    if ( (prop_no < 0) || (prop_no >= PropDb_get_size()) ) {
      fprintf(nusmv_stderr, 
              "Error: \"%d\" is not a valid property index\n", prop_no);
      nusmv_exit(1);
    }

    PropDb_verify_prop_at_index(prop_no);
  }
  else {
    /* Evaluates the Specifications */
    if (!opt_ignore_spec(options)) {
      PropDb_verify_all_type(Prop_Ctl);
    }

    if (!opt_ignore_compute(options)) {
      PropDb_verify_all_type(Prop_Compute);
    }

    /* Evaluates the LTL specifications */
    if (!opt_ignore_ltlspec(options)) {
      PropDb_verify_all_type(Prop_Ltl);
    }

    /* Evaluates the PSL specifications */
    if (!opt_ignore_pslspec(options)) {
      PropDb_verify_all_type(Prop_Psl);
    }

    /* Evaluates CHECKINVARIANTS */
    if (!opt_ignore_invar(options)) {
      PropDb_verify_all_type(Prop_Invar);
    }
  }

  /* Reporting of statistical information. */
  if (opt_verbose_level_gt(options, 0)) {  
    print_usage(nusmv_stdout);
  }

  /* Computing and Reporting of the Effect of Reordering */
  if (opt_reorder(options)) {
    fprintf(nusmv_stdout, "\n========= starting reordering ============\n");
    dd_reorder(dd_manager, get_reorder_method(options), DEFAULT_MINSIZE);
    /*    Compile_WriteOrder(get_output_order_file(options), 0); */
    fprintf(nusmv_stdout, "\n========= after reordering ============\n");
    if (opt_verbose_level_gt(options, 0)) {  
      print_usage(nusmv_stdout);
    }
  }
  
  /* Ordering file dump: */
  if ( opt_reorder(options) 
       || (!is_default_order_file(options) 
	   && !util_is_string_null(get_output_order_file(options))) ) {
    VarOrderingType dump_type;
    if (opt_write_order_dumps_bits(options)) dump_type = DUMP_BITS;
    else dump_type = DUMP_DEFAULT;

    BddEnc_write_order(Enc_get_bdd_encoding(), get_output_order_file(options), 
		       dump_type);
  }

  if (opt_print_reachable(options) == true) {
    /* Reporting of Reachable States */
    if (opt_cone_of_influence(options) == false) {
      BddFsm_print_reachable_states_info(Prop_master_get_bdd_fsm(), 
					 false, /* do not print states */
					 nusmv_stdout);
    }
    else {
      fprintf(nusmv_stderr, "WARNING: Statistics of reachable states is not currently available\n");
      fprintf(nusmv_stderr, "in batch mode if cone of influence reduction has been enabled.\n");
    }
  }
}

/**Function********************************************************************

  Synopsis           [Prints usage statistic.]

  Description        [Prints on <code>nusmv_stdout</code> usage
  statistics, i.e. the amount of memory used, the amount of time
  spent, the number of BDD nodes allocated and the size of the model
  in BDD.]

  SideEffects        []

  SeeAlso            [compilePrintBddModelStatistic]
******************************************************************************/
void print_usage(FILE * file)
{
  fprintf(nusmv_stdout, "######################################################################\n");
  util_print_cpu_stats(file);
  fprintf(file, "######################################################################\n");
  fprintf(file, "BDD statistics\n");
  fprintf(file, "--------------------\n");
  fprintf(file, "BDD nodes allocated: %d\n", get_dd_nodes_allocated(dd_manager));
  fprintf(file, "--------------------\n");
  if (opt_cone_of_influence(options) == false) {
    BddFsm_ptr bdd_fsm = Prop_master_get_bdd_fsm();
    if (bdd_fsm != BDD_FSM(NULL)) {
      BddFsm_print_info(bdd_fsm, file);    
    }
  }
  else {
    fprintf(nusmv_stderr, "WARNING: Model Statistics is not currently available\n");
    fprintf(nusmv_stderr, "if cone of influence reduction has been enabled.\n");
  }
}



/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void restore_nusmv_stdout()
{
  if (nusmv_stdout != def_nusmv_stdout) {
    fclose(nusmv_stdout);
    nusmv_stdout = def_nusmv_stdout;
  }
}



/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void restore_nusmv_stderr()
{
  if (nusmv_stderr != def_nusmv_stderr) {
    fclose(nusmv_stderr);
    nusmv_stderr = def_nusmv_stderr;
  }  
}

