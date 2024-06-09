/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                       HASHING                          * */
/* *                                                        * */
/* *  $Module:   HASHARRAY                                  * */ 
/* *                                                        * */
/* *  Copyright (C) 1997, 1998, 2000, 2001                  * */
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
/* $Date: 2011-05-22 13:39:01 $                             * */
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


/* $RCSfile: hasharray.c,v $ */

#include "hasharray.h"


/**************************************************************/
/* Functions                                                  */
/**************************************************************/
/*************************************************************
*Previously inlined functions                                *  
*************************************************************/
unsigned long hsh_Index(POINTER Key)
/**************************************************************
  INPUT:   A pointer
  RETURNS: A key for the hasharray
***************************************************************/
{
  return (unsigned long)Key % hsh__SIZE;
}

void hsh_Put(HASH H, POINTER Key, POINTER Value)
/**************************************************************
  INPUT:   A hasharray, a pointer used as key and a pointer to a data item
  EFFECT:  Add Value to the list of data items associated with the key,
           if it isn't a member already
***************************************************************/
{
  LIST Scan, Pair;
  unsigned long HashKey;
  HashKey = hsh_Index(Key);
  for (Scan = H[HashKey]; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    Pair = list_Car(Scan);
    if (list_PairFirst(Pair) == Key) {
      if (!list_PointerMember(list_PairSecond(Pair), Value))
	list_Rplacd(Pair, list_Cons(Value, list_PairSecond(Pair)));
#ifdef CHECK
      hsh_Check(H);
#endif
      return;
    }
  }
  H[HashKey] = list_Cons(list_PairCreate(Key, list_List(Value)), H[HashKey]);
#ifdef CHECK
  hsh_Check(H);
#endif
}

void hsh_PutList(HASH H, POINTER Key, LIST List)
/**************************************************************
  INPUT:   A hasharray, a pointer used as key and a list of data items
  EFFECT:  Add the list to the list of data items associated with the key,
           and delete all duplicates.
***************************************************************/
{
  LIST Scan, Pair;
  unsigned long HashKey;

  HashKey = hsh_Index(Key);
  for (Scan = H[HashKey]; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    Pair = list_Car(Scan);
    if (list_PairFirst(Pair) == Key) {
      list_Rplacd(Pair, list_Nconc(list_PairSecond(Pair), List));
#ifdef CHECK
      hsh_Check(H);
#endif
      return;
    }
  }
  H[HashKey] = list_Cons(list_PairCreate(Key, List), H[HashKey]);
#ifdef CHECK
  hsh_Check(H);
#endif
}

void hsh_PutListWithCompareFunc(HASH H, POINTER Key, 
						  LIST List,
						  BOOL (*Test)(POINTER, POINTER), 
						  unsigned long (*HashFunc)(POINTER))
/**************************************************************
  INPUT:   A hasharray, a pointer used as key, a list of data
           items, a test function for key equality and a
	   hashing function.
  EFFECT:  Add the list to the list of data items associated
           with the key, and delete all duplicates.
***************************************************************/
{
  LIST Scan, Pair;
  unsigned long HashKey;

  HashKey = (unsigned long) HashFunc(Key);
  for (Scan = H[HashKey]; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    Pair = (LIST) list_Car(Scan);
    if (Test(list_PairFirst(Pair), Key)) {
      list_Rplacd(Pair, list_Nconc(list_PairSecond(Pair), List));
#ifdef CHECK
      hsh_Check(H);
#endif
      return;
    }
  }
  H[HashKey] = list_Cons(list_PairCreate(Key, List), H[HashKey]);
#ifdef CHECK
  hsh_Check(H);
#endif
}

LIST hsh_Get(HASH H, POINTER Key)
/**************************************************************
  INPUT:   A hasharray and a pointer used as key
  RETURNS: The list of data items associated with the key
***************************************************************/
{
  LIST Scan, Pair;

  for (Scan = H[hsh_Index(Key)]; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    Pair = list_Car(Scan);
    if (list_PairFirst(Pair) == Key)
      return list_PairSecond(Pair);
  }
  return NULL;
}

void hsh_DelItem(HASH H, POINTER Key)
/**************************************************************
  INPUT:   A hasharray and a pointer used as key
  RETURNS: The information associated with the key is deleted
***************************************************************/
{
  LIST Scan, Pair;
  unsigned long k;

  k = hsh_Index(Key);
  for (Scan = H[k]; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    Pair = list_Car(Scan);
    if (list_PairFirst(Pair) == Key) {
      list_Delete(list_PairSecond(Pair));
      list_PairFree(Pair);
      H[k] = list_PointerDeleteElement(H[k], Pair);
      return;
    }
  }
}

LIST hsh_GetWithCompareFunc(HASH H, POINTER Key,
					      BOOL (*Test)(POINTER, POINTER),
					      unsigned long (*HashFunc)(POINTER))
/**************************************************************
  INPUT:   A hasharray, a pointer used as key, a compare function
           for keys and a hash function for keys.
  RETURNS: The list of data items associated with the key
***************************************************************/
{
  LIST Scan, Pair;

  for (Scan = H[HashFunc(Key)]; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    Pair = list_Car(Scan);
    if (Test(list_PairFirst(Pair), Key))
      return list_PairSecond(Pair);
  }
  return NULL;
}


unsigned long hsh_StringHashKey(const char* Label)
{
  unsigned long i, s;
  s = 0;
  for (i = 0; i <= strlen(Label); i++)
    s += Label[i];
  s = s % hsh__SIZE;
  return s;
}





/*************************************************************
*Previously inlined functions ends here                      *  
**************************************************************/

HASH hsh_Create(void)
/**************************************************************
  RETURNS: A new, empty hasharray
***************************************************************/
{
  HASH h;
  NAT  l;
  h = (LIST*) memory_Malloc(sizeof(LIST) * hsh__SIZE);
  for (l=0; l < hsh__SIZE; l++)
    h[l] = list_Nil();
  return h;
}

void hsh_Reset(HASH H)
/**************************************************************
  INPUT:   A hasharray
  EFFECT:  Deletes all information stored in the array but keeps
           the array itself.
           Keys and data items are not deleted !
***************************************************************/
{
  int  i;
  LIST Scan, Pair;
  for (i = 0; i < hsh__SIZE; i++) {
    for (Scan = H[i]; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
      Pair = list_Car(Scan);
      list_Delete(list_PairSecond(Pair));
      list_PairFree(Pair);
    }
    list_Delete(H[i]);
    H[i] = list_Nil();
  }
}

void hsh_Print(HASH H, void (*KeyPrint)(POINTER), void (*ValuePrint)(POINTER))
/**************************************************************
  INPUT:   A hasharray and a print function for the values
  EFFECT:  Prints all values in the hash array
***************************************************************/
{
  int  i;
  LIST Scan;
  
  for (i = 0; i < hsh__SIZE; i++) {
    Scan = H[i];
    printf("\n %d:",i);
    while (!list_Empty(Scan)) {
      KeyPrint(list_PairFirst(list_Car(Scan)));
      printf("-");
      ValuePrint(list_PairSecond(list_Car(Scan)));
      printf("\n   ");
      Scan = list_Cdr(Scan);
    }
  }
}

void hsh_Delete(HASH H)
/**************************************************************
  INPUT:   A hasharray
  EFFECT:  Deletes all information stored in the array and 
           the array itself.
           Keys and data items are not deleted !
***************************************************************/            
{
  hsh_Reset(H);
  memory_Free(H, sizeof(LIST) * hsh__SIZE);
}

LIST hsh_GetAllEntries(HASH H)
/**************************************************************
  INPUT:   A hasharray
  RETURNS: A new list of all data items stored in the hasharray
***************************************************************/
{
  LIST Scan, Result;
  NAT i;
  Result = list_Nil();
  for (i = 0; i < hsh__SIZE; i++) {
    for (Scan = H[i]; !list_Empty(Scan); Scan = list_Cdr(Scan))
      Result = list_Nconc(Result, list_Copy(list_PairSecond(list_Car(Scan))));
  }
  return Result;
}

LIST hsh_NGetAllKeyValueListPairs(HASH H)
/**************************************************************
  INPUT:   A hasharray
  RETURNS: A new list of all key valuelist pairs, where the
           key valuelist pairs are not copied
***************************************************************/
{
  LIST Result;
  NAT i;
  Result = list_Nil();
  for (i = 0; i < hsh__SIZE; i++) {
    if (!list_Empty(H[i]))
      Result = list_Nconc(list_Copy(H[i]), Result);
  }
  return Result;
}

void hsh_Check(HASH H)
/**************************************************************
  INPUT:   A hasharray
  EFFECT:  Traverses the whole array and the lists to find dangling pointers.
***************************************************************/
{
  LIST          Scan, Scan2, Pair;
  NAT           i;
  unsigned long Key;
  for (i = 0; i < hsh__SIZE; i++) {
    for (Scan = H[i]; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
      Pair = list_Car(Scan);
      Key = (unsigned long)list_PairFirst(Pair);
      for (Scan2 = list_PairSecond(Pair); !list_Empty(Scan2); Scan2 = list_Cdr(Scan2)) {
	POINTER Value;
	char Z;
	Value = list_Car(Scan2);
	Z = * ((char*) Value);
      }
    }
  }
}
