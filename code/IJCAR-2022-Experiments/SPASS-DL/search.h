/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *          REPRESENTATION OF PROOF SEARCH                * */
/* *                                                        * */
/* *  $Module:   PROOF SEARCH                               * */ 
/* *                                                        * */
/* *  Copyright (C) 1997, 1998, 1999, 2000                  * */
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
/* $Revision: 1.9 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-11-27 12:58:26 $                             * */
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


/* $RCSfile: search.h,v $ */

#ifndef _PROOFSEARCH_
#define _PROOFSEARCH_

#include "clause.h"
#include "sort.h"
#include "analyze.h"

/**************************************************************/
/* Data Structures and Constants                              */
/**************************************************************/

/* <isleft>:         true iff the split is a left branch         */
/* <blockedClauses>: list of (unshared) clauses containing the   */
/*                   "remainder" of the clause splitted at this  */
/*                   level and the negation of the first branch  */
/*                   if this branch created a ground clause.     */ 
/*                   The right clause has clause number 0, and   */
/*                   the negation clauses have number -1.        */
/* <deletedClauses>: list of (unshared) clauses made redundant   */
/*                   by a clause of this level. The split level  */
/*                   of these clauses may be above or below the  */
/*                   current level, but not equal to the current */
/*                   level.                                      */
/* <father>:         the unshared clause that was splitted.      */
/* <leftSplitfield>: the used splits of the left branch          */
/*                   corresponding to current split if it is a   */
/*                   right branch; not used for left splits      */
/* <rightSplitfield>: the used splits of the current split if it */
/*                   is a right branch; not used for left splits */
typedef struct {
  /* == 0 -> TOPLEVEL, 1,2,... */
  int     splitlevel;
  BOOL    isleft;
  BOOL    used;
  LIST    blockedClauses, deletedClauses;
  CLAUSE  father;
  SPLITFIELD leftSplitfield, rightSplitfield;
  unsigned leftSplitfield_length, rightSplitfield_length;
} *SPLIT, SPLIT_NODE;


typedef struct PROOFSEARCH_HELP {
  LIST            definitions;
  LIST            emptyclauses;
  LIST            usedemptyclauses;
  LIST            finmonpreds;
  SHARED_INDEX    woindex;
  LIST            wolist;
  SHARED_INDEX    usindex;
  LIST            uslist;
  SORTTHEORY      astatic;
  SORTTHEORY      adynamic;
  SORTTHEORY      dynamic;
  SHARED_INDEX    dpindex;
  LIST            dplist;
  PRECEDENCE      precedence;
  FLAGSTORE       store;
  LIST            stack;
  int             validlevel;
  int             lastbacktrack;
  int             splitcounter;
  int             keptclauses;
  int             derivedclauses;
  int             loops;
  int             backtracked;
  ANALYZE         analyze;
} PROOFSEARCH_NODE,*PROOFSEARCH;

/* There are two sets of clauses with their respective clause list: worked-off clauses   */
/* contained in <woindex>, <wolist> and usable clauses, contained in <usindex>,<uslist>. */
/* The assoc list <finitepreds> is a list of pairs (<pred>.(<gterm1>,...,<gtermn>))      */
/* where <pred> (monadic) has (at most) the extension <gterm1>,...,<gtermn>              */
/* Three sort theories: <astatic> is the static overall approximation, only available    */
/* in a non-equality setting, <adynamic> is the dynamic approximation only considering   */
/* maximal declarations, and <dynamic> is the (not approximated) dynamic sort theory of  */
/* all maximal declarations. Clauses that are no longer needed for the search, but for   */
/* proof documentation are stored in <dpindex>, <dplist>. If <dpindex> is NULL, then     */
/* this means that no proof documentation is required.                                   */
/* A search is also heavily influenced by the used <precedence> and flag values in       */
/* store.                                                                                */
/* The next components deal with splitting: the split stack, the current level           */
/* of splitting, the last backtrack level (for branch condensing) and the overall number */
/* of splittings stored in <splitcounter>.                                               */
/* Finally some statistics is stored: the number of kept, derived clauses ...            */
/* and the clause number of some clause that implies a non-trivial domain .              */

/* The analyze object stores the results of analysis of the problem being solved.        */

/**************************************************************/
/* Inline Functions                                           */
/**************************************************************/

ANALYZE      prfs_GetAnalyze(PROOFSEARCH Prf); 
LIST         prfs_EmptyClauses(PROOFSEARCH Prf);
void         prfs_SetEmptyClauses(PROOFSEARCH Prf, LIST Clauses);
LIST         prfs_Definitions(PROOFSEARCH Prf);
void         prfs_SetDefinitions(PROOFSEARCH Prf, LIST Definitions);
LIST         prfs_UsedEmptyClauses(PROOFSEARCH Prf);
void         prfs_SetUsedEmptyClauses(PROOFSEARCH Prf, LIST Clauses);
LIST         prfs_WorkedOffClauses(PROOFSEARCH Prf);
void         prfs_SetWorkedOffClauses(PROOFSEARCH Prf, LIST Clauses);
SHARED_INDEX prfs_WorkedOffSharingIndex(PROOFSEARCH Prf);
LIST         prfs_UsableClauses(PROOFSEARCH Prf);
void         prfs_SetUsableClauses(PROOFSEARCH Prf, LIST Clauses);
SHARED_INDEX prfs_UsableSharingIndex(PROOFSEARCH Prf);
LIST         prfs_DocProofClauses(PROOFSEARCH Prf);
void         prfs_SetDocProofClauses(PROOFSEARCH Prf, LIST Clauses);
SHARED_INDEX prfs_DocProofSharingIndex(PROOFSEARCH Prf);
void         prfs_AddDocProofSharingIndex(PROOFSEARCH Prf);
LIST         prfs_GetFinMonPreds(PROOFSEARCH Prf);
void         prfs_SetFinMonPreds(PROOFSEARCH Prf, LIST Preds);
void         prfs_DeleteFinMonPreds(PROOFSEARCH Prf);
SORTTHEORY   prfs_StaticSortTheory(PROOFSEARCH Prf);
void         prfs_SetStaticSortTheory(PROOFSEARCH Prf, SORTTHEORY Theory);
SORTTHEORY   prfs_DynamicSortTheory(PROOFSEARCH Prf);
void         prfs_SetDynamicSortTheory(PROOFSEARCH Prf, SORTTHEORY Theory);
SORTTHEORY   prfs_ApproximatedDynamicSortTheory(PROOFSEARCH Prf);
void         prfs_SetApproximatedDynamicSortTheory(PROOFSEARCH Prf, SORTTHEORY Theory);
PRECEDENCE   prfs_Precedence(PROOFSEARCH Prf);
FLAGSTORE    prfs_Store(PROOFSEARCH Prf);
BOOL         prfs_SplitLevelCondition(NAT OriginLevel, NAT RedundantLevel, NAT BacktrackLevel);
BOOL         prfs_IsClauseValid(CLAUSE C, int Level);
SPLIT        prfs_GetSplitOfLevel(int L, PROOFSEARCH Prf);
LIST       prfs_SplitStack(PROOFSEARCH Prf);
SPLIT        prfs_SplitStackTop(PROOFSEARCH Prf);
void         prfs_SplitStackPop(PROOFSEARCH Prf);
void         prfs_SplitStackPush(PROOFSEARCH Prf, SPLIT S);
BOOL         prfs_SplitStackEmpty(PROOFSEARCH Prf);
int          prfs_TopLevel(void); 
int          prfs_ValidLevel(PROOFSEARCH Prf);
void         prfs_SetValidLevel(PROOFSEARCH Prf, int Value);
void         prfs_IncValidLevel(PROOFSEARCH Prf);
void         prfs_DecValidLevel(PROOFSEARCH Prf);
int          prfs_LastBacktrackLevel(PROOFSEARCH Prf);
void         prfs_SetLastBacktrackLevel(PROOFSEARCH Prf, int Value);
int          prfs_SplitCounter(PROOFSEARCH Prf);
void         prfs_SetSplitCounter(PROOFSEARCH Prf, int c);
void         prfs_IncSplitCounter(PROOFSEARCH Prf);
int          prfs_KeptClauses(PROOFSEARCH Prf);
void         prfs_IncKeptClauses(PROOFSEARCH Prf);
int          prfs_DerivedClauses(PROOFSEARCH Prf);
void         prfs_IncDerivedClauses(PROOFSEARCH Prf, int k);
int          prfs_Loops(PROOFSEARCH Prf);
void         prfs_SetLoops(PROOFSEARCH Prf, int k);
void         prfs_DecLoops(PROOFSEARCH Prf);
int          prfs_BacktrackedClauses(PROOFSEARCH Prf);
void         prfs_SetBacktrackedClauses(PROOFSEARCH Prf, int k);
void         prfs_IncBacktrackedClauses(PROOFSEARCH Prf, int k);

/**************************************************************/
/* Functions for accessing SPLIT objects                      */
/**************************************************************/

void       prfs_SplitFree(SPLIT Sp);
void       prfs_SplitDelete(SPLIT S);
LIST       prfs_SplitBlockedClauses(SPLIT S);
void       prfs_SplitAddBlockedClause(SPLIT S, CLAUSE C);
void       prfs_SplitSetBlockedClauses(SPLIT S, LIST L);
LIST       prfs_SplitDeletedClauses(SPLIT S);
void       prfs_SplitSetDeletedClauses(SPLIT S, LIST L);
int        prfs_SplitSplitLevel(SPLIT S);
BOOL       prfs_SplitIsLeft(SPLIT S);
void       prfs_SplitSetLeft(SPLIT S);
void       prfs_SplitSetRight(SPLIT S);
BOOL       prfs_SplitIsUsed(SPLIT S);
BOOL       prfs_SplitIsUnused(SPLIT S);
void       prfs_SplitSetUsed(SPLIT S);
CLAUSE     prfs_SplitFatherClause(SPLIT S); 
void       prfs_SplitSetFatherClause(SPLIT S, CLAUSE C);
SPLITFIELD prfs_LeftSplitfield(SPLIT S, unsigned *Length);
SPLITFIELD prfs_RightSplitfield(SPLIT S, unsigned *Length);
void       prfs_SplitFreeSplitfields(SPLIT S);
BOOL       prfs_ExistsSplitOfLevel(int L, PROOFSEARCH PS);
BOOL       prfs_SplitsAvailable(PROOFSEARCH PS, FLAGSTORE Store);

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

PROOFSEARCH prfs_Create(void);
BOOL        prfs_Check(PROOFSEARCH);
void        prfs_CopyIndices(PROOFSEARCH, PROOFSEARCH);
void        prfs_Delete(PROOFSEARCH);
void        prfs_DeleteDocProof(PROOFSEARCH);
void        prfs_Clean(PROOFSEARCH);
void        prfs_Print(PROOFSEARCH);
void        prfs_PrintSplit(SPLIT);
void        prfs_PrintSplitStack(PROOFSEARCH);
void        prfs_InsertWorkedOffClause(PROOFSEARCH,CLAUSE);
void        prfs_InsertUsableClause(PROOFSEARCH,CLAUSE, BOOL);
void        prfs_InsertDocProofClause(PROOFSEARCH,CLAUSE);
void        prfs_MoveUsableWorkedOff(PROOFSEARCH, CLAUSE);
void        prfs_MoveWorkedOffDocProof(PROOFSEARCH, CLAUSE);
void        prfs_MoveUsableDocProof(PROOFSEARCH, CLAUSE);
void        prfs_ExtractWorkedOff(PROOFSEARCH, CLAUSE);
void        prfs_DeleteWorkedOff(PROOFSEARCH, CLAUSE);
void        prfs_ExtractUsable(PROOFSEARCH, CLAUSE);
void        prfs_DeleteUsable(PROOFSEARCH, CLAUSE);
void        prfs_ExtractDocProof(PROOFSEARCH, CLAUSE);
void        prfs_DeleteDocProofClause(PROOFSEARCH, CLAUSE);
void        prfs_MoveInvalidClausesDocProof(PROOFSEARCH);
void        prfs_SwapIndexes(PROOFSEARCH);

void        prfs_InstallFiniteMonadicPredicates(PROOFSEARCH, LIST, LIST);

CLAUSE      prfs_PerformSplitting(PROOFSEARCH, CLAUSE);
CLAUSE      prfs_DoSplitting(PROOFSEARCH, CLAUSE, LIST);
NAT         prfs_GetNumberOfInstances(PROOFSEARCH, LITERAL, BOOL);

void        prfs_SetLeftSplitfield(SPLIT, SPLITFIELD, unsigned);
void        prfs_SetRightSplitfield(SPLIT, SPLITFIELD, unsigned);
BOOL        prfs_SplitfieldContainsLevel(SPLITFIELD, unsigned, NAT);
NAT         prfs_SplitfieldHighestLevel(SPLITFIELD, unsigned);
void        prfs_AddLevelToSplitfield(SPLITFIELD*, unsigned*, NAT);
void        prfs_RemoveLevelFromSplitfield(SPLITFIELD*, unsigned*, NAT);
BOOL        prfs_SplitfieldIsSubset(SPLITFIELD, unsigned, SPLITFIELD, unsigned);
SPLITFIELD  prfs_SplitfieldUnion(SPLITFIELD, unsigned, SPLITFIELD, unsigned, unsigned*);
SPLITFIELD  prfs_SplitfieldIntersection(SPLITFIELD, unsigned, SPLITFIELD, unsigned, unsigned*);
SPLITFIELD  prfs_CombineSplitfields(SPLITFIELD, unsigned, SPLITFIELD, unsigned, NAT, unsigned*);

#endif

