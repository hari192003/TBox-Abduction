/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *        DFG LEXER                                       * */
/* *                                                        * */
/* *  $Module: DFG_LEXER_H                                  * */ 
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
/* $Date: 2016/03/23 16:53:37 $                             * */
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

#ifndef DFG_LEXER_H
#define DFG_LEXER_H

#include <stdio.h>

#include "dfg_token.h"

/**
 * Initialize the lexer.
 */
void lexer_init(void);

/**
 * Set the input stream and name used by the lexer.
 *
 * The name is used for source position information.
 */
void lexer_set_input(FILE* f, char const* name);

/**
 * Read stream until "*}" is reached.
 * 
 * The resulting T_TEXT token is stored in the global variable token.
 */
void lexer_lextext(void);

/**
 * Read the next token from the input.
 *
 * The token is stored in the global variable token.
 */
void lexer_next(void);

#endif
