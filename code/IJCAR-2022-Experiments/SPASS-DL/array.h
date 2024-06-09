/************************************************************/
/************************************************************/
/**                                                        **/
/**                       ARRAY                            **/
/**                                                        **/
/**          Growable array with bound checks.             **/
/**                                                        **/
/**                Author: Martin Suda                     **/
/**                                                        **/
/**                                                        **/
/**  This program is free software; you can redistribute   **/
/**  it and/or modify it under the terms of the FreeBSD    **/
/**  Licence.                                              **/
/**                                                        **/
/**  This program is distributed in the hope that it will  **/
/**  be useful, but WITHOUT ANY WARRANTY; without even     **/
/**  the implied warranty of MERCHANTABILITY or FITNESS    **/
/**  FOR A PARTICULAR PURPOSE.  See the LICENCE file       **/
/**  for more details.                                     **/
/************************************************************/
/************************************************************/


#ifndef _ARRAY_    
#define _ARRAY_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "memory.h"
#include "list.h"

/**************************************************************/
/* Structures                                                 */
/**************************************************************/

typedef struct ARRAY_HELP
{
  int         size;
  int         capacity;
  int         extend;
  intptr_t    data[];
} ARRAY_NODE, *ARRAY;


/**************************************************************/
/**************************************************************/
/*  This is the structure to implement growable array:        */
/*     size is the number of valid elements of the array.     */
/*     capacity denotes how much is currently allocated.      */
/*     increase is the factor when increasing the array in %  */
/*              default is 200                                */
/*     data contains a pointer to the actual internal array.  */
/*                                                            */
/*  The structure can hold general objects via pointers       */
/*    or integers.                                            */
/*                                                            */
/**************************************************************/
/**************************************************************/


/**************************************************************/
/* Functions                                                  */
/**************************************************************/

void   array_Init(void);
void   array_Free(void);

ARRAY  array_Create(int,int);
void   array_Claim(ARRAY);
int    array_GetSize(ARRAY);

ARRAY  array_DeleteIndex(ARRAY, int);
ARRAY  array_Clean(ARRAY);

ARRAY     array_Add(ARRAY, intptr_t);
ARRAY     array_AddAtIndex(ARRAY, int, intptr_t);
void      array_RemoveAtIndex(ARRAY, int);
ARRAY     array_AddGetIndex(ARRAY, intptr_t, int *);
intptr_t  array_GetElement(ARRAY, int);
void      array_SetElement(ARRAY, int, intptr_t);
BOOL      array_ContainsElement(ARRAY, intptr_t, int *);
BOOL      array_HasElement(ARRAY, intptr_t);

void   array_Sort(ARRAY, int (*) ( const void *, const void * ));
int    array_BSearch(ARRAY, const void *, int (*) ( const void *, const void * ));

intptr_t array_Top(ARRAY);
intptr_t array_Pop(ARRAY);
ARRAY    array_Nconc(ARRAY, ARRAY);
ARRAY    array_AddList(ARRAY, LIST);
LIST     array_ToList(ARRAY);
ARRAY    array_PointerDeleteElement(ARRAY, intptr_t);
ARRAY    array_DeleteElementIfFree(ARRAY, BOOL (*)(intptr_t), void (*)(intptr_t));



void   array_Delete(ARRAY);

#endif

