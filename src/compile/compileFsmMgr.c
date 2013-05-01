/**CFile***********************************************************************

  FileName    [compileFsmMgr.c]

  PackageName [compile]

  Synopsis    [High level object that can contruct FSMs]

  Description [Defines an high-level object that lives at 
  top-level, that is used to help contruction of FSMs. 
  It can control information that are not shared between 
  lower levels, so it can handle with objects that have not 
  the full knowledge of the whole system]
  
  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and ITC-irst. 

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


#include "compileFsmMgr.h"
#include "compileInt.h"

#include "fsm/bdd/FairnessList.h"
#include "parser/symbols.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Type************************************************************************

  Synopsis    [FSM builder class constructor]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct FsmBuilder_TAG 
{
  DdManager*  dd; /* for performance, readability, etc. */
  
} FsmBuilder;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

FsmBuilder_ptr global_fsm_builder = FSM_BUILDER(NULL);


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void fsm_builder_init ARGS((FsmBuilder_ptr self, DdManager* dd)); 

static void fsm_builder_deinit ARGS((FsmBuilder_ptr self));

static ClusterList_ptr 
fsm_builder_clusterize_expr ARGS((FsmBuilder_ptr self, 
				  BddEnc_ptr enc, Expr_ptr expr));

static void 
fsm_builder_clusterize_expr_aux ARGS((const FsmBuilder_ptr self, 
				      BddEnc_ptr enc, 
				      ClusterList_ptr clusters, 
				      Expr_ptr expr_trans, 
				      boolean is_inside_and));

static JusticeList_ptr
fsm_builder_justice_sexp_to_bdd ARGS((FsmBuilder_ptr self, 
				      BddEnc_ptr enc, 
				      node_ptr justice_sexp_list));

static CompassionList_ptr 
fsm_builder_compassion_sexp_to_bdd ARGS((FsmBuilder_ptr self, 
					 BddEnc_ptr enc, 
					 node_ptr compassion_sexp_list));



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The constructor creates a BddEnc and self handles it]

  Description        []

  SideEffects        []

******************************************************************************/
FsmBuilder_ptr FsmBuilder_create(DdManager* dd)
{
  FsmBuilder_ptr self = ALLOC(FsmBuilder, 1);

  FSM_BUILDER_CHECK_INSTANCE(self);

  fsm_builder_init(self, dd);
  return self;
}


/**Function********************************************************************

  Synopsis           [Class FsmBuilder destructor]

  Description        []

  SideEffects        []

******************************************************************************/
void FsmBuilder_destroy(FsmBuilder_ptr self)
{
  FSM_BUILDER_CHECK_INSTANCE(self);
  
  fsm_builder_deinit(self);

  FREE(self);
}



/**Function********************************************************************

  Synopsis           [Creates a new sexp fsm]

  Description        [The caller becomes the owner of the returned object]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr FsmBuilder_create_sexp_fsm(const FsmBuilder_ptr self, 
				       const Encoding_ptr senc, 
				       VarSet_ptr vars_list, 
				       const enum SexpFsmType type)
{
  SexpFsm_ptr res; 

  FSM_BUILDER_CHECK_INSTANCE(self);
  
  res = SexpFsm_create(vars_list, 
		       Compile_FlattenSexp(senc, cmp_struct_get_justice(cmps), 
					   Nil), 
		       Compile_FlattenSexp(senc, 
					   cmp_struct_get_compassion(cmps),
					   Nil), 
		       type);   
  
  return res;
}


/**Function********************************************************************

  Synopsis           [Creates a BddFsm instance from a given SexpFsm]

  Description        []

  SideEffects        []

******************************************************************************/
BddFsm_ptr FsmBuilder_create_bdd_fsm(const FsmBuilder_ptr self, 
				     BddEnc_ptr enc, 
				     const SexpFsm_ptr sexp_fsm, 
				     const TransType trans_type)
{
  /* to construct Bdd Fsm: */
  BddFsm_ptr bddfsm;
  BddTrans_ptr trans;
  JusticeList_ptr justice;
  CompassionList_ptr compassion;
  bdd_ptr init_bdd, invar_bdd, input_bdd;

  /* to construct trans: */
  ClusterList_ptr clusters;
  ClusterOptions_ptr cluster_options;
  BddVarSet_ptr state_vars_cube, input_vars_cube, next_state_vars_cube;


  FSM_BUILDER_CHECK_INSTANCE(self);


  /* ---------------------------------------------------------------------- */
  /* Trans construction                                                     */
  /* ---------------------------------------------------------------------- */
  

  cluster_options = ClusterOptions_create(options);
  
  state_vars_cube = BddEnc_get_state_vars_support(enc);
  input_vars_cube = BddEnc_get_input_vars_support(enc);
  next_state_vars_cube = BddEnc_get_next_state_vars_support(enc);
   
  clusters = fsm_builder_clusterize_expr(self, enc, 
					 SexpFsm_get_trans(sexp_fsm));
  
  trans = BddTrans_create(self->dd, 
			  clusters, 
			  (bdd_ptr) state_vars_cube, 
			  (bdd_ptr) input_vars_cube, 
			  (bdd_ptr) next_state_vars_cube, 
			  trans_type, 
			  cluster_options);

  bdd_free(self->dd, (bdd_ptr) state_vars_cube);
  bdd_free(self->dd, (bdd_ptr) input_vars_cube);
  bdd_free(self->dd, (bdd_ptr) next_state_vars_cube);

  ClusterOptions_destroy(cluster_options); /* this is no longer needed */ 


  /* ---------------------------------------------------------------------- */
  /* Bdd Fsm construction                                                   */
  /* ---------------------------------------------------------------------- */
  justice = fsm_builder_justice_sexp_to_bdd(self, enc, 
					    SexpFsm_get_justice(sexp_fsm));

  compassion = fsm_builder_compassion_sexp_to_bdd(self, enc, 
					SexpFsm_get_compassion(sexp_fsm));
  
  /* init */
  init_bdd = BddEnc_expr_to_bdd(enc, SexpFsm_get_init(sexp_fsm), Nil);
  
  /* invar */
  invar_bdd = BddEnc_expr_to_bdd(enc, SexpFsm_get_invar(sexp_fsm), Nil);
  
  /* input */
  input_bdd = BddEnc_expr_to_bdd(enc, SexpFsm_get_input(sexp_fsm), Nil);
  
  bddfsm = BddFsm_create(enc, 
			 BDD_STATES(init_bdd), 
			 BDD_INVAR_STATES(invar_bdd), 
			 BDD_INVAR_INPUTS(input_bdd), 
			 trans, 
			 justice, compassion);

  bdd_free(self->dd, input_bdd);
  bdd_free(self->dd, invar_bdd);
  bdd_free(self->dd, init_bdd);

  return bddfsm;
}



/*---------------------------------------------------------------------------*/
/* Static function definitions                                               */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
static void fsm_builder_init(FsmBuilder_ptr self, DdManager* dd)
{
  self->dd = dd;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
static void fsm_builder_deinit(FsmBuilder_ptr self)
{

}


/**Function********************************************************************

  Synopsis           [Converts an expression into a list of clusters. 
  This list can be used to create a BddFsm]

  Description        [Each cluster into the list represents a piece of 
  transition relation. One important requirement is that the given expr should 
  not contain duplicates. See for example SexpFsm_get_{init, invar, trans} on 
  how to obtain a well formed expression]

  SideEffects        []

******************************************************************************/
static ClusterList_ptr fsm_builder_clusterize_expr(FsmBuilder_ptr self, 
						   BddEnc_ptr enc, 
						   Expr_ptr expr)
{
  ClusterList_ptr clusters;
    
  clusters = ClusterList_create(self->dd); 
  fsm_builder_clusterize_expr_aux(self, enc, clusters, expr, false);

  return clusters;
}



/**Function********************************************************************

  Synopsis           [Auxiliary function to recursively traverse the 
  given expression, clusterizing each node as bdd. If called from outside, 
  parameter is_inside_and is false.]

  Description        []

  SideEffects        [given cluster list will change]

******************************************************************************/
static void
fsm_builder_clusterize_expr_aux(const FsmBuilder_ptr self, 
				BddEnc_ptr enc,
				ClusterList_ptr clusters, 
				Expr_ptr expr_trans, 
				boolean is_inside_and)
{
  add_ptr tmp; 
  node_ptr node = (node_ptr) expr_trans;

  if (node != Nil) {
    yylineno = node_get_lineno(node);

    switch (node_get_type(node)) {
    case AND:
      fsm_builder_clusterize_expr_aux(self, enc, clusters, car(node), true);
      fsm_builder_clusterize_expr_aux(self, enc, clusters, cdr(node), true);

      if (!is_inside_and && (ClusterList_length(clusters) == 0)) {
	/* Due to lazy evaluation, the list is going to be empty (and
	   the call is over). Adds a single true cluster */
	bdd_ptr one = bdd_one(self->dd);
	Cluster_ptr cluster = Cluster_create(self->dd);
	Cluster_set_trans(cluster, self->dd, one);
	ClusterList_append_cluster(clusters, cluster);
      }

      break;
      
    default:
      tmp = BddEnc_expr_to_add(enc, expr_trans, Nil); 

      if (! (add_is_one(self->dd, tmp) & is_inside_and)) {
	Cluster_ptr cluster = Cluster_create(self->dd);
	bdd_ptr tmp2 = add_to_bdd(self->dd, tmp);
	Cluster_set_trans(cluster, self->dd, tmp2);
	bdd_free(self->dd, tmp2);

	ClusterList_append_cluster(clusters, cluster);
      }

      add_free(self->dd, tmp);

    } /* switch */
  }
}



/**Function********************************************************************

  Synopsis           [Converts a list of expressions into a list of 
  bdds, wrapped inside a justice list object]

  Description        [The caller becomes the wner of the returned object. 
  Internally used by the bdd fsm building code]

  SideEffects        []

******************************************************************************/
static JusticeList_ptr
fsm_builder_justice_sexp_to_bdd(FsmBuilder_ptr self, 
				BddEnc_ptr enc, 
				node_ptr justice_sexp_list)
{
  JusticeList_ptr res;
  node_ptr iter; 

  res = JusticeList_create(self->dd);

  iter = justice_sexp_list;
  while (iter != Nil) {
    Expr_ptr expr = EXPR( car(iter) );
    bdd_ptr  p = BddEnc_expr_to_bdd(enc, expr, Nil);
    JusticeList_append_p(res, BDD_STATES(p));

    bdd_free(self->dd, p);
    iter = cdr(iter);
  } /* loop */
  
  return res;
}


/**Function********************************************************************

  Synopsis [Converts a list of couple of expressions into a list of couple of 
  bdds, wrapped inside a compassion list object]

  Description        [The caller becomes the wner of the returned object. 
  Internally used by the bdd fsm building code]

  SideEffects        []

******************************************************************************/
static CompassionList_ptr 
fsm_builder_compassion_sexp_to_bdd(FsmBuilder_ptr self, 
				   BddEnc_ptr enc,
				   node_ptr compassion_sexp_list)
{
  CompassionList_ptr res;
  node_ptr iter; 

  res = CompassionList_create(self->dd);

  iter = compassion_sexp_list;
  while (iter != Nil) {
    Expr_ptr expr;
    bdd_ptr  p, q;
    node_ptr couple = car(iter);

    expr = car(couple);
    p = BddEnc_expr_to_bdd(enc, expr, Nil);
    expr = cdr(couple);
    q = BddEnc_expr_to_bdd(enc, expr, Nil);

    CompassionList_append_p_q(res, BDD_STATES(p), BDD_STATES(q));

    bdd_free(self->dd, q);
    bdd_free(self->dd, p);

    iter = cdr(iter);
  } /* loop */
  
  return res;
}

