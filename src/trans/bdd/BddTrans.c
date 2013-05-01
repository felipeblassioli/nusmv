/**CFile***********************************************************************

  FileName    [BddTrans.c]

  PackageName [trans.bdd]

  Synopsis    [Routines related to Transition Relation in Bdd form]

  Description [This file contains the definition of the \"BddTrans\" class
  definition]

  
  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``trans.bdd'' package of NuSMV version 2. 
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

#include "BddTrans.h"
#include "transInt.h"
#include "generic/GenericTrans.h"
#include "generic/GenericTrans_private.h"

#include "ClusterList.h"
#include "Cluster.h"

static char rcsid[] UTIL_UNUSED = "$Id: BddTrans.c,v 1.1.2.5 2004/05/31 09:59:59 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Transition Relation Class "BddTrans"]

  Description [ This class contains informations about a transition
  relation in bdd-based form. It derives from the GenericTrace class<br>
  	<dl> 
            <dt><code>forward_trans</code>
                <dd> The list of clusters representing the transition relation 
		used when a forward image is performed	
            <dt><code>backward_trans</code>
                <dd> The list of clusters representing the transition
		relation used when a backwad image is performed
	</dl>]

  SeeAlso     []   
  
******************************************************************************/
typedef struct BddTrans_TAG
{
  INHERITS_FROM(GenericTrans);


  /* The list of clusters representing the transition
     relation used when a forward image is performed */
  ClusterList_ptr forward_trans;  

  /* The list of clusters representing the transition
     relation used when a backwad image is performed */  
  ClusterList_ptr backward_trans; 

} BddTrans; 



/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void bdd_trans_finalize ARGS((Object_ptr object, void* dummy));

static void bdd_trans_deinit ARGS((BddTrans_ptr self));

static void bdd_trans_init ARGS((BddTrans_ptr self, 
				 DdManager* dd_manager, 
				 const ClusterList_ptr clusters_bdd, 
				 bdd_ptr state_vars_cube, 
				 bdd_ptr input_vars_cube, 
				 bdd_ptr next_state_vars_cube, 
				 const TransType trans_type, 
				 const ClusterOptions_ptr cl_options));

static Object_ptr bdd_trans_copy ARGS((const Object_ptr object));

static void bdd_trans_copy_aux ARGS((const BddTrans_ptr self, 
				     BddTrans_ptr copy));


#ifdef TRANS_DEBUG_THRESHOLD  /* this is used only for debugging purposes */

static boolean 
bdd_trans_debug_partitioned ARGS((const BddTrans_ptr self, 
				  const ClusterList_ptr basic_clusters, 
				  FILE* file));
#endif 


/**Function********************************************************************

  Synopsis           [Builds the transition relation]

  Description        [None of given arguments will become owned by self. 
  You should destroy cl_options by yourself.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
BddTrans_ptr BddTrans_create(DdManager* dd_manager, 
			     const ClusterList_ptr clusters_bdd, 
			     bdd_ptr state_vars_cube, 
			     bdd_ptr input_vars_cube, 
			     bdd_ptr next_state_vars_cube, 
			     const TransType trans_type, 
			     const ClusterOptions_ptr cl_options)
{
  BddTrans_ptr self = ALLOC(BddTrans, 1);
  
  BDD_TRANS_CHECK_INSTANCE(self);

  bdd_trans_init(self, dd_manager, 
		 clusters_bdd, 
		 state_vars_cube, input_vars_cube, next_state_vars_cube, 
		 trans_type, cl_options);

  return self;
}




/**Function********************************************************************

  Synopsis    [Performs the synchronous product between two trans]

  Description [The result goes into self and contained forward and backward
  cluster lists would be rescheduled. Other will remain unchanged. ]

  SideEffects [self will change]

******************************************************************************/
void BddTrans_apply_synchronous_product(BddTrans_ptr self, 
					const BddTrans_ptr other, 
					bdd_ptr state_vars_cube,
					bdd_ptr input_vars_cube, 
					bdd_ptr next_state_vars_cube)
{
  BDD_TRANS_CHECK_INSTANCE(self);
  
  ClusterList_apply_synchronous_product(self->forward_trans, 
					other->forward_trans); 
  ClusterList_build_schedule(self->forward_trans, 
			     state_vars_cube, input_vars_cube);
    
    
  ClusterList_apply_synchronous_product(self->backward_trans, 
					other->backward_trans); 
  ClusterList_build_schedule(self->backward_trans, 
			     next_state_vars_cube, input_vars_cube);
}


/**Function********************************************************************

  Synopsis           [Returns backward transition relation.]

  Description        [self keeps the ownership of the returned instance.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ClusterList_ptr BddTrans_get_backward(const BddTrans_ptr self)
{
  BDD_TRANS_CHECK_INSTANCE(self);
  return self->backward_trans;
}


/**Function********************************************************************

  Synopsis           [Returns forward transition relation.]

  Description        [self keeps the ownership of the returned instance.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ClusterList_ptr BddTrans_get_forward(const BddTrans_ptr self)
{
  BDD_TRANS_CHECK_INSTANCE(self);
  return self->forward_trans;
}



/**Function********************************************************************

  Synopsis           [Computes the forward image by existentially quantifying
  over state variables only.]

  Description        [Returned bdd is referenced]

  SideEffects        [self keeps the ownership of the returned instance.]

  SeeAlso            []

******************************************************************************/
bdd_ptr BddTrans_get_forward_image_state(const BddTrans_ptr self, bdd_ptr s)
{
  BDD_TRANS_CHECK_INSTANCE(self);  
  return ClusterList_get_image_state(self->forward_trans, s);
}


/**Function********************************************************************

  Synopsis           [Computes the forward image by existentially quantifying
  over both state and input variables.]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddTrans_get_forward_image_state_input(const BddTrans_ptr self, bdd_ptr s)
{
  BDD_TRANS_CHECK_INSTANCE(self);
  return ClusterList_get_image_state_input(self->forward_trans, s);
}


/**Function********************************************************************

  Synopsis           [Computes the backward image by existentially quantifying
  over state variables only.]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddTrans_get_backward_image_state(const BddTrans_ptr self, bdd_ptr s)
{
  BDD_TRANS_CHECK_INSTANCE(self);
  return ClusterList_get_image_state(self->backward_trans, s);
}


/**Function********************************************************************

  Synopsis           [Computes the backward image by existentially quantifying
  over both state and input variables.]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddTrans_get_backward_image_state_input(const BddTrans_ptr self, 
						bdd_ptr s)
{
  BDD_TRANS_CHECK_INSTANCE(self);
  return ClusterList_get_image_state_input(self->backward_trans, s);
}

/**Function********************************************************************

  Synopsis           [Computes the k forward image by existentially quantifying
  over state variables only.]

  Description        [Returned bdd is referenced]

  SideEffects        [self keeps the ownership of the returned instance.]

  SeeAlso            []

******************************************************************************/
bdd_ptr BddTrans_get_k_forward_image_state(const BddTrans_ptr self, bdd_ptr s, int k)
{
  BDD_TRANS_CHECK_INSTANCE(self);  
  return ClusterList_get_k_image_state(self->forward_trans, s, k);
}


/**Function********************************************************************

  Synopsis           [Computes the k forward image by existentially quantifying
  over both state and input variables.]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddTrans_get_k_forward_image_state_input(const BddTrans_ptr self, bdd_ptr s, int k)
{
  BDD_TRANS_CHECK_INSTANCE(self);
  return ClusterList_get_k_image_state_input(self->forward_trans, s, k);
}


/**Function********************************************************************

  Synopsis           [Computes the k backward image by existentially quantifying
  over state variables only.]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddTrans_get_k_backward_image_state(const BddTrans_ptr self, bdd_ptr s, int k)
{
  BDD_TRANS_CHECK_INSTANCE(self);
  return ClusterList_get_k_image_state(self->backward_trans, s, k);
}


/**Function********************************************************************

  Synopsis           [Computes the k backward image by existentially 
  quantifying over both state and input variables.]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr BddTrans_get_k_backward_image_state_input(const BddTrans_ptr self, 
						  bdd_ptr s,
						  int k)
{
  BDD_TRANS_CHECK_INSTANCE(self);
  return ClusterList_get_k_image_state_input(self->backward_trans, s, k);
}



/**Function********************************************************************

  Synopsis           [Prints short info associated to a Trans]

  Description        [Prints info about the size of each cluster in
  forward/backward transition relations]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void BddTrans_print_short_info(const BddTrans_ptr self, FILE* file)
{
  BDD_TRANS_CHECK_INSTANCE(self);

  fprintf(file, "Forward Partitioning Schedule BDD cluster size (#nodes):\n");
  ClusterList_print_short_info(BddTrans_get_forward(self), file);

  fprintf(file, "Backward Partitioning Schedule BDD cluster size (#nodes):\n");
  ClusterList_print_short_info(BddTrans_get_backward(self), file);
}



/*---------------------------------------------------------------------------*/
/* Static functions definitions                                              */
/*---------------------------------------------------------------------------*/


static void bdd_trans_finalize(Object_ptr object, void* dummy)
{
  BddTrans_ptr self = BDD_TRANS(object);

  bdd_trans_deinit(self);
  FREE(self);
}


static void bdd_trans_deinit(BddTrans_ptr self)
{
  generic_trans_deinit(GENERIC_TRANS(self));
  
  ClusterList_destroy(self->forward_trans);
  ClusterList_destroy(self->backward_trans);
}


static void bdd_trans_init(BddTrans_ptr self, 
			   DdManager* dd_manager, 
			   const ClusterList_ptr clusters_bdd, 
			   bdd_ptr state_vars_cube, 
			   bdd_ptr input_vars_cube, 
			   bdd_ptr next_state_vars_cube, 
			   const TransType trans_type, 
			   const ClusterOptions_ptr cl_options)
{
  generic_trans_init(GENERIC_TRANS(self), trans_type);

  OVERRIDE(Object, finalize)   = bdd_trans_finalize;
  OVERRIDE(Object, copy)       = bdd_trans_copy;
		     
  switch (trans_type) {

  case TRANS_TYPE_MONOLITHIC:
    self->forward_trans  = ClusterList_apply_monolithic(clusters_bdd); 
    self->backward_trans = ClusterList_copy(self->forward_trans); 
    break;

  case TRANS_TYPE_THRESHOLD:
    self->forward_trans = ClusterList_apply_threshold(clusters_bdd, 
						      cl_options);
    self->backward_trans = ClusterList_copy(self->forward_trans);
    break;

  case TRANS_TYPE_IWLS95:

    self->forward_trans = 
      ClusterList_apply_iwls95_partition(clusters_bdd, 
					 state_vars_cube, 
					 input_vars_cube, 
					 next_state_vars_cube, 
					 cl_options);
  
    self->backward_trans =  
      ClusterList_apply_iwls95_partition(clusters_bdd, 
					 next_state_vars_cube, 
					 input_vars_cube, 
					 state_vars_cube, 
					 cl_options);
    break;

  default:
    nusmv_assert(false); /* no other types available */
    
  } /* switch on type */

  ClusterList_build_schedule(self->forward_trans, 
			     state_vars_cube, input_vars_cube); 
  
  ClusterList_build_schedule(self->backward_trans, 
			     next_state_vars_cube, input_vars_cube); 

# ifdef TRANS_DEBUG_THRESHOLD  /* self checking of partitioned trans */
  if (trans_type != TRANS_TYPE_MONOLITHIC) {
    trans_debug_partitioned(self, clusters_bdd, nusmv_stderr);
  }
# endif

}


/**Function********************************************************************

  Synopsis           [Copy constructor]

  Description        [Return a copy of the self.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Object_ptr bdd_trans_copy(const Object_ptr object)
{
  BddTrans_ptr self;
  BddTrans_ptr copy;

  self = BDD_TRANS(object);

  BDD_TRANS_CHECK_INSTANCE(self);
  
  /* copy is the owner of forward and backward: we do not destroy them */
  copy = ALLOC(BddTrans, 1);
  BDD_TRANS_CHECK_INSTANCE(copy);

  bdd_trans_copy_aux(self, copy);

  return OBJECT(copy);
}


static void bdd_trans_copy_aux(const BddTrans_ptr self, BddTrans_ptr copy)
{
  generic_trans_copy_aux(GENERIC_TRANS(self), GENERIC_TRANS(copy));

  copy->forward_trans = ClusterList_copy(BddTrans_get_forward(self));
  copy->backward_trans= ClusterList_copy(BddTrans_get_backward(self)); 
}



#ifdef TRANS_DEBUG_THRESHOLD

/**Function********************************************************************

  Synopsis           [ Checks the equality between given Monolithic and 
  Partitioned transition relations.]

  Description        [ It checks the equality in terms of transition relation
  and quantification schedule. ]
 

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean 
bdd_trans_debug_partitioned(const BddTrans_ptr self, 
			    const ClusterList_ptr basic_clusters, 
			    FILE* file)
{
  boolean res = true; 
  boolean tmp; 

  fprintf(file, "\nChecking partitioned transition:\n");

  fprintf(file, " Checking equality Partitioned vs. Monolithic:\n");  
  tmp = ClusterList_check_equality(BddTrans_get_backward(self), 
				   basic_clusters); 
  fprintf(file, "\t  Backward : %d\n", tmp);
  res = res && tmp;

  tmp = ClusterList_check_equality(BddTrans_get_forward(self), 
				   basic_clusters); 
  fprintf(file, "\t  Forward : %d\n", tmp);
  res = res && tmp;
  
  fprintf(nusmv_stderr," Checking Quantification Schedule:\n");  
  tmp = ClusterList_check_schedule(BddTrans_get_backward(self));
				  
  fprintf(file, "\t  Backward : %d\n", tmp);
  res = res && tmp;

  tmp = ClusterList_check_schedule(BddTrans_get_forward(self));
				   
  fprintf(file, "\t  Forward : %d\n", tmp);
  res = res && tmp;

  return res;  
}

#endif /* TRANS_DEBUG_THRESHOLD is defined */

