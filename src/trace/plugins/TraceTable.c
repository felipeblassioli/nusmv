/**CFile***********************************************************************

  FileName    [TraceTable.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TraceTable object.]

  Description [ This file contains the definition of \"TraceTable\" 
  class.]
		
  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2. 
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


#include "TraceTable_private.h"
#include "TraceTable.h"
#include "TracePlugin.h"
#include "trace/TraceNode.h"
#include "trace/pkg_traceInt.h"
#include "fsm/bdd/BddFsm.h"
#include "enc/bdd/BddEnc.h"

static char rcsid[] UTIL_UNUSED = "$Id: TraceTable.c,v 1.1.2.20.2.1 2005/07/14 15:58:26 nusmv Exp $";

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
static void trace_table_finalize ARGS((Object_ptr object, void* dummy)); 
static int trace_table_print_row_style ARGS((TraceTable_ptr self, 
                                       Trace_ptr trace,  FILE* output));

static int trace_table_print_column_style ARGS((TraceTable_ptr self, 
                                          Trace_ptr trace,  FILE* output)); 

static void trace_table_print_vars_rows ARGS((TraceTable_ptr self, 
					      Trace_ptr trace, 
					      NodeList_ptr vars, 
					      FILE* output));

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates an Table Plugin and initializes it.]

  Description [Table plugin constructor. As arguments it takes variable style
  which decides the style of printing the trace. The possible values of the
  style variable may be: TRACE_TABLE_TYPE_ROW and TRACE_TABLE_TYPE_COLUMN.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceTable_ptr TraceTable_create(TraceTableStyle style)
{
  TraceTable_ptr self = ALLOC(TraceTable, 1);

  TRACE_TABLE_CHECK_INSTANCE(self);

  trace_table_init(self, style);
  return self;
}

/* ---------------------------------------------------------------------- */
/*     Protected Methods                                                  */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis    [Initializes trace table object.]

  Description [As arguments it takes variable /"style /" to print the trace.
  The possible values for the style may be : TRACE_TABLE_TYPE_ROW and
  TRACE_TABLE_TYPE_COLUMN.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_table_init(TraceTable_ptr self, TraceTableStyle style) 
{
  if (style == TRACE_TABLE_TYPE_COLUMN) { 
    trace_plugin_init(TRACE_PLUGIN(self), 
                      "TRACE TABLE PLUGIN - column format");
  }
  else {
    trace_plugin_init(TRACE_PLUGIN(self), 
                      "TRACE TABLE PLUGIN - row format");
  }

  OVERRIDE(Object, finalize) = trace_table_finalize;
  OVERRIDE(TracePlugin, action) = trace_table_action;

  self->style = style;
}


/**Function********************************************************************

  Synopsis    [Deinitializes Explain object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_table_deinit(TraceTable_ptr self)
{
  trace_plugin_deinit(TRACE_PLUGIN(self));
}


/**Function********************************************************************

  Synopsis    [Action method associated with TraceTable class.]

  Description [ The action associated with TraceTable is to print the trace
  in the specified file in table format. There are two ways a trace can be
  printed: i). where states are listed row-wise. ii) Where states are listed
  column-wise. This depends on the style variable assoicated with the plugin.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int trace_table_action(const TracePlugin_ptr plugin, Trace_ptr trace,
                       void* opt)
{
  TraceTable_ptr self = TRACE_TABLE(plugin);
  FILE* output = (FILE *) opt;
  int result;

  self->enc = Trace_get_enc(trace);
  self->senc = BddEnc_get_symbolic_encoding(self->enc);

  if (self->style == TRACE_TABLE_TYPE_COLUMN) {
    result =  trace_table_print_column_style(self, trace, output);
  }
  else {
    result = trace_table_print_row_style(self, trace, output);
  }

  return result;
}


/*---------------------------------------------------------------------------*/
/* Static functions                                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Print Trace in Table format with each state on a seperate
  column.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static int 
trace_table_print_column_style(TraceTable_ptr self, Trace_ptr trace,  
                               FILE* output) 
{
  DdManager* dd = BddEnc_get_dd_manager(self->enc);
  TraceIterator_ptr iter;
  NodeList_ptr vars;
  ListIter_ptr vars_iter;

  int i, j;
  int num_of_states;
  int num_of_combs;
  int num_of_inputs;

  NodeList_ptr lists[] = {
    Encoding_get_model_state_symbols_list(self->senc), 
    Encoding_get_model_state_input_symbols_list(self->senc), 
    Encoding_get_model_input_symbols_list(self->senc)
  };
  
  
  /* ----- prints the header: first states, then comb and inputs ----- */
  fprintf(output, "Name\t");

  for (i=0; i < (sizeof(lists)/sizeof(lists[0])); ++i) {
    ListIter_ptr vars_iter; 
    vars_iter = NodeList_get_first_iter(lists[i]);
    while (!ListIter_is_end(vars_iter)) {
      node_ptr var;
      var = NodeList_get_elem_at(lists[i], vars_iter);
      print_node(output, var);
      fprintf(output,"\t");    
      vars_iter = ListIter_get_next(vars_iter);
    }
  }
  fprintf(output,"\n");    
  /* -------------------- header here is printed -------------------- */
  
  num_of_states = NodeList_get_length(
            Encoding_get_model_state_symbols_list(self->senc));
  
  num_of_combs = NodeList_get_length(
            Encoding_get_model_state_input_symbols_list(self->senc));

  num_of_inputs = NodeList_get_length(
            Encoding_get_model_input_symbols_list(self->senc));

  iter = Trace_begin(trace);
  i = 0;
  while (!TraceIterator_is_end(iter)) {
    add_ptr add_state;
    BddStates curr_state = TraceNode_get_state(iter);
    BddInputs curr_input = TraceNode_get_input(iter);

    fprintf(output, "S%d", i);

    add_state = bdd_to_add(dd, curr_state);

    /* State */
    vars = Encoding_get_model_state_symbols_list(self->senc);
    vars_iter = NodeList_get_first_iter(vars);
    while (!ListIter_is_end(vars_iter)) {
      node_ptr var;
      node_ptr var_value; 
      add_ptr var_add;
      add_ptr it_add;

      var = NodeList_get_elem_at(vars, vars_iter);
      var_add = BddEnc_expr_to_add(self->enc, var, Nil);
      it_add = add_if_then(dd, add_state, var_add);
      var_value = add_value(dd, it_add);
      add_free(dd, it_add);
      add_free(dd, var_add);

      fprintf(output, "\t");
      print_node(output, var_value);

      vars_iter = ListIter_get_next(vars_iter);
    } /* loop on states */

    add_free(dd, add_state);
    bdd_free(dd, curr_state);

    /* completes the table with empty values for inputs: */
    for (j=0; j < num_of_combs+num_of_inputs; ++j) { fprintf(output, "\t-"); }
    fprintf(output, "\n");

    /* Combo and inputs, if not the last step */
    if (curr_input != BDD_INPUTS(NULL)) {
      add_ptr add_input;

      bdd_ptr bdd_combs;
      add_ptr add_combs;

      fprintf(output,"C%d\t", i+1);
      for (j=0; j < num_of_states; ++j) {fprintf(output, "-\t");}

      bdd_combs = bdd_and(dd, curr_state, curr_input);
      add_combs = bdd_to_add(dd, bdd_combs);
      bdd_free(dd, bdd_combs);

      vars = Encoding_get_model_state_input_symbols_list(self->senc);
      vars_iter = NodeList_get_first_iter(vars);
      while (!ListIter_is_end(vars_iter)) {
	node_ptr var;
	node_ptr var_value; 
	add_ptr var_add;
	add_ptr it_add;

	var = NodeList_get_elem_at(vars, vars_iter);
	var_add = BddEnc_expr_to_add(self->enc, var, Nil);
	it_add = add_if_then(dd, add_combs, var_add);
	var_value = add_value(dd, it_add);
	add_free(dd, it_add);
	add_free(dd, var_add);
	
	print_node(output, var_value);
	fprintf(output, "\t");
	
	vars_iter = ListIter_get_next(vars_iter);
      }
      add_free(dd, add_combs);
      for (j=0; j < num_of_inputs; ++j) {fprintf(output, "-\t");}
      fprintf(output, "\n");
      
      /* completes the table with empty values for states: */     
      fprintf(output,"I%d\t", i+1);
      for (j=0; j < num_of_states+num_of_combs; ++j) {fprintf(output, "-\t");}

      add_input = bdd_to_add(dd, curr_input);
      vars = Encoding_get_model_input_symbols_list(self->senc);
      vars_iter = NodeList_get_first_iter(vars);
      while (!ListIter_is_end(vars_iter)) {
	node_ptr var;
	node_ptr var_value; 
	add_ptr var_add;
	add_ptr it_add;
      
	var = NodeList_get_elem_at(vars, vars_iter);
	var_add = BddEnc_expr_to_add(self->enc, var, Nil);
	it_add = add_if_then(dd, add_input, var_add);
	var_value = add_value(dd, it_add);
	add_free(dd, it_add);
	add_free(dd, var_add);
      
	print_node(output, var_value);
      
	vars_iter = ListIter_get_next(vars_iter);
	if (!ListIter_is_end(vars_iter)) { fprintf(output, "\t"); }
      } /* loop on inputs */
      add_free(dd, add_input);
      bdd_free(dd, curr_input);
      fprintf(output, "\n");
    }
    
    i += 1;
    iter = TraceIterator_get_next(iter);
  } /* loop on steps */

  return 0;
}

/**Function********************************************************************

  Synopsis    [Print Trace in table format with each state in a seperate row.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static int 
trace_table_print_row_style(TraceTable_ptr self, Trace_ptr trace, FILE* output)
{
  TraceIterator_ptr iter;
  int i;

  /* Print Header */
  iter = Trace_begin(trace);
  i = 0;
  fprintf(output, "Name\t");
  while (!TraceIterator_is_end(iter)) {
    iter = TraceIterator_get_next(iter);
    fprintf(output, "S%d\t", i++); 
    
    if (!TraceIterator_is_end(iter)) { fprintf(output, "C%d\t", i); }
    if (!TraceIterator_is_end(iter)) { fprintf(output, "I%d\t", i); }
    else fprintf(output, "\n");
  }

  trace_table_print_vars_rows(self, trace, 
	      Encoding_get_model_state_symbols_list(self->senc), 
			      output);

  trace_table_print_vars_rows(self, trace, 
	      Encoding_get_model_state_input_symbols_list(self->senc), 
			      output);

  trace_table_print_vars_rows(self, trace, 
	      Encoding_get_model_input_symbols_list(self->senc), 
			      output);

  return 0;
}


/**Function********************************************************************

  Synopsis    [Prints the rows of name/values associated to given vars list]

  Description [This methods take into account of positions associated
  to state and input variables. It is provided in order to allow printing of 
  a group of variables (e.g. only the state vars, or only the input vars)]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_table_print_vars_rows(TraceTable_ptr self, Trace_ptr trace, 
					NodeList_ptr vars, 
					FILE* output)
{
  DdManager* dd = BddEnc_get_dd_manager(self->enc);
  TraceIterator_ptr iter;
  ListIter_ptr vars_iter;
  boolean is_input;
  boolean is_state;
  boolean is_combinatory;

  vars_iter = NodeList_get_first_iter(vars);
  while (!ListIter_is_end(vars_iter)) {
    node_ptr var; 
    add_ptr var_value_add;

    /* prints the var name */
    var = NodeList_get_elem_at(vars, vars_iter);

    is_input = NodeList_belongs_to(
		   Encoding_get_model_input_symbols_list(self->senc), var);
    
    is_state = NodeList_belongs_to(
		   Encoding_get_model_state_symbols_list(self->senc), var);
    
    is_combinatory = !(is_input || is_state);

    print_node(output, var);

    var_value_add = BddEnc_expr_to_add(self->enc, var, Nil);

    iter = Trace_begin(trace);
    while (!TraceIterator_is_end(iter)) {
      node_ptr var_value;
      add_ptr vars_add;
      add_ptr it_add;

      if (is_input) {
	BddInputs inputs;

	vars_add = BDD_INPUTS(NULL);
	inputs = TraceNode_get_input(iter);

	/* checks if this is the last step: */
	if (inputs != BDD_INPUTS(NULL)) {
	  vars_add = bdd_to_add(dd, inputs);
	  bdd_free(dd, inputs); 
	}
      }
      else if (is_combinatory) {
	BddInputs inputs;

	vars_add = BDD_INPUTS(NULL);
	inputs = TraceNode_get_input(iter);

	/* checks if this is the last step: */
	if (inputs != BDD_INPUTS(NULL)) {
	  BddInputs states;
	  BddStatesInputs states_inputs;

	  states = TraceNode_get_state(iter);	  
	  states_inputs = bdd_and(dd, states, inputs);
	  vars_add = bdd_to_add(dd, states_inputs);
	  
	  bdd_free(dd, states_inputs); 
	  bdd_free(dd, states); 
	  bdd_free(dd, inputs); 
	}
      }
      else {
	/* state variable: */
	BddStates states;

	nusmv_assert(is_state);

	states = TraceNode_get_state(iter);
	vars_add = bdd_to_add(dd, states);
	bdd_free(dd, states); 
      }

      if (vars_add != (add_ptr) NULL) {
	it_add = add_if_then(dd, vars_add, var_value_add);
	add_free(dd, vars_add);

	var_value = add_value(dd, it_add);
	add_free(dd, it_add);

	if (iter == Trace_begin(trace)) {
	  if (is_combinatory) fprintf(output, "\t-\t"); 
	  else if (is_input)  fprintf(output, "\t-\t-\t"); 
	  else fprintf(output, "\t");    
	}
	else {
	  fprintf(output, "\t-\t-\t"); 
	}

	/* prints the value of var */
	print_node(output, var_value);	
      }

      iter = TraceIterator_get_next(iter);
    } /* loop on time steps */

    add_free(dd, var_value_add);

    if (is_combinatory) fprintf(output, "\t-\t-\n"); 
    else if (is_input)  fprintf(output, "\t-\n"); 
    else fprintf(output, "\n");      
   
    vars_iter = ListIter_get_next(vars_iter);
  } /* loop on variables */

}



/**Function********************************************************************

  Synopsis    [Trace Table finalize method.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_table_finalize(Object_ptr object, void* dummy) 
{
  TraceTable_ptr self = TRACE_TABLE(object);

  trace_table_deinit(self);
  FREE(self);
}
