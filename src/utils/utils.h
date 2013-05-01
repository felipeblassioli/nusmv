/**CHeaderFile*****************************************************************

  FileName    [utils.h]

  PackageName [utils]

  Synopsis    [External header of the utils package]

  Description [External header of the utils package.]

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
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

  Revision    [$Id: utils.h,v 1.18.4.8 2004/05/17 14:31:44 nusmv Exp $]

******************************************************************************/

#ifndef _UTILS_H
#define _UTILS_H

/* These are potential duplicates. */
#ifndef EXTERN
#   ifdef __cplusplus
#	define EXTERN extern "C"
#   else
#	define EXTERN extern
#   endif
#endif
#ifndef ARGS
#   if defined(__STDC__) || defined(__cplusplus)
#	define ARGS(protos)	protos		/* ANSI C */
#   else /* !(__STDC__ || __cplusplus) */
#	define ARGS(protos)	()		/* K&R C */
#   endif /* !(__STDC__ || __cplusplus) */
#endif
#ifndef NORETURN
#   if defined __GNUC__
#       define NORETURN __attribute__ ((__noreturn__))
#   else
#       define NORETURN
#   endif
#endif


#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include "util.h"
#include "utils/list.h"

#define nusmv_assert(expr) \
    assert(expr)

#define NIL_PTR(ptr_type)       \
    ((ptr_type)NULL)


/* These are used to "stringize" x after its macro-evaluation: */
#define MACRO_STRINGIZE_2nd_LEVEL(x) \
   #x

#define MACRO_STRINGIZE(x) \
    MACRO_STRINGIZE_2nd_LEVEL(x)


#undef NDEBUG


#if HAVE_SRANDOM
#  if HAVE_GETPID
#    define utils_random_set_seed() \
       srandom((unsigned int)getpid())
#  else
#    define utils_random_set_seed() \
       srandom((unsigned int)time())
#  endif
#else
#  if HAVE_GETPID
#    define utils_random_set_seed() \
       srand((unsigned int)getpid())
#  else
#    define utils_random_set_seed() \
       srand((unsigned int)time())
#  endif
#endif

#if HAVE_RANDOM
#  define utils_random() \
    random()
#else
#  define utils_random() \
    rand()
#endif


#ifdef __cplusplus
typedef bool boolean;
#else
typedef enum {false=0, true=1} boolean;
#endif

typedef enum outcome_TAG
{
  GENERIC_ERROR, 
  PARSER_ERROR,
  SYNTAX_ERROR, 
  FILE_ERROR, 
  SUCCESS_REQUIRED_HELP, 
  SUCCESS
} outcome;


EXTERN void Utils_FreeListOfLists ARGS((lsList list_of_lists));

EXTERN const char* Utils_StripPath ARGS((const char* pathfname));

EXTERN void 
Utils_StripPathNoExtension ARGS((const char* fpathname, char* filename));

EXTERN char* Utils_get_temp_filename ARGS((const char* templ));
EXTERN char* Utils_get_temp_filename_in_dir ARGS((const char* dir,
						  const char* templ));

EXTERN boolean Utils_file_exists_in_paths ARGS((const char* filename,
						const char* paths,
						const char* delimiters));

EXTERN boolean Utils_file_exists_in_directory ARGS((const char* filename,
						    char* directory));

#endif /* _UTILS_H */




