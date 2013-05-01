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

  For more information of NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

******************************************************************************/
 
#include "utils/utils.h"
#include "utils/ustring.h"
#include "utils/error.h"

#include "ord_grammar.h"
#include "ParserOrd.h"
#include "ParserOrd_private.h"

static char rcsid[] UTIL_UNUSED = "$Id: ord_input.l,v 1.1.2.1 2003/11/27 09:11:47 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/* The parser instance used by flex/bison generated code */
static ParserOrd_ptr _parser = PARSER_ORD(NULL);

void parser_ord_set_global_parser(ParserOrd_ptr parser)
{
  nusmv_assert(_parser == PARSER_ORD(NULL));
  _parser = parser;
}

void parser_ord_reset_global_parser(ParserOrd_ptr parser)
{
  PARSER_ORD_CHECK_INSTANCE(_parser);
  nusmv_assert(parser == _parser);
  _parser = PARSER_ORD(NULL);
}

ParserOrd_ptr parser_ord_get_global_parser(void)
{
   PARSER_ORD_CHECK_INSTANCE(_parser);
   return _parser;
}


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int nusmv_skip_comment ARGS((void));


/* The following are needed in order to allow parsing of long
   comments, that otherwise will cause errors since flex thinks that
   it is using REJECT even if it isn't.  The "%option noreject" seems
   to be ignored in the latest (and previous) versions of flex.
*/
#ifdef YY_USES_REJECT
#undef YY_USES_REJECT
#endif

%}
%a	15000
%o	25000
%option noreject
%option prefix="parser_ord_"
%option outfile="lex.yy.c"


%%
[ \n\t\r\f]             ;
"--"                    nusmv_skip_comment();
"."                     return DOT;
"["                     return LB;
"]"                     return RB;
[A-Za-z_][A-Za-z0-9_\$#-]*  { 
			  parser_ord_lval.node = 
			    parser_ord_mk_atom(parser_ord_get_global_parser(), 
					       parser_ord_text);
			  return ATOM;
			}
[0-9]+                  { 
                          int i;
                          sscanf(parser_ord_text, "%d", &i);
                          parser_ord_lval.node = 
			    parser_ord_mk_num(parser_ord_get_global_parser(), 
					      i);
                          return NUMBER;
                        }
.			{  // rpterr("illegal character");
                           printf("illegal character\n");
			   exit(1);
                        }
%%

static int nusmv_skip_comment() 
{
  register int c;

  do {
    c = input();
  } while ( c != '\n' && c != EOF );

  return 0;
}
 
