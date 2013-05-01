/**CFile***********************************************************************

  FileName    [compileUtil.c]

  PackageName [compile]

  Synopsis    [Routines for model computation.]

  Description [This file contains the code for the compilation of the
  flattened hierarchy into BDD:
  <ul>
  <li> Creation of the boolean variables.</li>
  <li> Creation of the BDD representing the inertia of the system when
       there are processes. In fact when a process is running the
       other processes are stopped, and their state variables don't
       change.</li>
  <li> Creation of the BDD representing what does not change in the
       system, i.e. the set of invariance. These are introduced in the
       model by the keyword "<tt>INVAR</tt>" or by the <em>normal
       assignments</em> (i.e. "<tt>ASSIGN x : = y & z;</tt>"). These
       states are not stored in the transition relation, they are
       stored in an a doc variable.
  <li> Creation of the BDD representing the set of initial states.
  <li> Creation of the BDD representing the transition relation. 
       Various ways of representing the transition relation are offered
       the users.
       <ul>
       <li> <em>Monolithic</em>: the monolithic transition relation is
            computed.</li>
       <li> <em>Conjunctive Partioned (Threshold)</em>: the transition 
            relation is stored as an implicitly conjoined list of 
            transition relation. This kind of partitioning can be 
            used only if the model considered is a synchronous one.</li>
       <li> <em>Conjunctive Partioned IWLS95</em>: as the above, but the
            heuristic proposed in \[1\] is used to order partition clusters. </li>
       </ul>
  <li> Computes the fairness constraints. I.e. each fairness constraint
       (which can be a CTL formula) is evaluated and the resulting BDD
       is stored in the list <tt>fairness_constraints_bdd</tt> to be
       then used in the model checking phase.
  </ul>
  \[1\] R. K. Ranjan and A. Aziz and B. Plessier and C. Pixley and R. K. Brayton,
      "Efficient BDD Algorithms for FSM Synthesis and Verification,
      IEEE/ACM Proceedings International Workshop on Logic Synthesis,
      Lake Tahoe (NV), May 1995.</li>
  ]

  SeeAlso     []

  Author      [Marco Roveri]

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
#include "compile/compile.h"
#include "compileInt.h" 
#include "parser/symbols.h"
#include "utils/ustring.h"
#include "utils/error.h"


static char rcsid[] UTIL_UNUSED = "$Id: compileUtil.c,v 1.29.4.12 2004/05/07 13:04:46 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef enum {
  State_Instantiation_Mode,
  Input_Instantiation_Mode
} Instantiation_Vars_Mode_Type;



/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [The NuSMV atom <tt>running</tt>.]

  Description [The internal representation of the NuSMV atom <tt>running</tt>.]

  SeeAlso     []

******************************************************************************/
node_ptr running_atom; 
 
/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Builds an internal representation for a given string.]

  Description        [Builds an internal representation for a given
  string. If the conversion has been performed in the past, then the
  hashed value is returned back, else a new one is created, hashed and
  returned. We hash this in order to allow the following:
  <pre>
  VAR
     x : {a1, a2, a3};
     y : {a3, a4, a5};

  ASSIGN
     next(x) := case
                 x = y    : a2;
                 !(x = y) : a1;
                 1        : a3;
                esac;
  </pre>
  i.e. to allow the equality test between x and y. This can be
  performed because we internally have a unique representation of the
  atom <tt>a3</tt>.]

  SideEffects        []

  SeeAlso            [find_atom]

******************************************************************************/
node_ptr sym_intern(char *s)
{
  return(find_node(ATOM, (node_ptr)find_string(s), Nil));
}




/**Function********************************************************************

  Synopsis           [Initializes the build model.]

  Description [Checks correctness of the NuSMV progrma if not yet
  checked. Initializes the build model if not yet initialized.]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void Compile_InitializeBuildModel(void) {
  if (cmp_struct_get_compile_check(cmps) == 0) {
    Compile_CheckProgram(cmp_struct_get_procs(cmps),
                         cmp_struct_get_spec(cmps),
                         cmp_struct_get_ltlspec(cmps),
                         cmp_struct_get_invar_spec(cmps),
                         cmp_struct_get_justice(cmps),
                         cmp_struct_get_compassion(cmps));
    /* We keep track that checks have been performed */
    cmp_struct_set_compile_check(cmps);
  }

  if (cmp_struct_get_build_model_setup(cmps) == 0) {

    /* We keep track that construction has been performed */
    cmp_struct_set_build_model_setup(cmps);
  }
  return;
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

