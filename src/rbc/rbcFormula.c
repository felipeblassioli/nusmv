/**CFile***********************************************************************

  FileName    [rbcFormula.c]

  PackageName [rbc]

  Synopsis    [Formula constructors.]

  Description [External functions included in this module:
		<ul>
		<li> <b>Rbc_GetOne()</b>        logical truth
		<li> <b>Rbc_GetZero()</b>       logical falsity
		<li> <b>Rbc_GetIthVar</b>       variables
		<li> <b>Rbc_MakeNot()</b>       negation
		<li> <b>Rbc_MakeAnd()</b>       conjunction
		<li> <b>Rbc_MakeOr()</b>        disjunction
		<li> <b>Rbc_MakeIff()</b>       coimplication
		<li> <b>Rbc_MakeXor()</b>       exclusive disjunction
		<li> <b>Rbc_MakeIte()</b>       if-then-else
		<li> <b>Rbc_GetLeftOpnd()</b>   return left operand
		<li> <b>Rbc_GetRightOpnd()</b>  return right operand
		<li> <b>Rbc_GetVarIndex()</b>   return the variable index 
		<li> <b>Rbc_Mark()</b>          make a vertex permanent
		<li> <b>Rbc_Unmark()</b>        make a vertex volatile
		</ul>]
		
  SeeAlso     []

  Author      [Armando Tacchella and Tommi Junttila]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by University of Genova. 

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

  Revision    [v. 1.0]

******************************************************************************/

#include "rbc/rbcInt.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
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

static Rbc_t * Reduce(Rbc_Manager_t * rbcManager, int op, Rbc_t * left, Rbc_t * right);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Logical constant 1 (truth).]

  Description [Returns the rbc that stands for logical truth.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_GetOne(
  Rbc_Manager_t * rbcManager)
{
  
  return (rbcManager -> one);

} /* End of Rbc_GetOne. */


/**Function********************************************************************

  Synopsis    [Logical constant 0 (falsity).]

  Description [Returns the rbc that stands for logical falsity.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_GetZero(
  Rbc_Manager_t * rbcManager)
{
  
  return (rbcManager -> zero);

} /* End of Rbc_GetZero. */


/**Function********************************************************************

  Synopsis    [Returns a variable.]

  Description [Returns a pointer to an rbc node containing the requested 
               variable. Works in three steps:
	       <ul>
	       <li> the requested variable index exceeds the current capacity:
	            allocated more room up to the requested index;
	       <li> the variable node does not exists: inserts it in the dag
	            and makes it permanent;
	       <li> returns the variable node.
	       </ul>]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_GetIthVar(
  Rbc_Manager_t * rbcManager,
  int             varIndex)
{
  int     i;

  /* Allocate more room for the varTable if needed. */
  if (rbcManager -> varCapacity <= varIndex) {
    rbcManager -> varTable = 
      REALLOC(Rbc_t*, rbcManager -> varTable, varIndex + 1);
    for (i = rbcManager -> varCapacity; i < varIndex + 1; i ++) {
      rbcManager -> varTable[i] = NIL(Rbc_t);
    }
    rbcManager -> varCapacity = varIndex + 1;
  }

  /* Create the variable if needed. */
  if (rbcManager -> varTable[varIndex] == NIL(Rbc_t)) {
    rbcManager -> varTable[varIndex] = 
      Dag_VertexInsert(rbcManager -> dagManager, 
		       RBCVAR, (char*)varIndex, (lsList)NULL);
    /* Make the node permanent. */
    Dag_VertexMark(rbcManager -> varTable[varIndex]);
    ++(rbcManager -> stats[RBCVAR_NO]);
  }

  /* Return the variable as rbc node. */
  return rbcManager -> varTable[varIndex];

} /* End of Rbc_GetIthVar. */


/**Function********************************************************************

  Synopsis    [Returns the complement of an rbc.]

  Description [Returns the complement of an rbc.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_MakeNot(
  Rbc_Manager_t * rbcManager,            
  Rbc_t         * left)
{

  return RbcId(left, RBC_FALSE);

} /* End of Rbc_MakeNot. */


/**Function********************************************************************

  Synopsis    [Makes the conjunction of two rbcs.]

  Description [Makes the conjunction of two rbcs.
               Works in three steps:
	       <ul>
	       <li> performs boolean simplification: if successfull, returns
	            the result of the simplification;
	       <li> orders left and right son pointers;
	       <li> looks up the formula in the dag and returns it.
	       </ul>]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_MakeAnd(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * left,
  Rbc_t         * right,
  Rbc_Bool_c      sign)
{
  Rbc_t* rTemp;
  lsList  sons;
  Rbc_t* l1;
  Rbc_t* l2;
  Rbc_t* r1;
  Rbc_t* r2;
  Rbc_t* d;

  int changed = 1;
  while (changed) {
    changed = 0;

    if (left == right) {
      /* AND(x,x) = x */
      right = NIL(Rbc_t);
    }
    else if (left == RbcId(right, RBC_FALSE)) {
      /* AND(x,!x) = F */
      left = rbcManager -> zero;
      right = NIL(Rbc_t);
    }
    else if ((left == rbcManager -> zero) || 
	(right == rbcManager -> zero)) {
      /* AND(F,y) = AND(x,F) = F */
      left = rbcManager -> zero;
      right = NIL(Rbc_t);
    }
    else if (left == rbcManager -> one) {
      /* AND(T,y) = y */
      left = right;
      right = NIL(Rbc_t);
    }
    else if (right == rbcManager -> one) {
      /* AND(x,T) = x */
      right = NIL(Rbc_t);
    }
    if (right == NIL(Rbc_t)) {
      return RbcId(left, sign);
    }
      
    nusmv_assert(left != NIL(Rbc_t));
    nusmv_assert(right != NIL(Rbc_t));

      if(!Dag_VertexIsSet(left) &&
	 (Dag_VertexGetRef(left)->symbol == RBCAND)) {
	  lsGen gen = lsStart(Dag_VertexGetRef(left)->outList);
	  l1 = 0; l2 = 0;
	  if(lsNext(gen, (lsGeneric*) &l1, LS_NH) != LS_OK) nusmv_assert(false);
	  if(lsNext(gen, (lsGeneric*) &l2, LS_NH) != LS_OK) nusmv_assert(false);
	  if(lsNext(gen, (lsGeneric*) &d, LS_NH) == LS_OK) nusmv_assert(false);
	  lsFinish(gen);
	  assert(l1 && l2);
	  if(right == l1) {
	    /* AND(AND(x,y),x) = AND(y,x) */
	    left = l2;
	    changed = 1;
	  }
	  else if(right == RbcId(l1, RBC_FALSE)) {
	    /* AND(AND(x,y),~x) = F */
	    return RbcId(rbcManager -> zero, sign);
	  }
	  else if(right == l2) {
	    /* AND(AND(x,y),y) = AND(x,y) */
	    left = l1;
	    changed = 1;
	  }
	  else if(right == RbcId(l2, RBC_FALSE)) {
	    /* AND(AND(x,y),~y)) = F */
	    return RbcId(rbcManager -> zero, sign);
	  }
	}

      if(Dag_VertexIsSet(left) &&
	 (Dag_VertexGetRef(left)->symbol == RBCAND))
	{
	  lsGen gen = lsStart(Dag_VertexGetRef(left)->outList);
	  l1 = 0; l2 = 0;
	  if(lsNext(gen, (lsGeneric*) &l1, LS_NH) != LS_OK) nusmv_assert(false);
	  if(lsNext(gen, (lsGeneric*) &l2, LS_NH) != LS_OK) nusmv_assert(false);
	  if(lsNext(gen, (lsGeneric*) &d, LS_NH) == LS_OK) nusmv_assert(false);
	  lsFinish(gen);
	  assert(l1 && l2);
	  if(right == l1) {
	    /* AND(~AND(x,y),x) = AND(~y,x) */
	    left = RbcId(l2, RBC_FALSE);
	    changed = 1;
	  }
	  else if(right == RbcId(l1, RBC_FALSE)) {
	    /* AND(~AND(x,y),~x) = ~x */
	    return RbcId(right, sign);
	  }
	  else if(right == l2) {
	    /* AND(~AND(x,y),y) = AND(~x,y)*/
	    left = RbcId(l1, RBC_FALSE);
	    changed = 1;
	  }
	  else if(right == RbcId(l2, RBC_FALSE)) {
	    /* AND(~AND(x,y),~y) = ~y */
	    return RbcId(right, sign);
	  }
	}
      if(!Dag_VertexIsSet(right) &&
	 (Dag_VertexGetRef(right)->symbol == RBCAND))
	{
	  lsGen gen = lsStart(Dag_VertexGetRef(right)->outList);
	  r1 = 0; r2 = 0;
	  if(lsNext(gen, (lsGeneric*) &r1, LS_NH) != LS_OK) nusmv_assert(false);
	  if(lsNext(gen, (lsGeneric*) &r2, LS_NH) != LS_OK) nusmv_assert(false);
	  if(lsNext(gen, (lsGeneric*) &d, LS_NH) == LS_OK) nusmv_assert(false);
	  lsFinish(gen);
	  assert(r1 && r2);
	  if(left == r1) {
	    /* AND(x,AND(x,y)) = AND(x,y) */
	    right = r2;
	    changed = 1;
	  }
	  else if(left == RbcId(r1, RBC_FALSE)) {
	    /* AND(~x,AND(x,y)) = F */
	    return RbcId(rbcManager -> zero, sign);
	  }
	  else if(left == r2) {
	    /* AND(y,AND(x,y)) = AND(y,x) */
	    right = r1;
	    changed = 1;
	  }
	  else if(left == RbcId(r2, RBC_FALSE)) {
	    /* AND(~y,AND(x,y)) = F */
	    return RbcId(rbcManager -> zero, sign);
	  }
	}
      if(Dag_VertexIsSet(right) &&
	 (Dag_VertexGetRef(right)->symbol == RBCAND))
	{
	  lsGen gen = lsStart(Dag_VertexGetRef(right)->outList);
	  r1 = 0; r2 = 0;
	  if(lsNext(gen, (lsGeneric*) &r1, LS_NH) != LS_OK) assert(false);
	  if(lsNext(gen, (lsGeneric*) &r2, LS_NH) != LS_OK) assert(false);
	  if(lsNext(gen, (lsGeneric*) &d, LS_NH) == LS_OK) assert(false);
	  lsFinish(gen);
	  assert(r1 && r2);
	  if(left == r1) {
	    /* AND(x,~AND(x,y)) = AND(x,~y) */
	    right = RbcId(r2, RBC_FALSE);
	    changed = 1;
	  }
	  else if(left == RbcId(r1, RBC_FALSE)) {
	    /* AND(~x,~AND(x,y)) = ~x */
	    return RbcId(left, sign);
	  }
	  else if(left == r2) {
	    /* AND(y,~AND(x,y)) = AND(y,~x) */
	    right = RbcId(r1, RBC_FALSE);
	    changed = 1;
	  }
	  else if(left == RbcId(r2, RBC_FALSE)) {
	    /* AND(~y,~AND(x,y)) = ~y */
	    return RbcId(left, sign);
	  }
	}
    }
  
  /* Order the vertices. */
  if (right < left) {
    rTemp = right; right = left; left = rTemp;
  }

  nusmv_assert(left != NIL(Rbc_t));
  nusmv_assert(right != NIL(Rbc_t));
  nusmv_assert(left != right);
  nusmv_assert(left != RbcId(right, RBC_FALSE));

  /* Create the list of sons. */
  sons = lsCreate();
  (void) lsNewEnd(sons, (lsGeneric) left, LS_NH);
  (void) lsNewEnd(sons, (lsGeneric) right, LS_NH);

  /* Lookup the formula in the dag. */
  rTemp =  Dag_VertexLookup(rbcManager -> dagManager, 
			    RBCAND, NIL(char), sons); 

  return RbcId(rTemp, sign);

} /* End of Rbc_MakeAnd. */


/**Function********************************************************************

  Synopsis    [Makes the disjunction of two rbcs.]

  Description [Makes the disjunction of two rbcs: casts the connective to
               the negation of a conjunction using De Morgan's law.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_MakeOr(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * left,
  Rbc_t         * right,
  Rbc_Bool_c      sign)
{
  /* Use De Morgan's law. */
  return Rbc_MakeAnd(rbcManager,
		     RbcId(left, RBC_FALSE),
		     RbcId(right, RBC_FALSE),
		     (Rbc_Bool_c)((int)sign ^ (int)RBC_FALSE));

} /* End of Rbc_MakeOr. */


/**Function********************************************************************

  Synopsis    [Makes the coimplication of two rbcs.]

  Description [Makes the coimplication of two rbcs.
               Works in four steps:
	       <ul>
	       <li> performs boolean simplification: if successfull, returns
	            the result of the simplification;
	       <li> orders left and right son pointers;
	       <li> re-encodes the negation 
	       <li> looks up the formula in the dag and returns it.
	       </ul>]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_MakeIff(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * left,
  Rbc_t         * right,
  Rbc_Bool_c      sign)
{
  Rbc_t * rTemp;
  lsList  sons;

  /* First, perform the reduction stage. */
  if ((rTemp = Reduce(rbcManager, RBCIFF, left, right)) != NIL(Rbc_t)) {
    return RbcId(rTemp, sign);
  }
  
  /* Negation always on top. */
  sign = sign ^ Dag_VertexIsSet(left) ^ Dag_VertexIsSet(right);
  Dag_VertexClear(left);
  Dag_VertexClear(right);

  /* Order the  vertices. */
  if (right < left) {
    rTemp = right; right = left; left = rTemp;
  }

  /* Create the list of sons. */
  sons = lsCreate();
  (void) lsNewEnd(sons, (lsGeneric) left, LS_NH);
  (void) lsNewEnd(sons, (lsGeneric) right, LS_NH);

  /* Lookup the formula in the dag. */
  rTemp =  Dag_VertexLookup(rbcManager -> dagManager, 
			     RBCIFF, NIL(char), sons); 

  return RbcId(rTemp, sign);

} /* End of Rbc_MakeIff. */


/**Function********************************************************************

  Synopsis    [Makes the exclusive disjunction of two rbcs.]

  Description [Makes the exclusive disjunction of two rbcs: casts the
               connective as the negation of a coimplication.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_MakeXor(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * left,
  Rbc_t         * right,
  Rbc_Bool_c      sign)
{
  /* Simply a negation of a coimplication. */
  return Rbc_MakeIff(rbcManager, left, right,
		     (Rbc_Bool_c)((int)sign ^ 
				  (int)RBC_FALSE));

} /* End of Rbc_MakeXor. */


/**Function********************************************************************

  Synopsis    [Makes the if-then-else of three rbcs.]

  Description [Makes the if-then-else of three rbcs: expands the connective
              into the corresponding product-of-sums.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t * 
Rbc_MakeIte(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * i,
  Rbc_t         * t,
  Rbc_t         * e,
  Rbc_Bool_c      sign)
{
  Rbc_t * rTemp;
  lsList  sons;

  /*
   * Bottom up simplification
   */
  int changed = 1;
  while(changed) {
      changed = 0;

      if (i == rbcManager -> one) {
	/* ITE(T,t,e) == t */
	return RbcId(t, sign);
      } 
      else if (i == rbcManager -> zero) {
	/* ITE(F,t,e) == e */
	return RbcId(e, sign);
      }
      else if (t == rbcManager -> one) {
	/* ITE(i,T,e) == OR(i,e) */
	return Rbc_MakeOr(rbcManager, i, e, sign);
      }
      else if (t == rbcManager -> zero) {
	/* ITE(i,F,e) == AND(~i,e) */
	return Rbc_MakeAnd(rbcManager, Rbc_MakeNot(rbcManager, i), e, sign);
      }
      else if (e == rbcManager -> one) {
	/* ITE(i,t,T) == OR(~i,t) */
	return Rbc_MakeOr(rbcManager, Rbc_MakeNot(rbcManager, i), t, sign);
      }
      else if (e == rbcManager -> zero) {
	/* ITE(i,t,F) == AND(i,t) */
	return Rbc_MakeAnd(rbcManager, i, t, sign);
      }
      if (i == t) {
	/* ITE(i,i,e) == OR(i,e) */
	return Rbc_MakeOr(rbcManager, i, e, sign);
      }
      else if(i == e) {
	/* ITE(i,t,i) == AND(i,t) */
	return Rbc_MakeAnd(rbcManager, i, t, sign);
      }
      else if(t == e) {
	/* ITE(i,t,t) == t */
	return RbcId(t, sign);
      }
      else if(i == RbcId(t, RBC_FALSE)) {
	/* ITE(i,~i,e) == AND(~i,e) */
	return Rbc_MakeAnd(rbcManager, Rbc_MakeNot(rbcManager, i), e, sign);
      }
      else if(i == RbcId(e, RBC_FALSE)) {
	/* ITE(i,t,~i) == OR(~i,t) */
	return Rbc_MakeOr(rbcManager, Rbc_MakeNot(rbcManager, i), t, sign);
      }
      else if(t == RbcId(e, RBC_FALSE)) {
	/* ITE(i,t,~t) == IFF(i,t) */
	return Rbc_MakeIff(rbcManager, i, t, sign);
      }
    }

  /* Create the list of sons. */
  sons = lsCreate();
  (void) lsNewEnd(sons, (lsGeneric) i, LS_NH);
  (void) lsNewEnd(sons, (lsGeneric) t, LS_NH);
  (void) lsNewEnd(sons, (lsGeneric) e, LS_NH);


  /* Lookup the formula in the dag. */
  rTemp =  Dag_VertexLookup(rbcManager -> dagManager, 
			    RBCITE, NIL(char), sons); 

  return RbcId(rTemp, sign);

} /* End of Rbc_MakeIte. */


/**Function********************************************************************

  Synopsis    [Gets the left operand.]

  Description [Gets the left operand.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t *
Rbc_GetLeftOpnd(
  Rbc_t * f)
{

  if (Dag_VertexGetRef(f) -> outList != (lsList)NULL) {
    /* Reusing f to avoid introduction of new variables. */
    (void) lsFirstItem(Dag_VertexGetRef(f) -> outList, (lsGeneric*) &f, LS_NH);
  }
  return f;

} /* End of Rbc_GetLeftOpnd. */


/**Function********************************************************************

  Synopsis    [Gets the right operand.]

  Description [Gets the right operand.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t *
Rbc_GetRightOpnd(
  Rbc_t * f)
{

  if (Dag_VertexGetRef(f) -> outList != (lsList)NULL) {
    /* Reusing f to avoid introduction of new variables. */
    (void) lsLastItem(Dag_VertexGetRef(f) -> outList, (lsGeneric*) &f, LS_NH);
  }
  return f;

} /* End of Rbc_GeRightOpnd. */


/**Function********************************************************************

  Synopsis    [Gets the variable index.]

  Description [Returns the variable index, 
               -1 if the rbc is not a variable.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
int
Rbc_GetVarIndex(
  Rbc_t * f)
{

  if (Dag_VertexGetRef(f) -> symbol == RBCVAR) {
    return (int)(Dag_VertexGetRef(f) -> data);
  }
  return -1;

} /* End of Rbc_GetVarIndex. */


/**Function********************************************************************

  Synopsis    [Makes a node permanent.]

  Description [Marks the vertex in the internal dag. This saves the rbc
               from being wiped out during garbage collection.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void 
Rbc_Mark(
  Rbc_Manager_t * rbc,         
  Rbc_t        * f)
{
  
  /* To avoid calling another function, do it directly! */
  ++(Dag_VertexGetRef(f) -> mark);
  return;

} /* End of Rbc_Mark. */


/**Function********************************************************************

  Synopsis    [Makes a node volatile.]

  Description [Unmarks the vertex in the internal dag. This exposes the rbc
               to garbage collection.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void  
Rbc_Unmark(
  Rbc_Manager_t * rbc,         
  Rbc_t * f)
{

  if (Dag_VertexGetRef(f) -> mark > 0) {
    --(Dag_VertexGetRef(f) -> mark);
  }
  return;

} /* End of Rbc_Unmark. */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Reduction (simplification) of rbcs.]

  Description [Reduction (simplification) of rbcs.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
static Rbc_t *
Reduce(
 Rbc_Manager_t * rbcManager,         
 int             op,
 Rbc_t         * left,
 Rbc_t         * right)
{

  switch (op) {
  case RBCAND:
    if (left == right) {
      /* AND(x,x) = x */
      return left;
    } else if (left == RbcId(right, RBC_FALSE)) {
      /* AND(x,~x) = F */
      return rbcManager -> zero;
    } else if ((left == rbcManager -> zero) || 
	       (right == rbcManager -> zero)) {
      /* AND(F,x)  = AND(x,F) = F */
      return rbcManager -> zero;
    } else if (left == rbcManager -> one) {
      /* AND(T,x) = x */
      return right;
    } else if (right == rbcManager -> one) {
      /* AND(x,T) = x */
      return left;
    } else {
      return NIL(Rbc_t);
    }
  case RBCIFF:
    if (left == right) {
      /* IFF(x,x) = T */
      return rbcManager -> one;
    } else if (left == RbcId(right, RBC_FALSE)) {
      /* IFF(x,~x) = F */
      return rbcManager -> zero;
    } else if (left == rbcManager -> zero) {
      /* IFF(F,y) = ~y */
      return RbcId(right, RBC_FALSE);
    } else if (right == rbcManager -> zero) {
      /* IFF(x,F) = ~x */
      return RbcId(left, RBC_FALSE);
    } else if (left == rbcManager -> one) {
      /* IFF(T,y) = y */
      return right;
    } else if (right == rbcManager -> one) {
      /* IFF(x,T) = x */
      return left;
    } else {
      return NIL(Rbc_t);
    }
  }

  return NIL(Rbc_t);

} /* End of Reduce. */
