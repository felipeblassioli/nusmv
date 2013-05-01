/**CFile***********************************************************************

  FileName    [TraceXmlLoader.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TraceXmlLoader class]

  Description [ This file contains the definition of TraceXmlLoader 
  class.]
		
  SeeAlso     []

  Author      [Ashutosh Trivedi, Roberto Cavada]

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

#include "TraceXml.h"
#include "TraceXml_private.h"

static char rcsid[] UTIL_UNUSED = "$Id: TraceXmlLoader.c,v 1.1.2.3 2004/05/21 08:09:58 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef void (*TagStartFunction_ptr)(void* , const char*, const char**);
typedef void (*TagEndFunction_ptr)(void* , const char*);
typedef void (*CharHandlerFunction_ptr)(void*, const char*, int);


/**Struct**********************************************************************

  Synopsis    [xml trace node, privately used by TraceXmlLoader]

  Description [ This structure represents a XML trace node. It stores the state
  and input BDD list for each state/ input variable. <br>
  <br>
  ]

  SeeAlso     []   
  
******************************************************************************/
typedef struct XmlNodes_TAG
{
  node_ptr state_vars_list;  
  node_ptr input_vars_list;  

  struct XmlNodes_TAG* next;

} XmlNodes;



/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void trace_xml_loader_finalize ARGS((Object_ptr object, void* dummy));

static void trace_xml_loader_reset ARGS((TraceXmlLoader_ptr self));

static boolean trace_xml_loader_load ARGS((TraceXmlLoader_ptr self, 
					   Trace_ptr trace, FILE* xml_file));


static int trace_xml_loader_fill_trace ARGS((TraceXmlLoader_ptr self, 
					     Trace_ptr trace));
static void 
trace_xml_loader_tag_begin ARGS((TraceXmlLoader_ptr self, 
				 const char* name, const char** atts));

static void trace_xml_loader_tag_end ARGS((TraceXmlLoader_ptr self, 
					   const char *name));

static void trace_xml_loader_char_handler ARGS((TraceXmlLoader_ptr self, 
						const char *txt, int txtlen));


/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceXmlLoader_ptr TraceXmlLoader_create()
{
  TraceXmlLoader_ptr self = ALLOC(TraceXmlLoader, 1);

  TRACE_XML_LOADER_CHECK_INSTANCE(self);

  trace_xml_loader_init(self);
  return self;
}


/* ---------------------------------------------------------------------- */
/*   Protected Methods                                                    */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_xml_loader_init(TraceXmlLoader_ptr self) 
{
  trace_plugin_init(TRACE_PLUGIN(self), "TRACE XML LOAD PLUGIN");
  
  /* this field depends on the trace will be filled out during the
     loading. Its value will be assigned when the loading begins: */
  self->enc = BDD_ENC(NULL);

  /* XML parser based on expat library */
  self->parser = XML_ParserCreate(NULL);

  /* to allow the parser access self data during parsing: */
  XML_SetUserData(self->parser, self); 

  /* setting up handlers for the parser: */
  XML_SetElementHandler(self->parser, 
                        (TagStartFunction_ptr) &trace_xml_loader_tag_begin, 
                        (TagEndFunction_ptr) &trace_xml_loader_tag_end);
  XML_SetCharacterDataHandler(self->parser,
                     (CharHandlerFunction_ptr) &trace_xml_loader_char_handler);
  
  /* other fields: */
  self->first_nodes = (XmlNodes_ptr) NULL;
  self->last_nodes = (XmlNodes_ptr) NULL;
  self->curr_parsing = TRACE_XML_INVALID_TAG;
  self->parse_error = false;

  self->curr_var = (char*) NULL;
  self->trace_desc = (char*) NULL;
  self->buffer[0] = '\0';

  /* virtual methods overriding: */
  OVERRIDE(Object, finalize) = trace_xml_loader_finalize;
  OVERRIDE(TracePlugin, action) = trace_xml_loader_action;
}


/**Function********************************************************************

  Synopsis    [Deallocates internal structures]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_xml_loader_deinit(TraceXmlLoader_ptr self) 
{
  
  nusmv_assert(self->enc == BDD_ENC(NULL)); /* no loading suspended */
  XML_ParserFree(self->parser);

  trace_plugin_deinit(TRACE_PLUGIN(self));
}


/**Function********************************************************************

  Synopsis    [Action method associated with TraceXmlLoader class.]

  Description [ Given trace is loaded from xml representation stored into 
  the file pointed by given additional parameter ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int trace_xml_loader_action(const TracePlugin_ptr plugin, Trace_ptr trace, 
			    void* opt)
{
  TraceXmlLoader_ptr self = TRACE_XML_LOADER(plugin);
  int res;
  
  res = trace_xml_loader_load(self, trace, (FILE *) opt);
  return res;
}



/* ---------------------------------------------------------------------- */
/*     Private Methods                                                    */
/* ---------------------------------------------------------------------- */


/**Function********************************************************************

  Synopsis    [Virtual destructor]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_xml_loader_finalize(Object_ptr object, void* dummy) 
{
  TraceXmlLoader_ptr self = TRACE_XML_LOADER(object);

  trace_xml_loader_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Cleans up after reading of xml source]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_xml_loader_reset(TraceXmlLoader_ptr self)
{
  XmlNodes_ptr iter;
  DdManager* dd = BddEnc_get_dd_manager(self->enc);

  iter = self->first_nodes;
  while (iter != (XmlNodes_ptr) NULL) {
    walk_dd(dd, bdd_free, iter->state_vars_list);
    free_list(iter->state_vars_list);

    walk_dd(dd, bdd_free, iter->input_vars_list);
    free_list(iter->input_vars_list);

    iter = iter->next;
  }

  self->first_nodes = (XmlNodes_ptr) NULL;
  self->last_nodes = (XmlNodes_ptr) NULL;

  self->curr_parsing = TRACE_XML_INVALID_TAG;
  self->parse_error = false;

  if (self->curr_var != (char*) NULL) {
    FREE(self->curr_var);
    self->curr_var = (char*) NULL;
  }

  if (self->trace_desc != (char*) NULL) {
    FREE(self->trace_desc);
    self->trace_desc = (char*) NULL;
  }

  self->enc = BDD_ENC(NULL);
  self->buffer[0] = '\0';
}


/**Function********************************************************************

  Synopsis    [Read the trace from the XML file pointed by /"input/".]

  Description [Returns true if an error occurs, false otherwise]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static boolean trace_xml_loader_load(TraceXmlLoader_ptr self, Trace_ptr trace, 
				     FILE* xml_file)
{
  boolean done;
  int err = false; /* success */

  /* before reading the trace, fills out all internal structures used 
     during parsing phase */
  self->enc = Trace_get_enc(trace);
 
  /* parses the XML file */
  do {
    size_t len; 
    char buf[EXPAT_BUFSIZE];

    len = fread(buf, sizeof(buf[0]), 
		EXPAT_BUFSIZE, xml_file);

    done = len < EXPAT_BUFSIZE; /* last read? */

    if ( !XML_Parse(self->parser, buf, len, done) ) {
      /* an error occurred */
      self->parse_error = true;
      fprintf( nusmv_stderr,
	       "%s at line %d\n",
	       XML_ErrorString(XML_GetErrorCode(self->parser)),
	       XML_GetCurrentLineNumber(self->parser) );
    }

    /* other parsing error occurred during parsing phase: */
    err = self->parse_error;
    
  } while (!done && !err);

  if (!err) {
    err = trace_xml_loader_fill_trace(self, trace);
    if (!Trace_is_valid(trace)) err = 1; /* trace is not valid */
  }
  
  trace_xml_loader_reset(self);
  return err;
}


/**Function********************************************************************

  Synopsis           [It converts the internal trace representation into 
  a Trace instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int trace_xml_loader_fill_trace(TraceXmlLoader_ptr self, 
				       Trace_ptr trace)
{
  DdManager* dd = BddEnc_get_dd_manager(self->enc);
  XmlNodes_ptr temp = self->first_nodes;
  node_ptr trace_list = Nil;
  boolean is_initial_state;

  while (temp != (XmlNodes_ptr) NULL) {
    bdd_ptr acc = bdd_one(dd);
    node_ptr tmp_vars = temp->state_vars_list;

    is_initial_state = (temp->next == (XmlNodes_ptr) NULL);

    while (tmp_vars != (node_ptr) NULL) {
      bdd_ptr term = add_to_bdd(dd, (add_ptr) car(tmp_vars));
      bdd_and_accumulate(dd, &acc, term);
      bdd_free(dd, term);
      tmp_vars = cdr(tmp_vars);
    }
    trace_list = cons((node_ptr) bdd_dup(acc), trace_list);
    bdd_free(dd, acc);

    if (!is_initial_state) {
      /* do not add final inputs */
      acc = bdd_one(dd); 
      tmp_vars = temp->input_vars_list;
      while (tmp_vars != (node_ptr)NULL) {
	bdd_ptr term = add_to_bdd(dd, (add_ptr) car(tmp_vars));
	bdd_and_accumulate(dd, &acc, term);
	bdd_free(dd, term);
	tmp_vars = cdr(tmp_vars);
      }
      trace_list = cons((node_ptr) bdd_dup(acc), trace_list);
      bdd_free(dd, acc);
    }

    temp = temp->next;
  } /* loop on states/inputs pairs */

  {
    node_ptr l = reverse(trace_list);
    Trace_set_start_state(trace, BDD_STATES(car(l)));
    Trace_append_from_list_input_state(trace, l);
    Trace_set_desc(trace, self->trace_desc);
    Trace_set_type(trace, self->type);

    FREE(self->trace_desc);
    self->trace_desc = (char*) NULL;
    walk_dd(dd, bdd_free, l);
    free_list(l);
  }

  return false;
}



/**Function********************************************************************

Synopsis           [Function that gets called when parser encounter start of
some tag.]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
static void 
trace_xml_loader_tag_begin(TraceXmlLoader_ptr self, 
			   const char* name, const char** atts)
{
  int i;
 
  strncpy(self->buffer, "", EXPAT_BUFSIZE);

  switch (TraceXmlTag_from_string(name)) {
  case TRACE_XML_CNTX_TAG:
    /* Attributes. */
    for (i = 0; atts[i]; i+=2) {
      if (! strncmp("type", atts[i], 4)) self->type = atoi(atts[i+1]);
      if (! strncmp("desc", atts[i], 4))  { 
	nusmv_assert(self->trace_desc == (char*) NULL);
        self->trace_desc = ALLOC(char, strlen(atts[i+1]) + 1);
        nusmv_assert(self->trace_desc != (char*) NULL);
        strncpy(self->trace_desc, atts[i+1], strlen(atts[i+1]) + 1);
      }
    }
    break;

  case TRACE_XML_NODE_TAG:
    {
      XmlNodes_ptr new_node = ALLOC(XmlNodes, 1);
      nusmv_assert(new_node != (XmlNodes_ptr) NULL);

      new_node->next = (XmlNodes_ptr) NULL;
      new_node->state_vars_list = Nil;
      new_node->input_vars_list = Nil;

      if (self->first_nodes == (XmlNodes_ptr) NULL) {
	self->first_nodes = new_node;
      }
      if (self->last_nodes != (XmlNodes_ptr) NULL) {
	self->last_nodes->next = new_node;
      }
      self->last_nodes = new_node;

    }
    break; 
    
  case TRACE_XML_STATE_TAG:
    self->curr_parsing = TRACE_XML_STATE_TAG;
    break;
    
  case TRACE_XML_COMB_TAG:
    self->curr_parsing = TRACE_XML_COMB_TAG;
    break;

  case TRACE_XML_INPUT_TAG:
    self->curr_parsing = TRACE_XML_INPUT_TAG;
    break;
    
  case TRACE_XML_VALUE_TAG:
    /* Attributes. */
      for (i = 0; atts[i]; i+=2) {
        if (! strncmp("variable", atts[i], 8)) {
          self->curr_var = util_strsav((char *) atts[i+1]);
        }
      }
    break;
    
  case TRACE_XML_INVALID_TAG:
    self->parse_error = true;
    return;
    
  default:
    nusmv_assert(false);
  }
  
  return;
}


/**Function********************************************************************

Synopsis           [Function that gets called when end of any tag is
encountered by the parser.]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
static void trace_xml_loader_tag_end(TraceXmlLoader_ptr self, 
				     const char *name)
{
  switch (TraceXmlTag_from_string(name)) {
  case TRACE_XML_CNTX_TAG:
    break;
    
  case TRACE_XML_NODE_TAG:
    break;
    
  case TRACE_XML_STATE_TAG:
    self->curr_parsing = TRACE_XML_INVALID_TAG;
    break;

  case TRACE_XML_COMB_TAG:
    self->curr_parsing = TRACE_XML_INVALID_TAG;
    break;
    
  case TRACE_XML_INPUT_TAG:
    self->curr_parsing = TRACE_XML_INVALID_TAG;
    break;
    
  case TRACE_XML_VALUE_TAG:
    {
      node_ptr parsed = Nil;
      add_ptr res;
      char* arg[2]; 

      if (self->curr_parsing == TRACE_XML_COMB_TAG) {
	/* ignores combinatorial, since it must be fully determined by
	   state/input */
	break;
      }

      arg[0] = (char*) NULL; 
      arg[1] = ALLOC(char, strlen(self->curr_var) + strlen(self->buffer) + 2);
      nusmv_assert(arg[1] != (char *)NULL);

      sprintf(arg[1],"%s=%s", self->curr_var, self->buffer);

      if (Parser_ReadCmdFromString(2, arg, "SIMPWFF ", ";\n", &parsed) == 0) { 
        res = BddEnc_expr_to_add(self->enc, cdr(car(parsed)), Nil);

        if (self->curr_parsing == TRACE_XML_STATE_TAG) {
          self->last_nodes->state_vars_list = 
	    cons((node_ptr) res, self->last_nodes->state_vars_list);
        }
        else if (self->curr_parsing == TRACE_XML_INPUT_TAG) {
          self->last_nodes->input_vars_list = 
	    cons((node_ptr) res, self->last_nodes->input_vars_list);
        }
	else self->parse_error = true;
      }
      else self->parse_error = true;

      FREE(self->curr_var);
      FREE(arg[1]);
    }
    break;
    
  case TRACE_XML_INVALID_TAG:
    self->parse_error = true;
    return;
    
  default:
    nusmv_assert(false);
  }
  return;
}

/**Function********************************************************************

Synopsis           [Character Handler used by parser.]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
static void trace_xml_loader_char_handler(TraceXmlLoader_ptr self, 
					  const char *txt, int txtlen)
{
  char* buffer;

  buffer = ALLOC(char, txtlen + 1); 
  nusmv_assert(buffer != (char *)NULL);

  /* We need only txtlen characters from string txt */
  strncpy(buffer, txt, txtlen);
  buffer[txtlen] = '\0';  /* to terminate the string */

  nusmv_assert((EXPAT_BUFSIZE - strlen(self->buffer)) > (txtlen + 1));
  strcat(self->buffer, buffer);

  FREE(buffer);
  return ;
}
