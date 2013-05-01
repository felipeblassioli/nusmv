/**CHeaderFile*****************************************************************

  FileName    [sm.h]

  PackageName [sm]

  Synopsis    ["Main" package of NuSMV ("sm" = system main).]

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

  For more information of NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

  Revision    [$Id: sm.h,v 1.4.2.4.2.1 2005/11/16 12:09:47 nusmv Exp $]

******************************************************************************/

#ifndef _SM
#define _SM

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/
#include "util.h"
#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE *nusmv_stderr;
extern FILE *nusmv_stdout;
extern FILE *nusmv_stdin;
extern FILE *nusmv_historyFile;
extern FILE *nusmv_stdpipe;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void Sm_Reset ARGS((void));

EXTERN char * Sm_NuSMVReadVersion ARGS((void));
EXTERN char * Sm_NuSMVObtainLibrary ARGS((void));
EXTERN void Sm_NuSMVInitPrintMore ARGS((void));
EXTERN int Sm_NuSMVEndPrintMore ARGS((void));

EXTERN void restore_nusmv_stdout ARGS((void));
EXTERN void restore_nusmv_stderr ARGS((void));

EXTERN char* get_preprocessor_call ARGS((const char* name));
EXTERN char* get_preprocessor_filename ARGS((const char* name));
EXTERN char* get_preprocessor_names ARGS((void));
EXTERN int get_preprocessors_num ARGS((void));

/**AutomaticEnd***************************************************************/

#endif /* _SM */
