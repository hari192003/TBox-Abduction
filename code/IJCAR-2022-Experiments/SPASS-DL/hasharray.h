/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                       HASHING                          * */
/* *                                                        * */
/* *  $Module:   HASHARRAY                                  * */ 
/* *                                                        * */
/* *  Copyright (C) 1997, 1998, 1999, 2000, 2001            * */
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
/* $Date: 2011-05-22 13:39:02 $                             * */
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


/* $RCSfile: hasharray.h,v $ */

#ifndef _HASHARRAY_
#define _HASHARRAY_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "list.h"

/**************************************************************/
/* Structures                                                 */
/**************************************************************/

#define hsh__SIZE 29            /* a prime */

/* Each Entry is a list of pairs <key,list of values> */

typedef LIST* HASH;

void hsh_Check(HASH H);

/**************************************************************/
/* Previously Inlined Functions                               */
/**************************************************************/

unsigned long hsh_Index(POINTER Key);

void hsh_Put(HASH H, POINTER Key, POINTER Value);

void hsh_PutList(HASH H, POINTER Key, LIST List);

void hsh_PutListWithCompareFunc(HASH H, POINTER Key, 
						  LIST List,
						  BOOL (*Test)(POINTER, POINTER), 
						  unsigned long (*HashFunc)(POINTER));

LIST hsh_Get(HASH H, POINTER Key);

void hsh_DelItem(HASH H, POINTER Key);

LIST hsh_GetWithCompareFunc(HASH H, POINTER Key,
					      BOOL (*Test)(POINTER, POINTER),
					      unsigned long (*HashFunc)(POINTER));
					      
unsigned long hsh_StringHashKey(const char* Label);

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

HASH hsh_Create(void);
void hsh_Reset(HASH);
void hsh_Delete(HASH);
void hsh_Print(HASH, void (*)(POINTER), void (*)(POINTER));
LIST hsh_GetAllEntries(HASH);
LIST hsh_NGetAllKeyValueListPairs(HASH);

#endif


