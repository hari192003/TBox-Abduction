/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *              INTERFACE FOR THE PROLOG PARSER           * */
/* *                                                        * */
/* *  $Module:   PRO                                        * */ 
/* *                                                        * */
/* *  Copyright (C) 1998, 2000, 2001 MPI fuer Informatik    * */
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
/* $Revision: 1.4 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2010-02-22 14:09:58 $                             * */
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


/* $RCSfile: pro.h,v $ */

#ifndef _PRO_
#define _PRO_

#include <stdio.h>
#include "list.h"
#include "flags.h"
#include "symbol.h"

/* Parser functions */
LIST pro_PrologParser(FILE*, FLAGSTORE, PRECEDENCE);

#endif
