%{
/**CFile***********************************************************************

  FileName    [lex.l]

  PackageName [ltl2smv]

  Synopsis    [The LTL to SMV Translator -- Lexical Analyzer]

  Description [This file provides routines for lexical analysis of LTL formulas.]

  SeeAlso     []

  Author      [Adapted to NuSMV by Marco Roveri.
               Extended to the past operators by Ariel Fuxman.]

  Copyright   [
  This file is part of the ``ltl2smv'' package of NuSMV version 2. 
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

#include <stdio.h>
#include "ltl2smvInt.h"
#include "yacc.h"

%}

W    [ \t]
L    [\n\r\f]
  
%%
{W}+          ;
{L}           {ltl2smv_inc_linenumber();}
"|"           return OR;
"&"           return AND;
"->"          return IMPLY;
"<->"         return EQUIV;
"!"           return NEG; 
"X"           return NEXT;
"F"           return FUTURE;
"G"           return GLOBAL;
"U"           return UNTIL;
"V"           return RELEASES;
"Y"           return PREV;
"Z"           return NOTPREVNOT;
"O"           return ONCE;
"H"           return HISTORY;
"S"           return SINCE;
"T"           return TRIGGERED;
"+"           return PLUS;
"-"           return MINUS;
"*"           return TIMES;
"/"           return DIVIDE;
"mod"         return MOD;
"xor"         return XOR;
"xnor"        return XNOR;
"case"        return CASE;
"esac"        return ESAC;
"="           return EQUAL;
"!="          return NOTEQUAL;
"<="          return LE;
">="          return GE;
"<"           return LT;
">"           return GT;
"union"       return UNION;
"in"          return SETIN;
".."          return TWODOTS;
[A-Za-z_][A-Za-z0-9_.\$#-]* {
                ltl2smv_yylval.str_ptr = ltl2smv_mycpy(ltl2smv_yytext);
                return(ATOM);
              };
\"[^\"]*\"    {
                ltl2smv_yylval.str_ptr = ltl2smv_mycpy(ltl2smv_yytext);
                return(ATOM);
              };
[0-9]+        {
                ltl2smv_yylval.str_ptr = ltl2smv_mycpy(ltl2smv_yytext);
                return(NUMBER);
              };
.             return ltl2smv_yytext[0];
