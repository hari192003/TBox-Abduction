/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *             GLOBAL SYSTEM VECTOR                       * */
/* *                                                        * */
/* *  $Module:   VECTOR                                     * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1998, 1999, 2000, 2001            * */
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
/* $Revision: 1.4 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-25 13:15:38 $                             * */
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


/* $RCSfile: vector.h,v $ */


#ifndef _VECTOR_
#define _VECTOR_

#include "misc.h"


/**************************************************************/
/* Type and Variable Definitions                              */
/**************************************************************/

#define vec_SIZE 20000

typedef POINTER VECTOR[vec_SIZE];

extern VECTOR vec_VECTOR;
extern int    vec_MAX;

/**************************************************************/
/* Inline Functions                                           */
/**************************************************************/

/* Stack operations */

void vec_Init(void);

void vec_Push(POINTER Value);

POINTER vec_GetNth(NAT Index);

void vec_PutNth(NAT Index, POINTER Value);

void vec_Pop(void);

POINTER vec_PopResult(void);

void vec_PopToN(int N);

void vec_NPop(int N);

POINTER vec_Top(void);

POINTER vec_NthTop(int N);

void vec_RplacTop(POINTER Value);

void vec_RplacNthTop(int N, POINTER Value);

int vec_ActMax(void);

void vec_SetMax(int Index);

BOOL vec_IsMax(int Index);

BOOL vec_IsEmpty(void);

/**************************************************************/
/* Prototypes                                                 */
/**************************************************************/

void vec_Swap(int, int);
void vec_PrintSel(int, int, void(*)(POINTER));
void vec_PrintAll(void(*)(POINTER));

#endif
