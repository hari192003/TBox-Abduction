/************************************************************/
/************************************************************/
/**                                                        **/
/**                       ARRAY                            **/
/**                                                        **/
/**          Growable array with bound checks.             **/
/**                                                        **/
/**                                                        **/
/**                Author: Martin Suda                     **/
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
/**                                                        **/
/************************************************************/
/************************************************************/


#include "array.h"


/**************************************************************/
/* Functions                                                  */
/**************************************************************/

void array_Init(void)
/**************************************************************
  INPUT:   None.
  RETURNS: void.
  SUMMARY: Placebo satisfying the general module layout.
***************************************************************/
{
  return;
}


void array_Free(void)
/**************************************************************
  INPUT:   None.
  RETURNS: void.
  SUMMARY: Placebo satisfying the general module layout.
***************************************************************/
{
  return;
}


ARRAY array_Create(int init, int extend)
/**************************************************************
  INPUT:   Initial capacity and increase in % of the array. Should be positive!
  RETURNS: A new array of the specified capacity and size = 0.
  MEMORY:  Allocates a new array struct
           and the internal array itself plus NULL initialization.		   .
***************************************************************/
{
  ARRAY ar;
  int   i;

  ar = (ARRAY)memory_Malloc(sizeof(ARRAY_NODE)+init*sizeof(intptr_t));
  ar->size     = 0;
  ar->extend   = extend;
  ar->capacity = init;  

  for(i=0;i<init;i++)
    ar->data[i] = (intptr_t)NULL;

  return ar;
}

void array_Claim(ARRAY ar)
/**************************************************************
  EFFECT:  Let all the allocated part be accessible.
           The content the newly accessible cells is init to zeros.
***************************************************************/
{
  int i;
  for(i = ar->size; i < ar->capacity; i++)
    ar->data[i] = (intptr_t)NULL;

  ar->size = ar->capacity;
}

ARRAY  array_DeleteIndex(ARRAY ar, int idx)
/**************************************************************
  INPUT:   Pointer to the array and an index.
  RETURNS: The updated array with the element at index deleted
         and the rest (higher indexed elemets) shitfed left by 1.
***************************************************************/
{
  int i;

  /*ASSERT((idx >= 0) && (idx < ar->size));*/

  for(i = idx+1; i < ar->size; i++ )
    ar->data[i-1] = ar->data[i];

  ar->size--;

  return ar;
}

ARRAY  array_Clean(ARRAY ar)
/**************************************************************
  INPUT:   Pointer to the array.
  RETURNS: The same array with all elements deleted.
  NOTE:    Just sets size to 0, capacity remains untoutched.
***************************************************************/
{
  ar->size = 0;
  return ar;
}

int     array_GetSize(ARRAY ar)
/**************************************************************
  INPUT:   Pointer to the array.
  RETURNS: The current size of the array.
***************************************************************/
{
  return ar->size;
}

ARRAY   array_Add(ARRAY ar, intptr_t el)
/**************************************************************
  INPUT:   A pointer to the array and element.
  RETURNS: The updated array with the element added to the end
             and size increased by one.
  MEMORY:  If the capacity is reached, capacity is doubled,
            new internal array allocated and the content copied.
***************************************************************/
{
  if (ar->size==ar->capacity)
  {
    int oldcap;
    oldcap = ar->capacity;
    ar->capacity = (ar->capacity * ar->extend)/100 + ar->capacity + 1;    /*added  + ar->capacity*/
    ar = memory_Realloc(ar,sizeof(ARRAY_NODE)+oldcap*sizeof(intptr_t),sizeof(ARRAY_NODE)+(ar->capacity)*sizeof(intptr_t));
    for(oldcap = ar->size; oldcap < ar->capacity; oldcap++)  /* reuse of oldcap saving one variable */
      ar->data[oldcap] = (intptr_t)NULL;
  }

  ar->data[ar->size] = el;
  ar->size++;

  return ar;
}

ARRAY   array_AddAtIndex(ARRAY ar, int idx, intptr_t el)
/**************************************************************
  INPUT:   A pointer to the array and index and element.
  RETURNS: The updated array with the element added at position <idx>
           and of the array increased arrordingly. New cells set
           to NULL.
  MEMORY:  If the capacity is reached, capacity is doubled until <idx> available,
           new internal array allocated and the content copied.
***************************************************************/
{
  int oldcap;

  oldcap = ar->capacity;
  
  while (ar->capacity <= idx)
    ar->capacity = (ar->capacity * ar->extend)/100 + 1 + ar->capacity;

  if (oldcap !=ar->capacity) {
    ar = memory_Realloc(ar,sizeof(ARRAY_NODE)+oldcap*sizeof(intptr_t),
			sizeof(ARRAY_NODE)+(ar->capacity)*sizeof(intptr_t));
    for(oldcap = ar->size; oldcap < ar->capacity; oldcap++)  /* reuse of oldcap saving one variable */
      ar->data[oldcap] = (intptr_t)NULL;
  }

  if (ar->size <= idx)
    ar->size =  idx+1;

  ar->data[idx] = el;

  return ar;
}

void array_RemoveAtIndex(ARRAY ar, int idx)
/**************************************************************
  INPUT:   A pointer to the array and index.
  RETURNS: The updated array where the element at position <idx>
           is removed. The size is decremented and the element
           at position <ar->size-1> moved to <idx>.
***************************************************************/
{
  (ar->size)--;
  ar->data[idx] = ar->data[ar->size];
}


ARRAY   array_AddGetIndex(ARRAY ar, intptr_t el, int *index)
/**************************************************************
  INPUT:   Pointer to the array, an element and a pointer to int
  RETURNS: The updated array with the element added to the end
           and size increased by one, the actual index is returned in <*index>
           New cells are set to NULL.
  MEMORY:  If the capacity is reached, capacity is doubled,
           new internal array allocated and the content copied.
***************************************************************/
{
  if (ar->size==ar->capacity)
  {
    int oldcap;
    oldcap = ar->capacity;
    ar->capacity = (ar->capacity * ar->extend)/100 + 1 + ar->capacity;
    ar = memory_Realloc(ar,sizeof(ARRAY_NODE)+oldcap*sizeof(intptr_t),
			sizeof(ARRAY_NODE)+(ar->capacity)*sizeof(intptr_t));
    for(oldcap = ar->size; oldcap < ar->capacity; oldcap++)  /* reuse of oldcap saving one variable */
      ar->data[oldcap] = (intptr_t)NULL;
  }                       

  ar->data[ar->size] = el;
  *index             = ar->size;
  ar->size++;

  return ar;
}

intptr_t  array_GetElement(ARRAY ar, int idx)
/**************************************************************
  INPUT:   Pointer to the array and a specific index.
  EFFECT:  In Check mode the bounds of <idx> are checked.
  RETURNS: The element at position <idx> of <ar>.
***************************************************************/
{
#ifdef CHECK
  if (!((idx >= 0) && (idx < ar->size))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n array_GetElement: %d outside array bounds. ", idx);
    misc_FinishErrorReport();
  }
#endif
  return ar->data[idx];
}

void   array_SetElement(ARRAY ar, int idx, intptr_t new_el)
/**************************************************************
  INPUT:   Pointer to the array, specific index and a new value.
  EFFECT:  In Check mode the bounds of <idx> are checked.
  RETURNS: The element at position <idx> of <ar> is set to <new_el>.
***************************************************************/
{
#ifdef CHECK
  if (!((idx >= 0) && (idx < ar->size))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n array_SetElement: %d outside array bounds. ", idx);
    misc_FinishErrorReport();
  }
#endif
  ar->data[idx] = new_el;
}

BOOL    array_ContainsElement(ARRAY ar, intptr_t el, int * index)
/**************************************************************
  INPUT:   Pointer to the array, an element and a pointer to integer.
  RETURNS: TRUE iff the array contains that int as its element.
           If so, <*index> is set to index of the first found occurence.
***************************************************************/
{
  int i;

  for(i = 0; i < ar->size; i++)
    if ((intptr_t)ar->data[i] == el) {
      *index = i;
      return TRUE;
    }

  return FALSE;
}

BOOL    array_HasElement(ARRAY ar, intptr_t el)
/**************************************************************
  INPUT:   Pointer to the array, an element and a pointer to integer.
  RETURNS: TRUE iff the array contains that int as its element.
***************************************************************/
{
  int i;

  for(i = 0; i < ar->size; i++)
    if ((intptr_t)ar->data[i] == el) {
      return TRUE;
    }

  return FALSE;
}


void   array_Sort(ARRAY ar, int (* compartor) ( const void *, const void * ))
/**************************************************************
  INPUT:   Pointer to the array and a comparator.
  EFFECT:  Uses stdlib qsort to sort the array with the help
           of the given comparator.
***************************************************************/
{
  qsort(ar->data, ar->size, sizeof(ar->data[0]), compartor);
}
                    
int    array_BSearch(ARRAY ar, const void * key, int (* comparator) ( const void *, const void * ))
/**************************************************************
  INPUT:   Pointer to the array, an element to look for and a comparator.
  RETURNS: an index of the element if found, -1 otherwise
  CAUTION: Assumes the array has been sorted previously using the same comparator "relation"
***************************************************************/
{
  intptr_t* result;
  result = (intptr_t*)bsearch(key,ar->data,ar->size,sizeof(ar->data[0]),comparator);
  if (result == NULL)
    return -1;
  else
    return (result - ar->data);
}

void    array_Delete(ARRAY ar)
/**************************************************************
  INPUT:   Pointer to the array to be deleted.
  MEMORY:  Frees the memory associated with the given array.
***************************************************************/
{
  memory_Free(ar,sizeof(ARRAY_NODE)+ar->capacity*sizeof(intptr_t));
}




intptr_t array_Top(ARRAY ar)
/**************************************************************
  INPUT:   A pointer to the array
  RETURNS: A pointer to the element at the end of the array
***************************************************************/
{
  return ar->data[ar->size-1];
}

intptr_t array_Pop(ARRAY ar)
/**************************************************************
  INPUT:   A pointer to the array
  RETURNS: A pointer to the element at the end of the array
           which then will be removed
***************************************************************/
{
  intptr_t ret;
  ret = ar->data[--ar->size];
  return ret;
}

ARRAY array_Nconc(ARRAY ar1, ARRAY ar2)
/**************************************************************
  INPUT:   Two arrays
  RETURNS: Adds all elements of <ar2> to <ar1> and returns new <ar1>
  EFFECT:  Size of <ar2> is set to 0, <ar2> is NOT deleted
***************************************************************/
{
  int i;
  for(i = 0; i < array_GetSize(ar2) ; i++){
    ar1 = array_Add(ar1,array_GetElement(ar2,i));      /* Assignation where missing*/
  }
  array_Clean(ar2);
  return ar1;
}

ARRAY array_AddList(ARRAY ar, LIST List)
/**************************************************************
  INPUT:   An array and a list.
  RETURNS: Adds all elements of <List> to <ar> and returns new <ar>
***************************************************************/
{
  while(!list_Empty(List)){
    ar   = array_Add(ar,(intptr_t)list_Car(List));
    List = list_Cdr(List);
  }
  return ar;
}

LIST array_ToList(ARRAY ar)
/**************************************************************
  INPUT:   An array.
  RETURNS: A list with the elements of <ar>, preserving order.
  EFFECT:  The array is emptied after the oparation.
***************************************************************/
{
  LIST list1;
  list1 = list_Nil();
  while(array_GetSize(ar) > 0){
    list1 = list_Cons((POINTER)array_Pop(ar),list1);
  }
  return list1;
}

ARRAY array_PointerDeleteElement(ARRAY ar, intptr_t Element)
/**************************************************************
  INPUT:   An array and an element.
  RETURNS: The array <ar> where all occurrences of <Element> with respect
           to pointer equality are removed and the gaps filled with elements
           of <ar> of higher index, preserving order.
***************************************************************/
{
  intptr_t scan;
  int      i,j;
  
  i = j = 0;

  while(i < array_GetSize(ar)){
    scan = array_GetElement(ar,i);
    if(scan != Element){
      array_SetElement(ar,j,scan);
      j++;
    }
    i++;
  }
  i = i-j;
  while(i>0){
    array_Pop(ar);
    i--;
  }
  
  return ar;
}

ARRAY array_DeleteElementIfFree(ARRAY ar, BOOL (*Test)(intptr_t), void (*Delete)(intptr_t))
/**************************************************************
  INPUT:   An array, a test for elements and a delete function
           for elements.
  RETURNS: The array <ar> where an element is deleted if <Test> on it
           succeeds. The element is then deleted with <Delete>.
           The gaps filled with elements of <ar> of higher index, 
           preserving order.
***************************************************************/
{
  intptr_t scan;
  int      i,j;

  i = j = 0;

  while(i < array_GetSize(ar)){
    scan = array_GetElement(ar,i);
    if(Test(scan)){
      Delete(scan);
      array_SetElement(ar,i,(intptr_t)NULL);
    }else{
      array_SetElement(ar,j,scan);
      j++;
    }
    i++;
  }
  i = i-j;
  while(i>0){
    array_Pop(ar);
    i--;
  }
  
  return ar;

}
