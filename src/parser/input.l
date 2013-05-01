/***************************************************************  -*-C-*-  ***/
%{
/**CFile***********************************************************************

  FileName    [input.l]

  PackageName [parser]

  Synopsis    [Lexical analyzer for the NuSMV input language]

  SeeAlso     [grammar.y]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``parser'' package of NuSMV version 2. 
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

  For more information on NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

******************************************************************************/
 
#include "util.h"

#include "utils/utils.h"
#include "utils/ustring.h"
#include "node/node.h"
#include "utils/error.h"

#include <errno.h>

#include "grammar.h" /* For yacc/lex interaction */

/* For direct interpretation of atomic (ATOM and NUMBER)
   and boolan (TRUE and FALSE) constants. */
#include "symbols.h" 

static char rcsid[] UTIL_UNUSED = "$Id: input.l,v 1.12.2.4.2.2 2005/09/13 14:37:10 nusmv Exp $";

static int nusmv_skip_comment ARGS((void));

/* 
   The following are needed in order to allow parsing of long
   comments, that otherwise will cause errors since flex thinks that
   it is using REJECT even if it isn't.  The "%option noreject" seems
   to be ignored in the latest (and previous) versions of flex.
*/
#ifdef YY_USES_REJECT
#undef YY_USES_REJECT
#endif


#define YY_INPUT(buf, result, max_size) \
	if ( YY_CURRENT_BUFFER->yy_is_interactive ) \
		{ \
		int c = '*', n; \
		for (n=0; n<max_size && \
		     (c = getc(yyin)) != EOF && \
                     c!=' ' && c!='\n' && c!='\t' && c!='\r' && c!='\f'; ++n ) \
			buf[n] = (char) c; \
		if (c==' ' || c=='\n' || c=='\t' || c=='\r' || c=='\f') buf[n++] = (char) c; \
		if (c == EOF && ferror(yyin)) YY_FATAL_ERROR( "input in flex scanner failed" ); \
		result = n; \
		} \
	else \
		{ \
		errno=0; \
		while ( (result = fread(buf, 1, max_size, yyin))==0 && ferror(yyin)) \
			{ \
			if(errno != EINTR) \
				{ \
				YY_FATAL_ERROR( "input in flex scanner failed" ); \
				break; \
				} \
			errno=0; \
			clearerr(yyin); \
			} \
		}


%}
%a	15000
%o	25000
%option noreject
%option always-interactive

%%
[ \n\t\r\f]             ;
"--"                    nusmv_skip_comment();
^"#"" "[0-9]+.*\n       sscanf(yytext,"# %d",&yylineno);
^"#".*\n                rpterr("Unexpected preprocessor directive:\n %s\n",yytext);
"ASYNC"                 return(TOK_ASYNC);
"MODULE"                return(TOK_MODULE);
"process"               return(TOK_PROCESS);
"DEFINE"                return(TOK_DEFINE);
"VAR"                   return(TOK_VAR);
"IVAR"                  return(TOK_IVAR);
"CONSTANT"              return(TOK_CONSTANT);
"INIT"                  return(TOK_INIT);
"TRANS"                 return(TOK_TRANS);
"INVAR"                 return(TOK_INVAR);
"FORMAT"                return(TOK_FORMAT);
"SPEC"                  return(TOK_SPEC);
"LTLSPEC"               return(TOK_LTLSPEC);
"PSLSPEC"               {Parser_switch_to_psl(); return(TOK_PSLSPEC);}
"COMPUTE"               return(TOK_COMPUTE);
"INVARSPEC"             return(TOK_INVARSPEC);
"CONSTRAINT"            return(TOK_CONSTRAINT);
"SIMPWFF"               return(TOK_SIMPWFF);
"CTLWFF"                return(TOK_CTLWFF);
"LTLWFF"                return(TOK_LTLWFF);
"COMPWFF"               return(TOK_COMPWFF);
"IN"                    return(TOK_INCONTEXT);
"FAIRNESS"              return(TOK_FAIRNESS);
"JUSTICE"               return(TOK_JUSTICE);
"COMPASSION"            return(TOK_COMPASSION);
"ISA"                   return(TOK_ISA);
"ASSIGN"                return(TOK_ASSIGN);
"INPUT"                 return(TOK_INPUT);
"OUTPUT"                return(TOK_OUTPUT);
"IMPLEMENTS"            return(TOK_IMPLEMENTS);
"GOTO"                  return(TOK_GOTO);
"LET"                   return(TOK_LET);
"STEP"                  return(TOK_STEP);
"EVAL"                  return(TOK_EVAL);
"RESET"                 return(TOK_RESET);
"array"                 return(TOK_ARRAY);
"of"                    return(TOK_OF);
"boolean"               return(TOK_BOOLEAN);
"EX"                    return(TOK_EX);
"AX"                    return(TOK_AX);
"EF"                    return(TOK_EF);
"AF"                    return(TOK_AF);
"EG"                    return(TOK_EG);
"AG"                    return(TOK_AG);
"E"                     return(TOK_EE);
"F"                     return(TOK_OP_FUTURE);
"O"                     return(TOK_OP_ONCE);
"G"                     return(TOK_OP_GLOBAL);
"H"                     return(TOK_OP_HISTORICAL);
"X"                     return(TOK_OP_NEXT);
"Y"                     return(TOK_OP_PREC);
"Z"                     return(TOK_OP_NOTPRECNOT);
"A"                     return(TOK_AA);
"U"                     return(TOK_UNTIL);
"S"                     return(TOK_SINCE);
"V"                     return(TOK_RELEASES);
"T"                     return(TOK_TRIGGERED);
"BU"                    return(TOK_BUNTIL);
"EBF"                   return(TOK_EBF);
"ABF"                   return(TOK_ABF);
"EBG"                   return(TOK_EBG);
"ABG"                   return(TOK_ABG);
"MIN"                   return(TOK_MMIN);
"MAX"                   return(TOK_MMAX);
"("                     return(TOK_LP);
")"                     return(TOK_RP);
"["                     return(TOK_LB);
"]"                     return(TOK_RB);
"{"                     return(TOK_LCB);
"}"                     return(TOK_RCB);
"FALSE"                 {
				yylval.node = new_node(FALSEEXP,Nil,Nil);
				return(TOK_FALSEEXP);
			}
"TRUE"                  {
				yylval.node = new_node(TRUEEXP,Nil,Nil);
				return(TOK_TRUEEXP);
			}
"apropos"               return(TOK_APROPOS);
"case"                  return(TOK_CASE);
"esac"                  return(TOK_ESAC);
"if"                    return(TOK_IF);
"then"                  return(TOK_THEN);
"else"                  return(TOK_ELSE);
":="                    return(TOK_EQDEF);
"+"                     return(TOK_PLUS);
"-"                     return(TOK_MINUS);
"*"                     return(TOK_TIMES);
"/"                     return(TOK_DIVIDE);
"mod"                   return(TOK_MOD);
"="                     return(TOK_EQUAL);
"!="                    return(TOK_NOTEQUAL);
"<="                    return(TOK_LE);
">="                    return(TOK_GE);
"<"                     return(TOK_LT);
">"                     return(TOK_GT);
"next"                  return(TOK_NEXT);
"init"                  return(TOK_SMALLINIT);
"sigma"                 return(TOK_SIGMA);
"self"                  return(TOK_SELF);
"union"                 return(TOK_UNION);
"in"                    return(TOK_SETIN);
".."                    return(TOK_TWODOTS);
"."                     return(TOK_DOT);
"->"                    return(TOK_IMPLIES);
"<->"                   return(TOK_IFF);
"|"                     return(TOK_OR);
"&"                     return(TOK_AND);
"xor"                   return(TOK_XOR);
"xnor"                  return(TOK_XNOR);
"!"                     return(TOK_NOT);
[A-Za-z_][A-Za-z0-9_\$#-]*  { 
                             yylval.node = new_node(ATOM,(node_ptr)find_string(yytext),Nil);
                             return(TOK_ATOM);
                           }
[0-9]+                  {
                          int i;
                          sscanf(yytext,"%d",&i);
                          yylval.node = new_node(NUMBER,(node_ptr)i,Nil);
                          return(TOK_NUMBER);
                        }
\"[^\"]*\"              {
                          yylval.node = new_node(ATOM,(node_ptr)find_string(yytext),Nil);
                          return(TOK_ATOM);
                        } /* Unused token */
","                     return(TOK_COMMA);
":"                     return(TOK_COLON);
";"                     return(TOK_SEMI);
.			rpterr("illegal character");
%%

static int nusmv_skip_comment() {
  register int c;

  do {
    c = input();
  } while ( c != '\n' && c != EOF );
  return(0);
}
