/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                  PARSER FOR DFG SYNTAX                 * */
/* *                                                        * */
/* *  $Module:   DFG                                        * */
/* *                                                        * */
/* *  Copyright (C) 1997, 1998, 1999, 2000, 2001            * */
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
/* $Revision: 1.15 $                                        * */
/* $State: Exp $                                            * */
/* $Date: 2016-03-23 16:54:37 $                             * */
/* $Author: weidenb $                                      * */
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

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#ifndef DFG_PARSER_H
#define DFG_PARSER_H

#include <stdbool.h>
#include <stdlib.h>

#include <ctype.h>
#include "dfg.h"
#include "symbol.h"
#include "term.h"
#include "foldfg.h"
#include "strings.h"
#include "array.h"
#include "eml.h"

#include "dfg_diagnostic.h"
#include "dfglexer.h"
#include "dfg_util.h"
#include "dfg_string_table.h"

typedef enum{
  DFG_PRDICAT = 304,
  DFG_FUNC    = 286,
  DFG_OPERAT  = 299,
  DFG_QUANTIF = 306,
  DFG_SRT     = 310
}dfg_symboltypes;

/**************************************************************/
/* Parsing Logic                                              */
/**************************************************************/

static void dfg_parse(FILE*,char* name);

void parse_translation_unit(void);

#endif
