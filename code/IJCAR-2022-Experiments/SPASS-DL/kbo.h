/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *         EXTENDED KNUTH BENDIX ORDERING                 * */
/* *                                                        * */
/* *  $Module:   KBO                                        * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 2001 MPI fuer Informatik    * */
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
/* $Date: 2011-11-27 12:18:32 $                             * */
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


/* $RCSfile: kbo.h,v $ */

#ifndef _KBO_
#define _KBO_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "term.h"
#include "context.h"
#include "foldfg.h"
#include "order.h"

/**************************************************************/
/*  FUNCTIONS                                                 */
/**************************************************************/

ord_RESULT kbo_Compare(TERM, TERM);
ord_RESULT kbo_CompareAux(TERM, TERM, BOOL);
ord_RESULT kbo_ContCompare(VARCONT, TERM, VARCONT, TERM); 
BOOL       kbo_ContGreater(VARCONT, TERM, VARCONT, TERM);
BOOL       kbo_ContGreaterSkolemSubst(VARCONT, TERM, VARCONT, TERM); 

#endif
