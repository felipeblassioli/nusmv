/**CHeaderFile*****************************************************************

  FileName    [parser.h]

  PackageName [parser]

  Synopsis    [Interface with the parser]

  Description [This file describe the interface with the parser. The
  result of the parsing is stored in a global variable called
  <code>parse_tree</code>.]

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

  For more information on NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

  Revision    [$Id: parser.h,v 1.3.6.2.2.1 2005/06/27 14:46:39 nusmv Exp $]

******************************************************************************/

#ifndef _PARSER_H
#define _PARSER_H

#include "utils/utils.h"
#include "node/node.h"

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void Parser_Init ARGS((void));
EXTERN void Parser_OpenInput ARGS((const char *filename));
EXTERN void Parser_CloseInput ARGS((void));
EXTERN int Parser_ReadCmdFromString ARGS((int argc, char **argv, char * head, char *tail, node_ptr *pc));
EXTERN int Parser_ReadSMVFromFile ARGS((const char *filename));
EXTERN int Parser_read_psl_from_string ARGS((int argc, char** argv, node_ptr* res));

#endif /* _PARSER_H */
