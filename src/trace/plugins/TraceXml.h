/**CHeaderFile*****************************************************************

  FileName    [TraceXml.h]

  PackageName [trace.plugins]

  Synopsis    [The header file for classes TraceXmlDumper and 
               TraceXmlLoader]

  Description []

  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace.plugins'' package of NuSMV version 2. 
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
#ifndef __TRACE_XML__H
#define __TRACE_XML__H

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "TracePlugin.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*///

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct TraceXmlDumper_TAG* TraceXmlDumper_ptr;

#if HAVE_LIBEXPAT
typedef struct TraceXmlLoader_TAG* TraceXmlLoader_ptr;
#endif

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define TRACE_XML_DUMPER(x) \
	 ((TraceXmlDumper_ptr) x)

#define TRACE_XML_DUMPER_CHECK_INSTANCE(x) \
	 (nusmv_assert(TRACE_XML_DUMPER(x) != TRACE_XML_DUMPER(NULL)))


#if HAVE_LIBEXPAT
#define TRACE_XML_LOADER(x) \
	 ((TraceXmlLoader_ptr) x)

#define TRACE_XML_LOADER_CHECK_INSTANCE(x) \
	 (nusmv_assert(TRACE_XML_LOADER(x) != TRACE_XML_LOADER(NULL)))
#endif

/**AutomaticStart*************************************************************/ 

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN TraceXmlDumper_ptr TraceXmlDumper_create ARGS(());

#if HAVE_LIBEXPAT
EXTERN TraceXmlLoader_ptr TraceXmlLoader_create ARGS(());
#endif

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_XML__H */

