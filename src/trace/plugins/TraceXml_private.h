/**CHeaderFile*****************************************************************

  FileName    [TraceXml_private.h]

  PackageName [trace.plugins]

  Synopsis    [The private header file for classes TraceXmlDumper and 
               TraceXmlLoader]

  Description []

  SeeAlso     []

  Author      [Ashutosh Trivedi, Roberto Cavada]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2. 
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

#ifndef __TRACE_XML_PRIVATE__H
#define __TRACE_XML_PRIVATE__H

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "TracePlugin_private.h"
#include "TraceXml.h"

#if HAVE_LIBEXPAT
# include "expat.h"
#endif

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*///

#if HAVE_LIBEXPAT
# define EXPAT_BUFSIZE  1024
#endif


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Numeric values for all possible tags that can occur in the xml 
  representation]

  Description []

  SeeAlso     []   
  
******************************************************************************/
typedef enum TraceXmlTag_TAG 
{
  TRACE_XML_INVALID_TAG = -1,
  TRACE_XML_CNTX_TAG    =  0,
  TRACE_XML_NODE_TAG,   
  TRACE_XML_STATE_TAG,
  TRACE_XML_COMB_TAG, 
  TRACE_XML_INPUT_TAG,
  TRACE_XML_VALUE_TAG,
  TRACE_XML_LOOPS_TAG
} TraceXmlTag;


#define TRACE_XML_CNTX_TAG_STRING   "counter-example"
#define TRACE_XML_NODE_TAG_STRING   "node"
#define TRACE_XML_STATE_TAG_STRING  "state"
#define TRACE_XML_COMB_TAG_STRING   "combinatorial"
#define TRACE_XML_INPUT_TAG_STRING  "input"
#define TRACE_XML_VALUE_TAG_STRING  "value"
#define TRACE_XML_LOOPS_TAG_STRING  "loops"


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [This is a plugin that dumps the XML representation of a trace]

  Description []

  SeeAlso     []   
  
******************************************************************************/
typedef struct TraceXmlDumper_TAG
{
  INHERITS_FROM(TracePlugin);
  
} TraceXmlDumper;


typedef struct XmlNodes_TAG* XmlNodes_ptr;



#if HAVE_LIBEXPAT
/**Struct**********************************************************************

  Synopsis    [This is the xml loader plugin class]

  Description []

  SeeAlso     []   
  
******************************************************************************/
typedef struct TraceXmlLoader_TAG
{
  INHERITS_FROM(TracePlugin);
  
  XML_Parser parser;

  /* these fields are used during the parsing phase, to keep state*/
  BddEnc_ptr enc;

  char buffer[EXPAT_BUFSIZE]; /* for reading */

  /* for xml attributes: */
  int type;
  char* trace_desc;
  char* curr_var;

  /* parsing flags: */
  TraceXmlTag curr_parsing;
  boolean parse_error;

  XmlNodes_ptr first_nodes;
  XmlNodes_ptr last_nodes;

} TraceXmlLoader;

#endif /* HAVE_LIBEXPAT */



/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN FILE* nusmv_stdout; 
EXTERN FILE* nusmv_stderr; 


/**AutomaticStart*************************************************************/ 

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

TraceXmlTag TraceXmlTag_from_string ARGS((const char* tag));

void trace_xml_dumper_init ARGS((TraceXmlDumper_ptr self));
int trace_xml_dumper_action ARGS((const TracePlugin_ptr plugin, 
				  Trace_ptr trace, void* opt));
#if HAVE_LIBEXPAT
void trace_xml_loader_init ARGS((TraceXmlLoader_ptr self));
int trace_xml_loader_action ARGS((const TracePlugin_ptr plugin, 
				  Trace_ptr trace, void* opt));
#endif


/**AutomaticEnd***************************************************************/

#endif /* __TRACE_XML_PRIVATE__H */

