/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                  STRUCTURE SHARING                     * */
/* *                                                        * */
/* *  $Module:   SHARING                                    * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 1998, 2000, 2001            * */
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
/* $Date: 2011-05-25 09:46:22 $                             * */
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


/* $RCSfile: sharing.h,v $ */

#ifndef _SHARING_
#define _SHARING_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "term.h"
#include "st.h"

/**************************************************************/
/* Data Structures and Constants                              */
/**************************************************************/

/* Symbol Tables; constants are just positive    */
/* integers and variables negative integers.     */
/* For constants and vars is a special symbol    */
/* table available, containing the  sharing      */
/* info, i.e. a POINTER to the term structure, if */
/* the symbol is already inserted in the sharing */
/* structure, a NULL Pointer else.               */
  
typedef TERM VARTABLE[symbol__MAXVARIABLES];

typedef TERM CONSTTABLE[symbol__MAXSIGNATURE];

typedef struct {
  st_INDEX   index;
  VARTABLE   vartable;
  CONSTTABLE consttable;
  NAT        stampId;
} SHARED_INDEX_NODE, *SHARED_INDEX;

/**************************************************************/
/* Inline Functions                                           */
/**************************************************************/

st_INDEX sharing_Index(SHARED_INDEX ShIndex);


void sharing_SetIndex(SHARED_INDEX ShIndex, st_INDEX ST);

const TERM* sharing_Vartable(SHARED_INDEX ShIndex);

const TERM* sharing_Consttable(SHARED_INDEX ShIndex);

NAT sharing_StampID(SHARED_INDEX ShIndex);

void sharing_SetStampID(SHARED_INDEX ShIndex, NAT Stamp);

TERM sharing_VartableEntry(SHARED_INDEX ShIndex, NAT Index);

void sharing_SetVartableEntry(SHARED_INDEX ShIndex,
						NAT Index, TERM Term);

TERM sharing_ConsttableEntry(SHARED_INDEX ShIndex,
					       NAT Index);

void sharing_SetConsttableEntry(SHARED_INDEX ShIndex,
						 NAT Index, TERM Term);

TERM sharing_GetVarFromSymbol(SYMBOL S, SHARED_INDEX ShIndex);

int sharing_VariableIndex(TERM Term);

int sharing_ConstantIndex(TERM Term);

BOOL sharing_IsSharedVar(TERM T, SHARED_INDEX ShIndex);

BOOL sharing_IsSharedConst(TERM T, SHARED_INDEX ShIndex);

BOOL sharing_IsNotReallyShared(TERM Term);

void sharing_RememberSharedTermCopy(TERM Term, TERM Copy);

TERM sharing_SharedTermCopy(TERM Term);

/**************************************************************/
/* Functions for Creation and Deletion of a SHARED_INDEX         */
/**************************************************************/

SHARED_INDEX sharing_IndexCreate(void); 
void         sharing_IndexDelete(SHARED_INDEX);

/**************************************************************/
/* Functions on term insertion and deletion.                  */
/**************************************************************/

TERM         sharing_Insert(POINTER, TERM, SHARED_INDEX);
void         sharing_Delete(POINTER, TERM, SHARED_INDEX);

void         sharing_PushOnStack(TERM);
void         sharing_PushReverseOnStack(TERM);
void         sharing_PushOnStackNoStamps(TERM);
void         sharing_PushListOnStack(LIST);
void         sharing_PushListReverseOnStack(LIST);
void         sharing_PushListReverseOnStackExcept(LIST, LIST);
void         sharing_PushListOnStackNoStamps(LIST);

/**************************************************************/
/* Functions to access unshared data by the shared terms.     */
/**************************************************************/

LIST         sharing_GetDataList(TERM, SHARED_INDEX);

void         sharing_StartDataIterator(TERM, SHARED_INDEX);
POINTER      sharing_GetNextData(void);
void         sharing_StopDataIterator(void);

LIST         sharing_NAtomDataList(TERM);
LIST         sharing_GetAllSuperTerms(SHARED_INDEX);

void         sharing_ResetAllTermStamps(SHARED_INDEX);

NAT          sharing_GetNumberOfOccurances(TERM);
NAT          sharing_GetNumberOfInstances(TERM, SHARED_INDEX);

/**************************************************************/
/* Output functions                                            */
/**************************************************************/

void         sharing_PrintVartable(SHARED_INDEX);
void         sharing_PrintConsttable(SHARED_INDEX);
void         sharing_PrintSharing(SHARED_INDEX);

/**************************************************************/
/* Debugging Functions                                        */
/**************************************************************/

void         sharing_PrintStack(void);
void         sharing_PrintSharingConstterms1(SHARED_INDEX);
void         sharing_PrintSharingVarterms1(SHARED_INDEX);
void         sharing_PrintSameLevelTerms(TERM);


#endif
