/**CFile***********************************************************************

  FileName    [node.c]

  PackageName [node]

  Synopsis    [The main routines of the <tt>node</tt> package.]

  Description [This file provides an abstract data type a la s-expression in LISP.]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``node'' package of NuSMV version 2. 
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

#include "nodeInt.h" 

static char rcsid[] UTIL_UNUSED = "$Id: node.c,v 1.8.12.1 2004/06/17 15:15:36 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
/**Variable********************************************************************

  Synopsis    [The variable used to store the memory manager of the
  <tt>node</tt> package.]

  Description [The variable used to store the memory manager of the
  <tt>node</tt> package.
  We avoid declaring a global variable to store the node manager, and
  to pass the node_manager as an argument to all the node manipulation
  functions.]

******************************************************************************/
static node_mgr_ *node_mgr;

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static unsigned node_hash_fun ARGS((node_ptr node));
static unsigned node_eq_fun ARGS((node_ptr node1, node_ptr node2));
static node_ptr node_alloc ARGS((void));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the <tt>node</tt> manager.]

  Description        [The <tt>node</tt> manager is initialized.]

  SideEffects        [None]

******************************************************************************/
void node_init()
{
  node_mgr = (node_mgr_ *)ALLOC(node_mgr_, 1);
  if (node_mgr == (node_mgr_ *)NULL){
    /* Check the possibility to use the rpterr(...) function */
    fprintf(stderr, "node_init: Out of Memory in allocating the node manager\n");
    exit(1);
  }
  node_mgr->allocated  = 0;
  node_mgr->memused    = 0;
  node_mgr->nodelist   = (node_ptr *)0;
  node_mgr->memoryList = (node_ptr *)0;
  node_mgr->nextFree   = (node_ptr)0;

  node_mgr->nodelist = (node_ptr *)ALLOC(node_ptr, NODE_HASH_SIZE);
  if (node_mgr->nodelist == (node_ptr *)NULL) {
    /* Check the possibility to use the rpterr(...) function */
    fprintf(stderr, "node_init: Out of Memory in allocating the node hash\n");
    nusmv_exit(1);
  }
  { /* Initializes the node cache */
    int i;

    for(i = 0; i < NODE_HASH_SIZE; i++) node_mgr->nodelist[i] = (node_ptr)NULL;
  }
  node_mgr->subst_hash = new_assoc();
}

/**Function********************************************************************

  Synopsis           [Quits the <tt>node</tt> manager.]

  Description        [Quits the <tt>node</tt> manager. All the
  memory allocated it's freed.]

  SideEffects        [All the memory allocated by the <tt>node</tt>
  manager are left to the operating system.]

******************************************************************************/
void node_quit()
{
  /* Shut down the node manager */
  node_ptr * next;
  node_ptr * memlist = node_mgr->memoryList;
  
  while(memlist != NULL) {
    next = (node_ptr *) memlist[0];
    FREE(memlist);
    memlist = next;
  }
  node_mgr->nextFree = (node_ptr)NULL;
  node_mgr->memoryList = (node_ptr *)NULL;
  clear_assoc(node_mgr->subst_hash);
  node_mgr->subst_hash = (hash_ptr)NULL;
  FREE(node_mgr->nodelist);
  FREE(node_mgr);
  node_mgr = (node_mgr_ *)NULL;
}

/**Function********************************************************************

  Synopsis           [Free a node of the <tt>node<tt> manager.]

  Description        [Free a node of the <tt>node<tt> manager. The
  node is available for next node allocation.]

  SideEffects        [None]

******************************************************************************/
void free_node(node_ptr node)
{
  /* 
     Check whether the node is in the hash. If it is, it should not 
     be freed.
  */
  
  {
    node_ptr * nodelist;
    node_ptr looking;
    unsigned int pos;
    
    nodelist = node_mgr->nodelist;
    pos = node_hash_fun(node) % NODE_HASH_SIZE;
    looking = nodelist[pos];

    while(looking != (node_ptr)NULL) {
      if (node_eq_fun(node, looking)) return;
      looking = looking->link;
    }
  }

  /*
    The node is not in the hash, so it can be freed.
  */
  node->link = node_mgr->nextFree;
  node_mgr->nextFree = node;
}

/**Function********************************************************************

  Synopsis           [Creates a new node.]

  Description        [A new <tt>node</tt> of type <tt>type</tt> and
  left and right branch <tt>left<tt> and <tt>right</tt> respectively
  is created. The returned node is not stored in the <tt>node</tt> hash.]

  SideEffects        [None]

  SeeAlso            [find_node]

******************************************************************************/
node_ptr new_node(int type, node_ptr left, node_ptr right)
{
    extern int yylineno;
    node_ptr node;

    node = node_alloc();
    node -> type           = type;
    node -> lineno         = yylineno;
    node -> left.nodetype  = left;
    node -> right.nodetype = right;
    return node;
}

/**Function********************************************************************

  Synopsis           [Creates a new node.]

  Description        [A new <tt>node</tt> of type <tt>type</tt> and
  left and right branch <tt>left<tt> and <tt>right</tt> respectively
  is created. The returned node is stored in the <tt>node</tt> hash.]

  SideEffects        [The <tt>node</tt> hash is modified.]

  SeeAlso            [new_node]

******************************************************************************/
node_ptr find_node(int type, node_ptr left, node_ptr right)
{
    extern int yylineno;
    node_rec node;

    node.type = type;
    node.lineno = yylineno;
    node.left.nodetype = left;
    node.right.nodetype = right;
    return(insert_node(&node));
}

/**Function********************************************************************

  Synopsis           [Search the <tt>node</tt> hash for a given node.]

  Description        [Search the <tt>node</tt> hash for a given
  node. If the node is not <tt>Nil</tt>, and the node is not stored in
  the hash, the new node is created, stored in the hash and then returned.]

  SideEffects        [The node <tt>hash</tt> may change.]

  SeeAlso            [find_node]

******************************************************************************/
node_ptr find_atom(node_ptr a)
{
  if (a == Nil) return(a);
  return(find_node(a->type, a->left.nodetype, a->right.nodetype));
}

/**Function********************************************************************

  Synopsis           [Conses two nodes.]

  Description        [Conses two nodes.]

  SideEffects        [None]

  SeeAlso            [car cdr]

******************************************************************************/
node_ptr cons(node_ptr x, node_ptr y)
{ return(new_node(CONS,x,y)); }

/**Function********************************************************************

  Synopsis           [Returns the left branch of a node.]

  Description        [Returns the left branch of a node.]

  SideEffects        [None]

  SeeAlso            [cdr cons]

******************************************************************************/
node_ptr car(node_ptr x)
{ return(x->left.nodetype);}

/**Function********************************************************************

  Synopsis           [Returns the right branch of a node.]

  Description        [Returns the right branch of a node.]

  SideEffects        [None]

  SeeAlso            [car cons]

******************************************************************************/
node_ptr cdr(node_ptr x)
{ return(x->right.nodetype); }

/**Function********************************************************************

  Synopsis           [Replaces the car of X with Y]

  Description        [Replaces the car of X with Y]

  SideEffects        [The car of X is replaced by Y.]

  SeeAlso            [car cdr cons setcdr]

******************************************************************************/
void setcar(node_ptr x, node_ptr y) {
  x->left.nodetype = y;
}

/**Function********************************************************************

  Synopsis           [Replaces the cdr of X with Y]

  Description        [Replaces the cdr of X with Y]

  SideEffects        [The cdr of X is replaced by Y.]

  SeeAlso            [car cdr cons setcar]

******************************************************************************/
void setcdr(node_ptr x, node_ptr y) {
  x->right.nodetype = y;
}

/**Function********************************************************************

  Synopsis           [Checks if node element X occurs in list.]

  Description        [Checks if node element X occurs in list.]

  SideEffects        [None]

  SeeAlso            [node_subtract in_list]

******************************************************************************/
int memberp(node_ptr x, node_ptr list)
{
  if(list == Nil) return(0);
  if(car(list) == x) return(1);
  return(memberp(x,cdr(list)));
}

/**Function********************************************************************

  Synopsis           [Appends two lists and returns the result.]

  Description        [Constructs a new list by concatenating its arguments.]

  SideEffects        [The modified list is returned. Side effects on
  the returned list were performed. It is equivalent to the lisp NCONC]

******************************************************************************/
node_ptr append(node_ptr x, node_ptr y)
{
  if(x==Nil)return(y);
  x->right.nodetype = append(x->right.nodetype,y);
  return(x);
}


/**Function********************************************************************

  Synopsis           [Appends two lists and returns the result.]

  Description        [Constructs a new list by concatenating its arguments.]

  SideEffects        [The modified list is returned. No side effects on
  the returned list were performed.]

******************************************************************************/
node_ptr append_ns(node_ptr x, node_ptr y)
{
  if(x==Nil)return(y);
  return(cons(car(x), append_ns(cdr(x), y)));
}

/**Function********************************************************************

  Synopsis           [Applies FUN to successive cars of LISTs and
  returns the results as a list.]

  Description        [Applies FUN to successive cars of LISTs and
  returns the results as a list.]

  SideEffects        [None]

  SeeAlso            [walk]

******************************************************************************/
node_ptr map(NPFN fun, node_ptr l)
{
  node_ptr t;

  if (l == Nil) return(Nil);
  t = (node_ptr)(*fun)(car(l));
  return(cons(t,map(fun,cdr(l))));
}

/**Function********************************************************************

  Synopsis           [Applies FUN to successive cars of LISTs.]

  Description        [Applies FUN to successive cars of LISTs.]

  SideEffects        [None]

  SeeAlso            [map]

******************************************************************************/
void walk(VPFN fun, node_ptr l)
{
  if (l == Nil) return;
  (void)(*fun)(car(l));
  walk(fun,cdr(l));
}




/**Function********************************************************************

  Synopsis           [Reverse a list.]

  Description        [Returns a new sequence containing the same
  elements as X but in reverse order.]

  SideEffects        [None]

  SeeAlso            [last car cons append]

******************************************************************************/
node_ptr reverse(node_ptr x)
{
  node_ptr y=Nil;

  while(x){
    node_ptr z = x->right.nodetype;

    x->right.nodetype = y;
    y = x;
    x = z;
  }
  return(y);
}


/**Function********************************************************************

  Synopsis           [reverses the list with no side-effect

  Description        [Returns a reversed version of the given list]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
node_ptr reverse_ns(node_ptr l)
{
  node_ptr iter;
  node_ptr res = Nil;
  
  iter = l;
  while (iter != Nil) {
    res = cons(car(iter), res);
    iter = cdr(iter);
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the last cons in X.]

  Description        [Returns the last cons in X.]

  SideEffects        [None]

  SeeAlso            [car]

******************************************************************************/
node_ptr last(node_ptr x)
{
  if(!x)internal_error("last: x == Nil");
  if(!cdr(x))return(car(x));
  return(last(cdr(x)));
}


/**Function********************************************************************

  Synopsis           [Frees all the elements of the list.]

  Description        [Frees all the elements of the list for further use.]

  SideEffects        [None]

  SeeAlso            [car]

******************************************************************************/
void free_list(node_ptr l) {
  while(l != Nil) {
    node_ptr tmp = l;
    
    l = cdr(l);
    free_node(tmp);
  }
}

/**Function********************************************************************

  Synopsis           [Extracts odd elements of list L.]

  Description        [Extracts odd elements of list L.]

  SideEffects        [None]

  SeeAlso            [even_elements]

******************************************************************************/
node_ptr odd_elements(node_ptr l)
{
  if (l == Nil) return(Nil);
  return(cons(car(l),even_elements(cdr(l))));
}

/**Function********************************************************************

  Synopsis           [Extracts even elements of list L.]

  Description        [Extracts even elements of list L.]

  SideEffects        [None]

  SeeAlso            [odd_elements]

******************************************************************************/
node_ptr even_elements(node_ptr l)
{
  if(l == Nil)return(Nil);
  return(odd_elements(cdr(l)));
}

/**Function********************************************************************

  Synopsis           [Deletes from list set2 the elements of list set1.]

  Description        [Deletes elements of list set1 from list set2
  without doing side effect. The resulting list is returned.] 

  SideEffects        [None]

******************************************************************************/
node_ptr node_subtract(node_ptr set1, node_ptr set2)
{
  if (set2 == Nil) return(Nil);
  if (memberp(car(set2),set1) == 1) return(node_subtract(set1,cdr(set2)));
  return(cons(car(set2),node_subtract(set1,cdr(set2))));
}

/**Function********************************************************************

  Synopsis           [Swaps two nodes.]

  Description        [Swaps two nodes.]

  SideEffects        [The two nodes are swapped.]

******************************************************************************/
void swap_nodes(node_ptr *n1, node_ptr *n2)
{
  node_ptr temp = *n1;

  *n1 = *n2;
  *n2 = temp;
}


/**Function********************************************************************

  Synopsis           [Checks list R to see if it contains the element N.]

  Description        [Checks list R to see if it contains the element N.]

  SideEffects        [None]

  SeeAlso            [node_subtract]

******************************************************************************/
int in_list(node_ptr n, node_ptr r)
{
  while (r) {
    if (car(r) == n) return(1);
    r = cdr(r);
  }
  return(0);
}

/**Function********************************************************************

  Synopsis           [Returns the length of list r.]

  Description        [Returns the length of list r.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int llength(node_ptr r)
{
  int l = 0;

  while (r) {
    l++;
    r = cdr(r);
  }
  return(l);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Inserts a node in the <tt>node</tt> hash.]

  Description        [Checks if node is in the cache, if it is the
  case then the hashed value is returned, else a new one is created,
  stored in the hash and returned.]

  SideEffects        [None]

  SeeAlso            [find_node]

******************************************************************************/
node_ptr insert_node(node_ptr node)
{
  node_ptr * nodelist;
  node_ptr looking;
  unsigned int pos;
  
  nodelist = node_mgr->nodelist;
  pos = node_hash_fun(node) % NODE_HASH_SIZE;
  looking = nodelist[pos];

  while(looking != (node_ptr)NULL) {
    if (node_eq_fun(node, looking)) return(looking);
    looking = looking->link;
  }
  /*
    The node is not in the hash, it is created and then inserted in it.
  */
  looking = node_alloc();
  if (looking == (node_ptr)NULL) {
    fprintf(stderr, "insert_node: Out of Memory\n");
    return((node_ptr)NULL);
  }
  looking->type = node->type;
  looking->lineno = node->lineno;
  looking->left.inttype = node->left.inttype;
  looking->right.inttype = node->right.inttype;
  looking->link = nodelist[pos];
  nodelist[pos] = looking;
  return(looking);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Hash function for <tt>node</tt>s.]

  SideEffects        [None]

  SeeAlso            [node_eq_fun]

******************************************************************************/
static unsigned node_hash_fun(node_ptr node)
{
  return(((((unsigned)  node->type)     )  +
	 (((unsigned)  node->left.inttype) << 1)  +
	 (((unsigned) node->right.inttype) << 2)) % NODE_HASH_SIZE);
}

/**Function********************************************************************

  Synopsis           [Equality function for <tt>node</tt> hash.]

  SideEffects        [None]

  SeeAlso            [node_hash_fun]

******************************************************************************/
static unsigned node_eq_fun(node_ptr node1, node_ptr node2)
{
  return((node1->left.inttype == node2->left.inttype) &&
	 (node1->right.inttype == node2->right.inttype) &&
	 (node1->type == node2->type));
}


/**Function********************************************************************

  Synopsis           [Allocates NODE_MEM_CHUNK records and stores them
  in the free list of the <tt>node</tt> manager.]

  Description        [Allocates NODE_MEM_CHUNK records and stores them
  in the free list of the <tt>node</tt> manager.]

  SideEffects        [The free list of the <tt>node</tt> manager is
  updated by appending the new allocated nodes.]

******************************************************************************/
static node_ptr node_alloc() {
  int i;
  node_ptr node;
  
  if (node_mgr->nextFree == (node_ptr)NULL) { /* memory is full */
    node_ptr list;
    node_ptr * mem = (node_ptr *)ALLOC(node_rec, NODE_MEM_CHUNK + 1);
    
    if (mem == (node_ptr *)NULL) { /* out of memory */
      fprintf(stderr, "node_alloc: out of memory\n");
      fprintf(stderr, "Memory in use for nodes = %ld\n", node_mgr->memused);
      return((node_ptr)NULL);
    }
    else { /* Adjust manager data structure */
      node_mgr->memused += (NODE_MEM_CHUNK + 1)* sizeof(node_rec);
      mem[0] = (node_ptr)node_mgr->memoryList;
      node_mgr->memoryList = mem;

      list = (node_ptr)mem;
      /* Link the new set of allocated node together */
      i = 1;
      do {
        list[i].link = &list[i+1];
      } while (++i < NODE_MEM_CHUNK);
      list[NODE_MEM_CHUNK].link = (node_ptr)NULL;

      node_mgr->nextFree = &list[1];
    }
  }
  /* Now the list of nextFree is not empty */
  node_mgr->allocated++;
  node = node_mgr->nextFree; /* Takes the first free available node */
  node_mgr->nextFree = node->link;
  node->link = (node_ptr)NULL;
  return(node);
}

/**Function********************************************************************

  Synopsis           [Returns a new empty list]

  Description        []

  SideEffects        [None]

******************************************************************************/
node_ptr new_list()  { return Nil; }


/**Function********************************************************************

  Synopsis           [Returns 1 is the list is empty, 0 otherwise]

  Description        []

  SideEffects        [None]

******************************************************************************/
int is_list_empty(node_ptr list) { return list == Nil; }


