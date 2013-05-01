/**CFile***********************************************************************

  FileName [bmcVarsMgr.c]

  PackageName [bmc]

  Synopsis [Variables Manager for the <tt>bmc</tt> package]

  Description []

  SeeAlso []

  Author [Alessandro Cimatti, Lorenzo Delana, Roberto Cavada]

  Copyright [ This file is part of the ``bmc'' package of NuSMV
  version 2.  Copyright (C) 2000-2001 by ITC-irst and University of
  Trento.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  as published by the Free Software Foundation; either version 2 of
  the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.

  For more information of NuSMV see <http://nusmv.irst.itc.it> or
  email to <nusmv-users@irst.itc.it>.  Please report bugs to
  <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to
  <nusmv@irst.itc.it>. ]

******************************************************************************/

#include "bmcVarsMgr.h"
#include "bmcInt.h" 

#include "be/be.h" 

static char rcsid[] UTIL_UNUSED = "$Id: bmcVarsMgr.c,v 1.23.4.6.2.5 2005/07/14 15:58:25 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis [Variables Manager for BMC. This class implements the
  boolean encoding for BMC, by providing variables groups and
  time-related functions. If the Boolean Exprttession (BE) layer
  provides actual access to variables, such as creation, shifting,
  substitution, expressions, etc., the BmcVarsMgr class provides the
  right semantics for those variables.]
  
  Description [Variables Manager for BMC. This class implements the
  boolean encoding for BMC, by providing variables groups and
  time-related functions. If the Boolean Exprttession (BE) layer
  provides actual access to variables, such as creation, shifting,
  substitution, expressions, etc., the BmcVarsMgr class provides the
  right semantics for those variables. The idea is to provide:
  1. A structured layer for variables (encoding)
  2. A set of functionalities to manage the variables. 

  The encoding is internally organized to provide efficient operations
  on generic BEs, like variables shifting and substitutions, and
  conversions between symbolic variables to BE variables, BE indices,
  etc.

  Encoding's structure is splitted into two distinct parts: 
  1.1 Untimed variables block.
  1.2 Timed variables block.

  1.1 Untimed variables block 

      Keeps the set of untimed BE variables, such that any (untimed)
      expression in term of BE will contain references to variables
      located in this logical block. This block is splitted into three
      sub-blocks, that group respectively untimed current state
      variables, input variables, and next state variables.

                 ---- -- ----      * : current state variable
	        |****|==|####|     = : input variable
                 ---- -- ----      # : next state variable
                 0123 45 6789      n : BE variable index

      The picture shows an example of a logical untimed variables
      block. The model contains 6 boolean variables (indices from
      0..5), where indices 0..3 refer the state variables, and indices
      4 and 5 refer the input variables. The set of state variables is
      than replicated to represent the next state variables, referred
      by indices 6..9 that constitutes the third sub-block in the
      picture.

      Notice that indices 0..9 refer a boolean variable allocated by
      the BE layer, that does not distinguish between state, input, or
      next variables. 


  1.2 Timed variables block
      
      Following the untimed variables block, the timed variables block
      holds the set of state and input variables that are instantiated
      at a given time. A BE expression instantiated at time t, will
      contain BE variables that belong to this block. 

      Timed vars block is logically splitted into separate frames,
      each of one corresponding to a given time t.  The structure of
      each frame depends on the specific time t and on the number of
      transisitions the model has. When the problem length k is 0,
      only frame from time 0 is allocated, and this frame is
      constituted by only state variables. In this condition the
      encoding structure (with untimed and timed blocks) would be
      this:

                    Untimed block          Timed block
        |-------------------------------| |-----------|
           current    input      next        state 0
         -- -- -- --  -- --  -- -- -- --   -- -- -- --
        |  |  |  |  ||  |  ||  |  |  |  | |  |  |  |  | 
         -- -- -- --  -- --  -- -- -- --   -- -- -- --
         00 01 02 03  04 05  06 07 08 09   10 11 12 13
      
      In the example BE variables 10..13 are allocated to keep current
      state variables at the initial state (time 0). Since there are
      no transitions, input variables are not allocated for this value
      of problem length k.

      When k=1, the encoding becomes:

                    Untimed block                     Timed block
        |-------------------------------| |-------------------------------|
           current    input      next        state 0   input 0   state 1
         -- -- -- --  -- --  -- -- -- --   -- -- -- --  -- --  -- -- -- --
        |  |  |  |  ||  |  ||  |  |  |  | |  |  |  |  ||  |  ||  |  |  |  |
         -- -- -- --  -- --  -- -- -- --   -- -- -- --  -- --  -- -- -- --
         00 01 02 03  04 05  06 07 08 09   10 11 12 13  14 15  16 17 18 19

      When k>0 the timed block is a sequence of state (input state)+
      timed sub-blocks. In the example above BE variables 14 and 15
      have been added to encode input variables at time 0 (since there
      is a transition leading to time 1), and variables 16..19 have
      been added to encode state variables at time 1.
      
      When a generic, untimed BE expressions E is instantiated at time
      t (i.e. is shifted to time t), each current state variable
      occurring in E will be replaced by the corresponding BE variable
      in the timed subblock t, as well any occurring input
      variable. Each next state variable will be replaced by the
      corresponding state variable located in the timed subblock
      (t+1).
      
      It is important to notice that the problem length k must be
      strictly taken into account when shifting operations are
      performed. In particular, as there are not transitions outgoing
      the last state at time t=k, neither inputs nor next states can
      be shifted at time t=k.
      
      From the logics point of view, when t=k inputs variables at time
      t do not exist at all (and the encoding directly maps this
      idea).  Because of this, the logic value of input variables at
      time t=k is false.
      
  
  * Some notes about indices and timings *

  It is important to explain the distinction between Variable indices
  and Be indices. Variable indices are BE indices that only refer
  variables in the untimed current state and input variables
  subblocks. In the 6 vars example above, indices 0..5 are Be
  variables indices, but are Variables indices as well. BE indices are
  0..19 when k=1. 

  The class BmcVarsMgr provides a large set of methods to query
  indices, and to convert indices to variables and vice-versa. 

  For example, BmcVarsMgr provides methods to know whether a given BE
  variable index belongs to the untimed vars block (i.e. it is a
  Variable index as well), and which sub-block it belongs to.
  
  See the class interface for further details on the provided
  features.
  ]

  SeeAlso     [bmc_vars_init, bmc_vars_quit]

******************************************************************************/
typedef struct BmcVarsMgr_TAG
{
  /* ----------------------------------------------------------------------- */
  /*    Accessible generic part, tied to rbc interface                       */
  /* ----------------------------------------------------------------------- */
  Encoding_ptr senc;               /* the symbolic encoding */
  Be_Manager_ptr env;              /* the boolean expr manager */
  int state_vars_num;              /* the number of state variables */
  int input_vars_num;              /* the number of input variables */
  int maxtime;                     /* max time reached until now for the
				      variables environment */

  /* ----------------------------------------------------------------------- */
  /*    Private part specific of implementation                              */
  /* ----------------------------------------------------------------------- */
  hash_ptr varname2currvar_table;   /* table for var name -> be var 
				     conversion */
  node_ptr* varindex2varname_table; /* table for var index -> var name
				       conversion */
  int* cnexp_subst_array;          /* table for current and next vars
				      substitution */

} BmcVarsMgr;


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
static boolean bmc_vars_mgr_extend_max_time ARGS((BmcVarsMgr_ptr self,
						  const int new_maxtime));

static int 
bmc_vars_mgr_get_max_be_var_index ARGS((const BmcVarsMgr_ptr self, 
					const int max_time));


/*---------------------------------------------------------------------------*/
/* Declaration of private functions                                          */
/*---------------------------------------------------------------------------*/

/* Following private wrappers assure a more generic access to the be part
   into Variables Manager structure: */
static void set_maxtime ARGS((BmcVarsMgr_ptr self, const int iMaxTime));

static void bmc_vars_mgr_init_tables ARGS((BmcVarsMgr_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [<b><i>Initialize</i> <tt>BMC</tt> Variables Manager</b>]

  Description        [Constructor of class BmcVarsMbr class. Caller keeps the 
  ownership of passed Encoding instance]

  SideEffects        []

  SeeAlso            [bmc_vars_mgr_init_tables]

******************************************************************************/
BmcVarsMgr_ptr BmcVarsMgr_Create(Encoding_ptr symb_encoding)
{

  BmcVarsMgr_ptr self = BMC_VARS_MGR(NULL);
  Be_Manager_ptr be_mgr; 
  int tmp;

  /* Allocates the variables manager core data container */
  self = ALLOC(BmcVarsMgr, 1);
  BMC_VARS_MGR_CHECK_INSTANCE(self);

  self->senc = symb_encoding;

  /* determine the number of state variables */
  self->state_vars_num = Encoding_get_bool_state_vars_num(self->senc);
  self->input_vars_num = Encoding_get_bool_input_vars_num(self->senc);
  
  if (opt_verbose_level_gt(options, 1)) {
    fprintf(nusmv_stderr, 
	    "BMC Vars Manager contains %d state variable(s) and %d input variable(s)\n",
	    self->state_vars_num, self->input_vars_num);
  }

  /* allocate variables for (current, input and next) frames
     used in initial model construction, and for the zero time frame.
     The variables environment is the manager for the be package */
  be_mgr = Be_RbcManager_Create((2 + 1) * self->state_vars_num + 
				self->input_vars_num);
  self->env = be_mgr;

  /* setup maxtime indicator in the variables manager */
  set_maxtime(self, 0);

  /* allocate the instance for the statevar->curvar table */
  self->varname2currvar_table = new_assoc();

  /* allocate the instance for the curvar->statevar table */
  self->varindex2varname_table = ALLOC(node_ptr, self->state_vars_num + 
				       self->input_vars_num);
  nusmv_assert(self->varindex2varname_table != NULL);

  /* scan input and state variables and update be <-> bexp tables */
  bmc_vars_mgr_init_tables(self);

  /* alloc the array for variables substitution */
  tmp = (BmcVarsMgr_GetStateInputVarsNum(self) + 
	 BmcVarsMgr_GetStateVarsNum(self));

  if (opt_verbose_level_gt(options, 4)) {
    fprintf(nusmv_stderr, "Allocating cnexp_subst_array: %d locations\n", tmp);
  }

  self->cnexp_subst_array = ALLOC(int, tmp);
  nusmv_assert(self->cnexp_subst_array != NULL);
  
  return self;
}


/**Function********************************************************************

  Synopsis           [<b><i>Quit</i> from <tt>BMC</tt> Variables Manager</b>]

  Description        []

  SideEffects        [Follow structures are freed in the <tt>variables
  manager (data container)</tt>:<br>
  - variables <tt>substitution array</tt>
  - variables <tt>current vars->state vars</tt> conversion table<br>
  - variables <tt>state vars->current vars</tt> conversion table<br>
  - variables <tt>environment</tt><br>
  Free <tt>variables manager (data container)</tt><br>]

  SeeAlso            [bmc_vars_init]

******************************************************************************/
void BmcVarsMgr_Delete(BmcVarsMgr_ptr* self_ref)
{
  nusmv_assert(self_ref != NULL);
  if (*self_ref == NULL) return;


  /* free the array for substitution */
  FREE((*self_ref)->cnexp_subst_array);

  /* free the instance for the curvar->statevar table */
  FREE((*self_ref)->varindex2varname_table);

  /* free the instance for the statevar->curvar table */
  clear_assoc((*self_ref)->varname2currvar_table);
  free_assoc((*self_ref)->varname2currvar_table);

  /* free the variables environment */
  Be_RbcManager_Delete(BmcVarsMgr_GetBeMgr(*self_ref));

  /* free the variables manager core data container */
  FREE(*self_ref);
  *self_ref = NULL;
}


/**Function********************************************************************

  Synopsis  [Returns the <tt>number of state variables currently
  handled by the Variables Manager</tt>]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_GetStateVarsNum(const BmcVarsMgr_ptr self) 
{ 
  BMC_VARS_MGR_CHECK_INSTANCE(self);

  return self->state_vars_num; 
}


/**Function********************************************************************

  Synopsis  [Returns the <tt>number of input variables currently
  handled by the Variables Manager</tt>]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_GetInputVarsNum(const BmcVarsMgr_ptr self) 
{ 
  BMC_VARS_MGR_CHECK_INSTANCE(self);

  return self->input_vars_num; 
}


/**Function********************************************************************

  Synopsis           [<b>Returns</b> the
  <tt>number of variables currently handled by the Variables Manager</tt>]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_GetStateInputVarsNum(const BmcVarsMgr_ptr self) 
{ 
  BMC_VARS_MGR_CHECK_INSTANCE(self);

  return self->state_vars_num + self->input_vars_num;
}


/**Function********************************************************************

  Synopsis           [<b>Returns</b> the
  <tt>maximum time currently handled by the variable environment</tt>]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_GetMaxTime(const BmcVarsMgr_ptr self)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  return self->maxtime;
}


/**Function********************************************************************

  Synopsis           [Provides the first variable index of input variables]

  Description        [Returns the first var index of the set of input
  variables. This helps on not assuming anything about how the encoding is 
  structured. Returns a valid index even if there are no input variables.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_GetFirstInputVarIndex(const BmcVarsMgr_ptr self)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  
  return BmcVarsMgr_GetStateVarsNum(self);;
}


/**Function********************************************************************

  Synopsis [Provides the first variable index of current state
  variables]

  Description        [Returns the first var index of the set of current state 
  variables. This helps on not assuming anything about how the encoding is 
  structured. Returns a valid index even if there are no state vars. 

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_GetFirstCurrStateVarIndex(const BmcVarsMgr_ptr self)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  
  return 0;
}


/**Function********************************************************************

  Synopsis [Provides the first variable index of next state variables]

  Description        [Returns the first var index of the set of next state 
  variables. This helps on not assuming anything about how the encoding is 
  structured. Returns a valid index even if there are no state variables.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_GetFirstNextStateVarIndex(const BmcVarsMgr_ptr self)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);

  return BmcVarsMgr_GetStateInputVarsNum(self);
}



/**Function********************************************************************

  Synopsis           [Returns the symbolic Encoding instance used by self]

  Description        [Returns the symbolic Encoding instance used by self. 
  self keeps the ownership of the returned object]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Encoding_ptr BmcVarsMgr_GetSymbEncoding(const BmcVarsMgr_ptr self)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  return self->senc;
}


/**Function********************************************************************

  Synopsis           [<b>Returns</b> the
  <tt>Boolean Expressions manager</tt> contained into the variable manager]

  Description        [Warning: do not delete the returned instance of 
  Be_Manager class]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Be_Manager_ptr BmcVarsMgr_GetBeMgr(const BmcVarsMgr_ptr self)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  return self->env;
}



/**Function********************************************************************

  Synopsis           [<b>Shift</b> given <i>current</i> <b>expression at
  next time</b>]

  Description        [Warning: argument 'exp' must contain only variables in
  current time, otherwise results will be unpredictible, surely inconsistent]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_ShiftCurrState2Next(const BmcVarsMgr_ptr self,
				      const be_ptr exp)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  return Be_ShiftVar( BmcVarsMgr_GetBeMgr(self), exp,
		      BmcVarsMgr_GetStateVarsNum(self) + 
		      BmcVarsMgr_GetInputVarsNum(self) );
}


/**Function********************************************************************

  Synopsis           [<b>Shift</b> given <i>current, next</i> <b>expression at
  specified <tt>time</tt></b>]

  Description        []

  SideEffects        [extends variables environment and maxtime, if required]

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_ShiftCurrNext2Time(const BmcVarsMgr_ptr self,
				     const be_ptr exp, const int time)
{
  
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert(time >= 0);

  /* extends maxtime, if required */
  bmc_vars_mgr_extend_max_time(self, time + 1);

  /* return exp with variables shifted at given time */
  return Be_ShiftVar( BmcVarsMgr_GetBeMgr(self), exp,
		      BmcVarsMgr_Time2BeIndex(self, time) );
}


/**Function********************************************************************

  Synopsis           [Shifting of current and inputs variables of exp 
  to time ctime, and of next vars to time ntime]

  Description [If required, this method extends the problem length
  (maxtime) to max(ctime, ntime). If ctime is or becomes maxtime,
  input variables will not be shifted (and result could not be what
  you expect). 
  Returns the shifted BE. 

  ===== WARNING!!!! === WARNING!!!! === WARNING!!!! === WARNING!!!! =====
  This method does not work with inputs, so make sure there are not
  inputs when you invoke it. The implementation of this method must be
  fixed to handle inputs.
  =======================================================================

  ]

  SideEffects        [extends variables environment and maxtime, if required]

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_ShiftCurrNext2Times(const BmcVarsMgr_ptr self,
				      const be_ptr exp,
				      const int ctime, const int ntime)
{
  int i;
  int firstvar_at_ctime, firstvar_at_ntime;
  int first_next_idx = 0;
  boolean extended; 

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  if (BmcVarsMgr_GetInputVarsNum(self) > 0) {
    internal_error("BmcVarsMgr_ShiftCurrNext2Times: "
		   "input variables are not supported by this method.\n");
  }

  /* lazy evaluation */
  if (Be_IsConstant(BmcVarsMgr_GetBeMgr(self), exp)) return exp;

  /* extends maxtime, if required */
  extended = bmc_vars_mgr_extend_max_time(self, max(ctime, ntime));

  first_next_idx = BmcVarsMgr_GetFirstNextStateVarIndex(self);

  firstvar_at_ctime = BmcVarsMgr_Time2BeIndex(self, ctime);
  firstvar_at_ntime = BmcVarsMgr_Time2BeIndex(self, ntime);

  /* sets current and next substitution indices */
  for (i = 0; i < BmcVarsMgr_GetStateVarsNum(self); ++i) {
    /* setup substitution for ith-curvar in the array for substitution,
       with ith-timedvar at time ctime */
    self->cnexp_subst_array[i] = firstvar_at_ctime + i;

    /* setup substitution for ith-nextvar in the array for substitution,
       with ith-timedvar at time ntime */
    self->cnexp_subst_array[i + first_next_idx] = firstvar_at_ntime + i;
  }

  /* return given exp with symbolic (current, next) variables substituited
     with correspondent timed variables at times ctime and ntime, resp. */
  return Be_VarSubst(BmcVarsMgr_GetBeMgr(self), exp, self->cnexp_subst_array);
}


/**Function********************************************************************

  Synopsis           [<b>Make an AND interval</b> of given expression using
  <b>range \[<tt>from</tt>, <tt>to</tt>\]</b>]

  Description        []

  SideEffects        [be hash may change]

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_MkAndCurrNextInterval(const BmcVarsMgr_ptr self,
					 const be_ptr exp,
					 const int from, const int to)
{
  be_ptr res;
  
  BMC_VARS_MGR_CHECK_INSTANCE(self);

  /* We accept the cases (from <= to) and (from == to - 1).
     The latter may exist when the unrolling is performed at high level */
  nusmv_assert(from <= to+1);

  if (from > to) {
    /* ends the recursion */
    res = Be_Truth(BmcVarsMgr_GetBeMgr(self));
  }
  else {
    res =  Be_And( BmcVarsMgr_GetBeMgr(self),
		   BmcVarsMgr_MkAndCurrNextInterval(self, exp,
						    from, to-1),
		   BmcVarsMgr_ShiftCurrNext2Time(self, exp, to) );
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [<b>Make an OR interval</b> of given expression using
  <b>range \[<tt>from</tt>, <tt>to</tt>\]</b>]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_MkOrCurrNextInterval(const BmcVarsMgr_ptr self,
					const be_ptr exp,
					const int from, const int to)
{
  be_ptr res;
  
  BMC_VARS_MGR_CHECK_INSTANCE(self);

  /* We accept the cases (from <= to) and (from == to - 1).
     The latter may exist when the unrolling is performed at high level */
  nusmv_assert(from <= to+1);

  if (from > to) {
    /* ends the recursion */
    res = Be_Falsity(BmcVarsMgr_GetBeMgr(self));
  }
  else {
    res =  Be_Or( BmcVarsMgr_GetBeMgr(self),
		  BmcVarsMgr_ShiftCurrNext2Time(self, exp, from),
		  BmcVarsMgr_MkOrCurrNextInterval(self, exp,
						  from + 1, to) );
  }
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Search the association from given symbolic variable and
  its relative be representation, and returns the corresponding be]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_Name2Curr(const BmcVarsMgr_ptr self,
			    const node_ptr var_name)
{
  be_ptr curvar;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  curvar = (be_ptr) find_assoc(self->varname2currvar_table, var_name);  

  if (curvar == (be_ptr) NULL) {
    if (Encoding_is_symbol_determ_var(self->senc, var_name)) {
      internal_error("Unknown determinization variable. This is known bug #3.");
    }
    else  nusmv_assert(curvar != (be_ptr) NULL);
  }

  return curvar;
}


/**Function********************************************************************

  Synopsis           [Given the relative index returns the (symbolic) variable
  which addresses]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
node_ptr BmcVarsMgr_VarIndex2Name(const BmcVarsMgr_ptr self, const int index)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert((index >= 0) && 
	       (index < BmcVarsMgr_GetStateInputVarsNum(self)));

  return (self->varindex2varname_table)[index];
}


/**Function********************************************************************

  Synopsis [Given a be representation of a current variable, returns
  the relative symbolic variable]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
node_ptr BmcVarsMgr_Curr2Name(const BmcVarsMgr_ptr self, const be_ptr curvar)
{
  int curvar_index;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  curvar_index = Be_Var2Index(BmcVarsMgr_GetBeMgr(self), curvar);
  nusmv_assert( (curvar_index >= 0)
		&& (curvar_index < BmcVarsMgr_GetStateInputVarsNum(self)) );
  
  return BmcVarsMgr_VarIndex2Name(self, curvar_index);
}


/**Function********************************************************************

  Synopsis           [Like BmcVarsMgr_VarIndex2Name but in terms of be
  representation]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_VarIndex2Curr(const BmcVarsMgr_ptr self, const int index)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert( (index >= 0) && 
		(index < BmcVarsMgr_GetStateInputVarsNum(self)) );

  return Be_Index2Var(BmcVarsMgr_GetBeMgr(self), index);
}


/**Function********************************************************************

  Synopsis           [Like BmcVarsMgr_VarIndex2Curr but relatively to next state
  variables block]

  Description        [given index must be relative to a state variable]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_VarIndex2Next(const BmcVarsMgr_ptr self, const int index)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert((index >= 0) && (index < BmcVarsMgr_GetStateVarsNum(self)));

  return Be_Index2Var( BmcVarsMgr_GetBeMgr(self),
		       BmcVarsMgr_GetStateInputVarsNum(self) + index );
}


/**Function********************************************************************

  Synopsis           [Returns the i-th variable at given time]

  Description        [If the variable is an input variable, time must 
  be lower than maxtime]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_VarIndex2Timed(const BmcVarsMgr_ptr self,
				 const int index, const int time, 
				 const int max_time)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);

  /* extends maxtime, if required */
  bmc_vars_mgr_extend_max_time(self, time);

  nusmv_assert(time <= max_time);
  nusmv_assert(max_time <= BmcVarsMgr_GetMaxTime(self));
  nusmv_assert(BmcVarsMgr_IsBeIndexInCurrStateInputBlock(self, index));

  if (BmcVarsMgr_IsBeIndexInInputBlock(self, index)) {
    /* input var */
    nusmv_assert(time < max_time);
  }

  return Be_Index2Var( BmcVarsMgr_GetBeMgr(self), 
		       (BmcVarsMgr_Time2BeIndex(self, time) + index) );
}


/**Function********************************************************************

  Synopsis [Given a be current state variable, returns the
  corresponding be state variable into the next variables block]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_Curr2Next(const BmcVarsMgr_ptr self, const be_ptr curvar)
{
  int curvar_index; 

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  curvar_index = Be_Var2Index(BmcVarsMgr_GetBeMgr(self), curvar);
  nusmv_assert((curvar_index >= 0)
	       && (curvar_index < BmcVarsMgr_GetStateVarsNum(self)));
  
  return BmcVarsMgr_VarIndex2Next(self, curvar_index);
}


/**Function********************************************************************

  Synopsis [Given a symbolic state variable, returns the corresponding
  be variable that belongs to the next state variables block]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_Name2Next(const BmcVarsMgr_ptr self, const node_ptr var_name)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert(Encoding_is_symbol_state_var(self->senc, var_name));
  
  return BmcVarsMgr_Curr2Next(self,
			      BmcVarsMgr_Name2Curr(self, var_name));
}


/**Function********************************************************************

  Synopsis [Given a symbolic (state or input) variable, returns the
  corresponding variable index into the current-state/input block]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_Name2VarIndex(const BmcVarsMgr_ptr self, 
			     const node_ptr var_name)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  
  return Be_Var2Index(BmcVarsMgr_GetBeMgr(self), 
		      BmcVarsMgr_Name2Curr(self, var_name));
}


/**Function********************************************************************

  Synopsis           [Given a be variable, that is supposed to belong to the
  next state variables group, returns the be variable
  corrisponding to the timed index variables block]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_Next2Timed(const BmcVarsMgr_ptr self,
			     const be_ptr nextvar, const int time)
{
  int nextvar_index;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  /* extends maxtime, if required */
  bmc_vars_mgr_extend_max_time(self, time + 1);

  nextvar_index = Be_Var2Index(BmcVarsMgr_GetBeMgr(self), nextvar);
  nusmv_assert(BmcVarsMgr_IsBeIndexInNextStateBlock(self, nextvar_index));
  
  /* We can use -1 as maxtime below, since it is a state variable */
  return BmcVarsMgr_VarIndex2Timed(self,
			nextvar_index - BmcVarsMgr_GetStateInputVarsNum(self),
			time, -1);
}


/**Function********************************************************************

  Synopsis [Given a be variable, that is supposed to belong to the
  current (state or input) variables group, returns the be variable
  corrisponding to the timed index variables block]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_Curr2Timed(const BmcVarsMgr_ptr self,
			     const be_ptr curvar, const int time, 
			     const int max_time)
{
  int curvar_index;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  /* extends maxtime, if required */
  bmc_vars_mgr_extend_max_time(self, time);

  curvar_index = Be_Var2Index(BmcVarsMgr_GetBeMgr(self), curvar);

  nusmv_assert(BmcVarsMgr_IsBeIndexInCurrStateInputBlock(self, curvar_index));

  return BmcVarsMgr_VarIndex2Timed(self, curvar_index, time, max_time);
}


/**Function********************************************************************

  Synopsis           [Returns the be variable allocates at given time, and 
  correpsonding to the given symbolic var]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_Name2Timed(const BmcVarsMgr_ptr self,
			     const node_ptr var_name, const int time, 
			     const int max_time)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self); 

  /* extends maxtime, if required */
  bmc_vars_mgr_extend_max_time(self, time);

  return BmcVarsMgr_Curr2Timed(self,
			       BmcVarsMgr_Name2Curr(self, var_name),
			       time, max_time);
}


/**Function********************************************************************

  Synopsis           [Given a variable which belongs to a timed block of
  variables, returns the corrisponding current variable in BE form]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_Timed2Curr(const BmcVarsMgr_ptr self, const be_ptr timedvar)
{
  int vars_block_size;
  int timedvar_index;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  timedvar_index = Be_Var2Index(BmcVarsMgr_GetBeMgr(self), timedvar);

  vars_block_size = BmcVarsMgr_GetStateInputVarsNum(self) + 
    BmcVarsMgr_GetStateVarsNum(self);

  return BmcVarsMgr_VarIndex2Curr(self, (timedvar_index -  vars_block_size) % 
			       BmcVarsMgr_GetStateInputVarsNum(self));
}


/**Function********************************************************************

  Synopsis           [Given a variable which belongs to a timed block of
  variables, returns the corrisponding symbolic variable]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr BmcVarsMgr_Timed2Name(const BmcVarsMgr_ptr self, 
			       const be_ptr timedvar)
{
  int vars_block_size;
  int timedvar_index;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  timedvar_index = Be_Var2Index(BmcVarsMgr_GetBeMgr(self), timedvar);

  vars_block_size = BmcVarsMgr_GetStateInputVarsNum(self) + 
    BmcVarsMgr_GetStateVarsNum(self);

  return BmcVarsMgr_VarIndex2Name(self, (timedvar_index -  vars_block_size) % 
			       BmcVarsMgr_GetStateInputVarsNum(self));
}


/**Function********************************************************************

  Synopsis           [Given a variable which belongs to the next block of
  variables, returns the corrisponding state (symbolic) variable]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr BmcVarsMgr_Next2Name(const BmcVarsMgr_ptr self, const be_ptr nextvar)
{
  int nextvar_index; 

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  nextvar_index = Be_Var2Index(BmcVarsMgr_GetBeMgr(self), nextvar);
  nusmv_assert( (nextvar_index >= BmcVarsMgr_GetStateInputVarsNum(self))
		&& (nextvar_index < (BmcVarsMgr_GetStateInputVarsNum(self) + 
				     BmcVarsMgr_GetStateVarsNum(self))) );
  
  return BmcVarsMgr_VarIndex2Name(self, nextvar_index - 
			       BmcVarsMgr_GetStateInputVarsNum(self));
}


/**Function********************************************************************

  Synopsis [Given a state variable which belongs to a timed block of
  variables, returns the corrisponding variable in BE form which
  belongs to the next block of variables]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_Timed2Next(const BmcVarsMgr_ptr self, const be_ptr timedvar)
{
  int vars_block_size;
  int timedvar_index;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  timedvar_index = Be_Var2Index(BmcVarsMgr_GetBeMgr(self), timedvar);

  vars_block_size = BmcVarsMgr_GetStateInputVarsNum(self) + 
    BmcVarsMgr_GetStateVarsNum(self);

  return BmcVarsMgr_VarIndex2Next(self, (timedvar_index - vars_block_size) % 
			       BmcVarsMgr_GetStateInputVarsNum(self));
}


/**Function********************************************************************

  Synopsis [Given a state variable which belongs to the next block of
  variables, returns the corrisponding current variable in BE form]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr BmcVarsMgr_Next2Curr(const BmcVarsMgr_ptr self, const be_ptr nextvar)
{
  int nextvar_index;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  nextvar_index = Be_Var2Index(BmcVarsMgr_GetBeMgr(self), nextvar);
  nusmv_assert( (nextvar_index >= BmcVarsMgr_GetStateInputVarsNum(self))
		&& (nextvar_index < (BmcVarsMgr_GetStateInputVarsNum(self) + 
				     BmcVarsMgr_GetStateVarsNum(self))) );

  return BmcVarsMgr_VarIndex2Curr(self, nextvar_index -  
				     BmcVarsMgr_GetStateInputVarsNum(self));
}


/**Function********************************************************************

  Synopsis [Returns the first variable into the be manager which
  belongs to the given block of variables indexed by 'time']

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_Time2BeIndex(const BmcVarsMgr_ptr self, const int time)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert((time >= 0) && (time <= BmcVarsMgr_GetMaxTime(self)));
  
  return ( BmcVarsMgr_GetStateVarsNum(self) * (time + 2) + 
	   BmcVarsMgr_GetInputVarsNum(self) * (time + 1) );
}


/**Function********************************************************************

  Synopsis           [Given a variable index returns the corresponding time]

  Description        [A current state/input variable index returns 0, 
  a next state variable and an input variable index return 1, 
  a timed variable returns its time (p0 returns 0, p1 returns 1,
  and so on)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_BeIndex2Time(const BmcVarsMgr_ptr self, const int be_idx)
{
  int time;
  

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  nusmv_assert( (be_idx >= 0) && 
		(be_idx <= bmc_vars_mgr_get_max_be_var_index(self, 
			      BmcVarsMgr_GetMaxTime(self))) );

  time = -1;
  if (BmcVarsMgr_IsBeIndexInCurrStateInputBlock(self, be_idx)) {
    time=0; /* current variable */
  }
  else {
    if (BmcVarsMgr_IsBeIndexInNextStateBlock(self, be_idx)) {
      time = 1; /* next variable */
    }
    else {
      /* timed variable */
      time = (be_idx - BmcVarsMgr_GetStateInputVarsNum(self) - 
	      BmcVarsMgr_GetStateVarsNum(self)) / 
	BmcVarsMgr_GetStateInputVarsNum(self);
    }
  }

  nusmv_assert( (time>=0) && (time <= BmcVarsMgr_GetMaxTime(self)) );

  return time;
}


/**Function********************************************************************

  Synopsis           [Given the index of a timed variable, returns the index
  of the corresponding state or input variable]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_BeIndex2VarIndex(const BmcVarsMgr_ptr self, const int be_idx)
{
  int vars_block_size;
  int res;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  nusmv_assert( (be_idx >= 0) && 
		(be_idx <= bmc_vars_mgr_get_max_be_var_index(self, 
				       BmcVarsMgr_GetMaxTime(self))) );

  vars_block_size = BmcVarsMgr_GetStateInputVarsNum(self) + 
    BmcVarsMgr_GetStateVarsNum(self);
  
  if (be_idx < vars_block_size) {
    /* in the vars block */
    res = be_idx % BmcVarsMgr_GetStateInputVarsNum(self);
  }
  else {
    /* in the timed block */
    res = (be_idx - vars_block_size) % BmcVarsMgr_GetStateInputVarsNum(self);
  }
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Given the index of a variable and the time,
  returns the index of the corresponding timed variable in the be package]

  Description        [Notice that last time has no input vars. An assertion 
  will be fired if last time and an input var index are given]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int BmcVarsMgr_VarIndex2BeIndex(const BmcVarsMgr_ptr self,
				const int var_idx, const int time, 
				const int max_time)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert(BmcVarsMgr_IsBeIndexInCurrStateInputBlock(self, var_idx));
  if (time == max_time) {
    nusmv_assert(BmcVarsMgr_IsBeIndexInCurrStateBlock(self, var_idx));
  }
  
  return (BmcVarsMgr_Time2BeIndex(self, time) + var_idx);
}




/**Function********************************************************************

  Synopsis [Checks whether given index corresponds to a state
  variable. Valid state variables are in current and next state blocks, and 
  in timed state area]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
BmcVarsMgr_IsBeIndexStateVar(const BmcVarsMgr_ptr self, const int index, 
			     const int max_time)
{
  boolean res;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  if (index > bmc_vars_mgr_get_max_be_var_index(self, max_time)) res = false;
  else {
    int var_index;
    
    /* this reduces also the case next state to current state */
    var_index = BmcVarsMgr_BeIndex2VarIndex(self, index);
    res = BmcVarsMgr_IsBeIndexInCurrStateBlock(self, var_index);
  }
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Checks whether given index corresponds to an input 
  variable. Valid input variables are in the input block, and 
  in timed input area]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
BmcVarsMgr_IsBeIndexInputVar(const BmcVarsMgr_ptr self, const int index, 
			     const int max_time)
{
  boolean res;

  BMC_VARS_MGR_CHECK_INSTANCE(self);

  if (index > bmc_vars_mgr_get_max_be_var_index(self, max_time)) res = false;
  else {
    res = BmcVarsMgr_IsBeIndexInInputBlock(self, 
				   BmcVarsMgr_BeIndex2VarIndex(self, index));
  }
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Checks whether given variable index points a variable 
  in the variables block (current, input and next vars)]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
BmcVarsMgr_IsBeIndexInVarsBlock(const BmcVarsMgr_ptr self, const int index)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert(index >= 0);

  return (index < (BmcVarsMgr_GetStateInputVarsNum(self) + 
		    BmcVarsMgr_GetStateVarsNum(self)));  
}


/**Function********************************************************************

  Synopsis           [Checks whether given variable index points a variable 
  in the current state variables block]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
BmcVarsMgr_IsBeIndexInCurrStateBlock(const BmcVarsMgr_ptr self, 
				     const int index)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert(index >= 0);
  
  return index < BmcVarsMgr_GetStateVarsNum(self);
}


/**Function********************************************************************

  Synopsis           [Checks whether given variable index points a variable 
  in the input variables block]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
BmcVarsMgr_IsBeIndexInInputBlock(const BmcVarsMgr_ptr self, const int index)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert(index >= 0);

  return ((index >= BmcVarsMgr_GetStateVarsNum(self)) &&
	  (index < BmcVarsMgr_GetStateInputVarsNum(self)));
}


/**Function********************************************************************

  Synopsis           [Checks whether given variable index points a variable 
  in the current state or input variables blocks]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
BmcVarsMgr_IsBeIndexInCurrStateInputBlock(const BmcVarsMgr_ptr self, 
					  const int index)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert(index >= 0);

  return (index < BmcVarsMgr_GetStateInputVarsNum(self));
}


/**Function********************************************************************

  Synopsis           [Checks whether given variable index points a variable 
  in the next state variables block]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
BmcVarsMgr_IsBeIndexInNextStateBlock(const BmcVarsMgr_ptr self, 
				     const int index)
{
  BMC_VARS_MGR_CHECK_INSTANCE(self);
  nusmv_assert(index >= 0);

  return ((index >= BmcVarsMgr_GetStateInputVarsNum(self)) &&
	  (index < (BmcVarsMgr_GetStateInputVarsNum(self) + 
		    BmcVarsMgr_GetStateVarsNum(self))));
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [<b>Extends</b> <tt>maxtime</tt> at given
  <tt>new_maxtime</tt>]

  Description        [Returns true if time has been extended, false otherwise]

  SideEffects        [be hash may change]

  SeeAlso            []

******************************************************************************/
static boolean bmc_vars_mgr_extend_max_time(BmcVarsMgr_ptr self, int new_maxtime)
{
  boolean res = false;

  nusmv_assert(new_maxtime >= 0);

  /* if is really necessary to do the maxtime extension... */
  if (new_maxtime > BmcVarsMgr_GetMaxTime(self)) {

    /* reserves space for (curvars, inputs, nextvars) + 
       one block of timed state vars + new_maxtime time (timed vars until
       new_maxtime) */

    Be_RbcManager_Reserve(BmcVarsMgr_GetBeMgr(self),
			  (new_maxtime + 3) * BmcVarsMgr_GetStateVarsNum(self) +
			  (new_maxtime + 1) * BmcVarsMgr_GetInputVarsNum(self));

    /* keeps maxtime change */
    set_maxtime(self, new_maxtime);
    res = true;
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [<b>Sets</b> the
  <tt>maximum time currently handled by the variable environment</tt>]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static void set_maxtime(BmcVarsMgr_ptr self, int iNewMaxTime)
{
  self->maxtime = iNewMaxTime;
}


/**Function********************************************************************

   Synopsis           [Builds all internal structures used in order to
   perform searches and conversion from different variable representation
   forms]

   Description        []

   SideEffects        [...]

   SeeAlso            [bmc_vars_init]

******************************************************************************/
static void bmc_vars_mgr_init_tables(BmcVarsMgr_ptr self)
{
  int list_index; 
  int var_index; 
  
  NodeList_ptr var_lists[] = { Encoding_get_bool_state_vars_list(self->senc), 
			       Encoding_get_bool_input_vars_list(self->senc) };
  
  var_index = 0;
  for (list_index = 0; 
       list_index < sizeof(var_lists) / sizeof(var_lists[0]); ++list_index) {
    ListIter_ptr iter;
    iter = NodeList_get_first_iter(var_lists[list_index]);
    while (! ListIter_is_end(iter)) {
      node_ptr var_name;
      
      var_name = NodeList_get_elem_at(var_lists[list_index], iter);
      /* update statevar->curvar table for associate ith state variable with
	 ith current variable, that are in the variables environment */
      insert_assoc( self->varname2currvar_table, var_name,
		    (node_ptr) Be_Index2Var(BmcVarsMgr_GetBeMgr(self), 
					    var_index) );
      
      /* update curvar->statevar table for associate ith current variable,
	 that are in the variables environment, with ith state variable */
      self->varindex2varname_table[var_index] = var_name;
      
      if (opt_verbose_level_gt(options, 1)) {
	if (Encoding_is_symbol_state_var(self->senc, var_name)) {
	  fprintf(nusmv_stderr, "[%3d] Boolean state variable ", var_index);
	}
	else {
	  nusmv_assert(Encoding_is_symbol_input_var(self->senc, var_name));
	  fprintf(nusmv_stderr, "[%3d] Boolean input variable ", var_index);
	}
	print_node(nusmv_stderr, var_name);
	fprintf(nusmv_stderr, "\n");
      }

      var_index += 1;
      iter = ListIter_get_next(iter);
    }
  } /* for each list loop */
  
}


/**Function********************************************************************

   Synopsis           [Returns the max absolute var index that can be used 
   in a certain moment]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int bmc_vars_mgr_get_max_be_var_index(const BmcVarsMgr_ptr self, 
					     const int max_time)
{
  return (BmcVarsMgr_GetStateVarsNum(self) * (max_time + 3) +
	  BmcVarsMgr_GetInputVarsNum(self) * (max_time + 1)) - 
    1;
}

/**AutomaticEnd***************************************************************/
