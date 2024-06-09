/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                     LISTS                              * */
/* *                                                        * */
/* *  $Module:   LIST                                       * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 1998, 1999, 2000            * */
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
/* $Revision: 1.3 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-22 13:51:46 $                             * */
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


/* $RCSfile: list.h,v $ */

#ifndef _LIST_
#define _LIST_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "memory.h"
#include "misc.h"

/**************************************************************/
/* Structures                                                 */
/**************************************************************/

typedef struct LIST_HELP {
  struct LIST_HELP *cdr;
  POINTER           car;
} LIST_NODE;

typedef LIST_NODE *LIST;

/**************************************************************/
/* Macros                                                     */
/**************************************************************/

void list_Free(LIST L);

LIST list_Nil(void);

BOOL list_Empty(LIST L);

BOOL list_Exist(LIST L);

POINTER list_Car(LIST L);

POINTER list_NCar(LIST *L);

LIST list_Cdr(LIST L);

POINTER list_First(LIST L);

POINTER list_Second(LIST L);

POINTER list_Third(LIST L);

POINTER list_Fourth(LIST L); 

POINTER list_Fifth(LIST L); 

void list_Rplacd(LIST L1, LIST L2);

void list_Rplaca(LIST L, POINTER P);

void list_RplacSecond(LIST L, POINTER P);


/**************************************************************/
/* Functions                                                  */
/**************************************************************/

LIST    list_Copy(const LIST);
LIST    list_CopyWithElement(const LIST, POINTER (*)(POINTER));
void    list_InsertNext(LIST, POINTER);

void    list_NMapCar(LIST, POINTER (*)(POINTER));
void    list_Apply(void (*)(POINTER), LIST);

LIST    list_Reverse(const LIST);
LIST    list_NReverse(LIST);

void    list_Split(LIST, LIST *, LIST *); 
LIST    list_PointerSort(LIST);
LIST    list_Merge(LIST, LIST, BOOL (*)(POINTER, POINTER));
LIST    list_MergeSort(LIST, BOOL (*)(POINTER, POINTER));
LIST    list_InsertionSort(LIST, BOOL (*)(POINTER, POINTER));
LIST    list_Sort(LIST, BOOL (*)(POINTER, POINTER));
BOOL    list_SortedInOrder(LIST, BOOL (*)(POINTER, POINTER));
LIST    list_NumberSort(LIST , NAT (*)(POINTER));
LIST    list_GreaterNumberSort(LIST , NAT (*)(POINTER));
LIST    list_NNumberMerge(LIST , LIST, NAT (*)(POINTER));

POINTER list_DequeueNext(LIST);
POINTER list_NthElement(LIST, NAT);
void    list_DeleteWithElement(LIST, void (*)(POINTER));
NAT     list_DeleteWithElementCount(LIST, void (*)(POINTER));
LIST    list_DeleteElement(LIST, POINTER, BOOL (*)(POINTER, POINTER));
LIST    list_DeleteElementIf(LIST, BOOL (*)(POINTER));
LIST    list_DeleteElementIfFree(LIST, BOOL (*)(POINTER), void (*)(POINTER));
LIST    list_DeleteElementFree(LIST, POINTER, BOOL (*)(POINTER, POINTER), void (*)(POINTER));
LIST    list_DeleteOneElement(LIST, POINTER, BOOL (*)(POINTER, POINTER));
LIST    list_PointerDeleteElement(LIST, POINTER);
LIST    list_PointerDeleteElementFree(LIST, POINTER, void (*)(POINTER));
LIST    list_PointerDeleteOneElement(LIST, POINTER);
BOOL    list_DeleteFromList(LIST*, POINTER);
BOOL    list_DeleteOneFromList(LIST*, POINTER);
LIST    list_DeleteDuplicates(LIST, BOOL (*)(POINTER, POINTER));
LIST    list_DeleteDuplicatesFree(LIST, BOOL (*)(POINTER, POINTER), void (*)(POINTER));
LIST    list_PointerDeleteDuplicates(LIST);

BOOL    list_IsSetOfPointers(LIST);
LIST    list_NPointerUnion(LIST, LIST);
LIST    list_NUnion(LIST, LIST, BOOL (*)(POINTER, POINTER));
LIST    list_NListTimes(LIST, LIST);
LIST    list_NIntersect(LIST, LIST, BOOL (*)(POINTER, POINTER));
LIST    list_Nconc(LIST, LIST);
void    list_NInsert(LIST, LIST);
LIST    list_NPointerIntersect(LIST, LIST);
BOOL    list_HasIntersection(LIST, LIST);
LIST    list_NPointerDifference(LIST, LIST);
LIST    list_NDifference(LIST, LIST, BOOL (*)(POINTER, POINTER));
LIST    list_NMultisetDifference(LIST, LIST, BOOL (*)(POINTER, POINTER));
BOOL    list_PointerReplaceMember(LIST, POINTER, POINTER);

void    list_DeleteAssocListWithValues(LIST, void (*)(POINTER));
POINTER list_AssocListValue(LIST, POINTER);
LIST    list_AssocListPair(LIST, POINTER);

LIST    list_MultisetDistribution(LIST);
int     list_CompareMultisetsByElementDistribution(LIST, LIST);

NAT     list_Length(LIST);
NAT     list_Bytes(LIST);

/**************************************************************/
/* Functional Inline Functions                                */
/**************************************************************/

LIST list_Cons(POINTER Ptr, const LIST List);

LIST list_List(POINTER P);

LIST list_Append(LIST List1, LIST List2);

void list_Delete(LIST L);

BOOL list_Member(LIST List, POINTER Element,
				   BOOL (*Test)(POINTER, POINTER));

POINTER list_GetMember(LIST List, POINTER Element,
				   BOOL (*Test)(POINTER, POINTER));

BOOL list_PointerMember(LIST List, POINTER Element);

/**************************************************************/
/* Stack Macros                                               */
/**************************************************************/

LIST list_StackBottom(void);


BOOL list_StackEmpty(LIST S);


LIST list_Push(POINTER I, LIST L);


POINTER list_Top(LIST L);


LIST list_Pop(LIST L);


void list_RplacTop(LIST L, POINTER P);


LIST list_StackFree(LIST L);


/**************************************************************/
/* Pair Macros                                                */
/**************************************************************/

LIST list_PairNull(void);

LIST list_PairCreate(POINTER P1, POINTER P2);

void list_PairFree(LIST L);

POINTER list_PairFirst(LIST L);

POINTER list_PairSecond(LIST L);

void list_PairRplacSecond(LIST L, POINTER P);

void list_DeletePairList(LIST L);

void list_DeleteDistribution(LIST L);

/**************************************************************/
/* Assoc Lists                                                */
/**************************************************************/

LIST list_AssocCons(LIST L, POINTER Key, POINTER Value);

#endif
