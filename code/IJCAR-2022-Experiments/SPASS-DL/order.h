/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *         INTERFACE FOR ALL ORDERING IMPLEMENTATIONS     * */
/* *                                                        * */
/* *  $Module:   ORDER                                      * */ 
/* *                                                        * */
/* *  Copyright (C) 1997, 2000, 2001 MPI fuer Informatik    * */
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
/* $Revision: 1.6 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-11-27 12:42:39 $                             * */
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


/* $RCSfile: order.h,v $ */

#ifndef _ORDER_
#define _ORDER_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/


#include "term.h"
#include "context.h"
#include "flags.h"
#include "symbol.h"

/**************************************************************/
/* TYPES and GLOBAL VARIABLES                                 */
/**************************************************************/

typedef enum { ord_UNCOMPARABLE,
	       ord_SMALLER_THAN,
	       ord_EQUAL,
	       ord_GREATER_THAN } ord_RESULT;

/* This array is used to count variable occurrences in two terms. */
/* It may be used by any available ordering.                     */
extern NAT ord_VARCOUNT[symbol__MAXSTANDARDVAR][2];

/* A precedence is needed in almost every ordering function. */
/* For performance reasons it is stored in a global variable, */
/* instead of passing it to all those functions, which are   */
/* often recursive. Nevertheless this variable must not be   */
/* set externally!                                           */
extern PRECEDENCE ord_PRECEDENCE;

/**************************************************************/
/*  INLINE FUNCTIONS                                          */
/**************************************************************/

ord_RESULT ord_Uncomparable(void);

ord_RESULT ord_Equal(void);

ord_RESULT ord_GreaterThan(void);

ord_RESULT ord_SmallerThan(void);

BOOL ord_IsGreaterThan(ord_RESULT Res);

BOOL ord_IsNotGreaterThan(ord_RESULT Res);

BOOL ord_IsNotGreaterThanOrEqual(ord_RESULT Res);

BOOL ord_IsSmallerThan(ord_RESULT Res);

BOOL ord_IsNotSmallerThan(ord_RESULT Res);

BOOL ord_IsNotSmallerThanOrEqual(ord_RESULT Res);

BOOL ord_IsEqual(ord_RESULT Res);

BOOL ord_IsUncomparable(ord_RESULT Res);


/**************************************************************/
/*  FUNCTIONS                                                 */
/**************************************************************/

ord_RESULT ord_Not(ord_RESULT);
ord_RESULT ord_CompareAux(TERM, TERM, FLAGSTORE, PRECEDENCE, BOOL);
ord_RESULT ord_Compare(TERM, TERM, FLAGSTORE, PRECEDENCE);
ord_RESULT ord_CompareSkolem(TERM, TERM, FLAGSTORE, PRECEDENCE);
ord_RESULT ord_ContCompare(VARCONT, TERM, VARCONT, TERM, FLAGSTORE, PRECEDENCE);
BOOL       ord_CompareEqual(TERM, TERM, FLAGSTORE);
BOOL       ord_ContGreater(VARCONT, TERM, VARCONT, TERM, FLAGSTORE, PRECEDENCE);
ord_RESULT ord_LiteralCompare(TERM,ord_RESULT,TERM,ord_RESULT,BOOL, FLAGSTORE, PRECEDENCE);
void       ord_Print(ord_RESULT);
ord_RESULT ord_LiteralCompareAux(TERM, ord_RESULT, TERM, ord_RESULT, BOOL, BOOL, FLAGSTORE, PRECEDENCE);

void ord_CompareCountVars(TERM, int);
BOOL ord_CompareVarsSubset(TERM, TERM);
BOOL ord_ContGreaterSkolemSubst(VARCONT, TERM, VARCONT, TERM, FLAGSTORE, PRECEDENCE);

#endif
