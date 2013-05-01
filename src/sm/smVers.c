/**CFile***********************************************************************

  FileName    [smVers.c]

  PackageName [sm]

  Synopsis    [Supplies the compile date and version information.]

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

******************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "smInt.h"

static char rcsid[] UTIL_UNUSED = "$Id: smVers.c,v 1.3.6.2 2004/06/03 09:15:54 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#ifndef PACKAGE_BUILD_DATE
#define PACKAGE_BUILD_DATE  "<compile date not supplied>"
#endif

#ifndef PACKAGE_STRING
#define PACKAGE_STRING         "NuSMV 2.2.x"
#endif

#if HAVE_SOLVER_ZCHAFF
# define PACKAGE_STRING_POSTFIX " zchaff"
#else
# define PACKAGE_STRING_POSTFIX ""
#endif

#ifndef NUSMV_SHARE_PATH
# ifdef DATADIR
#  define NUSMV_SHARE_PATH DATADIR "/nusmv"
# else
#  define NUSMV_SHARE_PATH "/usr/share/nusmv"
# endif
#endif


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static char * DateReadFromDateString(char * datestr);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Returns the current NuSMV version.]

  Description [Returns a static string giving the NuSMV version and compilation
  timestamp.  The user should not free this string.]
  
  SideEffects []

  SeeAlso     [Sm_NuSMVObtainLibrary]

******************************************************************************/
char* Sm_NuSMVReadVersion()
{
  static char version[1024];

  (void) sprintf(version, "%s%s (compiled on %s)", 
		 PACKAGE_STRING, PACKAGE_STRING_POSTFIX, 
		 PACKAGE_BUILD_DATE);
  return version;
}


/**Function********************************************************************

  Synopsis    [Returns the NuSMV library path.]

  Description [Returns a string giving the directory which contains the
  standard NuSMV library.  Used to find things like the default .nusmvrc, the
  on-line help files, etc. It is the responsibility of the user to free the
  returned string.]

  SideEffects []

  SeeAlso     [Sm_NuSMVReadVersion]

******************************************************************************/
char* Sm_NuSMVObtainLibrary()
{
  char * nusmv_lib_path;

  nusmv_lib_path = getenv("NuSMV_LIBRARY_PATH");
  if (nusmv_lib_path) {
    return util_tilde_expand(nusmv_lib_path);
  } else {
    return util_tilde_expand(NUSMV_SHARE_PATH);
  }
}

/**Function********************************************************************

  Synopsis           [Start piping stdout through the "more" command]

  Description        [This function is  called to initialize piping
  stdout through "more". It is important to call Sm_NuSMVEndPrintMore before
  returning from your function and after
  calling Sm_NuSMVInitPrintMore (preferably at the end of your printing;
  failing to do so will cause the stdin lines not to appear).]

  SideEffects        []

  SeeAlso            [ Sm_NuSMVEndPrintMore]

******************************************************************************/
void
Sm_NuSMVInitPrintMore(
  )
{
    fflush(nusmv_stdout);
    nusmv_stdpipe = popen("more","w"); 
}
/**Function********************************************************************

  Synopsis           [Stop piping stdout through the "more" command]

  Description        [This function is  called to terminate piping
  stdout through "more". It is important to call Sm_NuSMVEndPrintMore  before exiting
  your function (preferably at the end of your printing; failing to do so will cause
  the stdin lines not to appear). The function returns a 0 if it fails.]

  SideEffects        []

  SeeAlso            [ Sm_NuSMVInitPrintMore]

******************************************************************************/
int
Sm_NuSMVEndPrintMore(
  )
{
    if (nusmv_stdpipe != NIL(FILE)) {  
    (void) pclose(nusmv_stdpipe);
    return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Returns the date in a brief format assuming its coming from
  the program `date'.]

  Description [optional]

  SideEffects []

******************************************************************************/
static char *
DateReadFromDateString(
  char * datestr)
{
  static char result[25];
  char        day[10];
  char        month[10];
  char        zone[10];
  char       *at;
  int         date;
  int         hour;
  int         minute;
  int         second;
  int         year;

  if (sscanf(datestr, "%s %s %2d %2d:%2d:%2d %s %4d",
             day, month, &date, &hour, &minute, &second, zone, &year) == 8) {
    if (hour >= 12) {
      if (hour >= 13) hour -= 12;
      at = "PM";
    }
    else {
      if (hour == 0) hour = 12;
      at = "AM";
    }
    (void) sprintf(result, "%d-%3s-%02d at %d:%02d %s", 
                   date, month, year % 100, hour, minute, at);
    return result;
  }
  else {
    return datestr;
  }
}





