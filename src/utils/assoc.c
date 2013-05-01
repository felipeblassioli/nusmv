/**CFile***********************************************************************

  FileName    [assoc.c]

  PackageName [util]

  Synopsis    [A simple associative list]

  Description [This file provides the user with a data structure that
  implemnts an associative list. If there is already an entry with
  the same ky in the table, than the value associated is replaced with
  the new one.]

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

#include <stdlib.h>
#include "util.h"
#include "utils/utils.h" /* for nusmv_assert */
#include "node/node.h"
#include "utils/assoc.h"

static char rcsid[] UTIL_UNUSED = "$Id: assoc.c,v 1.5.6.2 2003/12/01 14:35:56 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/*
  Initial size of the associative table.
*/
#define ASSOC_HASH_SIZE 127
/* 
   Largest everage number of entries per hash element before the table
  is grown.
*/
#define ASSOC_MAX_DENSITY  ST_DEFAULT_MAX_DENSITY

/*
  The factor the table is grown when it becames too full. 
*/
#define ASSOC_GROW_FACTOR  ST_DEFAULT_GROW_FACTOR

/*
  If is non-zero, then every time an entry is found, it is moved on 
  top of the chain. 
*/
#define ASSOC_REORDER_FLAG ST_DEFAULT_REORDER_FLAG

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int assoc_hash_fun ARGS((node_ptr, int));
static int assoc_eq_fun ARGS((node_ptr, node_ptr));
static int assoc_neq_fun ARGS((node_ptr, node_ptr));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

hash_ptr new_assoc()
{
  st_table * new_table = st_init_table_with_params((ST_PFICPCP)assoc_neq_fun,
                                                   (ST_PFICPI)assoc_hash_fun,
                                                   ASSOC_HASH_SIZE,
                                                   ASSOC_MAX_DENSITY,
                                                   ASSOC_GROW_FACTOR,
                                                   ASSOC_REORDER_FLAG);
  if (new_table == (st_table *)NULL) {
    fprintf(stderr, "new_assoc: Out of Memory\n");
    exit(1);
  }
  return((hash_ptr)new_table);
}

hash_ptr copy_assoc(hash_ptr hash)
{
  return (hash_ptr) st_copy((st_table *) hash);
}

node_ptr find_assoc(hash_ptr hash, node_ptr key)
{
  node_ptr data;

  if (st_lookup((st_table *)hash, (char *)key, (char **)&data)) return(data);
  else return(Nil);
}

void insert_assoc(hash_ptr hash, node_ptr key, node_ptr data)
{
  (void)st_insert((st_table *)hash, (char *)key, (char *)data);
}

void remove_assoc(hash_ptr hash, node_ptr key, node_ptr data)
{
  (void)st_delete((st_table *)hash, (char **)&key, (char **)&data);
}

/*
  Frees any internal storage associated with the hash table.
  It's user responsibility to free any storage associated with the
  pointers in the table.
*/
void free_assoc(hash_ptr hash)
{
  (void)st_free_table((st_table *)hash);
}

static enum st_retval delete_entry(char *key, char *data, char * arg)
{
  return(ST_DELETE);
}

void clear_assoc(hash_ptr hash){
  st_foreach(hash, delete_entry, NULL);
}

void clear_assoc_and_free_entries(hash_ptr hash, ST_PFSR fn){
  nusmv_assert(hash != NULL);
  st_foreach(hash, fn, NULL);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

static int assoc_hash_fun(node_ptr key, int size)
{ return((int)(key) % size); }

static int assoc_eq_fun(node_ptr a1, node_ptr a2)
{ return((a1) == (a2)); }

static int assoc_neq_fun(node_ptr a1, node_ptr a2)
{ return((a1) != (a2)); }


