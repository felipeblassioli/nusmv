/**CFile***********************************************************************

  FileName    [dagVertex.c]

  PackageName [dag]

  Synopsis    [Vertex handling.]

  Description [External procedures included in this module:
		<ul>
		<li> <b>Dag_VertexLookup()</b> Lookup for a vertex;
		<li> <b>Dag_VertexInsert()</b> Insert a vertex;
                <li> <b>Dag_VertexMark()</b> make a vertex permanent;
                <li> <b>Dag_VertexUnmark()</b> make a vertex volatile;
		</ul>
               Internal procedures included in this module:
                <ul>
                <li> <b>DagVertexInit()</b> Initialize a vertex;
                <li> <b>DagVertexComp()</b> Compare two vertices;
                <li> <b>DagVertexHash()</b> calculate vertex hash code;
                </ul>]
		
  SeeAlso     [dagManager dagDfs]

  Copyright   [
  This file is part of the ``dag'' package of NuSMV version 2. 
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

#include "dagInt.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define HASH_LOWER16   (int) 0x0000FFFF   /* Lower word in 32 bit integer. */
#define HASH_UPPER16   (int) 0xFFFF0000   /* Upper word in 32 bit integer. */

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


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Vertex lookup.]

  Description [Uniquely adds a new vertex into the DAG and returns a
               reference to it:
               <ul>
               <li> vSymb is a NON-NEGATIVE  integer (vertex label);
               <li> vData is a pointer to generic user data;
               <li> vSons is a list of vertices (possibly NULL).
               </ul>
	       Returns NIL(Dag_vertex_t) if there is no dagManager and 
	       if vSymb is negative.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Dag_Vertex_t *
Dag_VertexLookup(
  Dag_Manager_t * dagManager,            
  int             vSymb,
  char          * vData,
  lsList          vSons)
{

  char        ** slot;
  int            found;
  Dag_Vertex_t * v;
  
  /* A vertex cannot be added to an uninitialized dag and vSymb
     cannot be a negative number. */
  if ((dagManager == NIL(Dag_Manager_t)) || (vSymb < 0)) {
    return NIL(Dag_Vertex_t);
  }
  
  /* Temporary allocate the vertex, and fill in just the basic
     information to calculate the hash code. */
  v = ALLOC(Dag_Vertex_t, 1);
  v -> symbol = vSymb;
  v -> data = vData;
  v -> outList = vSons;
  v -> dag = dagManager;

  /* Lookup the vertex in the hash table. */
  found = st_find_or_add(dagManager -> vTable, (char *) v, &slot);

  if (found) {
    /* The key already existed: free temporary allocations and
       let v be the vertex found in the table. */
    if (vSons != (lsList) NULL) {
      (void) lsDestroy(vSons, (void (*)()) NULL);
    }
    FREE(v);
    v = (Dag_Vertex_t*) *slot;
  } else {
    /* The key was not there: store the vertex (the value coincides
       with the key), and make the vertex information complete. */
    *slot = (char *) v;
    DagVertexInit(dagManager, v);
  }

  return v;

} /* End of Dag_VertexLookup. */


/**Function********************************************************************

  Synopsis    [Vertex insert.]

  Description [Adds a vertex into the DAG and returns a
               reference to it:
               <ul>
               <li> vSymb is an integer code (vertex label);
               <li> vData is a generic annotation;
               <li> vSons must be a list of vertices (the intended sons).
               </ul>
	       Returns NIL(Dag_vertex_t) if there is no dagManager and 
	       if vSymb is negative.]
	       NOTICE: as opposed to Dag_VertexLookup, the unique table 
	       is not accessed, so there is no guarantee of uniqueness 
	       for vertices added with this mechanism.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Dag_Vertex_t *
Dag_VertexInsert(
  Dag_Manager_t * dagManager,            
  int             vSymb,
  char          * vData,
  lsList          vSons)
{

  Dag_Vertex_t * v;
  
  /* A vertex cannot be added to an uninitialized dag and vSymb
     cannot be a negative number. */
  if ((dagManager == NIL(Dag_Manager_t)) || (vSymb < 0)) {
    return NIL(Dag_Vertex_t);
  }
  
  /* Allocate the vertex, and fill in the information. */
  v = ALLOC(Dag_Vertex_t, 1);
  v -> symbol = vSymb;
  v -> data = vData;
  v -> outList = vSons;

  /* Initialize the vertex and return it. */
  DagVertexInit(dagManager, v);
  
  return v;

} /* End of Dag_VertexInsert. */


/**Function********************************************************************

  Synopsis    [Marks a vertex as permanent.]

  Description [Increments the vertex mark by one, so it cannot be
               deleted by garbage collection unless unmarked.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void Dag_VertexMark(
  Dag_Vertex_t  * v)
{                         
  ++(Dag_VertexGetRef(v) -> mark);
  return;
} /* End of Dag_VertexMark. */


/**Function********************************************************************

  Synopsis    [Unmarks a vertex (makes it volatile).]

  Description [Decrements the vertex mark by one, so it can be
               deleted by garbage collection when fatherless.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void Dag_VertexUnmark(
  Dag_Vertex_t  * v)
{ 
  if (Dag_VertexGetRef(v) -> mark > 0) {
    --(Dag_VertexGetRef(v) -> mark);
  }
  return;
} /* End of Dag_VertexUnmark. */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Vertex initialization.]

  Description [Performs several tasks:
               <ul>
	       <li> connects the vertex to the sons by increasing the sons'
	            marks
	       <li> removes sons from the free list if their mark
		    is increased to one for the first time;
	       <li> clears the vertex mark and stores the vertex in the 
	            free list;
	       <li> clears other internal fields.
	       </ul>] 

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
DagVertexInit(
  Dag_Manager_t * dagManager,            
  Dag_Vertex_t  * v)
{

  lsGen          gen;
  Dag_Vertex_t * vSon;

  if (v -> outList != (lsList) NULL) {
    gen = lsStart(v -> outList);
    while (lsNext(gen, (lsGeneric*) &vSon, LS_NH) == LS_OK) {
      /* Clear vertex from possible bit annotation. */
      vSon = Dag_VertexGetRef(vSon);
      /* Increment the number of fathers for each son: when it becomes one,
	 remove the son from the garbage bin. */ 
      if (++(vSon -> mark) == 1) {
	(void) lsRemoveItem(vSon -> vHandle, (lsGeneric*) &vSon);
	vSon -> vHandle = (lsHandle) NULL;
      }
    }
    lsFinish(gen);
  }
  /* The vertex is created fatherless, and it sits in the garbage bin. */
  v -> mark = 0;
  lsNewBegin(dagManager -> gcList, (lsGeneric) v, &(v -> vHandle));
 
  /* The vertex is owned by dagManager and it was never visited. */
  v -> dag = dagManager;
  v -> visit = 0;
  
  /* Update statistics. */
  ++(v -> dag -> stats[DAG_NODE_NO]);

  return;

} /* End of DagVertexInit. */


/**Function********************************************************************

  Synopsis    [Compare two vertices.]

  Description [Gets two vertex pointers v1, v2, (as char pointers) and
               compares the symbol, the generic data reference and the
	       pointers to the sons. Returns -1 if v1 < v2, 0 if v1 =
	       v2 and 1 if v1 > v2, in lexicographic order of fields.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
DagVertexComp(
  const char * v1, 
  const char * v2)
{
  
  lsGen     gen1, gen2;
  lsGeneric son1, son2;
  int       c;

  /* First compare symbols... */
  if ((c = ( (Dag_Vertex_t*) v1) -> symbol - 
       ((Dag_Vertex_t*) v2 ) -> symbol) != 0) {
    return c;
  }

  /* ...then compare generic data references. */
  if ((c = (int) (((Dag_Vertex_t*) v1) -> data) - 
       (int) (((Dag_Vertex_t*) v2) -> data)) != 0) {
    return c;
  }

  /* Verify that both outLists are non NULL. */
  if ((((Dag_Vertex_t*) v1) -> outList != (lsList) NULL) &&
      (((Dag_Vertex_t*) v2) -> outList) != (lsList) NULL) {

    /* If we arrived here, this is an internal vertex:
       compare how many sons... */
    if ((c = lsLength(((Dag_Vertex_t*) v1) -> outList) - 
	 lsLength(((Dag_Vertex_t*) v2) -> outList)) != 0) {
      
      return c;
    }
    
    /* ... finally compare each son. */
    gen2 = lsStart(((Dag_Vertex_t*) v2) -> outList);
    lsForEachItem(((Dag_Vertex_t*) v1) -> outList, gen1, son1) {
      (void) lsNext(gen2,  &son2, LS_NH);
      if ((c = (int) son1 - (int) son2) != 0) break;
    }
    if (c != 0) lsFinish(gen1);
    lsFinish(gen2);

  }

  return c;

} /* End of DagVertexComp. */


/**Function********************************************************************

  Synopsis    [Calculate the hash key of a vertex.]

  Description [Calculate a preliminary index as follows:
                  v -> symbol                            + 
                  8 low order bits of (int) (v -> data)  +
		 16 low order bits of each son up to MAXSON +
		  1 for each son whose edge is annotated
	       Return the modulus of the index and the actual hash size.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
DagVertexHash(
  char * v, 
  int    modulus)
{
  
  lsGeneric     son;
  lsGen         gen;
  int           i;
  unsigned long h;
  static int    x[DAGMAX_WORDS];
  int           x_length;
  int         * hashFn;
  
  /* Get the symbol. */
  x[0] = (((Dag_Vertex_t*) v) -> symbol & HASH_UPPER16) >> DAGWORD_SIZE;
  x[1] = ((Dag_Vertex_t*) v) -> symbol & HASH_LOWER16;

  /* Get the data. */
  x[2] = ((int)(((Dag_Vertex_t*) v) -> data) & HASH_UPPER16) >> DAGWORD_SIZE;
  x[3] = (int)(((Dag_Vertex_t*) v) -> data) & HASH_LOWER16;

  /* Get the son(s). */
  x_length = 4;
  if (((Dag_Vertex_t*) v)->outList != LS_NIL) {
    lsForEachItem(((Dag_Vertex_t*) v) -> outList, gen, son) {
      nusmv_assert(x_length + 1 < DAGMAX_WORDS);
      x[x_length++] = ((int)son & HASH_UPPER16) >> DAGWORD_SIZE;
      x[x_length++] = (int)son & HASH_LOWER16;
    }
  }

  hashFn = ((Dag_Vertex_t*) v) -> dag -> hashFn;
  /* Calculate hash key. */
  for (i = 0, h = 0; i < x_length; i++) {
    h += x[i] * hashFn[i];
  }

  /* Get the key and return it (division method). */
  return (int)(h % modulus);

} /* End of DagVertexHash. */

