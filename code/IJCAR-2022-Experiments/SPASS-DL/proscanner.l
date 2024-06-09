/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                 SCANNER FOR PROLOG SYNTAX              * */
/* *                                                        * */
/* *  $Module:   PROLOG                                     * */ 
/* *                                                        * */
/* *  Copyright (C) 1998, 1999, 2001 MPI fuer Informatik    * */
/* *                                                        * */
/* *  This program is free software; you can redistribute   * */
/* *  it and/or modify it under the terms of the FreeBSD    * */
/* *  Licence.                                              * */
/* *                                                        * */
/* *  This program is distributed in the hope that it will  * */
/* *  be useful, but WITHOUT ANY WARRANTY; without even     * */
/* *  the implied warranty of MERCHANTABILITY or FITNESS    * */
/* *  FOR A PARTICULAR PURPOSE.  See the LICENCE file       * */ 
/* *  for more details.                                     * */
/* *                                                        * */
/* *                                                        * */
/* $Revision: 1.5 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-25 06:54:14 $                             * */
/* $Author: weidenb $                                         * */
/* *                                                        * */
/* *             Contact:                                   * */
/* *             Christoph Weidenbach                       * */
/* *             MPI fuer Informatik                        * */
/* *             Stuhlsatzenhausweg 85                      * */
/* *             66123 Saarbruecken                         * */
/* *             Email: spass@mpi-inf.mpg.de                * */
/* *             Germany                                    * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


/* $RCSfile: proscanner.l,v $ */

%{

#include <ctype.h>
#include "memory.h"
#include "term.h"
#include "proparser.h"

extern NAT pro_LINENUMBER;  /* defined in proparser.y */

 char* pro_StringCopy(void)
{
  char *copy;
  copy = (char*) memory_Malloc(yyleng+1);
  strcpy(copy, yytext);
  return copy;
}

%}

%x comment

%option noyywrap
/* Omit unused functions yyunput, yyinput */
%option nounput
%option noinput

%%

":-"				return PRO_IF;
"?-"				return PRO_QUESTION;
[0-9]+				{ pro_lval.string = pro_StringCopy();
                                  return PRO_NUM;
                                }
[_A-Z][_a-zA-Z0-9]*		{ pro_lval.string = pro_StringCopy();
				  return PRO_VAR;
				}
[a-z][_a-zA-Z0-9]*		{ pro_lval.string = pro_StringCopy();
                                  return PRO_ID;
                                }
"%"[^\n]*                       /* one-line comment */
"/*"				BEGIN(comment);

<comment>[^*\n]*		/* eat anything that's not a '*' */
<comment>"*"+[^*/\n]*		/* eat up '*'s not followed by '/'s */
<comment>\n			++pro_LINENUMBER;
<comment>"*"+"/"                BEGIN(INITIAL);
[ \t]+				/* ignore whitespace */
"\n"				++pro_LINENUMBER;
[()\[\],\.]			return yytext[0];
!				{ misc_StartUserErrorReport();
                                  misc_UserErrorReport("\n line %d: Cut operator is not allowed\n", pro_LINENUMBER);
                                  misc_FinishUserErrorReport();
                                }
.				{ misc_StartUserErrorReport();
                                  misc_UserErrorReport("\n In line %d: Illegal character'", pro_LINENUMBER);
                                  if (isprint((int)yytext[0]))
                                    misc_UserErrorReport("%c", yytext[0]);
                                  else 
                                    misc_UserErrorReport("\\x%x", (unsigned int) yytext[0]);
                                  misc_FinishUserErrorReport();
                                }

%%
