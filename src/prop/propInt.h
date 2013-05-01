/**CHeaderFile*****************************************************************

  FileName    [propInt.h]

  PackageName [prop]

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

  Author      [Marco Roveri, Roberto Cavada]

  Copyright   [
  This file is part of the ``prop'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by ITC-irst. 

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

  For more information on NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

  Revision    [$Id: propInt.h,v 1.5.6.7.2.1 2005/06/27 14:46:39 nusmv Exp $]

******************************************************************************/

#ifndef __PROP_INT_H__
#define __PROP_INT_H__

#include "prop.h"

#include "utils/utils.h"
#include "utils/array.h"
#include "node/node.h"
#include "opt/opt.h"

#include "fsm/sexp/Expr.h"
#include "fsm/sexp/SexpFsm.h"
#include "fsm/bdd/BddFsm.h"

#include "bmc/bmcVarsMgr.h"

#include "compile/compileFsmMgr.h"
#include "compile/compile.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Property Information Record]

  Description [This structure contains informations about a given
  property:<br>
  <dl>
  <dt><code>index</code>
     <dd>is the progressive number identifying the specification.</dd>
  <dt><code>prop</code>
      <dd>is the specification formula (s-expression).
  <dt><code>type</code>
      <dd>is the type of the specification (CTL, LTL, INVAR, COMPUTE). 
  <dt><code>cone</code>
      <dd>is the cone of influence of the formula.
  <dt><code>status</code>
      <dd>is the actual checking status of the specification.
  <dt><code>number</code>
      <dd>Result of a COMPUTE property. 
  <dt><code>trace</code>
      <dd>is the index of the counterexample produced when the
          formula is found to be false, otherwise is zero.
  <dt><code>scalar_fsm</code>
      <dd>The FSM associated to the property in scalar format. 
  <dt><code>bool_fsm</code>
      <dd>The FSM associated to the property in boolean format. 
  <dt><code>bdd_fsm</code>
      <dd>The FSM associated to the property in BDD format.
  <dt><code>be_fsm</code>
      <dd>The FSM associated to the property in BE format.
  </dl>
  ]

  SeeAlso     []

******************************************************************************/
typedef struct Prop_TAG 
{
  unsigned int index;  /* Progressive number */
  Expr_ptr     prop;   /* property formula (s-expression) */

  Expr_ptr     psl2smv;   /* used for lazily converting psl props */

  node_ptr     cone;   /* The cone of influence */
  Prop_Type    type;   /* type of specification */
  Prop_Status  status; /* verification status */
  int          number; /* The result of a quantitative spec */
  int          trace;  /* the counterexample number (if any) */

  FsmBuilder_ptr fsm_mgr;  /* Used to produce FSMs from cone */
  SexpFsm_ptr  scalar_fsm; /* the scalar FSM number */
  SexpFsm_ptr  bool_fsm;   /* The scalar FSM converted in Boolean */
  BddFsm_ptr   bdd_fsm;    /* The BDD FSM */
  Bmc_Fsm_ptr  be_fsm;     /* The BE FSM */

} Prop;



/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE* nusmv_stderr;
extern FILE* nusmv_stdout;
extern options_ptr options;
extern node_ptr all_variables;
extern cmp_struct_ptr cmps;

extern BmcVarsMgr_ptr gl_vars_mgr;
extern FsmBuilder_ptr global_fsm_builder;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int Prop_get_index ARGS((const Prop_ptr self));
EXTERN void Prop_set_index ARGS((Prop_ptr self, const int index));


/**AutomaticEnd***************************************************************/

#endif /* __PROP_INT_H__ */
