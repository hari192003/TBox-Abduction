/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *        DFG PARSER AND LEXER UTILITIES                  * */
/* *                                                        * */
/* *  $Module: DFG_UTIL_H                                   * */ 
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
/* $Date: 2016/03/23 16:56:37 $                             * */
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

#ifndef DFG_UTIL_H
#define DFG_UTIL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned u4;

/**
 * Allocate n objects of the given type.
 */
#define MALLOCN(type, n) ((type*)malloc((n) * sizeof(type)))

/**
 * Allocate n objects of the given type and zero them out.
 */
#define MALLOCNZ(type, n) ((type*)calloc((n), sizeof(type)))

/**
 * Determine whether the strings a and b are equal.
 *
 * This is an easier to use wrapper around strcmp().
 */
static inline bool streq(char const* const a, char const* const b)
{
	return strcmp(a, b) == 0;
}

/**
 * Print the message and abort the program.
 */
#define PANIC(msg) (fputs((msg), stderr), abort())

#endif
