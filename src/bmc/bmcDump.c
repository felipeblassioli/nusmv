/**CFile***********************************************************************

  FileName    [bmcDump.c]

  PackageName [bmc]

  Synopsis    [Dumping functionalities, like dimacs and others]

  Description [This module supplies services that dump a Bmc problem 
  into a file, in DIMACS format and others]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2. 
  Copyright (C) 2004 by ITC-irst.

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

#include "bmcDump.h"
#include "bmcInt.h"
#include "bmcUtils.h"

#include "utils/ucmd.h" /* for SubstString */


static char rcsid[] UTIL_UNUSED = "$Id: bmcDump.c,v 1.1.2.9 2004/11/10 08:18:06 nusmv Exp $";

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

static void
bmc_dump_expandFilename ARGS((const int k, const int l,
                         const int prop_idx,
                         const char* filename_to_be_expanded,
                         char* filename_expanded,
                         const size_t filename_expanded_maxlen));

static int 
bmc_dump_openDimacsFile ARGS((const char* filename, FILE** file_ref));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Dumps a cnf in different formats]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void Bmc_Dump_WriteProblem(const BmcVarsMgr_ptr vars_mgr, 
			   const Be_Cnf_ptr cnf, 
			   Prop_ptr prop, 
			   const int k, const int loop, 
			   const Bmc_DumpType dump_type,
			   const char* dump_fname_template)
{
  char dumpFilenameExpanded[BMC_DUMP_FILENAME_MAXLEN];    

  if (dump_type == BMC_DUMP_NONE) return;
    
  nusmv_assert(dump_fname_template != (char*) NULL);

  /* 10 here is the maximum length of extension */
  bmc_dump_expandFilename(k, loop, 		       
			  PropDb_get_prop_index(prop),
			  dump_fname_template,
			  dumpFilenameExpanded,
			  sizeof(dumpFilenameExpanded)-10);
  
  switch (dump_type) {
  
  case BMC_DUMP_DIMACS:

    strcat(dumpFilenameExpanded, ".dimacs");

    if (Prop_get_type(prop) == Prop_Invar) {
      Bmc_Dump_DimacsInvarProblemFilename(vars_mgr, cnf, dumpFilenameExpanded);
    }
    else {
      Bmc_Dump_DimacsProblemFilename(vars_mgr, cnf, dumpFilenameExpanded, k);
    }
    break;

  case BMC_DUMP_DA_VINCI:
    {
      FILE* davinci_file = NULL;

      strcat(dumpFilenameExpanded, ".davinci");      

      if (opt_verbose_level_gt(options, 1)) {
	fprintf(stderr, "\nOpening file '%s' for writing...\n", dumpFilenameExpanded);
      }  
      davinci_file = fopen(dumpFilenameExpanded, "w");
      if (davinci_file == (FILE*) NULL) {
	fprintf(nusmv_stdout, 
		"\n*************    WARNING    *************\
 \n An error has occurred when writing the file \"%s\".\
 \n DA VINCI dumping aborted.\
 \n*************  END WARNING  *************\n\n", dumpFilenameExpanded);
	break;
      }

      Be_DumpDavinci(BmcVarsMgr_GetBeMgr(vars_mgr),
		     Be_Cnf_GetOriginalProblem(cnf),
		     davinci_file);

      if (opt_verbose_level_gt(options, 1)) {
	printf("RBC DaVinci representation printed on %s\n",
	       dumpFilenameExpanded);
      }

      fclose(davinci_file);      
      break;
    }

  case BMC_DUMP_GDL:
    {
      FILE* gdl_file = NULL;
      
      strcat(dumpFilenameExpanded, ".gdl");      

      if (opt_verbose_level_gt(options, 1)) {
	fprintf(stderr, "\nOpening file '%s' for writing...\n", dumpFilenameExpanded);
      }  
      gdl_file = fopen(dumpFilenameExpanded, "w");
      if (gdl_file == (FILE*) NULL) {
	fprintf(nusmv_stdout, 
		"\n*************    WARNING    *************\
 \n An error has occurred when writing the file \"%s\".\
 \n GDL dumping aborted.\
 \n*************  END WARNING  *************\n\n", dumpFilenameExpanded);
	break;
      }
      
      Be_DumpGdl(BmcVarsMgr_GetBeMgr(vars_mgr),
		 Be_Cnf_GetOriginalProblem(cnf),
		 gdl_file);
      if (opt_verbose_level_gt(options, 1)) {
	printf("GDL graph printed on %s\n", dumpFilenameExpanded);
      }

      fclose(gdl_file);
      break;
    }

  default:
    internal_error("Bmc_DumpProblem: Unexpected value in dump_type");
  }
}



/**Function********************************************************************

  Synopsis           [Opens a new file named filename, than dumps the given 
  invar problem in DIMACS format]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Bmc_Dump_DimacsInvarProblemFilename(const BmcVarsMgr_ptr vars_mgr, 
					const Be_Cnf_ptr cnf, 
					const char* filename)
{
  FILE* file;
  int ret = bmc_dump_openDimacsFile(filename, &file);

  if (ret == 0) {
    Bmc_Dump_DimacsInvarProblem(vars_mgr, cnf, file);
    fclose(file);
  }
  return ret;
}


/**Function********************************************************************

  Synopsis           [Opens a new file named filename, than dumps the given 
  LTL problem in DIMACS format]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Bmc_Dump_DimacsProblemFilename(const BmcVarsMgr_ptr vars_mgr, 
				   const Be_Cnf_ptr cnf,
				   const char* filename,  
				   const int k)
{
  FILE* file;
  int ret = bmc_dump_openDimacsFile(filename, &file);

  if (ret == 0) {
    Bmc_Dump_DimacsProblem(vars_mgr, cnf, k, file);
    fclose(file);
  }
  return ret;
}


/**Function********************************************************************

  Synopsis           [Dumps the given invar problem in the given file]

  Description        [dimacsfile must be writable]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bmc_Dump_DimacsInvarProblem(const BmcVarsMgr_ptr vars_mgr, 
				 const Be_Cnf_ptr cnf,
				 FILE* dimacsfile)
{
  Bmc_Dump_DimacsProblem(vars_mgr, cnf, 1, dimacsfile);
}


/**Function********************************************************************

  Synopsis           [Dumps the given LTL problem in the given file]

  Description        [dimacsfile must be writable]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bmc_Dump_DimacsProblem(const BmcVarsMgr_ptr vars_mgr, 
			    const Be_Cnf_ptr cnf, 
			    const int k, 
			    FILE* dimacsfile)
{
  int time;
  nusmv_assert(dimacsfile != NULL); 

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(stderr, 
	    "Dumping problem to Dimacs file (problem length is %d)\n", k);
  }  

  /* Writes the readable mapping table as a comment: */
  fprintf(dimacsfile, "c BMC problem generated by NuSMV\n");
  fprintf(dimacsfile, 
	  "c Time steps from 0 to %d, %d State Variables and %d Input Variables\n",
	  k, BmcVarsMgr_GetStateVarsNum(vars_mgr), 
	  BmcVarsMgr_GetInputVarsNum(vars_mgr));

  fprintf(dimacsfile, "c Model to Dimacs Conversion Table\n");

  for (time = 0; time <= k; ++time) {
    int varindex;
    fprintf(dimacsfile, "c \nc @@@@@ Time %d\n", time);
    for (varindex = 0; varindex < BmcVarsMgr_GetStateInputVarsNum(vars_mgr); 
	 ++varindex) {
      /* to avoid dumping of input variables at time k */
      if ( (!BmcVarsMgr_IsBeIndexInInputBlock(vars_mgr, varindex)) || 
	   (time < k) ) {
	int cnf_index; 

	cnf_index = Be_BeIndex2CnfIndex( BmcVarsMgr_GetBeMgr(vars_mgr),  
			      BmcVarsMgr_VarIndex2BeIndex(vars_mgr, varindex, 
							  time, k) );
	if (cnf_index != 0) {
	  /* it is a cnf index of a real variable */
	  fprintf(dimacsfile, "c CNF variable %d => Time %d, Model Variable ",
		  cnf_index, time);
	  print_node(dimacsfile, BmcVarsMgr_VarIndex2Name(vars_mgr, varindex));
	  fprintf(dimacsfile, "\n");	
	}
      }
    }
  } /* time cycle */
  fprintf(dimacsfile, "c \n");

  /* Actually writes the dimacs data: */
  {
    lsList cl = LS_NIL;  
    lsGen  genCl, genLit;
    int    lit = 0;
  
    fprintf(dimacsfile, "c Beginning of the DIMACS dumping\n");
    /* Prints the model variables as a "special" comment line: */
    fprintf(dimacsfile, "c model %d\n", Be_Cnf_GetVarsNumber(cnf));
    fprintf(dimacsfile, "c ");

    genLit = lsStart(Be_Cnf_GetVarsList(cnf));
    while (lsNext(genLit, (lsGeneric*) &lit, LS_NH) == LS_OK) {
      fprintf(dimacsfile, "%d ", lit);
    }
    lsFinish(genLit);
    fprintf(dimacsfile, "0\n");    

    /* Prints the problem header. */
    fprintf(dimacsfile, "p cnf %d %d\n", 
	    Be_Cnf_GetMaxVarIndex(cnf), 
	    Be_Cnf_GetClausesNumber(cnf));

    /* print the clauses with the literal responsible for the polarity of 
       the formula.  This may be changed in future [AT] */
    if (Be_Cnf_GetFormulaLiteral(cnf) == INT_MAX) {      
      /* the formula is a constant. see Be_Cnf_ptr for more detail */
      /* check whether the constant value is true or false (see Be_Cnf_ptr) */
      if (0 == lsLength(Be_Cnf_GetClausesList(cnf))) {
	/* the constand is true => just output a comment */
	fprintf(dimacsfile, "c Warning: the true constant is printed out\n");
      } else {
	/* the constant is false => output a comment and a false formula */
	fprintf(dimacsfile, "c Warning: the false constant is printed out\n");
	fprintf(dimacsfile,"1 0\n-1 0\n"); /* this is always false */
      }
    } else {
      /* the formula is a usual formula, output its formula literal */
      fprintf(dimacsfile, "%d 0\n", Be_Cnf_GetFormulaLiteral(cnf));

      /* Prints the clauses. */
      genCl = lsStart(Be_Cnf_GetClausesList(cnf));
      while (lsNext(genCl, (lsGeneric*) &cl, LS_NH) == LS_OK) {
	genLit = lsStart(cl);
	while (lsNext(genLit, (lsGeneric*) &lit, LS_NH) == LS_OK) {
	  fprintf(dimacsfile, "%d ", lit);
	}
	lsFinish(genLit);
	fprintf(dimacsfile, "0\n");
      }
      lsFinish(genCl);
    }
    fprintf(dimacsfile, "c End of dimacs dumping\n");
  }

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(stderr, "End of dump.\n");
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Open a file named filename and returns its descriptor]

  Description        [The file is opened with the writable attribute. The 
  file descriptor is returned via the file_ref parameter. Returns 0 if the 
  function succedeed, otherwise the function prints out a warning in the 
  standard output and returns 1.]

  SideEffects        [file_ref will change]

  SeeAlso            []

******************************************************************************/
static int bmc_dump_openDimacsFile(const char* filename, FILE** file_ref)
{
  int ret = 0;
  if (opt_verbose_level_gt(options, 1)) {
    fprintf(stderr, "\nOpening file '%s' for writing...\n", filename);
  }  
  
  *file_ref = fopen(filename, "w");
  if ((*file_ref) == NULL)  {
    fprintf(nusmv_stdout, 
	    "\n*************    WARNING    *************\
 \n An error has occurred when writing the file \"%s\".\
 \n DIMACS dumping is not allowed.\
 \n*************  END WARNING  *************\n\n", filename);
    ret = 1;
  }
  
  return ret;
}


/**Function********************************************************************

  Synopsis           [This is only a useful wrapper for easily call
  Bmc_Utils_ExpandMacrosInFilename]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static void
bmc_dump_expandFilename(const int k, const int l,
			const int prop_idx,
			const char* filename_to_be_expanded,
			char* filename_expanded,
			const size_t filename_expanded_maxlen)
{
  char szBuffer[1024];
  char szLoopback[16];

  /* Prepares the structure for macro-expansion: */
  SubstString aSubstTable[] =  { 
    SYMBOL_CREATE(), /* 0 */
    SYMBOL_CREATE(), /* 1 */
    SYMBOL_CREATE(), /* 2 */
    SYMBOL_CREATE(), /* 3 */
    SYMBOL_CREATE(), /* 4 */
    SYMBOL_CREATE(), /* 5 */
    SYMBOL_CREATE()  /* 6 */
  };

  /* customizes the table with runtime values: */
  Utils_StripPathNoExtension(get_input_file(options), szBuffer);
  Bmc_Utils_ConvertLoopFromInteger(l, szLoopback, sizeof(szLoopback));

  /* this to protect @@ (see last rule) */
  SYMBOL_ASSIGN(aSubstTable[0], "@@", string,  "%s", "\1"); 

  SYMBOL_ASSIGN(aSubstTable[1], "@F", string,  "%s", get_input_file(options));
  SYMBOL_ASSIGN(aSubstTable[2], "@f", string,  "%s", szBuffer);
  SYMBOL_ASSIGN(aSubstTable[3], "@k", integer, "%d", k);
  SYMBOL_ASSIGN(aSubstTable[4], "@l", string, "%s", szLoopback);
  if (prop_idx != BMC_NO_PROPERTY_INDEX) {
    SYMBOL_ASSIGN(aSubstTable[5], "@n", integer, "%d", prop_idx);
  }
  else {
    SYMBOL_ASSIGN(aSubstTable[5], "@n", string, "%s", "undef");
  }

  /* this to restore @@ as @ */
  SYMBOL_ASSIGN(aSubstTable[6], "\1", string,  "%s", "@");

  Bmc_Utils_ExpandMacrosInFilename(filename_to_be_expanded,
           aSubstTable,
           sizeof(aSubstTable)/sizeof(aSubstTable[0]),
           filename_expanded,
           filename_expanded_maxlen);
}
