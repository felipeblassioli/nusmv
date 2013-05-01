/**CHeaderFile*****************************************************************

  FileName    [simVector.h]

  PackageName [sim]

  Synopsis    [Vector handling]

  Description [Macros that realize a C++/STL like vector:
  		<ul>
		<li> <b>VdeclareShort()</b> 2^bitsof(short) elements
		<li> <b>VdeclareLong()</b> 2^bitsof(long) elements
		<li> <b>Vdeclare()</b> 2^bitsof(int) elements
		<li> <b>V()</b> get the vector
		<li> <b>Vref()</b> get a reference to an element
		<li> <b>Vsize()</b> get the size
		<li> <b>Vcapacity()</b> get the capacity
		<li> <b>Vinit()</b> creates an empty vector
		<li> <b>VinitResize()</b> elements are initialized
		<li> <b>VinitReserve()</b> elements are not initialized
		<li> <b>Vclear()</b> deallocates the vector
		<li> <b>Vresize()</b> shrink or enlarge
		<li> <b>Vflush()</b> equivalent to Vresize(0) but faster
		<li> <b>Vreserve()</b> space available without reallocations
		<li> <b>VpushBack()</b> stack push (with reallocation)
		<li> <b>VforceBack()</b> stack push (unguarded!!!)
		<li> <b>VpushBackGrow()</b> stack push (controlled realloc.)
		<li> <b>VpopBack()</b> stack pop (decrease size with check)
		<li> <b>VextractBack()</b> stack pop (get element)
		<li> <b>VdropBack()</b> stack pop (decrease size)
		<li> <b>Vback()</b> same as V()[Vsize() - 1] 
		<li> <b>Vempty()</b> same as (Vsize() == 0)
		</ul>]

  SeeAlso     []

  Author      [Armando Tacchella]

  Copyright   [
  This file is part of the ``sim'' package of NuSMV version 2. 
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

#include <string.h>


#ifndef _SIMVECTOR
#define _SIMVECTOR


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************
  Synopsis     [Vector with 2^bitsof(short) - 1 elements at most.]
  Description  [Declares a pointer to `type', the size and the capacity; 
                variable names are obtained by chaining `name' and the
		suffixes Vec, Size and Capacity respectively.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VdeclareShort(type, name)		\
type * name##Vec;				\
short  name##Size;				\
short  name##Capacity

/**Macro***********************************************************************
  Synopsis     [Vector with 2^bitsof(long) - 1 elements at most.]
  Description  [Declares a pointer to `type', the size and the capacity: 
                variable names are obtained by chaining `name' and the
		suffixes Vec, Size and Capacity respectively.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VdeclareLong(type, name)		\
type * name##Vec;				\
long   name##Size;				\
long   name##Capacity

/**Macro***********************************************************************
  Synopsis     [Vector with 2^bitsof(int) - 1 elements at most.]
  Description  [Declares a pointer to `type', the size and the capacity: 
                variable names are obtained by chaining `name' and the
		suffixes Vec, Size and Capacity respectively.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define Vdeclare(type, name)			\
type * name##Vec;				\
int    name##Size;				\
int    name##Capacity

/**Macro***********************************************************************
  Synopsis     [Vector with 2^bitsof(short) - 1 elements at most.]
  Description  [Declares a pointer to `type', the size and the capacity; 
                variable names are obtained by chaining `name' and the
		suffixes Vec, Size and Capacity respectively.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VdeclareShortExt(type, name)		\
extern type * name##Vec;			\
extern short  name##Size;			\
extern short  name##Capacity

/**Macro***********************************************************************
  Synopsis     [Vector with 2^bitsof(long) - 1 elements at most.]
  Description  [Declares a pointer to `type', the size and the capacity: 
                variable names are obtained by chaining `name' and the
		suffixes Vec, Size and Capacity respectively.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VdeclareLongExt(type, name)		\
extern type * name##Vec;			\
extern long   name##Size;			\
extern long   name##Capacity

/**Macro***********************************************************************
  Synopsis     [Vector with 2^bitsof(int) - 1 elements at most.]
  Description  [Declares a pointer to `type', the size and the capacity: 
                variable names are obtained by chaining `name' and the
		suffixes Vec, Size and Capacity respectively.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VdeclareExt(type, name)			\
extern type * name##Vec;			\
extern int    name##Size;			\
extern int    name##Capacity

/**Macro***********************************************************************
  Synopsis     [Getting the vector data.]
  Description  [The vector data is obtained by chaining `name' and the
		suffixes Vec, Size and Capacity respectively.] 
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define V(name)	        name##Vec
#define Vref(name, i)   (name##Vec + i)
#define Vsize(name)     name##Size
#define Vcapacity(name) name##Capacity

/**Macro***********************************************************************
  Synopsis     [Initializing a vector.]
  Description  [Sets the vector to 0, size and capacity  to 0.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define Vinit(name)				\
name##Vec = 0;					\
name##Size = name##Capacity = 0

/**Macro***********************************************************************
  Synopsis     [Initializing a vector with a given size.]
  Description  [Sets the base pointer to a vector of `size' elements, 
                initialized to 0; sets size and capacity to `size'.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VinitResize(name, size)			\
name##Vec = calloc(size, sizeof(* name##Vec));	\
name##Size = name##Capacity = size

/**Macro***********************************************************************
  Synopsis     [Initializing a vector with a given capacity.]
  Description  [Sets the base pointer to a vector that, potentially, holds
                `capacity' elements; sets size to 0.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VinitReserve(name, capacity)			\
name##Vec = malloc((capacity) * sizeof(* name##Vec));	\
name##Size = 0;						\
name##Capacity = capacity

/**Macro***********************************************************************
  Synopsis     [Initializing a 0-term. vector with a given capacity.]
  Description  [Sets the base pointer to a vector that, potentially, holds
                `capacity' elements; sets size to 0.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VinitReserve0(name, capacity)				\
name##Vec = malloc((capacity + 1) * sizeof(* name##Vec));	\
* name##Vec = 0;						\
name##Size = 0;							\
name##Capacity = capacity

/**Macro***********************************************************************
  Synopsis     [Clearing a vector.]
  Description  [Frees a vector, sets it to 0, size and capacity  to 0.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define Vclear(name)				\
free(name##Vec);				\
name##Vec = 0;				\
name##Size = name##Capacity = 0

/**Macro***********************************************************************
  Synopsis     [Resizing a vector.]
  Description  [Checks if the requested `size' is greater than the current
                capacity: if so rellocates the vector and sets the current
		capacity to `size'; then checks if `size' is greater than the
		current size: if so initializes the new elements to 0;
		finally, sets the current size to `size'; elements exceeding 
		the new current size are not (re)initialized.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define Vresize(name, size)					\
if (size > (name##Capacity)) {					\
  name##Vec = realloc(name##Vec, (size) * sizeof(* name##Vec));	\
  name##Capacity = size;					\
}								\
if (size > (name##Size)) {					\
  memset(name##Vec + name##Size, 0, 				\
	 ((size) - name##Size) * sizeof(* name##Vec)); 		\
}								\
name##Size = size

/**Macro***********************************************************************
  Synopsis     [Resizing a vector to 0.]
  Description  [Sets the current size to 0 without performing any check:
                behaves as Vresize(name, 0) but saves time; elements 
		in the vector are not (re)initialized.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define Vflush(name) name##Size = 0

/**Macro***********************************************************************
  Synopsis     [Reserving space for a vector.]
  Description  [Rellocates the vector and sets the current capacity 
                to `capacity'; checks if the new capacity is smaller
		than the current size: if so, shrinks the size; when the
		capacity is increased, no initialization occurs.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define Vreserve(name, capacity)					\
name##Vec = realloc(name##Vec, (capacity) * sizeof(* name##Vec));	\
name##Capacity = capacity;						\
if (capacity < name##Size) {						\
  name##Size = capacity;						\
}

/**Macro***********************************************************************
  Synopsis     [Pushing a new element in the vector.]
  Description  [Checks if adding the element would cause an overflow: if
                so reallocates the vector increasing its capacity by one; 
		stores `elem' in the last position and increases the size.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VpushBack(name, elem)						\
if (name##Size == name##Capacity) {					\
  name##Capacity += 1;							\
  name##Vec = realloc(name##Vec, name##Capacity * sizeof(* name##Vec));	\
}									\
name##Vec[name##Size++] = elem

/**Macro***********************************************************************
  Synopsis     [Pushing a new element in a 0-terminated vector.]
  Description  [Checks if adding the element would cause an overflow: if
                so reallocates the vector increasing its capacity by one; 
		stores `elem' in the last position and increases the size.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VpushBack0(name, elem)						 \
if (name##Size == name##Capacity) {					 \
  name##Capacity += 1;							 \
  name##Vec = 								 \
    realloc(name##Vec, (name##Capacity + 1) * sizeof(* name##Vec));	 \
}									 \
name##Vec[name##Size++] = elem;						 \
name##Vec[name##Size] = 0
  
/**Macro***********************************************************************
  Synopsis     [Forces a new element in the vector without checks.]
  Description  [Stores `elem' in the last position and increases the size: 
                no overflow checking occurs.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VforceBack(name, elem) name##Vec[name##Size++] = elem

/**Macro***********************************************************************
  Synopsis     [Forces a new element in the vector (0 term.).]
  Description  [Stores `elem' in the last position and increases the size: 
                no overflow checking occurs.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VforceBack0(name, elem) 		\
name##Vec[name##Size++] = elem;			\
name##Vec[name##Size] = 0

/**Macro***********************************************************************
  Synopsis     [Pushing a new element in the vector with a growth factor.]
  Description  [Checks if adding the element would cause an overflow: if
                so reallocates the vector increasing its capacity of a
		factor determined by `grow'; stores `elem' in the last 
		position and increases the size.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VpushBackGrow(name, elem, grow)					\
if (name##Size == name##Capacity) {					\
  name##Capacity = (float)name##Capacity * (float)(grow);		\
  name##Vec = realloc(name##Vec, name##Capacity * sizeof(* name##Vec));	\
}									\
name##Vec[name##Size++] = elem

/**Macro***********************************************************************
  Synopsis     [Pushing a new element in the vector with a growth factor.]
  Description  [Checks if adding the element would cause an overflow: if
                so reallocates the vector increasing its capacity of a
		factor determined by `grow'; stores `elem' in the last 
		position and increases the size.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VpushBackGrow0(name, elem, grow)				      \
if (name##Size == name##Capacity) {					      \
  name##Capacity = (float)name##Capacity * (float)(grow);		      \
  name##Vec = realloc(name##Vec, (name##Capacity + 1) * sizeof(* name##Vec)); \
}									      \
name##Vec[name##Size++] = elem;						      \
name##Vec[name##Size] = 0

/**Macro***********************************************************************
  Synopsis     [Decreases the current size by one (funny as in STL vectors).]
  Description  [Checks if the current size is greater than 0: if so
                decreases it by one.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VpopBack(name)				\
if (name##Size > 0) {				\
  name##Size -= 1;				\
}

/**Macro***********************************************************************
  Synopsis     [Decreases the current size by one (0 term. vector).]
  Description  [Checks if the current size is greater than 0: if so
                decreases it by one (preserves 0 termination).]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VpopBack0(name)					\
if (name##Size > 0) {					\
  name##Size -= 1;					\
  name##Vec[name##Size] = 0;				\
}

/**Macro***********************************************************************
  Synopsis     [Extracts the element from the back without checks.]
  Description  [Decreases the size and gets the last element: no 
                underflow checking occurs.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VextractBack(name) name##Vec[--(name##Size)]

/**Macro***********************************************************************
  Synopsis     [Decreases the current size by one.]
  Description  [Decreases the current size by one.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VdropBack(name)	name##Size -= 1

/**Macro***********************************************************************
  Synopsis     [Decreases the current size by one (0 term. vector).]
  Description  [Decreases the current size by one (0 term. vector).]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VdropBack0(name)			\
name##Size -= 1;				\
name##Vec[name##Size] = 0;


/**Macro***********************************************************************
  Synopsis     [Gets the last element.]
  Description  [Simply returns V()[Vsize() - 1].]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define Vback(name) name##Vec[name##Size - 1]

/**Macro***********************************************************************
  Synopsis     [Gets the last element.]
  Description  [Simply returns V()[Vsize() - 1].]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define Vempty(name) name##Size == 0

/**Macro***********************************************************************
  Synopsis     [Iterates through the elements of a 0-terminated scalar vector.]
  Description  [This function works *only* for 0-terminated scalar vectors:
                an attempt to use it on a non 0-terminated vector will
		result in the program going berserk; attempts to use the
		construct with non-scalars will result in a compiler error.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define Vforeach0(name, gen, elem)				\
for (gen = name##Vec, elem = *gen; elem != 0; elem = *(++gen))   

/**Macro***********************************************************************
  Synopsis     [Reverse iterates through elements of a 0-initialized vector.]
  Description  [This function works *only* for 0-initialized scalar vectors:
                an attempt to use it on a non 0-initialized vector will
		result in the program going berserk; attempts to use the
		construct with non-scalars will result in a compiler error.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VforeachRev0(name, gen, elem)			\
for (gen = name##Vec + name##Size - 1, elem = *gen; 	\
       elem != 0; elem = *(--gen))   

/**Macro***********************************************************************
  Synopsis     [Iterates through the elements of a 0-terminated scalar vector.]
  Description  [This function works *only* for 0-terminated scalar vectors:
                an attempt to use it on a non 0-terminated vector will
		result in the program going berserk; attempts to use the
		construct with non-scalars will result in a compiler error.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
#define VforeachGen0(gen, elem)				\
for (elem = *gen; elem != 0; elem = *(++gen))   

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/

#endif /* _SIMVECTOR */


