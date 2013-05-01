/**CFile***********************************************************************

  FileName    [bmcTest.c]

  PackageName [bmc]

  Synopsis    [Test routines for <tt>bmc</tt> package]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada, Marco Benedetti]

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

  For more information of NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

******************************************************************************/

#include "bmcInt.h"
#include "bmcVarsMgr.h"
#include "bmcFsm.h"
#include "bmcUtils.h"
#include "bmcWff.h"
#include "bmcTableau.h"
#include "bmcConv.h"

#include "be/be.h"
#include "prop/prop.h"
#include "parser/symbols.h" /* for constants */
#include "utils/error.h"
#include "enc/enc.h"

#include <math.h>


static char rcsid[] UTIL_UNUSED = "$Id: bmcTest.c,v 1.20.4.5.2.2 2004/08/04 13:28:34 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define GEN_WFF_CONSES_OP_NUMBER 15

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static int iGeneratedFormulae = 0;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static node_ptr
bmc_test_mk_loopback_ltl ARGS((const BmcVarsMgr_ptr vars_mgr,
			       const int k, const int l));

static node_ptr
bmc_test_gen_wff ARGS((const BmcVarsMgr_ptr vars_mgr,
		       int max_depth, int max_conns, 
		       boolean usePastOperators));

static node_ptr
bmc_test_gen_tableau ARGS((const Bmc_Fsm_ptr be_fsm, const node_ptr ltl_nnf_wff,
			   const int k, const int l, 
			   boolean usePastOperators));

static int UsageBmcTestTableau ARGS((void));

static void
bmc_test_bexpr_output ARGS((const BmcVarsMgr_ptr vars_mgr, FILE* f,
			    const node_ptr bexp, const int output_type));


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Call this function to reset the test sub-package (into
  the reset command for example)]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bmc_TestReset()
{
  iGeneratedFormulae = 0;
}


/**Function********************************************************************

  Synopsis           [The first time Bmc_TestTableau is called in the current
  session this function creates a smv file with a model and generates a random
  ltl spec to test tableau. The following times it is called it appends a new
  formula to the file.]

  Description        [If you call this command with a loopback set to
  BMC_ALL_LOOPS you command execution is aborted.]

  SideEffects        []

  SeeAlso            []

  CommandName        [_bmc_test_tableau]

  CommandSynopsis    [Generates a random formula to logically test the
  equivalent tableau]

  CommandArguments   [\[-h\] | \[-n property_index\] | \[\[ -d max_depth\] \[-c max_conns\] \[-o operator\]\]
  ]

  CommandDescription [Use this hidden command to generate random formulae and
  to test the equivalent tableau. The first time this command is called in the
  current NuSMV session it creates a new smv file with a model and generates a
  random ltl spec to test tableau.
  The following times it is called it appends a new formula to the file.
  The generated model contains the same number of non-deterministic variables
  the currently model loaded into NuSMV contains. <BR>
  You cannot call this command if the bmc_loopback is set to '*' (all loops).
  ]

******************************************************************************/
int Bmc_TestTableau(int argc, char ** argv)
{
  Bmc_Fsm_ptr be_fsm  = NULL;
  BmcVarsMgr_ptr vars_mgr = NULL;
  node_ptr tableau_test;
  node_ptr wff=NULL;
  int k, l, max_depth, max_conns;
  boolean usePastOperators = false;
  boolean crossComparison = false;

  char szLoopback[16];

  FILE *f,*f1,*f2;

  /* user can generate a random wff based on a specified operator: */
  enum GenWffOperator {
    GWO_None, GWO_Globally, GWO_Future, GWO_Until, GWO_Releases,
              GWO_Historically, GWO_Once, GWO_Since, GWO_Triggered
  } wff_operator;

  nusmv_assert(iGeneratedFormulae>=0);

  wff_operator = GWO_None;

  k = get_bmc_pb_length(options);

  l = Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(options), NULL);
  l = Bmc_Utils_RelLoop2AbsLoop(l, k);

  if (Bmc_Utils_IsAllLoopbacks(l)) {
    /* not implemented yet */
    fprintf (nusmv_stderr, "Error: the case 'all loops' is not implemented yet.\nPlease set the variable 'bmc_loopback' to another value.\n\n");
    return 1;
  }

  max_depth = -1;  max_conns = 1; /* default values */

  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Please call bmc_setup before use this command.\n");
    return 1;
  }

  be_fsm  = Prop_master_get_be_fsm();
  vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);


  /* process command options */
  {
    int c;
    char* strNumber = NIL(char);
    char* szOperator= NIL(char);
    int prop_no;
    Prop_ptr ltlprop = PROP(NULL); /* The property being processed */
    node_ptr ltlspec;

    util_getopt_reset();
    while((c = util_getopt(argc, argv, "hn:d:c:po:x")) != EOF) {
      switch (c) {
      case 'h':
	return UsageBmcTestTableau();

      case 'n':
	{
	  char* err_occ[2];
	  strNumber = util_strsav(util_optarg);
	  err_occ[0] = "";
	  prop_no = strtol(strNumber, err_occ, 10);
	  if ((strcmp(err_occ[0], "") != 0)) {
	    fprintf(nusmv_stdout,
		    "Error: \"%s\" is not a valid value (must be integer).\n"
		    , strNumber);
	    return 1;
	  }
	}

	if (prop_no >= PropDb_get_size() || prop_no < 0) {
	  fprintf(nusmv_stdout,
		  "Error: \"%s\" is not a valid value, must be in the range [0,%d].\n",
		  strNumber, PropDb_get_size()-1);
	  return 1;
	}
	
	ltlprop = PropDb_get_prop_at_index(prop_no);
	
	if (Prop_get_type(ltlprop) != Prop_Ltl) {
	  fprintf(nusmv_stderr,
		  "Error: property %d is not of type LTL\n", prop_no);
	  return 1;
	}

	/* here prop is ok */
	ltlspec = Prop_get_expr(ltlprop);
	ltlspec = Compile_FlattenSexpExpandDefine(Enc_get_symb_encoding(), 
						  ltlspec, Nil);
	wff = Bmc_Wff_MkNnf(detexpr2bexpr(ltlspec));
	break;

	
      case 'd': /* for max_depth */
	{
	  char* err_occ[2];
	  strNumber = util_strsav(util_optarg);

	  err_occ[0] = "";
	  max_depth = strtol(strNumber, err_occ, 10);
	  if ((strcmp(err_occ[0], "") != 0)) {
	    fprintf(nusmv_stdout,
		    "Error: \"%s\" is not a valid value (must be integer).\n",
		    strNumber);
	    return 1;
	  }
	}
	break;

      case 'c': /* for max_conns */
	{
	  char* err_occ[2];
	  strNumber = util_strsav(util_optarg);

	  err_occ[0] = "";
	  max_conns = strtol(strNumber, err_occ, 10);
	  if ((strcmp(err_occ[0], "") != 0)) {
	    fprintf(nusmv_stdout,
		    "Error: \"%s\" is not a valid value (must be integer).\n",
		    strNumber);
	    return 1;
	  }
	}
	break;

      case 'p': /* for past operators */
	  usePastOperators = true;
	  break;

      case 'x':
	crossComparison = true;
	break;


      case 'o': /* operator specification */
	szOperator = util_strsav(util_optarg);
	if (strcmp(szOperator, "G")==0) wff_operator = GWO_Globally;
	else if (strcmp(szOperator, "F")==0) wff_operator = GWO_Future;
	else if (strcmp(szOperator, "U")==0) wff_operator = GWO_Until;
	else if (strcmp(szOperator, "R")==0) wff_operator = GWO_Releases;
	else if (strcmp(szOperator, "H")==0) wff_operator = GWO_Historically;
	else if (strcmp(szOperator, "O")==0) wff_operator = GWO_Once;
	else if (strcmp(szOperator, "S")==0) wff_operator = GWO_Since;
	else if (strcmp(szOperator, "T")==0) wff_operator = GWO_Triggered;

	if(!usePastOperators && (wff_operator == GWO_Historically ||
				 wff_operator == GWO_Once  ||
				 wff_operator == GWO_Since ||
				 wff_operator == GWO_Triggered ) ) {
	  fprintf(nusmv_stdout,
		  "Error: operator \"%s\" is not allowed, unless you turn on the \"p\" option.\n",
		  szOperator);
	  return 1;
	}

	if(wff_operator == GWO_None) {
	  fprintf(nusmv_stdout,
		  "Error: operator \"%s\" is not valid. Use G|F|X|U|R|Y|G|H|S|T\n",
		  szOperator);
	  return 1;
	}

	break;
      
      } /* switch */

    } /* while */

    if (argc>8) return UsageBmcTestTableau();
   
  }
  
  if (wff == NULL) {
    /* generates a random wff: */
    switch (wff_operator) {

    case GWO_None:
      wff = bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators);
      break;

    case GWO_Globally:
      wff = Bmc_Wff_MkGlobally(bmc_test_gen_wff(vars_mgr, max_depth, 
						max_conns, usePastOperators));
      break;

    case GWO_Future:
      wff = Bmc_Wff_MkEventually(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, 
						  usePastOperators));
      break;

    case GWO_Until:
      wff = Bmc_Wff_MkUntil(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
			    bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
      break;

    case GWO_Releases:
      wff = Bmc_Wff_MkReleases(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
             bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
      break;

    case GWO_Historically:
      wff = Bmc_Wff_MkHistorically(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
      break;

    case GWO_Once:
      wff = Bmc_Wff_MkOnce(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
      break;

    case GWO_Since:
      wff = Bmc_Wff_MkSince(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
          bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
      break;

    case GWO_Triggered:
      wff = Bmc_Wff_MkTriggered(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
             bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
      break;


    default:
      nusmv_assert(FALSE); /* no other types are expected here */
    }
    wff = Bmc_Wff_MkNnf(wff);
  }

  if (!crossComparison) {
        /* generates the test tableau */
        tableau_test = bmc_test_gen_tableau(be_fsm, wff, k, l, usePastOperators);

        /* writes down the imply formula */
          if (iGeneratedFormulae == 0) {
            int i=0;

            f = fopen("Bmc_TestTableau.smv", "w");
            nusmv_assert(f != NULL);

            /* writes down the non-deterministic model */
            fprintf(f, "MODULE main\nVAR\n");
            for (i = 0; i < BmcVarsMgr_GetStateVarsNum(vars_mgr); i++) {
              fprintf(f, "p%d: boolean;\n", i);
            }
           }
          else {
          /* this command has already been invoked */
            f = fopen("Bmc_TestTableau.smv", "a");
            nusmv_assert(f != NULL);
          }

          Bmc_Utils_ConvertLoopFromInteger(l, szLoopback, sizeof(szLoopback));
          fprintf(f, "\n\n-- Property %d (k=%d, l=%s, max_depth=%d, max_conns=%d): \n",
                  iGeneratedFormulae, k, szLoopback, max_depth, max_conns);
          fprintf(f, "LTLSPEC ");

          ++iGeneratedFormulae;

          fprintf (f, "\n");
          bmc_test_bexpr_output(vars_mgr, f, tableau_test,
                                BMC_BEXP_OUTPUT_SMV);
          fprintf(f, "\n\n");

          fclose(f);
 }
 else {
        /* writes down the formula */
          if (iGeneratedFormulae == 0) {
            int i=0;

            f1 = fopen("Bmc_TestTableau_BMC.smv", "w");
            f2 = fopen("Bmc_TestTableau_BDD.smv", "w");
            nusmv_assert(f1 != NULL);
            nusmv_assert(f2 != NULL);

            /* writes down the non-deterministic model */
            fprintf(f1, "MODULE main\nVAR\n");
            fprintf(f2, "MODULE main\nVAR\n");
            for (i = 0; i < BmcVarsMgr_GetStateVarsNum(vars_mgr); i++) {
              fprintf(f1, "p%d: boolean;\n", i);
              fprintf(f2, "p%d: boolean;\n", i);
            }
           }
          else {
          /* this command has already been invoked */
            f1 = fopen("Bmc_TestTableau_BMC.smv", "a");
            f2 = fopen("Bmc_TestTableau_BDD.smv", "a");
            nusmv_assert(f1 != NULL);
            nusmv_assert(f2 != NULL);
          }

          Bmc_Utils_ConvertLoopFromInteger(l, szLoopback, sizeof(szLoopback));
          fprintf(f1, "\n\n-- Property %d (k=%d, l=%s, max_depth=%d, max_conns=%d): \n",
                  iGeneratedFormulae, k, szLoopback, max_depth, max_conns);
          fprintf(f1, "LTLSPEC ");
          fprintf(f2, "\n\n-- Property %d (k=%d, l=%s, max_depth=%d, max_conns=%d): \n",
                  iGeneratedFormulae, k, szLoopback, max_depth, max_conns);
          fprintf(f2, "LTLSPEC ");

          ++iGeneratedFormulae;

          fprintf (f1, "\n");
          fprintf (f2, "\n");

          bmc_test_bexpr_output(vars_mgr, f1, wff, BMC_BEXP_OUTPUT_SMV);

          wff = Bmc_Wff_MkImplies(bmc_test_mk_loopback_ltl(Bmc_Fsm_GetVarsManager(be_fsm),k,l),wff);


          bmc_test_bexpr_output(vars_mgr, f2, wff, BMC_BEXP_OUTPUT_SMV);

          fprintf(f1, "\n\n");
          fprintf(f2, "\n\n");

          fclose(f1);
          fclose(f2);
 }
  return 0;
}




/**Function********************************************************************

  Synopsis           [Usage string for Bmc_TestTableau]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static int UsageBmcTestTableau(void)
{
  fprintf (nusmv_stderr,
"usage: _bmc_test_tableau [-h] | [-n <property_index>] | \n \
                         [[ -d <max_depth>] [-c <max_conns>] [-o <operator>] [-p]]\n");
  fprintf (nusmv_stderr, "  -h \t\t\tPrints the command usage.\n");
  fprintf (nusmv_stderr, "  -n <prop_idx> \tTest tableau of property represented by the given index\n");
  fprintf (nusmv_stderr, "  -d <max_depth>\tGenerates a random wff with the given max depth (default value is -1)\n");
  fprintf (nusmv_stderr, "  -c <max_conns>\tGenerates a random wff with the given max number of connectives\n\t\t\t(default value is 1).\n");
  fprintf (nusmv_stderr, "  -p \t\t\tGenerate future and past operators\n\t\t\t(only future operators are generated by default).\n");
  fprintf (nusmv_stderr, "  -o <operator> \tGenerates a random wff based on the specified operator, which will \n\t\t\tbe placed at top level. Valid values are G | F | U | R | H | O | S | T\n");
  fprintf (nusmv_stderr, "  -x \t\t\tGenerate two smv files with the same set of random formulas (not tautologies)\n\t\t\t with and without loop condition, respectively.\n");
  return 1;
}




/**Function********************************************************************

  Synopsis           [Bmc_TestVarsMgr]

  Description        []

  SideEffects        [...]

  SeeAlso            []

******************************************************************************/
void Bmc_TestVarsMgr(const BmcVarsMgr_ptr vars_mgr)
{
  int i;
  node_ptr X, X1, X2, X3, X4;
  be_ptr A, A1, A2, A3, A4,
    B, B1, B2, B3, B4,
    C, C1, C2, C3, C4;

  boolean local_test_passed, test_passed = true;
  int time;
  int max_time;

  max_time = BmcVarsMgr_GetMaxTime(vars_mgr);
  printf("maxtime = %d\n", max_time);

  for (i = 0; i < BmcVarsMgr_GetStateVarsNum(vars_mgr); i++) {
    for (time = 0; time <= max_time; time++) {
      X = BmcVarsMgr_VarIndex2Name(vars_mgr, i);
      A = Be_Index2Var(BmcVarsMgr_GetBeMgr(vars_mgr), i);
      B = Be_Index2Var(BmcVarsMgr_GetBeMgr(vars_mgr),
                       i + BmcVarsMgr_GetStateVarsNum(vars_mgr));
      C = Be_Index2Var(BmcVarsMgr_GetBeMgr(vars_mgr),
                       i + ((2 + time) * BmcVarsMgr_GetStateVarsNum(vars_mgr)));

      X1 = BmcVarsMgr_VarIndex2Name(vars_mgr, i);
      X2 = BmcVarsMgr_Curr2Name(vars_mgr, A);
      X3 = BmcVarsMgr_Next2Name(vars_mgr, B);
      X4 = BmcVarsMgr_Timed2Name(vars_mgr, C);

      A1 = BmcVarsMgr_VarIndex2Curr(vars_mgr, i);
      A2 = BmcVarsMgr_Name2Curr(vars_mgr, X);
      A3 = BmcVarsMgr_Next2Curr(vars_mgr, B);
      A4 = BmcVarsMgr_Timed2Curr(vars_mgr, C);

      B1 = BmcVarsMgr_VarIndex2Next(vars_mgr, i);
      B2 = BmcVarsMgr_Name2Next(vars_mgr, X);
      B3 = BmcVarsMgr_Curr2Next(vars_mgr, A);
      B4 = BmcVarsMgr_Timed2Next(vars_mgr, C);

      C1 = BmcVarsMgr_VarIndex2Timed(vars_mgr, i, time, max_time);
      C2 = BmcVarsMgr_Name2Timed(vars_mgr, X, time, max_time);
      C3 = BmcVarsMgr_Next2Timed(vars_mgr, B, time);
      C4 = BmcVarsMgr_Curr2Timed(vars_mgr, A, time, max_time);

      local_test_passed = (A1 == A2) && (A2 == A3) && (A3 == A4) &&
  (B1 == B2) && (B2 == B3) && (B3 == B4) &&
  (C1 == C2) && (C2 == C3) && (C3 == C4);

      printf("X = %p\tX1 = %p\tX2 = %p\tX3 = %p\tX4 = %p\n", X, X1, X2,
        X3, X4);
      printf("A = %p\tA1 = %p\tA2 = %p\tA3 = %p\tA4 = %p\n", A, A1, A2,
        A3, A4);
      printf("B = %p\tB1 = %p\tB2 = %p\tB3 = %p\tB4 = %p\n", B, B1, B2,
        B3, B4);
      printf("C = %p\tC1 = %p\tC2 = %p\tC3 = %p\tC4 = %p\n", C, C1, C2,
        C3, C4);
      printf("%d-ith variable, @%d time\n", i, time);
      printf("Test [%s]\n", local_test_passed ? "PASSED" : "NOT PASSED");

      test_passed = test_passed && local_test_passed;
    }
  }

  printf("Number of variables = %d\n", BmcVarsMgr_GetStateVarsNum(vars_mgr));
  printf("X = curvar_bexpr\nA = curvar\nB = nextvar\nC = timedvar\n");

  printf("\n\nWP4_test1b [%s]\n", test_passed ? "PASSED" : "\aNOT_PASSED");
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [For each variable p in the set of state variables,
  generates the global equivalence of p and X^(loop length), starting from
  the loop start]

  Description [ In the following example we suppose the loop starts
  from 2 and finishes to 6 (the bound).

  <PRE>
        ,-----------.
        V           |
  o--o--o--o--o--o--o--o--o--o--o--o--o- (...continues indefinitely)
  0  1  2  3  4  5  6  7  8  9  10 11 12

  </PRE>


  In general all variables in time 2 must be forced to be equivalent
  to the corresponding variables timed in 6, the variables in 3 to 7,
  and so on up to the variables in 6 (equivalent to variables in
  10). Then variables in 7 (or 3 again) must be forced to be equivalent
  to the varaibles in 11, and so on indefinitely.
  <BR><BR>
  In formula (let suppose we have only one boolean variable):
  <BR>
  (p2 <-> p6) && (p6 <-> p10) ...
  <BR><BR>
  In a more compact (and finite!) form, related to this example:
  XX(G (p <-> XXXX(p)))

  The first two neXtes force the formula to be effective only from the loop
  starting point.
  The generic formula implemented in the code is the following one:
  <PRE>
  X^(l) (G ((p0 <-> X^(k-l)(p0)) &&
            (p1 <-> X^(k-l)(p1)) &&
	                .
                        .
                        .
            (pn <-> X^(k-l)(pn)))
        )
  </PRE>
 where:
   p0..pn are all boolean variables into the model
   X^(n) is expanded to XXX..X n-times ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr
bmc_test_mk_loopback_ltl(const BmcVarsMgr_ptr vars_mgr, const int k, const int l)
{
  node_ptr result;
  node_ptr bigand_vars;
  node_ptr single_var_eq;
  node_ptr var;
  int i = 0;
  int iLoopLength = 0;

  nusmv_assert( !Bmc_Utils_IsNoLoopback(l) && (l < k) );
  nusmv_assert( BmcVarsMgr_GetStateVarsNum(vars_mgr) > 0 );

  iLoopLength = k-l;

  /* first cycle is performed manually, in order to optimize a bit */
  var = BmcVarsMgr_VarIndex2Name(vars_mgr, i);
  bigand_vars = Bmc_Wff_MkIff(var, Bmc_Wff_MkXopNext(var, iLoopLength));

  /* iterates across the remaining variables: */
  for (i = 1; i < BmcVarsMgr_GetStateVarsNum(vars_mgr); i++) {
    var = BmcVarsMgr_VarIndex2Name(vars_mgr, i);
    single_var_eq = Bmc_Wff_MkIff( var, Bmc_Wff_MkXopNext(var, iLoopLength) );
    bigand_vars = Bmc_Wff_MkAnd(bigand_vars, single_var_eq);
  }

  result = Bmc_Wff_MkGlobally(bigand_vars);
  result = Bmc_Wff_MkXopNext(result, l); /* shifts to loop starting point */

  return result;
}



/**Function********************************************************************

  Synopsis           [Given a WFF in NNF, converts it into a tableau
  formula, then back to WFF_(k,l) and returns WFF -> WFF_(k,l)]

  Description        [This function is used to test tableau formulae]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr
bmc_test_gen_tableau(const Bmc_Fsm_ptr be_fsm, const node_ptr ltl_nnf_wff,
		     const int k, const int l, boolean usePastOperators)
{
  node_ptr tableau_as_wff;
  node_ptr implies_formula;
  be_ptr tableau_k_l_ltl_wff;
  BmcVarsMgr_ptr vars_mgr = Bmc_Fsm_GetVarsManager(be_fsm);

  /* generates the tableau (with no fairness): */
  tableau_k_l_ltl_wff = Bmc_GetTestTableau(vars_mgr, ltl_nnf_wff, k, l);

  /* reconvert the tableau back to a wff_(k,l) */
  tableau_as_wff = Bmc_Conv_Be2Bexp(vars_mgr, tableau_k_l_ltl_wff);

  /* build the implies: */
  if (Bmc_Utils_IsNoLoopback(l)) {
    implies_formula = Bmc_Wff_MkImplies(tableau_as_wff, ltl_nnf_wff);
  }
  else {
    nusmv_assert(!Bmc_Utils_IsAllLoopbacks(l)); /* all loops are not allowed nowadays */
    implies_formula = Bmc_Wff_MkImplies(
      Bmc_Wff_MkAnd(tableau_as_wff,
        bmc_test_mk_loopback_ltl(Bmc_Fsm_GetVarsManager(be_fsm),
               k, l)),
      ltl_nnf_wff
      );
  }

  return implies_formula;
}


/**Function********************************************************************

  Synopsis           [Builds a <b>random LTL WFF</b> with specified
  <tt>max</tt> depth and <tt>max</tt> connectives.]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
static node_ptr bmc_test_gen_wff(const BmcVarsMgr_ptr vars_mgr,
				 int max_depth, int max_conns, 
				 boolean usePastOperators)
{
  int rnd;
  double rnd_tmp;

  /* generates a random number which refers to either a state variable or
     an operator. Propositional and future time operators are always
     allowed, whereas past time operators can be only generated when
     the "usePastOperators" flag is true.*/

  do {
   rnd_tmp = floor(rand()) / (RAND_MAX + 1.0);

   rnd = (int) floor((GEN_WFF_CONSES_OP_NUMBER + BmcVarsMgr_GetStateVarsNum(vars_mgr))
         * rnd_tmp) + 1;
  }
  while (!usePastOperators && (rnd>10 && rnd<=15));

  /* if depth or connses of wff are exausted get a random number such that:
     (rnd >= 0) && (rnd < 'number of state variables')... */
  if ((max_depth < 0) || (max_conns < 0)) {
    rnd = (int) (((float) BmcVarsMgr_GetStateVarsNum(vars_mgr) * rand()) /
     (RAND_MAX + 1.0));
    /* ...then return correspondent state variable to the random integer */
    return BmcVarsMgr_VarIndex2Name(vars_mgr, rnd);
  }

  /* exclude atoms from depth and connses decrement contributes */
  if (rnd <= GEN_WFF_CONSES_OP_NUMBER) {
    --max_depth; --max_conns;
  }

  switch (rnd) {
  /* Propositional operators */
  case 1:
    return Bmc_Wff_MkNot(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 2:
    return Bmc_Wff_MkOr (bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
                         bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 3:
    return Bmc_Wff_MkAnd(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
                         bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 4:
    return Bmc_Wff_MkImplies(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
                             bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 5:
    return Bmc_Wff_MkIff(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
                         bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));

  /* Future operators */
  case 6:
    return Bmc_Wff_MkOpNext    (bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 7:
    return Bmc_Wff_MkEventually(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 8:
    return Bmc_Wff_MkGlobally  (bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 9:
    return Bmc_Wff_MkUntil     (bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
                                bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 10:
    return Bmc_Wff_MkReleases  (bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
                                bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));

  /* Past operators */
  case 11:
    return Bmc_Wff_MkOpPrec      (bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 12:
    return Bmc_Wff_MkOnce        (bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 13:
    return Bmc_Wff_MkHistorically(bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 14:
    return Bmc_Wff_MkSince       (bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
                                  bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));
  case 15:
    return Bmc_Wff_MkTriggered   (bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators),
                                  bmc_test_gen_wff(vars_mgr, max_depth, max_conns, usePastOperators));

  default:
    return BmcVarsMgr_VarIndex2Name(vars_mgr, rnd - GEN_WFF_CONSES_OP_NUMBER - 1);
  }
}


/**Function********************************************************************

  Synopsis    [<b>Write</b> to specified FILE stream given node_ptr
  <b>formula</b> with specified <tt>output_type</tt> format. There are
  follow formats: <tt>BMC_BEXP_OUTPUT_SMV, BMC_BEXP_OUTPUT_LB</tt>]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
bmc_test_bexpr_output(const BmcVarsMgr_ptr vars_mgr, FILE* f,
		      const node_ptr bexp, const int output_type)
{
  int type;

  nusmv_assert(f != NULL);

  /* exit from recursion if given formula is Nil */
  if (bexp == Nil) return;

  /* assert that input formula type can't be a NEXT operator, that is
     used in model specification (section ASSIGN). We use here only OP_NEXT
     operator used in the module (section LTLSPEC). */
  nusmv_assert (node_get_type (bexp) != NEXT);

  type = node_get_type (bexp);

  switch (type) {
  case FALSEEXP:                        /* FALSEEXP  */
    fprintf (f, "%s", (output_type == BMC_BEXP_OUTPUT_SMV) ? "0" : "false");
    break;

  case TRUEEXP:                         /* TRUEEXP   */
    fprintf (f, "%s", (output_type == BMC_BEXP_OUTPUT_SMV) ? "1" : "true");
    break;

  case AND:                             /* AND       */
    fprintf(f, "(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf(f, " %s ", (output_type == BMC_BEXP_OUTPUT_SMV) ? "&" : "/\\");
    bmc_test_bexpr_output(vars_mgr, f, cdr (bexp), output_type);
    fprintf(f, ")");
    break;

  case OR:                              /* OR        */
    fprintf (f, "(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, " %s ", (output_type == BMC_BEXP_OUTPUT_SMV) ? "|" : "\\/");
    bmc_test_bexpr_output(vars_mgr, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case NOT:                             /* NOT       */
    fprintf (f, "%c", (output_type == BMC_BEXP_OUTPUT_SMV) ? '!' : '~');
    fprintf (f, "(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, ")");
    break;

  case IMPLIES:                         /* IMPLIES   */
    fprintf (f, "(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, " -> ");
    bmc_test_bexpr_output(vars_mgr, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case IFF:                             /* IFF       */
    fprintf (f, "(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, " <-> ");
    bmc_test_bexpr_output(vars_mgr, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case OP_FUTURE:                       /* OP_FUTURE */
    fprintf (f, "F(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, ")");
    break;

  case OP_ONCE:                       /* OP_ONCE */
    fprintf (f, "O(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, ")");
    break;

  case OP_GLOBAL:                       /* OP_GLOBAL */
    fprintf (f, "G(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, ")");
    break;

  case OP_HISTORICAL:                       /* OP_HISTORICAL */
    fprintf (f, "H(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, ")");
    break;

  case UNTIL:                           /* UNTIL     */
    fprintf (f, "(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, " U ");
    bmc_test_bexpr_output(vars_mgr, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case SINCE:                           /* SINCE     */
    fprintf (f, "(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, " S ");
    bmc_test_bexpr_output(vars_mgr, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case RELEASES:                        /* RELEASES  */
    fprintf (f, "(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, " V ");
    bmc_test_bexpr_output(vars_mgr, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case TRIGGERED:                       /* TRIGGERED  */
    fprintf (f, "(");
    bmc_test_bexpr_output(vars_mgr, f, car (bexp), output_type);
    fprintf (f, " T ");
    bmc_test_bexpr_output(vars_mgr, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case OP_NEXT:                         /* OP_NEXT   */
    {
      node_ptr temp_bexp = bexp;
      int i = 0;
      {
        /* prints out "X(" suffix while OP_NEXT is encountred */
        do {
          fprintf (f, "X(");
          temp_bexp = car(temp_bexp);
          nusmv_assert(temp_bexp != Nil);
          i++;
        } while (node_get_type(temp_bexp) == OP_NEXT);
      }

      /* then print the internal bexp */
      bmc_test_bexpr_output(vars_mgr, f, temp_bexp, output_type);

      while ((i--) > 0) fprintf (f, ")");
    }
    break;

  case OP_PREC:                         /* OP_PREC   */
    {
      node_ptr temp_bexp = bexp;
      int i = 0;
      {
        /* prints out "Y(" suffix while OP_PREC is encountred */
        do {
          fprintf (f, "Y(");
          temp_bexp = car(temp_bexp);
          nusmv_assert(temp_bexp != Nil);
          i++;
        } while (node_get_type(temp_bexp) == OP_PREC);
      }

      /* then print the internal bexp */
      bmc_test_bexpr_output(vars_mgr, f, temp_bexp, output_type);

      while ((i--) > 0) fprintf (f, ")");
    }
    break;

  case OP_NOTPRECNOT:                         /* OP_PREC   */
    {
      node_ptr temp_bexp = bexp;
      int i = 0;
      {
  /* prints out "Z(" suffix while OP_NOTPRECNOT is encountred */
        do {
          fprintf (f, "Z(");
          temp_bexp = car(temp_bexp);
          nusmv_assert(temp_bexp != Nil);
          i++;
        } while (node_get_type(temp_bexp) == OP_NOTPRECNOT);
      }

      /* then print the internal bexp */
      bmc_test_bexpr_output(vars_mgr, f, temp_bexp, output_type);

      while ((i--) > 0) fprintf (f, ")");
    }
    break;


  default:                              /* (default action) */
    {
      be_ptr r;

      /* gets the the be correspondent to the state variable */
      r = BmcVarsMgr_Name2Curr(vars_mgr, bexp);

      /* if bexp is a really state variable, then prints out the index
         of correspondent be variable */
      if (r != (be_ptr)NULL) {
        fprintf(f, "p%d", Be_Var2Index(BmcVarsMgr_GetBeMgr(vars_mgr), r));
      }
      else {
  internal_error("bmc_test_bexpr_output: given wff atom isn\' in BE environ\n");
      }
    }
  }

}





