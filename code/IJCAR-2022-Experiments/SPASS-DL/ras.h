/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *             RANDOM ACCESS STACK                        * */
/* *                                                        * */
/* *  $Module:   RAS                                        * */ 
/* *                                                        * */
/* *  Copyright (C) 1999, 2000, 2001 MPI fuer Informatik    * */
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
/* $Revision: 1.3 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-25 06:56:20 $                             * */
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

#ifndef _RAS_
#define _RAS_


/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "misc.h"
#include "memory.h"


/**************************************************************/
/* Constants and types                                        */
/**************************************************************/

#define ras_alloc   -1  /* index of size of allocated space */
#define ras_top     -2  /* index of next free element */
#define ras_head     2  /* size of stack head for management purposes */
#define ras_stdsize 16  /* standard stack size */


typedef POINTER *RAS;

/* A RAS (Random Access Stack) is a pointer to an array of elements */
/* where the actual size of the stack and its current top pointer   */
/* are stored one and two cells before the array pointer.           */


/**************************************************************/
/* Functions                                                  */
/**************************************************************/

RAS ras_InitWithSize(RAS ras, int size);

/**************************************************************/
/* Inline Functions                                           */
/**************************************************************/

RAS 	ras_CreateWithSize(intptr_t size);
RAS 	ras_Create(void);
RAS 	ras_Init(RAS ras);
int 	ras_Size(RAS ras);
RAS 	ras_FastPush(RAS ras, POINTER entry);
RAS 	ras_Push(RAS ras, POINTER entry);
BOOL 	ras_LegalIndex(RAS ras, int stack_index);
POINTER ras_Get(RAS ras, int stack_index);
RAS 	ras_Set(RAS ras, int stack_index, POINTER entry);
BOOL 	ras_Empty(RAS ras);
POINTER ras_Pop(RAS ras);
POINTER ras_Top(RAS ras);
void 	ras_Free(RAS ras);

#endif

