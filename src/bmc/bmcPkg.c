/**CFile***********************************************************************

  FileName    [bmcPkg.c]

  PackageName [bmc]

  Synopsis    [Bmc.Pkg module]

  Description [This module contains all the bmc package handling functions]

  SeeAlso     []

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

#include "bmcPkg.h"
#include "bmcInt.h" /* for 'options' */

#include "be/be.h"
#include "sat/sat.h"

#include "bmcVarsMgr.h"
#include "bmcFsm.h"
#include "bmcCmd.h"

#include "prop/prop.h"
#include "cmd/cmd.h"
#include "enc/enc.h"

#ifdef BENCHMARKING
  #include <time.h>
  clock_t start_time;
#endif


static char rcsid[] UTIL_UNUSED = "$Id: bmcPkg.c,v 1.3.2.9.2.6 2005/11/16 12:09:45 nusmv Exp $";

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

/**Variable********************************************************************

  Synopsis    [This is the singleton instance of the variables manager]
  Description [This instance is currently shared between all be-based fsm.]
  SeeAlso     []

******************************************************************************/
BmcVarsMgr_ptr gl_vars_mgr = NULL;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the BMC model manager structure]

  Description        [It builds the vars manager, and then creates the BE fsm
  from the Sexpr FSM. Currently the vars manager is a singleton global private variable
  which is shared between all the BE FSMs.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Bmc_Init()
{
  Bmc_Fsm_ptr fsm_be = NULL;
  Encoding_ptr senc;

  #ifdef BENCHMARKING
    fprintf(nusmv_stdout,":START:benchmarking Bmc_Init\n");
    start_time = clock();
  #endif

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Initializing the BMC package... \n");
  }

  senc = Enc_get_symb_encoding();

  /* sets up Conv module: */
  Bmc_Conv_init_cache();

  /* sets up an hash in the wff module: */
  bmc_init_wff2nnf_hash();

  /* builds the variables manager: */
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Building the BMC Variables Manager... \n");
  }
  nusmv_assert(gl_vars_mgr == NULL);
  gl_vars_mgr = BmcVarsMgr_Create(senc);

  /* builds the master bmc fsm: */
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Building the BMC FSM... \n");
  }
  /* :WARNING: The FSM is currently destroyed by the package 'prop', but it is
     built here! */
  fsm_be  = Bmc_Fsm_CreateFromSexprFSM(gl_vars_mgr, 
				       Prop_master_get_bool_sexp_fsm());
  Prop_master_set_be_fsm(fsm_be);

  /* Initializes the be generic interface layer: */
  Be_Init();

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Done \n");
  }
  
#ifdef BENCHMARKING
  fprintf(nusmv_stdout,":UTIME = %.4f secs.\n",((double)(clock()-start_time))/CLOCKS_PER_SEC);
  fprintf(nusmv_stdout,":STOP:benchmarking Bmc_Init\n");
#endif
  
  cmp_struct_set_bmc_setup(cmps);
}

/**Function********************************************************************

  Synopsis           [Frees all resources allocated for the BMC model manager]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bmc_Quit()
{
  /* Shuts down the Be layer: */
  Be_Quit();

  /* Resets the _bmc_test_tableau command private data: */
  Bmc_TestReset();

  /* quits an hash table in the wff module: */
  bmc_quit_wff2nnf_hash();

  /* quits Conv module */
  Bmc_Conv_quit_cache();

  /* Destroyes the variables manager: */
  if (gl_vars_mgr != (BmcVarsMgr_ptr) NULL) {
    BmcVarsMgr_Delete(&gl_vars_mgr);
  }
}




/**Function********************************************************************

  Synopsis           [Adds all bmc-related commands to the interactive shell]

  Description        []

  SideEffects        []

  SeeAlso            [SmInit]

******************************************************************************/
void Bmc_AddCmd()
{
  Cmd_CommandAdd("bmc_setup",         Bmc_CommandBmcSetup, 0);
  Cmd_CommandAdd("bmc_simulate",      Bmc_CommandBmcSimulate, 0);
  Cmd_CommandAdd("gen_ltlspec_bmc",
                 Bmc_CommandGenLtlSpecBmc, 0);
  Cmd_CommandAdd("gen_ltlspec_bmc_onepb",
                 Bmc_CommandGenLtlSpecBmcOnePb, 0);
  Cmd_CommandAdd("check_ltlspec_bmc", Bmc_CommandCheckLtlSpecBmc, 0);
  Cmd_CommandAdd("check_ltlspec_bmc_onepb",
		 Bmc_CommandCheckLtlSpecBmcOnePb, 0);
#if HAVE_INCREMENTAL_SAT
  Cmd_CommandAdd("check_ltlspec_bmc_inc", Bmc_CommandCheckLtlSpecBmcInc, 0);
#endif

  Cmd_CommandAdd("gen_invar_bmc",     Bmc_CommandGenInvarBmc, 0);
  Cmd_CommandAdd("check_invar_bmc",   Bmc_CommandCheckInvarBmc, 0);
#if HAVE_INCREMENTAL_SAT
  Cmd_CommandAdd("check_invar_bmc_inc",   Bmc_CommandCheckInvarBmcInc, 0);
#endif
  Cmd_CommandAdd("_bmc_test_tableau", Bmc_TestTableau, 0);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

