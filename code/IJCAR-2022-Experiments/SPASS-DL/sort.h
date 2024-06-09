/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                  SORTED REASONING                      * */
/* *                                                        * */
/* *  $Module:   SORT                                       * */ 
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
/* $Date: 2011-11-27 13:02:40 $                             * */
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


/* $RCSfile: sort.h,v $ */

#ifndef _SORT_
#define _SORT_

#include "clause.h"
#include "unify.h"
#include "hashmap.h"
#include "subsumption.h"


/**************************************************************/
/* Data Structures and Constants                              */
/**************************************************************/

/* Objects of type SORT are used to store the sort information */
/* for terms. SORT is a list of NODE's                        */

typedef LIST SORT;
/* List of sort nodes */

typedef LIST SOJU;
/* Pair: First Component Sort, Second Component List of Clauses */

typedef enum {SORTEQNONE=1, SORTEQDECR=2, SORTEQMANY=3} STR;
/* The overall result of an sort theory analysis with respect to equations */


typedef struct NODE_HELP {
  LIST   links;
  NAT    mark;
  NAT    start;
  NAT    extra;
  LIST   clauses;
  SYMBOL symbol;
} NODE_NODE,*NODE;

/* This is a node from the subsort graph with outgoing links the represented   */
/* sort symbol and a mark indicating whether the node is already visited,i.e,  */
/* it is TRUE. Whereas start indicates whether the node was put true right     */
/* from the beginning. Clauses are used to derive the subsort reletationship.  */


typedef struct SLINK_HELP {
  LIST   input;                    
  NODE   output;                   
  int    card;                     
  int    fire;
  SYMBOL var;
  CLAUSE clause;                   
} SLINK_NODE,*SLINK;

/* This is a link in the subsort graph, with a list of input nodes that have   */
/* to become true in order to fire the link and set the output node TRUE       */
/* constraint, antecedent, succedent literals are the extra literals of        */
/* <clause> and <var> is the subsort variable. It is always assumed that the   */
/* subsort variable is the maximal variable with respect to the constraint,    */
/* antecedent and succedent literals.                                          */

typedef struct SORTTHEORY_HELP {
  st_INDEX index;                                 
  NODE     basesorttable[symbol__MAXSIGNATURE];   
  LIST     suborigcls;
  LIST     termorigcls;
  NAT      mark;        
} SORTTHEORY_NODE,*SORTTHEORY;

/* The index contains the term declarations mapped to their possibly           */
/* approximated term declarations of inserted clauses. The subsort declarations*/
/* are handled by a specific graph initiated over the base sorts. There is     */
/* one node in the graph for every base sort (basesorttable) and links         */
/* correspond to subsort declaration clauses                                   */
/* The mark is used in the subsort graph to detect already visited nodes.      */
/* The lists suborigcls and termorigcls map original clauses to sort theory    */
/* clauses to links/terms respectively. They contain triples of this kind      */



/**************************************************************/
/* Macros                                                     */
/**************************************************************/

SORT sort_TopSort(void);

BOOL sort_IsTopSort(SORT S);

SORT sort_Copy(SORT S);

void sort_DeleteOne(SORT S);

/* static __inline__ void sort_DeleteSecond(LIST L) */
/* { */
/*   list_Delete(L); */
/* } */


SORT sort_Intersect(SORT S1, SORT S2);

SORT sort_DeleteBaseNode(SORT S, NODE N);

SORT sort_AddBaseNode(SORT S, NODE N);

void sort_NodeFree(NODE N);

BOOL sort_NodeEqual(NODE N1, NODE N2);

void sort_NodeDelete(NODE N);

LIST sort_NodeLinks(NODE N);

BOOL sort_NodeValue(SORTTHEORY S, NODE N);

BOOL sort_NodeExtraValue(SORTTHEORY S, NODE N);

BOOL sort_NodeStartValue(SORTTHEORY S, NODE N);

NAT sort_NodeMark(NODE N);

NAT sort_NodeStart(NODE N);

SYMBOL sort_NodeSymbol(NODE N);

LIST sort_NodeClauses(NODE N);

void sort_PutNodeMark(NODE N, NAT M);

void sort_PutNodeExtra(NODE N, NAT M);

void sort_PutNodeStart(NODE N, NAT S);

void sort_PutNodeSymbol(NODE N, SYMBOL S);

void sort_PutNodeLinks(NODE N, LIST C);

void sort_PutNodeClauses(NODE N, LIST C);

void sort_PutNodeTrue(SORTTHEORY S, NODE N);

void sort_PutNodeExtraTrue(SORTTHEORY S, NODE N);

void sort_PutNodeStartTrue(SORTTHEORY S, NODE N);

void sort_DeleteNodeClauses(NODE N);

LIST sort_LinkInput(SLINK S);

NODE sort_LinkOutput(SLINK S);

int sort_LinkFire(SLINK S);

int sort_LinkVar(SLINK S);

CLAUSE sort_LinkClause(SLINK S);

int sort_LinkCard(SLINK S);

void sort_PutLinkInput(SLINK S, LIST T);

void sort_PutLinkVar(SLINK S, SYMBOL V);

void sort_PutLinkOutput(SLINK S,NODE H);

void sort_PutLinkClause(SLINK S, CLAUSE C);

void sort_PutLinkCard(SLINK S,int L);

void sort_LinkResetFire(SLINK S);

int sort_LinkDecrementFire(SLINK S);

void sort_LinkFree(SLINK S);

void sort_LinkDelete(SLINK S);

BOOL sort_HasEqualSort(SORT S1, SORT S2);

POINTER sort_PairSort(LIST Pair);

POINTER sort_PairClauses(LIST Pair);

LIST sort_PairCreate(SORT S , LIST Clauses);

void sort_PairFree(LIST Pair);

void sort_PairDelete(LIST Pair);

LIST sort_PairCopy(LIST Pair);

NODE sort_TheoryNode(SORTTHEORY Theory, SYMBOL S);

NAT sort_TheoryMark(SORTTHEORY Theory);

LIST sort_TheorySuborigcls(SORTTHEORY Theory);

LIST sort_TheoryTermorigcls(SORTTHEORY Theory);

void sort_TheoryIncrementMark(SORTTHEORY Theory);

st_INDEX sort_TheoryIndex(SORTTHEORY Theory);

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

void sort_Init(void);
void sort_Free(void);

void sort_Delete(SORT);
BOOL sort_Eq(SORT, SORT);
void sort_DeleteSortPair(SOJU);

void sort_Print(SORT);
void sort_PairPrint(SOJU);

LIST sort_GetSymbolsFromSort(SORT);
BOOL sort_ContainsSymbol(SORT, SYMBOL);
BOOL sort_IsSort(SORT);


SORTTHEORY sort_ApproxStaticSortTheory(LIST, FLAGSTORE, PRECEDENCE);
SORTTHEORY sort_ApproxDynamicSortTheory(LIST);


SORTTHEORY sort_TheoryCreate(void);
void       sort_TheoryDelete(SORTTHEORY);
void       sort_TheoryPrint(SORTTHEORY);
void       sort_TheoryInsertClause(SORTTHEORY, CLAUSE, CLAUSE, LITERAL);
void       sort_TheoryDeleteClause(SORTTHEORY, CLAUSE);
SORT       sort_TheorySortOfSymbol(SORTTHEORY, SYMBOL);
BOOL       sort_TheorySortEqual(SORTTHEORY,SORT,SORT);
LIST       sort_TheoryIsSubsort(SORTTHEORY, SORT, SORT, BOOL*);
BOOL       sort_TheoryIsSubsortOf(SORTTHEORY, SORT, SORT);
BOOL       sort_TheoryIsSubsortOfExtra(SORTTHEORY , SORT , SORT , SORT);
LIST       sort_TheoryComputeAllSubsortHits(SORTTHEORY, SORT, SORT);
SOJU       sort_ComputeSort(SORTTHEORY,TERM, CLAUSE, int, FLAGSTORE, PRECEDENCE);
LIST       sort_ApproxMaxDeclClauses(CLAUSE, FLAGSTORE, PRECEDENCE);
STR        sort_AnalyzeSortStructure(LIST, FLAGSTORE, PRECEDENCE);

#endif
