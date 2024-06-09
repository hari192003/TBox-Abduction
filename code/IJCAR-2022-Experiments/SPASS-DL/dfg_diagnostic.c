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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "misc.h"
#include "dfg_diagnostic.h"

void dfg_UserErrorReportF(pos_t const* const pos, char const* const fmt, ...)
{
  FILE* const o = misc_USERERROROUT;

  if (pos) {
    if(pos->name){
      fprintf(o,"\n In file %s ", pos->name);
    }else{
      fprintf(o,"\n In the input ");
    }
    if (pos->line != 0) {
      fprintf(o, "at line %u,", pos->line);
      if (pos->col != 0) {
	fprintf(o, "column %u", pos->col);
      }
    }
    fputs(": ", o);
  }

  va_list ap;
  va_start(ap, fmt);
  vfprintf(o, fmt, ap);
  va_end(ap);

  fputc('\n', o);
}

void dfg_UserErrorReportHeader(pos_t const* const pos)
{

  FILE* const o = misc_USERERROROUT;

  if (pos) {
    fprintf(o,"In file %s ", pos->name);
    if (pos->line != 0) {
      fprintf(o, "at line %u,", pos->line);
      if (pos->col != 0) {
	fprintf(o, "column %u", pos->col);
      }
    }
    fputs(": ", o);
  }
}
