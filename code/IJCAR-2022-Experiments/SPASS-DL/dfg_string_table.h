/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *        DFG LEXER STRING TABLE                          * */
/* *                                                        * */
/* *  $Module: DFG_STRING_TABLE_H                           * */ 
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
/* $Date: 2016/03/23 16:54:37 $                             * */
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


#ifndef DFG_STRING_TABLE_H
#define DFG_STRING_TABLE_H

#include "dfg_token.h"
#include "dfg_util.h"

typedef struct string_t {
	char*        text; /**< The text representation of the string. */
	int kind; /**< The token kind associated with the string. */
	u4           hash;
} string_t;

/**
 * Find or insert the string s in(to) the string table.
 *
 * If the string is in the table already, then return the existing entry.
 * In particular the given kind is discarded in this case.
 * This is used to identify keywords by adding them to the table beforehand.
 *
 * If the string is not in the table, enter it with the given kind.
 * The string s is copied and needs not be retained.
 *
 * @return The existing or new string.
 */
string_t const* string_table_insert(char const* s, size_t size, int kind);

/**
 * Initialize the string table.
 */
void string_table_init(void);

#endif
