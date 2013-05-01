/**CFile***********************************************************************

  FileName    [range.c]

  PackageName [utils]

  Synopsis    [Contains function for checking of ranges and subranges]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
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

#include "range.h"
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: range.c,v 1.1.2.2.2.1 2005/11/16 12:09:47 nusmv Exp $";


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

/**Variable********************************************************************

  Synopsis    [Used by Utils_range_check]

  Description [Used by Utils_range_check callback]

  SeeAlso     [Utils_range_check, Utils_set_data_for_range_check]

******************************************************************************/
static node_ptr the_range = (node_ptr) NULL;


/**Variable********************************************************************

  Synopsis    [Used by Utils_range_check]

  Description [Used by Utils_range_check callback]

  SeeAlso     [Utils_range_check, Utils_set_data_for_range_check]

******************************************************************************/
static node_ptr the_var = (node_ptr) NULL;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Called before using Utils_range_check callback function]

  Description        []

  SideEffects        [Utils_range_check]

******************************************************************************/
void Utils_set_data_for_range_check(node_ptr var, node_ptr range)
{
  the_var = var;
  the_range = range;
}


/**Function********************************************************************

  Synopsis           [Checks if the values of <code>n</code> is in the
  range allowed for the variable.]

  Description        [Checks if the values of <code>n</code> is in the
  range allowed for the variable. The allowed values are stored in the
  global variable <code>the_range</code>, which is updated each time this
  function is called with the range allowed for the variable to be
  checked. If the value is not in the range, then error occurs.]

  SideEffects        [Utils_set_data_for_range_check]

******************************************************************************/
void Utils_range_check(node_ptr n)
{
  if (n == Nil) { internal_error("Utils_range_check: n == Nil"); }

  if (node_get_type(n) == CONS) {
    while (n != (node_ptr) NULL) {
      if (!in_list(car(n), the_range)) { range_error(car(n), the_var); }
      n = cdr(n);
    }
  }
  else if (!in_list(n, the_range)) { range_error(n, the_var); }
}


/**Function********************************************************************

  Synopsis           [Checks if the first argument is contained in the second.]

  Description        [Returns true if the first argument is contained in the
  set represented by the second, false otherwise. If the first
  argument is not a CONS, then it is considered to be a singleton.]

  SideEffects        [None]

  SeeAlso            [in_list]

******************************************************************************/
boolean Utils_is_in_range(node_ptr s, node_ptr d) 
{
  if (d == Nil) return false;

  if (node_get_type(s) == CONS) {
    while (s != Nil) {
      if (in_list(car(s), d) == 1) return true;
      s = cdr(s);
    }
  }
  else {
    if (in_list(s, d) == 1) return true;
  }

  return false;
}

 
/**Function********************************************************************

  Synopsis           [Checks that in given subrange n..m, n<=m]

  Description        [Returns True if in given subrange n..m n <= m. 
  Given node_ptr must be of TWODOTS type]

  SideEffects        []

  SeeAlso            [Utils_check_subrange_not_negative]

******************************************************************************/
boolean Utils_check_subrange(node_ptr subrange)
{
  int inf, sup;
  nusmv_assert(node_get_type(subrange) == TWODOTS);

  inf = node_get_int(car(subrange));
  sup = node_get_int(cdr(subrange));

  return inf < sup;		     
}


/**Function********************************************************************

  Synopsis           [Checks that in given subrange n..m, n<=m, and that n,m 
  are not negative]

  Description        [Check for correct positive (or zero) range]

  SideEffects        []

  SeeAlso            [Utils_check_subrange]

******************************************************************************/
boolean Utils_check_subrange_not_negative(node_ptr subrange)
{
  int inf, sup;
  nusmv_assert(node_get_type(subrange) == TWODOTS);

  inf = node_get_int(car(subrange));
  sup = node_get_int(cdr(subrange));

  return (inf >= 0) && Utils_check_subrange(subrange);
}

