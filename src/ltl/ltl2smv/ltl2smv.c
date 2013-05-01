/**CFile***********************************************************************

  FileName    [ltl2smv.c]

  PackageName [ltl2smv]

  Synopsis    [Functions performing convertion of LTL formula to
  SMV module]

  Description [see the header of file ltl2smv.h]

  SeeAlso     [mc]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``ltl2smv'' package of NuSMV version 2. 
  Copyright (C) 1998-2004 by CMU and ITC-irst. 

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

#include "ltl2smv.h"
#include "ltl2smvInt.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

static char rcsid[] UTIL_UNUSED = "$Id: ltl2smv.c,v 1.1.2.3 2004/11/19 14:53:47 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

struct node_struct {
  char* str;
  char op;
  char* el;
  char* pel;
  int flag; 
  struct node_struct* left;
  struct node_struct* right;
};

typedef struct str_list_struct{
  char* name;
  char* str;
  struct str_list_struct* next;
} str_list;

typedef struct output_list_struct{
  char* key;
  char* item;
  struct output_list_struct* next;
} output_list;

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* MACRO declaration                                                         */
/*---------------------------------------------------------------------------*/
/* To avoid the creation of too long item the parser cannot handle */
#undef ENABLE_COMMENTS_PRINT_OUT 

#define BWIDTH 1024
#define STRHASHSIZE 512
#define PRE_PREFIX "LTL_"
#define PREFIXNAME "_SPECF_"
#define MODULE_NAME "ltl_spec_"

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

extern FILE*  ltl2smv_yyin;

node* ltl2smv_Specf;

static int line_number = 0;
static str_list** Str_hash = NULL;
static output_list * Define_list = NULL;
static output_list * Next_list = NULL;
static output_list * Fairness_list = NULL;
static output_list * Initially_list = NULL;
static output_list * Var_list = NULL;
static int Spec_Counter = 0;
static int ccounter = 0;

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int get_digits ARGS((int n));
static char *Mymalloc ARGS((unsigned size));
static output_list *mymalloc_output_list ARGS((void));
static void print_node ARGS((FILE * ofile, node * t));
static void print_formula ARGS((FILE * ofile, node *t));
static node * expand_case_body ARGS((node * body));
static node *expand_formula ARGS((node *t));
static char *op_string ARGS((char *op, char *str1, char *str2));
static char* gen_name_number ARGS((int n));
static int str_hash_func ARGS((char *s));
static str_list *mymalloc_str_list ARGS((void));
static void add_var_list ARGS((char *key, char *item));
static char *reg_var ARGS((char *str));
static char *formula_string ARGS((node *t));
static char *gen_next_term ARGS((char *s));
static output_list *search_list ARGS((output_list *list, char *s));
static int define_check ARGS((char *s));
static void add_define_list ARGS((char *lit, char *s));
static int next_check ARGS((char *s));
static void add_next_list ARGS((char *lit, char *s));
static int fairness_check ARGS((char *s));
static void add_fairness_list ARGS((char *lit, char *s));
static int initially_check ARGS((char *s));
static void add_initially_list ARGS((char *lit, char *s));
static char *get_plit ARGS((node *t));
static void set_node ARGS((node *t));
static void init_str_hash ARGS((void));
static void quit_str_hash ARGS((void));
static void init_all_lists ARGS((void));
static void quit_all_lists ARGS((void));
static void remove_outer_par ARGS((char *s));
static void print_smv_format ARGS((FILE * ofile));
static void set_information ARGS((node *t));
static node *reduce_formula ARGS((node *t));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [The main routine converting an LTL formula to a fragment of an 
  SMV program.]

  Description [ the parameters are:
  counter - a number,  which is converted to the string representation 
            and then used as a part of the generated SMV models name.
            (_LTL#_SPECF_N_)
  inFile  - the file from which the LTL Formula to be translated is read.
  outFile - the file in which the SMV code corresponding to the tableau
            of LTL Formula is written in. If NULL, then stdout is used.
  ]

  SideEffects [creates a new file]

  SeeAlso     []

******************************************************************************/
void ltl2smv(int counter, const char* inFile, const char* outFile)
{
  FILE* input_file;
  FILE* output_file;

  Spec_Counter = counter;
  ccounter = 0;

  line_number = 0;
  Str_hash = NULL;

  init_all_lists();

  if ((char*)NULL == inFile) {
    fprintf(stderr, "Error: ltl2smv : the input file is not specified.\n");
    exit(1);
    
  }

  if (Spec_Counter < 0) {
    (void) fprintf(stderr, "Error: \"%d\" is not a natural number.\n",
		   Spec_Counter);
    exit(1);
  }

  /* opening the input file */
  input_file = fopen(inFile, "r");
  if (input_file == (FILE*)NULL) {
    (void) fprintf(stderr, "Error: Unable to open file \"%s\" for reading.\n",
		   inFile);
    exit(1);
  } else {
    ltl2smv_yyin = input_file;
  }

  if (ltl2smv_yyparse()) {
    (void) fclose(input_file);
    (void) fprintf(stderr, "Error: Parsing error\n");
    exit(1);
  }

  if (NULL != outFile) {
    output_file = fopen(outFile, "w");
    if (output_file == (FILE *)NULL) {
      (void) fclose(input_file);
      (void) fprintf(stderr, "Error: Unable to open file \"%s\" for writing.\n",
		     outFile);
      exit(1);
    }
  } else {
    output_file = stdout;
  }


  /* BEGIN COMMENT */
#ifdef ENABLE_COMMENTS_PRINT_OUT
  fprintf(output_file, "-- LTLSPEC ");
  print_formula(output_file, ltl2smv_Specf);
  fprintf(output_file, "\n");
#endif
  /* END COMMENT */

  ltl2smv_Specf = expand_formula(ltl2smv_Specf);

  /* BEGIN COMMENT */
#ifdef ENABLE_COMMENTS_PRINT_OUT
  fprintf(output_file, "-- (expanded) ");
  print_formula(output_file, ltl2smv_Specf);
  fprintf(output_file, "\n");
#endif
  /* END COMMENT */

  ltl2smv_Specf = reduce_formula(ltl2smv_Specf);

  /* BEGIN COMMENT */
#ifdef ENABLE_COMMENTS_PRINT_OUT
  fprintf(output_file, "-- (reduced) ");
  print_formula(output_file, ltl2smv_Specf);
  fprintf(output_file, "\n");
#endif
  /* END COMMENT */


  set_information(ltl2smv_Specf);
  print_smv_format(output_file);

  fclose(input_file);
  if (NULL != outFile) fclose(output_file);

  quit_str_hash();
  quit_all_lists();

  return ;
};



int ltl2smv_get_linenumber(void)
{
  return line_number;
}


void ltl2smv_inc_linenumber(void)
{
  ++line_number;
}

node *ltl2smv_gen_node(char op, node *left, node *right)
{
  node *ret;
  ret = ltl2smv_mymalloc_node();
  ret->op = op;
  ret->left = left;
  ret->right = right;
  return ret;
}

char *ltl2smv_mycpy(char *text)
{
  char *cptr;
  cptr = (char *) Mymalloc(strlen(text) + 1);
  strcpy(cptr, text);
  return(cptr);
}

node* ltl2smv_gen_leaf(char *str)
{
  node *ret;
  ret = ltl2smv_mymalloc_node();
  ret->op = 'l';
  ret->str = ltl2smv_mystrconcat3("(", str, ")");
  return (ret);
}


node* ltl2smv_mymalloc_node(void)
{
  node *ret;
  ret = (node*) Mymalloc(sizeof(node));
  ret->str = (char*) NULL;
  ret->op = (char) NULL;
  ret->el = (char*) NULL;
  ret->pel = (char*) NULL;
  ret->flag = 0;
  ret->left = (node*) NULL;
  ret->right = (node*) NULL;
  return(ret);
}




/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/* returns the number of digits of n */
static int get_digits(int n)
{
  int d;
  for (d = 1; n != 0; ++d)  n = n/10;
  return d;
}

static char *Mymalloc(unsigned size)
{
  char *ret;

  ret = malloc(size);
  if (ret == NULL) {
    fprintf(stderr, "malloc returns NULL\n");
    exit(1);
  }
  return(ret);
}


static output_list *mymalloc_output_list(void)
{
  output_list *ret;
  ret = (output_list *) Mymalloc(sizeof(output_list));
  ret->key = NULL;
  ret->item = NULL;
  ret->next = NULL;
  return (ret);
}



static void print_node(FILE * ofile, node * t)
{
  fprintf(ofile, " ----\n");
  fprintf(ofile, " pointer = %d\n", (int) t);
  if (t->op == 'l')
    fprintf(ofile, " leaf %s\n", t->str);
  else if (t->op != (char)NULL){
    fprintf(ofile, " op = %c\n", t->op);
    fprintf(ofile, " left pointer = %d\n", (int) t->left);
    fprintf(ofile, " right pointer = %d\n", (int) t->right);
    print_node(ofile, t->left);
    print_node(ofile, t->right);
  }
  else {
    fprintf(stderr, "t->op == NULL in print_tree\n");
    exit(1);
  }
  return;
}

static void print_formula(FILE * ofile, node *t)
{
  fprintf(ofile, "(");
  if (t->op == 'l')
    fprintf(ofile, "%s", t->str);
  else if (t->op != (char)NULL){
    if (t->right == NULL) {
      fprintf(ofile, "%c", t->op);
      print_formula(ofile, t->left);
    }
    else {
      print_formula(ofile, t->left);
      fprintf(ofile, "%c", t->op);
      print_formula(ofile, t->right);
    }
  }
  else {
    fprintf(stderr, "t->op == NULL in print_formula\n");
    exit(1);
  }
  fprintf(ofile, ")");
  return;
}


static node * expand_case_body(node * body)
{
  assert(body != NULL);
  assert((body->op == 'c') || (body->op == 'l'));

  if (body->op == 'c') {
    node * res;
    node * cond      = (body->left)->left;
    node * cond_res  = (body->left)->right;
    node * case_rest = expand_case_body(body->right);

    assert((body->left)->op == ':');
    res = ltl2smv_gen_node('|', ltl2smv_gen_node('&', cond, cond_res),
                        ltl2smv_gen_node('&', ltl2smv_gen_node('!', cond, NULL), case_rest));
    return(res);
  }
  else {
    return(body);
  }
}
    

static node *expand_formula(node *t)
{
  node *ret;
  node *body;
  node *or, *or1, *or2, *not, *not1, *not2, *not3, *not4, *tmp;

  if (t == NULL) return (NULL);

  switch (t->op) {
  case '|':
  case '!':
  case 'X':
  case 'U':
  case 'Y':
  case 'S':
    ret = t;
    ret->left = expand_formula(t->left);
    ret->right = expand_formula(t->right);
    break;
  case 'l':
    ret = t;
    break;
  case 'c': /* it is case expression */
    body = expand_case_body(t->left);
    ret = expand_formula(body);
    break;
  case '&':
    not1 = ltl2smv_gen_node('!',expand_formula(t->left),NULL);
    not2 = ltl2smv_gen_node('!',expand_formula(t->right),NULL);
    or  = ltl2smv_gen_node('|',not1,not2);
    ret = ltl2smv_gen_node('!',or,NULL);
    break;
  case '>':
    not = ltl2smv_gen_node('!',expand_formula(t->left),NULL);
    ret = ltl2smv_gen_node('|',not,expand_formula(t->right));
    break;
  case '=':
    /* a = b <-> (!(!a | !b) | !(a | b)) */
    not2 = ltl2smv_gen_node('!',expand_formula(t->left),NULL);
    not3 = ltl2smv_gen_node('!',expand_formula(t->right),NULL);
    or1 = ltl2smv_gen_node('|', not2, not3);
    not1 = ltl2smv_gen_node('!',or1, NULL);
    /* not{2,3}->left is the already exapnded version of the left
       right part of t */
    or2 = ltl2smv_gen_node('|', not2->left, not3->left);
    not4 = ltl2smv_gen_node('!', or2, NULL);
    ret = ltl2smv_gen_node('|',not1, not4);
    break;
  case 'x': /* xor */
    /* a xor b <-> (!(!a | b) | !(a | !b)) */
    not1 = ltl2smv_gen_node('!', expand_formula(t->left), NULL);
    not2 = ltl2smv_gen_node('!', expand_formula(t->right), NULL);
    /* not{1,2}->left is the already exapnded version of the left
       right part of t */
    or1 = ltl2smv_gen_node('|', not1, not2->left);
    or2 = ltl2smv_gen_node('|', not1->left, not2);
    not3 = ltl2smv_gen_node('!', or1, NULL);
    not4 = ltl2smv_gen_node('!', or2, NULL);
    ret = ltl2smv_gen_node('|', not3, not4);
    break;
  case 'n': /* xnor */
    /* a xnor b <-> a = b */
    not2 = ltl2smv_gen_node('!',expand_formula(t->left),NULL);
    not3 = ltl2smv_gen_node('!',expand_formula(t->right),NULL);
    or1 = ltl2smv_gen_node('|', not2, not3);
    not1 = ltl2smv_gen_node('!',or1, NULL);
    /* not{2,3}->left is the already exapnded version of the left
       right part of t */
    or2 = ltl2smv_gen_node('|', not2->left, not3->left);
    not4 = ltl2smv_gen_node('!', or2, NULL);
    ret = ltl2smv_gen_node('|',not1, not4);
    break;
  case 'Z':
    tmp = ltl2smv_gen_node('Y', ltl2smv_gen_node('!',expand_formula(t->left),NULL), NULL);
    ret = ltl2smv_gen_node('!', tmp, NULL);
    break;
  case 'F':
    ret = ltl2smv_gen_node('U',ltl2smv_gen_leaf(ltl2smv_mycpy("1")),expand_formula(t->left));
    break;
  case 'G':
    tmp = ltl2smv_gen_node('U', ltl2smv_gen_leaf(ltl2smv_mycpy("1")), 
                        ltl2smv_gen_node('!',expand_formula(t->left),NULL));
    ret = ltl2smv_gen_node('!', tmp, NULL);
    break;
  case 'V':
    tmp = ltl2smv_gen_node('U', ltl2smv_gen_node('!',expand_formula(t->left),NULL),
		        ltl2smv_gen_node('!',expand_formula(t->right),NULL));
    ret = ltl2smv_gen_node('!', tmp, NULL);
    break;
  case 'O':
    ret = ltl2smv_gen_node('S',ltl2smv_gen_leaf(ltl2smv_mycpy("1")),expand_formula(t->left));
    break;
  case 'H':
    tmp = ltl2smv_gen_node('S', ltl2smv_gen_leaf(ltl2smv_mycpy("1")), 
                        ltl2smv_gen_node('!',expand_formula(t->left),NULL));
    ret = ltl2smv_gen_node('!', tmp, NULL);
    break;
  case 'T':
    tmp = ltl2smv_gen_node('S', ltl2smv_gen_node('!',expand_formula(t->left),NULL),
		        ltl2smv_gen_node('!',expand_formula(t->right),NULL));
    ret = ltl2smv_gen_node('!', tmp, NULL);
    break;
  default:
    fprintf(stderr, "expand_formula: unknown operator \"%c\"\n", t->op);
    exit(1);
    break;
  }
  return(ret);
}

static char *op_string(char *op, char *str1, char *str2)
{
  char *ret;
  int l1,l2,l3,l;

  if (str2 == NULL) {
    l1 = strlen(op);
    l2 = strlen(str1);
    l = l1 + l2 + 2 + 1;
    ret = (char *)Mymalloc(sizeof(char)*l);
    strcpy(ret, "(");
    strcat(ret, op);
    strcat(ret, str1);
    strcat(ret, ")");
  }
  else {
    l1 = strlen(op);
    l2 = strlen(str1);
    l3 = strlen(str2);
    l = l1 + l2 + l3 + 2 + 1;
    ret = (char *)Mymalloc(sizeof(char)*l);
    strcpy(ret, "(");
    strcat(ret, str1);
    strcat(ret, op);
    strcat(ret, str2);
    strcat(ret, ")");
  }
  return(ret);
}

/* returned string must be freed */
static char* gen_name_number(int n)
{
  char* ret;
  size_t len;

  len = strlen(PRE_PREFIX) + strlen(PREFIXNAME) +
    get_digits(Spec_Counter) + get_digits(n) + 1;
  
  ret = (char*) Mymalloc(sizeof(char) * len);
  
  snprintf(ret, len, "%s%d%s%d", PRE_PREFIX, Spec_Counter, PREFIXNAME, n);
  return ret;
}


static int str_hash_func(char *s)
{
  int i,l,a;
  l = strlen(s);
  for (a = i = 0; i<l; i++) {
    a += (int) s[i];
  }
  a = a % STRHASHSIZE;
  return (a);
}

static str_list *mymalloc_str_list(void)
{
  str_list *ret;
  ret =(str_list *)Mymalloc(sizeof(str_list));
  ret->name = NULL;
  ret->str = NULL;
  ret->next = NULL;
  return (ret);
}

static void add_var_list(char *key, char *item)
{
  output_list *m;
 
  m = Var_list;
  while(m != NULL) {
    if ((strcmp(m->key, key) == 0) && (strcmp(m->item, item) == 0)) return;
    m = m->next;
  }
  m = mymalloc_output_list();
  m->key = key;
  m->item = item;
  m->next = Var_list;
  Var_list = m;
}

/* Creates a variable for the expression and returns it */
static char *reg_var(char *str)
{
  int v;
  str_list *l;

  v = str_hash_func(str);
  for (l = Str_hash[v]; l != NULL; l = l->next) {
    if (strcmp(l->str, str) == 0) {
      return(l->name);
    }
  }
  l = mymalloc_str_list();
  l->str = str;
  l->name = gen_name_number(ccounter++);
  l->next = Str_hash[v];
  Str_hash[v] = l;

  return(l->name);
}

/* Given a node, produces the string of its corresponding formula */
static char *formula_string(node *t)
{
  char *ret, *t1, *t2;
  switch(t->op) {
  case '|':
    t1 = formula_string(t->left);
    t2 = formula_string(t->right);

    ret=op_string("|", t1, t2);

    free(t1);
    free(t2);
    break;
  case '!':
    t1 = formula_string(t->left);

    ret = op_string("!", t1, NULL);

    free(t1);
    break;
  case 'X':
    t1 = formula_string(t->left);

    ret = op_string("X", t1, NULL);

    free(t1);
    break;
  case 'Y':
    t1 = formula_string(t->left);

    ret = op_string("Y", t1, NULL);

    free(t1);
    break;
  case 'U':
    t1 = formula_string(t->left);
    t2 = formula_string(t->right);

    ret = op_string("U", t1, t2);

    free(t1); 
    free(t2);
    break;
  case 'S':
    t1 = formula_string(t->left);
    t2 = formula_string(t->right);

    ret = op_string("S", t1, t2);

    free(t1);
    free(t2);
    break;
  case 'l':
    ret = ltl2smv_mycpy(t->str);
    break;
  default:
    ;
  }
  return (ret);
}

static char *gen_next_term(char *s)
{
  int l;
  char *ret;
  if (strcmp(s, "1") == 0) {
    ret = Mymalloc(sizeof(char)*2);
    strcpy(ret, "1");
  }
  else if (strcmp(s, "0") == 0) {
    ret = Mymalloc(sizeof(char)*2);
    strcpy(ret, "0");
  }
  else {
    l = strlen(s);
    l += (strlen("next(") + strlen(")") + 1);
    ret = Mymalloc(sizeof(char)*l);
    strcpy(ret, "next(");
    strcat(ret, s);
    strcat(ret, ")");
  }
  return(ret);
}

static output_list *search_list(output_list *list, char *s)
{
  output_list *l;
  for (l = list; l != NULL; l = l->next) {
    if (strcmp(l->key, s) == 0) {
      return(l);
    }
  }
  return(NULL);
}

void free_output_list(output_list *list) 
{
  while(list != NULL) {
    output_list * l = list;
    
    list = list->next;
    free(l);
  }
}

/* returns whether the string s is in the Define_List */
static int define_check(char *s)
{
  if (search_list(Define_list, s) != NULL) return(1);
  return(0);
}

static void add_define_list(char *lit, char *s)
{
  output_list *l;
  l = mymalloc_output_list();
  l->key = lit;
  l->item = s;
  l->next = Define_list;
  Define_list = l;
}  

static int next_check(char *s)
{
  if (search_list(Next_list, s) != NULL) return(1);
  return(0);
}

static void add_next_list(char *lit, char *s)
{
  output_list *l;
  l = mymalloc_output_list();
  l->key = lit;
  l->item = s;
  l->next = Next_list;
  Next_list = l;
}  

static int fairness_check(char *s)
{
  if (search_list(Fairness_list, s) != NULL) return(1);
  return(0);
}

static void add_fairness_list(char *lit, char *s)
{
  output_list *l;
  l = mymalloc_output_list();
  l->key = lit;
  l->item = s;
  l->next = Fairness_list;
  Fairness_list = l;
}  

static int initially_check(char *s)
{
  if (search_list(Initially_list, s) != NULL) return(1);
  return(0);
}

static void add_initially_list(char *lit, char *s)
{
  output_list *l;
  l = mymalloc_output_list();
  l->key = lit;
  l->item = s;
  l->next = Initially_list;
  Initially_list = l;
}  

/* return the name of the variable that represents the expression */
static char *get_plit(node *t)
{
  char *ret;
  ret = ((t->pel != NULL) ? t->pel : t->el);
  return(ret);
}

/* populates the lists (Var_List, Define_List, etc) from the parse tree */
static void set_node(node *t)
{
  char *fs;

  if (t == NULL) return;
  switch(t->op) {
  case '|':
    t->pel = reg_var(formula_string(t));
    break;
  case '!':
    t->pel = reg_var(formula_string(t));
    break;
  case 'X':
    fs = formula_string(t);
    t->el = reg_var(fs);
    add_var_list(fs, t->el);
    break;
  case 'Y':
    fs = formula_string(t);
    t->el = reg_var(fs);
    add_var_list(fs, t->el);
    break;
  case 'U':
    t->pel = reg_var(formula_string(t));
    fs = op_string("X", formula_string(t), NULL);
    t->el = reg_var(fs);
    add_var_list(fs, t->el);
    break;
  case 'S':
    t->pel = reg_var(formula_string(t));
    fs = op_string("Y", formula_string(t), NULL);
    t->el = reg_var(fs);
    add_var_list(fs, t->el);
    break;
  case 'l':
    t->el = t->str;
    break;
  default:
    ;
  }
  set_node(t->left);
  set_node(t->right);
  switch(t->op) {
  case '|':
    if (define_check(t->pel) == 0) {
      add_define_list(t->pel, op_string(" := ",
				t->pel, 
				op_string("|",
					  get_plit(t->left), 
					  get_plit(t->right))));
    }
    break;
  case '!':
    if (define_check(t->pel) == 0) {
      add_define_list(t->pel, op_string(" := ",
				t->pel, 
				op_string("!",
					  get_plit(t->left), NULL)));
    }
    break;
  case 'X':
    if (next_check(t->el) == 0) {
      add_next_list(t->el, op_string(" = ",
				     gen_next_term(get_plit(t->left)),
				     t->el));
    }
    break;
  case 'Y':
    if (next_check(t->el) == 0) {
      /* Note that the index is t->el as with X, but the generated 
         string has the terms in a different order */
      add_next_list(t->el, op_string(" = ",
				     get_plit(t->left),
				     gen_next_term(t->el)));
    }
    if (initially_check(t->el) == 0) {
      add_initially_list(t->el, op_string(" = ",
                                          t->el,
                                          "0"));
    }
    break;
  case 'U':
    if (define_check(t->pel) == 0) {
      add_define_list(t->pel, op_string(" := ",
				t->pel, 
				op_string("|",
					  get_plit(t->right), 
					  op_string("&",
						    get_plit(t->left),
						    t->el))));
							    
    }
    if (next_check(t->el) == 0) {
      add_next_list(t->el, op_string(" = ",
				     gen_next_term(get_plit(t)),
				     t->el));
    }
    if (fairness_check(t->pel) == 0) {
      add_fairness_list(t->pel, op_string("|",
				 op_string("!",t->pel,NULL),
				 get_plit(t->right)));
    }
    break;
  case 'S':
    if (define_check(t->pel) == 0) {
      add_define_list(t->pel, op_string(" := ",
				t->pel, 
				op_string("|",
					  get_plit(t->right), 
					  op_string("&",
						    get_plit(t->left),
						    t->el))));
							    
    }
    if (next_check(t->el) == 0) {
      add_next_list(t->el, op_string(" = ",
				     get_plit(t),
				     gen_next_term(t->el)));
    }


    if (initially_check(t->el) == 0) {
      add_initially_list(t->el, op_string(" = ",
                                          t->el,
                                          "0"));
    }
    break;
  default:
    ;
  }    
}

static void init_str_hash(void)
{
  int i;
  
  assert(Str_hash == NULL);
  Str_hash = (str_list **) Mymalloc(sizeof(str_list *) * STRHASHSIZE);
  for (i = 0; i < STRHASHSIZE; i++) {
    Str_hash[i] = NULL;
  }
}


static void quit_str_hash(void)
{
  int i;
  
  if (Str_hash != NULL) {
    for (i = 0; i < STRHASHSIZE; i++) {
      str_list * l = Str_hash[i];

      while (l != NULL) {
        str_list * n = l;
        l=l->next;
        free(n);
      }
      Str_hash[i] = NULL;
    }
    free(Str_hash);
    Str_hash = NULL;
  }
}

static void init_all_lists(void) {
  Define_list = NULL;
  Next_list = NULL;
  Fairness_list = NULL;
  Initially_list = NULL;
  Var_list = NULL;
}

static void quit_all_lists(void) {
  free_output_list(Define_list);
  Define_list = NULL;
  free_output_list(Next_list);
  Next_list = NULL;
  free_output_list(Fairness_list);
  Fairness_list = NULL;
  free_output_list(Initially_list);
  Initially_list = NULL;
  free_output_list(Var_list);
  Var_list = NULL;
}

static void remove_outer_par(char *s)
{
  int l;
  l = strlen(s);
  if (l < 2) return;
  if (s[0] == '(') s[0] = ' ';
  if (s[l-1] == ')') s[l-1] = ' ';
  return;
}

static void print_smv_format(FILE * ofile)
{
  output_list *l;

  fprintf(ofile, "MODULE %s%d\n", MODULE_NAME, Spec_Counter);
  if (Var_list != NULL) {
    fprintf(ofile, "VAR\n");
    for (l = Var_list; l != NULL; l = l ->next) {
      /* printf("%s : boolean; -- %s", l->item, l->key); */
      fprintf(ofile, "   %s : boolean; ", l->item);
      fprintf(ofile, "\n");
    }
  }
  if (Initially_list != NULL) {
    for (l = Initially_list; l != NULL; l = l ->next) {
      fprintf(ofile, "INIT\n");
      remove_outer_par(l->item);
      fprintf(ofile, "  %s", l->item);
      fprintf(ofile, "\n");
    }
  }
  if (Next_list != NULL) {
    for (l = Next_list; l != NULL; l = l ->next) {
      fprintf(ofile, "TRANS\n");
      remove_outer_par(l->item);
      fprintf(ofile, "  %s", l->item);
      fprintf(ofile, "\n");
    }
  }
  if (Define_list != NULL) {
    fprintf(ofile, "DEFINE\n");
    for (l = Define_list; l != NULL; l = l ->next) {
      remove_outer_par(l->item);
      fprintf(ofile, "  %s;", l->item);
      fprintf(ofile, "\n");
    }
  }
  if (Fairness_list != NULL) {
    for (l = Fairness_list; l != NULL; l = l ->next) {
      fprintf(ofile, "FAIRNESS\n");
      remove_outer_par(l->item);
      fprintf(ofile, "  %s", l->item);
      fprintf(ofile, "\n");
    }
  }
  fprintf(ofile, "INIT\n");
  fprintf(ofile, "   %s\n", get_plit(ltl2smv_Specf));
}

static void set_information(node *t)
{
  init_str_hash();
  set_node(t);
}

static node *reduce_formula(node *t)
{
  if (t == NULL) return (NULL);
  if (t->op == 'l') return (t);
  if (t->op == '!') {
    if (t->left != NULL) {
      if (t->left->op =='!') {
	t = t->left->left;
      }
    }
    else {
      fprintf(stderr, "Something is wrong in reduce_formula\n");
      exit(1);
    }
  }
  t->left = reduce_formula(t->left);
  t->right = reduce_formula(t->right);
  return (t);
}

#if 0 /* unused functions */
static void initbit(void)
{
  int i;
  bit[0] = 1;
  for (i = 1; i < 33; i++) {
    bit[i] = bit[i-1]<<1;
  }
} 

static char *shortitob(long num)
{
  char *b;
  int i;
  b = (char *)Mymalloc(sizeof(char)*(33));
  for (i = 0; i < 32; i++) {
    b[i] =  (((num & bit[i]) == 0)? '0':'1');
  }
  b[32] = '\0';
  return(b);
}

static char *itob(int num)
{
   char *b;
   int i;

   b = (char *)Mymalloc(sizeof(char)*(BWIDTH+1));
   for (i = 0; i < BWIDTH; i++) {
     b[i] = '0';
   }
   b[BWIDTH] = '\0';

   for (i = 0; i < 32; i++)
     b[i] =  (((num & bit[i]) == 0)? '0':'1');

/*   printf("itob(%d) = %d\n", num, btoi(b)); */
   return(b);
}
#endif
