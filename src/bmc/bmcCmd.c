/**CFile***********************************************************************

  FileName    [bmcCmd.c]

  PackageName [bmc]

  Synopsis    [Bmc.Cmd module]

  Description [This module contains all the bmc commands implementation. 
  Options parsing and checking is performed here, than the high-level Bmc 
  layer is called]

  SeeAlso     [bmcPkg.c, bmcBmc.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by ITC-irst and University of Trento. 

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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "bmcCmd.h"

#include "bmc.h"
#include "bmcInt.h"
#include "bmcBmc.h"
#include "bmcPkg.h"
#include "bmcUtils.h"

#include "prop/prop.h"
#include "enc/enc.h"


static char rcsid[] UTIL_UNUSED = "$Id: bmcCmd.c,v 1.85.4.5.2.18 2005/11/16 12:09:44 nusmv Exp $";

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

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int UsageBmcSetup    ARGS((void));
static int UsageBmcSimulate ARGS((void));

static int UsageBmcGenLtlSpec         ARGS((void));
static int UsageBmcGenLtlSpecOnePb    ARGS((void));
static int UsageBmcCheckLtlSpec       ARGS((void));
static int UsageBmcCheckLtlSpecOnePb  ARGS((void));
static int UsageBmcGenInvar           ARGS((void));
static int UsageBmcCheckInvar         ARGS((void));

#if HAVE_INCREMENTAL_SAT
static int UsageBmcCheckLtlSpecInc    ARGS((void));
static int UsageBmcCheckInvarInc      ARGS((void));
#endif


static outcome 
bmc_cmd_options_handling ARGS((const int argc, const char** argv, 
			       Prop_Type prop_type,
			       /* output parameters: */
			       Prop_ptr* res_prop, 
			       int* res_k, int* res_l, 
			       char** res_a, char** res_o));


/**AutomaticEnd***************************************************************/



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Initializes the bmc sub-system, and builds the model in
  a Boolean Expression format]

  Description        []

  SideEffects        [Overall the bmc system]

  SeeAlso            []

  CommandName        [bmc_setup] 	   

  CommandSynopsis    [Builds the model in a Boolean Epression format.]  

  CommandArguments   [\[-h\]]  

  CommandDescription [You must call this command before use any other 
  bmc-related command. Only one call per session is required.]  

******************************************************************************/
int Bmc_CommandBmcSetup(const int argc, const char** argv)
{
  /* processes the command options */ 
  int c;

  util_getopt_reset();
  while ((c = util_getopt((int)argc, (char**)argv, "h")) != EOF) {
    switch (c) {
    case 'h': return UsageBmcSetup();
    }
  }
  if (argc != 1) return UsageBmcSetup();

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

  if (cmp_struct_get_build_bool_model(cmps) == 0) {
    fprintf(nusmv_stderr, 
	    "The model must be built before. "
	    "(Use the \"build_boolean_model\" command.)\n");
    return 1;
  }

  if (cmp_struct_get_bmc_setup(cmps)) {
    fprintf (nusmv_stderr, "A call to bmc_setup has already been done.\n");
    return 1;
  }

  /* This function does the actual work: */
  Bmc_Init();

  return 0;
}


/**Function********************************************************************

  Synopsis           [Usage string for Bmc_CommandBmcSetup]

  Description        []

  SideEffects        []

  SeeAlso            [Bmc_CommandBmcSetup]
******************************************************************************/
static int UsageBmcSetup (void)
{
  fprintf (nusmv_stderr, "usage: bmc_setup [-h]\n");
  fprintf (nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf (nusmv_stderr, "   \t\tStarts up the BMC module and generates a Boolean Expression\n" 
	   "\t\twhich represents the model.\n");

  return 1;
}


/**Function********************************************************************

  Synopsis           [Bmc_CommandBmcSimulate generates a trace of the problem 
  represented from the simple path from 0 (zero) to k]

  Description        [Bmc_CommandBmcSimulate does not require a specification 
  to build the problem, because only the model is used to build it.]

  SideEffects        [None]

  SeeAlso            []

  CommandName        [bmc_simulate] 	   

  CommandSynopsis    [Generates a trace of the model from 0 (zero) to k]  

  CommandArguments   [\[-h | \[-p | -v\] -k <length>\]]  

  CommandDescription [bmc_simulate does not require a specification 
  to build the problem, because only the model is used to build it. 
  The problem length is represented by the <i>-k</i> command parameter, 
  or by its default value stored in the environment variable 
  <i>bmc_length</i>.<BR>
  Command options:<p>
  <dl>
    <dt> <tt>-p <i>prints out the generated trace, with only variables whose  
     values had changed</i>.</tt> <BR>
    <dt> <tt>-v <i>prints out the generated trace</i>.</tt> <BR>
    <dt> <tt>-k <i>length</i></tt>
       <dd> <i>length</i> is the length of the generated simulation. <BR>
  </dl>
  ]  

******************************************************************************/
int Bmc_CommandBmcSimulate(const int argc, const char** argv)
{
  int c; /* for command line options */
  int k = get_bmc_pb_length(options);
  boolean print = false;
  boolean changes_only = true;
  boolean p_specified = false;
  boolean v_specified = false;
  boolean k_specified = false;
 
  /* process command options */ 
  util_getopt_reset();
  while((c = util_getopt((int)argc, (char**)argv, "hpvk:")) != EOF) {
    switch (c) {
    case 'h':
      return UsageBmcSimulate();

    case 'p':
      if (v_specified) {
	fprintf(nusmv_stderr, "Options -v and -p are mutually exclusive.\n");
	return 1;
      }
      print = true;
      changes_only = true;
      p_specified = true;
      break;

    case 'v':
      if (p_specified) {
	fprintf(nusmv_stderr, "Options -v and -p are mutually exclusive.\n");
	return 1;
      }
      print = true;
      changes_only = false;
      v_specified= true;
      break;   
      
    case 'k':
      {
	char* strNumber;

	if (k_specified) {
	  fprintf(nusmv_stderr, 
		  "Option -k cannot be specified more than once.\n");
	  return 1;
	}

	strNumber = util_strsav(util_optarg);
	if (util_str2int(strNumber, &k) != 0) {
	  error_invalid_number(strNumber);
	  return 1;
	}
	if (k<0) {
 	  error_invalid_number(strNumber);
	  return 1;
	}

	k_specified = true;
	break;  
      }
      
    default:  return 1; 

    } /* switch stmt */
  } /* while */ 
  
  if (argc != util_optind) {
    return UsageBmcSimulate();
  }
    
  /* make sure the model hierarchy has been flattened */
  if (cmp_struct_get_flatten_hrc (cmps) == 0) {
    fprintf (nusmv_stderr, "The hierarchy must be flattened before. Use "\
	     "the \"flatten_hierarchy\" command.\n");
    
    return 1;
  }

  /* make sure bmc has been set up */
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	     "the \"bmc_setup\" command.\n");
    return 1;
  }
  
  /* This function does the actual work: */
  Bmc_Simulate(Prop_master_get_be_fsm(), k, print, changes_only);

  return 0;
}


/**Function********************************************************************

  Synopsis           [Usage string for UsageBmcSimulate]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandBmcSimulate]

******************************************************************************/
static int UsageBmcSimulate (void)
{
  fprintf (nusmv_stderr, "usage: bmc_simulate [-h] [-p | -v ] [-k length]\n");
  fprintf (nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf (nusmv_stderr, 
	   "  -p \t\tPrints the generated trace (only changed variables).\n");
  fprintf (nusmv_stderr, 
	   "  -v \t\tPrints the generated trace (all variables).\n");

  fprintf (nusmv_stderr, "  -k <length> \tSpecifies the simulation length\n" 
           "\t\tto be used when generating the simulated problem.\n");
  fprintf (nusmv_stderr, 
	   "\tGenerates a k-steps simulation using Bounded Model Checking.\n" 
	   "\tYou can specify k also by setting the variable bmc_length.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Generates length_max+1 problems iterating the problem 
  bound from zero to length_max, and dumps each problem to a dimacs file]

  Description        [Each problem is dumped for the given LTL specification, 
  or for all LTL specifications if no formula is given. 
  Generation parameters are the maximum bound and the loopback values. <BR>
  After command line processing it calls the function Bmc_GenSolveLtl 
  to generate and dump all problems from zero to k.]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_CommandGenLtlSpecBmcOnePb, Bmc_GenSolveLtl]

  CommandName        [gen_ltlspec_bmc] 	   

  CommandSynopsis    [Dumps into one or more dimacs files the given LTL 
  specification, or all LTL specifications if no formula is given. 
  Generation and dumping parameters are the maximum bound and the loopback 
  values]  

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\]\] 
  \[-k max_length\] \[-l loopback\] \[-o filename\]]  

  CommandDescription [  This command generates one or more problems, and  
  dumps each problem into a dimacs file. Each problem is related to a specific
  problem bound, which increases from zero (0) to the given maximum problem 
  bound. In this short description "<i>length</i>" is the bound of the 
  problem that system is going to dump out. <BR>
  In this context the maximum problem bound is represented by the 
  <i>max_length</i> parameter, or by its default value stored in the 
  environment variable <i>bmc_length</i>.<BR>
  Each dumped problem also depends on the loopback you can explicitly 
  specify by the <i>-l</i> option, or by its default value stored in the 
  environment variable <i>bmc_loopback</i>. <BR>
  The property to be checked may be specified using the <i>-n idx</i> or  
  the <i>-p "formula"</i> options. <BR>
  You may specify dimacs file name by using the option <i>-o "filename"</i>, 
  otherwise the default value stored in the environment variable 
  <i>bmc_dimacs_filename</i> will be considered.<BR>  
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification 
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula \[IN context\]"</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> <i>max_length</i> is the maximum problem bound used when 
       increasing problem bound starting from zero. Only natural number are
       valid values for this option. If no value is given the environment 
       variable <i>bmc_length</i> value is considered instead. 
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>max_length-1</i>). Positive sign ('+') can 
       be also used as prefix of the number. Any invalid combination of bound 
       and loopback will be skipped during the generation and 
       dumping process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case 
       <i>loopback</i> is considered a value relative to <i>max_length</i>. 
       Any invalid combination of bound and loopback will be skipped during 
       the generation process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to 
       <i>length-1</i>"
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of dumped dimacs files, without 
       extension. <BR>
       If this options is not specified, variable <i>bmc_dimacs_filename</i> 
       will be considered. The file name string may contain special symbols 
       which will be macro-expanded to form the real file name. 
       Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @k: current problem bound <BR>
       - @l: current loopback value <BR>
       - @n: index of the currently processed formula in the properties 
       database <BR>
       - @@: the '@' character
  </dl>]  
   

******************************************************************************/
int Bmc_CommandGenLtlSpecBmc(const int argc, const char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  outcome opt_handling_res; 
  int k = get_bmc_pb_length(options);
  char* fname = (char*) NULL;
  int relative_loop = Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), 
						      NULL);


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = bmc_cmd_options_handling(argc, argv, 
					      Prop_Ltl, &ltlprop, 
					      &k, &relative_loop, 
					      NULL, &fname);
  
  if (opt_handling_res == SUCCESS_REQUIRED_HELP) {
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcGenLtlSpec();
  }

  if (opt_handling_res != SUCCESS) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure the model hierarchy has been flattened */
  if (cmp_struct_get_flatten_hrc (cmps) == 0) {
    fprintf (nusmv_stderr, "The hierarchy must be flattened before. Use "\
	     "the \"flatten_hierarchy\" command.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure bmc has been set up */
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	     "the \"bmc_setup\" command.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  if (fname == (char*) NULL) {
    fname = util_strsav(get_bmc_dimacs_filename(options));
  }

  /* prepare the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsList props = PropDb_get_props_of_type(Prop_Ltl);
    lsGen  iterator; 
    Prop_ptr prop;

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveLtl(prop, k, relative_loop, 
			  true, /* iterate on k */ 
			  false, /* do not solve */
			  BMC_DUMP_DIMACS, fname) != 0) {
	FREE(fname);
	return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve: */
    if (Bmc_GenSolveLtl(ltlprop, k, relative_loop, 
			true, /* iterate on k */
			false, /* do not solve */
			BMC_DUMP_DIMACS, fname) != 0) {
      FREE(fname);
      return 1;
    }
  }

  FREE(fname);
  return 0; 
}


/**Function********************************************************************

  Synopsis           [Usage string for command gen_ltlspec_bmc]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandGenLtlSpecBmc]

******************************************************************************/
static int UsageBmcGenLtlSpec(void)
{
  fprintf(nusmv_stderr, "\nUsage: gen_ltlspec_bmc [-h | -n idx | -p \"formula\"] [-k max_length] [-l loopback]\n\t\t\t [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties.\n"); 
  fprintf(nusmv_stderr, "  -k max_length\tChecks the property using <max_length> value instead of using the\n\t\tvariable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value.\n");
  fprintf(nusmv_stderr, "  -o filename\tUses <filename> as dimacs file instead of using the\n\t\t\"bmc_dimacs_filename\" variable. <filename> may contain patterns.\n\n"); 

  return 1;
}


/**Function********************************************************************

  Synopsis           [Generates only one problem with fixed bound and 
  loopback, and dumps the problem to a dimacs file. The single problem 
  is dumped for the given LTL specification, or for all LTL 
  specifications if no formula is given]

  Description        [After command line processing it calls
  the function Bmc_GenSolveLtl to generate and dump the single problem.]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_CommandGenLtlSpecBmc, Bmc_GenSolveLtl]

  CommandName        [gen_ltlspec_bmc_onepb] 	   

  CommandSynopsis    [Dumps into one dimacs file the problem generated for
  the given LTL specification, or for all LTL specifications if no formula 
  is explicitly given. 
  Generation and dumping parameters are the problem bound and the loopback 
  values]  

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\]\] \[-k length\] 
  \[-l loopback\] \[-o filename\]]  

  CommandDescription [ As the <i>gen_ltlspec_bmc</i> command, but it generates
  and dumps only one problem given its bound and loopback. <BR>
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification 
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula \[IN context\]"</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-k <i>length</i></tt>
       <dd> <i>length</i> is the single problem bound used to generate and 
       dump it. Only natural number are valid values for this option. 
       If no value is given the environment variable <i>bmc_length</i> 
       is considered instead. 
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>length-1</i>). Positive sign ('+') can 
       be also used as prefix of the number. Any invalid combination of length 
       and loopback will be skipped during the generation and dumping 
       process.<BR>
       - a negative number in (-1, -<i>length</i>). 
       Any invalid combination of length and loopback will be skipped during 
       the generation process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to 
       <i>length-1</i>"
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file, without 
       extension. <BR>
       If this
       options is not specified, variable <i>bmc_dimacs_filename</i> will be
       considered. The file name string may contain special symbols which 
       will be macro-expanded to form the real file name. 
       Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @k: current problem bound <BR>
       - @l: current loopback value <BR>
       - @n: index of the currently processed formula in the properties 
       database <BR>
       - @@: the '@' character
  </dl>]  

******************************************************************************/
int Bmc_CommandGenLtlSpecBmcOnePb(const int argc, const char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  outcome opt_handling_res; 
  int k = get_bmc_pb_length(options);
  char* fname = (char*) NULL;
  int relative_loop = 
    Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), NULL);


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = bmc_cmd_options_handling(argc, argv, 
					      Prop_Ltl, &ltlprop, 
					      &k, &relative_loop, 
					      NULL, &fname);

  if (opt_handling_res == SUCCESS_REQUIRED_HELP) {
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcGenLtlSpecOnePb();
  }

  if (opt_handling_res != SUCCESS) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure the model hierarchy has been flattened */
  if (cmp_struct_get_flatten_hrc (cmps) == 0) {
    fprintf (nusmv_stderr, "The hierarchy must be flattened before. Use "\
	     "the \"flatten_hierarchy\" command.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure bmc has been set up */
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	     "the \"bmc_setup\" command.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* ----------------------------------------------------------------------- */

  if (fname == (char*) NULL) {
    fname = util_strsav(get_bmc_dimacs_filename(options)); 
  } 

  /* prepare the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsList props = PropDb_get_props_of_type(Prop_Ltl);
    lsGen  iterator; 
    Prop_ptr prop;

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveLtl(prop, k, relative_loop, 
			  false, /* do not iterate k */
			  false, /* do not solve */
			  BMC_DUMP_DIMACS, fname) != 0) {
	FREE(fname);
	return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve: */
    if (Bmc_GenSolveLtl(ltlprop, k, relative_loop, 
			false, /* do not iterate k */
			false, /* do not solve */
			BMC_DUMP_DIMACS, fname) != 0) {
      FREE(fname);
      return 1;
    }
  }

  FREE(fname);
  return 0; 
}


/**Function********************************************************************

  Synopsis           [Usage string for command gen_ltlspec_bmc_onepb]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandGenLtlSpecBmcOnePb]

******************************************************************************/
static int UsageBmcGenLtlSpecOnePb(void)
{
  fprintf(nusmv_stderr, "\nUsage: gen_ltlspec_bmc_onepb [-h | -n idx | -p \"formula\"] [-k length] [-l loopback]\n\t\t\t [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties.\n"); 
  fprintf(nusmv_stderr, "  -k length\tChecks the property using <length> value instead of using the\n\t\tvariable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value.\n");
  fprintf(nusmv_stderr, "  -o filename\tUses <filename> as dimacs file instead of \"bmc_dimacs_filename\"\n\t\tvariable. <filename> may contain patterns.\n\n"); 

  return 1;
}


/**Function********************************************************************

  Synopsis           [Checks the given LTL specification, or all LTL 
  specifications in the properties database if no formula is given]

  Description        [After command line processing this function calls
  the Bmc_GenSolveLtl to generate and solve all problems from 0 to k.
  Parameters are the maximum length and the loopback values.]

  SideEffects        [Properties database may change]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmcOnePb, Bmc_GenSolveLtl]

  CommandName        [check_ltlspec_bmc] 	   

  CommandSynopsis    [Checks the given LTL specification, or all LTL 
  specifications if no formula is given. Checking parameters are the maximum 
  length and the loopback values]  

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\]\] 
  \[-k max_length\] \[-l loopback\] \[-o filename\]]  

  CommandDescription [
  This command generates one or more problems, and calls 
  SAT solver for each one. Each problem is related to a specific problem 
  bound, which increases from zero (0) to the given maximum problem 
  length. Here "<i>length</i>" is the bound of the problem that system 
  is going to generate and/or solve. <BR>
  In this context the maximum problem bound is represented by the 
  <i>-k</i> command parameter, or by its default value stored in the 
  environment variable <i>bmc_length</i>.<BR>
  The single generated problem also depends on the "<i>loopback</i>"
  parameter you can explicitly specify by the <i>-l</i> option, or by its
  default value stored in the environment variable <i>bmc_loopback</i>. <BR>
  The property to be checked may be specified using the <i>-n idx</i> or  
  the <i>-p "formula"</i> options. <BR>
  If you need to generate a dimacs dump file of all generated problems, you 
  must use the option <i>-o "filename"</i>. <BR>  
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification 
       formula actually located in the properties database. <BR>
    <dt> <tt>-p "formula" \[IN context\]</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> <i>max_length</i> is the maximum problem bound must be reached. 
       Only natural number are valid values for this option. If no value 
       is given the environment variable <i>bmc_length</i> is considered 
       instead. 
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>max_length-1</i>). Positive sign ('+') can 
       be also used as prefix of the number. Any invalid combination of length
       and loopback will be skipped during the generation/solving process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case 
       <i>loopback</i> is considered a value relative to <i>max_length</i>. 
       Any invalid combination of length and loopback will be skipped 
       during the generation/solving process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to 
       <i>length-1</i>"
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file, without 
       extension. <BR>
       It may contain special symbols which will be macro-expanded to form 
       the real file name. Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @k: current problem bound <BR>
       - @l: current loopback value <BR>
       - @n: index of the currently processed formula in the properties 
       database <BR>
       - @@: the '@' character
  </dl>
  ]  

******************************************************************************/
int Bmc_CommandCheckLtlSpecBmc(const int argc, const char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  outcome opt_handling_res; 
  int k = get_bmc_pb_length(options);
  char* fname = (char*) NULL;
  int relative_loop = 
    Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), NULL);


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = bmc_cmd_options_handling(argc, argv, 
					      Prop_Ltl, &ltlprop, 
					      &k, &relative_loop, 
					      NULL, &fname);
  							  
  if (opt_handling_res == SUCCESS_REQUIRED_HELP) {
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcCheckLtlSpec();
  }

  if (opt_handling_res != SUCCESS) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* makes sure the model hierarchy has been flattened */
  if (cmp_struct_get_flatten_hrc (cmps) == 0) {
    fprintf (nusmv_stderr, "The hierarchy must be flattened before. Use "\
	     "the \"flatten_hierarchy\" command.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* makes sure bmc has been set up */
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	     "the \"bmc_setup\" command.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  /* prepares the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsList props = PropDb_get_props_of_type(Prop_Ltl);
    lsGen  iterator; 
    Prop_ptr prop;

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveLtl(prop, k, relative_loop, true, 
		  true, /* solve */
		  (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE, 
		  fname) != 0) {
	if (fname != (char*) NULL) FREE(fname);
	return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve (a single property): */
    if (Bmc_GenSolveLtl(ltlprop, k, relative_loop, true, 
		true, /* solve */
		(fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE, 
		fname) != 0) {
      if (fname != (char*) NULL) FREE(fname);
      return 1;
    }
  }

  if (fname != (char*) NULL) FREE(fname);
  return 0; 
}


/**Function********************************************************************

  Synopsis           [Usage string for command check_ltlspec_bmc]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmc]

******************************************************************************/
static int UsageBmcCheckLtlSpec(void)
{
  fprintf(nusmv_stderr, "\nUsage: check_ltlspec_bmc [-h | -n idx | -p \"formula\"] [-k max_length] [-l loopback]\n\t\t\t [-o <filename>]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties.\n"); 
  fprintf(nusmv_stderr, "  -k max_length\tChecks the property using <max_length> value instead of using the\n\t\tvariable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value.\n");
  fprintf(nusmv_stderr, "  -o filename\tGenerates dimacs output file too. <filename> may contain patterns.\n\n"); 

  return 1;
}


/**Function********************************************************************

  Synopsis           [Checks the given LTL specification, or all LTL 
  specifications if no formula is given. Checking parameters are the problem 
  bound and the loopback values]

  Description        [After command line processing this function calls 
  the Bmc_GenSolveLtl which generates and solve the singleton 
  problem with bound k and loopback l. <BR> 
  ] 

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmc, Bmc_GenSolveLtl]

  CommandName        [check_ltlspec_bmc_onepb] 	   

  CommandSynopsis    [Checks the given LTL specification, or all LTL 
  specifications if no formula is given. Checking parameters are the single
  problem bound and the loopback values]  

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\]\] 
  \[-k length\] \[-l loopback\] \[-o filename\]]  

  CommandDescription [As command check_ltlspec_bmc but it produces only one 
  single problem with fixed bound and loopback values, with no iteration
  of the problem bound from zero to max_length. <BR>  
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification 
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula \[IN context\]"</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-k <i>length</i></tt>
       <dd> <i>length</i> is the problem bound used when generating the 
       single problem. Only natural number are valid values for this option. 
       If no value is given the environment variable <i>bmc_length</i> is 
       considered instead. 
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>max_length-1</i>). Positive sign ('+') can 
       be also used as prefix of the number. Any invalid combination of length 
       and loopback will be skipped during the generation/solving process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case 
       <i>loopback</i> is considered a value relative to <i>length</i>. 
       Any invalid combination of length and loopback will be skipped 
       during the generation/solving process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to 
       <i>length-1</i>"
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file, without
       extension.<BR>
       It may contain special symbols which will be macro-expanded to form 
       the real file name. Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @k: current problem bound <BR>
       - @l: current loopback value <BR>
       - @n: index of the currently processed formula in the properties 
       database <BR>
       - @@: the '@' character
  </dl>]  

******************************************************************************/
int Bmc_CommandCheckLtlSpecBmcOnePb(const int argc, const char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  outcome opt_handling_res; 
  int k = get_bmc_pb_length(options); 
  char* fname = (char*) NULL;
  int relative_loop =
    Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), NULL);

 
  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = bmc_cmd_options_handling(argc, argv, 
					      Prop_Ltl, &ltlprop, 
					      &k, &relative_loop, 
					      NULL, &fname);

  if (opt_handling_res == SUCCESS_REQUIRED_HELP) {
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcCheckLtlSpecOnePb();
  }

  if (opt_handling_res != SUCCESS) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure the model hierarchy has been flattened */
  if (cmp_struct_get_flatten_hrc (cmps) == 0) {
    fprintf (nusmv_stderr, "The hierarchy must be flattened before. Use "\
	     "the \"flatten_hierarchy\" command.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure bmc has been set up */
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	     "the \"bmc_setup\" command.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* ----------------------------------------------------------------------- */


  /* prepare the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsList props = PropDb_get_props_of_type(Prop_Ltl);
    lsGen  iterator; 
    Prop_ptr prop;

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveLtl(prop, k, relative_loop, 
	      false, /* do not iterate k */ 
	      true, /* to always solve */
	      (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
              fname) != 0) {
	if (fname != (char*) NULL) FREE(fname);
	return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve: */
    if (Bmc_GenSolveLtl(ltlprop, k, relative_loop, 
		false, /* do not iterate k */
		true, /* to always solve */
		(fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
		fname) != 0) {
      if (fname != (char*) NULL) FREE(fname);
      return 1;
    }
  }

  if (fname != (char*) NULL) FREE(fname);
  return 0; 
}


/**Function********************************************************************

  Synopsis           [Usage string for command check_ltlspec_bmc_onepb]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmcOnePb]

******************************************************************************/
static int UsageBmcCheckLtlSpecOnePb(void)
{
  fprintf(nusmv_stderr, "\nUsage: check_ltlspec_bmc_onepb [-h | -n idx | -p \"formula\"] [-k length] [-l loopback]\n\t\t\t [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties.\n"); 
  fprintf(nusmv_stderr, "  -k length\tChecks the property using <length> value instead of using the\n\t\tvariable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value.\n");
  fprintf(nusmv_stderr, "  -o filename\tGenerates dimacs output file too. <filename> may contain patterns.\n\n"); 

  return 1;
}

#if HAVE_INCREMENTAL_SAT
/**Function********************************************************************

  Synopsis          [Checks the given LTL specification, or all LTL
  specifications in the properties database if no formula is given,
  using incremental algorithms]

  Description        [Parameters are the maximum length and the loopback
  values. The function is compiled only if there is at least
  one incremental SAT solver]

  SideEffects        [Properties database may change]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmcOnePb, Bmc_CommandCheckLtlSpecBmc]

  CommandName        [check_ltlspec_bmc_inc] 	   

  CommandSynopsis    [Checks the given LTL specification, or all LTL 
  specifications if no formula is given, using incremental algorithms.
  Checking parameters are the maximum length and the loopback values]  

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\]\] 
  \[-k max_length\] \[-l loopback\] ]  

  CommandDescription [
  This command generates one or more problems, and calls (incremental)
  SAT solver for each one. Each problem is related to a specific problem 
  bound, which increases from zero (0) to the given maximum problem 
  length. Here "<i>length</i>" is the bound of the problem that system 
  is going to generate and/or solve. <BR>
  In this context the maximum problem bound is represented by the 
  <i>-k</i> command parameter, or by its default value stored in the 
  environment variable <i>bmc_length</i>.<BR>
  The single generated problem also depends on the "<i>loopback</i>"
  parameter you can explicitly specify by the <i>-l</i> option, or by its
  default value stored in the environment variable <i>bmc_loopback</i>. <BR>
  The property to be checked may be specified using the <i>-n idx</i> or  
  the <i>-p "formula"</i> options. <BR>
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification 
       formula actually located in the properties database. <BR>
    <dt> <tt>-p "formula" \[IN context\]</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> <i>max_length</i> is the maximum problem bound must be reached. 
       Only natural number are valid values for this option. If no value 
       is given the environment variable <i>bmc_length</i> is considered 
       instead. 
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>max_length-1</i>). Positive sign ('+') can 
       be also used as prefix of the number. Any invalid combination of length
       and loopback will be skipped during the generation/solving process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case 
       <i>loopback</i> is considered a value relative to <i>max_length</i>. 
       Any invalid combination of length and loopback will be skipped 
       during the generation/solving process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to 
       <i>length-1</i>"
  </dl>
  ]  

******************************************************************************/
int Bmc_CommandCheckLtlSpecBmcInc(const int argc, const char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  outcome opt_handling_res; 
  int k = get_bmc_pb_length(options);
  int relative_loop = 
    Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), NULL);

  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = bmc_cmd_options_handling(argc, argv, 
					      Prop_Ltl, &ltlprop, 
					      &k, &relative_loop, 
					      NULL, NULL);
  							  
  if (opt_handling_res == SUCCESS_REQUIRED_HELP) {
    return UsageBmcCheckLtlSpecInc();
  }
  if (opt_handling_res != SUCCESS)  return 1;

  /* makes sure the model hierarchy has been flattened */
  if (cmp_struct_get_flatten_hrc (cmps) == 0) {
    fprintf (nusmv_stderr, "The hierarchy must be flattened before. Use "\
	     "the \"flatten_hierarchy\" command.\n");
    return 1;
  }

  /* makes sure bmc has been set up */
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	     "the \"bmc_setup\" command.\n");
    return 1;
  }

  /* ----------------------------------------------------------------------- */

  /* prepares the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsList props = PropDb_get_props_of_type(Prop_Ltl);
    lsGen  iterator; 
    Prop_ptr prop;

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveLtlInc(prop, k, relative_loop, true) != 0) return 1;
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve (a single property): */
    if (Bmc_GenSolveLtlInc(ltlprop, k, relative_loop, true) != 0) return 1;
  }

  return 0; 
}
#endif

#if HAVE_INCREMENTAL_SAT
/**Function********************************************************************

  Synopsis           [Usage string for command check_ltlspec_bmc_inc]

  Description        [The function is compiled only if there is at least
  one incremental SAT solver.]

  SideEffects        [None]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmc]

******************************************************************************/
static int UsageBmcCheckLtlSpecInc(void)
{
  fprintf(nusmv_stderr, "\nUsage: check_ltlspec_bmc_inc [-h | -n idx | -p \"formula\"] [-k max_length] [-l loopback]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx> \n"
                        "        \t(using incremental algorithms).\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property (using incremental algorithms).\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties (using \n"
	                "\t\tincremental algorithms).\n"); 
  fprintf(nusmv_stderr, "  -k max_length\tChecks the property using <max_length> value instead of using \n\t\tthe variable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value.\n\n");
  return 1;
}
#endif

/**Function********************************************************************

  Synopsis           [Generates and dumps the problem for the given 
  invariant or for all invariants if no formula is given. SAT solver is not
  invoked.]

  Description        [After command line processing calls Bmc_GenSolveInvar 
  to dump the generated invariant problem. 
  If you specify the <i>-o "filename"</i> option a dimacs file named  
  "filename" will be created, otherwise the environment variable 
  <i>bmc_invar_dimacs_filename</i> value will be considered.]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_GenSolveInvar]

  CommandName        [gen_invar_bmc] 	   

  CommandSynopsis    [Generates the given invariant, or all 
  invariants if no formula is given]  

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\]\] 
  \[-o filename\]]  

  CommandDescription [<p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid INVAR specification 
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula" \[IN context\]</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file, 
       without extension. <BR> 
       If you 
       do not use this option the dimacs file name is taken from the 
       environment variable <i>bmc_invar_dimacs_filename</i>. <BR> 
       File name may contain special symbols which will be macro-expanded 
       to form the real dimacs file name. Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @n: index of the currently processed formula in the properties 
       database <BR>
       - @@: the '@' character
  </dl>]  

******************************************************************************/
int Bmc_CommandGenInvarBmc(const int argc, const char** argv)
{
  Prop_ptr invarprop = PROP(NULL);   /* The property being processed */
  outcome opt_handling_res; 
  char* fname = (char*) NULL;
  char* algorithm_name = (char*) NULL; 


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = bmc_cmd_options_handling(argc, argv, 
					      Prop_Invar, &invarprop, 
					      NULL, NULL, 
					      &algorithm_name, &fname);
  
  if (opt_handling_res == SUCCESS_REQUIRED_HELP) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcGenInvar();
  }

  if (opt_handling_res != SUCCESS) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure the model hierarchy has been flattened */
  if (cmp_struct_get_flatten_hrc (cmps) == 0) {
    fprintf (nusmv_stderr, "The hierarchy must be flattened before. Use "\
	     "the \"flatten_hierarchy\" command.\n");
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure bmc has been set up */
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	     "the \"bmc_setup\" command.\n");
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  if (fname == (char*) NULL) {
    fname = util_strsav(get_bmc_invar_dimacs_filename(options)); 
  } 
  
  /* Checks algorithms: */
  if (algorithm_name == (char*) NULL) {
    algorithm_name = util_strsav((char*) get_bmc_invar_alg(options));
  }
  
  if (strcasecmp(algorithm_name, BMC_INVAR_ALG_CLASSIC) != 0) {
    fprintf (nusmv_stderr, 
	     "Generation of invariants are allowed only with " 
	     "'" BMC_INVAR_ALG_CLASSIC "'"
	     " algorithm.\n");
    FREE(algorithm_name);
    FREE(fname);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  FREE(algorithm_name);  

  /* prepare the list of properties if no property was selected: */
  if (invarprop == PROP(NULL)) {
    lsList props = PropDb_get_props_of_type(Prop_Invar);
    lsGen  iterator; 
    Prop_ptr prop;

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveInvar(prop,
			    false, /*do not solve */
			    BMC_DUMP_DIMACS, fname) != 0) {
	FREE(fname);
	return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to generate dimacs: */
    if (Bmc_GenSolveInvar(invarprop,
			  false, /*do not solve */
			  BMC_DUMP_DIMACS, fname) != 0) {
      FREE(fname);
      return 1;
    }
  }

  FREE(fname);
  return 0; 
}


/**Function********************************************************************

  Synopsis           [Usage string for command gen_invar_bmc]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandGenInvarBmc]

******************************************************************************/
static int UsageBmcGenInvar(void)
{
  fprintf(nusmv_stderr, "\nUsage: gen_invar_bmc [-h | -n idx | -p \"formula\"] [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the INVAR property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified INVAR propositional property.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all INVAR properties.\n"); 
  fprintf(nusmv_stderr, "  -o filename\tUses <filename> as dimacs file instead of using the\n\t\t\"bmc_invar_dimacs_filename\" variable. <filename> may contain patterns.\n\n"); 

  return 1;
}


/**Function********************************************************************

  Synopsis           [Generates and solve the given invariant, or all 
  invariants if no formula is given]

  Description        [After command line processing calls Bmc_GenSolveInvar 
  to solve and eventually dump the generated invariant problem. If you specify
  the <i>-o "filename"</i> option a dimacs file will be generated, otherwise
  no dimacs dump will be performed]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_GenSolveInvar]

  CommandName        [check_invar_bmc] 	   

  CommandSynopsis    [Generates and solve the given invariant, or all 
  invariants if no formula is given]  

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\]\] 
  \[-k max_length\] \[-a algorithm\] \[-o filename\] ]  

  CommandDescription [<p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid INVAR specification 
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula \[IN context\]"</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-k <i>max_length</i></tt> 
       <dd> (Use only when selected algorithm is een-sorensson). 
            Use to specify the maximal deepth to be reached by the een-sorensson 
            invariant checking algorithm. If not specified, the value assigned 
	    to the system variable <i>bmc_length</i> is taken. 
    <dt> <tt>-a <i>algorithm</i></tt> 
       <dd> Uses the specified algorithm to solve the invariant. If used, this 
            option will override system variable <i>bmc_invar_alg</i>. 
            At the moment, possible values are: "classic", "een-sorensson".
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file, without 
       extension. <BR>
       It may contain special symbols which will be macro-expanded to form 
       the real file name. Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @n: index of the currently processed formula in the properties 
       database <BR>
       - @@: the '@' character
  </dl>]  

******************************************************************************/
int Bmc_CommandCheckInvarBmc(const int argc, const char** argv)
{
  Prop_ptr invarprop = PROP(NULL);   /* The property being processed */
  outcome opt_handling_res; 
  char* fname = (char*) NULL;
  char* algorithm_name = (char*) NULL;   
  int max_k = -1;
  boolean use_classic_alg = true;
  int res = 0;


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = bmc_cmd_options_handling(argc, argv, 
					      Prop_Invar, &invarprop, 
					      &max_k, NULL, 
					      &algorithm_name, &fname);
  
  if (opt_handling_res == SUCCESS_REQUIRED_HELP) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcCheckInvar();
  }

  if (opt_handling_res != SUCCESS) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure the model hierarchy has been flattened */
  if (cmp_struct_get_flatten_hrc (cmps) == 0) {
    fprintf (nusmv_stderr, "The hierarchy must be flattened before. Use "\
	     "the \"flatten_hierarchy\" command.\n");
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure bmc has been set up */
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	     "the \"bmc_setup\" command.\n");
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  /* Checks algorithms: */
  if (algorithm_name == (char*) NULL) {
    algorithm_name = util_strsav((char*) get_bmc_invar_alg(options));
  }

  if ((strcasecmp(algorithm_name, BMC_INVAR_ALG_CLASSIC) != 0) && 
      (strcasecmp(algorithm_name, BMC_INVAR_ALG_EEN_SORENSSON) != 0)) {
    fprintf (nusmv_stderr, 
	     "'%s' is an invalid algorithm name.\n"
	     "Valid names are "
	     "'" BMC_INVAR_ALG_CLASSIC "'"
	     " and "
	     "'" BMC_INVAR_ALG_EEN_SORENSSON "'.\n", algorithm_name);
    FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* ----------------------------------------------------------------------- */

  /* choses the algorithm: */
  use_classic_alg = (strcasecmp(algorithm_name, BMC_INVAR_ALG_CLASSIC) == 0);
  FREE(algorithm_name);

  /* checks length: */
  if (use_classic_alg && max_k != -1) {
    fprintf (nusmv_stderr, 
	     "Option -k can be used only when '" 
	     BMC_INVAR_ALG_EEN_SORENSSON "' algorithm is selected.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* if not specified, selects length from bmc_pb_length */
  if (max_k == -1) {
    max_k = get_bmc_pb_length(options);
  }


  /* prepare the list of properties if no property was selected: */
  if (invarprop == PROP(NULL)) {
    lsList props = PropDb_get_props_of_type(Prop_Invar);
    lsGen  iterator; 
    Prop_ptr prop;

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (use_classic_alg) {
	res = Bmc_GenSolveInvar(prop, 
		 true, /* solve */
		 (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
               	 fname);
      }
      else {
	res = Bmc_GenSolveInvar_EenSorensson(prop, max_k, 
		     (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
               	     fname);
      }
      if (res != 0) break;
    } /* for loop */

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    if (use_classic_alg) {
      res = Bmc_GenSolveInvar(invarprop, 
	      true,  /* solve */
	      (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
	      fname);
    }
    else {
      res = Bmc_GenSolveInvar_EenSorensson(invarprop, max_k, 
		   (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
         	   fname);
    }
  }

  if (fname != (char*) NULL) FREE(fname);
  return res; 
}

/**Function********************************************************************

  Synopsis           [Usage string for command check_invar_bmc]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandCheckInvarBmc]

******************************************************************************/
static int UsageBmcCheckInvar(void)
{
  fprintf(nusmv_stderr, "\nUsage: check_invar_bmc [-h | -n idx | -p \"formula\"] [-k max_len] [-a alg] [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the INVAR property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified INVAR propositional property.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all INVAR properties.\n"); 
  fprintf(nusmv_stderr, "  -k max_len\tUpper bound for the search.\n"
	  "\t\tUse only when " BMC_INVAR_ALG_EEN_SORENSSON " algorithm is selected.\n"
	  "\t\tIf not specified, variable bmc_length is taken.\n");
  fprintf(nusmv_stderr, "  -a alg\tUses the specified algorithm. \n");
  fprintf(nusmv_stderr, "\t\tValid values are: " 
	  BMC_INVAR_ALG_CLASSIC ", " BMC_INVAR_ALG_EEN_SORENSSON
	  "\n\t\tDefault value is taken from variable bmc_invar_alg.\n");
  fprintf(nusmv_stderr, "  -o filename\tGenerates dimacs output file too. <filename> may contain patterns.\n\n");

  return 1;
}


#if HAVE_INCREMENTAL_SAT
/**Function********************************************************************

  Synopsis           [Solve the given invariant, or all 
  invariants if no formula is given, using incremental algorithms.]

  Description        [The function is compiled only if there is at least
  one incremental SAT solver]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_CommandCheckInvarBmc]

  CommandName        [check_invar_bmc_inc] 	   

  CommandSynopsis    [Generates and solve the given invariant, or all 
  invariants if no formula is given]  

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\]\] 
  \[-k max_length\] \[-a algorithm\] ]  

  CommandDescription [<p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid INVAR specification 
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula \[IN context\]"</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-k <i>max_length</i></tt> 
       <dd> Use to specify the maximal deepth to be reached by the incremental 
            invariant checking algorithm. If not specified, the value assigned 
	    to the system variable <i>bmc_length</i> is taken. 
    <dt> <tt>-a <i>algorithm</i></tt> 
       <dd> 
  </dl>]  

******************************************************************************/
int Bmc_CommandCheckInvarBmcInc(const int argc, const char** argv)
{
  Prop_ptr invarprop = PROP(NULL);   /* The property being processed */
  outcome opt_handling_res; 
  char* algorithm_name = (char*) NULL;   
  boolean use_dual_alg = true;
  int res = 0;
  int max_k = get_bmc_pb_length(options);


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = bmc_cmd_options_handling(argc, argv, 
					      Prop_Invar, &invarprop, 
					      &max_k, NULL, 
					      &algorithm_name, NULL);

  if (opt_handling_res == SUCCESS_REQUIRED_HELP) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    return UsageBmcCheckInvarInc();
  }

  if (opt_handling_res != SUCCESS) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    return 1;
  }

  /* make sure the model hierarchy has been flattened */
  if (cmp_struct_get_flatten_hrc (cmps) == 0) {
    fprintf (nusmv_stderr, "The hierarchy must be flattened before. Use "\
	     "the \"flatten_hierarchy\" command.\n");
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    return 1;
  }

  /* make sure bmc has been set up */
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	     "the \"bmc_setup\" command.\n");
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    return 1;
  }

  /* ----------------------------------------------------------------------- */

  /* Checks algorithms: */
  if (algorithm_name == (char*) NULL) {
    algorithm_name = util_strsav((char*) get_bmc_inc_invar_alg(options));
  }
  
  if ((strcasecmp(algorithm_name, BMC_INC_INVAR_ALG_DUAL) != 0) && 
      (strcasecmp(algorithm_name, BMC_INC_INVAR_ALG_ZIGZAG) != 0)) {
    fprintf (nusmv_stderr, 
	     "'%s' is an invalid algorithm name.\n"
	     "Valid names are "
	     "'" BMC_INC_INVAR_ALG_DUAL "'"
	     " and "
	     "'" BMC_INC_INVAR_ALG_ZIGZAG "'.\n", algorithm_name);
    FREE(algorithm_name);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  use_dual_alg = (strcasecmp(algorithm_name, BMC_INC_INVAR_ALG_DUAL) == 0);
  FREE(algorithm_name);

  /* prepare the list of properties if no property was selected: */
  if (invarprop == PROP(NULL)) {
    lsList props = PropDb_get_props_of_type(Prop_Invar);
    lsGen  iterator; 
    Prop_ptr prop;

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (use_dual_alg) res = Bmc_GenSolveInvarDual(prop, max_k);
      else res = Bmc_GenSolveInvarZigzag(prop, max_k);
      
      if (res != 0) break;
    } /* for loop */

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    if (use_dual_alg) res = Bmc_GenSolveInvarDual(invarprop, max_k);
    else res = Bmc_GenSolveInvarZigzag(invarprop, max_k);
  }

  return res; 
}
#endif


#if HAVE_INCREMENTAL_SAT
/**Function********************************************************************

  Synopsis           [Usage string for command check_invar_bmc_inc]

  Description        [The function is compiled only if there is at least
  one incremental SAT solver]

  SideEffects        [None]

  SeeAlso            [Bmc_CommandCheckInvarBmcInc]

******************************************************************************/
static int UsageBmcCheckInvarInc(void)
{
  fprintf(nusmv_stderr, "\nUsage: check_invar_bmc_inc [-h | -n idx | -p \"formula\"] [-k max_len] [-a alg]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the INVAR property specified with <idx> (using\n"
	  "\t\tincremental algorithms).\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified INVAR propositional property (using\n"
	  "\t\tincremental algorithms).\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all INVAR properties (using\n"
	  "\t\tincremental algorithms).\n"); 
  fprintf(nusmv_stderr, "  -k max_len\tUpper bound for the search.\n"
	  "\t\tIf not specified, variable bmc_length is taken.\n");
  fprintf(nusmv_stderr, "  -a alg\tUses the specified algorithm. \n");
  fprintf(nusmv_stderr, "\t\tValid values are: " 
	  BMC_INC_INVAR_ALG_DUAL ", " BMC_INC_INVAR_ALG_ZIGZAG
	  "\n\t\tDefault value is taken from variable bmc_inc_invar_alg.\n");

  return 1;
}
#endif



/**Function********************************************************************

  Synopsis           [Top-level function for bmc of PSL properties]

  Description        [The parameters are:
  - prop is the PSL property to be checked
  - dump_prob is true if the problem must be dumped as DIMACS file (default filename
  from system corresponding variable)
  - inc_sat is true if incremental sat must be used. If there is no
  support for inc sat, an internal error will occur.  
  - single_prob is true if k must be not incremented from 0 to k_max
    (single problem)
  - k and rel_loop are the bmc parameters.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int Bmc_check_psl_property(Prop_ptr prop, 
			   boolean dump_prob, 
			   boolean inc_sat, 
			   boolean single_prob, 
			   int k, int rel_loop)
{
  nusmv_assert(prop != PROP(NULL));
  nusmv_assert(Prop_get_type(prop) == Prop_Psl);

  /* checks the property is LTL compatible */
  if (!Prop_is_psl_ltl(prop)) {
    fprintf (nusmv_stderr, "BMC can be used only with Psl/ltl properies.\n");
    return 1;
  }
      
  /* BMC for ltl: makes sure bmc has been set up */
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	     "the \"bmc_setup\" command.\n");
    return 1;
  }

  if (inc_sat) {
#if HAVE_INCREMENTAL_SAT    
    return Bmc_GenSolveLtlInc(prop, k, rel_loop, !single_prob);
#else
    internal_error("Bmc_check_psl_property: Inc SAT Solving requested when not supported.\n");
#endif
  }

  return Bmc_GenSolveLtl(prop, k, rel_loop, 
			 !single_prob, /* incrementally */
			 true, /* solve */
			 dump_prob ? BMC_DUMP_DIMACS : BMC_DUMP_NONE, 
			 get_bmc_dimacs_filename(options));
}




/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis [Bmc commands options handling for commands (optionally)
  acceping options -k -l -o -a -n -p]

  Description [ Output variables called res_* are pointers to
  variables that will be changed if the user specified a value for the
  corresponding option. For example if the user specified "-k 2", then
  *res_k will be assigned to 2. The caller can selectively choose which 
  options can be specified by the user, by passing either a valid pointer 
  as output parameter, or NULL to disable the corresponding option. 
  For example by passing NULL as actual parameter of res_l, option -l will 
  be not accepted. 

  If both specified, k and l will be checked for mutual consistency. 
  Loop will contain a relative value, like the one the user specified. 

  prop_type is the expected property type, if specified. 

  All integers values will not be changed if the corresponding options
  had not be specified by the user, so the caller might assign them to
  default values before calling this function. 

  All strings will be allocated by the function if the corresponding
  options had been used by the user. In this case it is responsability
  of the caller to free them. Strings will be assigned to NULL if the
  user had not specified any corresponding option. 

  Returns GENERIC_ERROR if an error has occurred;
  Returns SUCCESS_REQUIRED_HELP if -h options had been specified; 
  Returns SUCCESS in all other cases. 
  ]

  SideEffects        [Result parameters might change]

  SeeAlso            []

******************************************************************************/
static outcome 
bmc_cmd_options_handling(const int argc, const char** argv, 
			 Prop_Type prop_type,
			 /* output parameters: */
			 Prop_ptr* res_prop, 
			 int* res_k, 
			 int* res_l, 
			 char** res_a,  
			 char** res_o)
{
  int c;
  int prop_idx;
  char* str_formula = (char*) NULL;
  char* str_loop = (char*) NULL; 
  
  boolean k_specified = false;
  boolean l_specified = false;

  /* If one or more options are added here, the size of this array
     must be changed. At the momemt seven options are supported.  */
  char opt_string[7*2+1]; 
  
  
  /* ---------------------------------------------------------------------- */
  /* Fills up the string to pass to util_getopt, depending on which 
     options are actually required */
  strcpy(opt_string, "h");  /* h is always needed */

  if (res_prop != (Prop_ptr*) NULL) {
    *res_prop = (Prop_ptr) NULL;
    strcat(opt_string, "n:p:");
  }
  if (res_k != (int*) NULL) strcat(opt_string, "k:");
  if (res_l != (int*) NULL) strcat(opt_string, "l:");
  if (res_a != (char**) NULL) {
    *res_a = (char*) NULL;
    strcat(opt_string, "a:");
  }

  if (res_o != (char**) NULL) {
    *res_o = (char*) NULL;
    strcat(opt_string, "o:");
  }

  util_getopt_reset();
  while ((c = util_getopt((int)argc, (char**) argv, opt_string)) != EOF) {
    switch (c) {

    case 'h': 
      return SUCCESS_REQUIRED_HELP;

    case 'n':
      {
	char* str_prop_idx = (char*) NULL;
	
	nusmv_assert(res_prop != (Prop_ptr*) NULL);
	
	/* check if a formula has already been specified: */
	if ((*res_prop != PROP(NULL)) || (str_formula != (char*) NULL)) {
	  error_property_already_specified();
	  return GENERIC_ERROR;
	}
	
	str_prop_idx = util_strsav(util_optarg);

	/* check if property idx is ok */
	prop_idx = PropDb_get_prop_index_from_string(str_prop_idx);
	FREE(str_prop_idx);

	if (prop_idx == -1) {
	  /* error messages have already been shown */
	  return GENERIC_ERROR;
	}
	  
	/* here property idx is ok */
	*res_prop = PropDb_get_prop_at_index(prop_idx);	
	if ( Prop_check_type(*res_prop, prop_type) != 0 ) {
	  /* specified property's type is not what the caller expected */
	  return GENERIC_ERROR;
	}

	break;
      } /* case 'n' */

    case 'p':
      nusmv_assert(res_prop != (Prop_ptr*) NULL);
      
      /* check if a formula has already been specified: */
      if ((*res_prop != PROP(NULL)) || (str_formula != (char*) NULL)) {
	error_property_already_specified();
	return GENERIC_ERROR;	  
      }

      str_formula = util_strsav(util_optarg);
      break;
      
    case 'k':
      {
	char* str_k; 
	int k; 

	nusmv_assert(res_k != (int*) NULL);

	/* check if a value has already been specified: */
	if (k_specified) {
	  fprintf(nusmv_stderr, 
		  "Option -k cannot be specified more than once.\n");
	  return GENERIC_ERROR;	
	}

	str_k = util_strsav(util_optarg);

	if (util_str2int(str_k, &k) != 0) {
 	  error_invalid_number(str_k);
	  FREE(str_k);	
	  return GENERIC_ERROR;
	}
	
	if (k < 0) {
 	  error_invalid_number(str_k);
	  FREE(str_k);	
	  return GENERIC_ERROR;
	}
	
	FREE(str_k);	
	*res_k = k; 
	k_specified = true;
	break;  
      }

    case 'l':
      nusmv_assert(res_l != (int*) NULL);
      
      /* check if a value has already been specified: */
      if (l_specified) {
	fprintf(nusmv_stderr, 
		"Option -l cannot be specified more than once.\n");
	return GENERIC_ERROR;	
      }


      str_loop = util_strsav(util_optarg);
      l_specified = true;
      /* checking of loopback value is delayed after command line
	 processing to allow any -k option evaluation before (see the
	 cheking code below) */
      break;

    case 'a':
      nusmv_assert(res_a != (char**) NULL);

      if (*res_a != (char*) NULL) {
	fprintf(nusmv_stderr, "Algorithm can be specified only once.\n\n");
	return GENERIC_ERROR;
      }

      *res_a = util_strsav(util_optarg);
      break;
      
    case 'o':
      nusmv_assert(res_o != (char**) NULL);

      *res_o = util_strsav(util_optarg);
      break;
      
    default:  return GENERIC_ERROR;

    } /* switch case */
  } /* end of cmd line processing */

  /* checks if there are unexpected options: */
  if (argc != util_optind) {
    fprintf(nusmv_stderr, "You specified one or more invalid options.\n\n");
    return GENERIC_ERROR;
  }  


  /* Checking of k,l constrains: */
  if (str_loop != (char*) NULL) {
    outcome res; 
    int rel_loop;
    
    rel_loop = Bmc_Utils_ConvertLoopFromString(str_loop, &res);
    
    if (res != SUCCESS) {
      error_invalid_number(str_loop);
      FREE(str_loop);
      return GENERIC_ERROR;
    }
    FREE(str_loop);

    if (Bmc_Utils_Check_k_l(*res_k, 
			    Bmc_Utils_RelLoop2AbsLoop(rel_loop, *res_k))
	!= SUCCESS) {
      error_bmc_invalid_k_l(*res_k, rel_loop);
      return GENERIC_ERROR;
    }
    
    *res_l = rel_loop;
  } /* k,l consistency check */

    
  /* Formula checking and commitment: */
  if (str_formula != (char*) NULL) {
    int idx;

    /* make sure the model hierarchy has been flattened */
    if (cmp_struct_get_flatten_hrc (cmps) == 0) {
      fprintf (nusmv_stderr, "The hierarchy must be flattened before. Use "\
	       "the \"flatten_hierarchy\" command.\n");
      FREE(str_formula);
      return GENERIC_ERROR;
    }

    /* make sure bmc has been set up */
    if (cmp_struct_get_bmc_setup(cmps) == 0) {
      fprintf (nusmv_stderr, "Bmc must be setup before. Use "\
	       "the \"bmc_setup\" command.\n");
      FREE(str_formula);
      return GENERIC_ERROR;
    }

    idx = PropDb_prop_parse_and_add(Enc_get_symb_encoding(), 
				    str_formula, prop_type);
    if (idx == -1) {
      FREE(str_formula);
      return GENERIC_ERROR;
    }

    /* index is ok */
    nusmv_assert(*res_prop == PROP(NULL));
    *res_prop = PropDb_get_prop_at_index(idx);

    FREE(str_formula);
  } /* formula checking and commit */

  return SUCCESS;
}
