/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                     HASHMAP                            * */
/* *                                                        * */
/* *  $Module:   HASHMAP                                    * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001      * */
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
/* $Revision: 1.5 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-11-27 12:13:11 $                             * */
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

#ifndef _HASHMAP_
#define _HASHMAP_


/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "memory.h"
#include "misc.h"
#include "list.h"

/**************************************************************/
/* Structures                                                 */
/**************************************************************/

typedef uintptr_t HASHMAP_HASH;

typedef HASHMAP_HASH (*HM_GET_HASH_FUNCTION)(POINTER);
typedef BOOL (*HM_KEY_EQUAL_FUNCTION)(POINTER, POINTER);

typedef struct HASHMAPBOX_HELP {
  POINTER       key;
  HASHMAP_HASH  hash;
  POINTER       value;
} HASHMAPBOX_NODE;

typedef HASHMAPBOX_NODE *HASHMAPBOX;

typedef struct HASHMAP_HELP {
  LIST* table;            /* the actual array, it is an array of lists of hashmapboxes */
  int   size;             /* size of the table, has to be a power of two */
  int   num_of_els;       /* number of elements currently stored */
  
  HM_GET_HASH_FUNCTION GetHash;   /* function to compute hashes from keys */
  HM_KEY_EQUAL_FUNCTION KeyEqual; /* function to test for equality of keys */  
    
  BOOL talking;           /* debug field */
} HASHMAP_NODE;

typedef HASHMAP_NODE *HASHMAP;

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

void         hm_Init(void);
void         hm_Free(void);

HASHMAP      hm_Create(int, HM_GET_HASH_FUNCTION, HM_KEY_EQUAL_FUNCTION, BOOL);

void         hm_Clear(HASHMAP);
void         hm_ClearWithElement(HASHMAP,void (*)(POINTER));

void         hm_Delete(HASHMAP);
void         hm_DeleteWithElement(HASHMAP,void (*)(POINTER));

HASHMAP_HASH hm_StringHash(const char*);

HASHMAP_HASH hm_PointerHash(POINTER);
BOOL         hm_PointerEqual(POINTER, POINTER);

void         hm_Insert(HASHMAP,POINTER,POINTER);
POINTER      hm_Retrieve(HASHMAP,POINTER);
POINTER      hm_RetrieveFound(HASHMAP,POINTER, BOOL*);
void         hm_Remove(HASHMAP,POINTER);                               

/* functions that make only sense when values stored are lists */
void         hm_InsertListAppend(HASHMAP,POINTER,LIST);                
void         hm_InsertListInsertUnique(HASHMAP,POINTER,POINTER);
void         hm_RemoveListDelete(HASHMAP,POINTER);
void         hm_DeleteList(HASHMAP);

#endif
