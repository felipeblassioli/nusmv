/**CFile***********************************************************************

  FileName    [TraceXmlDumper.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TraceXmlDumper class]

  Description [ This file contains the definition of TraceXmlDumper
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

static char rcsid[] UTIL_UNUSED = "$Id: TraceXmlDumper.c,v 1.1.2.3.2.2 2005/11/15 13:48:57 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define TRACE_XML_VERSION_INFO_STRING \
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void trace_xml_dumper_finalize ARGS((Object_ptr object, void* dummy));

static int trace_xml_dumper_dump ARGS((TraceXmlDumper_ptr self, 
				       Trace_ptr trace, FILE* output));



/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates an XML Plugin for dumping and initializes it.]

  Description [XML plugin constructor. Using this plugin, a trace can be dumped
  into or stored from the XML format.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceXmlDumper_ptr TraceXmlDumper_create()
{
  TraceXmlDumper_ptr self = ALLOC(TraceXmlDumper, 1);

  TRACE_XML_DUMPER_CHECK_INSTANCE(self);

  trace_xml_dumper_init(self);
  return self;
}



/**Function********************************************************************

Synopsis           [String to XML Tag converter.]

Description        [ Protected function that converts an string to
                     TraceXMLTag ]

SideEffects        []

SeeAlso            []

******************************************************************************/
TraceXmlTag TraceXmlTag_from_string(const char* tag)
{
  static char* tag_names[] = { 
    TRACE_XML_CNTX_TAG_STRING, 
    TRACE_XML_NODE_TAG_STRING, 
    TRACE_XML_STATE_TAG_STRING, 
    TRACE_XML_COMB_TAG_STRING, 
    TRACE_XML_INPUT_TAG_STRING, 
    TRACE_XML_VALUE_TAG_STRING,
    TRACE_XML_LOOPS_TAG_STRING
  };
  
  static TraceXmlTag tag_value[] = { 
    TRACE_XML_CNTX_TAG, 
    TRACE_XML_NODE_TAG, 
    TRACE_XML_STATE_TAG, 
    TRACE_XML_COMB_TAG, 
    TRACE_XML_INPUT_TAG, 
    TRACE_XML_VALUE_TAG, 
    TRACE_XML_LOOPS_TAG, 
    TRACE_XML_INVALID_TAG 
  };
  
  TraceXmlTag ret_val = TRACE_XML_INVALID_TAG;
  int i;

  for (i = 0; i < ARRAY_SIZE(tag_names); i++) {
    if (strncmp(tag, tag_names[i], strlen(tag)) == 0) {
      ret_val = tag_value[i];
      break;
    }
  }

 if (ret_val == TRACE_XML_INVALID_TAG) {
   fprintf(nusmv_stderr, "Invalid TAG : <%s> Encountered in XML File\n", tag);
 }

  return ret_val;
}



/*---------------------------------------------------------------------------*/
/* Definition of protected methods                                           */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Action method associated with TraceXmlDumper class.]

  Description [ Given trace is written into the file pointed by
  given additional parameter ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int trace_xml_dumper_action(const TracePlugin_ptr plugin, Trace_ptr trace, 
			    void* opt)
{
  TraceXmlDumper_ptr self = TRACE_XML_DUMPER(plugin);
  int res;
  
  res = trace_xml_dumper_dump(self, trace, (FILE *) opt);
  return res;
}


/* ---------------------------------------------------------------------- */
/*     Protected Methods                                                  */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis    [Class initializer]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_xml_dumper_init(TraceXmlDumper_ptr self) 
{
  trace_plugin_init(TRACE_PLUGIN(self),"TRACE XML DUMP PLUGIN");

  /* virtual methods overriding: */
  OVERRIDE(Object, finalize) = trace_xml_dumper_finalize;
  OVERRIDE(TracePlugin, action) = trace_xml_dumper_action;
}


/**Function********************************************************************

  Synopsis    [Deinitializes the TraceXmlDumper Plugin object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_xml_dumper_deinit(TraceXmlDumper_ptr self)
{
  trace_plugin_deinit(TRACE_PLUGIN(self));
}



/* ---------------------------------------------------------------------- */
/*     Private Methods                                                    */
/* ---------------------------------------------------------------------- */


/**Function********************************************************************

  Synopsis    [Plugin finalize method.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_xml_dumper_finalize(Object_ptr object, void* dummy) 
{
  TraceXmlDumper_ptr self = TRACE_XML_DUMPER(object);

  trace_xml_dumper_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Dump the trace in XML format into FILE pointed by /"output/".]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static int trace_xml_dumper_dump(TraceXmlDumper_ptr self, 
				 Trace_ptr trace, FILE* output)
{
  int i;
  TraceIterator_ptr iter;
  BddStates last_state;

  BddEnc_ptr enc = Trace_get_enc(trace);
  Encoding_ptr senc = BddEnc_get_symbolic_encoding(enc);
  DdManager* dd = BddEnc_get_dd_manager(enc);
  TraceNode_ptr last_node = Trace_get_last_node(trace);

  NodeList_ptr loops = NodeList_create(); /* contains loops information */

  fprintf(output,"%s\n", TRACE_XML_VERSION_INFO_STRING);
  fprintf(output,"<%s type=\"%d\" desc=\"%s\" >\n",
          TRACE_XML_CNTX_TAG_STRING, Trace_get_type(trace), 
          Trace_get_desc(trace));

  last_state = TraceNode_get_state(last_node);

  i = 1;
  iter = Trace_begin(trace);
  while (!TraceIterator_is_end(iter)) {
    ListIter_ptr los; 
    NodeList_ptr list;
    BddStates curr_state = TraceNode_get_state(iter);
    BddInputs curr_input = TraceNode_get_input(iter);
    add_ptr add_state = bdd_to_add(dd, curr_state);

    fprintf(output, "\t<%s>\n", TRACE_XML_NODE_TAG_STRING);
    fprintf(output, "\t\t<%s id=\"%d\">\n", TRACE_XML_STATE_TAG_STRING, i);

    if (last_state == curr_state && Trace_get_node(trace, iter) != last_node) {
      NodeList_append(loops, (node_ptr) i);
    }

    list = Encoding_get_model_state_symbols_list(senc); 
    los = NodeList_get_first_iter(list);
    while (! ListIter_is_end(los)) {
      node_ptr cur_sym_value;
      node_ptr cur_sym = NodeList_get_elem_at(list, los);
      add_ptr cur_sym_vals = BddEnc_expr_to_add(enc, cur_sym, Nil);
      add_ptr tmp = add_if_then(dd, add_state, cur_sym_vals);

      cur_sym_value = add_value(dd, tmp);

      add_free(dd, tmp);
      add_free(dd, cur_sym_vals);

      fprintf(output,
              "\t\t\t<%s variable=\"%s\">%s</%s>\n",
              TRACE_XML_VALUE_TAG_STRING,
              sprint_node(cur_sym),
              sprint_node(cur_sym_value),
              TRACE_XML_VALUE_TAG_STRING); 

      los = ListIter_get_next(los); 
    }

    fprintf(output,"\t\t</%s>\n", TRACE_XML_STATE_TAG_STRING);
    add_free(dd, add_state);

    /* combinatorial and inputs: */
    if (curr_input != BDD_INPUTS(NULL)) {
      bdd_ptr bdd_comb; 
      add_ptr add_input, add_comb;

      /* Combinatorial: */
      bdd_comb = bdd_and(dd, curr_state, curr_input);
      add_comb = bdd_to_add(dd, bdd_comb);
      bdd_free(dd, bdd_comb);

      fprintf(output,"\t\t<%s id=\"%d\">\n", TRACE_XML_COMB_TAG_STRING, i+1);
      list = Encoding_get_model_state_input_symbols_list(senc);
      los = NodeList_get_first_iter(list);
      while (! ListIter_is_end(los)) {
        node_ptr cur_sym_value;
        node_ptr cur_sym = NodeList_get_elem_at(list, los);
        add_ptr cur_sym_vals = BddEnc_expr_to_add(enc, cur_sym, Nil);
        add_ptr tmp = add_if_then(dd, add_comb, cur_sym_vals);

        cur_sym_value = add_value(dd, tmp);

        add_free(dd, tmp);
        add_free(dd, cur_sym_vals);

        fprintf(output,
                "\t\t\t<%s variable=\"%s\">%s</%s>\n",
                TRACE_XML_VALUE_TAG_STRING,
                sprint_node(cur_sym),
                sprint_node(cur_sym_value),
                TRACE_XML_VALUE_TAG_STRING); 

        los = ListIter_get_next(los);
      }
      add_free(dd, add_comb);
      fprintf(output,"\t\t</%s>\n", TRACE_XML_COMB_TAG_STRING);


      /* Inputs: */
      add_input = bdd_to_add(dd, curr_input);
      bdd_free(dd, curr_input);
      fprintf(output,"\t\t<%s id=\"%d\">\n", TRACE_XML_INPUT_TAG_STRING, i+1);

      list = Encoding_get_model_input_symbols_list(senc);
      los = NodeList_get_first_iter(list);
      while (! ListIter_is_end(los)) {
        node_ptr cur_sym_value;
        node_ptr cur_sym = NodeList_get_elem_at(list, los);
        add_ptr cur_sym_vals = BddEnc_expr_to_add(enc, cur_sym, Nil);
        add_ptr tmp = add_if_then(dd, add_input, cur_sym_vals);

        cur_sym_value = add_value(dd, tmp);

        add_free(dd, tmp);
        add_free(dd, cur_sym_vals);

        fprintf(output,
                "\t\t\t<%s variable=\"%s\">%s</%s>\n",
                TRACE_XML_VALUE_TAG_STRING,
                sprint_node(cur_sym),
                sprint_node(cur_sym_value),
                TRACE_XML_VALUE_TAG_STRING); 

        los = ListIter_get_next(los);
      }
      fprintf(output,"\t\t</%s>\n", TRACE_XML_INPUT_TAG_STRING);

      add_free(dd, add_input);
    }

    if (curr_state != BDD_STATES(NULL)) { bdd_free(dd, curr_state); }

    fprintf(output,"\t</%s>\n", TRACE_XML_NODE_TAG_STRING);

    i += 1;
    iter = TraceIterator_get_next(iter);
  }
  
  {  /* dumps loop info */   
    ListIter_ptr iter = NodeList_get_first_iter(loops);
    boolean stop = ListIter_is_end(iter);
    
    fprintf(output,"\t<%s> ", TRACE_XML_LOOPS_TAG_STRING);

    while (!stop) {
      fprintf(output, "%d", (int) NodeList_get_elem_at(loops, iter));

      iter = ListIter_get_next(iter);
      stop = ListIter_is_end(iter);
      if (!stop) fprintf(output, ", ");
    }

    fprintf(output," </%s>\n", TRACE_XML_LOOPS_TAG_STRING);
    NodeList_destroy(loops);
    if (last_state != BDD_STATES(NULL)) bdd_free(dd, last_state);
  }

  fprintf(output,"</%s>\n ", TRACE_XML_CNTX_TAG_STRING);

  return 0;
}
