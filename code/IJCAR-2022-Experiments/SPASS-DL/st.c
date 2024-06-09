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
/* $Date: 2011-11-27 13:12:28 $                             * */
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


/* $RCSfile: st.c,v $ */

#include "st.h"
/**************************************************************/
/* previously Inlined Functions                               */
/**************************************************************/
 st_INDEX st_Get(void)
{
  return (st_INDEX) memory_Malloc(sizeof(st_INDEX_NODE));
}

 void st_Free(st_INDEX ST)
{
  memory_Free(ST, sizeof(st_INDEX_NODE));
}

 SUBST st_Subst(st_INDEX ST)
{
  return ST->subst;
}

 LIST st_Entries(st_INDEX ST)
{
  return ST->entries;
}

 LIST st_Subnodes(st_INDEX ST)
{
  return ST->subnodes;
}

 st_MINMAX st_Max(st_INDEX ST)
{
  return ST->max;
}

 void st_SetMax(st_INDEX ST, st_MINMAX Value)
{
  ST->max = Value;
}

 st_MINMAX st_Min(st_INDEX ST)
{
  return ST->min;
}

 void st_SetMin(st_INDEX ST, st_MINMAX Value)
{
  ST->min = Value;
}

 BOOL st_IsLeaf(st_INDEX ST)
{
  return !list_Empty(st_Entries(ST));
}

 BOOL st_IsInner(st_INDEX ST)
{
  return !list_Empty(st_Subnodes(ST));
}

 BOOL st_Empty(st_INDEX ST)
{
  return (ST == NULL || (!st_IsLeaf(ST) && !st_IsInner(ST)));
}

 BOOL st_Exist(st_INDEX ST)
{
  return (ST != NULL && (st_IsLeaf(ST) || st_IsInner(ST)));
}

 void st_SetNode(st_INDEX Index, SUBST Subst,
				  LIST Subnodes, LIST Entries)
{
  Index->subst    = Subst;
  Index->subnodes = Subnodes;
  Index->entries  = Entries;
}

 st_INDEX st_CreateNode(SUBST Subst, LIST Subnodes,
					 LIST Entries)
{
  st_INDEX new_index;

  new_index = st_Get();
  st_SetNode(new_index, Subst, Subnodes, Entries);

  return new_index;
}


 void st_StackInit(void)
{
  st_STACKPOINTER = 0;
}

 void st_StackPush(POINTER Entry)
{
#ifdef CHECK
  if (st_STACKPOINTER >= st_STACKSIZE) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In st_StackPush: ST-Stack Overflow!\n");
    misc_FinishErrorReport();
  }
#endif
  
  st_STACK[st_STACKPOINTER++] = Entry;
}

 void st_StackPop(void)
{
  --st_STACKPOINTER;
}

 POINTER st_StackPopResult(void)
{
  return st_STACK[--st_STACKPOINTER];
}

 void st_StackNPop(int N)
{
  st_STACKPOINTER -= N;
}

 POINTER st_StackTop(void)
{
  return st_STACK[st_STACKPOINTER - 1];
}

 POINTER st_StackNthTop(int N)
{
  return st_STACK[st_STACKPOINTER - (1 + N)];
}

 void st_StackRplacTop(POINTER Entry)
{
  st_STACK[st_STACKPOINTER - 1] = Entry;
}

 void st_StackRplacNthTop(int N, POINTER Entry)
{
  st_STACK[st_STACKPOINTER - (1 + N)] = Entry;
}

 void st_StackRplacNth(int N, POINTER Entry)
{
  st_STACK[N] = Entry;
}

 int st_StackBottom(void)
{
  return st_STACKPOINTER;
}

 void st_StackSetBottom(int Pointer)
{
  st_STACKPOINTER = Pointer;
}

 BOOL st_StackEmpty(int Pointer)
{
  return st_STACKPOINTER == Pointer;
}


/**************************************************************/
/* Inline Functions ends here                                 */
/**************************************************************/


/**************************************************************/
/* Local Variables                                            */
/**************************************************************/

static st_RETRIEVAL_TYPE st_CURRENT_RETRIEVAL;
static st_WHERE_TYPE     st_WHICH_VARCONTS;
static VARCONT           st_INDEX_VARCONT;
static st_MINMAX         st_EXIST_MINMAX;

/**************************************************************/
/* Global Variables                                           */
/**************************************************************/

ST_STACK st_STACK;
int      st_STACKPOINTER;
int      st_STACKSAVE;


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  CREATION OF INDEX                                     * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


st_INDEX st_IndexCreate(void)
/**************************************************************
  INPUT:   None.
  RETURNS: A pointer to a new St-Index.
  SUMMARY: Creates a new St-index.
***************************************************************/
{
  st_INDEX StIndex;

  StIndex           = st_Get();
  StIndex->subnodes = list_Nil();
  StIndex->entries  = list_Nil();
  StIndex->subst    = subst_Nil();
  st_SetMax(StIndex, 0);
  st_SetMin(StIndex, 0);

  return StIndex;
}


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  CREATION OF INDEX NODES                               * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


static st_INDEX st_NodeAddLeaf(SUBST Subst, st_MINMAX MinMax, POINTER Pointer)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{ 
  st_INDEX NewLeaf;

  NewLeaf           = st_Get();
  NewLeaf->subnodes = list_Nil();
  NewLeaf->entries  = list_List(Pointer);
  NewLeaf->subst    = Subst;
  st_SetMax(NewLeaf, MinMax);
  st_SetMin(NewLeaf, MinMax);

  return NewLeaf;
}


static void st_NodeAddInner(st_INDEX StIndex, SUBST SubstOld, SUBST SubstNew,
			    SUBST ComGen, st_MINMAX MinMax, POINTER Pointer)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  st_INDEX OldIndexNode;

  OldIndexNode           = st_Get();
  OldIndexNode->subst    = SubstOld;
  OldIndexNode->entries  = StIndex->entries;
  OldIndexNode->subnodes = StIndex->subnodes;
  st_SetMax(OldIndexNode, st_Max(StIndex));
  st_SetMin(OldIndexNode, st_Min(StIndex));

  subst_Delete(StIndex->subst);

  StIndex->subst         = ComGen;
  StIndex->entries       = list_Nil();
  StIndex->subnodes      = list_Cons(st_NodeAddLeaf(SubstNew, MinMax, Pointer),
				     list_List(OldIndexNode));

  if (st_Max(StIndex) < MinMax)
    st_SetMax(StIndex, MinMax);
  else if (st_Min(StIndex) > MinMax)
    st_SetMin(StIndex, MinMax);
}


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  MERGING OF INDEX NODES                                * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


static void st_NodeMergeWithSon(st_INDEX StIndex)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  st_INDEX SubNode;

  SubNode = (st_INDEX)list_Car(StIndex->subnodes);

  list_Delete(StIndex->subnodes);

  StIndex->subst    = subst_Merge(SubNode->subst, StIndex->subst);
  StIndex->entries  = SubNode->entries;
  StIndex->subnodes = SubNode->subnodes;
  st_SetMax(StIndex, st_Max(SubNode));
  st_SetMin(StIndex, st_Min(SubNode));

  subst_Delete(SubNode->subst);

  st_Free(SubNode);
}


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  VARIABLE HANDLING FOR TERMS AND SUBSTS.               * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


static void st_CloseUsedVariables(const VARCONT Context, LIST NodeList)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  for (; list_Exist(NodeList); NodeList = list_Cdr(NodeList)) {
    SUBST Subst;

    for (Subst = ((st_INDEX)list_Car(NodeList))->subst;
	 subst_Exist(Subst);
	 Subst = subst_Next(Subst))
      if (!cont_VarIsUsed(Context, subst_Dom(Subst)))
	cont_CreateClosedBinding(Context, subst_Dom(Subst));

    if (!st_IsLeaf((st_INDEX)list_Car(NodeList)))
      st_CloseUsedVariables(Context, ((st_INDEX)list_Car(NodeList))->subnodes);
  }
}


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  HEURISTICS FOR INSERTION OF TERMS AND SUBSTS.         * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


static st_INDEX st_FirstVariant(const VARCONT Context, LIST Subnodes, st_INDEX* BestNonVariant)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  st_INDEX EmptyVariant;

  for (EmptyVariant = NULL, *BestNonVariant = NULL;
       list_Exist(Subnodes);
       Subnodes = list_Cdr(Subnodes)) {
    st_INDEX CurrentNode;

    CurrentNode = (st_INDEX)list_Car(Subnodes);

    cont_StartBinding();

    if (subst_Variation(Context, CurrentNode->subst)) {
      if (subst_Exist(CurrentNode->subst)) {	
	subst_CloseVariables(Context, CurrentNode->subst);

	return CurrentNode;
      } else
	EmptyVariant = CurrentNode;

    } else if (*BestNonVariant == NULL)
      if (subst_MatchTops(Context, CurrentNode->subst))
	*BestNonVariant = CurrentNode;

    cont_BackTrack();
  }

  return EmptyVariant;
}


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  INSERTION OF TERMS                                    * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


void st_EntryCreate(st_INDEX StIndex, POINTER Pointer, TERM Term, const VARCONT Context)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  st_INDEX  Current;
  st_INDEX  BestNonVariant;
  SYMBOL    FirstDomain;
  st_MINMAX MinMax;

  cont_Check();

  MinMax         = term_ComputeSize(Term);
  BestNonVariant = (st_INDEX)NULL;

  /* CREATE INITIAL BINDING AND INITIALIZE LOCAL VARIABLES */
  FirstDomain = cont_NextIndexVariable(Context);
  cont_CreateBinding(Context, FirstDomain, Context, Term);

  Current = StIndex;

  if (st_Max(Current) < MinMax)
    st_SetMax(Current, MinMax);
  else if (st_Min(Current) > MinMax)
    st_SetMin(Current, MinMax);

  /* FIND "LAST" VARIATION */
  while (!st_IsLeaf(Current) &&
	 (Current = st_FirstVariant(Context, Current->subnodes, &BestNonVariant))) {
    if (st_Max(Current) < MinMax)
      st_SetMax(Current, MinMax);
    else if (st_Min(Current) > MinMax)
      st_SetMin(Current, MinMax);
    
    StIndex = Current;
  }

  if (cont_BindingsSinceLastStart()==0 && Current && st_IsLeaf(Current)) {
    
    /* INSERT ENTRY EQUAL MODULO RENAMING */
    Current->entries = list_Cons(Pointer, Current->entries);
  
  } else if (BestNonVariant) {
    
    /* CREATE INNER NODE AND A NEW LEAF */
    SUBST ComGen, SubstOld, SubstNew;

    if (!st_IsLeaf(BestNonVariant))
      st_CloseUsedVariables(Context, BestNonVariant->subnodes);

    ComGen = subst_ComGen(Context, BestNonVariant->subst, &SubstOld, &SubstNew);

    st_NodeAddInner(BestNonVariant,
		    SubstOld,
		    subst_CloseOpenVariables(SubstNew),
		    ComGen,
		    MinMax,
		    Pointer);
    
  } else
    
    /* ADD A SINGLE LEAF NODE TO FATHER */
    StIndex->subnodes =
      list_Cons(st_NodeAddLeaf(subst_CloseOpenVariables(subst_Nil()), MinMax,
			       Pointer),
		StIndex->subnodes);

  cont_Reset();
}


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  DELETION OF INDEX                                     * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/

void st_IndexDeleteWithElement(st_INDEX StIndex)
/**************************************************************
 INPUT:   A pointer to an existing St-Index.
 SUMMARY: Deletes the whole index structure and
 the terms contained in the index
 ***************************************************************/
{
	if (StIndex == NULL)
		return;
	else if (st_IsLeaf(StIndex))
		list_DeleteWithElement(StIndex->entries, (void (*) (POINTER)) term_Delete);
	else
		/* Recursion */
		list_DeleteWithElement(StIndex->subnodes,
				(void(*)(POINTER)) st_IndexDeleteWithElement);

	subst_Delete(StIndex->subst);
	st_Free(StIndex);

}

void st_IndexDelete(st_INDEX StIndex)
/**************************************************************
  INPUT:   A pointer to an existing St-Index.
  SUMMARY: Deletes the whole index structure.
***************************************************************/
{
  if (StIndex == NULL)
    return;
  else if (st_IsLeaf(StIndex))
    list_Delete(StIndex->entries);
  else
    /* Recursion */
    list_DeleteWithElement(StIndex->subnodes, (void (*)(POINTER))st_IndexDelete);

  subst_Delete(StIndex->subst);
  st_Free(StIndex);
}


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  DELETION OF ENTRY FOR TERMS                           * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


static st_INDEX st_EntryDeleteHelp(const VARCONT Context, st_INDEX StIndex, POINTER Pointer,
				   BOOL* Found)
/**************************************************************
  INPUT:   The root of an abstraction tree (StIndex), a
           pointer to a specific entry of the tree and
           a query term.
  RETURNS: Nothing.
  SUMMARY: Uses Term in order to find Pointer in the tree.
  EFFECTS: Will delete nodes of StIndex.
***************************************************************/
{
  if (st_IsLeaf(StIndex)) {

    *Found = list_DeleteFromList(&(StIndex->entries), Pointer);

    if (list_Exist(StIndex->entries))
      return StIndex;
    else {
      subst_Delete(StIndex->subst);
      st_Free(StIndex);

      return NULL;
    }

  } else {

    LIST Subnodes;

    for (Subnodes = StIndex->subnodes;
	 list_Exist(Subnodes);
	 Subnodes = list_Cdr(Subnodes)) {
      st_INDEX CurrentNode;

      CurrentNode = (st_INDEX)list_Car(Subnodes);

      cont_StartBinding();

      if (subst_Variation(Context, CurrentNode->subst)) {
	list_Rplaca(Subnodes, st_EntryDeleteHelp(Context,
						 CurrentNode,
						 Pointer,
						 Found));
	if (*Found) {
	  if (list_DeleteFromList(&(StIndex->subnodes), NULL))
	    if (list_Empty(list_Cdr(StIndex->subnodes))) {
	      /* 'StIndex' has one subnode only. */
	      st_NodeMergeWithSon(StIndex);
	  
	      return StIndex;
	    }

	  /* Assertion: 'StIndex' is an inner node. */

	  CurrentNode      = (st_INDEX)list_Car(StIndex->subnodes);
	  st_SetMax(StIndex, st_Max(CurrentNode));
	  st_SetMin(StIndex, st_Min(CurrentNode));

	  for (Subnodes = list_Cdr(StIndex->subnodes);
	       list_Exist(Subnodes);
	       Subnodes = list_Cdr(Subnodes)) {
	    CurrentNode = (st_INDEX)list_Car(Subnodes);

	    if (st_Max(CurrentNode) > st_Max(StIndex))
	      st_SetMax(StIndex, st_Max(CurrentNode));

	    if (st_Min(CurrentNode) < st_Min(StIndex))
	      st_SetMin(StIndex, st_Min(CurrentNode));
	  }

	  return StIndex;
	}
      }

      cont_BackTrack();
    }

    return StIndex;
  }
}


BOOL st_EntryDelete(st_INDEX StIndex, POINTER Pointer, TERM Term, const VARCONT Context)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{ 
  BOOL   Found;
  LIST   Subnodes;
  SYMBOL FirstDomain;

  cont_Check();

  FirstDomain = symbol_FirstIndexVariable();
  cont_CreateBinding(Context, FirstDomain, Context, Term);

  for (Found = FALSE, Subnodes = StIndex->subnodes;
       list_Exist(Subnodes);
       Subnodes = list_Cdr(Subnodes)) {
    st_INDEX CurrentNode;

    CurrentNode = (st_INDEX)list_Car(Subnodes);

    cont_StartBinding();

    if (subst_Variation(Context, CurrentNode->subst)) {
      list_Rplaca(Subnodes, st_EntryDeleteHelp(Context, CurrentNode, Pointer, &Found));

      if (Found) {
	StIndex->subnodes = list_PointerDeleteElement(StIndex->subnodes, NULL);

	if (list_Exist(StIndex->subnodes)) {
	  CurrentNode       = (st_INDEX)list_Car(StIndex->subnodes);
	  st_SetMax(StIndex, st_Max(CurrentNode));
	  st_SetMin(StIndex, st_Min(CurrentNode));

	  for (Subnodes = list_Cdr(StIndex->subnodes);
	       list_Exist(Subnodes);
	       Subnodes = list_Cdr(Subnodes)) {
	    CurrentNode = (st_INDEX)list_Car(Subnodes);

	    if (st_Max(CurrentNode) > st_Max(StIndex))
	      st_SetMax(StIndex, st_Max(CurrentNode));

	    if (st_Min(CurrentNode) < st_Min(StIndex))
	      st_SetMin(StIndex, st_Min(CurrentNode));
	  }
	} else {
	  st_SetMax(StIndex, 0);
	  st_SetMin(StIndex, 0);
	}

	break;
      }
    }

    cont_BackTrack();
  }

  cont_Reset();

  return Found;
}


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  RETRIEVAL FOR TERMS                                   * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


static LIST st_TraverseTreeUnifier(VARCONT IndexContext, st_INDEX StIndex)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  int      Save;
  LIST     Result, CurrentList;
  st_INDEX CurrentNode;

  /* PREPARE TRAVERSAL */
  Save = stack_Bottom();

  Result      = list_Nil();
  CurrentList = StIndex->subnodes;
  
  cont_StartBinding();

  for (;;) {
    
    /* BACKTRACK A BIG STEP */
    if (list_Empty(CurrentList)) {
      cont_StopAndBackTrack();

      if (stack_Empty(Save))
	return Result;

      CurrentList = stack_PopResult();
    }
    
    /* DESCENDING */
    for (CurrentNode = (st_INDEX)list_Car(CurrentList);
	 subst_Unify(IndexContext, CurrentNode->subst);
	 CurrentList = CurrentNode->subnodes,
	 CurrentNode = (st_INDEX)list_Car(CurrentList))
      if (st_IsLeaf(CurrentNode)) {
	Result = list_Append(CurrentNode->entries, Result);
	break;
      } else if (list_Exist(list_Cdr(CurrentList))) {
	stack_Push(list_Cdr(CurrentList));
	cont_StartBinding();
      } else
	cont_StopAndStartBinding();
    
    /* BACKTRACK LEAF OR INNER NODE */
    CurrentList = list_Cdr(CurrentList);
    cont_BackTrackAndStart();
  }
}


static LIST st_TraverseTreeGen(VARCONT IndexContext, st_INDEX StIndex)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  int      Save;
  LIST     Result, CurrentList;
  st_INDEX CurrentNode;

  /* PREPARE TRAVERSAL */
  Save = stack_Bottom();

  Result      = list_Nil();
  CurrentList = StIndex->subnodes;

  cont_StartBinding();
  
  for (;;) {
    
    /* BACKTRACK A BIG STEP */
    if (list_Empty(CurrentList)) {
      cont_StopAndBackTrack();

      if (stack_Empty(Save))
	return Result;

      CurrentList = stack_PopResult();
    }
    
    /* DESCENDING */
    for (CurrentNode = (st_INDEX)list_Car(CurrentList);
	 subst_Match(IndexContext, CurrentNode->subst);
	 CurrentList = CurrentNode->subnodes,
	 CurrentNode = (st_INDEX)list_Car(CurrentList))
      if (st_IsLeaf(CurrentNode)) {
	Result = list_Append(CurrentNode->entries, Result);
	break;
      } else if (list_Cdr(CurrentList)) {
	stack_Push(list_Cdr(CurrentList));
	cont_StartBinding();
      } else
	cont_StopAndStartBinding();
    
    /* BACKTRACK LEAF OR INNER NODE */
    CurrentList = list_Cdr(CurrentList);
    cont_BackTrackAndStart();
  }
}


static LIST st_TraverseTreeInstance(VARCONT IndexContext, st_INDEX StIndex)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  int      Save;
  LIST     Result, CurrentList;
  st_INDEX CurrentNode;

  /* PREPARE TRAVERSAL */
  Save = stack_Bottom();

  Result      = list_Nil();
  CurrentList = StIndex->subnodes;

  cont_StartBinding();

  for (;;) {
    
    /* BACKTRACK A BIG STEP */
    if (list_Empty(CurrentList)) {
      cont_StopAndBackTrack();

      if (stack_Empty(Save))
	return Result;

      CurrentList = stack_PopResult();
    }
    
    /* DESCENDING */
    for (CurrentNode = (st_INDEX)list_Car(CurrentList);
	 subst_MatchReverse(IndexContext, CurrentNode->subst);
	 CurrentList = CurrentNode->subnodes,
	 CurrentNode = (st_INDEX)list_Car(CurrentList))
      if (st_IsLeaf(CurrentNode)) {
	Result = list_Append(CurrentNode->entries, Result);
	break;
      } else if (list_Cdr(CurrentList)) {
	stack_Push(list_Cdr(CurrentList));
	cont_StartBinding();
      } else
	cont_StopAndStartBinding();
    
    /* BACKTRACK LEAF OR INNER NODE */
    CurrentList = list_Cdr(CurrentList);
    cont_BackTrackAndStart();
  }
}


static LIST st_TraverseTreeGenPreTest(VARCONT IndexContext,
				      st_INDEX StIndex,
				      st_MINMAX MinMax)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  int      Save;
  LIST     Result, CurrentList;
  st_INDEX CurrentNode;

  /* PREPARE TRAVERSAL */
  Save = stack_Bottom();

  Result      = list_Nil();
  CurrentList = StIndex->subnodes;

  cont_StartBinding();

  for (;;) {
    
    /* BACKTRACK A BIG STEP */
    if (list_Empty(CurrentList)) {
      cont_StopAndBackTrack();

      if (stack_Empty(Save))
	return Result;

      CurrentList = stack_PopResult();
    }
    
    /* DESCENDING */
    for (CurrentNode = (st_INDEX)list_Car(CurrentList);
	 (MinMax >= st_Min(CurrentNode)) &&
	   subst_Match(IndexContext, CurrentNode->subst);
	 CurrentList = CurrentNode->subnodes,
	 CurrentNode = (st_INDEX)list_Car(CurrentList))
      if (st_IsLeaf(CurrentNode)) {
	Result = list_Append(CurrentNode->entries, Result);
	break;
      } else if (list_Cdr(CurrentList)) {
	stack_Push(list_Cdr(CurrentList));
    	cont_StartBinding();
      } else
	cont_StopAndStartBinding();

    /* BACKTRACK LEAF OR INNER NODE */
    CurrentList = list_Cdr(CurrentList);
    cont_BackTrackAndStart();
  }
}


static LIST st_TraverseTreeInstancePreTest(VARCONT IndexContext, st_INDEX StIndex, st_MINMAX MinMax)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  int      Save;
  LIST     Result, CurrentList;
  st_INDEX CurrentNode;

  /* PREPARE TRAVERSAL */
  Save = stack_Bottom();

  Result      = list_Nil();
  CurrentList = StIndex->subnodes;

  cont_StartBinding();

  for (;;) {
    
    /* BACKTRACK A BIG STEP */
    if (list_Empty(CurrentList)) {
      cont_StopAndBackTrack();

      if (stack_Empty(Save))
	return Result;

      CurrentList = stack_PopResult();
    }
    
    /* DESCENDING */
    for (CurrentNode = (st_INDEX)list_Car(CurrentList);
	 (MinMax <= st_Max(CurrentNode)) &&
	   subst_MatchReverse(IndexContext, CurrentNode->subst);
	 CurrentList = CurrentNode->subnodes,
	 CurrentNode = (st_INDEX)list_Car(CurrentList))
      if (st_IsLeaf(CurrentNode)) {
	Result = list_Append(CurrentNode->entries, Result);
	break;
      } else if (list_Cdr(CurrentList)) {
	stack_Push(list_Cdr(CurrentList));
    	cont_StartBinding();
      } else
	cont_StopAndStartBinding();

    /* BACKTRACK LEAF OR INNER NODE */
    CurrentList = list_Cdr(CurrentList);
    cont_BackTrackAndStart();
  }
}


LIST st_GetUnifier(VARCONT IndexContext, st_INDEX StIndex, VARCONT TermContext, TERM Term)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST   Result;
  SYMBOL FirstDomain;

  cont_Check();

  FirstDomain = symbol_FirstIndexVariable();
  cont_CreateBinding(IndexContext, FirstDomain, TermContext, Term);

  Result = st_TraverseTreeUnifier(IndexContext, StIndex);

  cont_Reset();
  
  return Result;
}


LIST st_GetGen(VARCONT IndexContext, st_INDEX StIndex, TERM Term)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST   Result;
  SYMBOL FirstDomain;

  cont_Check();

  FirstDomain = symbol_FirstIndexVariable();
  cont_CreateBinding(IndexContext, FirstDomain, cont_InstanceContext(), Term);

  Result = st_TraverseTreeGen(IndexContext, StIndex);

  cont_Reset();
  
  return Result;
}


LIST st_GetGenPreTest(VARCONT IndexContext, st_INDEX StIndex, TERM Term)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST   Result;
  SYMBOL FirstDomain;

  cont_Check();

  FirstDomain = symbol_FirstIndexVariable();
  cont_CreateBinding(IndexContext, FirstDomain, cont_InstanceContext(), Term);

  Result = st_TraverseTreeGenPreTest(IndexContext, StIndex, term_ComputeSize(Term));

  cont_Reset();
  
  return Result;
}


LIST st_GetInstance(VARCONT IndexContext, st_INDEX StIndex, TERM Term)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST   Result;
  SYMBOL FirstDomain;

  cont_Check();

  FirstDomain = symbol_FirstIndexVariable();
  cont_CreateBinding(IndexContext, FirstDomain, IndexContext, Term);

  Result = st_TraverseTreeInstance(IndexContext, StIndex);

  cont_Reset();
  
  return Result;
}


LIST st_GetInstancePreTest(VARCONT IndexContext, st_INDEX StIndex, TERM Term)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST   Result;
  SYMBOL FirstDomain;

  cont_Check();

  FirstDomain = symbol_FirstIndexVariable();
  cont_CreateBinding(IndexContext, FirstDomain, IndexContext, Term);

  Result = st_TraverseTreeInstancePreTest(IndexContext, StIndex, term_ComputeSize(Term));

  cont_Reset();
  
  return Result;
}


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  INCREMENTAL RETRIEVAL                                 * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


static POINTER st_TraverseForExistUnifier(VARCONT IndexContext)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST     CurrentList;
  st_INDEX CurrentNode;

  /* Caution: In case an entry is found
     the procedure returns immediately
     without backtracking the current bindings. */

  CurrentList = list_Nil();
  
  for (;;) {
    
    /* BACKTRACK A BIG STEP */
    if (list_Empty(CurrentList)) {
      cont_StopAndBackTrack();

      if (st_StackEmpty(st_STACKSAVE))
	return NULL;

      CurrentList = st_StackPopResult();
    }
    
    /* DESCENDING */
    for (CurrentNode = (st_INDEX)list_Car(CurrentList);
	 subst_Unify(IndexContext, CurrentNode->subst);
	 CurrentList = CurrentNode->subnodes,
	 CurrentNode = (st_INDEX)list_Car(CurrentList)) {
      if (list_Exist(list_Cdr(CurrentList))) {
	st_StackPush(list_Cdr(CurrentList));
	cont_StartBinding();
      } else
	cont_StopAndStartBinding();
    
      if (st_IsLeaf(CurrentNode)) {
	st_StackPush(list_Cdr(CurrentNode->entries));
	return list_Car(CurrentNode->entries);
      }
    }
    
    /* BACKTRACK LEAF OR INNER NODE */
    CurrentList = list_Cdr(CurrentList);
    cont_BackTrackAndStart();
  }
}


static POINTER st_TraverseForExistGen(VARCONT IndexContext)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST     CurrentList;
  st_INDEX CurrentNode;

  /* Caution: In case an entry is found
     the procedure returns immediately
     without backtracking the current bindings. */

  CurrentList = list_Nil();
  
  for (;;) {
    
    /* BACKTRACK A BIG STEP */
    if (list_Empty(CurrentList)) {
      cont_StopAndBackTrack();

      if (st_StackEmpty(st_STACKSAVE))
	return NULL;

      CurrentList = st_StackPopResult();
    }
    
    /* DESCENDING */
    for (CurrentNode = (st_INDEX)list_Car(CurrentList);
	 subst_Match(IndexContext, CurrentNode->subst);
	 CurrentList = CurrentNode->subnodes,
	 CurrentNode = (st_INDEX)list_Car(CurrentList)) {
      if (list_Exist(list_Cdr(CurrentList))) {
	st_StackPush(list_Cdr(CurrentList));
	cont_StartBinding();
      } else
	cont_StopAndStartBinding();

      if (st_IsLeaf(CurrentNode)) {
	st_StackPush(list_Cdr(CurrentNode->entries));
	return list_Car(CurrentNode->entries);
      }
    }
    
    /* BACKTRACK LEAF OR INNER NODE */
    CurrentList = list_Cdr(CurrentList);
    cont_BackTrackAndStart();
  }
}


static POINTER st_TraverseForExistGenPreTest(VARCONT IndexContext)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST     CurrentList;
  st_INDEX CurrentNode;

  /* Caution: In case an entry is found
     the procedure returns immediately
     without backtracking the current bindings. */

  CurrentList = list_Nil();
  
  for (;;) {
    
    /* BACKTRACK A BIG STEP */
    if (list_Empty(CurrentList)) {
      cont_StopAndBackTrack();

      if (st_StackEmpty(st_STACKSAVE))
	return NULL;

      CurrentList = st_StackPopResult();
    }
    
    /* DESCENDING */
    for (CurrentNode = (st_INDEX)list_Car(CurrentList);
	 (st_EXIST_MINMAX >= st_Min(CurrentNode)) &&
	   subst_Match(IndexContext, CurrentNode->subst);
	 CurrentList = CurrentNode->subnodes,
	 CurrentNode = (st_INDEX)list_Car(CurrentList)) {
      if (list_Exist(list_Cdr(CurrentList))) {
	st_StackPush(list_Cdr(CurrentList));
	cont_StartBinding();
      } else
	cont_StopAndStartBinding();

      if (st_IsLeaf(CurrentNode)) {
	st_StackPush(list_Cdr(CurrentNode->entries));
	return list_Car(CurrentNode->entries);
      }
    }
    
    /* BACKTRACK LEAF OR INNER NODE */
    CurrentList = list_Cdr(CurrentList);
    cont_BackTrackAndStart();
  }
}


static POINTER st_TraverseForExistInstance(VARCONT IndexContext)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST     CurrentList;
  st_INDEX CurrentNode;

  /* Caution: In case an entry is found
     the procedure returns immediately
     without backtracking the current bindings. */

  CurrentList = list_Nil();
  
  for (;;) {
    
    /* BACKTRACK A BIG STEP */
    if (list_Empty(CurrentList)) {
      cont_StopAndBackTrack();

      if (st_StackEmpty(st_STACKSAVE))
	return NULL;

      CurrentList = st_StackPopResult();
    }
    
    /* DESCENDING */
    for (CurrentNode = (st_INDEX)list_Car(CurrentList);
	 subst_MatchReverse(IndexContext, CurrentNode->subst);
	 CurrentList = CurrentNode->subnodes,
	 CurrentNode = (st_INDEX)list_Car(CurrentList)) {
      if (list_Exist(list_Cdr(CurrentList))) {
	st_StackPush(list_Cdr(CurrentList));
	cont_StartBinding();
      } else
	cont_StopAndStartBinding();

      if (st_IsLeaf(CurrentNode)) {
	st_StackPush(list_Cdr(CurrentNode->entries));
	return list_Car(CurrentNode->entries);
      }
    }
    
    /* BACKTRACK LEAF OR INNER NODE */
    CurrentList = list_Cdr(CurrentList);
    cont_BackTrackAndStart();
  }
}


static POINTER st_TraverseForExistInstancePreTest(VARCONT IndexContext)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST     CurrentList;
  st_INDEX CurrentNode;

  /* Caution: In case an entry is found
     the procedure returns immediately
     without backtracking the current bindings. */

  CurrentList = list_Nil();
  
  for (;;) {
    
    /* BACKTRACK A BIG STEP */
    if (list_Empty(CurrentList)) {
      cont_StopAndBackTrack();

      if (st_StackEmpty(st_STACKSAVE))
	return NULL;

      CurrentList = st_StackPopResult();
    }
    
    /* DESCENDING */
    for (CurrentNode = (st_INDEX)list_Car(CurrentList);
	 (st_EXIST_MINMAX <= st_Max(CurrentNode)) &&
	     subst_MatchReverse(IndexContext, CurrentNode->subst);
	 CurrentList = CurrentNode->subnodes,
	 CurrentNode = (st_INDEX)list_Car(CurrentList)) {
      if (list_Exist(list_Cdr(CurrentList))) {
	st_StackPush(list_Cdr(CurrentList));
	cont_StartBinding();
      } else
	cont_StopAndStartBinding();

      if (st_IsLeaf(CurrentNode)) {
	st_StackPush(list_Cdr(CurrentNode->entries));
	return list_Car(CurrentNode->entries);
      }
    }
    
    /* BACKTRACK LEAF OR INNER NODE */
    CurrentList = list_Cdr(CurrentList);
    cont_BackTrackAndStart();
  }
}


void st_CancelExistRetrieval(void)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  if (st_CURRENT_RETRIEVAL != st_NOP) {

#ifdef CHECK
    cont_CheckState();
#endif

    st_StackSetBottom(st_STACKSAVE);

    switch (st_WHICH_VARCONTS) {

    case st_STANDARD:
      cont_Reset();
      break;

    case st_NOC:
      break;

    default:
      misc_StartErrorReport();
      misc_ErrorReport("\n In st_CancelExistRetrieval: Unknown context type.\n");
      misc_FinishErrorReport();
    }

    st_CURRENT_RETRIEVAL = st_NOP;
    st_WHICH_VARCONTS    = st_NOC;
    st_INDEX_VARCONT     = NULL;
    st_EXIST_MINMAX      = 0;
  }
}


POINTER st_ExistUnifier(VARCONT IndexContext, st_INDEX StIndex, VARCONT TermContext, TERM Term)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  SYMBOL  FirstDomain;
  POINTER Result;

#ifdef CHECK
  if (!st_StackEmpty(st_STACKSAVE)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In st_ExistUnifier: ST-Stack not empty.\n");
    misc_FinishErrorReport();
  } else if (st_CURRENT_RETRIEVAL != st_NOP) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In st_ExistUnifier: %d Retrieval already in progress.\n",
		     st_CURRENT_RETRIEVAL);
    misc_FinishErrorReport();
  }
#endif

  cont_Check();

  if (st_Exist(StIndex)) {

    st_CURRENT_RETRIEVAL = st_UNIFIER;
    st_WHICH_VARCONTS    = st_STANDARD;
    st_INDEX_VARCONT     = IndexContext;

    st_STACKSAVE = st_StackBottom();

    FirstDomain = symbol_FirstIndexVariable();
    cont_CreateBinding(IndexContext, FirstDomain, TermContext, Term);

    cont_StartBinding();
    st_StackPush(StIndex->subnodes);
    cont_StartBinding();

    Result = st_TraverseForExistUnifier(IndexContext);

#ifdef CHECK
    cont_SaveState();
#endif

    if (Result == NULL)
      st_CancelExistRetrieval();

    return Result;
  } else
    return NULL;
}


POINTER st_ExistGen(VARCONT IndexContext, st_INDEX StIndex, TERM Term)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  SYMBOL  FirstDomain;
  POINTER Result;

#ifdef CHECK
  if (!st_StackEmpty(st_STACKSAVE)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In st_ExistGen: ST-Stack not empty.\n");
    misc_FinishErrorReport();
  } 
  else 
    if (st_CURRENT_RETRIEVAL != st_NOP) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In st_ExistGen: %d Retrieval already in progress.\n",
		       st_CURRENT_RETRIEVAL);
      misc_FinishErrorReport();
    }
#endif

  cont_Check();

  if (st_Exist(StIndex)) {

    st_CURRENT_RETRIEVAL = st_GEN;
    st_WHICH_VARCONTS    = st_STANDARD;
    st_INDEX_VARCONT     = IndexContext;
  
    st_STACKSAVE = st_StackBottom();

    FirstDomain = symbol_FirstIndexVariable();
    cont_CreateBinding(IndexContext, FirstDomain, cont_InstanceContext(), Term);

    cont_StartBinding();
    st_StackPush(StIndex->subnodes);
    cont_StartBinding();

    Result = st_TraverseForExistGen(IndexContext);

#ifdef CHECK
    cont_SaveState();
#endif

    if (Result == NULL)
      st_CancelExistRetrieval();

    return Result;
  } else
    return NULL;
}


POINTER st_ExistGenPreTest(VARCONT IndexContext, st_INDEX StIndex, TERM Term)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  SYMBOL  FirstDomain;
  POINTER Result;

#ifdef CHECK
  if (!st_StackEmpty(st_STACKSAVE)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In st_ExistGenPreTest: ST-Stack not empty.\n");
    misc_FinishErrorReport();
  } 
  else 
    if (st_CURRENT_RETRIEVAL != st_NOP) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In st_ExistGenPreTest: %d Retrieval already in progress.\n",
		       st_CURRENT_RETRIEVAL);
      misc_FinishErrorReport();
    }
#endif

  cont_Check();

  if (st_Exist(StIndex)) {

    st_CURRENT_RETRIEVAL = st_GENPRETEST;
    st_WHICH_VARCONTS    = st_STANDARD;
    st_INDEX_VARCONT     = IndexContext;
  
    st_STACKSAVE = st_StackBottom();


    FirstDomain     = symbol_FirstIndexVariable();
    st_EXIST_MINMAX = term_ComputeSize(Term);
    cont_CreateBinding(IndexContext, FirstDomain, IndexContext, Term);

    cont_StartBinding();
    st_StackPush(StIndex->subnodes);
    cont_StartBinding();

    Result = st_TraverseForExistGenPreTest(IndexContext);

#ifdef CHECK
    cont_SaveState();
#endif

    if (Result == NULL)
      st_CancelExistRetrieval();

    return Result;
  } else
    return NULL;
}


POINTER st_ExistInstance(VARCONT IndexContext, st_INDEX StIndex, TERM Term)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  SYMBOL  FirstDomain;
  POINTER Result;

#ifdef CHECK
  if (!st_StackEmpty(st_STACKSAVE)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In st_ExistInstance: ST-Stack not empty.\n");
    misc_FinishErrorReport();
  } 
  else 
    if (st_CURRENT_RETRIEVAL != st_NOP) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In st_ExistInstance: %d Retrieval already in progress.\n",
		       st_CURRENT_RETRIEVAL);
      misc_FinishErrorReport();
    }
#endif

  cont_Check();

  if (st_Exist(StIndex)) {

    st_CURRENT_RETRIEVAL = st_INSTANCE;
    st_WHICH_VARCONTS    = st_STANDARD;
    st_INDEX_VARCONT     = IndexContext;

    st_STACKSAVE = st_StackBottom();

    FirstDomain = symbol_FirstIndexVariable();
    cont_CreateBinding(IndexContext, FirstDomain, IndexContext, Term);

    cont_StartBinding();
    st_StackPush(StIndex->subnodes);
    cont_StartBinding();

    Result = st_TraverseForExistInstance(IndexContext);

#ifdef CHECK
    cont_SaveState();
#endif

    if (Result == NULL)
      st_CancelExistRetrieval();

    return Result;
  } else
    return NULL;
}


POINTER st_ExistInstancePreTest(VARCONT IndexContext, st_INDEX StIndex, TERM Term)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  SYMBOL  FirstDomain;
  POINTER Result;

#ifdef CHECK
  if (!st_StackEmpty(st_STACKSAVE)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In st_ExistInstancePreTest: ST-Stack not empty.\n");
    misc_FinishErrorReport();
  } 
  else 
    if (st_CURRENT_RETRIEVAL != st_NOP) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In st_ExistInstancePreTest: %d Retrieval already in progress.\n",
		       st_CURRENT_RETRIEVAL);
      misc_FinishErrorReport();
    }
#endif

  cont_Check();

  if (st_Exist(StIndex)) {

    st_CURRENT_RETRIEVAL = st_INSTANCEPRETEST;
    st_WHICH_VARCONTS    = st_STANDARD;
    st_INDEX_VARCONT     = IndexContext;

    st_STACKSAVE = st_StackBottom();

    FirstDomain     = symbol_FirstIndexVariable();
    st_EXIST_MINMAX = term_ComputeSize(Term);
    cont_CreateBinding(IndexContext, FirstDomain, IndexContext, Term);

    cont_StartBinding();
    st_StackPush(StIndex->subnodes);
    cont_StartBinding();

    Result = st_TraverseForExistInstancePreTest(IndexContext);

#ifdef CHECK
    cont_SaveState();
#endif

    if (Result == NULL)
      st_CancelExistRetrieval();

    return Result;
  } else
    return NULL;
}


POINTER st_NextCandidate(void)
/**************************************************************
  INPUT:   
  RETURNS: 
  EFFECTS: 
***************************************************************/
{
  LIST Result;

#ifdef CHECK
  if (st_StackEmpty(st_STACKSAVE)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In st_NextCandidate: ST-Stack empty.\n");
    misc_FinishErrorReport();
  } 
  else 
    if (st_CURRENT_RETRIEVAL == st_NOP) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In st_NextCandidate: No retrieval in progress.\n");
      misc_FinishErrorReport();
    }

  cont_CheckState();
#endif

  Result = st_StackPopResult();

  if (list_Exist(Result)) {
    st_StackPush(list_Cdr(Result));
#ifdef CHECK
    cont_SaveState();
#endif
    return list_Car(Result);
  } else {
    POINTER NewResult;

    NewResult = NULL;

    if (st_WHICH_VARCONTS == st_STANDARD)
      switch (st_CURRENT_RETRIEVAL) {

      case st_UNIFIER:
	NewResult = st_TraverseForExistUnifier(st_INDEX_VARCONT);
	break;

      case st_GEN:
	NewResult = st_TraverseForExistGen(st_INDEX_VARCONT);
	break;

      case st_GENPRETEST:
	NewResult = st_TraverseForExistGenPreTest(st_INDEX_VARCONT);
	break;

      case st_INSTANCE:
	NewResult = st_TraverseForExistInstance(st_INDEX_VARCONT);
	break;

      case st_INSTANCEPRETEST:
	NewResult = st_TraverseForExistInstancePreTest(st_INDEX_VARCONT);

      default:
	misc_StartErrorReport();
	misc_ErrorReport("\n In st_NextCandidate: Unknown retrieval type.\n");
	misc_FinishErrorReport();
      }
    else {
      misc_StartErrorReport();
      misc_ErrorReport("\n In st_NextCandidate: Unknown context type.\n");
      misc_FinishErrorReport();
    }

#ifdef CHECK
    cont_SaveState();
#endif

    if (NewResult == NULL)
      st_CancelExistRetrieval();

    return NewResult;
  }
}


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  OUTPUT                                                * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


static void st_PrintHelp(st_INDEX St, int Position, void (*Print)(POINTER))
/**************************************************************
  INPUT:   A node of an St, an indention and a print
           function for the entries.
  SUMMARY: Prints an St starting at node St.
***************************************************************/
{
  int i;
  
  if (St == (st_INDEX)NULL)
    return;
  
  for (i = 0; i < Position; i++)
    putchar(' ');
  puts("|");
  
  for (i = 0; i < Position; i++)
    putchar(' ');
  fputs("+-", stdout);

  printf(" Max: %d, Min: %d, ", st_Max(St), st_Min(St));
  subst_Print(St->subst);
  putchar('\n');

  if (st_IsLeaf(St)) {

    LIST Scan;

    for (i = 0; i < Position; i++)
      putchar(' ');
    fputs("  =>", stdout);

    if (Print)
      for (Scan = St->entries; Scan != NULL; Scan = list_Cdr(Scan)) {
	putchar(' ');
	Print(list_Car(Scan));
      }
    else
      printf(" %zu Entries", list_Length(St->entries));

    putchar('\n');
    
  } else {
    LIST Scan;

    for (Scan = St->subnodes; Scan != NULL; Scan = list_Cdr(Scan))
      st_PrintHelp(list_Car(Scan), Position + 2, Print);
  }
}


void st_Print(st_INDEX StIndex, void (*Print)(POINTER))
/**************************************************************
  INPUT:   The root of an St.
  SUMMARY: Prints a whole St.
***************************************************************/
{
  LIST Scan;

  if (st_Empty(StIndex)) {
    puts("\n\nIndex empty.");
    return;
  }

  fputs("\n\nroot: ", stdout);
  printf(" Max: %d, Min: %d, ", st_Max(StIndex), st_Min(StIndex));
  subst_Print(StIndex->subst);
  putchar('\n');
  if (st_IsLeaf(StIndex)) {
    fputs("  =>", stdout);

    if (Print)
      for (Scan = StIndex->entries; Scan != NULL; Scan = list_Cdr(Scan)) {
	putchar(' ');
	Print(list_Car(Scan));
      }
    else
      printf(" %zu Entries", list_Length(StIndex->entries));

  } else
    for (Scan = StIndex->subnodes; Scan != NULL; Scan = list_Cdr(Scan))
      st_PrintHelp(list_Car(Scan),2, Print);
  puts("\n");
}
