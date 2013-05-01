/**CFile***********************************************************************

  FileName    [heap.c]

  PackageName [utils]

  Synopsis    [Heap related routines]

  Description []

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
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

#if HAVE_CONFIG_H
# include "config.h"
#endif

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
#endif

#if HAVE_STDLIB_H
# include <stdlib.h>
#endif 

#include <stdio.h>
#include <assert.h>

#define HEAP_MAXLENGTH_INIT 31

struct heap_el 
{
  float val;
  void * el;
};
  
struct heap_ 
{
  int maxlength;
  int length;
  struct heap_el * array;
};

typedef struct heap_ * heap;

heap heap_create()
{
  heap h = malloc(sizeof(struct heap_));
  assert(h);

  h->maxlength = HEAP_MAXLENGTH_INIT;
  h->length = 0;
  h->array = malloc(sizeof(struct heap_el) * h->maxlength);
  assert(h->array);

  return h;
}

void heap_destroy(heap h)
{
  assert(h);
  assert(h->length == 0);
  
  free(h->array);
  free(h);
}

static void heap_switch(heap h, int pos1, int pos2)
{
  float val = h->array[pos2].val;
  void * el = h->array[pos2].el;
  
  h->array[pos2].val = h->array[pos1].val;
  h->array[pos2].el  = h->array[pos1].el;

  h->array[pos1].val = val;
  h->array[pos1].el  = el;  
}

void heap_add(heap h, float val, void * el)
{
  assert(h);

  if (h->length == h->maxlength) {
    h->maxlength = h->maxlength * 2 + 1;
    h->array = realloc(h->array, sizeof(struct heap_el) * h->maxlength);
    assert(h->array);
  }

  {
    int pos = h->length;

    h->length++;

    h->array[pos].val = val; 
    h->array[pos].el = el; 

    while (pos > 0) {
      int newpos = (pos - 1) / 2;
      
      if (h->array[pos].val > h->array[newpos].val) {
	heap_switch(h,pos,newpos);
	pos = newpos;
      } else {
	break;
      }
    }
  }
}

int heap_isempty(heap h)
{
  assert(h);
  return (h->length == 0);
}

void * heap_getmax(heap h)
{
  void * el;

  assert(h);
  assert(h->length > 0);

  el = h->array[0].el;
  
  h->length--;

  if (h->length) {
    int pos = 0;

    h->array[0].val = h->array[h->length].val; 
    h->array[0].el  = h->array[h->length].el; 
      
    while (2 * pos + 1 < h->length) {
      
      int newpos = 2 * pos + 1;
      
      if ((h->array[pos].val < h->array[newpos].val) || 
	  (h->array[pos].val < h->array[newpos+1].val)) {
	if (h->array[newpos].val >= h->array[newpos+1].val) {
	  heap_switch(h,pos,newpos);
	  pos = newpos;
	} else {
	  heap_switch(h,pos,newpos+1);	  
	  pos = newpos+1;
	}
      } else {
	break;
      }
    }
  }
  return el;
}

#ifdef HEAP_TEST

int main(int argc, char * argv[])
{
  int length, num, c;
  int * array;
  heap h;
  if (argc != 3) {
    fprintf(stderr,"Usage: %s <heap-length> <num-tests>\n", argv[0]);
    return 1;
  }
  length = atoi(argv[1]);
  num = atoi(argv[2]);

  if (length <= 0) {
    fprintf(stderr,"Error: length <= 0\n");
    return 1;    
  }

  h = heap_create();
  
  array = malloc(sizeof(int) * length);
  assert(array);

  for (c = 1; c <= num; c++) {
    int i, i1, i2;
    printf("Test %2d:", c);
    for (i = 0; i < length; i++) {
      array[i] = i+1;
    }
    for (i = 0; i < 4 * length; i++) {
      i1 = utils_random() % length;
      i2 = utils_random() % length;
      if (i1 == i2) continue;
      array[i1] = array[i1] + array[i2];
      array[i2] = array[i1] - array[i2];
      array[i1] = array[i1] - array[i2];
    }
    for (i = 0; i < length; i++) {
      printf(" %d", array[i]);
      heap_add(h, -array[i], (void *)array[i]);
    }
    printf("\n------->");
    for (i = 0; i < length; i++) {
      int val = (int)heap_getmax(h);
      printf(" %d", val);
      assert(val == i+1);
    }
    printf("\n");
    assert(heap_isempty(h));
  }

  heap_destroy(h);

  return 0;
}

#endif
