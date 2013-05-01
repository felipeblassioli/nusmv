/**CFile***********************************************************************

  FileName    [compileCmd.c]

  PackageName [compile]

  Synopsis    [Shell interface for the compile package.]

  Description [This file contains the interface of the compile package
  with the interactive shell.]

  SeeAlso     [cmdCmd.c]

  Author      [Marco Roveri]

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

******************************************************************************/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "compileInt.h"
#include "compileFsmMgr.h" /* for FsmBuilder */

#include "parser/symbols.h"
#include "utils/utils_io.h"
#include "utils/error.h" /* for CATCH */

#include "cmd/cmd.h"
#include "fsm/sexp/SexpFsm.h"
#include "fsm/bdd/BddFsm.h"
#include "prop/prop.h"
#include "mc/mc.h"
#include "enc/enc.h"
#include "utils/ucmd.h"


static char rcsid[] UTIL_UNUSED = "$Id: compileCmd.c,v 1.42.2.28.2.9 2005/11/16 12:09:45 nusmv Exp $";

/* prototypes of the command functions */
int CommandProcessModel ARGS((int argc, char **argv));
int CommandFlattenHierarchy ARGS((int argc, char **argv));
int CommandShowVars   ARGS((int argc, char **argv));
int CommandEncodeVariables ARGS((int argc, char **argv));
int CommandBuildModel ARGS((int argc, char **argv));
int CommandBuildFlatModel ARGS((int argc, char **argv));
int CommandBuildBooleanModel ARGS((int argc, char **argv));
int CommandDumpModel ARGS((int argc, char **argv));
int CommandAddTrans ARGS((int argc, char **argv));
int CommandAddInit ARGS((int argc, char **argv));
int CommandAddFairness ARGS((int argc, char **argv));
int CommandRestoreModel ARGS((int argc, char **argv));
int CommandWriteOrder ARGS((int argc, char **argv));
int CommandIwls95PrintOption ARGS((int argc, char **argv));
int CommandCPPrintClusterInfo ARGS((int argc, char **argv));
int CommandGo ARGS((int argc, char **argv));
int CommandGoBmc ARGS((int argc, char **argv));
int CommandGetInternalStatus ARGS((int argc, char **argv));
int CommandCheckWff ARGS((int argc, char **argv));
int CommandWriteModelFlat ARGS((int argc, char **argv));
int CommandWriteModelFlatBool ARGS((int argc, char **argv));

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageProcessModel ARGS((void));
static int UsageFlattenHierarchy ARGS((void));
static int UsageShowVars   ARGS((void));
static int UsageBuildModel ARGS((void));
static int UsageBuildFlatModel ARGS((void));
static int UsageBuildBooleanModel ARGS((void));
static int UsageEncodeVariables ARGS((void));
static int UsageWriteOrder ARGS((void));
static int UsageIwls95PrintOption ARGS((void));
static int UsageCPPrintClusterInfo ARGS((void));
static int UsageGo ARGS((void));
static int UsageGoBmc ARGS((void));
static int UsageGetInternalStatus ARGS((void));
static int UsageCheckWff ARGS((void));
static int UsageWriteModelFlat ARGS((void));
static int UsageWriteModelFlatBool ARGS((void));

static void compile_create_flat_model ARGS((void));
static void compile_create_boolean_model ARGS((void));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the compile package.]

  Description        [Initializes the compile package.]

  SideEffects        []

******************************************************************************/
void Compile_Init(void)
{
  cmps = cmp_struct_init();

  Cmd_CommandAdd("process_model", CommandProcessModel, 0);
  Cmd_CommandAdd("flatten_hierarchy", CommandFlattenHierarchy, 0);
  Cmd_CommandAdd("show_vars", CommandShowVars, 0);
  Cmd_CommandAdd("encode_variables", CommandEncodeVariables, 0);
  Cmd_CommandAdd("build_model", CommandBuildModel, 0);
  Cmd_CommandAdd("build_flat_model", CommandBuildFlatModel, 0);
  Cmd_CommandAdd("build_boolean_model", CommandBuildBooleanModel, 0);
  Cmd_CommandAdd("write_order", CommandWriteOrder, 0);
  Cmd_CommandAdd("print_iwls95options", CommandIwls95PrintOption, 0);
  Cmd_CommandAdd("print_clusterinfo", CommandCPPrintClusterInfo, 0);
  Cmd_CommandAdd("go", CommandGo, 0);
  Cmd_CommandAdd("go_bmc", CommandGoBmc, 0);
  Cmd_CommandAdd("get_internal_status", CommandGetInternalStatus, 0);
  Cmd_CommandAdd("check_wff", CommandCheckWff, 0);
  Cmd_CommandAdd("write_flat_model", CommandWriteModelFlat, 0);
  Cmd_CommandAdd("write_boolean_model", CommandWriteModelFlatBool, 0);
}

/**Function********************************************************************

  Synopsis           [Shut down the compile package]

  Description        [Shut down the compile package]

  SideEffects        []

******************************************************************************/
void Compile_End(void)
{
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Performs the batch steps and then returns
  control to the interactive shell.]

  CommandName        [process_model]       

  CommandSynopsis    [Performs the batch steps and then returns control
  to the interactive shell.]  

  CommandArguments   [\[-h\] \[-i model-file\] \[-m Method\]]  

  CommandDescription [ Reads the model, compiles it into BDD and
  performs the model checking of all the specification contained in
  it. If the environment variable <tt>forward_search</tt> has been set
  before, then the set of reachable states is computed. If the
  environment variables <tt>enable_reorder</tt> and
  <tt>reorder_method</tt> are set, then the reordering of variables is
  performed accordingly. This command simulates the batch behavior of
  NuSMV and then returns the control to the interactive shell.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-i model-file</tt>
       <dd> Sets the environment variable <tt>input_file</tt> to file
           <tt>model-file</tt>, and reads the model from file
           <tt>model-file</tt>.
    <dt> <tt>-m Method</tt>
       <dd> Sets the environment variable <tt>partition_method</tt> to
       <tt>Method</tt> and uses it as partitioning method.
  </dl>
  ]  

  SideEffects        []

******************************************************************************/
int CommandProcessModel(int argc, char **argv)
{
  int c;
  char * partition_method = NIL(char);

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"i:m:h")) != EOF){
    switch(c){
    case 'i': {
      set_input_file(options, util_optarg);
      break;
    }
    case 'm': {
      partition_method = ALLOC(char, strlen(util_optarg)+1);
      strcpy(partition_method, util_optarg);
      break;
    }
    case 'h': return(UsageProcessModel());
    default:  return(UsageProcessModel());
    }
  }

  if (argc != util_optind) return(UsageProcessModel());

  if (get_input_file(options) == (char *)NULL) {
    fprintf(nusmv_stderr, "Input file is (null). You must set the input file before.\n");
    return 1;
  }

  if (partition_method != NIL(char)) {
    if (TransType_from_string(partition_method) != TRANS_TYPE_INVALID) {
      set_partition_method(options, TransType_from_string(partition_method));
    } else {
      fprintf(nusmv_stderr, "The only possible values for \"-m\" option are:\n\t");
      print_partition_method(nusmv_stderr);
      fprintf(nusmv_stderr, "\n");
      return 1;
    }
  }
  
  if (cmp_struct_get_read_model(cmps) == 0)
    if (Cmd_CommandExecute("read_model")) return 1;
  if (cmp_struct_get_flatten_hrc(cmps) == 0)
    if (Cmd_CommandExecute("flatten_hierarchy")) return 1;
  if (cmp_struct_get_encode_variables(cmps) == 0)
    if (Cmd_CommandExecute("encode_variables")) return 1;
  if (cmp_struct_get_build_model(cmps) == 0)
    if(Cmd_CommandExecute("build_model")) return 1;
  if (cmp_struct_get_build_flat_model(cmps) == 0)
    if(Cmd_CommandExecute("build_flat_model")) return 1;
  if (cmp_struct_get_build_bool_model(cmps) == 0)
    if(Cmd_CommandExecute("build_boolean_model")) return 1;

  if (opt_forward_search(options)) 
    if (Cmd_CommandExecute("compute_reachable")) return 1;

  if (opt_check_fsm(options)) 
    if (Cmd_CommandExecute("check_fsm")) return 1;

  if (! opt_ignore_spec(options))
    if (Cmd_CommandExecute("check_spec")) return 1;

  if (! opt_ignore_compute(options))
    if (Cmd_CommandExecute("compute")) return 1;

  if (! opt_ignore_ltlspec(options))
    if (Cmd_CommandExecute("check_ltlspec")) return 1;

  if (! opt_ignore_pslspec(options))
    if (Cmd_CommandExecute("check_pslspec")) return 1;

  if (! opt_ignore_invar(options))
    if (Cmd_CommandExecute("check_invar")) return 1;

  if (opt_verbose_level_gt(options, 0))
    if (Cmd_CommandExecute("print_usage")) return 1;

  if (opt_reorder(options)) { /* If the case activate reordering */
    fprintf(nusmv_stdout, "\n========= starting reordering ============\n");
    dd_reorder(dd_manager, get_reorder_method(options), DEFAULT_MINSIZE);

    if (Cmd_CommandExecute("write_order")) return 1;

    fprintf(nusmv_stdout, "\n========= after reordering ============\n");

    if (opt_verbose_level_gt(options, 0))
      if (Cmd_CommandExecute("print_usage")) return 1;

  }
  return 0;
}

static int UsageProcessModel()
{
  fprintf(nusmv_stderr, "usage: process_model [-h] [-i model-file] [-m method]\n");
  fprintf(nusmv_stderr, "   -h \t\t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -i model-file \tReads the model from file \"model-file\".\n");
  fprintf(nusmv_stderr, "   -m method\t\tUses \"method\" as partition method in model construction.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Flattens the hierarchy of modules]

  CommandName        [flatten_hierarchy]           

  CommandSynopsis    [Flattens the hierarchy of modules]  

  CommandArguments   [\[-h\]]  

  CommandDescription [
  This command is responsible of the instantiation of modules and
  processes. The instantiation is performed by substituting the actual
  parameters for the formal parameters, and then by prefixing the result via 
  the instance name.]  

  SideEffects        []

******************************************************************************/
int CommandFlattenHierarchy(int argc, char ** argv)
{
  int c;
  int err;

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"h")) != EOF){
    switch(c){
    case 'h': return(UsageFlattenHierarchy());
    default:  return(UsageFlattenHierarchy());
    }
  }
  if (argc != util_optind) return(UsageFlattenHierarchy());
  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr, "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }


  if (cmp_struct_get_flatten_hrc(cmps)) {
    fprintf(nusmv_stderr, "The hierarchy has already been flattened.\n");
    return 1;
  }
  if (opt_verbose_level_gt(options, 0))
      fprintf(nusmv_stderr, "Flattening hierarchy...");

  init_module_hash();

  /* Initializes the encodings before the flattening phase.  Encodings
     will be re-initialized after the cretion of the boolean
     SexpFsm */
  Enc_init_symbolic_encoding();  

  /* Processing of the parse tree and constructions of all the
    expressions for the state machine(s). Here the expansions are
    performed so that modules and processes are created. The expansion
    of modules is such that the formal parameters (if any) are
    replaced by the actual ones and the machine is replicated. */
  {
    Encoding_ptr senc = Enc_get_symb_encoding();
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

    Compile_FlattenHierarchy(senc, 
			     sym_intern("main"), Nil, &trans_expr, &init_expr,
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
  }

  if (err == 0) {
    cmp_struct_set_flatten_hrc(cmps);
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "...done\n");
    }
  }
  
  return err;
}


static int UsageFlattenHierarchy()
{
  fprintf(nusmv_stderr, "usage: flatten_hierarchy [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Shows model's symbolic variables and their values]

  CommandName        [show_vars]

  CommandSynopsis    [Shows model's symbolic variables and their values]

  CommandArguments   [\[-h\] \[-s\] \[-i\] \[-m | -o output-file\]]

  CommandDescription [
  Prints symbolic input and state variables of the model with their range of
  values (as defined in the input file).
  <p>
  Command Options:<p>
  <dl>
    <dt> <tt>-s</tt>
       <dd> Prints only state variables.
    <dt> <tt>-i</tt>
       <dd> Prints only input variables.
    <dt> <tt>-m</tt>
       <dd> Pipes the output to the program specified by the
           <tt>PAGER</tt> shell variable if defined, else through the
           <tt>UNIX</tt> command "more".
    <dt> <tt>-o output-file</tt>
       <dd> Writes the output generated by the command to <tt>output-file</tt>
  </dl> ]

  SideEffects        []

******************************************************************************/
int CommandShowVars(int argc, char ** argv)
{
  int c = 0;
  boolean statevars = false;
  boolean inputvars = false;
  short int useMore = 0;
  char * dbgFileName = NIL(char);
#if HAVE_GETENV
  char * pager;
#endif
FILE * old_nusmv_stdout = NIL(FILE);

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"hsimo:")) != EOF){
    switch(c){
    case 'h': return(UsageShowVars());
    case 's':
      statevars = true;
      break;
    case 'i':
      inputvars = true;
      break;
    case 'o':
      if (useMore == 1) return(UsageShowVars());
      dbgFileName = util_strsav(util_optarg);
      fprintf(nusmv_stdout, "Output to file: %s\n", dbgFileName);
      break;
    case 'm':
      if (dbgFileName != NIL(char)) return(UsageShowVars());
      useMore = 1;
      break;
    default: return(UsageShowVars());
    }
  }
  if (argc != util_optind) return(UsageShowVars());
  if (statevars == false && inputvars == false) return(UsageShowVars());

  /* we need only a flattened hierarchy to be able to see the variables */

  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }

  if (useMore) {
    old_nusmv_stdout = nusmv_stdout;
#if HAVE_GETENV
    pager = getenv("PAGER");
    if (pager == NULL) {
      nusmv_stdout = popen("more", "w");
      if (nusmv_stdout == NULL) {
        fprintf(nusmv_stderr, "Unable to open pipe with \"more\".\n");
        nusmv_stdout = old_nusmv_stdout;
        return 1;
      }
    }
    else {
      nusmv_stdout = popen(pager, "w");
      if (nusmv_stdout == NULL) {
        fprintf(nusmv_stderr, "Unable to open pipe with \"%s\".\n", pager);
        nusmv_stdout = old_nusmv_stdout;
        return 1;
      }
    }
#else
    nusmv_stdout = popen("more", "w");
    if (nusmv_stdout == NULL) {
      fprintf(nusmv_stderr, "Unable to open pipe with \"more\".\n");
      nusmv_stdout = old_nusmv_stdout;
      return 1;
    }
#endif
  }
  if (dbgFileName != NIL(char)) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = fopen(dbgFileName, "w");
    if (nusmv_stdout == NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", dbgFileName);
      nusmv_stdout = old_nusmv_stdout;
      return 1;
    }
  }
  {
    Encoding_ptr senc = Enc_get_symb_encoding();
    NodeList_ptr list = NODE_LIST(NULL);
    ListIter_ptr iter;

    if (statevars == true && inputvars == true) {
      list = Encoding_get_all_model_vars_list(senc);
    }
    else if (inputvars == true) {
      list = Encoding_get_input_vars_list(senc);
    }
    else { list = Encoding_get_state_vars_list(senc); } 

    set_indent_size(1);
    iter = NodeList_get_first_iter(list);
    while (! ListIter_is_end(iter)) {
      node_ptr name = NodeList_get_elem_at(list, iter);

      if (Encoding_is_symbol_var(senc, name)) {
        print_node(nusmv_stdout, name);
        fprintf(nusmv_stdout, " : ");
        print_node(nusmv_stdout, Encoding_get_var_range(senc, name));
        fprintf(nusmv_stdout, "\n");
      }

      iter = ListIter_get_next(iter);
    } /* loop */
    reset_indent_size();
  }

  if (useMore) {
    pclose(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }
  
  if (dbgFileName != NIL(char)) {
    fflush(nusmv_stdout);
    fclose(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }
  
  return 0;
}


static int UsageShowVars () {
  fprintf(nusmv_stderr,"usage: show_vars [-h] [-s] [-i] [-m | -o file]\n");
  fprintf(nusmv_stderr,"  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr,"  -s \t\tPrints only the state variables.\n");
  fprintf(nusmv_stderr,"  -i \t\tPrints only the input variables.\n");
  fprintf(nusmv_stderr,"  -m \t\tPipes output through the program specified by the \"PAGER\"\n");
  fprintf(nusmv_stderr,"     \t\tenvironment variable if defined, else through the UNIX command \"more\".\n");
  fprintf(nusmv_stderr,"  -o file\tWrites the generated output to \"file\".\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Builds the BDD variables necessary to compile the
  model into BDD.]

  CommandName        [encode_variables]

  CommandSynopsis    [Builds the BDD variables necessary to compile the
  model into BDD.]  

  CommandArguments   [\[-h\] \[-i order-file\]]  

  CommandDescription [
  Generates the boolean BDD variables and the ADD needed to encode
  propositionally the (symbolic) variables declared in the model.<br>

  The variables are created as default in the order in which they
  appear in a depth first traversal of the hierarchy.<p>

  The input order file can be partial and can contain variables not
  declared in the model. Variables not declared in the model are
  simply discarded. Variables declared in the model which are not
  listed in the ordering input file will be created and appended at the 
  end of the given ordering list, according to the default ordering.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-i order-file</tt>
       <dd> Sets the environment variable <tt>input_order_file</tt> to
       <tt>order-file</tt>, and reads the variable ordering to be used from
       file <tt>order-file</tt>. This can be combined with the
       <tt>write_order</tt> command. The variable ordering is written to a 
       file, which can be inspected and reordered by the user, and then 
       read back in.
  </dl>]  

  SideEffects        []

******************************************************************************/
int CommandEncodeVariables(int argc, char ** argv)
{
  int c;
  char * input_order_file_name = NIL(char);

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"i:h")) != EOF){
    switch(c){
    case 'i': {
      input_order_file_name = ALLOC(char, strlen(util_optarg)+1);
      strcpy(input_order_file_name, util_optarg);
      break;
    }    
    case 'h': return(UsageEncodeVariables());
    default:  return(UsageEncodeVariables());
    }
  }

  if (argc != util_optind) return(UsageEncodeVariables());

  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }  

  if (cmp_struct_get_encode_variables(cmps)) {
    fprintf(nusmv_stderr, "The variables appear to be already built.\n");
    return 1;
  }

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Building variables...");
  }

  if (input_order_file_name != NIL(char))
    set_input_order_file(options, input_order_file_name);


  /* the global fsm builder creation */
  nusmv_assert(global_fsm_builder == FSM_BUILDER(NULL));
  global_fsm_builder = FsmBuilder_create(dd_manager);

  Encoding_encode_vars(Enc_get_symb_encoding());
  cmp_struct_set_encode_variables(cmps);

  /* init of bdd encoding after encoding */
  Enc_init_bdd_encoding();

  Compile_InitializeBuildModel();
    
  if (!opt_reorder(options) 
      && !is_default_order_file(options)
      && !util_is_string_null(get_output_order_file(options))) {
    VarOrderingType dump_type;
    if (opt_write_order_dumps_bits(options)) dump_type = DUMP_BITS;
    else dump_type = DUMP_DEFAULT;

    BddEnc_write_order(Enc_get_bdd_encoding(), get_output_order_file(options),
		       dump_type);

    /* batch mode: */
    if (opt_batch(options)) { nusmv_exit(0); }
  }

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "...done\n");
  }

  return 0;
}


static int UsageEncodeVariables()
{
  fprintf(nusmv_stderr, "usage: encode_variables [-h] [-i <file>]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -i <file> \tReads variable ordering from file <file>.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Compiles the flattened hierarchy into BDD]

  CommandName        [build_model]         

  CommandSynopsis    [Compiles the flattened hierarchy into BDD]  

  CommandArguments   [\[-h\] \[-f\] \[-m Method\]]  

  CommandDescription [
  Compiles the flattened hierarchy into BDD (initial states, invariants,
  and transition relation) using the method specified in the environment
  variable <tt>partition_method</tt> for building the transition relation.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-m Method</tt>
       <dd> Sets the environment variable <tt>partition_method</tt> to
           the value <tt>Method</tt>, and then builds the transition
           relation. Available methods are <code>Monolithic</code>,
           <code>Threshold</code> and <code>Iwls95CP</code>.
    <dt> <tt>-f</tt>
       <dd> Forces model construction. By default, only one partition
            method is allowed. This option allows to overcome this
            default, and to build the transition relation with different
            partitioning methods.
  </dl>]  

  SideEffects        []

******************************************************************************/
int CommandBuildModel(int argc, char ** argv)
{
  int c;
  int Build_Model_Force = 0;
  char * partition_method = NIL(char);

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"m:fh")) != EOF){
    switch(c){
    case 'm': {
      partition_method = ALLOC(char, strlen(util_optarg)+1);
      strcpy(partition_method, util_optarg);
      break;
    }
    case 'f': {
      Build_Model_Force = 1;
      break;
    }
    case 'h': return(UsageBuildModel());
    default:  return(UsageBuildModel());
    }
  }
  if (argc != util_optind) return(UsageBuildModel());

  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }

  if (Build_Model_Force != 1) {
    if (cmp_struct_get_build_model(cmps)) {
      fprintf(nusmv_stderr, "A model appear to be already built from file: %s.\n",
              get_input_file(options));
      return 1;
    }
  }

  if (partition_method != NIL(char)) {
    if (TransType_from_string(partition_method) != TRANS_TYPE_INVALID) {
      if ((Build_Model_Force == 1) &&
          (TransType_from_string(partition_method) == get_partition_method(options))) {
        if (cmp_struct_get_build_model(cmps)) {
          fprintf(nusmv_stderr, "A model for the chosen method has already been constructed.\n");
          return 1;
        }
      }
      set_partition_method(options, TransType_from_string(partition_method));
    } else {
      fprintf(nusmv_stderr, "The only possible values for \"-m\" option are:\n\t");
      print_partition_method(nusmv_stderr);
      fprintf(nusmv_stderr, "\n");
      return 1;
    }
  }
  
  {
    SexpFsm_ptr sexp_fsm;
    BddFsm_ptr  bdd_fsm;
    
    /* creates the model only if required (i.e. build_flat_model not called) */
    compile_create_flat_model();

    /* creates the boolean model only if required (i.e. build_boolean_model not called) */
    compile_create_boolean_model();

    sexp_fsm = Prop_master_get_scalar_sexp_fsm();
    SEXP_FSM_CHECK_INSTANCE(sexp_fsm);

    bdd_fsm = FsmBuilder_create_bdd_fsm(global_fsm_builder, 
					Enc_get_bdd_encoding(), 
					sexp_fsm, 
					get_partition_method(options));
  
    /* Finally stores the built FSMs: */
    Prop_master_set_bdd_fsm(bdd_fsm);
  }

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr,
            "\nThe model has been built from file %s.\n", get_input_file(options));
  }
  /* We keep track that the master FSM has been built. */
  cmp_struct_set_build_model(cmps);
  return 0;
}

static int UsageBuildModel()
{
  fprintf(nusmv_stderr, "usage: build_model [-h] [-f] [-m Method]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage\n");
  fprintf(nusmv_stderr, "   -m Method \tUses \"Method\" as partitioning method, and set it as default method\n");
  fprintf(nusmv_stderr, "\t\tto be used in the following image computations.\n");
  fprintf(nusmv_stderr, "\t\tThe currently available methods are:\n\t\t");
  print_partition_method(nusmv_stderr);
  fprintf(nusmv_stderr, "\n   -f \t\tForces the model re-construction, even if a model has already been built\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Compiles the flattened hierarchy into SEXP]

  CommandName        [build_flat_model]            

  CommandSynopsis    [Compiles the flattened hierarchy into SEXP]  

  CommandArguments   [\[-h\]]  

  CommandDescription [
  Compiles the flattened hierarchy into SEXP (initial states, invariants,
  and transition relation).<p>]  

  SideEffects        []

******************************************************************************/
int CommandBuildFlatModel(int argc, char ** argv)
{
  int c;

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"h")) != EOF){
    switch(c){
    case 'h': return(UsageBuildFlatModel());
    default:  return(UsageBuildFlatModel());
    }
  }
  if (argc != util_optind) return(UsageBuildFlatModel());

  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }

  if (cmp_struct_get_build_flat_model(cmps)) {
    fprintf(nusmv_stderr, "A model appear to be already built from file: %s.\n",
            get_input_file(options));
    return 1;
  }
  
  /* does the work: */
  compile_create_flat_model();

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr,
            "\nThe sexp model has been built from file %s.\n", get_input_file(options));
  }
  /* We keep track that the master FSM has been built. */
  cmp_struct_set_build_flat_model(cmps);
  return 0;
}

static int UsageBuildFlatModel()
{
  fprintf(nusmv_stderr, "usage: build_flat_model [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Compiles the flattened hierarchy into boolean SEXP]

  CommandName        [build_boolean_model]         

  CommandSynopsis    [Compiles the flattened hierarchy into boolean SEXP]  

  CommandArguments   [\[-h\]]  

  CommandDescription [
  Compiles the flattened hierarchy into boolean SEXP 
  (initial states, invariants, and transition relation).<p>]  

  SideEffects        []

******************************************************************************/
int CommandBuildBooleanModel(int argc, char ** argv)
{
  int c;

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"h")) != EOF){
    switch(c){
    case 'h': return(UsageBuildBooleanModel());
    default:  return(UsageBuildBooleanModel());
    }
  }
  if (argc != util_optind) return(UsageBuildBooleanModel());

  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }

  if (cmp_struct_get_build_bool_model(cmps)) {
    fprintf(nusmv_stderr, "A model appear to be already built from file: %s.\n",
            get_input_file(options));
    return 1;
  }
  
  /* does the work */
  compile_create_boolean_model();

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr,
            "\nThe boolean sexp model has been built from file %s.\n", get_input_file(options));
  }
  /* We keep track that the master FSM has been built. */
  cmp_struct_set_build_bool_model(cmps);
  return 0;
}

static int UsageBuildBooleanModel()
{
  fprintf(nusmv_stderr, "usage: build_boolean_model [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Writes variable order to file.]

  CommandName        [write_order]         

  CommandSynopsis    [Writes variable order to file.]  

  CommandArguments   [\[-h\] \[-b\] \[(-o | -f) order-file\]]  

  CommandDescription [Writes the current order of BDD variables in the
  file specified via the -o option. If no option is specified the environment 
  variable <tt>output_order_file</tt> will be considered. If the variable 
  <tt>output_order_file</tt> is unset (or set to an empty value) then standard 
  output will be used. The option <tt>-b</tt> forces the dumped 
  variable ordering to contain only boolean variables. 
  All the scalar variables will be substituted by those variables bits
  that encode them.  The variables bits will occur within the dumped
  variable ordering depending on the position they have within the
  system when the command is executed. 
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-b</tt> 
       <dd> Dumps bits of scalar variables instead of the single
       scalar variables. When specified, this option temporary
       overloads the current value of the system variable
       <tt>write_order_dumps_bits</tt>.

    <dt> <tt>-o order-file</tt>
       <dd> Sets the environment variable <tt>output_order_file</tt>
       to <tt>order-file</tt> and then dumps the ordering list into that file.
    <dt> <tt>-f order-file</tt>
       <dd> Alias for -o option. Supplied for backward compatibility. 
  </dl>]  

  SideEffects        []

******************************************************************************/
int CommandWriteOrder(int argc, char **argv)
{
  int c;
  char* order_output_fname = NIL(char);
  VarOrderingType dump_type;

  if (opt_write_order_dumps_bits(options)) dump_type = DUMP_BITS;
  else dump_type = DUMP_DEFAULT;

  /*
   * Parse the command line.
  */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "o:f:hb")) != EOF) {
    switch (c) {
    case 'h':
      if (order_output_fname != NIL(char)) FREE(order_output_fname); 
      return(UsageWriteOrder());

    case 'b':
      dump_type = DUMP_BITS;
      break;

    case 'o':
    case 'f':
      if (order_output_fname != NIL(char)) {
        /* already called (via the alias): exit */
        FREE(order_output_fname);
        return UsageWriteOrder();
      } 
      order_output_fname = ALLOC(char, strlen(util_optarg)+1);
      nusmv_assert(order_output_fname);
      strcpy(order_output_fname, util_optarg);
      break;

    default:
      if (order_output_fname != NIL(char)) FREE(order_output_fname);
      return(UsageWriteOrder());
    }
  }
  
  /* side effect on variable output_order_file: */
  if (order_output_fname != NIL(char)) { 
    set_output_order_file(options, order_output_fname);
    FREE(order_output_fname);
  }

  if (dd_manager == NIL(DdManager)) {
    fprintf(nusmv_stderr, "The DD Manager has not been created yet.\n");
    return 1;
  }
  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }

  BddEnc_write_order(Enc_get_bdd_encoding(), get_output_order_file(options), 
		     dump_type);
  
  /* batch mode: */
  if (opt_batch(options) && !opt_reorder(options))  { nusmv_exit(0); }

  return 0;
}

static int UsageWriteOrder()
{
  fprintf(nusmv_stderr, "usage: write_order [-h] | [-b] [(-o | -f) <file>]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, 
	  "   -b \t\tDumps bits of scalar variables instead of the single \n"\
	  "      \t\tscalar variables. \n"\
	  "      \t\tSee also the system variable write_order_dumps_bits.\n");
  fprintf(nusmv_stderr, "   -o <file>\tWrites ordering to file <file>.\n");
  fprintf(nusmv_stderr, "   -f <file>\tThe same of option -o. Supplied for backward compatibility.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Prints out the information of the clustering.]

  CommandName        [print_clusterinfo]

  CommandSynopsis    [Prints out the information of the clustering.]  

  CommandArguments   [\[-h\] \| \[-m\] \| \[-o output-file\]]

  CommandDescription [This command prints out the information
  regarding each cluster. In particular for each cluster it prints
  out, the cluster number, the size of the cluster (in BDD nodes), the
  variables occurring in it, the size of the cube that has to be
  quantified out relative to the cluster and the variables to be
  quantified out.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-m</tt>
       <dd> Pipes the output generated by the command through the
            program specified by the <tt>PAGER</tt> shell variable if
            defined, or through the UNIX utility "more".
    <dt> <tt>-o output-file</tt>
       <dd> Redirects the generated output to the file
            <tt>output-file</tt>.
  </dl>          
  ]

  SideEffects        []

******************************************************************************/
int CommandCPPrintClusterInfo(int argc, char ** argv)
{
  int c;
  int useMore = 0;
 char * dbgFileName = NIL(char);
#if HAVE_GETENV
  char * pager;
#endif
  FILE * old_nusmv_stdout = NIL(FILE);

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hmo")) != EOF) {
    switch (c) {
    case 'h':
      return(UsageCPPrintClusterInfo());
    case 'o':
      if (useMore == 1) return(UsageCPPrintClusterInfo());
      dbgFileName = util_strsav(util_optarg);
      fprintf(nusmv_stdout, "Output to file: %s\n", dbgFileName);
      break;
    case 'm':
      if (dbgFileName != NIL(char)) return(UsageCPPrintClusterInfo());
      useMore = 1;
      break;
    default:
      return(UsageCPPrintClusterInfo());
    }
  } /* while */
  if (useMore) {
    old_nusmv_stdout = nusmv_stdout;
#if HAVE_GETENV
    pager = getenv("PAGER");
    if (pager == NULL) {
      nusmv_stdout = popen("more", "w");
      if (nusmv_stdout == NULL) {
        fprintf(nusmv_stderr, "Unable to open pipe with \"more\".\n");
        nusmv_stdout = old_nusmv_stdout;
        return 1;
      }
    }
    else {
      nusmv_stdout = popen(pager, "w"); 
      if (nusmv_stdout == NULL) {
        fprintf(nusmv_stderr, "Unable to open pipe with \"%s\".\n", pager);
        nusmv_stdout = old_nusmv_stdout;
        return 1;
      }
    }
#else
    nusmv_stdout = popen("more", "w");
    if (nusmv_stdout == NULL) {
      fprintf(nusmv_stderr, "Unable to open pipe with \"more\".\n");
      nusmv_stdout = old_nusmv_stdout;
      return 1;
    }
#endif
  }
  if (dbgFileName != NIL(char)) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = fopen(dbgFileName, "w");
    if (nusmv_stdout == NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", dbgFileName);
      nusmv_stdout = old_nusmv_stdout;
      return 1;
    }
  }

  {
    BddFsm_ptr fsm = Prop_master_get_bdd_fsm();
    
    if (fsm != BDD_FSM(NULL)) {
      BddFsm_print_info(fsm, nusmv_stdout);
    }
  }

  if (useMore) {
    pclose(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }
  if (dbgFileName != NIL(char)) {
    fflush(nusmv_stdout);
    fclose(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }
  return 0;
}


static int UsageCPPrintClusterInfo()
{
  fprintf(nusmv_stderr, "usage: print_clusterinfo [-h] [-m] [-o file]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage\n");
  fprintf(nusmv_stderr, "   -m \t\tPipes output through the program specified by\n");
  fprintf(nusmv_stderr, "      \t\tthe \"PAGER\" shell variable if defined,\n");
  fprintf(nusmv_stderr, "      \t\t else through the UNIX command \"more\"\n");
  fprintf(nusmv_stderr, "   -o file\tWrites the cluster info to \"file\".\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Prints the Iwls95 Options.]

  CommandName        [print_iwls95options]         

  CommandSynopsis    [Prints the Iwls95 Options.]  

  CommandArguments   [\[-h\]]  

  CommandDescription [This command prints out the configuration
  parameters of the IWLS95 clustering algorithm, i.e.
  <tt>image_verbosity</tt>, <tt>image_cluster_size</tt> and
  <tt>image_W{1,2,3,4}</tt>.]

  SideEffects        []

******************************************************************************/
int CommandIwls95PrintOption(int argc, char ** argv)
{
  int c;
  ClusterOptions_ptr opts;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch (c) {
    case 'h':
      return(UsageIwls95PrintOption());
    default:
      return(UsageIwls95PrintOption());
    }
  } /* while */
  
  opts = ClusterOptions_create(options);
  ClusterOptions_print(opts, nusmv_stdout);
  ClusterOptions_destroy(opts);
  
  return 0;
}

static int UsageIwls95PrintOption()
{
  fprintf(nusmv_stderr, "usage: print_iwls95options [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Implements the go command]

  CommandName        [go]

  CommandSynopsis    [Initializes the system for the verification.]

  CommandArguments   [\[-h\]]

  CommandDescription [This command initializes the system for
  verification. It is equivalent to the command sequence
  <tt>read_model</tt>, <tt>flatten_hierarchy</tt>,
  <tt>encode_variables</tt>, <tt>build_model</tt>,
  <tt>build_flat_model</tt>, <tt>build_boolean_model</tt>. 
  If some commands have already been
  executed, then only the remaining ones will be invoked.<p>
  Command options:<p>  
  <dl><dt> -h
  <dd> Prints the command usage.<p> 
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandGo(int argc, char ** argv)
{
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch (c) {
    case 'h':
      return(UsageGo());
    default:
      return(UsageGo());
    }
  } /* while */
  
  if (cmp_struct_get_read_model(cmps) == 0)
    if (Cmd_CommandExecute("read_model")) return 1;
  if (cmp_struct_get_flatten_hrc(cmps) == 0)
    if (Cmd_CommandExecute("flatten_hierarchy")) return 1;
  if (cmp_struct_get_encode_variables(cmps) == 0)
    if (Cmd_CommandExecute("encode_variables")) return 1;
  if (cmp_struct_get_build_flat_model(cmps) == 0)
    if(Cmd_CommandExecute("build_flat_model")) return 1;
  if (cmp_struct_get_build_model(cmps) == 0)
    if(Cmd_CommandExecute("build_model")) return 1;
  return 0;
}

static int UsageGo()
{
  fprintf(nusmv_stderr, "usage: go [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Implements the go_bmc command]

  CommandName        [go_bmc]

  CommandSynopsis    [Initializes the system for the BMC verification.]

  CommandArguments   [\[-h\]]

  CommandDescription [This command initializes the system for
  verification. It is equivalent to the command sequence
  <tt>read_model</tt>, <tt>flatten_hierarchy</tt>,
  <tt>encode_variables</tt>, <tt>build_boolean_model</tt>, <tt>bmc_setup</tt>. 
  If some commands have already been
  executed, then only the remaining ones will be invoked.<p>
  Command options:<p>  
  <dl><dt> -h
  <dd> Prints the command usage.<p> 
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandGoBmc(int argc, char ** argv)
{
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch (c) {
    case 'h':
      return(UsageGoBmc());
    default:
      return(UsageGoBmc());
    }
  } /* while */
  
  if (cmp_struct_get_read_model(cmps) == 0)
    if (Cmd_CommandExecute("read_model")) return 1;
  if (cmp_struct_get_flatten_hrc(cmps) == 0)
    if (Cmd_CommandExecute("flatten_hierarchy")) return 1;
  if (cmp_struct_get_encode_variables(cmps) == 0)
    if (Cmd_CommandExecute("encode_variables")) return 1;
  if (cmp_struct_get_build_bool_model(cmps) == 0)
    if(Cmd_CommandExecute("build_boolean_model")) return 1;
  if (cmp_struct_get_bmc_setup(cmps) == 0)
    if(Cmd_CommandExecute("bmc_setup")) return 1;
  return 0;
}

static int UsageGoBmc()
{
  fprintf(nusmv_stderr, "usage: go_bmc [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Implements the get_internal_status command]

  CommandName        [get_internal_status]

  CommandSynopsis    [Returns the internal status of the system.]

  CommandArguments   [\[-h\]]

  CommandDescription [Prints out the internal status of the system. i.e.
  <ul>
  <li> -1 : read_model has not yet been executed or an error occurred
            during its execution. </li>
  <li>  0 : flatten_hierarchy has not yet been executed or an error
            occurred during its execution. </li>
  <li>  1 : encode_variables has not yet been executed or an error
            occurred during its execution. </li>
  <li>  2 : build_model has not yet been executed or an error occurred
            during its execution. </li>
  </ul>
  Command options:<p>  
  <dl><dt> -h
  <dd> Prints the command usage.<p> 
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandGetInternalStatus(int argc, char ** argv)
{
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch (c) {
    case 'h':
      return(UsageGetInternalStatus());
    default:
      return(UsageGetInternalStatus());
    }
  } /* while */
  
  if (cmp_struct_get_read_model(cmps) == 0){
    fprintf(nusmv_stderr, "The internal status is: -1\n");
    return 0;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0){
    fprintf(nusmv_stderr, "The internal status is: 0\n");
    return 0;
  }
  if (cmp_struct_get_encode_variables(cmps) == 0){
    fprintf(nusmv_stderr, "The internal status is: 1\n");
    return 0;
  }
  if (cmp_struct_get_build_model(cmps) == 0){
    fprintf(nusmv_stderr, "The internal status is: 2\n");
    return 0;
  }
  return 0;
}

static int UsageGetInternalStatus()
{
  fprintf(nusmv_stderr, "usage: get_internal_status [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Checks formulas potential errors.]

  CommandName        [check_wff]           

  CommandSynopsis    [Checks formulas potential errors.]  

  CommandArguments   [\[-h | -i invar-expr \[IN context\]   |
                             -c ctl-expr \[IN context\]     |
                             -l ltl-expr \[IN context\]     |
                             -q compute-expr \[IN context\] |
                             -n spec-number\] ]

  CommandDescription [Checks formulas potential errors.<BR>
  A potential error may arise when arguments of operands do not match
  the respective domain, i.e. <tt>a AND b</tt>, where <tt>a</tt> or
  <tt>b</tt> are not of type boolean. It analizes a formula on the
  basis of its arguments and operators. This command does not perform
  checks for undefined symbols. It only checks for \"well\" formed
  expressions. 
  <p>
  Command options:<p>
  <dl>
     <dt><tt>-i invar-expr \[IN context\]</tt>
       <dd> Attempts to check an INVAR expression.
     <dt><tt>-c ctl-expr \[IN context\]</tt>
       <dd> Attempts to check a CTL expression.
     <dt><tt>-l ltl-expr \[IN context\]</tt>
       <dd> Attempts to check an LTL expression.
     <dt><tt>-q compute-expr \[IN context\]</tt>
       <dd> Attempts to check a COMPUTE expression.
     <dt><tt>context</tt>
       <dd> The context to which the variables of the formula refers to.
  </dl>]  

  SideEffects        []

******************************************************************************/
int CommandCheckWff(int argc, char **argv)
{
  int c = 0;
  char * strNum = NIL(char);
  int usedType = 0;
  Spec_Type type = ST_Notype;

  util_getopt_reset();
  while((c = util_getopt(argc, argv, "hcliq")) != EOF) {
    switch(c){
    case 'h': return(UsageCheckWff());
    case 'c': 
      if(++usedType > 1 || strNum != NIL(char)) return(UsageCheckWff());
      type = ST_Ctl;
      break;
    case 'l': 
      if(++usedType > 1 || strNum != NIL(char)) return(UsageCheckWff());
      type = ST_Ltl;
      break;
    case 'i': 
      if(++usedType > 1 || strNum != NIL(char)) return(UsageCheckWff());
      type = ST_Invar;
      break;
    case 'q': 
      if(++usedType > 1 || strNum != NIL(char)) return(UsageCheckWff());
      type = ST_Compute;
      break;
    default: return(UsageCheckWff());
    }
  }
  if (argc <= 2) return(UsageCheckWff());

  /* command hierarchy control */
  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }

  { /* there is a wff at command line */
    node_ptr parsed_command = Nil;
    int status = 0;

    argv+=util_optind-1;
    argc-=util_optind-1;
    
    switch(type) {
    case ST_Invar:
      status = Parser_ReadCmdFromString(argc , argv, "SIMPWFF ", ";\n", &parsed_command);
      if (status == 1) fprintf(nusmv_stderr, "check_wff: expecting a simple expression.\n");
      break;
    case ST_Ctl:
      status = Parser_ReadCmdFromString(argc , argv, "CTLWFF ", ";\n", &parsed_command);
      if (status == 1) fprintf(nusmv_stderr, "check_wff: expecting a CTL expression.\n");
      break;
    case ST_Ltl:
      status = Parser_ReadCmdFromString(argc , argv, "LTLWFF ", ";\n", &parsed_command);
      if (status == 1) fprintf(nusmv_stderr, "check_wff: expecting an LTL expression.\n");
      break;
    case ST_Compute:
      status = Parser_ReadCmdFromString(argc , argv, "COMPWFF ", ";\n", &parsed_command);
      if (status == 1) fprintf(nusmv_stderr, "check_wff: expecting a COMPUTE expression.\n");
      break;
    default:
      status = 1;
      break;
    }
    
    if (status == 0) {
      CATCH {
        if ((parsed_command != Nil) &&
            (
             (node_get_type(parsed_command) == SIMPWFF) ||
             (node_get_type(parsed_command) == CTLWFF) ||
             (node_get_type(parsed_command) == LTLWFF) ||
             (node_get_type(parsed_command) == COMPWFF)
            ))
          check_wff(car(parsed_command), Nil);
      }
      FAIL {
        fprintf(nusmv_stderr, "Parsing error!!!\n");
        return 1;
      }
    }
    else {
      fprintf(nusmv_stderr, "Parsing error!!\n");
      return 1;
    }
  }
  return 0;
}

static int UsageCheckWff(void)
{
  fprintf(nusmv_stderr, "usage: check_wff [-h] \n");
  fprintf(nusmv_stderr, "       check_wff [-i invar-expr [IN context]]\n");
  fprintf(nusmv_stderr, "       check_wff [-c ctl-expr [IN context]]\n");
  fprintf(nusmv_stderr, "       check_wff [-l ltl-expr [IN context]]\n");
  fprintf(nusmv_stderr, "       check_wff [-q compute-expr [IN context]]\n");

  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  invar-expr \tAn INVAR expression to be checked.\n");
  fprintf(nusmv_stderr, "  ctl-expr \tA CTL expression to be checked.\n");
  fprintf(nusmv_stderr, "  ltl-expr \tA LTL expression to be checked.\n");
  fprintf(nusmv_stderr, "  compute-expr \tA COMPUTE expression to be checked.\n");
  fprintf(nusmv_stderr, "  context \tThe context to which the variables of the formula refer to.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Writes the currently loaded SMV model in the
  specified file, after having flattened it]

  CommandName        [write_flat_model]            

  CommandSynopsis    [Writes a flat model of a given SMV file]  

  CommandArguments   [\[-h\] \[-o filename\]]  

  CommandDescription [Processes are eliminated
  and a corresponding equivalent model is printed out.
  If no file is specified, the file specified with the environment variable
  <tt>output_flatten_model_file</tt> is used if any, otherwise standard output
  is used as output.
  <p>
  Command options:<p>
  <dl>
     <dt><tt>-o filename</tt>
       <dd> Attempts to write the flat SMV model in <tt>filename</tt>.
  </dl>
  ]  

  SideEffects        []

******************************************************************************/
int CommandWriteModelFlat(int argc, char **argv)
{
  int c = 0;
  int rv = 0;
  char * output_file = NIL(char);
  FILE * ofileid = NIL(FILE);
  int bSpecifiedFilename = FALSE; 
  
  util_getopt_reset();
  while((c = util_getopt(argc, argv, "ho:")) != EOF) {
    switch(c){
    case 'h': return(UsageWriteModelFlat());
    case 'o':
      output_file = ALLOC(char, strlen(util_optarg)+1);
      nusmv_assert(output_file);
      strcpy(output_file, util_optarg);
      bSpecifiedFilename = TRUE; 
      break;

    default:
      break;
    }
  }

  if (argc != util_optind) return(UsageWriteModelFlat());

  if (output_file == NIL(char)) {
    output_file = get_output_flatten_model_file(options);
  }
  if (output_file == NIL(char)) {
    ofileid = nusmv_stdout;
  } 
  else {
    ofileid = fopen(output_file, "w");
    if (ofileid == NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", output_file);
      if (bSpecifiedFilename == TRUE)  FREE(output_file);
      return 1;
    }
  }

  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Writing flat model into file \"%s\"..", output_file == (char *)NULL ? "stdout" : output_file);
  }

  CATCH {
    Compile_WriteFlatten(Enc_get_symb_encoding(), ofileid, cmps);
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, ".. done.\n");
    }
  } FAIL {
    rv = 1;
  }
  fflush(ofileid);

  if (ofileid != nusmv_stdout) {
    fclose(ofileid);
    if (bSpecifiedFilename) FREE(output_file);
  }
  return(rv);
}

static int UsageWriteModelFlat(void)
{
  fprintf(nusmv_stderr, "usage: write_flat_model [-h] [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -o filename\tWrites output to \"filename\"\n");
  return 1;
}



/**Function********************************************************************

  Synopsis           [Writes a flat and boolean model of a given SMV file]

  CommandName        [write_boolean_model]         

  CommandSynopsis    [Writes a flattened and booleanized model of a 
  given SMV file]  

  CommandArguments   [\[-h\] \[-o filename\]]  

  CommandDescription [Writes the currently loaded SMV model in the
  specified file, after having flattened and booleanized it. Processes
  are eliminated and a corresponding equivalent model is printed
  out. If no file is specified, the file specified via 
  the environment variable <tt>output_flatten_model_file</tt> is used if any,
  otherwise standard output is used.
  <p>
  Command options:<p>
  <dl>
     <dt><tt>-o filename</tt>
       <dd> Attempts to write the flat and boolean SMV model in 
       <tt>filename</tt>.
  </dl>
  ]  

  SideEffects        []

******************************************************************************/
int CommandWriteModelFlatBool(int argc, char **argv)
{
  int c = 0;
  int rv = 0;
  char * output_file = NIL(char);
  FILE * ofileid = NIL(FILE);
  int bSpecifiedFilename = FALSE; 
  
  util_getopt_reset();
  while((c = util_getopt(argc, argv, "ho:")) != EOF) {
    switch(c){
    case 'h': return(UsageWriteModelFlatBool());
    case 'o':
      output_file = ALLOC(char, strlen(util_optarg)+1);
      nusmv_assert(output_file);
      strcpy(output_file, util_optarg);
      bSpecifiedFilename = TRUE; 
      break;

    default:
      break;
    }
  }

  if (argc != util_optind) return(UsageWriteModelFlatBool());

  if (output_file == NIL(char)) { 
    output_file = get_output_boolean_model_file(options);
  }

  if (output_file == NIL(char)) {
    ofileid = nusmv_stdout;
  } 
  else {
    ofileid = fopen(output_file, "w");
    if (ofileid == NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", output_file);
      if (bSpecifiedFilename == TRUE)  FREE(output_file);
      return 1;
    }
  }

  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    return 1;
  }
  if (cmp_struct_get_build_bool_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "The boolean model should be built before. Use the \"build_boolean_model\" command.\n");
    return 1;
  }

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Writing boolean model into file \"%s\"..", output_file == (char *)NULL ? "stdout" : output_file);
  }

  CATCH {
    Compile_WriteFlattenBool(Enc_get_symb_encoding(), ofileid, 
			     Prop_master_get_bool_sexp_fsm(), cmps);
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, ".. done.\n");
    }
  } FAIL {
    rv = 1;
  }
  fflush(ofileid);

  if (ofileid != nusmv_stdout) {
    fclose(ofileid);
    if (bSpecifiedFilename == TRUE)  FREE(output_file);
  }
  return(rv);
}

static int UsageWriteModelFlatBool(void)
{
  fprintf(nusmv_stderr, "usage: write_boolean_model [-h] [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -o filename\tWrites output to \"filename\".\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Initializes the fsm builder if needed, and creates the 
  master scalar fsm]

  Description        [Initializes the fsm builder if needed, and creates the 
  master scalar fsm only if required]

  SideEffects        []

******************************************************************************/
static void compile_create_flat_model()
{ 
  if (Prop_master_get_scalar_sexp_fsm() == SEXP_FSM(NULL)) {
    SexpFsm_ptr sexp_fsm;
    NodeList_ptr vars;

    if (global_fsm_builder == FSM_BUILDER(NULL)) {
      /* to create global_fsm_builder instance */
      Compile_InitializeBuildModel(); 
    }
    
    vars = Encoding_get_all_model_vars_list(Enc_get_symb_encoding());

    sexp_fsm = FsmBuilder_create_sexp_fsm(global_fsm_builder, 
					  Enc_get_symb_encoding(), 
					  NodeList_to_node_ptr(vars), 
					  SEXP_FSM_TYPE_SCALAR);
    
    /* Builds the sexp FSM of the whole read model */
    Prop_master_set_scalar_sexp_fsm(sexp_fsm);
  }
}


static void compile_create_boolean_model()
{
  if (Prop_master_get_bool_sexp_fsm() == SEXP_FSM(NULL)) {
    SexpFsm_ptr sexp_fsm_bool;
    NodeList_ptr vars; 
    int reord_status;
    dd_reorderingtype rt;
    DdManager* dd;

    if (global_fsm_builder == FSM_BUILDER(NULL)) {
      /* to create global_fsm_builder instance */
      Compile_InitializeBuildModel(); 
    }
    
    dd = BddEnc_get_dd_manager(Enc_get_bdd_encoding());
    reord_status = dd_reordering_status(dd, &rt);
    if (reord_status == 1) { dd_autodyn_disable(dd); }

    vars = Encoding_get_all_model_vars_list(Enc_get_symb_encoding());

    sexp_fsm_bool = FsmBuilder_create_sexp_fsm(global_fsm_builder, 
					       Enc_get_symb_encoding(), 
					       NodeList_to_node_ptr(vars), 
					       SEXP_FSM_TYPE_BOOLEAN);

    /* If dynamic reordering was enabled, then it is re-enabled */
    if (reord_status == 1) { dd_autodyn_enable(dd, rt); }
    
    /* Builds the sexp FSM of the whole read model */
    Prop_master_set_bool_sexp_fsm(sexp_fsm_bool);

    /* This is neede to deal correctly with the possible
       determinization vriables introduced while building the boolean
       sexp model */
    Enc_reinit_bdd_encoding();
  }
}
