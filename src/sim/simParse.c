/**CFile***********************************************************************

  FileName    [simParse.c]

  PackageName [sim]

  Synopsis    [Parsing routines.]

  Description [External procedure included in this module:
		<ul>
		<li> <b>Sim_DllParseDimacsFile()</b> parses a DIMACS
		     cnf problem file; 
		</ul>
	       Internal procedures included in this module:
	        <ul>
		<li> <b>SimDllParseDimacsCls()</b> parses a set of
  		     clauses in DIMACS format; 
		</ul>

  SeeAlso     [simData.c]

  Author      [Armando Tacchella Davide Zambonin]

  Copyright   [
  This file is part of the ``sim'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by University of Genova. 

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

  Revision    [v. 1.0]

******************************************************************************/

#include "simInt.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
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


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Parses a DIMACS cnf problem file into a DLL module. ]

  Description [Takes a pointer to a file and tries to parse (using
               DIMACS format) a cnf problem. Takes also the initialization 
	       parameters for the dll module. Returns a pointer to a
	       DLL module if successfull; NULL if (i) the file does
	       not exists, or (ii) there is no line starting with p,
	       or (iii) such line does not define a DIMACS cnf
	       problem. ]

  SideEffects [none]

  SeeAlso     [Sim_DllInit] 

******************************************************************************/
int
Sim_DllParseDimacs(
  FILE             * inFile,
  int              * params) 
{

  char        buffer[SIMMAX_CL_CH];
  int         varNum;
  int         prop;
  int       * genprop;
  Vdeclare(int, tmpModelProps);

  /* Check if the input file is OK. */
  if (fgets(buffer, SIMMAX_CL_CH, inFile) == 0) {
    return -1;
  }

  Vinit(tmpModelProps); 
  /* Scan the input file searching for a line that starts with 'p'. */
  while (buffer[0] != 'p') {
    if (buffer[0] == 'c') {
      if (sscanf(buffer, "c model %d", &varNum) != 0) {
        VinitReserve0(tmpModelProps, varNum);
	while (!feof(inFile) && (fgetc(inFile) != 'c'));
	while(!feof(inFile) && fscanf(inFile, "%d", &prop) && (prop != 0)) {
	  VforceBack0(tmpModelProps, prop);
	}
      } 
    }
    if (fgets(buffer, SIMMAX_CL_CH, inFile) == 0) {
      /* If there is no such line, this is not a DIMACS problem. */
      return -1;
    }
  }
    
  /* Check if the problem is a cnf. */ 
  if (sscanf(buffer, "p cnf %d %d", 
	     &params[SIM_MAX_VAR_NUM], &params[SIM_MAX_CL_NUM]) != 0) {
    /* Initialize the DLL module. */
    Sim_DllInit(params);
    /* Start the timer. */
    SimDllStartTimer(SIMPARSE_TIME);
    /* Parse the clauses. */
    SimDllParseDimacsCls(inFile);
  }

  if (Vsize(tmpModelProps) > 0) {
    Vforeach0(tmpModelProps, genprop, prop) {
      Sim_DllPropMakeIndep(prop);
    }
    Vclear(tmpModelProps);
  }

  /* Stop the timer. */
  SimDllStopTimer(SIMPARSE_TIME);

  return 1;

} /* End of Sim_DllParseDimacs. */


/**Function********************************************************************

  Synopsis    [Parses clauses from a DIMACS file.]

  Description [Takes a reference to a file and tries to parse  
               non tautolougus *sets* of literals (clauses). Duplicate
	       literals are removed. Tautologies and empty clauses
	       are discarded. The parsing is successfull only when the
	       clause does not fall in the above categories. This
	       function does not sort the literals in the clauses.]

  SideEffects [none]

  SeeAlso     [Sim_DllParseDimacsFile]

******************************************************************************/
void
SimDllParseDimacsCls(
  FILE *      inFile)
{
  int   lit, check, i, clId;
    
  /* Read clauses until (i) the file ends or (ii) the number of total
     clauses is reached (trailing garbage is ignored). */
  for (i = 0; (i < SimParam[SIM_MAX_CL_NUM]) && !feof(inFile); i++) {

    /* Initiate building the clause. */
    clId = Sim_DllNewCl();
    
    /* Read the clause. */
    while ((fscanf(inFile, "%d", &lit) > 0) && (lit != 0)) {
      check = Sim_DllAddLit(clId, lit);
      switch (check) {
      case 0 : 
	/* A tautology: go to the end of the clause. */
	while ((fscanf(inFile, "%d", &lit) > 0) && (lit != 0));
	/* Initiate a new clause. */
	clId = Sim_DllNewCl();
	break;
      case -1 :
	/* The program is gone berserk... */
	SimDllThrow(SIM_IO_ERROR, SIM_NO_LOCATION, "SimDllParseDimacsCls");
	break;
      default :
	/* Everything is Ok. */
	break;
      }
    }

    /* Commit the clause. */
    Sim_DllCommitCl(clId);

  } /* (i < SimParam[SIM_MAX_CL_NUM]) && !feof(inFile) */

  return;

} /* End of SimDllParseDimacsCls. */





