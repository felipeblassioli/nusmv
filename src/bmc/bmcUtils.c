/**CFile***********************************************************************

  FileName    [bmcUtils.c]

  PackageName [bmc]

  Synopsis    [Utilities for the bmc package]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by ITC-irst and University of Trento.

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

#include "bmcInt.h"
#include "bmcUtils.h"
#include "bmcSatTrace.h"




#include <limits.h>

static char rcsid[] UTIL_UNUSED = "$Id: bmcUtils.c,v 1.19.12.4 2005/07/14 15:58:25 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define BMC_NO_LOOP   -(INT_MAX-1) /* must be negative! */
#define BMC_ALL_LOOPS BMC_NO_LOOP+1

/* ---------------------------------------------------------------------- */
/* You can define your own symbols for these: */
#define BMC_ALL_LOOPS_USERSIDE_SYMBOL "*"
#define BMC_NO_LOOP_USERSIDE_SYMBOL   "X"
/* ---------------------------------------------------------------------- */

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

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Given a string representing a loopback possible value,
  returns the corresponding integer. 
  The (optional) parameter result will be assigned to SUCCESS if the conversion
  has been successfully performed, otherwise to GENERIC_ERROR is the conversion
  failed. If result is NULL, SUCCESS is the aspected value, and an assertion is
  implicitly performed to check the conversion outcome.]

  Description        [Use this function to correctly convert a string
  containing a loopback user-side value to the internal representation of
  the same loopback value]

  SideEffects        [result will change if supplied]

  SeeAlso            []

******************************************************************************/
int Bmc_Utils_ConvertLoopFromString(const char* strValue, outcome* result)
{
  outcome res = SUCCESS;
  int l = 0;
  
  if (strValue == NIL(char)) {
    res = GENERIC_ERROR;
  }
  else if (Bmc_Utils_IsAllLoopbacksString(strValue))  {
    l = Bmc_Utils_GetAllLoopbacks();
  }
  else if (Bmc_Utils_IsNoLoopbackString(strValue))  {
    l = Bmc_Utils_GetNoLoopback();
  }
  else if (util_str2int(strValue, &l) == 0) {
    /* User could have supplied a private integer value which corresponds to 
       a reserved value:: */
    if (Bmc_Utils_IsAllLoopbacks(l) || Bmc_Utils_IsNoLoopback(l)) {
      res = GENERIC_ERROR;
    }
  }
  else res = GENERIC_ERROR; /* bad string value */

  /* This implements the auto-check (to simplify coding): */
  if (result == NULL)  nusmv_assert(res == SUCCESS);
  else  *result = res;
  
  return l;
}


/**Function********************************************************************

  Synopsis           [Given an integer containing the inner representation
  of the loopback value, returns as parameter the corresponding user-side
  value as string]

  Description        [Inverse semantic of 
  Bmc_Utils_ConvertLoopFromString. bufsize is the maximum buffer size]

  SideEffects        [String buffer passed as argument will change]

  SeeAlso            [Bmc_Utils_ConvertLoopFromString]

******************************************************************************/
void Bmc_Utils_ConvertLoopFromInteger(const int iLoopback,
				      char* szLoopback,
				      const int _bufsize)
{
  int iCheck; /* for buffer operations checking only */
  int bufsize = _bufsize-2; /* to store terminator */

  nusmv_assert(bufsize > 0); /* one character+terminator at least! */
  szLoopback[bufsize+1] = '\0'; /* put terminator */

  if (Bmc_Utils_IsAllLoopbacks(iLoopback)) {
    iCheck = snprintf(szLoopback, bufsize, "%s",
          BMC_ALL_LOOPS_USERSIDE_SYMBOL);
    nusmv_assert(iCheck>=0);
  }
  else if (Bmc_Utils_IsNoLoopback(iLoopback)) {
    iCheck = snprintf(szLoopback, bufsize, "%s", BMC_NO_LOOP_USERSIDE_SYMBOL);
    nusmv_assert(iCheck>=0);
  }
  else {
    /* value is ok, convert to string: */
    iCheck = snprintf(szLoopback, bufsize, "%d", iLoopback);
    nusmv_assert(iCheck>=0);
  }
}


/**Function********************************************************************

  Synopsis           [Returns true if l has the internally encoded "no loop"
  value]

  Description        [This is supplied in order to hide the internal value of 
  loopback which corresponds to the "no loop" semantic.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Bmc_Utils_IsNoLoopback(const int l)
{
  return (l == BMC_NO_LOOP)? true : false;
}


/**Function********************************************************************

  Synopsis           [Returns true if the given string represents the 
  no loopback value]

  Description        [This is supplied in order to hide the internal value of 
  loopback which corresponds to the "no loop" semantic.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Bmc_Utils_IsNoLoopbackString(const char* str)
{
  return (strcmp(str, BMC_NO_LOOP_USERSIDE_SYMBOL) == 0)? true : false;
}


/**Function********************************************************************

  Synopsis           [Returns true if the given loop value represents 
  a single (relative or absolute) loopback]

  Description        [Both cases "no loop" and "all loops" make this 
  function returning false, since these values are not single loops.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Bmc_Utils_IsSingleLoopback(const int l)
{
  return (Bmc_Utils_IsNoLoopback(l) == false)
    && (Bmc_Utils_IsAllLoopbacks(l) == false);
}


/**Function********************************************************************

  Synopsis           [Returns true if the given loop value represents 
  the "all possible loopbacks" semantic]

  Description        [This is supplied in order to hide the internal value of 
  loopback which corresponds to the "all loops" semantic.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Bmc_Utils_IsAllLoopbacks(const int l)
{
  return (l == BMC_ALL_LOOPS)? true : false;
}


/**Function********************************************************************

  Synopsis           [Returns true if the given string represents the 
  "all possible loops" value.]

  Description        [This is supplied in order to hide the internal value of 
  loopback which corresponds to the "all loops" semantic.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Bmc_Utils_IsAllLoopbacksString(const char* str)
{
  return (strcmp(str, BMC_ALL_LOOPS_USERSIDE_SYMBOL) == 0)? true : false;
}


/**Function********************************************************************

  Synopsis           [Returns the integer value which represents the 
  "no loop" semantic]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Bmc_Utils_GetNoLoopback() { return BMC_NO_LOOP; }


/**Function********************************************************************

  Synopsis           [Returns the integer value which represents the 
  "all loops" semantic]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Bmc_Utils_GetAllLoopbacks() { return BMC_ALL_LOOPS; }


/**Function********************************************************************

  Synopsis           [Returns a constant string  which represents the 
  "all loops" semantic.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
const char* Bmc_Utils_GetAllLoopbacksString() 
{
  return BMC_ALL_LOOPS_USERSIDE_SYMBOL;
}


/**Function********************************************************************

  Synopsis           [Converts a relative loop value (wich can also be 
  an absolute loop value) to an absolute loop value]

  Description        [For example the -4 value when k is 10 is 
  the value 6, but the value 4 (absolute loop value) is still 4]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Bmc_Utils_RelLoop2AbsLoop(const int upov_loop, const int k)
{
  if ((Bmc_Utils_IsNoLoopback(upov_loop))
      || (Bmc_Utils_IsAllLoopbacks(upov_loop))
      || (upov_loop >=0)) {
    return upov_loop;
  }
  else return k + upov_loop;
}


/**Function********************************************************************

  Synopsis           [Checks the (k,l) couple. l must be absolute.]

  Description        [Returns SUCCESS if k and l are compatible, otherwise
  return GENERIC_ERROR]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
outcome Bmc_Utils_Check_k_l(const int k, const int l)
{
  outcome ret = GENERIC_ERROR;

  if ( (k>=0) &&                    /* k has to be non-negative in all cases */
       ( Bmc_Utils_IsNoLoopback(l)  /* the no-loop case */
         ||
         Bmc_Utils_IsAllLoopbacks(l) /* the all-loops case */
         ||
         ( (l>=0) && (l<k) )  /* the single-loop case with the new semantics */
        )
      )
    ret = SUCCESS;

  return ret;
}


/**Function********************************************************************

  Synopsis           [Given time<=k and a \[l, k\] interval, returns next
  time, or BMC_NO_LOOP if time is equal to k and there is no loop]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Bmc_Utils_GetSuccTime(const int time, const int k, const int l)
{
  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );

  if (Bmc_Utils_IsNoLoopback(l))
    if (time<k)
      return (time + 1);
    else
      return l;
  else
    if (time < k-1)
      return (time + 1);
    else
      return l;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Search into a given string any symbol which belongs to a
  determined set of symbols, and expand each found symbol, finally returning
  the resulting string]

  Description        [This function is used in order to perform the macro expansion
  of filenames. table_ptr is the pointer to a previously prepared table which fixes
  any corrispondence from symbol to strings to be substituited from.
  table_len is the number of rows in the table (i.e. the number of symbols to
  search for.)]

  SideEffects        [filename_expanded string data will change]

  SeeAlso            []

******************************************************************************/
void Bmc_Utils_ExpandMacrosInFilename(const char* filename_to_be_expanded,
				      const SubstString* table_ptr,
				      const size_t table_len,
				      char* filename_expanded,
				      size_t buf_len)
{
  int i;
  /* copy the source string into the destination one: */
  strncpy(filename_expanded, filename_to_be_expanded, buf_len);

  for(i=0; i < table_len; ++i) {
      apply_string_macro_expansion(table_ptr + i, filename_expanded, buf_len);
    } /* for each symbol template */
}




/**Function********************************************************************

  Synopsis           [Given a problem, and a solver containing a model 
  for that problem, generates and prints a counter-example]

  Description [A trace is generated and printed using the currently
  selected plugin. Generated trace is returned, in order to make
  possible for the caller to do some other operation, like association
  with the checked property. Returned trace object *cannot* be
  destroyed by the caller.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Trace_ptr Bmc_Utils_generate_and_print_cntexample(BmcVarsMgr_ptr vars_mgr, 
						  SatSolver_ptr solver, 
						  BddEnc_ptr bdd_enc, 
						  be_ptr be_prob, 
						  const int k, 
						  const char* trace_name)
{
  BmcSatTrace_ptr sat_trace;
  Trace_ptr trace;
  lsList be_model;
  node_ptr path; 
  int bmc_tr;
	  
  be_model = Be_CnfModelToBeModel(BmcVarsMgr_GetBeMgr(vars_mgr), 
				  SatSolver_get_model(solver));

  sat_trace = BmcSatTrace_create(be_prob, be_model);
  path = NodeList_to_node_ptr(
	    BmcSatTrace_get_symbolic_model(sat_trace, vars_mgr, k));
	  
  trace = Trace_create_from_state_input_list(bdd_enc,
					     trace_name,
					     TRACE_TYPE_CNTEXAMPLE,
					     path);

  bmc_tr = TraceManager_register_trace(global_trace_manager, trace);
	  
  /* Print the trace using default plugin */
  fprintf(nusmv_stdout,
	  "-- as demonstrated by the following execution sequence\n");

  TraceManager_execute_plugin(global_trace_manager, 
		      TraceManager_get_default_plugin(global_trace_manager),
		      bmc_tr);

  BmcSatTrace_destroy(&sat_trace);
  lsDestroy(be_model, 0);

  return trace;
}




/**Function********************************************************************

  Synopsis [Creates a list of be variables that are intended to be
  used by the routine that makes the state unique in invariant checking.]

  Description   [If coi is enabled, than the returned list will contain only 
  those boolean state variable given property actually depends on. Otherwise 
  the set of state boolean vars will occur in the list. 
  
  Returned list must be destroyed by the called.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
lsList Bmc_Utils_get_vars_list_for_uniqueness(BmcVarsMgr_ptr vars_mgr, 
					      Prop_ptr invarprop)
{
  SexpFsm_ptr bool_sexp_fsm;
  Encoding_ptr senc;
  node_ptr iter;
  lsList crnt_state_be_vars;

  senc = BmcVarsMgr_GetSymbEncoding(vars_mgr);
  crnt_state_be_vars = lsCreate();
  bool_sexp_fsm = Prop_get_bool_sexp_fsm(invarprop);
      
  nusmv_assert(SEXP_FSM(NULL) != bool_sexp_fsm);
  /* if coi was performed the list will not contain unnecessary vars */

  iter = SexpFsm_get_vars_list(bool_sexp_fsm);      
  while (iter != Nil) {
    node_ptr sexp_var = car(iter);
	
    if (Encoding_is_symbol_state_var(senc, sexp_var)) {
      be_ptr be_var;
      lsStatus status;

      if (Encoding_is_symbol_boolean_var(senc, sexp_var)) {
	be_var = BmcVarsMgr_Name2Curr(vars_mgr, sexp_var);	  
	status = lsNewEnd(crnt_state_be_vars, (lsGeneric) be_var, 0);
	nusmv_assert(LS_OK == status);
      }
      else {
	/* scalar var, retrieves the list of bits that make its encoding */
	NodeList_ptr bits;
	ListIter_ptr bits_iter;
	bits = Encoding_get_var_encoding_bool_vars(senc, sexp_var);
	bits_iter = NodeList_get_first_iter(bits);
	while (! ListIter_is_end(bits_iter)) {
	  node_ptr bit;
	  bit = NodeList_get_elem_at(bits, bits_iter);
	  be_var = BmcVarsMgr_Name2Curr(vars_mgr, bit);	  
	  status = lsNewEnd(crnt_state_be_vars, (lsGeneric) be_var, 0);
	  nusmv_assert(LS_OK == status);	      
	      
	  bits_iter = ListIter_get_next(bits_iter);
	}
	NodeList_destroy(bits);
      }
    }

    iter = cdr(iter);
  } /* loop */

  return crnt_state_be_vars;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


