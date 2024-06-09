/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *        DFG PARSER DIAGNOSTICS                          * */
/* *                                                        * */
/* *  $Module: DFG_DIAGNOSTIC_H                             * */ 
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
/* $Date: 2016/03/23 16:52:37 $                             * */
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


#ifndef DFG_DIAGNOSTIC_H
#define DFG_DIAGNOSTIC_H

#include <stddef.h>

#include "misc.h"
#include "dfg_pos.h"

/**
 * Print a formatted error message optionally with position information.
 *
 * The format is:
 *   In <file> at line <line>, column <col>: <text>
 */
void dfg_UserErrorReportF(pos_t const* pos, char const* fmt, ...);

/**
 * Print an error message header.
 *
 * The format is:
 *   In <file> at line <line>, column <col>:
 */
void dfg_UserErrorReportHeader(pos_t const* pos);

#endif
