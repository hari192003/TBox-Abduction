/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *             GLOBAL SYSTEM VECTOR                       * */
/* *                                                        * */
/* *  $Module:   VECTOR                                     * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 2000, 2001 MPI fuer Informatik    * */
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


/* $RCSfile: vector.c,v $ */

#include "vector.h"
/**************************************************************/
/* Global Variables                                           */
/**************************************************************/

VECTOR vec_VECTOR;
int    vec_MAX;

/**************************************************************/
/* previously Inlined Functions                               */
/**************************************************************/
void vec_Init(void)
{
  vec_MAX = 0;
}

void vec_Push(POINTER Value)
{
#ifdef CHECK
  if (vec_MAX >= vec_SIZE) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In vec_Push: Vector Overflow.");
    misc_FinishErrorReport();
  }
#endif

  vec_VECTOR[vec_MAX++] = Value;
}


POINTER vec_GetNth(NAT Index)
{
#ifdef CHECK
  if (Index >= vec_SIZE || Index >= vec_MAX) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In vec_GetNth: Illegal vector access.");
    misc_FinishErrorReport();
  }
#endif

  return vec_VECTOR[Index];
}


void vec_PutNth(NAT Index, POINTER Value)
{
#ifdef CHECK
  if (Index >= vec_SIZE || Index >= vec_MAX) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In vec_PutNth: Illegal vector access.");
    misc_FinishErrorReport();
  }
#endif

  vec_VECTOR[Index] = Value;
}


void vec_Pop(void)
{
  --vec_MAX;
}

POINTER vec_PopResult(void)
{
  return vec_VECTOR[--vec_MAX];
}

void vec_PopToN(int N)
{
  vec_VECTOR[N] = vec_VECTOR[--vec_MAX];
}

void vec_NPop(int N)
{
  vec_MAX -= N;
}

POINTER vec_Top(void)
{
  return vec_VECTOR[vec_MAX-1];
}

POINTER vec_NthTop(int N)
{
  return vec_VECTOR[vec_MAX-(1+N)];
}


void vec_RplacTop(POINTER Value)
{
  vec_VECTOR[vec_MAX-1] = Value;
}

void vec_RplacNthTop(int N, POINTER Value)
{
  vec_VECTOR[vec_MAX-(1+N)] = Value;
}

int vec_ActMax(void)
{
  return vec_MAX;
}

void vec_SetMax(int Index)
{
  vec_MAX = Index;
}

BOOL vec_IsMax(int Index)
{
  return vec_MAX == Index;
}

BOOL vec_IsEmpty(void)
{
  return vec_MAX == 0;
}

/**************************************************************/
/* Inline Functions ends here                                 */
/**************************************************************/

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

void vec_Swap(int i, int j)
/**********************************************************
  INPUT:   Two integers i and j which designate the i-th and
           the j-th cell of the vector.
  RETURNS: None.
  CAUTION: The values in the i-th and the j-th cell in the
            vector are interchanged.
********************************************************/
{
  POINTER help;

  help  = vec_GetNth(i);
  vec_PutNth(i, vec_GetNth(j));
  vec_PutNth(j, help);
 
}


void vec_PrintSel(int beg, int end, void (*ElementPrint)(POINTER))
/**********************************************************
  INPUT:   An int for the start position, an int for
           the end position and a print function for
	   elements.
  RETURNS: None.
  EFFECT:  Writes the vector from beg to end to stdout.
  CAUTION: None.
********************************************************/
{
  int i;

  if (vec_ActMax() > 0) {
    for (i = beg; i < end; i++){
      printf("Entry %d:\t",i);
      ElementPrint(vec_GetNth(i));
      putchar('\n');
    }
  } else
    puts("Vector is empty");
}


void vec_PrintAll(void (*ElementPrint)(POINTER))
/**********************************************************
  INPUT:   A print function for the elements of the vector.
  RETURNS: None.
  EFFECT:  Writes the vector to stdout.
  CAUTION: None.
********************************************************/
{
  int i;

  if (vec_ActMax() > 0) {
    for (i = 0; i < vec_ActMax(); i++) {
      printf("Entry %d:\t", i);
      ElementPrint(vec_GetNth(i));
      putchar('\n');
    }
  } else
    puts("Vector is empty");
}
