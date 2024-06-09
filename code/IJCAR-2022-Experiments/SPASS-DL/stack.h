/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *              GLOBAL SYSTEM STACK                       * */
/* *                                                        * */
/* *  $Module:      STACK                                   * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1998, 1999, 2001                  * */
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
/* $Revision: 1.3 $                                     * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-25 09:51:04 $                             * */
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


/* $RCSfile: stack.h,v $ */

#ifndef _STACK_
#define _STACK_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "misc.h"

/**************************************************************/
/* More basic types and macros                                */
/**************************************************************/


#define stack_SIZE 10000

typedef POINTER STACK[stack_SIZE];


/**************************************************************/
/* Global Variables                                           */
/**************************************************************/

extern STACK stack_STACK;
extern NAT   stack_POINTER;


/**************************************************************/
/* Inline Functions                                           */
/**************************************************************/

 void stack_Init(void);

 void stack_Push(POINTER Entry);
  
 int stack_Pop(void);

 POINTER stack_PopResult(void);

 void stack_NPop(NAT N);

 POINTER stack_Top(void);

 POINTER stack_NthTop(NAT N);

 void stack_RplacTop(POINTER Entry);

 void stack_RplacNthTop(NAT N, POINTER Entry);

 void stack_RplacNth(NAT N, POINTER Entry);

 NAT stack_Bottom(void);

 void stack_SetBottom(NAT Ptr);

 BOOL stack_Empty(NAT Ptr);

#endif


