/**CFile***********************************************************************

  FileName    [TraceLabel.c]

  PackageName [trace]

  Synopsis    [Routines related to functionality related to a node of a trace.]

  Description [This file contains the definition of the \"TraceLabel\" class.]
		
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

#include "TraceLabel.h"
#include "node/node.h"
#include "parser/symbols.h" 
#include "parser/parser.h" 

static char rcsid[] UTIL_UNUSED = "$Id: TraceLabel.c,v 1.1.2.4 2004/05/31 13:58:13 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [TraceLabel Constructor]

  Description [returns a label for the specified trace and state index.]

  SideEffects []

  SeeAlso     [TraceLabel_create_from_string]

******************************************************************************/
TraceLabel TraceLabel_create(int trace_id, int state_id)
{
  TraceLabel self;

  self = find_node(DOT, find_node(NUMBER, (node_ptr) trace_id, Nil), 
                   find_node(NUMBER, (node_ptr)state_id, Nil));
  return self;
}

/**Function********************************************************************

  Synopsis    [TraceLabel Constructor]

  Description [creates the label from the specified string. In case of any
  error, it returns TRACE_LABEL_INVALID as result.]

  SideEffects []

  SeeAlso     [TraceLabel_create]

******************************************************************************/
TraceLabel TraceLabel_create_from_string(char* str)
{
  TraceLabel self = TRACE_LABEL_INVALID;
  char* label_str[] = {"goto_state", str, " "};
  node_ptr parsed_command = Nil;

  if (Parser_ReadCmdFromString(3, label_str, "GOTO ", 
			       ";\n", &parsed_command) == 0) {
    if ((parsed_command != Nil) && (node_get_type(parsed_command) == GOTO)) {
      node_ptr label = car(parsed_command);
      int trace_no = (int) car(car(label));
      int state_no = (int) car(cdr(label));

      self = TraceLabel_create(trace_no-1, state_no-1);
    }
  }
  return self;
}

/**Function********************************************************************

  Synopsis    [Returns the trace index associated with the TraceLabel.]

  Description []

  SideEffects []

  SeeAlso     [TraceLabel_get_state]

******************************************************************************/
int TraceLabel_get_trace(TraceLabel self)
{
  int result = (int)car(car((node_ptr)self));

  return result;
}

/**Function********************************************************************

  Synopsis    [Returns the state index associated with the TraceLabel.]

  Description []

  SideEffects []

  SeeAlso     [TraceLabel_get_trace]

******************************************************************************/
int TraceLabel_get_state(TraceLabel self)
{
  int result = (int)car(cdr((node_ptr)self));

  return result;
}

