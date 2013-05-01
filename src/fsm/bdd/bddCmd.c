/**CFile***********************************************************************

  FileName    [bddCmd.c]

  PackageName [bdd]

  Synopsis    [Bdd FSM commands]

  Description [This file contains all the shell commands to dela with
  computation and printing of reachable states, fair states and fair
  transitions.]

  Author      [Marco Roveri]

  Copyright   [ This file is part of the ``mc'' package of NuSMV version 2. 
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

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>.]

******************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "bdd.h" 
#include "bddInt.h" 
#include "parser/symbols.h" 
#include "utils/error.h"
#include "cmd/cmd.h"
#include "utils/utils_io.h"
#include "enc/enc.h"
#include "compile/compile.h"

static char rcsid[] UTIL_UNUSED = "$Id: bddCmd.c,v 1.1.2.4 2004/05/27 12:43:31 nusmv Exp $";

int CommandCheckFsm ARGS((int argc, char **argv));
int CommandComputeReachable ARGS((int argc, char **argv));
int CommandPrintReachableStates ARGS((int argc, char **argv));
int CommandPrintFairStates ARGS((int argc, char **argv));
int CommandPrintFairTransitions ARGS((int argc, char **argv));

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN cmp_struct_ptr cmps;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageCheckFsm ARGS((void));
static int UsageComputeReachable ARGS((void));
static int UsagePrintReachableStates ARGS((void));
static int UsagePrintFairStates ARGS((void));
static int UsagePrintFairTransitions ARGS((void));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the BddFsm package.]

  Description        [Initializes the BddFsm package.]

  SideEffects        []

******************************************************************************/
void Bdd_Init(void){
  Cmd_CommandAdd("check_fsm", CommandCheckFsm, 0);
  Cmd_CommandAdd("print_reachable_states", CommandPrintReachableStates, 0);
  Cmd_CommandAdd("compute_reachable", CommandComputeReachable, 0);
  Cmd_CommandAdd("print_fair_states", CommandPrintFairStates, 0);
  Cmd_CommandAdd("print_fair_transitions", CommandPrintFairTransitions, 0);
}

/**Function********************************************************************

  Synopsis           [Quit the BddFsm package]

  Description        [Quit the BddFsm package]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bdd_End(void)
{ }

/**Function********************************************************************

  Synopsis           [Checks the fsm for totality and deadlock states.]

  CommandName        [check_fsm]         

  CommandSynopsis    [Checks the transition relation for totality.]  

  CommandArguments   [\[-h\] \[-m | -o output-file\]]

  CommandDescription [
  Checks if the transition relation is total. If the transition
  relation is not total then a potential deadlock state is shown out.
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-m</tt>
       <dd> Pipes the output generated by the command to the program
            specified by the <tt>PAGER</tt> shell variable if
            defined, else through the UNIX command "more".
    <dt> <tt>-o output-file</tt>
       <dd> Writes the output generated by the command to the file
       <tt>output-file</tt>.
  </dl>
  At the beginning reachable states are computed in order to guarantee 
  that deadlock states are actually reachable.]

  SideEffects        []

******************************************************************************/
int CommandCheckFsm(int argc, char **argv)
{
  int c;
  int useMore = 0;
  char * dbgFileName = NIL(char);
#if HAVE_GETENV
  char * pager;
#endif
  FILE * old_nusmv_stdout = NIL(FILE);

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"hmo:")) != EOF) {
    switch (c) {
    case 'h': return UsageCheckFsm();
    case 'o':
      if (useMore == 1) return UsageCheckFsm();
      dbgFileName = util_strsav(util_optarg);
      fprintf(nusmv_stdout, "Output to file: %s\n", dbgFileName);
      break;
    case 'm':
      if (dbgFileName != NIL(char)) return UsageCheckFsm();
      useMore = 1;
      break;
    default:  return UsageCheckFsm();
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
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, 
            "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }

  if (!cmp_struct_get_build_model(cmps)) {
    fprintf(nusmv_stderr, 
            "The current partition method %s has not yet be computed.\n", 
            TransType_to_string(get_partition_method(options)));
    fprintf(nusmv_stderr, 
            "Use \t \"build_model -f -m %s\"\nto build the transition relation.\n", 
            TransType_to_string(get_partition_method(options)));
    return 1;
  }
  
  if (argc != util_optind) return UsageCheckFsm();

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

  BddFsm_check_machine(Prop_master_get_bdd_fsm());
  set_forward_search(options);

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

static int UsageCheckFsm()
{
  fprintf(nusmv_stderr, "usage: check_fsm [-h] [-m | -o file]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -m \t\tPipes output through the program specified by\n");
  fprintf(nusmv_stderr, "      \t\tthe \"PAGER\" environment variable if defined,\n");
  fprintf(nusmv_stderr, "      \t\telse through the UNIX command \"more\".\n");
  fprintf(nusmv_stderr, "   -o file\tWrites the generated output to \"file\".\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Enables the future computation of the set of reachable
                      states]

  CommandName        [compute_reachable]           

  CommandSynopsis    [Enables the future computation of the set of reachable
                      states]  

  CommandArguments   [\[-h\]]  

  CommandDescription [Sets a flag in the system that triggers the lazy 
  calculation of the set of reachable states every time it is needed. 

  The set of reachable states is used to simplify image and
  preimage computations. This can result in improved performances for
  models with sparse state spaces. Sometimes this option may slow down
  the performances because the computation of reachable states may be
  very expensive. The environment variable <tt>forward_search</tt> is
  set after the execution of this command.]

  SideEffects        []

******************************************************************************/
int CommandComputeReachable(int argc, char **argv)
{
  int c;
  
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
  if (cmp_struct_get_build_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be built before. Use the \"build_model\" command.\n");
    return 1;
  }
  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"h")) != EOF) {
    switch (c) {
    case 'h': return UsageComputeReachable();
    default:  return UsageComputeReachable();
    }
  }
  if (argc != util_optind) return UsageComputeReachable();

  if (!cmp_struct_get_build_model(cmps)) {
    fprintf(nusmv_stderr, "The current partition method %s has not yet be computed.\n", 
	    TransType_to_string(get_partition_method(options)));
    fprintf(nusmv_stderr, "Use \t \"build_model -f -m %s\"\nto build the transition relation.\n", 
	    TransType_to_string(get_partition_method(options)));
    return 1;
  }

  if (!opt_use_reachable_states(options)) {
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Enabling the use of reachable states...");
    }
    set_use_reachable_states(options);
    set_forward_search(options);
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "...done.\n");
    }
  }
  else {
    fprintf(nusmv_stderr, "Reachable States already enabled.\n");
  }

  return 0;
}

static int UsageComputeReachable()
{
  fprintf(nusmv_stderr, "usage: compute_reachable [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Prints the reachable states.]

  CommandName        [print_reachable_states]      

  CommandSynopsis    [Prints out the number of reachable states. In verbose mode,
  prints also the list of reachable states.]

  CommandArguments   [\[-h\] \[-v\]]  

  CommandDescription [Prints the number of reachable states of the
  given model. In verbose mode, prints also the list of all reachable states.
  The reachable states are computed if needed.]

  SideEffects        []

******************************************************************************/
int CommandPrintReachableStates(int argc, char **argv)
{
  int c;
  boolean verbose = false;

  /* Parse the command line */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hv")) != EOF) {
    switch (c) {
    case 'h': return UsagePrintReachableStates();
    case 'v': 
      verbose = true;
      break;
    default:
      return UsagePrintReachableStates();
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
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }
  if (cmp_struct_get_build_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be built before. Use the \"build_model\" command.\n");
    return 1;
  }

  if (!cmp_struct_get_build_model(cmps)) {
    fprintf(nusmv_stderr, "The current partition method %s has not yet be computed.\n", 
	    TransType_to_string(get_partition_method(options)));
    fprintf(nusmv_stderr, "Use \t \"build_model -f -m %s\"\nto build the transition relation.\n", 
	    TransType_to_string(get_partition_method(options)));
    return 1;
  }

  set_forward_search(options);
  set_print_reachable(options);
  fprintf(nusmv_stdout, 
	  "######################################################################\n");
  BddFsm_print_reachable_states_info(Prop_master_get_bdd_fsm(), verbose, 
				     nusmv_stdout);
  fprintf(nusmv_stdout, 
	  "######################################################################\n");
  return 0;
}

static int UsagePrintReachableStates()
{
  fprintf(nusmv_stderr, "usage: print_reachable_states [-h] [-v]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -v \t\tPrints the list of reachable states.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Prints the fair states.]

  CommandName        [print_fair_states]      

  CommandSynopsis    [Prints out the number of fair states. In verbose mode,
  prints also the list of fair states.]

  CommandArguments   [\[-h\] \[-v\]]  

  CommandDescription []

  SideEffects        []

******************************************************************************/
int CommandPrintFairStates(int argc, char **argv)
{
  int c;
  boolean verbose = false;
  /*
   * Parse the command line.
  */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hv")) != EOF) {
    switch (c) {
    case 'h': return UsagePrintFairStates();
    case 'v': 
      verbose = true;
      break;
    default:
      return UsagePrintFairStates();
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
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }
  if (cmp_struct_get_build_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be built before. Use the \"build_model\" command.\n");
    return 1;
  }

  if (!cmp_struct_get_build_model(cmps)) {
    fprintf(nusmv_stderr, "The current partition method %s has not "
	    "yet be computed.\n", 
	    TransType_to_string(get_partition_method(options)));
    fprintf(nusmv_stderr, "Use \t \"build_model -f -m %s\"\nto build the "
	    "transition relation.\n", 
	    TransType_to_string(get_partition_method(options)));
    return 1;
  }

  fprintf(nusmv_stdout, 
	  "######################################################################\n");
  BddFsm_print_fair_states_info(Prop_master_get_bdd_fsm(), verbose, nusmv_stdout);
  fprintf(nusmv_stdout, 
	  "######################################################################\n");

  return 0;
}

static int UsagePrintFairStates()
{
  fprintf(nusmv_stderr, "usage: print_fair_states [-h] [-v]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -v \t\tPrints the list of fair states.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Prints the fair transitions.]

  CommandName        [print_fair_transitions]      

  CommandSynopsis    [Prints the number of fair transitions. In verbose mode,
  prints also the list of fair states.

   NOTE: THESE ARE NOT REALLY TRANSITIONS, BUT RATHER STATE_INPUT PAIRS.]

  CommandArguments   [\[-h\] \[-v\]]  

  CommandDescription []

  SideEffects        []

******************************************************************************/
int CommandPrintFairTransitions(int argc, char **argv)
{
  int c;
  boolean verbose = false;

  /* Parse the command line */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hv")) != EOF) {
    switch (c) {
    case 'h': return UsagePrintFairTransitions();
    case 'v': 
      verbose = true;
      break;
    default:
      return UsagePrintFairTransitions();
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
  if (cmp_struct_get_encode_variables(cmps) == 0) {
    fprintf(nusmv_stderr, "The variables must be built before. Use the \"encode_variables\" command.\n");
    return 1;
  }
  if (cmp_struct_get_build_model(cmps) == 0) {
    fprintf(nusmv_stderr,
            "A model must be built before. Use the \"build_model\" command.\n");
    return 1;
  }

  if (!cmp_struct_get_build_model(cmps)) {
    fprintf(nusmv_stderr, "The current partition method %s has not yet be computed.\n", 
	    TransType_to_string(get_partition_method(options)));
    fprintf(nusmv_stderr, "Use \t \"build_model -f -m %s\"\nto build the transition relation.\n", 
	    TransType_to_string(get_partition_method(options)));
    return 1;
  }

  fprintf(nusmv_stdout, 
	  "######################################################################\n");
  BddFsm_print_fair_transitions_info(Prop_master_get_bdd_fsm(), verbose, 
				     nusmv_stdout);
  fprintf(nusmv_stdout, 
	  "######################################################################\n");

  return 0;
}

static int UsagePrintFairTransitions()
{
  fprintf(nusmv_stderr, "usage: print_fair_transitions [-h] [-v]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -v \t\tPrints the list of fair transitions.\n");
  return 1;
}
