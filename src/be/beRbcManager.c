/**CFile***********************************************************************

  FileName    [beRbcManager.c]

  PackageName [be]

  Synopsis    [Implementation for the RBC-based Boolean Expressions module. ]

  Description [This implementation is a wrapper for the RBC structure.]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``be'' package of NuSMV version 2.
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

#include "beRbcManager.h"
#include "be.h"
#include "beInt.h"

#include "rbc/rbc.h"
#include "opt/opt.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Constant********************************************************************

  Synopsis    [A switcher of the function Be_Cnf_RemoveDuplicateLiterals]

  Description [After BE2CNF conversion the function
  Be_Cnf_RemoveDuplicateLiterals removes duplicate literals 
  from clauses (see function Be_ConvertToCnf).
  But in the current implementation of RBC and BE2CNF-conversion,
  duplicate literals cannot appear. 
  If you nevertheless want to enable this function, define the macro
  REMOVE_DUPLICATE_LITERALS to 1.
  NOTE: for some reason Be_Cnf_RemoveDuplicateLiterals requires a lot of
  time. The problems seems to be in ListNode_destroy and node_find]

  SideEffects []

  SeeAlso     []

******************************************************************************/
#define REMOVE_DUPLICATE_LITERALS 0


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN options_ptr options;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro********************************************************************

  Synopsis    [Given a be_manager returns the contained rbc manager.]

  Description [This is a macro which can be used to simplify the code.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
#define GET_RBC_MGR(be_manager) \
  (Rbc_Manager_t*) be_manager->spec_manager


/**Macro********************************************************************

  Synopsis    [Converts a rbc into a be]

  Description [This is a macro which can be used to simplify the code.]

  SideEffects []

  SeeAlso     [RBC]

******************************************************************************/
#define BE(be_manager, spec) \
  be_manager->be2spec_converter(be_manager, (void*)spec)


/**Macro********************************************************************

  Synopsis    [Converts a be into a rbc]

  Description [This is a macro which can be used to simplify the code.]

  SideEffects []

  SeeAlso     [BE]

******************************************************************************/
#define RBC(be_manager, be) \
  (Rbc_t*) be_manager->spec2be_converter(be_manager, be)



/*---------------------------------------------------------------------------*/
/* Declarations of internal functions                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void* beRbc_Be2Rbc(Be_Manager_ptr mgr, be_ptr be);
static be_ptr beRbc_Rbc2Be(Be_Manager_ptr mgr, void* rbc);


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates a rbc-specific Be_Manager]

  Description [You must call Be_RbcManager_Delete when the created instance
  is no longer used.]

  SideEffects []

  SeeAlso     [Be_RbcManager_Delete]

******************************************************************************/
Be_Manager_ptr Be_RbcManager_Create(const size_t capacity)
{
  Rbc_Manager_t* spec = Rbc_ManagerAlloc(capacity);
  Be_Manager_ptr self = Be_Manager_Create(spec, &beRbc_Rbc2Be, &beRbc_Be2Rbc);
  return self;
}


/**Function********************************************************************

  Synopsis    [Destroys the given Be_MAnager instance you previously
  created by using Be_RbcManager_Create]

  Description []

  SideEffects []

  SeeAlso     [Be_RbcManager_Create]

******************************************************************************/
void Be_RbcManager_Delete(Be_Manager_ptr self)
{
  Rbc_ManagerFree((Rbc_Manager_t*) self->spec_manager);
  Be_Manager_Delete(self);
}


/**Function********************************************************************

  Synopsis    [Changes the maximum number of variables the rbc manager can
  handle]

  Description []

  SideEffects [The given rbc manager will possibly change]

  SeeAlso     []

******************************************************************************/
void Be_RbcManager_Reserve(Be_Manager_ptr self, const size_t size)
{
  Rbc_ManagerReserve((Rbc_Manager_t*)self->spec_manager, size);
}


/**Function********************************************************************

  Synopsis           [Returns true if the given be is the true value,
  otherwise returns false]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Be_IsTrue(Be_Manager_ptr manager, be_ptr arg)
{
  return (arg == Be_Truth(manager))? true : false;
}


/**Function********************************************************************

  Synopsis           [Returns true if the given be is the false value,
  otherwise returns false]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Be_IsFalse(Be_Manager_ptr manager, be_ptr arg)
{
  return (arg == Be_Falsity(manager))? true : false;
}


/**Function********************************************************************

  Synopsis           [Returns true if the given be is a constant value,
  such as either False or True]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Be_IsConstant(Be_Manager_ptr manager, be_ptr arg)
{
  return (Be_IsTrue(manager, arg) || Be_IsFalse(manager, arg));
}


/**Function********************************************************************

  Synopsis           [Builds a 'true' constant value]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Truth(Be_Manager_ptr manager)
{
  return BE(manager, Rbc_GetOne(GET_RBC_MGR(manager)));
}


/**Function********************************************************************

  Synopsis           [Builds a 'false' constant value]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Falsity(Be_Manager_ptr manager)
{
  return BE(manager, Rbc_GetZero(GET_RBC_MGR(manager)));
}


/**Function********************************************************************

  Synopsis           [Negates its argument]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Not(Be_Manager_ptr manager, be_ptr left)
{
  return BE( manager, Rbc_MakeNot(GET_RBC_MGR(manager), RBC(manager, left)) );
}


/**Function********************************************************************

  Synopsis           [Builds a new be which is the conjunction between
  its two arguments]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_And(Be_Manager_ptr manager, be_ptr left, be_ptr right)
{
  return BE(manager, Rbc_MakeAnd(GET_RBC_MGR(manager),
                                 RBC(manager, left),
                                 RBC(manager, right),
                                 RBC_TRUE));
}


/**Function********************************************************************

  Synopsis           [Builds a new be which is the disjunction of
  its two arguments]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Or(Be_Manager_ptr manager, be_ptr left, be_ptr right)
{
  return BE(manager, Rbc_MakeOr(GET_RBC_MGR(manager),
                                RBC(manager, left),
                                RBC(manager, right),
                                RBC_TRUE));
}


/**Function********************************************************************

  Synopsis           [Builds a new be which is the exclusive-disjunction
  of its two arguments]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Xor(Be_Manager_ptr manager, be_ptr left, be_ptr right)
{
  return BE(manager, Rbc_MakeXor(GET_RBC_MGR(manager),
                                 RBC(manager, left),
                                 RBC(manager, right),
                                 RBC_TRUE));
}


/**Function********************************************************************

  Synopsis           [Builds a new be which is the implication between
  its two arguments]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Implies(Be_Manager_ptr manager, be_ptr arg1, be_ptr arg2)
{
  /* (a -> b) <-> !(a & !b) <-> (!a | b) */
  return Be_Or(manager, Be_Not(manager, arg1), arg2);
}


/**Function********************************************************************

  Synopsis           [Builds a new be which is the logical equivalence
  between its two arguments]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Iff(Be_Manager_ptr manager, be_ptr left, be_ptr right)
{
  return BE(manager, Rbc_MakeIff(GET_RBC_MGR(manager),
                                 RBC(manager, left),
                                 RBC(manager, right),
                                 RBC_TRUE));
}


/**Function********************************************************************

  Synopsis           [Builds an if-then-else operation be]

  Description        []

  SideEffects        [...]

  SeeAlso            []

******************************************************************************/
be_ptr Be_Ite(Be_Manager_ptr manager, be_ptr c, be_ptr t, be_ptr e)
{
  return BE( manager, Rbc_MakeIte(GET_RBC_MGR(manager),
                                  RBC(manager, c),
                                  RBC(manager, t),
                                  RBC(manager, e),
                                  RBC_TRUE) );
}


/**Function********************************************************************

  Synopsis    [Creates a fresh copy G(X') of the be F(X) by shifting
  each variable index of a given amount]

  Description [Shifting operation replaces each occurence of the variable x_i
  in `f' with the variable x_(i + shift).
  A simple lazy mechanism is implemented to optimize that cases which
  given expression is a constant in]

  SideEffects []

  SeeAlso     []

******************************************************************************/
be_ptr Be_ShiftVar(Be_Manager_ptr manager, be_ptr f, int shift)
{
  shift_memoize_key key;
  be_ptr result = NULL;

  nusmv_assert(htShift_ptr != NULL); /* init problems? */

  /* lazy evaluation: */
  if (Be_IsConstant(manager, f)) return f;

  /* create the key, then search for it: */
  key.be   = f;
  key.shift = shift;

  if ( !st_lookup(htShift_ptr, (char*) &key, (char**) &result) ) {
    /* Duplicates the key */
    /* This ALLOC is deleted by Be_shiftHash_CallbackDeleteEntryAndKey: */
    shift_memoize_key* key_copy = ALLOC(shift_memoize_key, 1);
    key_copy->be    = key.be;
    key_copy->shift = key.shift;

    result = BE( manager, Rbc_Shift(GET_RBC_MGR(manager),
                                    RBC(manager, f), shift) );
    st_insert( htShift_ptr, (char*) key_copy, (char*) result );
  }

  nusmv_assert(result!=NULL);
  return result;
}


/**Function********************************************************************

  Synopsis    [Replaces all variables in f with other variables]

  Description [Replaces every occurence of the variable x_i in in `f'
  with the variable x_j provided that subst[i] = j.
  There is no need for `subst' to contain all the  variables,
  but it should map at least the variables in `f' in order for
  the substitution to work properly.]

  SideEffects [f will change]

  SeeAlso     []

******************************************************************************/
be_ptr Be_VarSubst(Be_Manager_ptr manager, be_ptr f, int* subst)
{
  return BE( manager, Rbc_Subst(GET_RBC_MGR(manager), RBC(manager, f), subst) );
}


/**Function********************************************************************

  Synopsis    [Converts the given be into the corresponding CNF-ed be]

  Description [Since it creates a new Be_Cnf structure, the caller
  is responsible for deleting it when it is no longer used
  (via Be_Cnf_Delete)]

  SideEffects []

  SeeAlso     [Be_Cnf_Delete]

******************************************************************************/
Be_Cnf_ptr Be_ConvertToCnf(Be_Manager_ptr manager, be_ptr f)
{
  Be_Cnf_ptr cnf;
  int max_var_idx; 
  int literalAssignedToWholeFormula = INT_MIN;

  /* performs the cnf conversion: */
  if (opt_verbose_level_gt(options, 0)) {
    fprintf(nusmv_stderr, "\nConverting the BE problem into CNF problem...\n");
  }

  cnf = Be_Cnf_Create(f);
  max_var_idx = Rbc_Convert2Cnf(GET_RBC_MGR(manager),
				RBC(manager, f),
				Be_Cnf_GetClausesList(cnf),
				Be_Cnf_GetVarsList(cnf),
				&literalAssignedToWholeFormula);
  
  nusmv_assert(literalAssignedToWholeFormula >= INT_MIN);

#if REMOVE_DUPLICATE_LITERALS 
  /* see the description of  REMOVE_DUPLICATE_LITERALS*/
  Be_Cnf_RemoveDuplicateLiterals(cnf);
#endif

  Be_Cnf_SetMaxVarIndex(cnf, max_var_idx);

  if (opt_verbose_level_gt(options, 1)) {
    fprintf(nusmv_stderr, " Conversion returned maximum variable index = %d\n",
	    Be_Cnf_GetMaxVarIndex(cnf));
    fprintf(nusmv_stderr, " Length of list of clauses = %d\n", 
	    Be_Cnf_GetClausesNumber(cnf));
    fprintf(nusmv_stderr, " Length of list of variables = %d\n", 
	    Be_Cnf_GetVarsNumber(cnf));
  }

  Be_Cnf_SetFormulaLiteral(cnf, literalAssignedToWholeFormula);
  return cnf;
}


/**Function********************************************************************

  Synopsis    [Converts the given CNF model into BE model]

  Description [Since it creates a new lsit , the caller
  is responsible for deleting it when it is no longer used
  (via lsDestroy)]

  SideEffects []

  SeeAlso     []

******************************************************************************/
lsList Be_CnfModelToBeModel(Be_Manager_ptr manager, lsList cnfModel)
{
  lsList beModel = lsCreate();
  lsGen gen;
  int cnfLiteral;
  int beLiteral;

  lsForEachItem(cnfModel, gen, cnfLiteral) {
    beLiteral = Be_CnfLiteral2BeLiteral(manager, cnfLiteral);
    /* if there is corresponding rbc variable => remember it */
    if (0 != beLiteral) {
      lsNewEnd(beModel, (lsGeneric)beLiteral, 0);
    }
  };
  
  return beModel;
}


/**Function********************************************************************

  Synopsis    [Converts a CNF literal into BE literal]

  Description [Literal is an index with optinal sign, so index = abs(literal)..
  The function returns 0 if there is no BE index associated with the given CNF
  index.
  A given CNF literal should be created by given BE manager (through 
  Be_ConvertToCnf).
  ]

  SideEffects []

  SeeAlso     [Be_ConvertToCnf]

******************************************************************************/
int Be_CnfLiteral2BeLiteral(const Be_Manager_ptr self, int cnfLiteral)
{
  int cnfIndex;
  int rbcIndex;

  /* literal is always != 0, otherwise the sign cannot be represented. */
  nusmv_assert(0 != cnfLiteral); 
  
  cnfIndex = abs(cnfLiteral); 
  rbcIndex = Rbc_CnfVar2RbcIndex(GET_RBC_MGR(self), cnfIndex);

  if (-1 != rbcIndex) return (cnfLiteral > 0) ? rbcIndex : -rbcIndex;  
  else return 0;
}


/**Function********************************************************************

  Synopsis    [Returns a CNF index associated with a given BE index]

  Description [If no CNF index is associated with a given BE index, 0 is
  returned.
  BE indexes are associated with CNF indexes through function Be_ConvertToCnf.
  Note: Input BE index cannot be 0, it is impossible in current implementation.
  ]

  SideEffects []

  SeeAlso     [Be_ConvertToCnf]

******************************************************************************/
int Be_BeIndex2CnfIndex(const Be_Manager_ptr self, int beIndex)
{
  return Rbc_RbcIndex2CnfVar(GET_RBC_MGR(self), beIndex);
}


/**Function********************************************************************

  Synopsis    [Dumps the given be into a file with Davinci format]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_DumpDavinci(Be_Manager_ptr manager, be_ptr f, FILE* outFile)
{
  Rbc_OutputDaVinci(GET_RBC_MGR(manager), RBC(manager, f), outFile);
}


/**Function********************************************************************

  Synopsis    [Dumps the given be into a file with Davinci format]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_DumpGdl(Be_Manager_ptr manager, be_ptr f, FILE* outFile)
{
  Rbc_OutputGdl(GET_RBC_MGR(manager), RBC(manager, f), outFile);
}


/**Function********************************************************************

  Synopsis    [Dumps the given be into a file]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_DumpSexpr(Be_Manager_ptr manager, be_ptr f, FILE* outFile)
{
  Rbc_OutputSexpr(GET_RBC_MGR(manager), RBC(manager, f), outFile);
}


/**Function********************************************************************

  Synopsis    [Converts the given variable index into the corresponding be]

  Description [If corresponding index had not been previously
  allocated, it will be allocated. If corresponding node does not
  exist in the dag, it will be inserted.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
be_ptr Be_Index2Var(Be_Manager_ptr manager, int varIndex)
{
  return BE(manager, Rbc_GetIthVar(GET_RBC_MGR(manager), varIndex));
}


/**Function********************************************************************

  Synopsis    [Converts the given variable (as boolean expression) into
  the corresponding index]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Be_Var2Index(Be_Manager_ptr manager, be_ptr var)
{
  return Rbc_GetVarIndex( RBC(manager, var) );
}


/**Function********************************************************************

  Synopsis    [Prints out some statistical data about the underlying
  rbc structure]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_PrintStats(Be_Manager_ptr manager, int clustSize, FILE* outFile)
{
  Rbc_PrintStats(GET_RBC_MGR(manager), clustSize, outFile);
}


/*---------------------------------------------------------------------------*/
/* Definitions of internal functions                                         */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definitions of static functions                                           */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Converts a be into a rbc]

  Description [The current implementation is really a simple type renaming.
  Internally used by Be_Manager via the inheritance mechanism.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void* beRbc_Be2Rbc(Be_Manager_ptr mgr, be_ptr be)
{
  return (Rbc_t*) be;
}


/**Function********************************************************************

  Synopsis    [Converts a rbc into a be]

  Description [The current implementation is really a simple type renaming.
  Internally used by Be_Manager via the inheritance mechanism.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static be_ptr beRbc_Rbc2Be(Be_Manager_ptr mgr, void* rbc)
{
  return (be_ptr) rbc;
}
