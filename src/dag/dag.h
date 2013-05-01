/**CHeaderFile*****************************************************************

  FileName    [dag.h]

  PackageName [dag]

  Synopsis    [Directed acyclic graphs with sharing.]

  Description [External functions and data strucures of the dag package.]

  SeeAlso     []

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

#ifndef __DAG_H__
#define __DAG_H__

#if HAVE_CONFIG_H
# include "config.h"
#endif


/* Standard includes. */
#if HAVE_MALLOC_H
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif  
# include <malloc.h>
#elif HAVE_SYS_MALLOC_H
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif  
# include <sys/malloc.h>
#elif HAVE_STDLIB_H
# include <stdlib.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

/* GLU library includes. */
#include "utils/utils.h"
#include "utils/list.h"
#include "util.h"
#include "st.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* Default parameters for the unique table. */
#define DAG_DEFAULT_VERTICES_NO    65537
#define DAG_DEFAULT_DENSITY           20
#define DAG_DEFAULT_GROWTH           1.5

/* Setting and clearing leftmost bit in 32-bit pointers. */
#define DAG_BIT_SET   (int) 0x80000000
#define DAG_BIT_CLEAR (int) 0x7FFFFFFF

/* Dag statistics. */
#define DAG_NODE_NO  (int)  0  /* How many nodes created (overall). */
#define DAG_GC_NO    (int)  1  /* How many nodes collected. */
#define DAG_MAX_STAT (int)  2

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef void  (*Dag_ProcPtr_t)();   /* Procedures. */
typedef int   (*Dag_IntPtr_t)();    /* Functions returning int. */

typedef struct DagManager       Dag_Manager_t;       /* The dag manager. */
typedef struct Dag_Vertex       Dag_Vertex_t;        /* The vertices. */
typedef struct Dag_DfsFunctions Dag_DfsFunctions_t;  /* Depth First Search. */

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************
  Synopsis      [DAG vertex.]
  Description   [The main fields (used for hashing) are:
                 <ul>
                 <li> symbol, an integer code
                 <li> data, a generic pointer (vertex annotation)
                 <li> outList, a list of sons ((lsList)NULL for leafs)
                 </ul>
                 Some fields are for internal purposes:
                 <ul>
                 <li> dag, a reference to the dag manager that owns the node
                 <li> mark, how many fathers (for garbage collection)
                 <li> visit, how many visits (for DFS)
                 <li> vBro, the vertex brother (a fatherless brother is rescued
                                                from GC by its non-orphan one)
                 <li> vHandle, back-reference to the free list
                 </ul>
                 The fields above should never be modified directly, unless
                 it is clear how they work! General purpose fields are:
                 <ul>
                 <li> gRef, a generic char pointer
                 <li> iRef, a generic integer value
                 </ul>
                 The dag manager makes no assumptions about the latter
                 fields and no efforts to ensure their integrity.
                 </ul>]
  SeeAlso       []
******************************************************************************/
struct Dag_Vertex {
  int             symbol;
  char          * data;
  lsList          outList;

  Dag_Manager_t * dag;
  int             mark;
  int             visit;
  lsHandle        vHandle;

  char          * gRef;
  int             iRef;
};


/**Struct**********************************************************************
  Synopsis      [DFS function struct.]
  Description   [The generic DFS functions:
                 <ul>
                 <li> Set, may force a different behaviour in the visit
                 <li> FirstVisit, invoked at the beginning of the visit
                 <li> BackVisit, invoked after each operand's visit
                 <li> LastVisit, invoked at the end of the visit
                 </ul>
                 All functions must be of the form:

                 <type> f(Dag_Vertex_t * v, char * d, int b)

                 where <type>=(int) in the case of `Set()' and <type>=(void)
                 in all the other cases. `v' is the current vertex, `d' is
                 a generic data reference, and `b' is set to the incoming edge
                 annotation (if any). DFS beahaves differently according to
                 the return value of `Set()': -1 forces visiting, 0 default
                 behaviour (all nodes visited once and only once), 1 forces
                 backtracking.]
  SeeAlso       []
******************************************************************************/
struct Dag_DfsFunctions {
  Dag_IntPtr_t   Set;
  Dag_ProcPtr_t  FirstVisit;
  Dag_ProcPtr_t  BackVisit;
  Dag_ProcPtr_t  LastVisit;
};


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/* A predefined DFS to clean user data in vertices. */
extern Dag_DfsFunctions_t dag_DfsClean;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro**********************************************************************
  Synopsis    [Filters a pointer from bit annotations.]
  Description [The leftmost bit is filtered to 0 by a bitwise-and with
               DAG_BIT_CLEAR == 0x7FFFFFFF mask. The result is the pointer
               purified from the bit annotation.]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
#define Dag_VertexGetRef(p)\
((Dag_Vertex_t*)((int)p & DAG_BIT_CLEAR))

/**Macro**********************************************************************
  Synopsis    [Sets (forces) a bit annotation to 1.]
  Description [The leftmost bit is forced to 1 by a bitwise-or with
               DAG_BIT_SET == 0x80000000 mask .]
  SideEffects [The value of p changes to the purified value.]
  SeeAlso     []
******************************************************************************/
#define Dag_VertexSet(p)\
(p = (Dag_Vertex_t*)((int)p | DAG_BIT_SET))

/**Macro**********************************************************************
  Synopsis    [Clears (forces) a bit annotation to 0.]
  Description [The leftmost bit is forced to 0 by a bitwise-and with
               DAG_BIT_CLEAR == 0x7FFFFFFF mask.]
  SideEffects [The value of p changes to the purified value.]
  SeeAlso     []
******************************************************************************/
#define Dag_VertexClear(p)\
(p = (Dag_Vertex_t*)((int)p & DAG_BIT_CLEAR))

/**Macro**********************************************************************
  Synopsis    [Tests if the edge is annotated.]
  Description [Uses a bitwise-and with DAG_BIT_SET == 0x80000000 to test the
               leftmost bit of p. The result is either 0x00000000 if the bit
               is not set, or 0x80000000 if the bit is set.]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
#define Dag_VertexIsSet(p)\
((int)p & DAG_BIT_SET)

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void Dag_Dfs(Dag_Vertex_t * dfsRoot, Dag_DfsFunctions_t * dfsFun, char * dfsData);
EXTERN Dag_Manager_t * Dag_ManagerAlloc();
EXTERN Dag_Manager_t * Dag_ManagerAllocWithParams(int dagInitVerticesNo, int maxDensity, int growthFactor);
EXTERN void Dag_ManagerFree(Dag_Manager_t * dagManager, Dag_ProcPtr_t freeData, Dag_ProcPtr_t freeGen);
EXTERN void Dag_ManagerGC(Dag_Manager_t * dagManager, Dag_ProcPtr_t freeData, Dag_ProcPtr_t freeGen);
EXTERN void Dag_PrintStats(Dag_Manager_t * dagManager, int clustSz, FILE * outFile);
EXTERN Dag_Vertex_t * Dag_VertexLookup(Dag_Manager_t * dagManager, int vSymb, char * vData, lsList vSons);
EXTERN Dag_Vertex_t * Dag_VertexInsert(Dag_Manager_t * dagManager, int vSymb, char * vData, lsList vSons);
EXTERN void Dag_VertexMark(Dag_Vertex_t * v);
EXTERN void Dag_VertexUnmark(Dag_Vertex_t * v);

/**AutomaticEnd***************************************************************/

EXTERN void PrintStat(Dag_Vertex_t* dfsRoot, FILE* statFile);
EXTERN Dag_Vertex_t* Dag_Ennarize (Dag_Vertex_t* dfsRoot);


#endif /* __DAG_H__ */
