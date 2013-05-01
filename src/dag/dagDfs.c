/**CFile***********************************************************************

  FileName    [dagDfs.c]

  PackageName [dag]

  Synopsis    [Depth First Search routines.]

  Description [External procedures included in this module:
		<ul>
                <li> <b>Dag_Dfs()<b> Generic depth first search engine.
		</ul>]
		
  SeeAlso     [dagManager.c dagVertex.c]

  Author      [Armando Tacchella]

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


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void DFS(Dag_Vertex_t * v, Dag_DfsFunctions_t * dfsFun, char * dfsData, int vBit);
static int CleanSet(Dag_Vertex_t * f, char * cleanData, int sign);
static void CleanFirst(Dag_Vertex_t * f, char * cleanData, int sign);
static void CleanBack(Dag_Vertex_t * f, char * cleanData, int sign);
static void CleanLast(Dag_Vertex_t * f, char * cleanData, int sign);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/* A predefined DFS to clean user data in vertices. */
Dag_DfsFunctions_t dag_DfsClean = {CleanSet, 
				   CleanFirst, 
				   CleanBack,
				   CleanLast};


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Performs a generic DFS on the dag.]

  Description [The parameters are:
               <ul>
               <li> dfsRoot, the dag vertex where to start the DFS
               <li> dfsFun, the functions to perform the DFS steps
               <li> dfsData, a reference to generic data
               </ul>
               The function increments the DFS code for the dag
               manager owning dfsRoot and starts the DFS. Increment of
               the code guarantees that each node is visited once and 
	       only once. dfsFun -> Set() may change the default behaviour by 
	       forcing to DFS to visit nodes more than once, or by preventing
	       DFS to do a complete visit.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
Dag_Dfs(
  Dag_Vertex_t       * dfsRoot,           
  Dag_DfsFunctions_t * dfsFun,
  char               * dfsData)
{

  /* DFS cannot start from a NULL vertex. */
  if (dfsRoot == NIL(Dag_Vertex_t)) {
    return; 
  }
  
  /* Increment the current DFS code for the dag manager. */
  ++(Dag_VertexGetRef(dfsRoot) -> dag -> dfsCode);
  
  /* Start the real thing. */
   DFS(Dag_VertexGetRef(dfsRoot), dfsFun, dfsData, Dag_VertexIsSet(dfsRoot));

   return;

} /* End of Dag_Dfs. */
  

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs a generic (recursive) DFS on the dag.]

  Description [The parameters are:
               <ul>
               <li> v, the current dag vertex
               <li> dfsFun, the functions to perform the DFS
               <li> dfsData, a reference to generic data
	       <li> vBit, the incoming link annotation (0 or not-0)
               </ul>]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
static void
DFS(
  Dag_Vertex_t       * v,           
  Dag_DfsFunctions_t * dfsFun,
  char               * dfsData,
  int                  vBit)
{
  lsGen           gen;
  Dag_Vertex_t * vSon;
  int             set;

  /* dfsFun -> Set() is -1 if the node is to be visited and 1 if the node
     is not to be visited; 0 means that the DFS should decide what to do. */
  set = dfsFun -> Set(v, dfsData, vBit);
  if ((set == 1) || ((set == 0) && (v -> visit == v -> dag -> dfsCode))) {
    return;
  } else {
    v -> visit = v -> dag -> dfsCode;
  }

  /* Do the first visit. */
  (dfsFun -> FirstVisit)(v, dfsData, vBit);

  /* Visit each son (if any). */
  if (v -> outList != (lsList) NULL) {
   
    gen = lsStart(v -> outList);
    while (lsNext(gen, (lsGeneric*) &vSon, LS_NH) == LS_OK) {
      DFS(Dag_VertexGetRef(vSon), dfsFun, dfsData, Dag_VertexIsSet(vSon));
      /* Do the back visit. */
      (dfsFun -> BackVisit)(v, dfsData, vBit);
    }
    lsFinish(gen);

  }
   
  /* Do the last visit and return . */
  (dfsFun -> LastVisit)(v, dfsData, vBit);

  return;

} /* End of DFS. */


/**Function********************************************************************

  Synopsis    [Dfs Set for cleaning.]

  Description [Dfs Set for cleaning.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int
CleanSet(
 Dag_Vertex_t  * f,
 char          * cleanData,
 int             sign)
{

  /* All the nodes are visited once and only once. */
  return (0);

} /* End of CleanSet. */


/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for cleaning.]

  Description [Dfs FirstVisit for cleaning.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
CleanFirst(
 Dag_Vertex_t  * f,
 char          * cleanData,
 int             sign)
{

  /* Clean data. */
  f -> gRef = NIL(char);
  f -> iRef = 0;

  return;

} /* End of CleanFirst. */


/**Function********************************************************************

  Synopsis    [Dfs BackVisit for cleaning.]

  Description [Dfs BackVisit for cleaning.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
CleanBack(
 Dag_Vertex_t  * f,
 char          * cleanData,
 int             sign)
{

  return;

} /* End of CleanBack. */


/**Function********************************************************************

  Synopsis    [Dfs LastVisit for cleaning.]

  Description [Dfs LastVisit for cleaning.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
CleanLast(
 Dag_Vertex_t  * f,
 char          * cleanData,
 int             sign)
{

  return;

} /* End of CleanLast. */

