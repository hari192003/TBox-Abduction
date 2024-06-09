/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *        DFG LEXER TOKEN                                 * */
/* *                                                        * */
/* *  $Module: DFG_TOKEN_H                                  * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001      * */
/* *  MPI fuer Informatik                                   * */
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
/* $Revision: 1.0 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2016/03/23 16:55:37 $                             * */
/* $Author: weidenb $                                       * */
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

#include <stdlib.h>

#include "dfg_token.h"
#include "dfg_util.h"

char const* token_kind2str(token_kind_t const kind)
{
  switch (kind) {
#define KEYWORD(name)     case T_##name: return "'" #name "'";
#define TOKEN(kind, text) case T_##kind: return "'" text "'";
#include "tokens.inc"
#undef TOKEN
#undef KEYWORD
  }
  PANIC("invalid token kind");
}
