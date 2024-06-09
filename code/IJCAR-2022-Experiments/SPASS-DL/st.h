/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                    ST INDEXING                         * */
/* *                                                        * */
/* *  $Module:   ST                                         * */ 
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
/* $Date: 2011-11-27 13:12:29 $                             * */
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


/* $RCSfile: st.h,v $ */

#ifndef _ST_
#define _ST_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "foldfg.h"
#include "term.h"
#include "symbol.h"
#include "list.h"

#include "subst.h"
#include "unify.h"

/**************************************************************/
/* Data Structures                                            */
/**************************************************************/

typedef enum {st_NOP,             st_UNIFIER,    st_GEN,
	      st_GENPRETEST,      st_INSTANCE,   st_INSTANCEPRETEST
}  st_RETRIEVAL_TYPE;

typedef enum {st_STANDARD, st_NOC} st_WHERE_TYPE;

typedef unsigned short int st_MINMAX;


/**************************************************************/
/* Type st_INDEX and Inline Functions                         */
/**************************************************************/

typedef struct st {
  SUBST     subst;
  LIST      subnodes;
  LIST      entries;
  st_MINMAX max, min;
} st_INDEX_NODE, *st_INDEX;


 st_INDEX st_Get(void);

 void st_Free(st_INDEX ST);

 SUBST st_Subst(st_INDEX ST);

 LIST st_Entries(st_INDEX ST);

 LIST st_Subnodes(st_INDEX ST);

 st_MINMAX st_Max(st_INDEX ST);

 void st_SetMax(st_INDEX ST, st_MINMAX Value);

 st_MINMAX st_Min(st_INDEX ST);

 void st_SetMin(st_INDEX ST, st_MINMAX Value);

 BOOL st_IsLeaf(st_INDEX ST);

 BOOL st_IsInner(st_INDEX ST);

 BOOL st_Empty(st_INDEX ST);

 BOOL st_Exist(st_INDEX ST);

 void st_SetNode(st_INDEX Index, SUBST Subst,
				  LIST Subnodes, LIST Entries);

 st_INDEX st_CreateNode(SUBST Subst, LIST Subnodes,
					 LIST Entries);


typedef enum {st_EMPTY = 1, st_FCT, st_CONST, st_VAR,
	      st_STAR, st_FIRST} NODETYPE;


/**************************************************************/
/* A special ST-Stack for sequential retrieval operations     */
/**************************************************************/

#define st_STACKSIZE 5000

typedef POINTER ST_STACK[st_STACKSIZE];

extern ST_STACK st_STACK;
extern int      st_STACKPOINTER;
extern int      st_STACKSAVE;

/* Stack operations */

 void st_StackInit(void);


 void st_StackPush(POINTER Entry);

 void st_StackPop(void);

 POINTER st_StackPopResult(void);

 void st_StackNPop(int N);

 POINTER st_StackTop(void);

 POINTER st_StackNthTop(int N);

 void st_StackRplacTop(POINTER Entry);

 void st_StackRplacNthTop(int N, POINTER Entry);

 void st_StackRplacNth(int N, POINTER Entry);

 int st_StackBottom(void);

 void st_StackSetBottom(int Pointer);

 BOOL st_StackEmpty(int Pointer);


/**************************************************************/
/* Functions for Creation and Deletion of an st_INDEX         */
/**************************************************************/

st_INDEX st_IndexCreate(void); 
void     st_IndexDelete(st_INDEX);
void     st_IndexDeleteWithElement(st_INDEX);

/**************************************************************/
/* Add and Remove Entries to an st_INDEX                      */
/**************************************************************/

void     st_EntryCreate(st_INDEX, POINTER, TERM, const VARCONT);
BOOL     st_EntryDelete(st_INDEX, POINTER, TERM, const VARCONT);

/**************************************************************/
/* Functions for Retrieval in the Index                       */
/**************************************************************/

LIST     st_GetUnifier(VARCONT, st_INDEX, VARCONT, TERM);
LIST     st_GetGen(VARCONT, st_INDEX, TERM);
LIST     st_GetGenPreTest(VARCONT, st_INDEX, TERM);
LIST     st_GetInstance(VARCONT, st_INDEX, TERM);
LIST     st_GetInstancePreTest(VARCONT, st_INDEX, TERM);

void     st_CancelExistRetrieval(void);

POINTER  st_ExistUnifier(VARCONT, st_INDEX, VARCONT, TERM);
POINTER  st_ExistGen(VARCONT, st_INDEX, TERM);
POINTER  st_ExistGenPreTest(VARCONT, st_INDEX, TERM);
POINTER  st_ExistInstance(VARCONT, st_INDEX, TERM);
POINTER  st_ExistInstancePreTest(VARCONT, st_INDEX, TERM);

POINTER  st_NextCandidate(void);

/**************************************************************/
/* Function for Output                                        */
/**************************************************************/

void    st_Print(st_INDEX, void (*)(POINTER));

#endif
