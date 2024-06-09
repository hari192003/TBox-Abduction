/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *             RANDOM ACCESS STACK                        * */
/* *                                                        * */
/* *  $Module:   RAS                                        * */ 
/* *                                                        * */
/* *  Copyright (C) 2005 MPI fuer Informatik                * */
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

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "ras.h"

/**************************************************************/
/* Functions                                                  */
/**************************************************************/
/**************************************************************
 *Previously Inlined Function                                 *
 * ***********************************************************/
RAS ras_CreateWithSize(intptr_t size)
/****************************************************************
  INPUT:   The maximal expected size of the stack to create.
  RETURNS: A new empty stack.
*****************************************************************/
{
  RAS result;

#ifdef CHECK
  if (size <= 0) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ras_CreateWithSize: size not positive.");
    misc_FinishErrorReport();
  }
#endif

  result            = (RAS) memory_Malloc((size + ras_head) * sizeof(POINTER));
  result            = result + ras_head;  /* leave space for head */
  result[ras_alloc] = (POINTER) size;
  result[ras_top]   = (POINTER) 0;
  return result;
}


RAS ras_Create(void)
{
  return ras_CreateWithSize(ras_stdsize);
}


void ras_Free(RAS ras)
{
  if (ras != NULL) {
    memory_Free (
      ras - ras_head,
      (ras_head + (intptr_t) ras[ras_alloc]) * sizeof(POINTER)
    );
  }
}

RAS ras_Init(RAS ras)
/****************************************************************
  INPUT:   A random access stack.
  RETURNS: The initialized and potentially new stack.
  CAUTION: Because it potentially frees the old stack this
           function must be called inside an assignment like:
              stack = ras_InitWithSize(stack, ...)
*****************************************************************/
{
  return ras_InitWithSize(ras, ras_stdsize);
}


int ras_Size(RAS ras)
{
  return (intptr_t) ras[ras_top];
}


RAS ras_FastPush(RAS ras, POINTER entry)
/*********************************************************
  INPUT:   A random access stack and an element to push.
  RETURNS: The modified stack.
  CAUTION: The function does not care about stack overflow!
**********************************************************/    
{
  intptr_t top;

#ifdef CHECK
  if (ras_Size(ras) == (intptr_t) ras[ras_alloc]) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ras_FastPush: stack overflow.");
    misc_FinishErrorReport();
  }
#endif

  top          = ras_Size(ras);
  ras[top++]   = entry;
  ras[ras_top] = (POINTER) top;
  return ras;
}


RAS ras_Push(RAS ras, POINTER entry)
/*********************************************************
  INPUT:   A random access stack and an element to push.
  RETURNS: The modified and potentially new stack.
  SUMMARY: Before the push the stack is checked for overflow
           and in case of overflow its size is doubled while
           elements are copied to the (new) stack.
  CAUTION: Must be called inside an assignment:
              stack = ras_Push(stack, ...)
**********************************************************/  
{
  RAS old;
  intptr_t oldsize;
  POINTER *oldscan, *scan;

  /* if not enough space allocated, double it: */
  if (ras_Size(ras) == (intptr_t) ras[ras_alloc]) {
    old          = ras;
    oldsize      = (intptr_t) old[ras_alloc];
    ras          = ras_CreateWithSize(oldsize * 2);
    ras[ras_top] = (POINTER) oldsize;

    /* copy entries: */
    for (oldscan = old + oldsize - 1,scan = ras + oldsize - 1; oldscan >= old;
        oldscan--, scan--)
      *scan = *oldscan;

    ras_Free(old);
  }

  return ras_FastPush(ras, entry);
}


BOOL ras_LegalIndex(RAS ras, int stack_index)
{
  return 0 <= stack_index && stack_index < ras_Size(ras);
}


POINTER ras_Get(RAS ras, int stack_index)
{
#ifdef CHECK
  if (!ras_LegalIndex(ras, stack_index)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ras_Get: illegal stack index.");
    misc_FinishErrorReport();
  }
#endif

  return ras[stack_index];
}


RAS ras_Set(RAS ras, int stack_index, POINTER entry)
{
#ifdef CHECK
  if (!ras_LegalIndex(ras, stack_index)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ras_Set: illegal stack index.");
    misc_FinishErrorReport();
  }
#endif

  ras[stack_index] = entry;
  return ras;
}


BOOL ras_Empty(RAS ras)
{
  return ras_Size(ras) == 0;
}


POINTER ras_Pop(RAS ras)
{
  intptr_t top;

#ifdef CHECK
  if (ras_Empty(ras)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ras_Pop: empty stack.");
    misc_FinishErrorReport();
  }
#endif

  top          = ras_Size(ras) - 1;
  ras[ras_top] = (POINTER) top;
  return ras[top];
}


POINTER ras_Top(RAS ras)
{
#ifdef CHECK
  if (ras_Empty(ras)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ras_Top: empty stack.");
    misc_FinishErrorReport();
  }
#endif

  return ras[ras_Size(ras) - 1];
}

/**************************************************************
 *Previously Inlined Functions Ends here                      *
 * ***********************************************************/
 
RAS ras_InitWithSize(RAS ras, int size)
/****************************************************************
  INPUT:   A random access stack the maximal expected size of the
           stack to init.
  RETURNS: The initialized and potentially new stack.
  CAUTION: Because it potentially frees the old stack this
           function must be called inside an assignment like:
              stack = ras_InitWithSize(stack, ...)
*****************************************************************/
{

#ifdef CHECK
  if (size <= 0) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ras_InitWithSize: size not positive.");
    misc_FinishErrorReport();
  }
#endif

  if (size > (intptr_t) ras[ras_alloc]) {
    ras_Free(ras);
    ras = ras_CreateWithSize(size);
  }
  else
    ras[ras_top] = (POINTER) 0;
  return ras;
}

