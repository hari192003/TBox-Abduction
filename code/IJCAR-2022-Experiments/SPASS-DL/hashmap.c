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
/* $Date: 2011-11-27 12:13:10 $                             * */
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


#include "hashmap.h"

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

static int hm_GetSize(HASHMAP hm) {
  return hm->size;
}

static void hm_SetSize(HASHMAP hm, int val) {
  hm->size = val;
}

static int hm_GetNumOfEls(HASHMAP hm) {
  return hm->num_of_els;
}

static void hm_ResetNumOfEls(HASHMAP hm) {
  hm->num_of_els = 0;
}

static void hm_IncNumOfEls(HASHMAP hm) {
  hm->num_of_els++;
}

static void hm_DecNumOfEls(HASHMAP hm) {
  hm->num_of_els--;
}

static LIST hm_GetTableEntry(HASHMAP hm, int idx) {
  return hm->table[idx];
}

static void hm_SetTableEntry(HASHMAP hm, int idx, LIST val) {
  hm->table[idx] = val;
}

static void hm_SetTable(HASHMAP hm, LIST* val) {
  hm->table = val;
}

static HM_GET_HASH_FUNCTION hm_GetFuncitonGetHash(HASHMAP hm) {
  return hm->GetHash;
}

static HM_KEY_EQUAL_FUNCTION hm_GetFuncitonKeyEqual(HASHMAP hm) {
  return hm->KeyEqual;
}

void hm_Init(void)
/**************************************************************
  INPUT:   None.
  RETURNS: void.
  SUMMARY: Placebo satisfying the general module layout.
***************************************************************/
{
  return;
}

void hm_Free(void)
/**************************************************************
  INPUT:   None.
  RETURNS: void.
  SUMMARY: Placebo satisfying the general module layout.
***************************************************************/
{
  return;
}

#ifdef CHECK
static BOOL hm_IsTalking(HASHMAP hm) 
{
  return hm->talking;
}

static void hm_SetTalking(HASHMAP hm, BOOL val) 
{
  hm->talking = val;
}

static void hm_Check(HASHMAP hm)
{
  int    i;
  int    count;
  double mean, sumofdiff2;
  
#ifdef HM_DEBUGPRINT  
  if (hm_IsTalking(hm))
    printf("\nLengths:");
#endif  
  
  mean = (double)hm_GetNumOfEls(hm) / hm_GetSize(hm);
  sumofdiff2 = 0.0;
  
  count = 0;
  
  for(i = 0; i < hm_GetSize(hm); i++) {
    int len;
    
    len = list_Length(hm_GetTableEntry(hm,i));
    count += len;
    
    sumofdiff2 += (mean-len)*(mean-len);
    
#ifdef HM_DEBUGPRINT   
   if (hm_IsTalking(hm))
     printf("%d,",len); 
#endif      
  }  
  
#ifdef HM_DEBUGPRINT   
   if (hm_IsTalking(hm))
     printf("\n Variance: %f, computed: %f",(mean*(1.0-1.0/hm_GetSize(hm))),sumofdiff2/(hm_GetSize(hm)-1));
#endif          
  
  if (count != hm_GetNumOfEls(hm)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In hm_Check: Entry count error.");
    misc_FinishErrorReport();
  }  
}
#endif

HASHMAP hm_Create(int init_size_bits, HM_GET_HASH_FUNCTION GetHash, HM_KEY_EQUAL_FUNCTION KeyEqual, BOOL talking)
/**************************************************************
  INPUT:   
     An int denoting the logarithm of the initial size
     (the new size will be (1<<init_size_bits)),
     a GetHash function and KeyEqual function.
     
     It should be the case that 
     KeyEqual(K1,K2)=TRUE => GetHash(K1) = GetHash(K2)
     
  RETURNS: A newly allocated hashmap.
  MEMORY:  Allocates a new hashmap struct
            and the internal array.
***************************************************************/
{
  int i;
  HASHMAP hm;
  
  hm = (HASHMAP)memory_Malloc(sizeof(HASHMAP_NODE));
  hm_SetSize(hm,1<<init_size_bits);
  hm_SetTable(hm,(LIST*)memory_Malloc(sizeof(LIST)*hm_GetSize(hm)));  
  hm_ResetNumOfEls(hm);
    
  hm->GetHash    = GetHash;
  hm->KeyEqual   = KeyEqual;  
      
  for(i = 0; i < hm_GetSize(hm); i++)
    hm_SetTableEntry(hm,i,list_Nil());

#ifdef HM_DEBUGPRINT
  hm_SetTalking(talking);
  if (hm_IsTalking(hm))
    printf("\n hm_Create(%d)=%p",init_size_bits,(POINTER)hm);
#endif

#ifdef CHECK
  hm_Check(hm);
#endif

  return hm;
}

static HASHMAPBOX hm_CreateBox(POINTER key, HASHMAP_HASH hash, POINTER value)
{
  HASHMAPBOX Box;

  Box = (HASHMAPBOX)memory_Malloc(sizeof(HASHMAPBOX_NODE));
  Box->key = key;
  Box->hash = hash;
  Box->value = value;
  return Box;
}

static void hm_DeleteBox(HASHMAPBOX Box) 
{
  memory_Free(Box, sizeof(HASHMAPBOX_NODE));
}

static POINTER hm_BoxGetKey(HASHMAPBOX Box) 
{
  return Box->key;
}

static HASHMAP_HASH hm_BoxGetHash(HASHMAPBOX Box) 
{
  return Box->hash;
}

static POINTER hm_BoxGetValue(HASHMAPBOX Box) 
{
  return Box->value;
}

static void hm_BoxSetValue(HASHMAPBOX Box, POINTER newvalue) 
{
  Box->value = newvalue;
}

void hm_Clear(HASHMAP hm)
/**************************************************************
  INPUT:   Pointer to the hashmap to be cleared.
  MEMORY:  Makes the hashmap empty (but preserves its size).
***************************************************************/
{
#ifdef HM_DEBUGPRINT
  if (hm_IsTalking(hm))
    printf("\n hm_Clear(%p)",(POINTER)hm);
#endif

  int i;
  for(i = 0; i < hm_GetSize(hm); i++) {
    list_DeleteWithElement(hm_GetTableEntry(hm,i),(void (*)(POINTER))hm_DeleteBox);    
    hm_SetTableEntry(hm,i,list_Nil());
  }
  
  hm_ResetNumOfEls(hm);

#ifdef CHECK
  hm_Check(hm);
#endif  
}

void hm_ClearWithElement(HASHMAP hm, void (*ElementDelete)(POINTER))
/**************************************************************
  INPUT:   Pointer to the hashmap to be cleared.
  MEMORY:  Makes the hashmap empty (but preserves its size),
           also the individual stored values are disposed using ElementDelete.
***************************************************************/
{
  int i;
  
#ifdef HM_DEBUGPRINT
  if (hm_IsTalking(hm))
    printf("\n hm_ClearWithElement(%p)",(POINTER)hm);
#endif
  
  for(i = 0; i < hm_GetSize(hm); i++) {
    LIST List;
    LIST Scan;    
    HASHMAPBOX Box;
    
    List = hm_GetTableEntry(hm,i);
    
    while (!list_Empty(List)) {
      Scan = list_Cdr(List);
      Box = list_Car(List);
                 
      ElementDelete(hm_BoxGetValue(Box));
      hm_DeleteBox(Box);      
      list_Free(List);
      
      List = Scan;
    }
    
    hm_SetTableEntry(hm,i,list_Nil());
  }
  
  hm_ResetNumOfEls(hm);
  
#ifdef CHECK
  hm_Check(hm);
#endif  
}

void hm_Delete(HASHMAP hm)
/**************************************************************
  INPUT:   Pointer to the hashmap to be deleted.
  MEMORY:  Frees the memory associated with the given hashmap.
***************************************************************/
{
  hm_Clear(hm);
  
  memory_Free(hm->table,sizeof(LIST)*hm_GetSize(hm));
  memory_Free(hm,sizeof(HASHMAP_NODE));
}

void hm_DeleteWithElement(HASHMAP hm, void (*ElementDelete)(POINTER))
/**************************************************************
  INPUT:   Pointer to the hashmap to be deleted.
  MEMORY:  Frees the memory associated with the given hashmap,
           also the individual stored values are disposed using ElementDelete.
***************************************************************/
{
  hm_ClearWithElement(hm,ElementDelete);
  
  memory_Free(hm->table,sizeof(LIST)*hm_GetSize(hm));
  memory_Free(hm,sizeof(HASHMAP_NODE));
}

HASHMAP_HASH hm_StringHash(const char* key) 
/**************************************************************
  INPUT:   A string.
  RETURNS: A hash of a given string.
  COMMENT: The formula is made to ensure both
           admissable speed and hashing robustness.
***************************************************************/
{
  int i;
  HASHMAP_HASH result;
  
  i      = 0;
  result = 0;
  while (key[i] != '\0') {
    result = result*31 + key[i];
    i++;
  }
  return result;
}

HASHMAP_HASH hm_PointerHash(POINTER key)
/**************************************************************
  INPUT:   A pointer.
  RETURNS: A hash of the given pointer.
***************************************************************/
{

  HASHMAP_HASH a = (HASHMAP_HASH)key;

   a = (a+0x7ed55d16) + (a<<12);
   a = (a^0xc761c23c) ^ (a>>19);
   a = (a+0x165667b1) + (a<<5);
   a = (a+0xd3a2646c) ^ (a<<9);
   a = (a+0xfd7046c5) + (a<<3);
   a = (a^0xb55a4f09) ^ (a>>16);
   return a;

  /* return ((uintptr_t)key) >> 2;     ClauseToTermLabellist from top.c on 6800 */
  /* return ((uintptr_t)key) >> 3;     ClauseToTermLabellist from top.c on 6802 */
  /* in case of memory management, shift by binary log of memory_ALIGN */
  /* Could also depend on debug / release aspect, memory management on / off */    
}

BOOL hm_PointerEqual(POINTER p1, POINTER p2)
/**************************************************************
  INPUT:   Two pointers.
  RETURNS: True iff the pointers are equal.
  COMMENT: Equality check for pointer keys.
***************************************************************/
{
  return (p1 == p2);
}

static unsigned hm_IndexFor(HASHMAP_HASH h, int length) 
/**************************************************************
  Transformes a given hash to an address in an array of a given length.
  The length must be a power of 2.
***************************************************************/
{
  return h & (length-1);
}

static void hm_ChangeSize(HASHMAP hm, int new_size)
/**************************************************************
  INPUT:  A hashmap, a new size for it (a power of two).
  EFFECT: Changes the size of the underlying array and 
          rehashes all the elements. The old arry is freed.
***************************************************************/
{
  int          i;
  HASHMAP_HASH hash;
  LIST*        new_table;
  
#ifdef HM_DEBUGPRINT
  if (hm_IsTalking(hm))
    printf("\n hm_ChangeSize(%p,%d)",(POINTER)hm,new_size);
#endif
  
  new_table = (LIST*)memory_Malloc(sizeof(LIST)*new_size);
  
  for(i = 0; i < new_size; i++)
    new_table[i] = list_Nil();

  for(i = 0; i < hm_GetSize(hm); i++) {
    LIST Scan;
    Scan = hm_GetTableEntry(hm,i);
    
    while (!list_Empty(Scan)) {
      int j;
      LIST Elem;
      HASHMAPBOX Box;
      
      Elem = Scan;
      Box  = list_Car(Elem);
      hash = hm_BoxGetHash(Box);
      j = hm_IndexFor(hash,new_size);

      Scan = list_Cdr(Scan);
      list_Rplacd(Elem,new_table[j]);
      new_table[j] = Elem;
    }
  }
  
  memory_Free(hm->table,sizeof(LIST)*hm_GetSize(hm));
  hm_SetSize(hm,new_size);  
  hm_SetTable(hm,new_table);
}

static void hm_InsertCore(HASHMAP hm, POINTER key, HASHMAP_HASH hash, POINTER value, int index) 
{
  hm_SetTableEntry(hm,index,list_Cons(hm_CreateBox(key,hash,value),hm_GetTableEntry(hm,index)));
  hm_IncNumOfEls(hm);
  
  if (hm_GetNumOfEls(hm) > (hm_GetSize(hm) << 1))
    hm_ChangeSize(hm,hm_GetSize(hm) << 1);
}
 
void hm_Insert(HASHMAP hm, POINTER key, POINTER value)
/**************************************************************
  INPUT:   A hashmap, and a (key,value) pair to be inserted.
  EFFECT:  Inserts the given pair into the hashmap.
  
           Assumes there wasn't an entry for the given key already there.
     
  If the number of values stored in the hashmap
  exceeds a double of the current table size
  the table is doubled, new array allocated,
  and the records are rehashed. (This is completely
  heuristical and could be modified to exchange
  speed for memory and vice versa.)
***************************************************************/
{
  intptr_t i;
  HASHMAP_HASH hash;  
#ifdef CHECK
  BOOL found;
#endif

#ifdef HM_DEBUGPRINT
  if (hm_IsTalking(hm))
    printf("\n hm_Insert(%p,%p,%p)",(POINTER)hm,(POINTER)key,(POINTER)value); 
#endif  
  
#ifdef CHECK
  hm_RetrieveFound(hm,key,&found);
  if (found) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In hm_Insert: key already present.");
    misc_FinishErrorReport();
  }
#endif
  
  hash = hm_GetFuncitonGetHash(hm)(key);
  i = hm_IndexFor(hash,hm_GetSize(hm));  
  hm_InsertCore(hm,key,hash,value,i);
#ifdef CHECK
  hm_Check(hm);
#endif
}

static LIST hm_ExtractEntry(HASHMAP hm, POINTER key, HASHMAP_HASH hash, int index) 
/**************************************************************
  INPUT:   A hashmap, key, its hash, and an its index.
  EFFECT:  Extracts the LIST_NODE* from the list of Entries at <index>,
           and updates the list at that position.
  
  RETURNS: The extracted LIST_NODE* or NULL (if not found).
     
  A low-level function used to implement hm_Remove and
  hm_Retrieve with Move-To-Front strategy.
  
***************************************************************/
{
  LIST        List;
  LIST        Scan1,Scan2;
  HASHMAPBOX  Box;

  List = hm_GetTableEntry(hm,index);
  
  if (list_Empty(List))
    return NULL;
  
  Box = list_Car(List);  
    
  if ((hm_BoxGetHash(Box) == hash) && (hm_GetFuncitonKeyEqual(hm)(hm_BoxGetKey(Box),key))) {
    hm_SetTableEntry(hm,index,list_Cdr(List));
    return List;
  }
  
  Scan2 = List;
  Scan1 = list_Cdr(List);

  while (!list_Empty(Scan1)) {
    Box = list_Car(Scan1);
  
    if ((hm_BoxGetHash(Box) == hash) && (hm_GetFuncitonKeyEqual(hm)(hm_BoxGetKey(Box),key))) {
      list_Rplacd(Scan2, list_Cdr(Scan1));           
      hm_SetTableEntry(hm,index,List);      
      return Scan1;
    }
    else {
      Scan2 = Scan1;
      Scan1 = list_Cdr(Scan1);
    }
  }
  
  return NULL;     
}

POINTER hm_Retrieve(HASHMAP hm, POINTER key)
/**************************************************************
  INPUT:   A hashmap and a key.
  RETURNS: The first value found for key
           or NULL if the search fails.           
***************************************************************/
{
  int i;  
  HASHMAP_HASH hash;  
  LIST Node;

#ifdef HM_DEBUGPRINT
  if (hm_IsTalking(hm))
    printf("\n hm_Retrieve(%p,%p)=",(POINTER)hm,(POINTER)key);
#endif
  
  hash = hm_GetFuncitonGetHash(hm)(key);
  i    = hm_IndexFor(hash,hm_GetSize(hm));    
  Node = hm_ExtractEntry(hm,key,hash,i);
  
  if (Node) {
    HASHMAPBOX Box = list_Car(Node);    
    list_Rplacd(Node,hm_GetTableEntry(hm,i));
    hm_SetTableEntry(hm,i,Node);    
    
#ifdef HM_DEBUGPRINT
    if (hm_IsTalking(hm))
      printf("%p",hm_BoxGetValue(Box));
#endif    

    return hm_BoxGetValue(Box);  
  } else {
#ifdef HM_DEBUGPRINT
    if (hm_IsTalking(hm))
      printf("%p",(POINTER)NULL);
#endif
    
    return NULL;  
  }     
}


POINTER hm_RetrieveFound(HASHMAP hm, POINTER key, BOOL *found)
/**************************************************************
  INPUT:   A hashmap and a key.
  RETURNS: The first value found for key
           or NULL if the search fails. 
           <*found>  is set to TRUE iff <key> is found in <hm>
***************************************************************/
{
  int i;  
  HASHMAP_HASH hash;  
  LIST Node;

#ifdef HM_DEBUGPRINT
  if (hm_IsTalking(hm))
    printf("\n hm_Retrieve(%p,%p)=",(POINTER)hm,(POINTER)key);
#endif
  
  hash = hm_GetFuncitonGetHash(hm)(key);
  i    = hm_IndexFor(hash,hm_GetSize(hm));    
  Node = hm_ExtractEntry(hm,key,hash,i);
  
  if (Node) {
    HASHMAPBOX Box = list_Car(Node);    
    list_Rplacd(Node,hm_GetTableEntry(hm,i));
    hm_SetTableEntry(hm,i,Node);
    *found       = TRUE;
    
#ifdef HM_DEBUGPRINT
    if (hm_IsTalking(hm))
      printf("%p",hm_BoxGetValue(Box));
#endif    

    return hm_BoxGetValue(Box);  
  } else {
#ifdef HM_DEBUGPRINT
    if (hm_IsTalking(hm))
      printf("%p",(POINTER)NULL);
#endif
    *found = FALSE;
    return NULL;  
  }     
}

void hm_Remove(HASHMAP hm, POINTER key)
/**************************************************************
  INPUT:   A hashmap, a key for the element to be removed.
  
  EFFECT:  Removes the (<key>,value) pair from appropriate
           list in the hashmap's table.
           
           Assumes exactly one such entry is present.           
***************************************************************/
{
  int i;  
  HASHMAP_HASH hash;  
  LIST Node;

#ifdef HM_DEBUGPRINT
  if (hm_IsTalking(hm))
    printf("\n hm_Remove(%p,%p)",(POINTER)hm,(POINTER)key);
#endif  
  
  hash = hm_GetFuncitonGetHash(hm)(key);  
  i    = hm_IndexFor(hash,hm_GetSize(hm));  
  Node = hm_ExtractEntry(hm,key,hash,i);
     
  if (Node) {
    HASHMAPBOX Box = list_Car(Node);     
    hm_DeleteBox(Box);
    list_Free(Node);
    
    hm_DecNumOfEls(hm);
    if (hm_GetNumOfEls(hm) < (hm_GetSize(hm) >> 1))
      hm_ChangeSize(hm,hm_GetSize(hm) >> 1);
      
#ifdef CHECK
  hm_Check(hm);
#endif
  } else {
#ifdef CHECK  
  misc_StartErrorReport();
  misc_ErrorReport("\n In hm_Remove: key not present.");
  misc_FinishErrorReport();  
#endif    
  }
}

void hm_InsertListAppend(HASHMAP hm, POINTER key, LIST add) 
/**************************************************************
  INPUT:   A hashmap, a key, and a list.
  
  EFFECT:  Add the list to the list of data items associated with the key.
           
  REMARK:  Only makes sense when when values stored are lists.           
***************************************************************/
{
  int i;  
  HASHMAP_HASH hash;  
  LIST Node;

#ifdef HM_DEBUGPRINT
  if (hm_IsTalking(hm))
    printf("\n hm_InsertListAppend(%p,%p)",(POINTER)hm,(POINTER)key);
#endif  
  
  hash = hm_GetFuncitonGetHash(hm)(key);
  i    = hm_IndexFor(hash,hm_GetSize(hm));  
  Node = hm_ExtractEntry(hm,key,hash,i);
     
  if (Node) {
    HASHMAPBOX Box = list_Car(Node);    
    list_Rplacd(Node,hm_GetTableEntry(hm,i));
    hm_SetTableEntry(hm,i,Node);    
    
    hm_BoxSetValue(Box,list_Nconc(hm_BoxGetValue(Box), add));      
  } else {
    hm_InsertCore(hm,key,hash,add,i);
  }
  
#ifdef CHECK
  hm_Check(hm);
#endif  
}

void hm_InsertListInsertUnique(HASHMAP hm, POINTER key, POINTER value)
/**************************************************************
  INPUT:   A hashmap, a key, and a pointer to a data item.
  
  EFFECT:  Add Value to the list of data items associated with the key,
           if it isn't a member already
           
  REMARK:  Only makes sense when when values stored are lists.
***************************************************************/
{
  int i;  
  HASHMAP_HASH hash;  
  LIST Node;

#ifdef HM_DEBUGPRINT
  if (hm_IsTalking(hm))
    printf("\n hm_InsertListInsertUnique(%p,%p,%p)",(POINTER)hm,(POINTER)key,(POINTER)value);
#endif  
  
  hash = hm_GetFuncitonGetHash(hm)(key);
  i    = hm_IndexFor(hash,hm_GetSize(hm));  
  Node = hm_ExtractEntry(hm,key,hash,i);
     
  if (Node) {
    LIST values;
    HASHMAPBOX Box = list_Car(Node);    
    list_Rplacd(Node,hm_GetTableEntry(hm,i));
    hm_SetTableEntry(hm,i,Node);    
        
    values = hm_BoxGetValue(Box);
    if (!list_PointerMember(values, value))
      hm_BoxSetValue(Box, list_Cons(value, values));       
  } else {
    hm_InsertCore(hm,key,hash,list_List(value),i);
  }
  
#ifdef CHECK
  hm_Check(hm);
#endif  
}

void hm_RemoveListDelete(HASHMAP hm, POINTER key)
/**************************************************************
  INPUT:   A hashmap, and a key.
  
  EFFECT:  The list of values associated with the key is deleted.
           
  REMARK:  Only makes sense when when values stored are lists.
***************************************************************/
{
  int i;  
  HASHMAP_HASH hash;  
  LIST Node;

#ifdef HM_DEBUGPRINT
  if (hm_IsTalking(hm))
    printf("\n hm_RemoveListDelete(%p,%p)",(POINTER)hm,(POINTER)key);
#endif  
  
  hash = hm_GetFuncitonGetHash(hm)(key);
  i    = hm_IndexFor(hash,hm_GetSize(hm));  
  Node = hm_ExtractEntry(hm,key,hash,i);
     
  if (Node) {
    HASHMAPBOX Box = list_Car(Node);
    list_Delete(hm_BoxGetValue(Box));
    hm_DeleteBox(Box);
    list_Free(Node);  

    hm_DecNumOfEls(hm);
    if (hm_GetNumOfEls(hm) < (hm_GetSize(hm) >> 1))
      hm_ChangeSize(hm,hm_GetSize(hm) >> 1);
      
#ifdef CHECK
  hm_Check(hm);
#endif      
  } else {
#ifdef CHECK  
  misc_StartErrorReport();
  misc_ErrorReport("\n In hm_RemoveListDelete: key not present. ");
  misc_FinishErrorReport();  
#endif    
  }
}

void hm_DeleteList(HASHMAP hm)
/**************************************************************
  INPUT:   Pointer to the hashmap to be deleted.
  MEMORY:  Frees the memory associated with the given hashmap,
           including lists of values associated with keys.
           Keys and data items are not deleted!
           
  REMARK:  Only makes sense when when values stored are lists.           
***************************************************************/
{
  hm_DeleteWithElement(hm,(void (*)(POINTER))list_Delete);
}
