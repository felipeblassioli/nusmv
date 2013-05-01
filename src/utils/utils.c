/**CFile***********************************************************************

  FileName    [utils.c]

  PackageName [utils]

  Synopsis    [Contains useful functions and structures]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

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

******************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_DIRENT_H
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <dirent.h>
#endif

#include "utils/utils.h"
#include "utils/error.h"
#include "parser/symbols.h"



static char rcsid[] UTIL_UNUSED = "$Id: utils.c,v 1.1.4.6.2.4 2005/05/03 12:42:45 nusmv Exp $";

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
static void freeListOfLists_aux ARGS((lsList list)); 

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Returns pathname without path prefix]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
const char* Utils_StripPath(const char* pathfname)
{
  const char* old_pos = pathfname;

  while ((*pathfname) != '\0') {
    if ((*pathfname) == '/') old_pos = pathfname + 1;
    ++pathfname;
  }
  return old_pos;
}


/**Function********************************************************************

  Synopsis           [Returns filename without path and extension]

  Description        [Example: given "~/.../test.smv", "test" will be returned.
  filename must be a string whose length is large enought to contain the "pure"
  filename]

  SideEffects        [the string pointed by 'filename' changes]

  SeeAlso            []

******************************************************************************/
void Utils_StripPathNoExtension(const char* fpathname, char* filename)
{
  char* szExt = NULL;
  const char* szNoPath = Utils_StripPath(fpathname);

  szExt = strstr(szNoPath, ".");
  if (szExt != NULL) {
    strncpy(filename, szNoPath, (size_t)(szExt - szNoPath));
    *(filename+(size_t)(szExt - szNoPath)) = '\0'; /* terminates the string */
  }
  else {
    strcpy(filename, szNoPath);
  }
}


/**Function********************************************************************

  Synopsis           [Destroys a list of list]

  Description        [This function can be used to destroy lists of list. The
  contained set of lists is removed from memory as the top level list.
  More than two levels are not handled at the moment.]

  SideEffects        [Lists are deallocated]

  SeeAlso            [lsDestroy]

******************************************************************************/
void Utils_FreeListOfLists(lsList list_of_lists)
{
  lsDestroy(list_of_lists, &freeListOfLists_aux);
}




/**Function********************************************************************

  Synopsis           [Return a string to be used as temporary file]

  Description [This functions gets a template parameter for the file
  name, with 6 'X' that will be substituted by an unique id. See
  mkstemp for further info. Ensures that the filename is not already
  in use in the given directory. If NULL is passed as the directory,
  then the standard temporary directory is used instead. Returned
  string must be freed. Returtns NULL if the filename cannot be found
  or if we do not have write priviledges in the specified directory.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
#define _TEMPDIR "/tmp"
char* Utils_get_temp_filename_in_dir(const char* dir, const char* templ) 
{
  char* dirname = (char*) NULL;
  char* name = (char*) NULL;
  char* var;
  int len;
  
# if HAVE_MKSTEMP
  int fn;
# endif

# if defined(__MINGW32__)
  char dir_separator = '\\';
# else
  char dir_separator = '/';
# endif


  if (dir == NULL) {
    /* 1) Search for the directory */
#   if defined(__MINGW32__)
    var = (char*) NULL;
#   if HAVE_GETENV
    var = getenv("TEMP");
    if (var == (char*) NULL) var = getenv("TMP");
#   endif /* HAVE_GETENV */

    if (var != (char*) NULL) dirname = util_strsav(var);
    else dirname = util_strsav(".");

#   else /* ! defined __MINGW32__ */
    var = (char*) NULL;
#   if HAVE_GETENV
    var = getenv("TEMPDIR");
#   endif /* HAVE_GETENV */

    if (var != (char*) NULL) dirname = util_strsav(var);
    else dirname = util_strsav(_TEMPDIR);
#   endif /* __MINGW32__ */
  }
  else {
    dirname = ALLOC(char, strlen(dir) + 1);
    strcpy(dirname, dir);
    if (dir[strlen(dir) - 1] == dir_separator) {
      dirname[strlen(dir) - 1] = '\0';
    }
  }

  nusmv_assert(dirname != (char*) NULL);


  /* 2) Tries to open the file: */
  len = strlen(dirname) + 1 + strlen(templ) + 1;
  name = ALLOC(char, len);
  nusmv_assert(name != (char*) NULL);

  snprintf(name, len, "%s%c%s", dirname, dir_separator, templ);
  FREE(dirname);

# if HAVE_MKSTEMP
  fn = mkstemp(name);
  if (fn == -1) {
    /* tries with the current dir */
    sprintf(name, "%s", templ);
    fn = mkstemp(name);
    if (fn == -1) {
      /* no way */
      FREE(name);
      name = (char*) NULL;
    }
  }

  if (name != (char*) NULL) {
    nusmv_assert(fn != -1);
    close(fn);
    /* the file created needs to be removed */
    if (remove(name) == -1) {
      rpterr("error deleting temporary file \"%s\"", name);
    }
  }

# else
  if (mktemp(name) == (char*) NULL) {
    /* tries with the current dir */
    sprintf(name, "%s", templ);
    if (mktemp(name) == (char*) NULL) {
      /* no way */
      FREE(name);
      name = (char*) NULL;
    }
  } 
# endif  
  
  return name;
}

/**Function********************************************************************

  Synopsis           [Checks a list of directories for a given file.]

  Description        [The list of directories (delimited by the charaters given)
  are checked for the existence of the file.]

  SideEffects        []

******************************************************************************/

boolean Utils_file_exists_in_paths(const char* filename, 
				   const char* paths,
				   const char* delimiters) 
{
  char pathscopy[strlen(paths) + 1];
  char* dir;
  boolean result = false;

  strcpy(pathscopy, paths);
  dir = strtok(pathscopy, delimiters);

  while ((!result) && (dir != NULL)) {
    result = Utils_file_exists_in_directory(filename, dir);
    dir = strtok(NULL, delimiters);
  }
  return result;
}

/**Function********************************************************************

  Synopsis           [Checks for the existence of a file within a directory.]

  Description        []

  SideEffects        []

******************************************************************************/
boolean Utils_file_exists_in_directory(const char* filename, char* directory)
{
  struct dirent *dirfile;
  int l1 = strlen(filename);  

  DIR *dir = opendir(directory);
  boolean fileexists = false;

  if (dir != NULL){
    while ((!fileexists) && (dirfile = readdir(dir))) {
      if (strlen(dirfile->d_name) == l1) {
	fileexists = (strcmp(filename, dirfile->d_name) == 0);
      }
    }
    (void) closedir(dir);
  }

  return fileexists;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Private service for Utils_FreeListOfLists]

  SideEffects        []

  SeeAlso            [Utils_FreeListOfLists]

******************************************************************************/
static void freeListOfLists_aux(lsList list)
{
  lsDestroy(list, NULL);
}

