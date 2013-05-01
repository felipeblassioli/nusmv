/**CFile***********************************************************************

  FileName    [parserCmd.c]

  PackageName [parser]

  Synopsis    [Interface of the parser package with the shell.]

  Description [Provides command for reading the NuSMV input file and
  build an internal representation of it.]

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``parser'' package of NuSMV version 2. 
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

#include "parserInt.h" 
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: parserCmd.c,v 1.4.6.2 2003/07/22 15:24:59 nusmv Exp $";

int CommandReadModel(int argc, char **argv);

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageReadModel ARGS((void));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the parser]

  Description        [Initializes the parser]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Parser_Init(void)
{
  Cmd_CommandAdd("read_model", CommandReadModel, 0);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Reads a NuSMV file into NuSMV.]

  CommandName        [read_model] 	   

  CommandSynopsis    [Reads a NuSMV file into NuSMV.]  

  CommandArguments   [\[-h\] \[-i model-file\]]  

  CommandDescription [Reads a NuSMV file. If the <tt>-i</tt> option is
  not specified, it reads from the file specified in the environment
  variable <tt>input_file</tt>.<p>
  Command options:<p>
  <dl>
    <dt> <tt>-i model-file</tt>
       <dd> Sets the environment variable <tt>input_file</tt> to
           <tt>model-file</tt>, and reads the model from the specified file.
  </dl>]  

  SideEffects        []

******************************************************************************/
int CommandReadModel(int argc, char ** argv)
{
  int c;

  if (cmp_struct_get_read_model(cmps)) {
    fprintf(nusmv_stderr, "A model appear to be already read from file: %s.\n",
            get_input_file(options));
    return(1);
  }

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"hi:")) != EOF){
    switch(c){
    case 'i': {
      set_input_file(options, util_optarg);
      break;
    }
    case 'h': return(UsageReadModel());
    default:  return(UsageReadModel());
    }
  }
  if (argc != util_optind) return(UsageReadModel());

  if (get_input_file(options) == (char *)NULL) {
    fprintf(nusmv_stderr, "Input file is (null). You must set the input file before.\n");
    return(1);
  }

  /* Parse the input file */
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "Parsing file \"%s\" ..... ", get_input_file(options));
    fflush(nusmv_stderr);
  }

  if (Parser_ReadSMVFromFile(get_input_file(options))) nusmv_exit(1);

  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "done.\n");
    fflush(nusmv_stderr);
  }
  
  cmp_struct_set_read_model(cmps);
  return(0);
}

static int UsageReadModel()
{
  fprintf(nusmv_stderr, "usage: read_model [-h] [-i <file>]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -i <file> \tReads the model from the specified <file>.\n");
  return(1);
}
