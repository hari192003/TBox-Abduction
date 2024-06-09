/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                     Definitions                        * */
/* *                                                        * */
/* *  $Module:   DEFS                                       * */ 
/* *                                                        * */
/* *  Copyright (C) 1998, 1999, 2000, 2001                  * */
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
/* $Revision: 1.3 $                                     * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-22 12:16:59 $                             * */
/* $Author: weidenb $                                         * */
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


/* $RCSfile: defs.h,v $ */

#ifndef _DEFS_
#define _DEFS_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/


#include "clause.h"
#include "term.h"
#include "list.h"
#include "search.h"

/**************************************************************/
/* Structures                                                 */
/**************************************************************/
typedef enum { PREDOCCURONCE = 1, /* whether predicate occurs only once */
               ISEQUALITY    = 2  /* whether predicate is equality */
} DEF_ATTRIBUTES;

typedef struct DEF_HELP {
  TERM   expansion;        /* The definition of the predicate term*/
  TERM   predicate;        /* The predicate which is defined*/
  TERM   toprove;
  LIST   parentclauses;    /* List of clause numbers plus list of literal indices */
  const char *label;            /* The label of the predicate term*/
  BOOL   conjecture;
  NAT    attributes;       /* The attributes of the predicate*/
} *DEF, DEF_NODE;

/**************************************************************/
/* Inline functions                                           */
/**************************************************************/
TERM def_Expansion(DEF D);

void def_RplacExp(DEF D, TERM E);

TERM def_Predicate(DEF D);

void def_RplacPred(DEF D, TERM Pred);

TERM def_ToProve(DEF D);

void def_RplacToProve(DEF D, TERM ToProve);

LIST def_ClauseNumberList(DEF D);

void def_RplacClauseNumberList(DEF D, LIST L);

LIST def_ClauseLitsList(DEF D);

void def_RplacClauseLitsList(DEF D, LIST L);

const char* def_Label(DEF D);

void def_RplacLabel(DEF D, const char* L);

BOOL def_Conjecture(DEF D);

void def_SetConjecture(DEF D);

void def_AddAttribute(DEF D, DEF_ATTRIBUTES Attribute);

NAT def_HasAttribute(DEF D, DEF_ATTRIBUTES Attribute);

void def_RemoveAttribute(DEF D, DEF_ATTRIBUTES Attribute);

BOOL def_HasGuard(DEF D);

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

DEF  def_CreateFromClauses(TERM, TERM, LIST, LIST, BOOL);
DEF  def_CreateFromTerm(TERM, TERM, TERM, const char*);

LIST def_ExtractDefsFromTerm(TERM, const char*);
void def_ExtractDefsFromTermlist(PROOFSEARCH, LIST, LIST);
void def_ExtractDefsFromClauselist(PROOFSEARCH, LIST);

TERM def_ApplyDefToTermOnce(DEF, TERM, FLAGSTORE, PRECEDENCE, BOOL*);
TERM def_ApplyDefToTermExhaustive(PROOFSEARCH, TERM);
LIST def_ApplyDefToTermlist(DEF, LIST, FLAGSTORE, PRECEDENCE, BOOL*, BOOL);
LIST def_ApplyDefinitionToTermList(LIST, LIST, FLAGSTORE, PRECEDENCE);
/*
LIST def_GetTermsForProof(TERM, TERM, int);
BOOL def_FindProofForGuard(TERM, TERM, TERM, FLAGSTORE);
*/

LIST def_ApplyDefToClauseOnce(DEF, CLAUSE, FLAGSTORE, PRECEDENCE);
LIST def_ApplyDefToClauseExhaustive(PROOFSEARCH, CLAUSE);
LIST def_ApplyDefToClauselist(PROOFSEARCH, DEF, LIST, BOOL);

LIST def_FlattenWithOneDefinition(PROOFSEARCH, DEF);
void def_FlattenWithOneDefinitionSemiDestructive(PROOFSEARCH, DEF);
void def_FlattenWithOneDefinitionDestructive(PROOFSEARCH, DEF);
void def_FlattenDefinitionsDestructive(PROOFSEARCH);

void def_Delete(DEF);
void def_Print(DEF);

int  def_PredicateOccurrences(TERM, SYMBOL);
#endif
