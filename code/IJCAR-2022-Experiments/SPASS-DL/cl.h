/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *              INTERFACE FOR THE CLAUSE PARSER           * */
/* *                                                        * */
/* *  $Module:   CL                                         * */
/* *                                                        * */
/* *  Copyright (C) 1999, 2000 MPI fuer Informatik          * */
/* *                                                        * */
/* *  This program is free software; you can redistribute   * */
/* *  it and/or modify it under the terms of the FreeBSD    * */
/* *  Licence.                                              * */
/* *                                                        * */
/* *  This program is distributed in the hope that it will  * */
/* *  be useful, but WITHOUT ANY WARRANTY; without even     * */
/* *  the implied warranty of MERCHANTABILITY or FITNESS    * */
/* *  FOR A PARTICULAR PURPOSE.  See the LICENCE file       * */  
/* * for more details.                                      * */
/* *                                                        * */
/* *                                                        * */
/* $Revision: 1.2 $                                     * */
/* $State: Exp $                                            * */
/* $Date: 2010-02-22 14:09:57 $                             * */
/* $Author: weidenb $                                         * */
/* *                                                        * */
/* *             Contact:                                   * */
/* *             Christoph Weidenbach                       * */
/* *             MPI fuer Informatik                        * */
/* *             Im Stadtwald                               * */
/* *             66123 Saarbruecken                         * */
/* *             Email: weidenb@mpi-sb.mpg.de               * */
/* *             Germany                                    * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


/* $RCSfile: cl.h,v $ */

#ifndef _CLPARSER_
#define _CLPARSER_

#include "misc.h"
#include "symbol.h"
#include "flags.h"


/* Parser functions */
POINTER cl_GetClauseOrFormula(const char*, FLAGSTORE, PRECEDENCE);

/* Interaction between scanner and parser */
void cl_ScannerInit(const char*);
void cl_ScannerFree(void);

#endif
