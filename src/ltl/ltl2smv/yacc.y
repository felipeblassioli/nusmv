/**CFile***********************************************************************

  FileName    [yacc.y]

  PackageName [ltl2smv]

  Synopsis    [The LTL to SMV Translator -- LTL parser]

  Description [This file provides routines for the parsing of LTL formulas.]

  SeeAlso     []

  Author      [Adapted to NuSMV by Marco Roveri.
               Extended to the past operators by Ariel Fuxman. ]

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
%{
#include "ltl2smvInt.h"
#include <stdio.h>
  
EXTERN char ltl2smv_yytext[];

EXTERN int ltl2smv_yywrap ARGS((void));
EXTERN void ltl2smv_yyerror ARGS((char *msg));

static char *mystrconcat2 ARGS((char *s1, char *s2));
static char *mystrconcat3 ARGS((char *s1, char *s2, char *s3));
static char *mystrconcat3ns ARGS((char *s1, char *s2, char *s3));
static char *mystrconcat4 ARGS((char *s1, char *s2, char *s3, char *s4));


%}
%expect 123
%start ltlformula

%token OR AND IMPLY EQUIV NEG TWODOTS
%token FUTURE GLOBAL UNTIL RELEASES NEXT 
%token ONCE HISTORY SINCE TRIGGERED PREV NOTPREVNOT
%token PLUS MINUS TIMES DIVIDE MOD EQUAL NOTEQUAL
%token LE GE LT GT UNION SETIN UMINUS
%token ATOM NUMBER CASE ESAC

%right IMPLY
%left  EQUIV
%left  OR 
%left  XOR 
%left  XNOR 
%left  AND
%left  NEG 
%left  NEXT FUTURE GLOBAL UNTIL RELEASES
%left  UNION
%left  SETIN
%left  MOD
%left  PLUS MINUS
%left  TIMES DIVIDE
%left  UMINUS		/* supplies precedence for unary minus */
%left  CASE ESAC

%union {
  node* node_ptr;
  char* str_ptr;
  }
%type  <node_ptr>  ltlf orexpr andexpr atomexpr untilexpr case_body
%type  <str_ptr>   term number constant neatomset smvexpr ATOM NUMBER

%%

ltlformula : ltlf {ltl2smv_Specf = $1;}
           ;
ltlf       : orexpr {$$ = $1;}
           | orexpr IMPLY ltlf {$$ = ltl2smv_gen_node('>',$1,$3);}
           | ltlf EQUIV orexpr {$$ = ltl2smv_gen_node('=',$1,$3);}
           | ltlf XOR orexpr   {$$ = ltl2smv_gen_node('x',$1,$3);}
           | ltlf XNOR orexpr  {$$ = ltl2smv_gen_node('n',$1,$3);}
           ;
case_body  : {$$ = ltl2smv_gen_leaf(ltl2smv_mycpy("1"));}
           | ltlf ':' ltlf ';' case_body
             { $$ = ltl2smv_gen_node('c', ltl2smv_gen_node(':', $1, $3), $5); }
           ;
orexpr     : andexpr {$$ = $1;}
           | orexpr OR andexpr {$$ = ltl2smv_gen_node('|',$1,$3);}
           ;
andexpr    : untilexpr {$$ = $1;}
           | andexpr AND untilexpr {$$ = ltl2smv_gen_node('&',$1,$3);}
           ;
untilexpr  : atomexpr {$$ = $1;}
           | untilexpr UNTIL atomexpr {$$ = ltl2smv_gen_node('U',$1,$3);}
           | untilexpr SINCE atomexpr {$$ = ltl2smv_gen_node('S',$1,$3);}
           | untilexpr RELEASES atomexpr {$$ = ltl2smv_gen_node('V',$1,$3);}
           | untilexpr TRIGGERED atomexpr {$$ = ltl2smv_gen_node('T',$1,$3);}
           ;
atomexpr   : NEG atomexpr {$$ = ltl2smv_gen_node('!',$2,NULL);}
           | NEXT atomexpr {$$ = ltl2smv_gen_node('X',$2,NULL);}
           | GLOBAL atomexpr {$$ = ltl2smv_gen_node('G',$2,NULL);}
           | FUTURE atomexpr {$$ = ltl2smv_gen_node('F',$2,NULL);}
           | PREV atomexpr {$$ = ltl2smv_gen_node('Y',$2,NULL);}
           | NOTPREVNOT atomexpr {$$ = ltl2smv_gen_node('Z',$2,NULL);}
           | HISTORY atomexpr {$$ = ltl2smv_gen_node('H',$2,NULL);}
           | ONCE atomexpr {$$ = ltl2smv_gen_node('O',$2,NULL);}
           | '(' ltlf ')' {$$ = $2;}
           | CASE case_body ESAC {$$ = ltl2smv_gen_node('c',$2,NULL);}
           | smvexpr {$$ = ltl2smv_gen_leaf($1);}
           ;

term       : ATOM {$$ = $1;}
           | term '[' smvexpr ']' {$$ = mystrconcat4($1,"[",$3,"]");}
           | term '.' term {$$ = mystrconcat3ns($1,".",$3);}
           | term '.' number {$$ = mystrconcat3ns($1,".",$3);}
           ;

number     : NUMBER {$$ = $1;}
	   | PLUS NUMBER %prec UMINUS
	     { $$ = mystrconcat2("+",$2);}
           | MINUS NUMBER %prec UMINUS
	     { $$ = mystrconcat2("-",$2);}
           ;

constant   : ATOM {$$ = $1;}
           | number {$$ = $1;}
           ;

neatomset  : constant
           | neatomset ',' constant {$$ = mystrconcat3($1,",",$3);}
           ; 

smvexpr    : term {$$ = $1;}
           | number {$$ = $1;}
           | '(' smvexpr ')' {$$ = mystrconcat3("(",$2,")");}
           | smvexpr PLUS smvexpr { $$ = mystrconcat3($1,"+",$3); }
           | smvexpr MINUS smvexpr { $$ = mystrconcat3($1,"-",$3); }
           | smvexpr TIMES smvexpr { $$ = mystrconcat3($1,"*",$3); }
           | smvexpr DIVIDE smvexpr { $$ = mystrconcat3($1,"/",$3); }
           | smvexpr MOD smvexpr { $$ = mystrconcat3($1,"mod",$3); }
           | smvexpr EQUAL smvexpr { $$ = mystrconcat3($1,"=",$3); }
           | smvexpr NOTEQUAL smvexpr { $$ = mystrconcat3($1,"!=",$3); }
           | smvexpr LT smvexpr { $$ = mystrconcat3($1,"<",$3); }
           | smvexpr GT smvexpr { $$ = mystrconcat3($1,">",$3); }
           | smvexpr LE smvexpr { $$ = mystrconcat3($1,"<=",$3); }
           | smvexpr GE smvexpr { $$ = mystrconcat3($1,">=",$3); }
           | '{' neatomset '}' { $$ = mystrconcat3("{",$2,"}"); } 
           | number TWODOTS number { $$ = mystrconcat3($1,"..",$3); }
           | smvexpr UNION smvexpr { $$ = mystrconcat3($1,"union",$3); }
           | smvexpr SETIN smvexpr { $$ = mystrconcat3($1,"in",$3); }
           ;

%%
/* ----------------------------------------------------------------- */
/* yacc's functions that have to be defined */
/* ----------------------------------------------------------------- */
int ltl2smv_yywrap(void)
{
  return(1);
}

void ltl2smv_yyerror(char *msg)
{
 (void) fprintf(stderr, "%s\n", msg);
 (void) fprintf(stderr, "%s : Line %d\n", ltl2smv_yytext,
		ltl2smv_get_linenumber());
 /* exit(1); */
}

/* ----------------------------------------------------------------- */
/* this function is used in l2s.c, so it is external */
/* ----------------------------------------------------------------- */
char* ltl2smv_mystrconcat3(char *s1, char *s2, char *s3)
{
  return mystrconcat3(s1, s2, s3);
};


/* ----------------------------------------------------------------- */
/* internla static functions  */
/* ----------------------------------------------------------------- */

static char *mystrconcat2(char *s1, char *s2)
{
  char *ret;
  int len;
  len = strlen(s1);
  len += strlen(s2);
  ret = (char *)malloc(sizeof(char)*(len+1));
  strcpy(ret,s1);
  strcat(ret,s2);
  return(ret);
}

static char *mystrconcat3(char *s1, char *s2, char *s3)
{
  char *ret;
  int len;
  len = strlen(s1);
  len += strlen(s2);
  len += strlen(s3);
  ret = (char*)malloc(sizeof(char)*(len+3));
  strcpy(ret,s1);
  strcat(ret," ");
  strcat(ret,s2);
  strcat(ret," ");
  strcat(ret,s3);
  return(ret);
}

static char *mystrconcat3ns(char *s1, char *s2, char *s3)
{
  char *ret;
  int len;
  len = strlen(s1);
  len += strlen(s2);
  len += strlen(s3);
  ret = (char*)malloc(sizeof(char)*(len+1));
  strcpy(ret,s1);
  strcat(ret,s2);
  strcat(ret,s3);
  return(ret);
}

static char *mystrconcat4(char *s1, char *s2, char *s3, char *s4)
{
  char *ret;
  int len;
  len = strlen(s1);
  len += strlen(s2);
  len += strlen(s3);
  len += strlen(s4);
  ret = (char*)malloc(sizeof(char)*(len+1));
  strcpy(ret,s1);
  strcat(ret,s2);
  strcat(ret,s3);
  strcat(ret,s4);
  return(ret);
}






