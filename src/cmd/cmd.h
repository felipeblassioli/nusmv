/**CHeaderFile*****************************************************************

  FileName    [cmd.h]

  PackageName [cmd]

  Synopsis    [Implements command line interface, and miscellaneous commands.]

  Author      [Adapted to NuSMV by Marco Roveri]

  Copyright   [
  This file is part of the ``cmd'' package of NuSMV version 2. 
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

  Revision    [$Id: cmd.h,v 1.4.2.1.2.1 2005/11/16 12:09:45 nusmv Exp $]

******************************************************************************/

#ifndef _CMD
#define _CMD

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/
#include <setjmp.h>
#include <signal.h>

#if HAVE_CONFIG_H
# include "config.h"
#endif 

#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef int (*PFI)();

 
/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

#if HAVE_LIBREADLINE
EXTERN char *readline(char *PROMPT);
EXTERN void add_history(char *line);
#endif
#if HAVE_SETVBUF
EXTERN int setvbuf(FILE*, char*, int mode, size_t size);
#endif
#ifdef PURIFY
EXTERN void purify_all_inuse();
#endif
EXTERN void Cmd_CommandAdd(char * name, PFI funcFp, int changes);
EXTERN int Cmd_CommandExecute(char * command);
EXTERN int Cmd_SecureCommandExecute(char* command);
EXTERN FILE * Cmd_FileOpen(char * fileName, char * mode, char ** realFileName_p, int silent);
EXTERN char * Cmd_FlagReadByName(char * flag);
EXTERN void Cmd_Init();
EXTERN void Cmd_End();

/**AutomaticEnd***************************************************************/

#endif /* _CMD */















