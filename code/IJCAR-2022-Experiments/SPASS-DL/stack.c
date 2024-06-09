/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *              GLOBAL SYSTEM STACK                       * */
/* *                                                        * */
/* *  $Module:      STACK                                   * */ 
/* *                                                        * */
/* *  Copyright (C) 1999, 2001 MPI fuer Informatik          * */
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


/* $RCSfile: stack.c,v $ */

#include "stack.h"

 void stack_Init(void)
{
  stack_POINTER = 0;
}

 void stack_Push(POINTER Entry)
{
#ifdef CHECK
  if (stack_POINTER >= stack_SIZE) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In stack_Push: Stack Overflow.");
    misc_FinishErrorReport();
  }
#endif
  
  stack_STACK[stack_POINTER++]= Entry;
}

 int stack_Pop(void)
{
  return --stack_POINTER;
}

 POINTER stack_PopResult(void)
{
  return stack_STACK[--stack_POINTER];
}

 void stack_NPop(NAT N)
{
  stack_POINTER -= N;
}

 POINTER stack_Top(void)
{
  return stack_STACK[stack_POINTER-1];
}

 POINTER stack_NthTop(NAT N)
{
  return stack_STACK[stack_POINTER-(1+N)];
}

 void stack_RplacTop(POINTER Entry)
{
  stack_STACK[stack_POINTER-1] = Entry;
}

 void stack_RplacNthTop(NAT N, POINTER Entry)
{
  stack_STACK[stack_POINTER-(1+N)] = Entry;
}

 void stack_RplacNth(NAT N, POINTER Entry)
{
  stack_STACK[N] = Entry;
}

 NAT stack_Bottom(void)
{
  return stack_POINTER;
}

 void stack_SetBottom(NAT Ptr)
{
  stack_POINTER = Ptr;
}

 BOOL stack_Empty(NAT Ptr)
{
  return stack_POINTER == Ptr;
}

/**************************************************************/
/* Global Variables                                           */
/**************************************************************/

STACK stack_STACK;
NAT   stack_POINTER;
