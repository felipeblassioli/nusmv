/**CFile***********************************************************************

  FileName    [main.c]

  PackageName [ltl2smv]

  Synopsis    [The LTL to SMV Translator]

  Description [This file contains the main function
  which invokes a routine for reducing LTL model
  checking to CTL model checking. see file ltl2smv.h]

  SeeAlso     [ltl2smv.h]

  Author      [Adapted to NuSMV by Marco Roveri. 
               Extended to the past operators by Ariel Fuxman.]

  Copyright   [
  This file is part of the ``ltl2smv'' package of NuSMV version 2. 
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

#include "ltl2smv.h"
#include <stdio.h>
#include <string.h>


int main(int argc, char **argv)
{
  int spec_Counter;
  
  if ((argc > 4) || (argc < 3)) {
    fprintf(stderr, "%s: Converts an LTL formula to a fragment of an "
	    "SMV program.\n", argv[0]);
    fprintf(stderr, "%s: %s # <ifile> [<ofile>]\n", argv[0], argv[0]);
    fprintf(stderr, "Where:\n\t#\t is a number, which is converted to a "
	    "string and then used\n"
	    "\t\t as a part of the generated SMV model name _LTL#_SPECF_N_.\n");
    fprintf(stderr, "\t<ifile>\t is the file from which the LTL Formula "
	    "to be translated\n\t\t is read in.\n");
    fprintf(stderr, "\t<ofile>\t is the file in which the SMV code "
	    "corresponding to the\n\t\t tableau of LTL Formula is "
	    "written in.\n\t\t If not specified than stdout is used.\n");
    exit(1);
  }
  
  {
    char* err_occ[1];
    err_occ[0] = "";
    spec_Counter = (int) strtol(argv[1], err_occ, 10);
    if (strcmp(err_occ[0], "") != 0) {
      fprintf(stderr, "Error: \"%s\" is not a natural number.\n", err_occ[0]);
      exit(1);
    }
  }
  ltl2smv(spec_Counter, argv[2], 4 == argc ? argv[3] : (const char*)NULL);
  return 0;
}


/* ---------------------------------------------------------------------- */
/* Some portability code 
   This code is provided to be compiled under mingw and other non-gnu 
   platforms. Under GNU it will be ignored.                               */

#if !HAVE_MALLOC 
# undef malloc
# if HAVE_MALLOC_H
#  include <malloc.h>
# elif HAVE_STDLIB_H
#  include <stdlib.h>
# endif

# ifndef malloc
void* malloc(size_t);
# endif /* ifndef malloc */

/**Function********************************************************************

  Synopsis           [This function is used instead of malloc when 
  a GNU compatible malloc function is not available.]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void* rpl_malloc(size_t size)
{
  if (size == 0) size = 1;
  return malloc(size);
}
#endif /* if not HAVE_MALLOC */


#if !HAVE_MALLOC 
# undef realloc
# if HAVE_MALLOC_H
#  include <malloc.h>
# elif HAVE_STDLIB_H
#  include <stdlib.h>
# endif

# ifndef realloc
void* realloc(void*, size_t);
# endif /* ifndef realloc */

/**Function********************************************************************

  Synopsis           [This function is used instead of malloc when 
  a GNU compatible malloc function is not available.]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void* rpl_realloc(void* ptr, size_t size)
{
  if (size == 0) size = 1;
  return realloc(ptr, size);
}
#endif /* if not HAVE_MALLOC */

