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

#ifndef DFG_TOKEN_H
#define DFG_TOKEN_H

#include "dfg_pos.h"

/**
 * Enumeration of token kinds.
 *
 * The enumeration is generated automatically from the tokens.inc.
 */
typedef enum token_kind_t {
#define KEYWORD(name)     T_##name,
#define TOKEN(kind, text) T_##kind,
#include "tokens.inc"
#undef TOKEN
#undef KEYWORD
} token_kind_t;

/**
 * Returns the string representation of the given token kind.
 */
char const* token_kind2str(token_kind_t);

typedef struct token_t {
	token_kind_t kind; /**< The kind of the token. */
	char const*  text; /**< The text of the token. */
	pos_t        pos;  /**< The source position of the token. */
} token_t;

extern token_t token; /**< The current token. */

#endif
