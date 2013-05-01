/**CFile***********************************************************************

  FileName    [propProp.c]

  PackageName [prop]

  Synopsis    [Main routines for the prop data structure]

  Description [Main routines for the manipulation of the prop data
  structure. A "master" property is also defined to be used to
  represent the whole system, for instance to perform reachability or
  to perform simulation. Moreover a primitives to create, query and
  manipulate a database of property is provided.]

  SeeAlso     []

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

******************************************************************************/

#include "prop/prop.h"
#include "propInt.h" 
#include "mc/mc.h"
#include "ltl/ltl.h"
#include "utils/error.h"
#include "parser/symbols.h"
#include "parser/psl/pslNode.h"
#include "utils/utils_io.h"
#include "enc/enc.h"

#include <string.h>

static char rcsid[] UTIL_UNUSED = "$Id: propProp.c,v 1.27.2.14.2.5 2005/11/16 11:44:40 nusmv Exp $";

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
static Prop_ptr Prop_Master = PROP(NULL);

EXTERN DdManager* dd_manager;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* Used to encode that a property MIN/MAX has not yet been checked. */
#define PROP_UNCHECKED -2

/* Used to encode the infinite distanca between two set of states in
   MIN/MAX properties */
#define PROP_INFINITE -1

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void prop_init ARGS((Prop_ptr self));
static void prop_deinit ARGS((Prop_ptr self));
static void prop_print ARGS((const Prop_ptr self, FILE* file));

static void prop_set_scalar_sexp_fsm 
ARGS((Prop_ptr self, SexpFsm_ptr fsm, const boolean duplicate));

static void prop_set_bool_sexp_fsm 
ARGS((Prop_ptr self, SexpFsm_ptr fsm, const boolean duplicate));

static void prop_set_bdd_fsm 
ARGS((Prop_ptr self, BddFsm_ptr fsm, const boolean duplicate));

static void prop_set_be_fsm 
ARGS((Prop_ptr self, Bmc_Fsm_ptr fsm, const boolean duplicate));


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Initializes the package: master property and 
  property database are allocated]

  Description        [After you had called this, you must also call 
  PropPkg_init_cmd if you need to use the interactive shell for 
  commands]

  SideEffects        []

******************************************************************************/
void PropPkg_init()
{
  nusmv_assert(Prop_Master == PROP(NULL));
  Prop_Master = Prop_create();
  PropDb_create();
}

/**Function********************************************************************

  Synopsis           [Quits the package]

  Description        []

  SideEffects        []

******************************************************************************/
void PropPkg_quit()
{
  PROP_CHECK_INSTANCE(Prop_Master);

  PropDb_destroy();

  Prop_destroy(Prop_Master);
  Prop_Master = PROP(NULL);
}





/**Function********************************************************************

  Synopsis           [Allocate a property]

  Description        [Allocate a property. If no more room is
  available then a call to <tt>numsv_exit</tt> is performed. All the
  fields of the prop structure are initialized to either NULL or the
  corresponding default type (e.g. Prop_NoType for property type).]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Prop_ptr Prop_create() 
{
  Prop_ptr self = ALLOC(Prop, 1);

  PROP_CHECK_INSTANCE(self);

  prop_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [Free a property]

  Description        [Free a property. Notice that before freeing the
  property all the elements of the property that needs to be freed
  will be automatically freed.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Prop_destroy(Prop_ptr self) 
{
  PROP_CHECK_INSTANCE(self);

  prop_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Returns the property]

  Description        [Returns the property stored in the prop. If the property 
  is PSL, then it is converted to smv before being returned.]

  SideEffects        []

******************************************************************************/
Expr_ptr Prop_get_expr(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  
  if (Prop_get_type(self) == Prop_Psl) {
    if (self->psl2smv == EXPR(NULL)) { /* lazy evalution */
      PslNode_ptr psl_prop = PslNode_convert_from_node_ptr(self->prop);

      /* removal of forall */
      psl_prop = PslNode_remove_forall_replicators(psl_prop);

      if (PslNode_is_handled_psl(psl_prop)) {
	/* either smooth LTL or SERE: */
	if (!PslNode_is_ltl(psl_prop)) {
	  /* here it is a SERE: must be converted to LTL */
	  psl_prop = PslNode_remove_sere(psl_prop);
	}

	/* converts to SMV ltl */
	self->psl2smv = PslNode_pslltl2ltl(psl_prop, PSL2SMV);
      }
      else {
	/* here the property may be either OBE or unmanageable */
	if (PslNode_is_obe(psl_prop)) {
	  self->psl2smv = PslNode_pslobe2ctl(psl_prop, PSL2SMV);
	}
	else error_psl_not_supported_feature();
      }
    }
      
    return self->psl2smv;
  }

  return self->prop;
}


/**Function********************************************************************

  Synopsis           [Returns the cone of a property]

  Description        [If the cone of influence of a property has been
  computed, this function returns it.]

  SideEffects        []

******************************************************************************/
Set_t Prop_get_cone(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);

  return self->cone;
}


/**Function********************************************************************

  Synopsis           [Sets the cone of a property]

  Description        [Stores the cone of influence of the property]

  SideEffects        []

******************************************************************************/
void Prop_set_cone(Prop_ptr self, Set_t cone)
{
  PROP_CHECK_INSTANCE(self);

  self->cone = cone;
}


/**Function********************************************************************

  Synopsis           [Returns the property type]

  Description        [Returns the property kind of the stroed
  property, i.e. CTL, LTL, ...]

  SideEffects        []

******************************************************************************/
Prop_Type Prop_get_type(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);

  return self->type ;
}


/**Function********************************************************************

  Synopsis           [Returns the status of the property]

  Description        [Returns the status of the property]

  SideEffects        []

******************************************************************************/
Prop_Status Prop_get_status(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);

  return self->status;
}

/**Function********************************************************************

  Synopsis           [Sets the status of the property]

  Description        [Sets the status of the property]

  SideEffects        []

******************************************************************************/
void Prop_set_status(Prop_ptr self, Prop_Status s)
{
  PROP_CHECK_INSTANCE(self);
  self->status = s;
}


/**Function********************************************************************

  Synopsis           [Returns the number of the property]

  Description        [For COMPUTE properties returns the number
  resulting from the evaluation of the property.]

  SideEffects        []

******************************************************************************/
int Prop_get_number(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->number;
}


/**Function********************************************************************

  Synopsis           [Sets the number of the property]

  Description        [Sets the number resulting from the
  evaluation of the property.]

  SideEffects        []

******************************************************************************/
void Prop_set_number(Prop_ptr self, int n)
{
  PROP_CHECK_INSTANCE(self);
  self->number = n;
}

/**Function********************************************************************

  Synopsis           [Sets the number of the property to INFINITE]

  Description        [Sets the to INFINITE the number resulting from the
  evaluation of the property.]

  SideEffects        []

******************************************************************************/
void Prop_set_number_infinite(Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  self->number = PROP_INFINITE;
}


/**Function********************************************************************

  Synopsis           [Returns the trace number associated to a property]

  Description        [For unsatisfied properties, the trace number of
  the asscociated counterexample is returned.]

  SideEffects        []

******************************************************************************/
int Prop_get_trace(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->trace;
}


/**Function********************************************************************

  Synopsis           [Sets the trace number]

  Description        [Sets the trace number for an unsatisfied property.]

  SideEffects        []

******************************************************************************/
void Prop_set_trace(Prop_ptr self, int t)
{
  PROP_CHECK_INSTANCE(self);
  self->trace = t;
}


/**Function********************************************************************

  Synopsis           [Returns the scalar FSM in sexp of a property]

  Description        [Resturns the scalar FSM in sexp associated to
  the property. Self keeps the ownership of the given fsm]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr Prop_get_scalar_sexp_fsm(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->scalar_fsm;
}


/**Function********************************************************************

  Synopsis           [Sets the scalar FSM in sexp of a property]

  Description        [The given fsm will be duplicated, so the caller keeps 
  the ownership of fsm]

  SideEffects        []

******************************************************************************/
void Prop_set_scalar_sexp_fsm(Prop_ptr self, SexpFsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(self);
  prop_set_scalar_sexp_fsm(self, fsm, true);
}



/**Function********************************************************************

  Synopsis           [Returns the boolean FSM in sexp of a property]

  Description        [Resturns the boolean FSM in sexp associated to
  the property. Self keeps the ownership of the given fsm]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr Prop_get_bool_sexp_fsm(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->bool_fsm;
}


/**Function********************************************************************

  Synopsis           [Sets the boolean FSM in sexp of a property]

  Description        [The given fsm will be duplicated, so the caller keeps 
  the ownership of fsm]

  SideEffects        []

******************************************************************************/
void Prop_set_bool_sexp_fsm(Prop_ptr self, SexpFsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(self);
  prop_set_bool_sexp_fsm(self, fsm, true);
}


/**Function********************************************************************

  Synopsis           [Returns the boolean FSM in BDD of a property]

  Description        [Returns the boolean FSM in BDD associated to
  the property. Self keeps the ownership of the given fsm]

  SideEffects        []

******************************************************************************/
BddFsm_ptr Prop_get_bdd_fsm(Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->bdd_fsm;
}


/**Function********************************************************************

  Synopsis           [Sets the boolean FSM in BDD of a property]

  Description        [The given fsm will be duplicated, so the caller keeps 
  the ownership of fsm]

  SideEffects        []

******************************************************************************/
void Prop_set_bdd_fsm(Prop_ptr self, BddFsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(self);
  prop_set_bdd_fsm(self, fsm, true);
}


/**Function********************************************************************

  Synopsis           [Returns the boolean FSM in BE of a property]

  Description        [Returns the boolean FSM in BE associated to
  the property. Self keeps the ownership of the given fsm]

  SideEffects        []

******************************************************************************/
Bmc_Fsm_ptr Prop_get_be_fsm(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);
  return self->be_fsm;
}


/**Function********************************************************************

  Synopsis           [Sets the boolean FSM in BE of a property]

  Description        [The given fsm will be duplicated, so the caller keeps 
  the ownership of fsm]

  SideEffects        []

******************************************************************************/
void Prop_set_be_fsm(Prop_ptr self, Bmc_Fsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(self);
  prop_set_be_fsm(self, fsm, true);
}


/**Function********************************************************************

  Synopsis           [Applies cone of influence to the given property]

  Description        [The COI is applied only for BDD-based model checking. 
  To apply for BMC, use Prop_apply_coi_for_bmc]

  SideEffects        [Internal FSMs are computed]

******************************************************************************/
void Prop_apply_coi_for_bdd(Prop_ptr self, FsmBuilder_ptr helper) 
{
  SexpFsm_ptr scalar_fsm;
  BddFsm_ptr  bdd_fsm;
  Encoding_ptr senc;
  boolean applied = false;


  PROP_CHECK_INSTANCE(self);

  senc = Enc_get_symb_encoding();
  scalar_fsm = Prop_get_scalar_sexp_fsm(self);
  bdd_fsm    = Prop_get_bdd_fsm(self);

  /* scalar sexp fsm */
  if (scalar_fsm == SEXP_FSM(NULL)) {
    VarSet_ptr vars; 

    Set_t spec_dep = Formulae_GetDependencies(senc, 
              Prop_get_expr(self), 
	      Compile_FlattenSexp(senc, cmp_struct_get_justice(cmps), Nil), 
	      Compile_FlattenSexp(senc, cmp_struct_get_compassion(cmps), Nil));

    Set_t cone = ComputeCOI(senc, spec_dep);

    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Cone of influence\n");
    }
    
    vars = Set_Set2List(cone);
    scalar_fsm = FsmBuilder_create_sexp_fsm(helper, senc, vars, 
					    SEXP_FSM_TYPE_SCALAR);
    
    Prop_set_cone(self, cone);
    prop_set_scalar_sexp_fsm(self, scalar_fsm, false); /* does not dup */

    applied = true;
  }

  /* bdd fsm */
  if (bdd_fsm == BDD_FSM(NULL)) {
    bdd_fsm = FsmBuilder_create_bdd_fsm(helper, Enc_get_bdd_encoding(), 
					scalar_fsm, 
					get_partition_method(options));
    prop_set_bdd_fsm(self, bdd_fsm, false); /* does not dup */
    applied = true;
  }

  if (! applied) {
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Using previously built model for COI...\n");
    }
  }
  
}


/**Function********************************************************************

  Synopsis           [Applies cone of influence to the given property]

  Description        [The COI is applied only for BMC-based model checking. 
  To apply for BDD, use Prop_apply_coi_for_bdd]

  SideEffects        [Internal FSMs are computed]

******************************************************************************/
void Prop_apply_coi_for_bmc(Prop_ptr self, FsmBuilder_ptr helper) 
{
  SexpFsm_ptr bool_fsm;
  Bmc_Fsm_ptr be_fsm;
  Encoding_ptr senc;
  boolean applied = false;

  PROP_CHECK_INSTANCE(self);

  bool_fsm   = Prop_get_bool_sexp_fsm(self);
  be_fsm    = Prop_get_be_fsm(self);
  senc = Enc_get_symb_encoding();

  /* boolean sexp fsm */
  if (bool_fsm == SEXP_FSM(NULL)) {
    VarSet_ptr vars; 

    Set_t spec_dep = 
      Formulae_GetDependencies(senc, Prop_get_expr(self), 
			       cmp_struct_get_justice(cmps), 
			       cmp_struct_get_compassion(cmps));

    Set_t cone = ComputeCOI(senc, spec_dep);

    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Cone of influence\n");
    }
    
    vars = Set_Set2List(cone);
    bool_fsm = FsmBuilder_create_sexp_fsm(helper, senc, vars,
					  SEXP_FSM_TYPE_BOOLEAN);
    
    Prop_set_cone(self, cone);
    prop_set_bool_sexp_fsm(self, bool_fsm, false); /* does not dup */

    applied = true;
  }

  /* be fsm */
  if (be_fsm == (Bmc_Fsm_ptr) NULL) {

    /* Notice that currently a single variable manager instance
       exists, and it is handled by the BMC package as a public 
       global variable.  Current implementation is temporary kept in
       this format. */     
    be_fsm = Bmc_Fsm_CreateFromSexprFSM(gl_vars_mgr, bool_fsm);
    prop_set_be_fsm(self, be_fsm, false); /* does not dup */
    applied = true;
  }
  
  if (! applied) {
    if (opt_verbose_level_gt(options, 0)) {
      fprintf(nusmv_stderr, "Using previously built model for COI...\n");
    }
  }

}


/**Function********************************************************************

  Synopsis           [Prints a property with info or its position and status 
  within the database]

  Description        [Prints a property on the specified FILE
  stream. Some of the information stored in the property structure are
  printed out (e.g. property, property kind, property status, ...).]

  SideEffects        []

******************************************************************************/
void Prop_print_db(Prop_ptr self, FILE* file) 
{
  PROP_CHECK_INSTANCE(self);

  fprintf(file, "%.3d : ", self->index);
  prop_print(self, file); 
  fprintf(file, "\n");

  fprintf(file, "\t[%-15s", Prop_get_type_as_string(self));
  if (self->type == Prop_Compute) {
    char* str_number = Prop_get_number_as_string(self);
    fprintf(file, "%-15s", str_number);
    FREE(str_number);
  }
  else fprintf(file, "%-15s", Prop_get_status_as_string(self));
  fprintf(file, (self->trace == 0 ? "N/A ]\n" : "T%-3d]\n"), self->trace);
}


/**Function********************************************************************

  Synopsis           [Prints a property]

  Description        [Prints a property. 
  PSL properties are specially handled.]

  SideEffects        []

******************************************************************************/
void Prop_print(Prop_ptr self, FILE* file) 
{
  PROP_CHECK_INSTANCE(self);
  prop_print(self, file); 
}


/**Function********************************************************************

  Synopsis           [Returns the number value as a string (only for compute 
  types)]

  Description        [Returns a number, 'Inifinite' or 'Unchecked'. 
  The returned string is dynamically created, and caller must free it.]

  SideEffects        []

******************************************************************************/
char* Prop_get_number_as_string(const Prop_ptr self)
{
  char buf[16];
  char* ret = NULL;
  int n; 

  PROP_CHECK_INSTANCE(self);

  nusmv_assert(Prop_get_type(self) == Prop_Compute); /* compute type only */

  n = Prop_get_number(self);
  if (n == PROP_UNCHECKED) snprintf(buf, sizeof(buf), "Unchecked");
  else if (n == PROP_INFINITE) snprintf(buf, sizeof(buf), "Infinite");
  else snprintf(buf, sizeof(buf), "%d", n);
  
  ret = ALLOC(char, strlen(buf)+sizeof(char));
  nusmv_assert(ret != NULL);

  strcpy(ret, buf);
  return ret;
}


/**Function********************************************************************

  Synopsis           [Returns the context name of a property]

  Description        [If the property has no explicit context, 'Main' will
  be returned. The returned string must be deleted by the caller.]

  SideEffects        []

******************************************************************************/
char* Prop_get_context_text(const Prop_ptr self) 
{
  char* cntx = (char *)NULL;
  char* EMTPY_CONTEXT_STR = "Main";
  node_ptr context;

  PROP_CHECK_INSTANCE(self);

  context = (node_ptr) self->prop;

  if (node_get_type(context) == CONTEXT) { 
    context = car(context);
    if (context != Nil) {
      cntx = sprint_node(context);
    }
    else {
      cntx = ALLOC(char, strlen(EMTPY_CONTEXT_STR)+1);
      nusmv_assert(cntx != NULL);
      strcpy(cntx, EMTPY_CONTEXT_STR);  
    }    
  }
  else {
    cntx = ALLOC(char, strlen(EMTPY_CONTEXT_STR)+1);
    nusmv_assert(cntx != NULL);
    strcpy(cntx, EMTPY_CONTEXT_STR);
  }

  return cntx;
}


/**Function********************************************************************

  Synopsis           [Returns the property text, with no explicit context]

  Description        [The returned string must be deleted by the caller.]

  SideEffects        []

******************************************************************************/
char* Prop_get_text(const Prop_ptr self) 
{
  node_ptr p;

  PROP_CHECK_INSTANCE(self);

  p = (node_ptr) Prop_get_expr(self);
  if (node_get_type(p) == CONTEXT) p = cdr(p);  /* skip context */

  return sprint_node(p);
}




/**Function********************************************************************

  Synopsis           [Returns the scalar FSM]

  Description        [Returns the scalar FSM stored in the master prop]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr Prop_master_get_scalar_sexp_fsm()
{
  PROP_CHECK_INSTANCE(Prop_Master);
  return Prop_Master->scalar_fsm; 
}

/**Function********************************************************************

  Synopsis           [Set the scalar FSM]

  Description        [Set the scalar FSM of the master prop]

  SideEffects        []

******************************************************************************/
void  Prop_master_set_scalar_sexp_fsm(SexpFsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(Prop_Master);
  nusmv_assert(Prop_Master->scalar_fsm == SEXP_FSM(NULL));

  Prop_Master->scalar_fsm = fsm;
}

/**Function********************************************************************

  Synopsis           [Returns the boolean FSM in sexp]

  Description        [Returns the boolean FSM in sexp stored in the master prop. 
  The prop package becomes the owner of the given fsm]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr Prop_master_get_bool_sexp_fsm()
{
  PROP_CHECK_INSTANCE(Prop_Master);
  return Prop_Master->bool_fsm;
}

/**Function********************************************************************

  Synopsis           [Set the boolean FSM in sexp]

  Description        [Set the boolean FSM in sexp of the master prop. The 
  prop package becomes the owner of the given fsm]

  SideEffects        []

******************************************************************************/
void Prop_master_set_bool_sexp_fsm(SexpFsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(Prop_Master);
  nusmv_assert(Prop_Master->bool_fsm == SEXP_FSM(NULL));

  Prop_Master->bool_fsm = fsm;
}

/**Function********************************************************************

  Synopsis           [Returns the boolean FSM in BDD]

  Description        [Returns the boolean FSM in BDD stored in the master prop]

  SideEffects        []

******************************************************************************/
BddFsm_ptr Prop_master_get_bdd_fsm()
{
  PROP_CHECK_INSTANCE(Prop_Master);
  return Prop_Master->bdd_fsm;
}

/**Function********************************************************************

  Synopsis           [Set the boolean FSM in BDD]

  Description        [Set the boolean FSM in BDD of the master prop. The 
  prop package becomes the owner of the given fsm]

  SideEffects        []

******************************************************************************/
void Prop_master_set_bdd_fsm(BddFsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(Prop_Master);
  nusmv_assert(Prop_Master->bdd_fsm == BDD_FSM(NULL));

  Prop_Master->bdd_fsm = fsm;
}

/**Function********************************************************************

  Synopsis           [Returns the boolean FSM in BE]

  Description        [Returns the boolean FSM in BE stored in the master prop]

  SideEffects        []

******************************************************************************/
Bmc_Fsm_ptr Prop_master_get_be_fsm()
{
  PROP_CHECK_INSTANCE(Prop_Master);
  return Prop_Master->be_fsm;
}

/**Function********************************************************************

  Synopsis           [Set the boolean FSM in BE]

  Description        [Set the boolean FSM in BE of the master prop. The 
  prop package becomes the owner of the given fsm]

  SideEffects        []

******************************************************************************/
void Prop_master_set_be_fsm(Bmc_Fsm_ptr fsm)
{
  PROP_CHECK_INSTANCE(Prop_Master);
  nusmv_assert(Prop_Master->be_fsm == (Bmc_Fsm_ptr) NULL);

  Prop_Master->be_fsm = fsm;
}


/**Function********************************************************************

  Synopsis           [Copies master prop FSM data into prop]

  Description        [Copies the FSM informations stored in the master
  prop into the corresponding fields of the given prop structure.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Prop_set_fsm_to_master(Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);

  Prop_set_scalar_sexp_fsm(self, Prop_master_get_scalar_sexp_fsm());
  Prop_set_bool_sexp_fsm(self, Prop_master_get_bool_sexp_fsm());
  Prop_set_bdd_fsm(self, Prop_master_get_bdd_fsm());
  Prop_set_be_fsm(self, Prop_master_get_be_fsm());
}



/**Function********************************************************************

  Synopsis           [Returns the a string associated to a property type]

  Description        [Returns the string corresponding to a property
  type for printing it. Returned string must NOT be deleted]

  SideEffects        []

******************************************************************************/
const char* Prop_get_type_as_string(Prop_ptr self) 
{
  PROP_CHECK_INSTANCE(self);
  return PropType_to_string(Prop_get_type(self));
}

/**Function********************************************************************

  Synopsis           [Returns the a string associated to a property status]

  Description        [Returns the string corresponding to a property
  status for printing it. The caller must NOT free the returned string, 
  dince it is a constant.]

  SideEffects        []

******************************************************************************/
const char* Prop_get_status_as_string(const Prop_ptr self) 
{
  char* res = (char*) NULL;
  Prop_Status t;

  PROP_CHECK_INSTANCE(self);

  t = Prop_get_status(self);

  switch (t) {
  case Prop_NoStatus:    res = PROP_NOSTATUS_STRING; break; 
  case Prop_Unchecked:   res = PROP_UNCHECKED_STRING; break; 
  case Prop_True:        res = PROP_TRUE_STRING; break; 
  case Prop_False:       res = PROP_FALSE_STRING; break; 
  case Prop_Wrong:       res = PROP_WRONG_STRING; break; 
  case Prop_Number:      res = PROP_NUMBER_STRING; break; 

  default:  nusmv_assert(false); /* invalid status */
  }

  return res;
}



/**Function********************************************************************

  Synopsis           [Check if a property in the database is of a given type]

  Description        [Checks if a property in the database is of a given type.
  If the type is correct, value 0 is returned, otherwise an error message 
  is emitted and value 1 is returned.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Prop_check_type(const Prop_ptr self, Prop_Type type) 
{ 
  PROP_CHECK_INSTANCE(self);

  if (Prop_get_type(self) != type) {
    fprintf(nusmv_stderr,
            "Error: specified property type is %s, "
	    "but type %s was expected.\n", 
            Prop_get_type_as_string(self), PropType_to_string(type));
    return 1;
  }

  return 0;
}




/**Function********************************************************************

  Synopsis           [Verifies a given property]

  Description        [Depending the property, different model checking
  algorithms are called. The status of the property is updated
  accordingly to the result of the verification process.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Prop_verify(Prop_ptr self)
{
  if (Prop_get_status(self) != Prop_Wrong) {
    if (Prop_get_status(self) == Prop_Unchecked)  {
      switch (Prop_get_type(self)) {
	
      case Prop_Ctl: 
	if (opt_ag_only(options)) {
	  if ( opt_forward_search(options)) { Mc_CheckAGOnlySpec(self); }
	  else {
	    /* Cannot use AG-only since reachables must be calculated before */
	    warning_ag_only_without_reachables();
	    Mc_CheckCTLSpec(self);
	  }
	}
	else { Mc_CheckCTLSpec(self); }
	break;
	
      case Prop_Compute:  Mc_CheckCompute(self); break;

      case Prop_Invar:    Mc_CheckInvar(self); break;

      case Prop_Ltl:      Ltl_CheckLtlSpec(self); break;

      case Prop_Psl:
	if (Prop_is_psl_ltl(self)) { Ltl_CheckLtlSpec(self); }
	else {
	  if (Prop_is_psl_obe(self)) {
	    if (opt_ag_only(options)) {
	      if ( opt_forward_search(options)) { Mc_CheckAGOnlySpec(self); }
	      else {
		/* Cannot use AG-only since reachables must be calculated before */
		warning_ag_only_without_reachables();
		Mc_CheckCTLSpec(self);
	      }
	    }
	    else { Mc_CheckCTLSpec(self); }
	  }
	  else { error_psl_not_supported_feature(); }
	}
	break; 

      default:  nusmv_assert(false); /* invalid type */
      }
    }
  }
}


/**Function********************************************************************

  Synopsis           [Returns the a string associated to a property type]

  Description        [Returns the string corresponding to a property
  type for printing it. Returned string must NOT be deleted]

  SideEffects        []

******************************************************************************/
const char* PropType_to_string(const Prop_Type type) 
{
  char* res = (char*) NULL;
  
  switch (type) {
  case Prop_NoType:  res = PROP_NOTYPE_STRING; break;
  case Prop_Ctl:     res = PROP_CTL_STRING; break;
  case Prop_Ltl:     res = PROP_LTL_STRING; break;
  case Prop_Psl:     res = PROP_PSL_STRING; break;
  case Prop_Invar:   res = PROP_INVAR_STRING; break; 
  case Prop_Compute: res = PROP_COMPUTE_STRING; break; 

  default: nusmv_assert(false); /* unknown type! */
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Returns true if the property is PSL property and it 
  is LTL compatible, that means that can be converted to LTL]

  Description        []

  SideEffects        []

******************************************************************************/
boolean Prop_is_psl_ltl(const Prop_ptr self)
{
  return (Prop_get_type(self) == Prop_Psl) && 
    PslNode_is_handled_psl(PslNode_remove_forall_replicators(self->prop));
}


/**Function********************************************************************

  Synopsis           [Returns true if the property is PSL property and it 
  is CTL compatible]

  Description        []

  SideEffects        []

******************************************************************************/
boolean Prop_is_psl_obe(const Prop_ptr self)
{
  return (Prop_get_type(self) == Prop_Psl) && PslNode_is_obe(self->prop);
}

/**Function********************************************************************

  Synopsis           [Creates a property, but does not insert it within 
  the database, so the property can be used on the fly.]

  Description [Creates a property structure filling only the property
  and property type fields. The property index within the db is not set.]

  SideEffects        []

******************************************************************************/
Prop_ptr Prop_create_partial(Expr_ptr expr, Prop_Type type)
{
  Prop_ptr self = Prop_create();
  PROP_CHECK_INSTANCE(self);

  self->index = -1;
  self->status = Prop_Unchecked;
  self->prop = expr;
  self->type = type;
  return self;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/**Function********************************************************************

  Synopsis           [Returns the index of a property]

  Description        [Returns the unique identifier of a property]

  SideEffects        []

******************************************************************************/
int Prop_get_index(const Prop_ptr self)
{
  PROP_CHECK_INSTANCE(self);

  return self->index;
}


/**Function********************************************************************

  Synopsis           [Sets the index of a property]

  Description        [Sets the unique identifier of a property]

  SideEffects        []

******************************************************************************/
void Prop_set_index(Prop_ptr self, const int index)
{
  PROP_CHECK_INSTANCE(self);
  self->index = index;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the property]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void prop_init(Prop_ptr self) 
{
  self->index = 0;
  self->prop = EXPR(NULL);
  self->psl2smv = EXPR(NULL);
  self->cone = (node_ptr) NULL;
  self->type = Prop_NoType;
  self->status = Prop_NoStatus;
  self->number = PROP_UNCHECKED;
  self->trace = 0;
  self->scalar_fsm = SEXP_FSM(NULL);
  self->bool_fsm = SEXP_FSM(NULL);
  self->bdd_fsm = BDD_FSM(NULL);
  self->be_fsm = (Bmc_Fsm_ptr)NULL;
}


/**Function********************************************************************

  Synopsis           [Destroy the elements of a property]

  Description        [Destroy the elements of a property]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void prop_deinit(Prop_ptr self) 
{
  if (self->be_fsm != NULL)  Bmc_Fsm_Delete( &(self->be_fsm) );

  if (self->bdd_fsm != BDD_FSM(NULL))  BddFsm_destroy(self->bdd_fsm);

  if (self->bool_fsm != SEXP_FSM(NULL))  SexpFsm_destroy(self->bool_fsm);

  if (self->scalar_fsm != SEXP_FSM(NULL))  SexpFsm_destroy(self->scalar_fsm);
}


/**Function********************************************************************

  Synopsis           [Prints a property]

  Description        [Prints a property. 
  PSL properties are specially handled.]

  SideEffects        []

******************************************************************************/
static void prop_print(const Prop_ptr self, FILE* file) 
{
  node_ptr p = (node_ptr) self->prop;
  node_ptr context = Nil;
  
  if (Prop_get_type(self) == Prop_Psl) {
    PslNode_print(file, p);
    return;
  }

  if (node_get_type(p) == CONTEXT) {
    context = car(p);
    p = cdr(p);
  }
  indent_node(file, " ", p, " ");
  if (context != Nil) {
    fprintf(file, "IN ");
    print_node(file, context);
  }
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
static void prop_set_scalar_sexp_fsm(Prop_ptr self, SexpFsm_ptr fsm, 
				     const boolean duplicate)
{
  if (self->scalar_fsm != SEXP_FSM(NULL)) SexpFsm_destroy(self->scalar_fsm);
  if (duplicate && (fsm != SEXP_FSM(NULL))) { 
    self->scalar_fsm = SexpFsm_copy(fsm);
  }
  else self->scalar_fsm = fsm; 
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
static void prop_set_bool_sexp_fsm(Prop_ptr self, SexpFsm_ptr fsm, 
				   const boolean duplicate)
{
  if (self->bool_fsm != SEXP_FSM(NULL)) SexpFsm_destroy(self->bool_fsm);
  if (duplicate && (fsm != SEXP_FSM(NULL))) {
    self->bool_fsm = SexpFsm_copy(fsm);
  }
  else self->bool_fsm = fsm;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
static void prop_set_bdd_fsm(Prop_ptr self, BddFsm_ptr fsm, 
			     const boolean duplicate)
{
  if (self->bdd_fsm != BDD_FSM(NULL)) BddFsm_destroy(self->bdd_fsm);
  if (duplicate && (fsm != BDD_FSM(NULL))) {
    self->bdd_fsm = BddFsm_copy(fsm);
  }
  else self->bdd_fsm = fsm; 
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
static void prop_set_be_fsm(Prop_ptr self, Bmc_Fsm_ptr fsm, 
			    const boolean duplicate)
{
  if (self->be_fsm != (Bmc_Fsm_ptr) NULL) Bmc_Fsm_Delete( &(self->be_fsm) );
  if (duplicate && (fsm != (Bmc_Fsm_ptr) NULL)) {
    self->be_fsm = Bmc_Fsm_Dup(fsm);
  }
  else self->be_fsm = fsm;
}

