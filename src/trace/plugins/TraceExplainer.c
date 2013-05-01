/**CFile***********************************************************************

  FileName    [TraceExplainer.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TraceExplainer object.]

  Description [ This file contains the definition of \"TraceExplainer\" 
  class. TraceExplainer plugin simply prints the trace.]
		
  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace.plugins'' package of NuSMV version 2. 
  Copyright (C) 2003 by ITC-irst.

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

#include "TraceExplainer_private.h"
#include "TraceExplainer.h"
#include "TracePlugin.h"
#include "trace/TraceNode.h"
#include "trace/pkg_traceInt.h"
#include "fsm/bdd/BddFsm.h"

static char rcsid[] UTIL_UNUSED = "$Id: TraceExplainer.c,v 1.1.2.31 2004/05/31 13:58:13 nusmv Exp $";
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void trace_explainer_finalize ARGS((Object_ptr object, void* dummy)); 

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Creates an Explainer Plugin and initializes it.]

  Description [Explainer plugin constructor. As arguments it takes the boolean
  variable /"changes_only/". If <tt>changes_only</tt> is 1, than only state
  variables which assume a different value from the previous printed one are
  printed out.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceExplainer_ptr TraceExplainer_create(boolean changes_only)
{
  TraceExplainer_ptr self = ALLOC(TraceExplainer, 1);

  TRACE_EXPLAINER_CHECK_INSTANCE(self);

  trace_explainer_init(self, changes_only);
  return self;
}

/* ---------------------------------------------------------------------- */
/*     Private Methods                                                    */
/* ---------------------------------------------------------------------- */
/**Function********************************************************************

  Synopsis    [Initializes trace explain object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_explainer_init(TraceExplainer_ptr self, boolean changes_only) 
{
  if (changes_only) {
    trace_plugin_init(TRACE_PLUGIN(self),
		      "BASIC TRACE EXPLAINER - shows changes only");
  }
  else {
    trace_plugin_init(TRACE_PLUGIN(self),
		      "BASIC TRACE EXPLAINER - shows all variables");
  }

  OVERRIDE(Object, finalize) = trace_explainer_finalize;
  OVERRIDE(TracePlugin, action) = trace_explainer_action;

  self->changes_only = changes_only;
}

/**Function********************************************************************

  Synopsis    [Deinitializes Explain object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_explainer_deinit(TraceExplainer_ptr self)
{
  trace_plugin_deinit(TRACE_PLUGIN(self));
}

/**Function********************************************************************

  Synopsis    [Action method associated with TraceExplainer class.]

  Description [ The action associated with TraceExplainer is to print the trace
  on the nusmv_stdout. If <tt>changes_only</tt> is 1, than only state variables
  which assume a different value from the previous printed one are printed
  out.]

  SideEffects [<tt>print_hash</tt> is modified.]

  SeeAlso     []

******************************************************************************/
int trace_explainer_action(const TracePlugin_ptr plugin, Trace_ptr trace,
                           void* opt)
{
  TraceExplainer_ptr self = TRACE_EXPLAINER(plugin);
  BddStates last_state;
  node_ptr states_inputs_list;
  boolean print_flag = false;
  TraceIterator_ptr start_iter = TRACE_ITERATOR(opt);
  TraceIterator_ptr iter;
  DdManager* dd = BddEnc_get_dd_manager(Trace_get_enc(trace));
  TraceNode_ptr last_node = Trace_get_last_node(trace);
  BddEnc_ptr enc = Trace_get_enc(trace);
  Encoding_ptr senc = BddEnc_get_symbolic_encoding(enc);
  int i;


  self->state_vars_list = 
    NodeList_to_node_ptr(Encoding_get_model_state_symbols_list(senc));
  self->input_vars_list = 
    NodeList_to_node_ptr(Encoding_get_model_input_symbols_list(senc));

  states_inputs_list = NodeList_to_node_ptr(
                 Encoding_get_model_state_input_symbols_list(senc));

  fprintf(nusmv_stdout, "Trace Description: %s \n", Trace_get_desc(trace));
  fprintf(nusmv_stdout, "Trace Type: %s \n",
          TraceType_to_string(Trace_get_type(trace)));

  last_state = TraceNode_get_state(last_node);
  nusmv_assert(start_iter != NULL);

  inc_indent_size();
  BddEnc_print_bdd_begin(enc, self->state_vars_list, self->changes_only);

  i = 0;
  iter = Trace_begin(trace);
  if (!print_flag && iter == start_iter) print_flag = true;
  while (!TraceIterator_is_end(iter)) {

    if (print_flag) {
      BddStates curr_state = TraceNode_get_state(iter);
      BddInputs curr_input = TraceNode_get_input(iter);

      if (last_state == curr_state && 
                      (Trace_get_node(trace, iter) != last_node)) {
        fprintf(nusmv_stdout, "-- Loop starts here\n");
      }
      fprintf(nusmv_stdout, "-> State: %d.%d <-\n", 
	      Trace_get_id(trace), i + 1);
      i = i + 1;

      BddEnc_print_bdd(enc, curr_state, nusmv_stdout);

      /* Combinatorial and Input if needed: */
      if ((curr_input != BDD_INPUTS(NULL)) && 
	  (Encoding_get_model_input_vars_num(senc) > 0)) {

	/* Combinatorial if needed: */
	if (states_inputs_list != (node_ptr) NULL) {
	  bdd_ptr comb; 
	  
	  BddEnc_print_bdd_begin(enc, states_inputs_list, 
				 self->changes_only);
	  
	  comb = bdd_and(dd, curr_state, curr_input);
	  BddEnc_print_bdd(enc, comb, nusmv_stdout);
	  bdd_free(dd, comb);
	  
	  BddEnc_print_bdd_end(enc);
	}

	/* input: */
        fprintf(nusmv_stdout, "-> Input: %d.%d <-\n", 
		Trace_get_id(trace), i + 1);
        
        BddEnc_print_bdd_begin(enc, self->input_vars_list, 
			       self->changes_only);
        BddEnc_print_bdd(enc, curr_input, nusmv_stdout);
        BddEnc_print_bdd_end(enc);
      }

      if (curr_state != BDD_STATES(NULL)) bdd_free(dd, curr_state);
      if (curr_input != BDD_INPUTS(NULL)) bdd_free(dd, curr_input);
    }
    else i = i + 1;

    iter = TraceIterator_get_next(iter);
    if (!print_flag && iter == start_iter) print_flag = true;
  } /* while loop */

  BddEnc_print_bdd_end(enc);
  dec_indent_size();

  if (last_state != BDD_STATES(NULL)) bdd_free(dd, last_state);
  return 0;
}

/*---------------------------------------------------------------------------*/
/* Static functions                                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Trace Explainer finalize method.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_explainer_finalize(Object_ptr object, void* dummy) 
{
  TraceExplainer_ptr self = TRACE_EXPLAINER(object);

  trace_explainer_deinit(self);
  FREE(self);
}
