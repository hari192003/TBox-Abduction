/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                   CLAUSES                              * */
/* *                                                        * */
/* *  $Module:   CLAUSE                                     * */
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
/* $Revision: 1.18 $                                        * */
/* $State: Exp $                                            * */
/* $Date: 2011-11-27 10:57:34 $                             * */
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


/* $RCSfile: clause.c,v $ */

#include "clause.h"

/**************************************************************/
/* Global variables and constants                             */
/**************************************************************/

/* Means weight of literal or clause is undefined */
const NAT clause_WEIGHTUNDEFINED = NAT_MAX;

int  clause_CLAUSECOUNTER;
NAT  clause_STAMPID;

/* The following array is used for bucket sort on clauses */
#define clause_MAXWEIGHT 20
LIST clause_SORT[clause_MAXWEIGHT+1];
/**************************************************************/
/*Previously Inlined functions                                */ 
/**************************************************************/
LIST clause_CopyClauseList(LIST List)
{
  return list_CopyWithElement(List, (POINTER (*)(POINTER)) clause_Copy);
}

BOOL clause_IsTransitivityAxiom(CLAUSE Clause) 
{
  SYMBOL s;
  BOOL b;
  return clause_IsTransitivityAxiomExt(Clause,&s,&b);
}

/**************************************************************/
/* Accessing Literals 1                                       */
/**************************************************************/

 TERM clause_LiteralSignedAtom(LITERAL L)
{
  return L->atomWithSign;
}


 CLAUSE clause_LiteralOwningClause(LITERAL L)
{
  return L->owningClause;
}

 void clause_LiteralSetOwningClause(LITERAL L, CLAUSE C)
{
  L->owningClause = C;
}


 void clause_LiteralSetOrderStatus(LITERAL L, ord_RESULT OS)
{
  L->ord_stat = OS;
}

 ord_RESULT clause_LiteralGetOrderStatus(LITERAL L)
{
  return L->ord_stat;
}

 NAT clause_LiteralWeight(LITERAL L)
{
#ifdef CHECK
  if (L->weight == clause_WEIGHTUNDEFINED) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralWeight:");
    misc_ErrorReport(" Tried to access undefined weight.");
    misc_FinishErrorReport();
  }
#endif
  return L->weight;
}


 void clause_UpdateLiteralWeight(LITERAL L, FLAGSTORE Flags)
{
  L->weight = clause_LiteralComputeWeight(L, Flags);
}


 void clause_LiteralFlagReset(LITERAL L)
{
  L->maxLit = 0;
}

 void clause_LiteralFlagResetAndKeepSelFlag(LITERAL L)
{
  L->maxLit &= LITSELECT;
}
 
 BOOL clause_LiteralGetFlag(LITERAL L, MAXFLAG Flag)
{
  return ((L->maxLit & Flag) != 0);
}

 void clause_LiteralSetFlag(LITERAL L, MAXFLAG Flag)
{
  L->maxLit = (L->maxLit) | Flag;
}

 void clause_LiteralClearFlag(LITERAL L, MAXFLAG Flag)
{
  L->maxLit = (L->maxLit) & ~Flag;
}

 BOOL clause_LiteralIsMaximal(LITERAL L)
{
  return clause_LiteralGetFlag(L, MAXIMAL);
}

 BOOL clause_LiteralIsOrientedEquality(LITERAL L)
{
  return fol_IsEquality(clause_LiteralAtom(L)) && (L->ord_stat == ord_GREATER_THAN);  
}

 BOOL clause_LiteralIsNotOrientedEquality(LITERAL L)
{
  return !fol_IsEquality(clause_LiteralAtom(L)) || (L->ord_stat != ord_GREATER_THAN);
}


/**************************************************************/
/* Literal Comparison 1                                       */
/**************************************************************/

 BOOL clause_LiteralIsNegative(LITERAL L)
{
  return (term_TopSymbol(clause_LiteralSignedAtom(L)) == fol_Not());
}

 BOOL clause_LiteralIsPositive(LITERAL L)
{
  return !clause_LiteralIsNegative(L);
}


 BOOL clause_LiteralsAreComplementary(LITERAL L1, LITERAL L2)
{
  return ((clause_LiteralIsNegative(L1) &&
	   clause_LiteralIsPositive(L2)) ||
	  (clause_LiteralIsNegative(L2) &&
	   clause_LiteralIsPositive(L1)));      /* xor */
}

 BOOL clause_HyperLiteralIsBetter(LITERAL Dummy1, NAT S1,
						   LITERAL Dummy2, NAT S2)
/**************************************************************
  INPUT:   Two literals and its sizes wrt. some substitution.
  RETURNS: TRUE, if the first literal is 'better' than the second literal,
           FALSE otherwise.
  EFFECT:  A literal is 'better' than another, if S1 > Ss.
           Since we have to find unifiable complementary literals
	   for every remaining antecedent literal, it seems to be
	   a good idea to try the most 'difficult' literal first,
	   in order to stop the search as early as possible..
	   Here we prefer the literal with the highest number
	   of symbols..
           This function is used as parameter for the function
	   clause_MoveBestLiteralToFront.
  CAUTION: The parameters <Dummy1> and <Dummy2> are unused, they're just
           added to keep the compiler quiet.
***************************************************************/
{
  return (S1 > S2);
}


/**************************************************************/
/* Accessing Literals 2                                       */
/**************************************************************/

 SYMBOL clause_LiteralPredicate(LITERAL L)
{
  return term_TopSymbol(clause_LiteralAtom(L));
}

 BOOL clause_LiteralIsPredicate(LITERAL L)
{
  return !fol_IsEquality(clause_LiteralAtom(L));
}

 BOOL clause_LiteralIsEquality(LITERAL L)
{
  return fol_IsEquality(clause_LiteralAtom(L));
}

 void clause_LiteralSetAtom(LITERAL L, TERM A)
{
  if (clause_LiteralIsNegative(L))
    list_Rplaca(term_ArgumentList(clause_LiteralSignedAtom(L)),A);
  else
    L->atomWithSign = A;
}

 void clause_LiteralSetNegAtom(LITERAL L, TERM A)
{
  list_Rplaca(term_ArgumentList(clause_LiteralSignedAtom(L)), A);
}

 void clause_LiteralSetPosAtom(LITERAL L, TERM A)
{
  L->atomWithSign = A;
}

 void clause_NLiteralSetLiteral(LITERAL L, TERM LIT)
{
  L->atomWithSign = LIT;
}

/**************************************************************/
/* Memory management                                          */
/**************************************************************/

 void clause_LiteralFree(LITERAL L)
{
  memory_Free(L, sizeof(LITERAL_NODE));
}


/**************************************************************/
/* Functions to access literals.                                 */
/**************************************************************/

 LITERAL clause_GetLiteral(CLAUSE C, intptr_t Index)
{
  return C->literals[Index];
}

 void clause_SetLiteral(CLAUSE C, int Index, LITERAL L)
{
  C->literals[Index]= L;
}

 TERM clause_GetLiteralTerm(CLAUSE C, int Index)
{
  return clause_LiteralSignedAtom(clause_GetLiteral(C, Index));
}

 TERM clause_GetLiteralAtom(CLAUSE C, intptr_t Index)
{
  return clause_LiteralAtom(clause_GetLiteral(C, Index));
}

 int clause_NumOfConsLits(CLAUSE Clause)
{
  return Clause->c;
}

 int clause_NumOfAnteLits(CLAUSE Clause)
{
  return Clause->a;
}

 int clause_NumOfSuccLits(CLAUSE Clause)
{
  return Clause->s;
}

 void clause_SetNumOfConsLits(CLAUSE Clause, int Number)
{
  Clause->c = Number;
}

 void clause_SetNumOfAnteLits(CLAUSE Clause, int Number)
{
  Clause->a = Number;
}

 void clause_SetNumOfSuccLits(CLAUSE Clause, int Number)
{
  Clause->s = Number;
}

 int clause_Length(CLAUSE Clause)
{
  return (clause_NumOfConsLits(Clause) +
	  clause_NumOfAnteLits(Clause) +
	  clause_NumOfSuccLits(Clause));
}


 int clause_LastLitIndex(CLAUSE Clause)
{
  return clause_Length(Clause) - 1;
}

 intptr_t clause_FirstLitIndex(void)
{
  return 0;
}

 int clause_FirstConstraintLitIndex(CLAUSE Clause)
{
  return 0;
}

 intptr_t clause_FirstAntecedentLitIndex(CLAUSE Clause)
{
  return clause_NumOfConsLits(Clause);
}

 intptr_t clause_FirstSuccedentLitIndex(CLAUSE Clause)
{
  return (clause_NumOfAnteLits(Clause) + clause_NumOfConsLits(Clause));
}


 int clause_LastConstraintLitIndex(CLAUSE Clause)
{
  return clause_NumOfConsLits(Clause) - 1;
}

 int clause_LastAntecedentLitIndex(CLAUSE Clause)
{
  return clause_FirstSuccedentLitIndex(Clause) - 1;
}

 int clause_LastSuccedentLitIndex(CLAUSE Clause)
{
  return clause_Length(Clause) - 1;
}

 LIST clause_GetLiteralList(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: A new list is created containing all literals of the
           clause. The list contains pointers, not literal indexes.
***************************************************************/
{
  LIST Result;
  int  i;

  Result = list_Nil();
  for (i=clause_FirstLitIndex(); i<=clause_LastLitIndex(Clause); i++)
    Result = list_Cons(clause_GetLiteral(Clause, i), Result);
  return Result;
}


 LIST clause_GetLiteralListExcept(CLAUSE Clause, int Index)
/**************************************************************
  INPUT:   A clause.
  RETURNS: A new list is created containing all literals of the
           clause except the literal at <Index>. The list contains
	   pointers, not literal indexes.
***************************************************************/
{
  LIST Result;
  int  i;

  Result = list_Nil();
  for (i=clause_FirstLitIndex(); i<=clause_LastLitIndex(Clause); i++)
    if (i != Index)
      Result = list_Cons(clause_GetLiteral(Clause, i), Result);
  return Result;
}


/**************************************************************/
/* Clause Access Macros                                       */
/**************************************************************/

 int clause_Counter(void)
{
  return clause_CLAUSECOUNTER;
}

 void clause_SetCounter(int Value)
{
#ifdef CHECK
  if (Value < 0) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_SetCounter: new counter value is negative.");
    misc_FinishErrorReport();
  }
#endif
  clause_CLAUSECOUNTER = Value;
}

 int clause_IncreaseCounter(void)
{
#ifdef CHECK
  if (clause_CLAUSECOUNTER == INT_MAX) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_IncreaseCounter: counter overflow.");
    misc_FinishErrorReport();
  }
#endif
  return clause_CLAUSECOUNTER++;
}

 void clause_DecreaseCounter(void)
{
#ifdef CHECK
  if (clause_CLAUSECOUNTER == 0) {
    misc_FinishErrorReport();
    misc_ErrorReport("\n In clause_DecreaseCounter: counter underflow.");
    misc_FinishErrorReport();
  }
#endif
  clause_CLAUSECOUNTER--;
}

 NAT clause_Depth(CLAUSE Clause)
{
  return Clause->depth;
}

 void clause_SetDepth(CLAUSE Clause, NAT NewDepth)
{
  Clause->depth = NewDepth;
}


 NAT clause_Weight(CLAUSE Clause)
{
#ifdef CHECK
  if (Clause->weight == clause_WEIGHTUNDEFINED) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_Weight: Tried to access undefined weight.");
    misc_FinishErrorReport();
  }
#endif
  return Clause->weight;
}

 void clause_UpdateWeight(CLAUSE Clause, FLAGSTORE Flags)
{
  Clause->weight = clause_ComputeWeight(Clause, Flags);
}

 float clause_SplitPotential(CLAUSE Clause)
{
  return Clause->splitpotential;
}

 void clause_SetSplitPotential(CLAUSE Clause, float pot)
{
  Clause->splitpotential = pot;
}

 intptr_t clause_Number(const CLAUSE Clause)
{
  return Clause->clausenumber;
}

 void clause_SetNumber(CLAUSE Clause, int Number)
{
  Clause->clausenumber = Number;
}

 void clause_NewNumber(CLAUSE Clause)
{
  Clause->clausenumber = clause_IncreaseCounter();
}


 NAT clause_SplitLevel(CLAUSE Clause)
{
  return Clause->validlevel;
}

 BOOL clause_CheckSplitLevel(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: TRUE, if the splitlevel invariant for the clause is fulfilled.
  EFFECT:  Checks, if the validlevel of the clause is the order
           of the highest set bit in the SPLITFIELD entry
           of the clause.
***************************************************************/
{
  if (Clause->validlevel == 0)
    return (Clause->splitfield == NULL);
  else {
    int i, j;
    for (i = Clause->splitfield_length-1; i >= 0; i--)
      if (Clause->splitfield[i] != 0)
	break;
    for (j = sizeof(SPLITFIELDENTRY)*CHAR_BIT-1; j >= 0; j--)
      if (Clause->splitfield[i] & ((SPLITFIELDENTRY)1 << j))
	break;
    return (Clause->validlevel == (i*sizeof(SPLITFIELDENTRY)*CHAR_BIT+j));
  }
}

 NAT clause_SplitLevelDependencies(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: The number of split levels the clause depends on
***************************************************************/
{
  if (Clause->validlevel == 0)
    return 0;
  else {
    int i, j;
    j = 0;
    for (i = Clause->splitfield_length-1; i >= 0; i--)
      if (Clause->splitfield[i] != 0)
	j++;
    return j;
  }
}

 LIST clause_ParentClauses(CLAUSE Clause)
{
  return Clause->parentCls;
}

 LIST clause_ParentLiterals(CLAUSE Clause)
{
  return Clause->parentLits;
}


 SYMBOL clause_MaxVar(CLAUSE Clause)
{
  return Clause->maxVar;
}

 void clause_SetMaxVar(CLAUSE Clause, SYMBOL Variable)
{
  Clause->maxVar = Variable;
}


 RULE clause_Origin(CLAUSE Clause)
{
  return Clause->origin;
}

 BOOL clause_Exists(CLAUSE Clause)
{
  return (Clause != (CLAUSE)NULL);
}

 BOOL clause_LiteralExists(LITERAL L)
{
  return (L != (LITERAL)NULL);
}

 CLAUSE clause_Null(void)
{
  return (CLAUSE) NULL;
}

 void clause_SetSplitLevel(CLAUSE Clause, NAT Level)
{
  Clause->validlevel = Level;
}

 void clause_InitSplitData(CLAUSE C)
{
  C->splitfield = NULL;
  C->splitfield_length = 0;
  clause_SetSplitLevel(C, 0);
}

 void clause_SetSplitField(CLAUSE Clause, SPLITFIELD B,
					    unsigned Length)
{
  unsigned i;
  if (Clause->splitfield_length != Length) {
    if (Clause->splitfield != NULL) {
      memory_Free(Clause->splitfield,
		  sizeof(SPLITFIELDENTRY) * Clause->splitfield_length);
    }
    if (Length != 0) {
      Clause->splitfield = memory_Malloc(sizeof(SPLITFIELDENTRY) * Length);
    }
    else
      Clause->splitfield = NULL;
    Clause->splitfield_length = Length;
  }
  for (i=0; i < Length; i++)
    Clause->splitfield[i] = B[i];
}


 NAT  clause_ComputeSplitFieldAddress(NAT n, NAT* field)
{
  *field = 0;
  while (n >= (sizeof(SPLITFIELDENTRY) * CHAR_BIT)) {
    (*field)++;
    n -= sizeof(SPLITFIELDENTRY) * CHAR_BIT;
  }
  return n;
}

 void clause_ExpandSplitField(CLAUSE C, NAT Length)
{
  SPLITFIELD NewField;
  NAT i;
  if (C->splitfield_length < Length) {
    NewField = memory_Malloc(sizeof(SPLITFIELDENTRY) * Length);
    for (i=0; i < C->splitfield_length; i++)
      NewField[i] = C->splitfield[i];
    for (i=C->splitfield_length; i < Length; i++)
      NewField[i] = 0;
    if (C->splitfield != NULL) {
      memory_Free(C->splitfield,
		  sizeof(SPLITFIELDENTRY) * C->splitfield_length);
    }
    C->splitfield = NewField;
    C->splitfield_length = Length;
  }
}

 void clause_ClearSplitField(CLAUSE C)
{
  int i;

  for (i=C->splitfield_length-1; i >=0; i--)
    C->splitfield[i] = 0;
}
	
 void clause_SetSplitFieldBit(CLAUSE Clause, NAT n)
{
  uintptr_t field;
  
  n = clause_ComputeSplitFieldAddress(n, &field);
  if (field >= Clause->splitfield_length)
    clause_ExpandSplitField(Clause, field + 1);
  Clause->splitfield[field] = (Clause->splitfield[field]) |
    ((SPLITFIELDENTRY)1 << n);
}

 BOOL clause_GetFlag(CLAUSE Clause, CLAUSE_FLAGS Flag)
{
  return (Clause->flags & Flag) != 0;
}

 void clause_SetFlag(CLAUSE Clause, CLAUSE_FLAGS Flag)
{
  Clause->flags = Clause->flags | Flag;
}

 void clause_RemoveFlag(CLAUSE Clause, CLAUSE_FLAGS Flag)
{
  if (Clause->flags & Flag)
    Clause->flags = Clause->flags - Flag;
}

 void clause_ClearFlags(CLAUSE Clause)
{
  Clause->flags = 0;
}


 BOOL clause_DependsOnSplitLevel(CLAUSE C, NAT N)
{
  if (N==0)
    return TRUE;
  else {
    uintptr_t field;
    N = clause_ComputeSplitFieldAddress(N, &field);
    if (field >= C->splitfield_length)
      return FALSE;
    else
      return (C->splitfield[field] & ((SPLITFIELDENTRY)1 << N)) != 0;
  }
}

 void clause_UpdateSplitDataFromNewSplitting(CLAUSE Result,
							      CLAUSE Father,
							      NAT Level)
{
  uintptr_t field;
  NAT i;
  
  clause_SetSplitLevel(Result, Level);
  Level = clause_ComputeSplitFieldAddress(Level, &field);

  if (field >= Result->splitfield_length) {
    if (Result->splitfield != NULL)
      memory_Free(Result->splitfield,
		  sizeof(SPLITFIELDENTRY) * Result->splitfield_length);
    Result->splitfield = memory_Malloc((field + 1) * sizeof(SPLITFIELDENTRY));
    Result->splitfield_length = field + 1;
  }
  if (clause_GetFlag(Father, CONCLAUSE))
    clause_SetFlag(Result, CONCLAUSE);
  for (i=0; i < Father->splitfield_length; i++)
    Result->splitfield[i] = Father->splitfield[i];
  for (i=Father->splitfield_length; i < Result->splitfield_length; i++)
    Result->splitfield[i] = 0;
  Result->splitfield[field] = (Result->splitfield[field] | ((SPLITFIELDENTRY)1 << Level));
}

 void clause_UpdateSplitDataFromPartner(CLAUSE Result,
							 CLAUSE Partner)
{
  if (clause_GetFlag(Partner, CONCLAUSE))
    clause_SetFlag(Result, CONCLAUSE);
  if (clause_SplitLevel(Partner) == 0)
    return;
  /* Set Split level to misc_Max(Partner, Result) */
  clause_SetSplitLevel(Result, clause_SplitLevel(Partner) > clause_SplitLevel(Result)
		       ? clause_SplitLevel(Partner)
		       : clause_SplitLevel(Result));
  clause_UpdateSplitField(Result, Partner);
}

 void clause_SetParentClauses(CLAUSE Clause, LIST PClauses)
{
  Clause->parentCls = PClauses;
}

 void clause_AddParentClause(CLAUSE Clause, intptr_t PClause)
{
  Clause->parentCls = list_Cons((POINTER) PClause, Clause->parentCls);
}

 void clause_SetParentLiterals(CLAUSE Clause, LIST PLits)
{
  Clause->parentLits = PLits;
}

 void clause_AddParentLiteral(CLAUSE Clause, intptr_t PLit)
{
  Clause->parentLits = list_Cons((POINTER) PLit, Clause->parentLits);
}


 BOOL clause_ValidityIsNotSmaller(CLAUSE C1, CLAUSE C2)
{
  return (C1->validlevel <= C2->validlevel);
}

 BOOL clause_IsMoreValid(CLAUSE C1, CLAUSE C2)
{
  return (C1->validlevel < C2->validlevel);
}


 BOOL  clause_CompareAbstractLEQ (CLAUSE Left, CLAUSE Right) 
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: TRUE if left <= right, FALSE otherwise.
  EFFECTS: Internal function used to compare clauses for
           sorting.
  CAUTION: Expects clause literal order to be fixed.
***************************************************************/
{
  return (BOOL) (clause_CompareAbstract(Left, Right) <= 0);
}


 BOOL clause_IsFromRewriting(CLAUSE Clause)
{
  return Clause->origin == REWRITING;
}

 BOOL clause_IsFromCondensing(CLAUSE Clause)
{
  return Clause->origin == CONDENSING;
}

 BOOL clause_IsFromObviousReductions(CLAUSE Clause)
{
  return Clause->origin == OBVIOUS_REDUCTIONS;
}

 BOOL clause_IsFromSortSimplification(CLAUSE Clause)
{
  return Clause->origin == SORT_SIMPLIFICATION;
}

 BOOL clause_IsFromMatchingReplacementResolution(CLAUSE Clause)
{
  return Clause->origin == MATCHING_REPLACEMENT_RESOLUTION;
}

  BOOL clause_IsFromClauseDeletion(CLAUSE Clause)
{
  return Clause->origin == CLAUSE_DELETION;
}

 BOOL clause_IsFromEmptySort(CLAUSE Clause)
{
  return Clause->origin == EMPTY_SORT;
}

 BOOL clause_IsFromSortResolution(CLAUSE Clause)
{
  return Clause->origin == SORT_RESOLUTION;
}

 BOOL clause_IsFromUnitConflict(CLAUSE Clause)
{
  return Clause->origin == UNIT_CONFLICT;
}

 BOOL clause_IsFromEqualityResolution(CLAUSE Clause)
{
  return Clause->origin == EQUALITY_RESOLUTION;
}

 BOOL clause_IsFromEqualityFactoring(CLAUSE Clause)
{
  return Clause->origin == EQUALITY_FACTORING;
}

 BOOL clause_IsFromMergingParamodulation(CLAUSE Clause)
{
  return Clause->origin == MERGING_PARAMODULATION;
}

 BOOL clause_IsFromSuperpositionRight(CLAUSE Clause)
{
  return Clause->origin == SUPERPOSITION_RIGHT;
}

 BOOL clause_IsFromSuperpositionLeft(CLAUSE Clause)
{
  return Clause->origin == SUPERPOSITION_LEFT;
}

 BOOL clause_IsFromGeneralResolution(CLAUSE Clause)
{
  return Clause->origin == GENERAL_RESOLUTION;
}

 BOOL clause_IsFromGeneralFactoring(CLAUSE Clause)
{
  return Clause->origin == GENERAL_FACTORING;
}

 BOOL clause_IsFromSplitting(CLAUSE Clause)
{
  return Clause->origin == SPLITTING;
}

 BOOL clause_IsFromDefApplication(CLAUSE Clause)
{
  return Clause->origin == DEFAPPLICATION;
}

 BOOL clause_IsFromTerminator(CLAUSE Clause)
{
  return Clause->origin == TERMINATOR;
}

 BOOL clause_IsTemporary(CLAUSE Clause)
{
  return Clause->origin == TEMPORARY;
}

 BOOL clause_IsFromInput(CLAUSE Clause)
{
  return Clause->origin == INPUTCLAUSE;
}

 BOOL clause_IsFromOrderedChaining(CLAUSE Clause)
{
  return Clause->origin == ORDERED_CHAINING;
}

 BOOL clause_IsFromNegativeChaining(CLAUSE Clause)
{
  return Clause->origin == NEGATIVE_CHAINING;
}

 BOOL clause_IsFromCompositionResolution(CLAUSE Clause)
{
  return Clause->origin == COMPOSITION_RESOLUTION;
}

 BOOL clause_HasReducedPredecessor(CLAUSE Clause)
{
  RULE origin = clause_Origin(Clause);

  return (origin == CONDENSING                   ||
	  origin == REWRITING                    ||
	  origin == SPLITTING                    ||
	  origin == ASSIGNMENT_EQUATION_DELETION ||
	  origin == SORT_SIMPLIFICATION          ||
	  origin == OBVIOUS_REDUCTIONS);
}

 BOOL clause_IsSplitFather(CLAUSE C1, CLAUSE C2)
{
  return (C1->clausenumber == (intptr_t)list_Car(C2->parentCls));
}


 void clause_SetFromRewriting(CLAUSE Clause)
{
  Clause->origin = REWRITING;
}

 void clause_SetFromContextualRewriting(CLAUSE Clause)
{
  Clause->origin = CONTEXTUAL_REWRITING;
}

 void clause_SetFromUnitConflict(CLAUSE Clause)
{
  Clause->origin = UNIT_CONFLICT;
}

 void clause_SetFromCondensing(CLAUSE Clause)
{
  Clause->origin = CONDENSING;
}

 void clause_SetFromAssignmentEquationDeletion(CLAUSE Clause)
{
  Clause->origin = ASSIGNMENT_EQUATION_DELETION;
}

 void clause_SetFromObviousReductions(CLAUSE Clause)
{
  Clause->origin = OBVIOUS_REDUCTIONS;
}

 void clause_SetFromSortSimplification(CLAUSE Clause)
{
  Clause->origin = SORT_SIMPLIFICATION;
}

 void clause_SetFromMatchingReplacementResolution(CLAUSE Clause)
{
  Clause->origin = MATCHING_REPLACEMENT_RESOLUTION;
}

 void clause_SetFromClauseDeletion(CLAUSE Clause)
{
  Clause->origin = CLAUSE_DELETION;
}

 void clause_SetFromEmptySort(CLAUSE Clause)
{
  Clause->origin = EMPTY_SORT;
}

 void clause_SetFromSortResolution(CLAUSE Clause)
{
  Clause->origin = SORT_RESOLUTION;
}

 void clause_SetFromEqualityResolution(CLAUSE Clause)
{
  Clause->origin = EQUALITY_RESOLUTION;
}

 void clause_SetFromEqualityFactoring(CLAUSE Clause)
{
  Clause->origin = EQUALITY_FACTORING;
}

 void clause_SetFromMergingParamodulation(CLAUSE Clause)
{
  Clause->origin = MERGING_PARAMODULATION;
}

 void clause_SetFromParamodulation(CLAUSE Clause)
{
  Clause->origin = PARAMODULATION;
}
 void clause_SetFromOrderedParamodulation(CLAUSE Clause)
{
  Clause->origin = ORDERED_PARAMODULATION;
}
 void clause_SetFromSuperpositionRight(CLAUSE Clause)
{
  Clause->origin = SUPERPOSITION_RIGHT;
}

 void clause_SetFromSuperpositionLeft(CLAUSE Clause)
{
  Clause->origin = SUPERPOSITION_LEFT;
}

 void clause_SetFromGeneralResolution(CLAUSE Clause)
{
  Clause->origin = GENERAL_RESOLUTION;
}

 void clause_SetFromOrderedHyperResolution(CLAUSE Clause)
{
  Clause->origin = ORDERED_HYPER;
}

 void clause_SetFromSimpleHyperResolution(CLAUSE Clause)
{
  Clause->origin = SIMPLE_HYPER;
}

 void clause_SetFromURResolution(CLAUSE Clause)
{
  Clause->origin = UR_RESOLUTION;
}

 void clause_SetFromGeneralFactoring(CLAUSE Clause)
{
  Clause->origin = GENERAL_FACTORING;
}

 void clause_SetFromSplitting(CLAUSE Clause)
{
  Clause->origin = SPLITTING;
}

 void clause_SetFromDefApplication(CLAUSE Clause)
{
  Clause->origin = DEFAPPLICATION;
}

 void clause_SetFromTerminator(CLAUSE Clause)
{
  Clause->origin = TERMINATOR;
}

 void clause_SetTemporary(CLAUSE Clause)
{
  Clause->origin = TEMPORARY;
}

 void clause_SetFromOrderedChaining(CLAUSE Clause)
{
  Clause->origin = ORDERED_CHAINING;
}  

 void clause_SetFromNegativeChaining(CLAUSE Clause)
{
  Clause->origin = NEGATIVE_CHAINING;
}

 void clause_SetFromCompositionResolution(CLAUSE Clause)
{
  Clause->origin = COMPOSITION_RESOLUTION;
}

 void clause_SetFromInput(CLAUSE Clause)
{
  Clause->origin = INPUTCLAUSE;
}


 LITERAL clause_FirstConstraintLit(CLAUSE Clause)
{
  return Clause->literals[0];
}

 LITERAL clause_FirstAntecedentLit(CLAUSE Clause)
{
  return Clause->literals[clause_FirstAntecedentLitIndex(Clause)];
}

 LITERAL clause_FirstSuccedentLit(CLAUSE Clause)
{
  return Clause->literals[clause_FirstSuccedentLitIndex(Clause)];
}

 LITERAL clause_LastConstraintLit(CLAUSE Clause)
{
  return Clause->literals[clause_LastConstraintLitIndex(Clause)];
}

 LITERAL clause_LastAntecedentLit(CLAUSE Clause)
{
  return Clause->literals[clause_LastAntecedentLitIndex(Clause)];
}

 LITERAL clause_LastSuccedentLit(CLAUSE Clause)
{
  return Clause->literals[clause_LastSuccedentLitIndex(Clause)];
}


 BOOL clause_HasEmptyConstraint(CLAUSE Clause)
{
  return clause_NumOfConsLits(Clause) == 0;
}

 BOOL clause_HasEmptyAntecedent(CLAUSE Clause)
{
  return clause_NumOfAnteLits(Clause) == 0;
}

 BOOL clause_HasEmptySuccedent(CLAUSE Clause)
{
  return clause_NumOfSuccLits(Clause) == 0;
}


 BOOL clause_IsGround(CLAUSE Clause)
{
#ifdef CHECK
  if (!symbol_Equal(clause_MaxVar(Clause), clause_SearchMaxVar(Clause))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_IsGround: Clause is corrupted.");
    misc_FinishErrorReport();
  }
#endif
  return (symbol_VarIndex(clause_MaxVar(Clause)) ==
	  symbol_GetInitialStandardVarCounter());
}

 BOOL clause_IsEmptyClause(CLAUSE C)
{
  return (C != (CLAUSE)NULL &&
	  clause_HasEmptyAntecedent(C) &&
	  clause_HasEmptySuccedent(C)  &&
	  clause_HasEmptyConstraint(C));
}

 intptr_t clause_LiteralGetIndex(LITERAL L)
{
  intptr_t j = 0;

  while (clause_GetLiteral(clause_LiteralOwningClause(L), j) != L)
    j++;

  return j;
}

 BOOL clause_LiteralIsFromConstraint(LITERAL Literal)
{
  int    literal_index  = clause_LiteralGetIndex(Literal);
  CLAUSE clause = clause_LiteralOwningClause(Literal);

  return (literal_index <= clause_LastConstraintLitIndex(clause) &&
	  literal_index >= clause_FirstConstraintLitIndex(clause));
}

 BOOL clause_LiteralIsFromAntecedent(LITERAL Literal)
{
  int    literal_index  = clause_LiteralGetIndex(Literal);
  CLAUSE clause = clause_LiteralOwningClause(Literal);

  return (literal_index <= clause_LastAntecedentLitIndex(clause) &&
	  literal_index >= clause_FirstAntecedentLitIndex(clause));
}

 BOOL clause_LiteralIsFromSuccedent(LITERAL Literal)
{
   int    literal_index;
   CLAUSE clause;
   literal_index  = clause_LiteralGetIndex(Literal);
   clause = clause_LiteralOwningClause(Literal);
   return (literal_index <= clause_LastSuccedentLitIndex(clause) &&
	   literal_index >= clause_FirstSuccedentLitIndex(clause));
}

 BOOL clause_IsSimpleSortClause(CLAUSE Clause)
{
  return (clause_HasEmptyAntecedent(Clause) &&
	  (clause_NumOfSuccLits(Clause) == 1) &&
	  clause_LiteralIsSort(clause_GetLiteral(Clause,
				clause_NumOfConsLits(Clause))) &&
	  clause_HasSolvedConstraint(Clause));
}

 BOOL clause_IsSubsortClause(CLAUSE Clause)
{
  return (clause_IsSimpleSortClause(Clause) &&
	  term_IsVariable(term_FirstArgument(
            clause_LiteralSignedAtom(
	      clause_GetLiteral(Clause, clause_NumOfConsLits(Clause))))));
}


 BOOL clause_HasSuccLits(CLAUSE Clause)
{
  return (clause_NumOfSuccLits(Clause) > 1);
}

 BOOL clause_HasGroundSuccLit(CLAUSE Clause)
{
  int  i, l;

  l = clause_Length(Clause);
  for (i = clause_FirstSuccedentLitIndex(Clause); i < l; i++)
    if (term_IsGround(Clause->literals[i]->atomWithSign))
      return TRUE;
  
  return FALSE;
}

 LITERAL clause_GetGroundSuccLit(CLAUSE Clause)
{
  int  i, l;

  l = clause_Length(Clause);
  for (i = clause_FirstSuccedentLitIndex(Clause); i < l; i++)
    if (term_IsGround(Clause->literals[i]->atomWithSign))
      return Clause->literals[i];

  return (LITERAL)NULL;
}


 void clause_Free(CLAUSE Clause)
{
  memory_Free(Clause, sizeof(CLAUSE_NODE));
}


 void clause_ReInit(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence)
{
  clause_Normalize(Clause);
  clause_SetMaxLitFlags(Clause, Flags, Precedence);
  clause_UpdateWeight(Clause, Flags);
  Clause->splitpotential = 0.0;
  clause_UpdateMaxVar(Clause);
}

 void clause_ReInitSkolem(CLAUSE Clause,
				     FLAGSTORE Flags,
				     PRECEDENCE Precedence)
{
  /* No clause_Normalize(Clause) because variables are interpreted
   * as constants */
  clause_SetMaxLitFlagsSkolem(Clause, Flags, Precedence);
  clause_UpdateWeight(Clause, Flags);
  /* Variables are assumed to be constants - therefore 
   * <Clause> has no variables */
  clause_SetMaxVar(Clause, symbol_GetInitialStandardVarCounter());

}

 void clause_PrecomputeOrderingAndReInit(CLAUSE Clause, FLAGSTORE Flags,
					      PRECEDENCE Precedence)
{
  clause_PrecomputeOrdering(Clause, Flags, Precedence);
  clause_ReInit(Clause, Flags, Precedence);
}


 void clause_PrecomputeOrderingAndReInitSkolem(CLAUSE Clause, FLAGSTORE Flags,
                                                PRECEDENCE Precedence)
{
    
    clause_PrecomputeOrderingSkolem(Clause, Flags, Precedence);
    clause_ReInitSkolem(Clause, Flags, Precedence);

}

 void clause_SetDataFromFather(CLAUSE Result, CLAUSE Father,
						int i, FLAGSTORE Flags,
						PRECEDENCE Precedence)
{
  clause_PrecomputeOrderingAndReInit(Result, Flags, Precedence);
  clause_SetSplitDataFromFather(Result, Father);
  clause_SetDepth(Result, clause_Depth(Father) + 1);
  clause_AddParentClause(Result, clause_Number(Father));
  clause_AddParentLiteral(Result, i);
}


 void clause_SetDataFromParents(CLAUSE Result, CLAUSE Father,
						 int i, CLAUSE Mother, int j,
						 FLAGSTORE Flags,
						 PRECEDENCE Precedence)
{
  clause_PrecomputeOrderingAndReInit(Result, Flags, Precedence);
  clause_SetSplitDataFromParents(Result, Father, Mother);
  clause_SetDepth(Result,
		  misc_Max(clause_Depth(Father), clause_Depth(Mother)) +1);
  clause_AddParentClause(Result, clause_Number(Father));
  clause_AddParentLiteral(Result, i);
  clause_AddParentClause(Result, clause_Number(Mother));
  clause_AddParentLiteral(Result, j);
}

 SPLITFIELD clause_GetSplitfield(CLAUSE C, unsigned *Length)
{
  *Length = C->splitfield_length;
  return C->splitfield;
}

/**************************************************************/
/*Previously Inlined functions ends here                      */ 
/**************************************************************/

/**************************************************************/
/* Inline functions                                           */
/**************************************************************/

 void clause_FreeLitArray(CLAUSE Clause)
{
  NAT Length = clause_Length(Clause);
  if (Length > 0)
    memory_Free(Clause->literals, sizeof(LITERAL) * Length);
}


/**************************************************************/
/* Primitive literal functions                                */
/**************************************************************/

BOOL clause_LiteralIsLiteral(LITERAL Literal)
/*********************************************************
  INPUT:   A literal.
  RETURNS: TRUE, if 'Literal' is not NULL and has a predicate
           symbol as its atoms top symbol.
**********************************************************/
{
  return ((Literal != NULL) &&
	  symbol_IsPredicate(clause_LiteralPredicate(Literal)));
}

NAT clause_LiteralComputeWeight(LITERAL Literal, FLAGSTORE Flags)
/*********************************************************
  INPUT:   A literal and a flag store.
  RETURNS: The weight of the literal.
  CAUTION: This function does not update the weight of the literal!
**********************************************************/
{
  TERM Term;
  int  Bottom;
  NAT  Number;

#ifdef CHECK
  if (!clause_LiteralIsLiteral(Literal)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralComputeWeight :");
    misc_ErrorReport("Illegal Input. Input not a literal.");
    misc_FinishErrorReport();
  }
#endif

  Term = clause_LiteralSignedAtom(Literal);
  Bottom = stack_Bottom();
  Number = 0;
  
  do {
    if (term_IsComplex(Term)) {
      if (symbol_HasProperty(term_TopSymbol(Term), TRANSITIVE)) {
        Number += flag_GetFlagIntValue(Flags, flag_TransWeight);
      } else
        Number += flag_GetFlagIntValue(Flags, flag_FUNCWEIGHT);
      
      stack_Push(term_ArgumentList(Term));
    }
    else
      if (term_IsVariable(Term))
	Number += flag_GetFlagIntValue(Flags, flag_VARWEIGHT);
      else
	Number += flag_GetFlagIntValue(Flags, flag_FUNCWEIGHT);
    
    while (!stack_Empty(Bottom) && list_Empty(stack_Top()))
      stack_Pop();
    if (!stack_Empty(Bottom)) {
      Term = (TERM) list_Car(stack_Top());
      stack_RplacTop(list_Cdr(stack_Top()));
    }
  } while (!stack_Empty(Bottom));

  return Number;

}

LITERAL clause_LiteralCreate(TERM Atom, CLAUSE Clause)
/**********************************************************
  INPUT:   A Pointer to a signed Predicate-Term and one to a
           clause it shall belong to.
  RETURNS: An appropriate literal where the other values
           are set to default values.
  MEMORY:  A LITERAL_NODE is allocated.
  CAUTION: The weight of the literal is not set correctly!
***********************************************************/
{
  LITERAL Result;

#ifdef CHECK
  if (!term_IsTerm(Atom)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralCreate:");
    misc_ErrorReport("\n Illegal input. Input not a term.");
    misc_FinishErrorReport();
  }
  if (!symbol_IsPredicate(term_TopSymbol(Atom)) &&
      (term_TopSymbol(Atom) != fol_Not())) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralCreate:");
    misc_ErrorReport("\n Illegal input. No predicate- or negated term.");
    misc_FinishErrorReport();
  }
#endif

  Result                = (LITERAL)memory_Malloc(sizeof(LITERAL_NODE));
  
  Result->atomWithSign  = Atom;
  Result->ord_stat      = ord_UNCOMPARABLE;
  Result->weight        = clause_WEIGHTUNDEFINED;
  Result->maxLit        = 0;
  Result->owningClause  = Clause;

  return Result;
}


LITERAL clause_LiteralCreateNegative(TERM Atom, CLAUSE Clause)
/**********************************************************
  INPUT:   A Pointer to a unsigned Predicate-Term and one to a
           clause it shall belong to.
  RETURNS: An appropriate literal where the other values
           are set to default values and the term gets a sign.
  MEMORY:  A LITERAL_NODE is allocated.
  CAUTION: The weight of the literal is not set correctly!
***********************************************************/
{
  LITERAL Result;

#ifdef CHECK
  if (!term_IsTerm(Atom)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralCreateNegative:");
    misc_ErrorReport("\n Illegal input. Input not an atom.");
    misc_FinishErrorReport();
  }
  if (!symbol_IsPredicate(term_TopSymbol(Atom)) &&
      (term_TopSymbol(Atom) != fol_Not())) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralCreateNegative:");
    misc_ErrorReport("\n Illegal input. Input term not normalized.");
    misc_FinishErrorReport();
  }
#endif

  Result                = (LITERAL)memory_Malloc(sizeof(LITERAL_NODE));
  
  term_RplacSupertermList(Atom, list_Nil());

  Result->atomWithSign  = term_Create(fol_Not(), list_List(Atom));
  Result->ord_stat      = ord_UNCOMPARABLE;
  Result->maxLit        = 0;
  Result->weight        = clause_WEIGHTUNDEFINED;
  Result->owningClause  = Clause;

  return Result;
}


void clause_LiteralDelete(LITERAL Literal)
/*********************************************************
  INPUT:   A literal, which has an unshared Atom. For Shared
           literals clause_LiteralDeleteFromSharing(Lit) is
	   available.
  RETURNS: Nothing.
  MEMORY:  The Atom of 'Literal' is deleted and its memory
           is freed as well as the LITERAL_NODE.
**********************************************************/
{
#ifdef CHECK
  if (!clause_LiteralIsLiteral(Literal)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralDelete:");
    misc_ErrorReport("\n Illegal input. Input not a literal.");
    misc_FinishErrorReport();
  }
#endif

  term_Delete(clause_LiteralSignedAtom(Literal));

  clause_LiteralFree(Literal);
}


void clause_LiteralInsertIntoSharing(LITERAL Lit, SHARED_INDEX ShIndex)
/**********************************************************
  INPUT:   A Literal with an unshared Atom and an Index.
  RETURNS: The literal with a shared Atom inserted into the
           'Index'.
  MEMORY:  Allocates TERM_NODE memory needed to insert 'Lit'
           into the sharing and frees the memory of the
           unshared Atom.
***********************************************************/
{

  TERM Atom;
  
#ifdef CHECK
  if (!clause_LiteralIsLiteral(Lit)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralInsertIntoSharing:");
    misc_ErrorReport("\n Illegal literal input.");
    misc_FinishErrorReport();
  }
#endif
  
  Atom = clause_LiteralAtom(Lit);
  
  clause_LiteralSetAtom(Lit, sharing_Insert(Lit, Atom, ShIndex));
  
  term_Delete(Atom);
}


void clause_LiteralDeleteFromSharing(LITERAL Lit, SHARED_INDEX ShIndex)
/**********************************************************
  INPUT:   A Literal and an 'Index'.
  RETURNS: Nothing.
  MEMORY:  Deletes 'Lit' from Sharing, frees no more used
           TERM memory and frees the LITERAL_NODE.
********************************************************/
{

  TERM Atom;

#ifdef CHECK
  if (!clause_LiteralIsLiteral(Lit)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralDeleteFromSharing:");
    misc_ErrorReport("\n Illegal literal input.");
    misc_FinishErrorReport();
  }
#endif

  Atom = clause_LiteralAtom(Lit);

  if (clause_LiteralIsNegative(Lit)) {
    list_Free(term_ArgumentList(clause_LiteralSignedAtom(Lit)));
    term_Free(clause_LiteralSignedAtom(Lit));
  }
 
  sharing_Delete(Lit, Atom, ShIndex);

  clause_LiteralFree(Lit);

}


NAT clause_GetNumberOfInstances(TERM Term, SHARED_INDEX Index)
/**************************************************************
  INPUT:   A (!) shared term and a shared index. The term has to be
           part of the index.
  RETURNS: How many clauses contain a literal that is an instance of <Atom>.
***************************************************************/
{
  NAT  Result;
  TERM Instance;
  LIST Scan;

  Result   = 0;
  Instance = st_ExistInstance(cont_LeftContext(), sharing_Index(Index), Term);
  if (term_IsAtom(Term)) {
    while (Instance != NULL) {
      for (Scan=sharing_NAtomDataList(Instance);!list_Empty(Scan);Scan=list_Cdr(Scan))
	if (!clause_GetFlag(clause_LiteralOwningClause(list_Car(Scan)), MARK)) {
	  clause_SetFlag(clause_LiteralOwningClause(list_Car(Scan)), MARK);
	  Result++;
	}
      Instance = st_NextCandidate();
    }
  }
  else {
    while (Instance != NULL) {
      Scan = sharing_GetDataList(Instance, Index);
      for ( ; !list_Empty(Scan); Scan = list_Pop(Scan))
	if (!clause_GetFlag(clause_LiteralOwningClause(list_Car(Scan)), MARK)) {
	  clause_SetFlag(clause_LiteralOwningClause(list_Car(Scan)), MARK);
	  Result++;
	}
      Instance = st_NextCandidate();
    }
  }
  return Result;
}


static LIST clause_CopyLitInterval(CLAUSE Clause, int Start, int End)
/**************************************************************
 INPUT:   A clause and two integers representing
          literal indices.
 RETURNS: Copies of all literals in <Clause> with index i and
          in the interval [Start:End] are prepended to <List>.
 MEMORY:  All atoms are copied.
***************************************************************/
{
  TERM Atom;
  LIST List;

  List = list_Nil();

  for ( ; Start <= End; Start++) {
    Atom = term_Copy(clause_GetLiteralAtom(Clause, Start));
    List = list_Cons(Atom, List);
  }
  
  return List;
}


static LIST clause_CopyLitIntervalExcept(CLAUSE Clause, int Start, int End,
					 int i)
/**************************************************************
 INPUT:   A clause and three integers representing
          literal indeces.
 RETURNS: A list of atoms from literals within the interval
          [Start:End] except the literal at index <i>.
 MEMORY:  All atoms are copied.
***************************************************************/
{
  TERM Atom;
  LIST Result;
  
  Result = list_Nil();
  for ( ; End >= Start; End--)
    if (End != i) {
      Atom   = term_Copy(clause_GetLiteralAtom(Clause, End));
      Result = list_Cons(Atom, Result);
    }
  return Result;
}


LIST clause_CopyConstraint(CLAUSE Clause)
/**************************************************************
 INPUT:   A clause.
 RETURNS: The list of copied constraint literals from <Clause>.
***************************************************************/
{
  return clause_CopyLitInterval(Clause,
				clause_FirstConstraintLitIndex(Clause),
				clause_LastConstraintLitIndex(Clause));
}


LIST clause_CopyAntecedentExcept(CLAUSE Clause, int i)
/**************************************************************
 INPUT:   A clause.
 RETURNS: A list containing copies of all antecedent literals
          except <i>.
***************************************************************/
{
  return clause_CopyLitIntervalExcept(Clause,
				      clause_FirstAntecedentLitIndex(Clause),
				      clause_LastAntecedentLitIndex(Clause),
				      i);
}

LIST clause_CopySuccedent(CLAUSE Clause)
/**************************************************************
 INPUT:   A clause.
 RETURNS: The list of copied succedent literals from <Clause>.
***************************************************************/
{
  return clause_CopyLitInterval(Clause,
				clause_FirstSuccedentLitIndex(Clause),
				clause_LastSuccedentLitIndex(Clause));
}


LIST clause_CopySuccedentExcept(CLAUSE Clause, int i)
/**************************************************************
 INPUT:   A clause.
 RETURNS: A list containing copies of all succedent literals
          except <i>.
***************************************************************/
{
  return clause_CopyLitIntervalExcept(Clause,
				      clause_FirstSuccedentLitIndex(Clause),
				      clause_LastSuccedentLitIndex(Clause),
				      i);
}


/**************************************************************/
/* Specials                                                   */
/**************************************************************/

BOOL clause_IsUnorderedClause(CLAUSE Clause)
/*********************************************************
  INPUT:   A clause.
  RETURNS: TRUE, if the invariants concerning splitting etc. hold
           Invariants concerning maximality restrictions are not tested.
**********************************************************/
{
  return ((Clause != NULL) &&
	  clause_CheckSplitLevel(Clause) &&
	  (clause_IsEmptyClause(Clause) ||
	   /* Check the first literal as a "random" sample */
	   clause_LiteralIsLiteral(clause_GetLiteral(Clause,clause_FirstLitIndex()))) &&
	  (clause_SplitLevel(Clause) == 0 || Clause->splitfield_length>0) &&
	  clause_DependsOnSplitLevel(Clause,clause_SplitLevel(Clause)));
}


BOOL clause_IsClauseAux(CLAUSE Clause, FLAGSTORE FlagStore, PRECEDENCE Precedence,
                        BOOL VarIsConst)
/*********************************************************
  INPUT:   A clause, a flag store and a precedence and a flag.
  RETURNS: TRUE, if literals are correctly ordered and the
           invariants concerning splitting etc. hold
  CAUTION: If <VarIsConst> is set to TRUE variables are
           interpreted as constants
**********************************************************/
{
  int     i;
  LITERAL ActLit;

  if (!clause_IsUnorderedClause(Clause))
    return FALSE;

  for (i=clause_FirstAntecedentLitIndex(Clause);i<=clause_LastSuccedentLitIndex(Clause);i++) {
    ActLit = clause_GetLiteral(Clause,i);

    if (!clause_LiteralIsLiteral(ActLit))
      return FALSE;

    if (fol_IsEquality(clause_LiteralAtom(ActLit))) {
      ord_RESULT HelpRes;
	
      HelpRes =
	ord_CompareAux(term_FirstArgument(clause_LiteralAtom(ActLit)),
		    term_SecondArgument(clause_LiteralAtom(ActLit)),
		    FlagStore, Precedence, VarIsConst);
	
      if (ord_IsSmallerThan(HelpRes))
        return FALSE;
    }
    
    if (symbol_HasProperty(term_TopSymbol(clause_LiteralAtom(ActLit)), TRANSITIVE)) {
      ord_RESULT HelpRes;
      
      HelpRes =
        ord_CompareAux(term_FirstArgument(clause_LiteralAtom(ActLit)),
          term_SecondArgument(clause_LiteralAtom(ActLit)),
          FlagStore, Precedence, VarIsConst);    
    
      if (HelpRes != clause_LiteralGetOrderStatus(ActLit))
        return FALSE;
    }
  }

  return TRUE;
}


BOOL clause_IsClause(CLAUSE Clause, FLAGSTORE FlagStore, PRECEDENCE Precedence)
/*********************************************************
  INPUT:   A clause, a flag store and a precedence.
  RETURNS: TRUE, if literals are correctly ordered and the
           invariants concerning splitting etc. hold
**********************************************************/
{
        return clause_IsClauseAux(Clause, FlagStore, Precedence, FALSE);
}


BOOL clause_IsClauseSkolem(CLAUSE Clause, FLAGSTORE FlagStore, PRECEDENCE Precedence)
/*********************************************************
  INPUT:   A clause, a flag store and a precedence.
  RETURNS: TRUE, if literals are correctly ordered and the
           invariants concerning splitting etc. hold
  CAUTION: Variables of <Clause> are interpreted as
           constants
**********************************************************/
{
    return clause_IsClauseAux(Clause, FlagStore, Precedence, TRUE);
}



BOOL clause_ContainsPositiveEquations(CLAUSE Clause)
/*********************************************************
  INPUT:   A clause.
  RETURNS: TRUE, if the clause contains a positive equality literal.
**********************************************************/
{
  int i;
  
  for (i = clause_FirstSuccedentLitIndex(Clause);
       i < clause_Length(Clause);
       i++)
    if (clause_LiteralIsEquality(clause_GetLiteral(Clause, i)))
      return TRUE;

  return FALSE;
}


BOOL clause_ContainsNegativeEquations(CLAUSE Clause)
/*********************************************************
  INPUT:   A clause.
  RETURNS: TRUE, if the clause contains a positive equality literal.
**********************************************************/
{
  int i;
  
  for (i = clause_FirstAntecedentLitIndex(Clause);
       i < clause_FirstSuccedentLitIndex(Clause);
       i++)
    if (clause_LiteralIsEquality(clause_GetLiteral(Clause, i)))
      return TRUE;

  return FALSE;
}


int clause_ContainsFolAtom(CLAUSE Clause, BOOL *Prop, BOOL *Grd, BOOL *Monadic,
			   BOOL *NonMonadic)
/*********************************************************
  INPUT:   A clause.
  RETURNS: The number of boolean variables changed.
           If <*Prop> is FALSE and the clause contains a propositional
	   variable, it is changed to TRUE.
	   If <*Grd> is FALSE and the clause contains a non-propositional
	   ground atom, it is changed to TRUE.
           If <*Monadic> is FALSE and the clause contains a monadic atom,
	      it is changed to TRUE.
	   If <*NonMonadic> is FALSE and the clause contains an at least
	   2-place non-equality atom, it is changed to TRUE.
**********************************************************/
{
  int  i,Result,Arity;
  BOOL Ground;

  Result = 0;
  i      = clause_FirstLitIndex();

  while (Result < 4 &&
	 i < clause_Length(Clause) &&
	 (!(*Prop) || !(*Monadic) || !(*NonMonadic))) {
    Arity  = symbol_Arity(term_TopSymbol(clause_GetLiteralAtom(Clause,i)));
    Ground = term_IsGround(clause_GetLiteralAtom(Clause,i));
    if (!(*Prop) && Arity == 0) {
      Result++;
      *Prop = TRUE;
    }
    if (!(*Grd) && Arity > 0 && Ground &&
	!fol_IsEquality(clause_GetLiteralAtom(Clause,i))) {
      Result++;
      *Grd = TRUE;
    }
    if (!(*Monadic) && Arity == 1 && !Ground) {
      Result++;
      *Monadic = TRUE;
    }
    if (!(*NonMonadic) && Arity > 1 && !Ground &&
	!fol_IsEquality(clause_GetLiteralAtom(Clause,i))) {
      Result++;
      *NonMonadic = TRUE;
    }
    i++;
  }
  return Result;
}


BOOL clause_ContainsVariables(CLAUSE Clause)
/*********************************************************
  INPUT:   A clause.
  RETURNS: TRUE, if the clause contains at least one variable
**********************************************************/
{
  int  i;
  TERM Term;

  for (i = clause_FirstLitIndex(); i < clause_Length(Clause); i++) {
    Term = clause_GetLiteralAtom(Clause,i);
    if (term_NumberOfVarOccs(Term)>0)
      return TRUE;
  }

  return FALSE;
}


void clause_ContainsSortRestriction(CLAUSE Clause, BOOL *Sortres, BOOL *USortres)
/*********************************************************
  INPUT:   A clause.
  RETURNS: TRUE, if the clause contains a negative monadic atom with
           a variable argument
**********************************************************/
{
  int  i;
  TERM Term;

  for (i = clause_FirstLitIndex();
       i <= clause_LastAntecedentLitIndex(Clause) && (!*Sortres || !*USortres);
       i++) {
    Term = clause_GetLiteralAtom(Clause,i);
    if (symbol_IsBaseSort(term_TopSymbol(Term))) {
      *USortres = TRUE;
      if (symbol_IsVariable(term_TopSymbol(term_FirstArgument(Term))))
	*Sortres = TRUE;
    }
  }
}

BOOL clause_ContainsFunctions(CLAUSE Clause)
/*********************************************************
  INPUT:   A clause.
  RETURNS: TRUE, if the clause contains at least one function symbol
**********************************************************/
{
  int  i;
  TERM Term;

  for (i = clause_FirstLitIndex(); i < clause_Length(Clause); i++) {
    Term = clause_GetLiteralAtom(Clause,i);
    if (term_ContainsFunctions(Term))
      return TRUE;
  }

  return FALSE;
}

BOOL clause_ContainsSymbol(CLAUSE Clause, SYMBOL Symbol)
/*********************************************************
  INPUT:   A clause and a symbol.
  RETURNS: TRUE, if the clause contains the symbol
**********************************************************/
{
  int  i;

  for (i = clause_FirstLitIndex(); i < clause_Length(Clause); i++)
    if (term_ContainsSymbol(clause_GetLiteralAtom(Clause,i), Symbol))
      return TRUE;
  return FALSE;
}

NAT clause_NumberOfSymbolOccurrences(CLAUSE Clause, SYMBOL Symbol)
/*********************************************************
  INPUT:   A clause and a symbol.
  RETURNS: the number of occurrences of <Symbol> in <Clause>
**********************************************************/
{
  int  i;
  NAT  Result;

  Result = 0;

  for (i = clause_FirstLitIndex(); i < clause_Length(Clause); i++)
    Result += term_NumberOfSymbolOccurrences(clause_GetLiteralAtom(Clause,i), Symbol);

  return Result;
}

BOOL clause_ImpliesFiniteDomain(CLAUSE Clause)
/*********************************************************
  INPUT:   A clause.
  RETURNS: TRUE, if the clause consists of a positive disjunction
           of equations, where each equation is of the form "x=t" for
	   some variable "x" and ground term "t"
**********************************************************/
{
  int  i;
  TERM Term;

  if (clause_FirstLitIndex() != clause_FirstSuccedentLitIndex(Clause))
    return FALSE;

  for (i = clause_FirstLitIndex(); i < clause_Length(Clause); i++) {
    Term = clause_GetLiteralTerm(Clause,i);
    if (!symbol_Equal(term_TopSymbol(Term),fol_Equality()) ||
	(!symbol_IsVariable(term_TopSymbol(term_FirstArgument(Term))) &&
	 !symbol_IsVariable(term_TopSymbol(term_SecondArgument(Term)))) ||
	(symbol_IsVariable(term_TopSymbol(term_FirstArgument(Term))) &&
	 !term_IsGround(term_SecondArgument(Term))) ||
	(symbol_IsVariable(term_TopSymbol(term_SecondArgument(Term))) &&
	 !term_IsGround(term_FirstArgument(Term))))
      return FALSE;
  }

  return TRUE;
}

BOOL clause_ImpliesNonTrivialDomain(CLAUSE Clause)
/*********************************************************
  INPUT:   A clause.
  RETURNS: TRUE, if the clause consists of a negative equation
           with two syntactically different arguments
**********************************************************/
{
  if (clause_Length(Clause) == 1 &&
      !clause_HasEmptyAntecedent(Clause) &&
      clause_LiteralIsEquality(clause_FirstAntecedentLit(Clause)) &&
      !term_Equal(term_FirstArgument(clause_LiteralAtom(clause_FirstAntecedentLit(Clause))),
		  term_SecondArgument(clause_LiteralAtom(clause_FirstAntecedentLit(Clause)))))
    return TRUE;
  
  return FALSE;
}

LIST clause_FiniteMonadicPredicates(LIST Clauses)
/*********************************************************
  INPUT:   A list of clauses.
  RETURNS: A list of all predicate symbols that are guaranteed
           to have a finite extension in any minimal Herbrand model.
	   These predicates must only positively occur
           in unit clauses and must have a ground term argument.
**********************************************************/
{
  LIST   Result, NonFinite, Scan;
  CLAUSE Clause;
  int    i, n;
  SYMBOL Pred;

  Result    = list_Nil();
  NonFinite = list_Nil();

  for (Scan=Clauses;!list_Empty(Scan);Scan=list_Cdr(Scan)) {
    Clause = (CLAUSE)list_Car(Scan);
    n      = clause_Length(Clause);
    for (i=clause_FirstSuccedentLitIndex(Clause);i<n;i++) {
      Pred = term_TopSymbol(clause_GetLiteralAtom(Clause,i));
      if (symbol_Arity(Pred) == 1 &&
	  !list_PointerMember(NonFinite, (POINTER)Pred)) {
	if (term_NumberOfVarOccs(clause_GetLiteralAtom(Clause,i)) > 0 ||
	    n > 1) {
	  NonFinite = list_Cons((POINTER)Pred, NonFinite);
	  Result    = list_PointerDeleteElement(Result, (POINTER)Pred);
	}
	else {
	  if (!list_PointerMember(Result, (POINTER)Pred))
	    Result = list_Cons((POINTER)Pred, Result);
	}
      }
    }
  }
  list_Delete(NonFinite);

  Result = list_PointerDeleteElement(Result, (POINTER)fol_Equality());

  return Result;
}

NAT clause_NumberOfVarOccs(CLAUSE Clause)
/**************************************************************
  INPUT:   A Clause.
  RETURNS: The number of variable occurrences in the clause.
***************************************************************/
{
  int i,n;
  NAT Result;

  Result = 0;
  n      = clause_Length(Clause);

  for (i = clause_FirstLitIndex(); i < n; i++)
    Result += term_NumberOfVarOccs(clause_GetLiteralTerm(Clause,i));
  
  return Result;
}


NAT clause_ComputeWeight(CLAUSE Clause, FLAGSTORE Flags)
/**************************************************************
  INPUT:   A clause and a flag store.
  RETURNS: The Weight of the literals in the clause,
           up to now the number of variable symbols plus
	   twice the number of signature symbols.
  EFFECT:  The weight of the literals is updated, but not the
           weight of the clause!
***************************************************************/
{
  int     i, n;
  NAT     Weight;
  LITERAL Lit;

  Weight = 0;
  n      = clause_Length(Clause);
  
  for (i = clause_FirstLitIndex(); i < n; i++) {
    Lit = clause_GetLiteral(Clause, i);
    clause_UpdateLiteralWeight(Lit, Flags);
    Weight += clause_LiteralWeight(Lit);
  }
  return Weight;
}


NAT clause_ComputeTermDepth(CLAUSE Clause)
/**************************************************************
  INPUT:   A Clause.
  RETURNS: Maximal depth of a literal in <Clause>.
***************************************************************/
{
  int     i,n;
  NAT     Depth,Help;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_ComputeTermDepth:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif

  Depth = 0;
  n     = clause_Length(Clause);

  for (i = clause_FirstLitIndex();i < n;i++) {
    Help   = term_Depth(clause_GetLiteralAtom(Clause,i));
    if (Help > Depth)
      Depth = Help;
  }
  return Depth;
}

NAT clause_MaxTermDepthClauseList(LIST Clauses)
/**************************************************************
  INPUT:   A list of clauses.
  RETURNS: Maximal depth of a clause in <Clauses>.
***************************************************************/
{
  NAT  Depth,Help;

  Depth = 0;

  while (!list_Empty(Clauses)) {
    Help = clause_ComputeTermDepth(list_Car(Clauses));
    if (Help > Depth)
      Depth = Help;
    Clauses = list_Cdr(Clauses);
  }

  return Depth;
}


NAT clause_ComputeSize(CLAUSE Clause)
/**************************************************************
  INPUT:   A Clause.
  RETURNS: The Size of the literals in the clause,
           up to now the number of symbols.
***************************************************************/
{
  int     i,n;
  NAT     Size;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_ComputeSize:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif

  Size = 0;
  n    = clause_Length(Clause);

  for (i = clause_FirstLitIndex();i < n;i++)
    Size += term_ComputeSize(clause_GetLiteralTerm(Clause,i));
  
  return Size;
}


BOOL clause_WeightCorrect(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence)
/*********************************************************
  INPUT:   A clause, a flag store and a precedence.
  RETURNS: TRUE iff the weight fields of the clause and its
           literals are correct.
**********************************************************/
{
  int     i, n;
  NAT     Weight, Help;
  LITERAL Lit;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_WeightCorrect:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif

  Weight = 0;
  n      = clause_Length(Clause);

  for (i = clause_FirstLitIndex(); i < n; i++) {
    Lit  = clause_GetLiteral(Clause, i);
    Help = clause_LiteralComputeWeight(Lit, Flags);
    if (Help != clause_LiteralWeight(Lit))
      return FALSE;
    Weight += Help;
  }

  return (clause_Weight(Clause) == Weight);
}


LIST clause_InsertWeighed(CLAUSE Clause, LIST UsList, FLAGSTORE Flags,
			  PRECEDENCE Precedence)
/*********************************************************
  INPUT:   A clause, a list to insert the clause into,
           a flag store and a precedence.
  RETURNS: The list where the clause is inserted wrt its
           weight (Weight(Car(list)) <= Weight(Car(Cdr(list)))).
  MEMORY:  A new listnode is allocated.
**********************************************************/
{
  LIST Scan;
  NAT  Weight;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_InsertWeighted:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif

  Weight = clause_Weight(Clause);

  Scan = UsList;

  if (list_Empty(Scan) ||
      (clause_Weight(list_Car(Scan)) > Weight)) {
    return list_Cons(Clause, Scan);

  } else {
    while (!list_Empty(list_Cdr(Scan)) &&
	   (clause_Weight(list_Car(list_Cdr(Scan))) <= Weight)) {
      Scan = list_Cdr(Scan);
    }
    list_Rplacd(Scan, list_Cons(Clause, list_Cdr(Scan)));
    return UsList;
  }
}


LIST clause_ListSortWeighed(LIST Clauses)
/*********************************************************
  INPUT:   A list of clauses.
  RETURNS: The clause list sorted with respect to the weight
           of clauses, minimal weight first.
  EFFECT:  The original list is destructively changed!
           This function doesn't sort stable!
           The function uses bucket sort for clauses with weight
	   smaller than clause_MAXWEIGHT, and the usual list sort
	   function for clauses with weight >= clause_MAXWEIGHT.
	   This implies the function uses time O(n-c + c*log c),
	   where n is the length of the list and c is the number
	   of clauses with weight >= clause_MAXWEIGHT.
	   For c=0 you get time O(n), for c=n you get time (n*log n).
**********************************************************/
{
  int  weight;
  LIST Scan;

  for (Scan=Clauses; !list_Empty(Scan); Scan=list_Cdr(Scan)) {
    weight = clause_Weight(list_Car(Scan));
    if (weight < clause_MAXWEIGHT)
      clause_SORT[weight] = list_Cons(list_Car(Scan),clause_SORT[weight]);
    else
      clause_SORT[clause_MAXWEIGHT] = list_Cons(list_Car(Scan),clause_SORT[clause_MAXWEIGHT]);
  }
  Scan = list_NumberSort(clause_SORT[clause_MAXWEIGHT],
			 (NAT (*)(POINTER)) clause_Weight);
  clause_SORT[clause_MAXWEIGHT] = list_Nil();
  for (weight = clause_MAXWEIGHT-1; weight >= 0; weight--) {
    Scan                = list_Nconc(clause_SORT[weight],Scan);
    clause_SORT[weight] = list_Nil();
  }
  list_Delete(Clauses);
  return Scan;
}


LITERAL clause_LiteralCopy(LITERAL Literal)
/*********************************************************
  INPUT:   A literal.
  RETURNS: An unshared copy of the literal, where the owning
           clause-slot is set to NULL.
  MEMORY:  Memory for a new LITERAL_NODE and all its TERMs
           subterms is allocated.
**********************************************************/
{

  LITERAL Result;

#ifdef CHECK
  if (!clause_LiteralIsLiteral(Literal)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralCopy:");
    misc_ErrorReport("\n Illegal input. Input not a literal.");
    misc_FinishErrorReport();
  }
#endif

  Result                = (LITERAL)memory_Malloc(sizeof(LITERAL_NODE));
  
  Result->atomWithSign  = term_Copy(clause_LiteralSignedAtom(Literal));
  Result->ord_stat      = clause_LiteralGetOrderStatus(Literal);
  Result->maxLit        = Literal->maxLit;
  Result->weight        = Literal->weight;
  Result->owningClause  = (POINTER)NULL;

  return Result;
}

void clause_SetSplitDataFromList(CLAUSE Result, LIST List)
{
  CLAUSE TempClause;
  LIST Scan;
  NAT  l;
  Scan = List;
  l = Result->splitfield_length;
  while (!list_Empty(Scan)) {
    TempClause = (CLAUSE) list_Top(Scan);
    if (clause_GetFlag(TempClause, CONCLAUSE))
      clause_SetFlag(Result, CONCLAUSE);
    clause_SetSplitLevel(Result,
			 clause_SplitLevel(TempClause) > clause_SplitLevel(Result)
			 ? clause_SplitLevel(TempClause)
			 : clause_SplitLevel(Result));
    if (l < TempClause->splitfield_length)
      l = TempClause->splitfield_length;
    Scan = list_Cdr(Scan);
  }
  if (l > Result->splitfield_length) {
    if (Result->splitfield != NULL)
      memory_Free(Result->splitfield,
		  sizeof(SPLITFIELDENTRY) * Result->splitfield_length);
    Result->splitfield = memory_Malloc(sizeof(SPLITFIELDENTRY) * l);
    Result->splitfield_length = l;
  }
  
  for (l=0; l < Result->splitfield_length; l++)
    Result->splitfield[l] = 0;
  
  while (!list_Empty(List)) {
    TempClause= (CLAUSE) list_Top(List);
    List = list_Cdr(List);
    for (l=0; l < TempClause->splitfield_length; l++)
      Result->splitfield[l] = Result->splitfield[l] | TempClause->splitfield[l];
  }
}


void clause_SetSplitDataFromParents(CLAUSE Result,
				    CLAUSE Mother,
				    CLAUSE Father)
{
  NAT i;
  if (clause_GetFlag(Father, CONCLAUSE) || clause_GetFlag(Mother, CONCLAUSE))
    clause_SetFlag(Result, CONCLAUSE);
  if ((clause_SplitLevel(Father) == 0) && (clause_SplitLevel(Mother) == 0))
    return;
  clause_SetSplitLevel(Result, clause_SplitLevel(Mother) > clause_SplitLevel(Father)
		       ? clause_SplitLevel(Mother)
		       : clause_SplitLevel(Father));
  
  if (Mother->splitfield_length > Father->splitfield_length) {
    if (Result->splitfield != NULL)
      memory_Free(Result->splitfield,
		  sizeof(SPLITFIELDENTRY) * Result->splitfield_length);
    Result->splitfield = memory_Malloc(sizeof(SPLITFIELDENTRY) *
				       Mother->splitfield_length);
    Result->splitfield_length = Mother->splitfield_length;
    for (i=0; i < Father->splitfield_length; i++)
      Result->splitfield[i] =
	Mother->splitfield[i] | Father->splitfield[i];
    for (i=Father->splitfield_length; i < Mother->splitfield_length; i++)
      Result->splitfield[i] = Mother->splitfield[i];
  }
  else {
    if (Result->splitfield != NULL)
      memory_Free(Result->splitfield,
		  sizeof(SPLITFIELDENTRY) * Result->splitfield_length);
    Result->splitfield = memory_Malloc(sizeof(SPLITFIELDENTRY) *
				       Father->splitfield_length);
    Result->splitfield_length = Father->splitfield_length;
    for (i=0; i < Mother->splitfield_length; i++)
      Result->splitfield[i] =
	Mother->splitfield[i] | Father->splitfield[i];
    for (i=Mother->splitfield_length; i < Father->splitfield_length; i++)
      Result->splitfield[i] = Father->splitfield[i];
  }
}

void clause_SetSplitDataFromFather(CLAUSE Result,
				   CLAUSE Father)
{
  if (clause_GetFlag(Father, CONCLAUSE))
    clause_SetFlag(Result, CONCLAUSE);
  clause_SetSplitLevel(Result, clause_SplitLevel(Father));
  clause_SetSplitField(Result, Father->splitfield, Father->splitfield_length);
}

void clause_UpdateSplitField(CLAUSE C1, CLAUSE C2)
/*********************************************************
  INPUT:   Two clauses C1 and C2.
  RETURNS: None.
  EFFECT:  Add the split data of <C2> to <C1>.
**********************************************************/
{
  unsigned i;
  if (C1->splitfield_length < C2->splitfield_length)
    clause_ExpandSplitField(C1, C2->splitfield_length);
  for (i=0; i < C2->splitfield_length; i++)
    C1->splitfield[i] = C1->splitfield[i] | C2->splitfield[i];
}

BOOL clause_LiteralIsSort(LITERAL L)
{
  SYMBOL S;
  S = clause_LiteralPredicate(L);
  return (symbol_IsPredicate(S) &&
	  (symbol_Arity(S) == 1));
}

TERM clause_LiteralAtom(LITERAL L)
{
  if (clause_LiteralIsNegative(L))
    return term_FirstArgument(clause_LiteralSignedAtom(L));
  else
    return clause_LiteralSignedAtom(L);
}


CLAUSE clause_Copy(CLAUSE Clause)
/*********************************************************
  INPUT:   A Clause.
  RETURNS: An unshared copy of the Clause.
  MEMORY:  Memory for a new CLAUSE_NODE, LITERAL_NODE and
           all its TERMs subterms is allocated.
**********************************************************/
{

  CLAUSE Result;
  int i,c,a,s,l;

  Result                 = (CLAUSE)memory_Malloc(sizeof(CLAUSE_NODE));

  Result->clausenumber   = clause_Number(Clause);
  Result->maxVar         = clause_MaxVar(Clause);
  Result->flags          = Clause->flags;
  clause_InitSplitData(Result);
  Result->validlevel     = clause_SplitLevel(Clause);
  clause_SetSplitField(Result, Clause->splitfield, Clause->splitfield_length);
  Result->depth          = clause_Depth(Clause);
  Result->weight         = Clause->weight;
  Result->splitpotential = Clause->splitpotential;
  Result->parentCls      = list_Copy(clause_ParentClauses(Clause));
  Result->parentLits     = list_Copy(clause_ParentLiterals(Clause));
  Result->origin         = clause_Origin(Clause);

  Result->c              = (c = clause_NumOfConsLits(Clause));
  Result->a              = (a = clause_NumOfAnteLits(Clause));
  Result->s              = (s = clause_NumOfSuccLits(Clause));

  l = c + a + s;
  if (l != 0)
    Result->literals      = (LITERAL *)memory_Malloc(l * sizeof(LITERAL));

  for (i = 0; i < l; i++) {
    clause_SetLiteral(Result, i,
		      clause_LiteralCopy(clause_GetLiteral(Clause, i)));
    clause_LiteralSetOwningClause((Result->literals[i]), Result);
  }

  return Result;
}


SYMBOL clause_LiteralMaxVar(LITERAL Literal)
/*********************************************************
  INPUT:   A literal.
  RETURNS: The maximal symbol of the literals variables,
           if the literal is ground, symbol_GetInitialStandardVarCounter().
**********************************************************/
{

  TERM Term;
  int Bottom;
  SYMBOL MaxSym,Help;

#ifdef CHECK
  if (!clause_LiteralIsLiteral(Literal)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralMaxVar:");
    misc_ErrorReport("\n Illegal input. Input not a literal.");
    misc_FinishErrorReport();
  }
#endif

  Bottom = stack_Bottom();
  MaxSym = symbol_GetInitialStandardVarCounter();
  Term   = clause_LiteralAtom(Literal);

  do {
    if (term_IsComplex(Term))
      stack_Push(term_ArgumentList(Term));
    else
      if (term_IsVariable(Term))
	MaxSym = ((MaxSym < (Help = term_TopSymbol(Term))) ?
		  Help : MaxSym);
    while (!stack_Empty(Bottom) && list_Empty(stack_Top()))
      stack_Pop();
    if (!stack_Empty(Bottom)) {
      Term = (TERM) list_Car(stack_Top());
      stack_RplacTop(list_Cdr(stack_Top()));
    }
  } while (!stack_Empty(Bottom));

  return MaxSym;
}


SYMBOL clause_AtomMaxVar(TERM Term)
/*********************************************************
  INPUT:   A term.
  RETURNS: The maximal symbol of the lcontained variables,
           if <Term> is ground, symbol_GetInitialStandardVarCounter().
**********************************************************/
{
  int Bottom;
  SYMBOL VarSym,Help;

  Bottom = stack_Bottom();
  VarSym = symbol_GetInitialStandardVarCounter();

  do {
    if (term_IsComplex(Term))
      stack_Push(term_ArgumentList(Term));
    else
      if (term_IsVariable(Term))
	VarSym = ((VarSym < (Help = term_TopSymbol(Term))) ?
		  Help : VarSym);
    while (!stack_Empty(Bottom) && list_Empty(stack_Top()))
      stack_Pop();
    if (!stack_Empty(Bottom)) {
      Term = (TERM)list_Car(stack_Top());
      stack_RplacTop(list_Cdr(stack_Top()));
    }
  } while (!stack_Empty(Bottom));

  return VarSym;
}

void clause_SetMaxLitFlagsAux(CLAUSE Clause, FLAGSTORE FlagStore,
			   PRECEDENCE Precedence, BOOL VarIsConst)
/**********************************************************
  INPUT:   A clause, a flag store and a precedence.
           A boolean flag.
  RETURNS: Nothing.
  EFFECT:  Sets the maxLit-flag for maximal literals.
           if <VarIsConst> is set then variables are
           interpreted as skolem constants.
***********************************************************/
{
  int        i,j,n,fa;
  LITERAL    ActLit,CompareLit;
  BOOL       Result, Twin;
  ord_RESULT HelpRes;
  
  n   = clause_Length(Clause);
  fa  = clause_FirstAntecedentLitIndex(Clause);

  clause_RemoveFlag(Clause,CLAUSESELECT);
  for (i = clause_FirstLitIndex(); i < n; i++)
    clause_LiteralFlagReset(clause_GetLiteral(Clause, i));
  if (term_StampOverflow(clause_STAMPID))
    for (i = clause_FirstLitIndex(); i < n; i++)
      term_ResetTermStamp(clause_LiteralSignedAtom(clause_GetLiteral(Clause, i)));
  term_StartStamp();

  /*printf("\n Start: "); clause_Print(Clause);*/

  for (i = fa; i < n; i++) {
    ActLit = clause_GetLiteral(Clause, i);
    if (!term_HasTermStamp(clause_LiteralSignedAtom(ActLit))) {
      Result  = TRUE;
      Twin    = FALSE;
      for (j = fa; j < n && Result; j++)
	if (i != j) {
	  CompareLit = clause_GetLiteral(Clause, j);
	  HelpRes    = ord_LiteralCompareAux(clause_LiteralSignedAtom(ActLit),					  
            clause_LiteralGetOrderStatus(ActLit),
					  clause_LiteralSignedAtom(CompareLit),					  
            clause_LiteralGetOrderStatus(CompareLit),
					  FALSE, VarIsConst, FlagStore, Precedence);
	  
	  /*printf("\n\tWe compare: ");
	    fol_PrintDFG(clause_LiteralAtom(ActLit));
	    putchar(' ');
	    fol_PrintDFG(clause_LiteralAtom(CompareLit));
	    printf(" Result: "); ord_Print(HelpRes);*/

	  if (ord_IsEqual(HelpRes))
	    Twin = TRUE;
	  if (ord_IsSmallerThan(HelpRes))
	    Result = FALSE;
	  if (ord_IsGreaterThan(HelpRes))
	    term_SetTermStamp(clause_LiteralSignedAtom(CompareLit));
	}
      if (Result) {
	clause_LiteralSetFlag(ActLit, MAXIMAL);
	if (!Twin)
	  clause_LiteralSetFlag(ActLit, STRICTMAXIMAL);
      }
    }
  }
  term_StopStamp();
  /*printf("\n End: "); clause_Print(Clause);*/
}


void clause_SetMaxLitFlagsSkolem(CLAUSE Clause, FLAGSTORE FlagStore,
			   PRECEDENCE Precedence)
/**********************************************************
  INPUT:   A clause, a flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  Sets the maxLit-flag for maximal literals.
           variables are interpreted as skolem constants
***********************************************************/
{
    clause_SetMaxLitFlagsAux(Clause, FlagStore, Precedence, TRUE);
}

void clause_SetMaxLitFlags(CLAUSE Clause, FLAGSTORE FlagStore,
			   PRECEDENCE Precedence)
/**********************************************************
  INPUT:   A clause, a flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  Sets the maxLit-flag for maximal literals.
***********************************************************/
{
    clause_SetMaxLitFlagsAux(Clause, FlagStore, Precedence, FALSE);
}


void clause_SetNativeMaxLitFlags(CLAUSE Clause, FLAGSTORE FlagStore,
				 PRECEDENCE Precedence)
/**********************************************************
  INPUT:   A clause, a flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  Sets the maxLit-flag for maximal literals, 
	   keeping the sel flag on literals in input clauses.
***********************************************************/
{
  int        i,j,n,fa;
  LITERAL    ActLit,CompareLit;
  BOOL       Result, Twin;
  ord_RESULT HelpRes;
  
  n   = clause_Length(Clause);
  fa  = clause_FirstAntecedentLitIndex(Clause);

  /* Reset literal flags first. */
  if (clause_GetFlag(Clause, CLAUSESELECT))
    /* If selected literals need to be kept, 
       only reset maxlit and strictmaxlit flags. */
    for (i = clause_FirstLitIndex(); i < n; i++)
      clause_LiteralFlagResetAndKeepSelFlag(clause_GetLiteral(Clause, i));
  else {
   /* Otherwise, reset the literal flag. */
    for (i = clause_FirstLitIndex(); i < n; i++)
      clause_LiteralFlagReset(clause_GetLiteral(Clause, i));
  }
  if (term_StampOverflow(clause_STAMPID))
    for (i = clause_FirstLitIndex(); i < n; i++)
      term_ResetTermStamp(clause_LiteralSignedAtom(clause_GetLiteral(Clause, i)));
  term_StartStamp();

  /*printf("\n Start: "); clause_Print(Clause);*/

  for (i = fa; i < n; i++) {
    ActLit = clause_GetLiteral(Clause, i);
    if (!term_HasTermStamp(clause_LiteralSignedAtom(ActLit))) {
      Result  = TRUE;
      Twin    = FALSE;
      for (j = fa; j < n && Result; j++)
	if (i != j) {
	  CompareLit = clause_GetLiteral(Clause, j);
	  HelpRes    = ord_LiteralCompare(clause_LiteralSignedAtom(ActLit),					  
            clause_LiteralGetOrderStatus(ActLit),
					  clause_LiteralSignedAtom(CompareLit),					  
            clause_LiteralGetOrderStatus(CompareLit),
					  FALSE, FlagStore, Precedence);
	  
	  /*printf("\n\tWe compare: ");
	    fol_PrintDFG(clause_LiteralAtom(ActLit));
	    putchar(' ');
	    fol_PrintDFG(clause_LiteralAtom(CompareLit));
	    printf(" Result: "); ord_Print(HelpRes);*/

	  if (ord_IsEqual(HelpRes))
	    Twin = TRUE;
	  if (ord_IsSmallerThan(HelpRes))
	    Result = FALSE;
	  if (ord_IsGreaterThan(HelpRes))
	    term_SetTermStamp(clause_LiteralSignedAtom(CompareLit));
	}
      if (Result) {
	clause_LiteralSetFlag(ActLit, MAXIMAL);
	if (!Twin)
	  clause_LiteralSetFlag(ActLit, STRICTMAXIMAL);
      }
    }
  }
  term_StopStamp();
  /*printf("\n End: "); clause_Print(Clause);*/
}

SYMBOL clause_SearchMaxVar(CLAUSE Clause)
/**********************************************************
  INPUT:   A clause.
  RETURNS: The maximal symbol of the clauses variables.
           If the clause is ground, symbol_GetInitialStandardVarCounter().
***********************************************************/
{
  int i, n;
  SYMBOL Help, MaxSym;
  
  n = clause_Length(Clause);

  MaxSym = symbol_GetInitialStandardVarCounter();

  for (i = 0; i < n; i++) {
    Help = clause_LiteralMaxVar(clause_GetLiteral(Clause, i));
    if (Help > MaxSym)
      MaxSym = Help;
  }
  return MaxSym;
}


void clause_RenameVarsBiggerThan(CLAUSE Clause, SYMBOL MinVar)
/**********************************************************
  INPUT:   A clause and a variable symbol.
  RETURNS: The clause with variables renamed in a way, that
           all vars are (excl.) bigger than MinVar.
***********************************************************/
{
  int i,n;
  
#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_RenameVarsBiggerThan:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  if (MinVar != symbol_GetInitialStandardVarCounter()) {
    n = clause_Length(Clause);

    term_StartMaxRenaming(MinVar);

    for (i = clause_FirstLitIndex(); i < n; i++)
      term_Rename(clause_GetLiteralTerm(Clause, i));
  }
}

void clause_Normalize(CLAUSE Clause)
/**********************************************************
  INPUT:   A clause.
  RETURNS: The term with normalized Variables, DESTRUCTIVE
           on the variable subterms' topsymbols.
***********************************************************/
{
  int i,n;
  
  n = clause_Length(Clause);

  term_StartMinRenaming();

  for (i = clause_FirstLitIndex(); i < n; i++)
    term_Rename(clause_GetLiteralTerm(Clause, i));
}


void clause_SubstApply(SUBST Subst, CLAUSE Clause)
/**********************************************************
  INPUT:   A clause.
  RETURNS: Nothing.
  EFFECTS: Applies the substitution to the clause.
***********************************************************/
{
  int i,n;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_SubstApply:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  n = clause_Length(Clause);

  for (i=clause_FirstLitIndex(); i<n; i++)
    clause_LiteralSetAtom(clause_GetLiteral(Clause, i),
			  subst_Apply(Subst,clause_GetLiteralAtom(Clause,i)));
}


void clause_ReplaceVariable(CLAUSE Clause, SYMBOL Var, TERM Term)
/**********************************************************
  INPUT:   A clause, a variable symbol, and a term.
  RETURNS: Nothing.
  EFFECTS: All occurences of the variable <Var> in <Clause>
           are replaced by copies if <Term>.
  CAUTION: The maximum variable of the clause is not updated!
***********************************************************/
{
  int i, li;

#ifdef CHECK
  if (!symbol_IsVariable(Var)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_ReplaceVariable: symbol is not a variable");
    misc_FinishErrorReport();
  }
#endif

  li = clause_LastLitIndex(Clause);
  for (i = clause_FirstLitIndex(); i <= li; i++)
    term_ReplaceVariable(clause_GetLiteralAtom(Clause,i), Var, Term);
}


void clause_UpdateMaxVar(CLAUSE Clause)
/**********************************************************
  INPUT:   A clause.
  RETURNS: Nothing.
  EFFECTS: Actualizes the MaxVar slot wrt the actual literals.
***********************************************************/
{
  clause_SetMaxVar(Clause, clause_SearchMaxVar(Clause));
}

BOOL clause_DLLitContainsRedundantSkFunc(TERM term){
/**********************************************************
  INPUT:   A term
  RETURNS: TRUE iff this term uses more a skolem function more than once.
***********************************************************/
  while (term_IsComplex(term)){
    SYMBOL s=term_TopSymbol(term);
    if(!symbol_DLIsMarked(s)){
      symbol_DLMark(s);
    }else{
      return TRUE;
    }
    term=term_FirstArgument(term);
  }
  return FALSE;
}

BOOL clause_DLcontainsRedundantLiteral(CLAUSE Clause){
/**********************************************************
  INPUT:   A Clause
  RETURNS: TRUE iff there is redundant literal based on
           clause_DLLitContainsRedundantSkFunc.
***********************************************************/
  int i;
  for (i=clause_FirstLitIndex(); i<=clause_LastLitIndex(Clause); i++) {
    TERM    t;
    symbol_DLUnmarkAll();
    t = clause_GetLiteralAtom(Clause, i);
    TERM first=term_FirstArgument(t);
    if(clause_DLLitContainsRedundantSkFunc(first)){
      return TRUE;
    }
    symbol_DLUnmarkAll();
    if(list_Length(term_ArgumentList(t))==2){
      TERM second=term_SecondArgument(t);
      if(clause_DLLitContainsRedundantSkFunc(second)){
        return TRUE;
      }
    }
  }
  return FALSE;
}

void clause_PrecomputeOrderingAux(CLAUSE Clause, FLAGSTORE FlagStore, 
			     PRECEDENCE Precedence, BOOL VarIsConst)
/**********************************************************
  INPUT:   An unshared clause, a flag store and a precedence,
           a boolean flag
  RETURNS: Nothing.
  EFFECTS: Reorders the arguments of equality literals
           wrt. the ordering. Thus first arguments aren't smaller
	         after the application. Also computes the ordering
           status of transitive predicates.
           If <VarIsConst> is set
           variables are treated as skolem constants
***********************************************************/
{
  int        i,length;
  LITERAL    Lit;
  ord_RESULT HelpRes;

  /*printf("\n Clause: ");clause_Print(Clause);*/

  length = clause_Length(Clause);

  for (i = clause_FirstLitIndex(); i < length; i++) {
    Lit = clause_GetLiteral(Clause, i);

    if (clause_LiteralIsEquality(Lit)) {
      HelpRes = ord_CompareAux(term_FirstArgument(clause_LiteralAtom(Lit)),
			    term_SecondArgument(clause_LiteralAtom(Lit)),
			    FlagStore, Precedence, VarIsConst);
      
      /*printf("\n\tWe compare: ");
      fol_PrintDFG(term_FirstArgument(clause_LiteralAtom(Lit)));
      putchar(' ');
      fol_PrintDFG(term_SecondArgument(clause_LiteralAtom(Lit)));
      printf("\nResult: "); ord_Print(HelpRes);*/

      switch(HelpRes) {

      case ord_SMALLER_THAN:
	/* printf("\nSwapping: ");
	   term_Print(clause_LiteralAtom(Lit)); DBG */
	term_EqualitySwap(clause_LiteralAtom(Lit));	
  clause_LiteralSetOrderStatus(Lit,ord_GREATER_THAN);
	/*	Swapped = TRUE; */
	break;
      case ord_GREATER_THAN:	
  clause_LiteralSetOrderStatus(Lit,ord_GREATER_THAN);
	break;
      default:
  clause_LiteralSetOrderStatus(Lit,HelpRes);
	break;
      }
    }
    else {      
      if (symbol_HasProperty(term_TopSymbol(clause_LiteralAtom(Lit)), TRANSITIVE)) {
           
        HelpRes = ord_CompareAux(term_FirstArgument(clause_LiteralAtom(Lit)),
			    term_SecondArgument(clause_LiteralAtom(Lit)),
			    FlagStore, Precedence, VarIsConst);
          
          /*
      printf("\n\tWe compare: ");
      fol_PrintDFG(term_FirstArgument(clause_LiteralAtom(Lit)));
      putchar(' ');
      fol_PrintDFG(term_SecondArgument(clause_LiteralAtom(Lit)));
      printf("\nResult: "); ord_Print(HelpRes);
          */

        clause_LiteralSetOrderStatus(Lit,HelpRes);
      } else /* This is slightly arbitrary (although consistent with the theory).*/
        clause_LiteralSetOrderStatus(Lit,ord_GREATER_THAN);
    }
  }
}

void clause_PrecomputeOrderingSkolem(CLAUSE Clause, FLAGSTORE FlagStore,
                                    PRECEDENCE Precedence)
/**********************************************************
  INPUT:   A clause, a Flagstore and a precedence
  RETURNS: Nothing.
  EFFECTS: Reorders the arguments of equalities. 
           Also computes the ordering
           status of transitive predicates.
           Variables are interpreted as skolem constants
***********************************************************/
{
      clause_PrecomputeOrderingAux(Clause, FlagStore, Precedence, TRUE);
}

void clause_PrecomputeOrdering(CLAUSE Clause, FLAGSTORE FlagStore,
                             PRECEDENCE Precedence)
/**********************************************************
  INPUT:   A Clause, a Flagstore and a precedence
  RETURNS: Nothing.
  EFFECTS: Reorders the arguments of equality literals
           wrt. the ordering. Thus first arguments aren't smaller
	   after the application. Also computes the ordering
           status of transitive predicates. If <VarIsConst> is set
           variables are treated as skolem constants
***********************************************************/
{
        clause_PrecomputeOrderingAux(Clause, FlagStore, Precedence, FALSE);
}


void  clause_InsertFlatIntoIndex(CLAUSE Clause, st_INDEX Index)
/**********************************************************
  INPUT:   An unshared clause and an index.
  EFFECT:  The atoms of <Clause> are inserted into the index.
           A link from the atom to its literal via the supertermlist
	   is established.
***********************************************************/
{
  int     i,n;
  LITERAL Lit;
  TERM    Atom    ;

  n = clause_Length(Clause);

  for (i=clause_FirstLitIndex();i<n;i++) {
    Lit  = clause_GetLiteral(Clause,i);
    Atom = clause_LiteralAtom(Lit);
    term_RplacSupertermList(Atom, list_List(Lit));
    st_EntryCreate(Index, Atom, Atom, cont_LeftContext());
  }
}

void  clause_DeleteFlatFromIndex(CLAUSE Clause, st_INDEX Index)
/**********************************************************
  INPUT:   An unshared clause and an index.
  EFFECT:  The clause is deleted from the index and deleted itself.
***********************************************************/
{
  int     i,n;
  LITERAL Lit;
  TERM    Atom    ;

  n = clause_Length(Clause);

  for (i=clause_FirstLitIndex();i<n;i++) {
    Lit  = clause_GetLiteral(Clause,i);
    Atom = clause_LiteralAtom(Lit);
    list_Delete(term_SupertermList(Atom));
    term_RplacSupertermList(Atom, list_Nil());
    st_EntryDelete(Index, Atom, Atom, cont_LeftContext());
  }
  clause_Delete(Clause);
}


void  clause_DeleteClauseListFlatFromIndex(LIST Clauses, st_INDEX Index)
/**********************************************************
  INPUT:   An list of unshared clause and an index.
  EFFECT:  The clauses are deleted from the index and deleted itself.
           The list is deleted.
***********************************************************/
{
  LIST Scan;

  for (Scan=Clauses;!list_Empty(Scan);Scan=list_Cdr(Scan))
    clause_DeleteFlatFromIndex(list_Car(Scan), Index);
  
  list_Delete(Clauses);
}


/******************************************************************/
/* Some special functions used by hyper inference/reduction rules */
/******************************************************************/

static void clause_VarToSizeMap(SUBST Subst, NAT* Map, NAT MaxIndex)
/**************************************************************
  INPUT:   A substitution, an array <Map> of size <MaxIndex>+1.
  RETURNS: Nothing.
  EFFECT:  The index i within the array corresponds to the index
           of a variable x_i. For each variable x_i, 0<=i<=MaxIndex
           the size of its substitution term is calculated
           and written to Map[i].
***************************************************************/
{
  NAT *Scan;

#ifdef CHECK
  if (subst_Empty(Subst) || Map == NULL) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_VarToSizeMap: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  /* Initialization */
  for (Scan = Map + MaxIndex; Scan >= Map; Scan--)
    *Scan = 1;
  /* Compute the size of substitution terms */
  for ( ; !subst_Empty(Subst); Subst = subst_Next(Subst))
    Map[subst_Dom(Subst)] = term_ComputeSize(subst_Cod(Subst));
}


static NAT clause_ComputeTermSize(TERM Term, NAT* VarMap)
/**************************************************************
  INPUT:   A term and a an array of NATs.
  RETURNS: The number of symbols in the term.
  EFFECT:  This function calculates the number of symbols in <Term>
           with respect to some substitution s.
           A naive way to do this is to apply the substitution
           to a copy of the term, and to count the number of symbols
           in the copied term.
           We use a more sophisticated algorithm, that first stores
	   the size of every variable's substitution term in <VarMap>.
           We then 'scan' the term and for a variable occurrence x_i
           we simply add the corresponding value VarMap[i] to the result.
           This way we avoid copying the term and the substitution terms,
           which is especially useful if we reuse the VarMap several times.
***************************************************************/
{
  NAT Stack, Size;

#ifdef CHECK
  if (!term_IsTerm(Term)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_ComputeTermSize: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  Size  = 0;
  Stack = stack_Bottom();
  do {
    if (VarMap!=NULL && symbol_IsVariable(term_TopSymbol(Term)))
      Size += VarMap[symbol_VarIndex(term_TopSymbol(Term))];
    else {
      Size++;
      if (term_IsComplex(Term))
        stack_Push(term_ArgumentList(Term));
    }
    while (!stack_Empty(Stack) && list_Empty(stack_Top()))
      stack_Pop();

    if (!stack_Empty(Stack)) {
      Term = list_Car(stack_Top());
      stack_RplacTop(list_Cdr(stack_Top()));
    }
  } while (!stack_Empty(Stack));

  return Size;
}


LIST clause_MoveBestLiteralToFront(LIST Literals, SUBST Subst, SYMBOL MaxVar,
				   BOOL (*Better)(LITERAL, NAT, LITERAL, NAT))
/**************************************************************
  INPUT:   A list of literals, a substitution, the maximum variable
           from the domain of the substitution, and a comparison
	   function. The function <Better> will be called with two
	   literals L1 and L2 and two number S1 and S2, where Si is
	   the size of the atom of Li with respect to variable bindings
	   in <Subst>.
  RETURNS: The same list.
  EFFECT:  This function moves the first literal L to the front of the
           list, so that no other literal L' is better than L with respect
	   to the function <Better>.
	   The function exchanges only the literals, the order of list
	   items within the list is not changed.
***************************************************************/
{
  NAT  *Map, MapSize, BestSize, Size;
  LIST Best, Scan;

#ifdef CHECK
  if (!list_IsSetOfPointers(Literals)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_MoveBestLiteralToFront: List contains duplicates");
    misc_FinishErrorReport();
  }
#endif

  if (list_Empty(Literals) || list_Empty(list_Cdr(Literals)))
    /* < 2 list items, so nothing to do */
    return Literals;

  Map     = NULL;
  MapSize = 0;

  if (!subst_Empty(Subst)) {
    MapSize = symbol_VarIndex(MaxVar) + 1;
    Map     = memory_Malloc(sizeof(NAT)*MapSize);
    clause_VarToSizeMap(Subst, Map, MapSize-1);
  }
  Best     = Literals; /* Remember list item, not literal itself */
  BestSize = clause_ComputeTermSize(clause_LiteralAtom(list_Car(Best)), Map);
  for (Scan = list_Cdr(Literals); !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    Size = clause_ComputeTermSize(clause_LiteralAtom(list_Car(Scan)), Map);
    if (Better(list_Car(Scan), Size, list_Car(Best), BestSize)) {
      /* Actual literal is better than the best encountered so far */
      BestSize = Size;
      Best     = Scan;
    }
  }
  if (Best != Literals) {
    /* Move best literal to the front. We just exchange the literals. */
    LITERAL h = list_Car(Literals);
    list_Rplaca(Literals, list_Car(Best));
    list_Rplaca(Best, h);
  }
  /* cleanup */
  if (Map != NULL)
    memory_Free(Map, sizeof(NAT)*MapSize);

  return Literals;
}


/**************************************************************/
/* Literal Output Functions                                   */
/**************************************************************/

void clause_LiteralPrint(LITERAL Literal)
/**************************************************************
  INPUT:   A Literal.
  RETURNS: Nothing.
***************************************************************/
{
#ifdef CHECK
  if (!clause_LiteralIsLiteral(Literal)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralPrint:");
    misc_ErrorReport("\n Illegal input. Input not a literal.");
    misc_FinishErrorReport();
  }
#endif

  term_PrintPrefix(clause_LiteralSignedAtom(Literal));
}


void clause_LiteralListPrint(LIST LitList)
/**************************************************************
  INPUT:   A list of literals.
  RETURNS: Nothing.
  SUMMARY: Prints the literals  to stdout.
***************************************************************/
{
  while (!(list_Empty(LitList))) {
    clause_LiteralPrint(list_First(LitList));
    LitList = list_Cdr(LitList);

    if (!list_Empty(LitList))
      putchar(' ');
  }
}


void clause_LiteralPrintUnsigned(LITERAL Literal)
/**************************************************************
  INPUT:   A Literal.
  RETURNS: Nothing.
  SUMMARY:
***************************************************************/
{
#ifdef CHECK
  if (!clause_LiteralIsLiteral(Literal)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralPrintUnsigned:");
    misc_ErrorReport("\n Illegal input. Input not a literal.");
    misc_FinishErrorReport();
  }
#endif

  term_PrintPrefix(clause_LiteralAtom(Literal));
  fflush(stdout);
}


void clause_LiteralPrintSigned(LITERAL Literal)
/**************************************************************
  INPUT:   A Literal.
  RETURNS: Nothing.
  SUMMARY:
***************************************************************/
{
#ifdef CHECK
  if (!clause_LiteralIsLiteral(Literal)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralPrintSigned:");
    misc_ErrorReport("\n Illegal input. Input not a literal.");
    misc_FinishErrorReport();
  }
#endif

  term_PrintPrefix(clause_LiteralSignedAtom(Literal));
  fflush(stdout);
}


void clause_LiteralFPrint(FILE* File, LITERAL Lit)
/**************************************************************
  INPUT:   A file and a literal.
  RETURNS: Nothing.
************************************************************/
{
  term_FPrintPrefix(File, clause_LiteralSignedAtom(Lit));
}

void clause_LiteralFPrintUnsigned(FILE* File, LITERAL Lit)
/**************************************************************
  INPUT:   A file and a literal.
  RETURNS: Nothing.
************************************************************/
{
  term_FPrintPrefix(File, clause_LiteralAtom(Lit));
}

LIST clause_GetLiteralSubSetList(CLAUSE Clause, int StartIndex, int EndIndex, 
				 FLAGSTORE FlagStore, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a start literal index, an end index, a
           flag store and a precedence.
  RETURNS: The list of literals between the start and the end 
           index. 
	   It is a list of pointers, not a list of indices.
**************************************************************/
{

  LIST Result;
  int  i;

#ifdef CHECK
  if (!clause_IsClause(Clause, FlagStore, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_ReplaceLiteralSubSet:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
  if ((StartIndex < clause_FirstLitIndex())
      || (EndIndex > clause_LastLitIndex(Clause))) {
    misc_ErrorReport("\n In clause_ReplaceLiteralSubSet:");
    misc_ErrorReport("\n Illegal input.");
    misc_ErrorReport("\n Index out of range.");
    misc_FinishErrorReport();
  }
#endif

  Result = list_Nil();

  for (i=StartIndex;
       i<=EndIndex;
       i++) {
    Result = list_Cons(clause_GetLiteral(Clause, i), Result);
  }

  return Result;
}


void clause_ReplaceLiteralSubSet(CLAUSE Clause, int StartIndex, 
				 int EndIndex, LIST Replacement,
				 FLAGSTORE FlagStore, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a start literal index, an end literal 
           index, a flag store and a precedence.
  RETURNS: None.
  EFFECT:  Replaces the subset of literals in <Clause> with 
           indices between (and including) <StartIndex> and
	   <EndIndex> with literals from the <Replacement>
	   list.
**************************************************************/
{

  int i;
  LIST Scan;

#ifdef CHECK
  if (!clause_IsClause(Clause, FlagStore, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_ReplaceLiteralSubSet:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
  if ((StartIndex < clause_FirstLitIndex())
      || (EndIndex > clause_LastLitIndex(Clause))) {
    misc_ErrorReport("\n In clause_ReplaceLiteralSubSet:");
    misc_ErrorReport("\n Illegal input.");
    misc_ErrorReport("\n Index out of range.");
    misc_FinishErrorReport();
  }
  if (list_Length(Replacement) != (EndIndex - StartIndex + 1)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_ReplaceLiteralSubSet:");
    misc_ErrorReport("\n Illegal input.  Replacement list size");
    misc_ErrorReport("\n and set size don't match");
    misc_FinishErrorReport();
  }
#endif

  for (i = StartIndex, Scan = Replacement; 
       i <= EndIndex; 
       i++, Scan = list_Cdr(Scan)) {
    clause_SetLiteral(Clause, i, list_Car(Scan));
  } 
}

 BOOL clause_LiteralsCompare(LITERAL Left, LITERAL Right)
/**************************************************************
  INPUT:   Two literals.
  RETURNS: TRUE if Left <= Right, FALSE otherwise.
  EFFECT:  Compares literals by comparing their terms' arities.
***************************************************************/
{
#ifdef CHECK
  if (!(clause_LiteralIsLiteral(Left) && clause_LiteralIsLiteral(Right))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_LiteralsCompare:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  return term_CompareAbstractLEQ(clause_LiteralSignedAtom(Left),
				 clause_LiteralSignedAtom(Right));
}

 void clause_FixLiteralSubsetOrder(CLAUSE Clause, 
						    int StartIndex, 
						    int EndIndex,
						    FLAGSTORE FlagStore,
						    PRECEDENCE Precedence) 
/**************************************************************
  INPUT:   A clause, a start index, an end index a flag store
           and a precedence.
  RETURNS: None.
  EFFECT:  Sorts literals with indices between (and including)
           <StartIndex> and <EndIndex> with respect to an abstract
	   list representation of terms that identifies function
	   symbols with their arity.
***************************************************************/
{

  LIST literals;

#ifdef CHECK
  if ((StartIndex < clause_FirstLitIndex())
      || (EndIndex > clause_LastLitIndex(Clause))) {
    misc_ErrorReport("\n In clause_FixLiteralSubSetOrder:");
    misc_ErrorReport("\n Illegal input.");
    misc_ErrorReport("\n Index out of range.");
    misc_FinishErrorReport();
  }
#endif

  /* Get the literals */
  literals = clause_GetLiteralSubSetList(Clause, StartIndex, EndIndex, FlagStore, Precedence);

  /* Sort them */
  literals = list_Sort(literals, (BOOL (*) (POINTER, POINTER)) clause_LiteralsCompare);

  /* Replace clause literals in subset with sorted literals */
  clause_ReplaceLiteralSubSet(Clause, StartIndex, EndIndex, literals, FlagStore, Precedence);

  list_Delete(literals);
}

void clause_FixLiteralOrder(CLAUSE Clause, FLAGSTORE FlagStore, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a flag store, and a precedence.
  RETURNS: None.
  EFFECT:  Fixes literal order in a <Clause>. Different literal
           types are ordered separately.
***************************************************************/
{
#ifdef CHECK
  if (!clause_IsClause(Clause, FlagStore, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_FixLiteralOrder:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  /* Fix antecedent literal order */
  clause_FixLiteralSubsetOrder(Clause,
			       clause_FirstAntecedentLitIndex(Clause), 
			       clause_LastAntecedentLitIndex(Clause),
			       FlagStore, Precedence);

  /* Fix succedent literal order */
  clause_FixLiteralSubsetOrder(Clause,
			       clause_FirstSuccedentLitIndex(Clause), 
			       clause_LastSuccedentLitIndex(Clause),
			       FlagStore, Precedence);

  /* Fix constraint literal order */
  clause_FixLiteralSubsetOrder(Clause,
			       clause_FirstConstraintLitIndex(Clause), 
			       clause_LastConstraintLitIndex(Clause),
			       FlagStore, Precedence);  

  /* Normalize clause, to get variable names right. */
  clause_Normalize(Clause);
}

static int clause_CompareByWeight(CLAUSE Left, CLAUSE Right)
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by their weight.
***************************************************************/
{
  NAT lweight, rweight;
  int result;

  lweight = clause_Weight(Left);
  rweight = clause_Weight(Right);

  if (lweight < rweight) {
    result = -1;
  }
  else if (lweight > rweight) {
    result = 1;
  }
  else {
    result = 0;
  }

  return result;
}

static int clause_CompareByClausePartSize(CLAUSE Left, CLAUSE Right) 
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by the number of literals in
           the antecedent, succedent and constraint.
***************************************************************/
{
  int lsize, rsize;
  
  lsize = clause_NumOfAnteLits(Left);
  rsize = clause_NumOfAnteLits(Right);
  if (lsize < rsize)
    return -1;
  else if (lsize > rsize)
    return 1;
  
  lsize = clause_NumOfSuccLits(Left);
  rsize = clause_NumOfSuccLits(Right);
  
  if (lsize < rsize)
    return -1;
  else if (lsize > rsize)
    return 1;
  
  lsize = clause_NumOfConsLits(Left);
  rsize = clause_NumOfConsLits(Right);
  
  if (lsize < rsize)
    return -1;
  else if (lsize > rsize)
    return 1;
  
  return 0;
}

void clause_CountSymbols(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: None.
  EFFECT:  Counts the non-variable symbols in the clause, and 
           increases their counts accordingly.
***************************************************************/
{
  int i;

  for (i=clause_FirstLitIndex(); i<=clause_LastLitIndex(Clause); i++) {
    LITERAL l;
    TERM    t;

    l = clause_GetLiteral(Clause, i);
    if (clause_LiteralIsPredicate(l)) {
      SYMBOL S;

      S = clause_LiteralPredicate(l);
      symbol_SetCount(S, symbol_GetCount(S) + 1);
    }

    t = clause_GetLiteralAtom(Clause, i);

    term_CountSymbols(t);
  }
}


LIST clause_ListOfPredicates(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: A list of symbols.
  EFFECT:  Creates a list of predicates occurring in the clause.
           A predicate occurs several times in the list, if 
	   it does so in the clause.
***************************************************************/
{
  LIST preds;
  int i;

  preds = list_Nil();

  for (i=clause_FirstLitIndex(); i<=clause_LastLitIndex(Clause); i++) {
    LITERAL l;
	
    l = clause_GetLiteral(Clause, i);
    if (clause_LiteralIsPredicate(l)) {
      preds = list_Cons((POINTER) clause_LiteralPredicate(l), preds);
    }
  }

  return preds;
}

LIST clause_ListOfConstants(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: A list of symbols.
  EFFECT:  Creates a list of constants occurring in the clause.
           A constant occurs several times in the list, if 
	   it does so in the clause.
***************************************************************/
{
  LIST consts;
  int i;

  consts = list_Nil();

  for (i=clause_FirstLitIndex(); i<=clause_LastLitIndex(Clause); i++) {
    TERM t;
	
    t = clause_GetLiteralAtom(Clause, i);

    consts = list_Nconc(term_ListOfConstants(t), consts);
  }

  return consts;
}

LIST clause_ListOfVariables(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: A list of variables.
  EFFECT:  Creates a list of variables occurring in the clause.
           A variable occurs several times in the list, if 
	   it does so in the clause.
***************************************************************/
{
  LIST vars;
  int i;

  vars = list_Nil();

  for (i=clause_FirstLitIndex(); i<=clause_LastLitIndex(Clause); i++) {
    TERM t;
	
    t = clause_GetLiteralAtom(Clause, i);

    vars = list_Nconc(term_ListOfVariables(t), vars);
  }

  return vars;
}

LIST clause_ListOfFunctions(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: A list of symbols.
  EFFECT:  Creates a list of functions occurring in the clause.
           A function occurs several times in the list, if 
	   it does so in the clause.
***************************************************************/
{
  LIST funs;
  int i;

  funs = list_Nil();

  for (i=clause_FirstLitIndex(); i<=clause_LastLitIndex(Clause); i++) {
    TERM t;
	
    t = clause_GetLiteralAtom(Clause, i);

    funs = list_Nconc(term_ListOfFunctions(t), funs);
  }

  return funs;
}

static int clause_CompareByPredicateDistribution(CLAUSE Left, CLAUSE Right)
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by the frequency of predicates.
***************************************************************/
{
  LIST lpreds, rpreds;
  int  result;

  lpreds = clause_ListOfPredicates(Left);
  rpreds = clause_ListOfPredicates(Right);

  result = list_CompareMultisetsByElementDistribution(lpreds, rpreds);

  list_Delete(lpreds);
  list_Delete(rpreds);

  return result;
}

static int clause_CompareByConstantDistribution(CLAUSE Left, CLAUSE Right)
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by the frequency of constants.
***************************************************************/
{
  LIST lconsts, rconsts;
  int  result;

  lconsts = clause_ListOfConstants(Left);
  rconsts = clause_ListOfConstants(Right);

  result = list_CompareMultisetsByElementDistribution(lconsts, rconsts);

  list_Delete(lconsts);
  list_Delete(rconsts);

  return result;
}

static int clause_CompareByVariableDistribution(CLAUSE Left, CLAUSE Right)
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by the frequency of variables.
***************************************************************/
{
  LIST lvars, rvars;
  int  result;

  lvars = clause_ListOfVariables(Left);
  rvars = clause_ListOfVariables(Right);

  result = list_CompareMultisetsByElementDistribution(lvars, rvars);

  list_Delete(lvars);
  list_Delete(rvars);

  return result;
}

static int clause_CompareByFunctionDistribution(CLAUSE Left, CLAUSE Right)
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by the frequency of functions.
***************************************************************/
{
  LIST lfuns, rfuns;
  int  result;

  lfuns = clause_ListOfFunctions(Left);
  rfuns = clause_ListOfFunctions(Right);

  result = list_CompareMultisetsByElementDistribution(lfuns, rfuns);

  list_Delete(lfuns);
  list_Delete(rfuns);

  return result;
}

static int clause_CompareByDepth(CLAUSE Left, CLAUSE Right) 
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by their depth.
***************************************************************/
{
  if (clause_Depth(Left) < clause_Depth(Right))
    return -1;
  else if (clause_Depth(Left) > clause_Depth(Right))
    return 1;

  return 0;
}

static int clause_CompareByMaxVar(CLAUSE Left, CLAUSE Right)
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by their maximal variable.
***************************************************************/ 
{
  if (clause_MaxVar(Left) < clause_MaxVar(Right))
    return -1;
  else if (clause_MaxVar(Left) > clause_MaxVar(Right))
    return 1;

  return 0;
}

static int clause_CompareByLiterals(CLAUSE Left, CLAUSE Right) 
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by comparing their literals 
           from left to right.
***************************************************************/
{
  int firstlitleft, lastlitleft;
  int firstlitright, lastlitright;
  int i, j;
  int result;

  result = 0;

  /* Compare sorted literals from right to left */
  
  firstlitleft  = clause_FirstLitIndex();
  lastlitleft   = clause_LastLitIndex(Left);
  firstlitright = clause_FirstLitIndex();
  lastlitright  = clause_LastLitIndex(Right);
  
  for (i = lastlitleft, j = lastlitright;
       i >= firstlitleft && j >= firstlitright;
       --i, --j) {
    TERM lterm, rterm;
    
    lterm = clause_GetLiteralTerm(Left, i);
    rterm = clause_GetLiteralTerm(Right, j);
    
    result = term_CompareAbstract(lterm, rterm);
    
    if (result != 0)
      break;
  }

  if (result == 0) {
    /* All literals compared equal, so check if someone has 
       uncompared literals left over.
    */
    if ( i > j) {
      /* Left clause has uncompared literals left over. */
      result =  1;
    }
    else if (i < j) {
      /* Right clause has uncompared literals left over. */
      result = -1;
    }
  }

  return result;
}

static int clause_CompareBySymbolOccurences(CLAUSE Left, CLAUSE Right) 
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by comparing the occurrences of
           symbols in their respective literals from left to 
	   right. If a symbol occurs less frequently than
	   its positional equivalent in the other clause,
	   then the first clause is smaller.
***************************************************************/
{
  int firstlitleft, lastlitleft;
  int firstlitright, lastlitright;
  int i, j;
  int result;

  result = 0;

  /* Compare sorted literals from right to left */
  
  firstlitleft  = clause_FirstLitIndex();
  lastlitleft   = clause_LastLitIndex(Left);
  firstlitright = clause_FirstLitIndex();
  lastlitright  = clause_LastLitIndex(Right);
  
  for (i = lastlitleft, j = lastlitright;
       i >= firstlitleft && j >= firstlitright;
       --i, --j) {
    TERM lterm, rterm;
    LITERAL llit, rlit;
	
    llit = clause_GetLiteral(Left, i);
    rlit = clause_GetLiteral(Right, j);

    if (clause_LiteralIsPredicate(llit)) {
      if (clause_LiteralIsPredicate(rlit)) {
	if (symbol_GetCount(clause_LiteralPredicate(llit)) 
	    < symbol_GetCount(clause_LiteralPredicate(rlit))) {
	  return -1;
	}
	else if (symbol_GetCount(clause_LiteralPredicate(llit)) 
	    > symbol_GetCount(clause_LiteralPredicate(rlit))) {
	  return 1;
	}
      }
    }
    
    lterm = clause_GetLiteralTerm(Left, i);
    rterm = clause_GetLiteralTerm(Right, j);
    
    result = term_CompareBySymbolOccurences(lterm, rterm);
    
    if (result != 0)
      break;
  }

  return result;
}

int clause_CompareAbstract(CLAUSE Left, CLAUSE Right)
/**************************************************************
  INPUT:   Two clauses.
  RETURNS: 1 if left > right, -1 if left < right, 0 otherwise.
  EFFECT:  Compares two clauses by their weight. If the weight
           is equal, it compares the clauses by the arity of
	   their literals from right to left.
  CAUTION: Expects clause literal order to be fixed.
***************************************************************/
{

  typedef int (*CLAUSE_COMPARE_FUNCTION) (CLAUSE, CLAUSE);

  static const CLAUSE_COMPARE_FUNCTION clause_compare_functions [] = {
    clause_CompareByWeight,
    clause_CompareByDepth,
    clause_CompareByMaxVar,
    clause_CompareByClausePartSize,
    clause_CompareByLiterals,
    clause_CompareBySymbolOccurences,
    clause_CompareByPredicateDistribution,
    clause_CompareByConstantDistribution,
    clause_CompareByFunctionDistribution,
    clause_CompareByVariableDistribution,
  };

  int result;
  int i;
  int functions;

  result    = 0;
  functions = sizeof(clause_compare_functions)/sizeof(CLAUSE_COMPARE_FUNCTION);

  for (i = 0; i < functions; i++) {
    result = clause_compare_functions[i](Left, Right);

    if ( result != 0) {
      return result;
    }
  }

  return 0;
}


/**************************************************************/
/* Clause functions                                           */
/**************************************************************/

void clause_Init(void)
/**************************************************************
  INPUT:   None.
  RETURNS: Nothing.
  SUMMARY: Initializes the clause counter and other variables
           from this module.
***************************************************************/
{
  int i;
  clause_SetCounter(1);
  clause_STAMPID = term_GetStampID();
  for (i = 0; i <= clause_MAXWEIGHT; i++)
    clause_SORT[i] = list_Nil();
}


CLAUSE clause_CreateBody(int ClauseLength)
/**************************************************************
  INPUT:   The number of literals as integer.
  RETURNS: A new generated clause node for 'ClauseLength'
  MEMORY:  Allocates a CLAUSE_NODE and the needed array for LITERALs.
*************************************************************/
{
  CLAUSE Result;

  Result                 = (CLAUSE)memory_Malloc(sizeof(CLAUSE_NODE));

  Result->clausenumber   = clause_IncreaseCounter();
  Result->maxVar         = symbol_GetInitialStandardVarCounter();
  Result->flags          = 0;
  Result->depth          = 0;
  Result->splitpotential = 0.0;
  clause_InitSplitData(Result);
  Result->weight         = clause_WEIGHTUNDEFINED;
  Result->parentCls      = list_Nil();
  Result->parentLits     = list_Nil();

  Result->c              = 0;
  Result->a              = 0;
  Result->s              = 0;

  if (ClauseLength != 0)
    Result->literals =
      (LITERAL *)memory_Malloc((ClauseLength) * sizeof(LITERAL));

  clause_SetFromInput(Result);

  return Result;
}



CLAUSE clause_CreateAux(LIST Constraint, LIST Antecedent, LIST Succedent,
		     FLAGSTORE Flags, PRECEDENCE Precedence, BOOL VarIsConst)
/**************************************************************
  INPUT:   Three lists of pointers to atoms, a flag store, 
           a precedence and a flag
  RETURNS: The new generated clause.
  MEMORY:  Allocates a CLAUSE_NODE and the needed LITERAL_NODEs,
           uses the terms from the lists, additionally allocates
	   termnodes for the fol_Not() in Const. and Ante.
  CAUTION: If <VarIsConst> is TRUE then variables are interpreted
           as constants
*************************************************************/
{
  CLAUSE Result;
  int    i, c, a, s;
  
  Result                = (CLAUSE)memory_Malloc(sizeof(CLAUSE_NODE));
  
  Result->clausenumber  = clause_IncreaseCounter();
  Result->flags         = 0;
  Result->depth         = 0;
  Result->weight        = clause_WEIGHTUNDEFINED;
  Result->splitpotential = 0.0;
  clause_InitSplitData(Result);
  Result->parentCls     = list_Nil();
  Result->parentLits    = list_Nil();

  Result->c             = (c = list_Length(Constraint));
  Result->a             = (a = list_Length(Antecedent));
  Result->s             = (s = list_Length(Succedent));

  if (!clause_IsEmptyClause(Result))
    Result->literals =
      (LITERAL *) memory_Malloc((c+a+s) * sizeof(LITERAL));
  
  for (i = 0; i < c; i++) {
    Result->literals[i] =
      clause_LiteralCreate(term_Create(fol_Not(),
	list_List((TERM)list_Car(Constraint))),Result);
    Constraint = list_Cdr(Constraint);
  }
  
  a += c;
  for ( ; i < a; i++) {
    Result->literals[i] =
      clause_LiteralCreate(term_Create(fol_Not(),
	list_List((TERM)list_Car(Antecedent))), Result);
    Antecedent = list_Cdr(Antecedent);
  }
  
  s += a;
  for ( ; i < s; i++) {
    Result->literals[i] =
      clause_LiteralCreate((TERM) list_Car(Succedent), Result);
    Succedent = list_Cdr(Succedent);
  }
  
  if(VarIsConst)
        clause_PrecomputeOrderingAndReInitSkolem(Result, Flags, Precedence);
  else
        clause_PrecomputeOrderingAndReInit(Result, Flags, Precedence);

  clause_SetFromInput(Result);
  
  return Result;
}


CLAUSE clause_Create(LIST Constraint, LIST Antecedent, LIST Succedent,
		     FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   Three lists of pointers to atoms, a flag store and 
           a precedence.
  RETURNS: The new generated clause.
  MEMORY:  Allocates a CLAUSE_NODE and the needed LITERAL_NODEs,
           uses the terms from the lists, additionally allocates
	   termnodes for the fol_Not() in Const. and Ante.
*************************************************************/
{
        return clause_CreateAux(Constraint, Antecedent, Succedent,
                                Flags, Precedence, FALSE);
}


CLAUSE clause_CreateSkolem(LIST Constraint, LIST Antecedent, LIST Succedent,
		     FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   Three lists of pointers to atoms, a flag store and 
           a precedence.
  RETURNS: The new generated clause.
  MEMORY:  Allocates a CLAUSE_NODE and the needed LITERAL_NODEs,
           uses the terms from the lists, additionally allocates
	   termnodes for the fol_Not() in Const. and Ante.
  CAUTION: Variables are interpreted to as constants
*************************************************************/
{
        return clause_CreateAux(Constraint, Antecedent, Succedent,
                                Flags, Precedence, TRUE);
}


CLAUSE clause_CreateCrude(LIST Constraint, LIST Antecedent, LIST Succedent,
			  BOOL Con)
/**************************************************************
  INPUT:   Three lists of pointers to literals (!) and a Flag indicating
           whether the clause comes from the conjecture part of of problem.
	   It is assumed that Constraint and Antecedent literals are negative,
	   whereas Succedent literals are positive.
  RETURNS: The new generated clause, where a clause_PrecomputeOrderingAndReInit has still
           to be performed, i.e., variables are not normalized, maximal literal
	   flags not set, equations not oriented, the weight is not set.
  MEMORY:  Allocates a CLAUSE_NODE and the needed LITERAL_NODEs,
           uses the terms from the lists, additionally allocates
	   termnodes for the fol_Not() in Const. and Ante.
****************************************************************/
{
  CLAUSE Result;
  int i,c,a,s;
  
  Result                = (CLAUSE)memory_Malloc(sizeof(CLAUSE_NODE));
  
  Result->clausenumber  = clause_IncreaseCounter();
  Result->flags         = 0;
  if (Con)
    clause_SetFlag(Result, CONCLAUSE);

  Result->depth         = 0;
  Result->weight        = clause_WEIGHTUNDEFINED;
  Result->splitpotential = 0.0;
  clause_InitSplitData(Result);
  Result->parentCls     = list_Nil();
  Result->parentLits    = list_Nil();

  Result->c             = (c = list_Length(Constraint));
  Result->a             = (a = list_Length(Antecedent));
  Result->s             = (s = list_Length(Succedent));

  if (!clause_IsEmptyClause(Result))
    Result->literals = (LITERAL *)memory_Malloc((c+a+s) * sizeof(LITERAL));
  
  for (i = 0; i < c; i++) {
    Result->literals[i] =
      clause_LiteralCreate(list_Car(Constraint),Result);
    Constraint = list_Cdr(Constraint);
  }
  
  a += c;
  for ( ; i < a; i++) {
    Result->literals[i] =
      clause_LiteralCreate(list_Car(Antecedent), Result);
    Antecedent = list_Cdr(Antecedent);
  }
  
  s += a;
  for ( ; i < s; i++) {
    Result->literals[i] = clause_LiteralCreate(list_Car(Succedent), Result);
    Succedent = list_Cdr(Succedent);
  }
  
  clause_SetFromInput(Result);
  
  return Result;
}


CLAUSE clause_CreateUnnormalized(LIST Constraint, LIST Antecedent,
				 LIST Succedent)
/**************************************************************
  INPUT:   Three lists of pointers to atoms.
  RETURNS: The new generated clause.
  MEMORY:  Allocates a CLAUSE_NODE and the needed LITERAL_NODEs,
           uses the terms from the lists, additionally allocates
	   termnodes for the fol_Not() in Const. and Ante.
  CAUTION: The weight of the clause is not set correctly and
           equations are not oriented!
****************************************************************/
{
  CLAUSE Result;
  int i,c,a,s;

  Result                = (CLAUSE)memory_Malloc(sizeof(CLAUSE_NODE));

  Result->clausenumber  = clause_IncreaseCounter();
  Result->flags         = 0;
  Result->depth         = 0;
  Result->weight        = clause_WEIGHTUNDEFINED;
  Result->splitpotential = 0.0;
  clause_InitSplitData(Result);
  Result->parentCls     = list_Nil();
  Result->parentLits    = list_Nil();

  Result->c             = (c = list_Length(Constraint));
  Result->a             = (a = list_Length(Antecedent));
  Result->s             = (s = list_Length(Succedent));

  if (!clause_IsEmptyClause(Result)) {
    Result->literals = (LITERAL *)memory_Malloc((c+a+s) * sizeof(LITERAL));
  
    for (i = 0; i < c; i++) {
      Result->literals[i] =
	clause_LiteralCreate(term_Create(fol_Not(),
					 list_List(list_Car(Constraint))),
			     Result);
      Constraint = list_Cdr(Constraint);
    }

    a += c;
    for ( ; i < a; i++) {
      Result->literals[i]  =
	clause_LiteralCreate(term_Create(fol_Not(),
					 list_List(list_Car(Antecedent))),
			     Result);
      Antecedent = list_Cdr(Antecedent);
    }

    s += a;
    for ( ; i < s; i++) {
      Result->literals[i] =
	clause_LiteralCreate((TERM)list_Car(Succedent), Result);
      Succedent = list_Cdr(Succedent);
    }
    clause_UpdateMaxVar(Result);
  }

  return Result;
}

CLAUSE clause_CreateFromLiteralLists(LIST Constraint, LIST Antecedent, 
									 LIST Succedent, BOOL Conclause,
									 TERM selected)
/**************************************************************
  INPUT:   Three lists of literals, a boolean flag indicating 
           whether the clause is a conjecture clause, and a 
           selected term.
  RETURNS: The new generated clause.
  EFFECT:  The result clause will be normalized and the maximal
           variable will be set. If the flag is set, the clause 
           will be set as a conjecture clause. If the selected 
           term is not NULL, its corresponding literal will be 
           selected. 
           This function is intended for the parser for creating 
           clauses at a time when the ordering and weight flags 
           aren't determined finally.
  MEMORY:  Allocates a CLAUSE_NODE and the needed LITERAL_NODEs,
           uses the terms from the lists.
****************************************************************/
{
	CLAUSE Result;
	
    Result = clause_CreateUnnormalized(Constraint, Antecedent, Succedent);

	if(Conclause)
      clause_SetFlag(Result, CONCLAUSE);

    if (selected != (TERM) NULL) {
      int i;
      for (i = clause_FirstAntecedentLitIndex(Result); 
           i <= clause_LastAntecedentLitIndex(Result); 
           ++i) {
      	TERM negated;      	
      	negated = clause_GetLiteralAtom(Result, i);
      	if (negated == selected) {
      	  clause_LiteralSetFlag(clause_GetLiteral(Result, i),
      		                    LITSELECT);
      	  clause_SetFlag(Result, CLAUSESELECT);
      	  break;
      	}
      }
    }
    	
    clause_Normalize(Result);
    clause_UpdateMaxVar(Result);
    clause_SetFromInput(Result);
    
    return Result;
} 

CLAUSE clause_CreateFromLiterals(LIST LitList, BOOL Sorts, BOOL Conclause,
				 BOOL Ordering, FLAGSTORE Flags,
				 PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A list of literals, three boolean flags indicating whether
           sort constraint literals should be generated, whether the
	   clause is a conjecture clause, whether the ordering information
	   should be established, a flag store and a precedence.
  RETURNS: The new generated clause.
  EFFECT:  The result clause will be normalized and the maximal
           variable will be set. If <Ordering> is FALSE no additional
	   initializations will be done. This mode is intended for
	   the parser for creating clauses at a time when the ordering
	   and weight flags aren't determined finally.
	   Only if <Ordering> is TRUE the equations will be oriented,
	   the maximal literals will be marked and the clause weight
	   will be set correctly.
  MEMORY:  Allocates a CLAUSE_NODE and the needed LITERAL_NODEs,
           uses the terms from the lists.
****************************************************************/
{
  CLAUSE Result;
  LIST   Antecedent,Succedent,Constraint;
  TERM   Atom;

  Antecedent = list_Nil();
  Succedent  = list_Nil();
  Constraint = list_Nil();

  while (!list_Empty(LitList)) {
    if (term_TopSymbol(list_Car(LitList)) == fol_Not()) {
      Atom = term_FirstArgument(list_Car(LitList));
      if (Sorts && symbol_IsBaseSort(term_TopSymbol(Atom)) && term_IsVariable(term_FirstArgument(Atom)))
	Constraint = list_Cons(list_Car(LitList),Constraint);
      else
	Antecedent = list_Cons(list_Car(LitList),Antecedent);
    }
    else
      Succedent = list_Cons(list_Car(LitList),Succedent);
    LitList = list_Cdr(LitList);
  }
  
  Constraint = list_NReverse(Constraint);
  Antecedent = list_NReverse(Antecedent);
  Succedent  = list_NReverse(Succedent);
  Result = clause_CreateCrude(Constraint, Antecedent, Succedent, Conclause);

  list_Delete(Constraint);
  list_Delete(Antecedent);
  list_Delete(Succedent);

  if (Ordering)
    clause_PrecomputeOrderingAndReInit(Result, Flags, Precedence);
  else {
    clause_Normalize(Result);
    clause_UpdateMaxVar(Result);
  }
  
  return Result;
}

void clause_SetSortConstraint(CLAUSE Clause, BOOL Strong, FLAGSTORE Flags,
			      PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a flag indicating whether also negative
           monadic literals with a real term argument should be
	   put in the sort constraint, a flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  Negative monadic literals are put in the sort constraint.
****************************************************************/
{
  LITERAL ActLit,Help;
  TERM    ActAtom;
  int     i,k,NewConLits;


#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_SetSortConstraint:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  i          = clause_LastConstraintLitIndex(Clause);
  NewConLits = 0;

  for (k=clause_FirstAntecedentLitIndex(Clause);k<=clause_LastAntecedentLitIndex(Clause);k++) {
    ActLit  = clause_GetLiteral(Clause,k);
    ActAtom = clause_LiteralAtom(ActLit);
    if (symbol_IsBaseSort(term_TopSymbol(ActAtom)) &&
	(Strong || term_IsVariable(term_FirstArgument(ActAtom)))) {
      if (++i != k) {
	Help = clause_GetLiteral(Clause,i);
	clause_SetLiteral(Clause,i,ActLit);
	clause_SetLiteral(Clause,k,Help);
      }
      NewConLits++;
    }
  }

  clause_SetNumOfConsLits(Clause, clause_NumOfConsLits(Clause) + NewConLits);
  clause_SetNumOfAnteLits(Clause, clause_NumOfAnteLits(Clause) - NewConLits);
  clause_ReInit(Clause, Flags, Precedence);

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_SetSortConstraint:");
    misc_ErrorReport("\n Illegal computations.");
    misc_FinishErrorReport();
  }
#endif

}



void clause_Delete(CLAUSE Clause)
/**************************************************************
  INPUT:   A Clause.
  RETURNS: Nothing.
  MEMORY:  Frees the memory of the clause.
***************************************************************/
{
  int i, n;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) { /* Clause may be a byproduct of some hyper rule */
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_Delete:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  n = clause_Length(Clause);
  
  for (i = 0; i < n; i++)
    clause_LiteralDelete(clause_GetLiteral(Clause,i));

  clause_FreeLitArray(Clause);
  
  list_Delete(clause_ParentClauses(Clause));
  list_Delete(clause_ParentLiterals(Clause));
#ifdef CHECK
  if ((Clause->splitfield != NULL) && (Clause->splitfield_length == 0))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In clause_Delete:");
      misc_ErrorReport("\n Illegal splitfield_length.");
      misc_FinishErrorReport();
    }
  if ((Clause->splitfield == NULL) && (Clause->splitfield_length != 0))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In clause_Delete:");
      misc_ErrorReport("\n Illegal splitfield.");
      misc_FinishErrorReport();
    }
#endif
  if (Clause->splitfield != NULL) {
    
    memory_Free(Clause->splitfield,
		sizeof(SPLITFIELDENTRY) * Clause->splitfield_length);
  }
  clause_Free(Clause);
}


/**************************************************************/
/* Functions to use the sharing for clauses.                  */
/**************************************************************/

void clause_InsertIntoSharing(CLAUSE Clause, SHARED_INDEX ShIndex,
			      FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, an index, a flag store and a precedence.
  RETURNS: Nothing.
  SUMMARY: Inserts the unsigned atoms of 'Clause' into the sharing index.
***************************************************************/
{
  int i, litnum;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_Delete:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
  clause_Check(Clause, Flags, Precedence);
#endif

  litnum = clause_Length(Clause);

  for (i = 0; i < litnum; i++) {
    clause_LiteralInsertIntoSharing(clause_GetLiteral(Clause,i), ShIndex);
  }
}


void clause_DeleteFromSharing(CLAUSE Clause, SHARED_INDEX ShIndex,
			      FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, an Index, a flag store and a precedence.
  RETURNS: Nothing.
  SUMMARY: Deletes 'Clause' and all its literals from the sharing,
           frees the memory of 'Clause'.
***************************************************************/
{
  int i, length;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_DeleteFromSharing:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  length = clause_Length(Clause);
  
  for (i = 0; i < length; i++)
    clause_LiteralDeleteFromSharing(clause_GetLiteral(Clause,i),ShIndex);
  
  clause_FreeLitArray(Clause);
  
  list_Delete(clause_ParentClauses(Clause));
  list_Delete(clause_ParentLiterals(Clause));
#ifdef CHECK
  if ((Clause->splitfield != NULL) && (Clause->splitfield_length == 0))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In clause_DeleteFromSharing:");
      misc_ErrorReport("\n Illegal splitfield_length.");
      misc_FinishErrorReport();
    }
  if ((Clause->splitfield == NULL) && (Clause->splitfield_length != 0))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In clause_DeleteFromSharing:");
      misc_ErrorReport("\n Illegal splitfield.");
      misc_FinishErrorReport();
    }
#endif
  if (Clause->splitfield != NULL) {
    memory_Free(Clause->splitfield,
		sizeof(SPLITFIELDENTRY) * Clause->splitfield_length);
  }
  clause_Free(Clause);
}


void clause_MakeUnshared(CLAUSE Clause, SHARED_INDEX ShIndex)
/**************************************************************
  INPUT:   A Clause and an Index.
  RETURNS: Nothing.
  SUMMARY: Deletes the clauses literals from the sharing and
           replaces them by unshared copies.
***************************************************************/
{
  LITERAL ActLit;
  TERM SharedAtom, AtomCopy;
  int i,LastAnte,length;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_MakeUnshared:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  LastAnte = clause_LastAntecedentLitIndex(Clause);
  length   = clause_Length(Clause);

  for (i = clause_FirstLitIndex(); i <= LastAnte; i++) {
    ActLit     = clause_GetLiteral(Clause, i);
    SharedAtom = clause_LiteralAtom(ActLit);
    AtomCopy   = term_Copy(SharedAtom);
    sharing_Delete(ActLit, SharedAtom, ShIndex);
    clause_LiteralSetNegAtom(ActLit, AtomCopy);
  }

  for ( ; i < length; i++) {
    ActLit     = clause_GetLiteral(Clause, i);
    SharedAtom = clause_LiteralSignedAtom(ActLit);
    AtomCopy   = term_Copy(SharedAtom);
    sharing_Delete(ActLit, SharedAtom, ShIndex);
    clause_LiteralSetPosAtom(ActLit, AtomCopy);
  }
}

void clause_MoveSharedClause(CLAUSE Clause, SHARED_INDEX Source,
			     SHARED_INDEX Destination, FLAGSTORE Flags,
			     PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, two indexes, a flag store, and a precedence.
  RETURNS: Nothing.
  EFFECT:  Deletes <Clause> from <Source> and inserts it into 
           <Destination>.
***************************************************************/
{
  LITERAL Lit;
  TERM    Atom,Copy;
  int     i,length;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_MoveSharedClause:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  length   = clause_Length(Clause);

  for (i = clause_FirstLitIndex(); i < length; i++) {
    Lit  = clause_GetLiteral(Clause, i);
    Atom = clause_LiteralAtom(Lit);
    Copy = term_Copy(Atom);        /* sharing_Insert works destructively on <Copy>'s superterms */
    clause_LiteralSetAtom(Lit, sharing_Insert(Lit, Copy, Destination));
    sharing_Delete(Lit, Atom, Source);
    term_Delete(Copy);
  }
}


void clause_DeleteSharedLiteral(CLAUSE Clause, int Indice, SHARED_INDEX ShIndex, 
				FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, a literals indice, an Index, a flag store
           and a precedence.
  RETURNS: Nothing.
  SUMMARY: Deletes the shared literal from the clause.
  MEMORY:  Various.
***************************************************************/
{

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_DeleteSharedLiteral:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  clause_MakeUnshared(Clause, ShIndex);
  clause_DeleteLiteral(Clause, Indice, Flags, Precedence);
  clause_InsertIntoSharing(Clause, ShIndex, Flags, Precedence);
}


void clause_DeleteClauseList(LIST ClauseList)
/**************************************************************
  INPUT:   A list of unshared clauses.
  RETURNS: Nothing.
  SUMMARY: Deletes all clauses in the list and the list.
  MEMORY:  Frees the lists and the clauses' memory.
 ***************************************************************/
{
  LIST Scan;

  for (Scan = ClauseList; !list_Empty(Scan); Scan = list_Cdr(Scan))
    if (clause_Exists(list_Car(Scan)))
      clause_Delete(list_Car(Scan));

  list_Delete(ClauseList);
}


void clause_DeleteSharedClauseList(LIST ClauseList, SHARED_INDEX ShIndex,
				   FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A list of clauses, an index, a flag store and 
           a precedence.
  RETURNS: Nothing.
  SUMMARY: Deletes all clauses in the list from the sharing.
  MEMORY:  Frees the lists and the clauses' memory.
***************************************************************/
{
  LIST Scan;

  for (Scan = ClauseList; !list_Empty(Scan); Scan = list_Cdr(Scan))
    clause_DeleteFromSharing(list_Car(Scan), ShIndex, Flags, Precedence);

  list_Delete(ClauseList);
}


void clause_DeleteAllIndexedClauses(SHARED_INDEX ShIndex, FLAGSTORE Flags,
				    PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An Index, a flag store and a precedence.
  RETURNS: Nothing.
  SUMMARY: Deletes all clauses' terms from the sharing, frees their
           memory.
  MEMORY:  Frees the memory of all clauses with terms in the index.
***************************************************************/
{
  LIST TermList,DelList,Scan;
  TERM NewVar;
  SYMBOL NewVarSymbol;

  NewVar = term_CreateStandardVariable();
  NewVarSymbol = term_TopSymbol(NewVar);

  TermList = st_GetInstance(cont_LeftContext(), sharing_Index(ShIndex), NewVar);
  /* This should yield a list of all terms in the index
     and thus the sharing. */

  while (!list_Empty(TermList)) {

    DelList = sharing_GetDataList(list_Car(TermList), ShIndex);

    for (Scan = DelList;
	 !list_Empty(Scan);
	 Scan = list_Cdr(Scan))
      list_Rplaca(Scan, clause_LiteralOwningClause(list_Car(Scan)));

    DelList = list_PointerDeleteDuplicates(DelList);

    for (Scan = DelList;
	 !list_Empty(Scan);
	 Scan = list_Cdr(Scan))
      clause_DeleteFromSharing(list_Car(Scan), ShIndex, Flags, Precedence);

    list_Delete(TermList);

    TermList = st_GetInstance(cont_LeftContext(), sharing_Index(ShIndex), NewVar);

    list_Delete(DelList);
  }
  term_Delete(NewVar);
  symbol_Delete(NewVarSymbol);
}


void clause_PrintAllIndexedClauses(SHARED_INDEX ShIndex)
/**************************************************************
  INPUT:   An Index.
  RETURNS: Nothing.
  SUMMARY: Prints all indexed clauses to stdout.
***************************************************************/
{
  LIST TermList,ClList,PrintList,Scan;
  TERM NewVar;
  SYMBOL NewVarSymbol;

  NewVar = term_CreateStandardVariable();
  NewVarSymbol = term_TopSymbol(NewVar);

  TermList = st_GetInstance(cont_LeftContext(), sharing_Index(ShIndex), NewVar);
  /* This should yield a list of all terms in the index
     and thus the sharing. */

  PrintList = list_Nil();

  while (!list_Empty(TermList)) {

    ClList = sharing_GetDataList(list_Car(TermList), ShIndex);

    for (Scan = ClList;
	 !list_Empty(Scan);
	 Scan = list_Cdr(Scan))
      list_Rplaca(Scan, clause_LiteralOwningClause(list_Car(Scan)));

    PrintList = list_NPointerUnion(ClList, PrintList);

    Scan = TermList;
    TermList = list_Cdr(TermList);
    list_Free(Scan);
  }
  clause_ListPrint(PrintList);

  list_Delete(PrintList);

  term_Delete(NewVar);
  symbol_Delete(NewVarSymbol);
}


LIST clause_AllIndexedClauses(SHARED_INDEX ShIndex)
/**************************************************************
  INPUT:   An index
  RETURNS: A list of all the clauses in the index
  MEMORY:  Memory is allocated for the list nodes
***************************************************************/
{
  LIST clauselist, scan;
  clauselist = sharing_GetAllSuperTerms(ShIndex);
  for (scan = clauselist; scan != list_Nil(); scan = list_Cdr(scan))
    list_Rplaca(scan, clause_LiteralOwningClause(list_Car(scan)));
  clauselist = list_PointerDeleteDuplicates(clauselist);
  return clauselist;
}


/**************************************************************/
/* Clause Access Functions                                    */
/**************************************************************/

void clause_DeleteLiteralNN(CLAUSE Clause, int Indice)
/**************************************************************
  INPUT:   An unshared clause, and a literal index.
  RETURNS: Nothing.
  EFFECT:  The literal is position <Indice> is deleted from <Clause>.
           The clause isn't reinitialized afterwards.
  MEMORY:  The memory of the literal with the 'Indice' and 
           memory of its atom is freed.
***************************************************************/
{
  int     i, lc, la, length, shift;
  LITERAL *Literals;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause) || (clause_Length(Clause) <= Indice) ||
      Indice < 0) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_DeleteLiteral:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  lc     = clause_LastConstraintLitIndex(Clause);
  la     = clause_LastAntecedentLitIndex(Clause);
  length = clause_Length(Clause);

  /* Create a new literal array */
  if (length > 1)
    Literals = (LITERAL*) memory_Malloc(sizeof(LITERAL) * (length-1));
  else
    Literals = (LITERAL*) NULL;

  /* Copy literals to the new array */
  shift = 0;
  length--;  /* The loop iterates over the new array */
  for (i = 0; i < length; i++) {
    if (i == Indice)
      shift = 1;
    Literals[i] = Clause->literals[i+shift];
  }

  /* Free literal and old array and set new one */
  clause_LiteralDelete(clause_GetLiteral(Clause, Indice));
  clause_FreeLitArray(Clause);
  Clause->literals = Literals;

  /* Update clause */
  if (Indice <= lc)
    clause_SetNumOfConsLits(Clause, clause_NumOfConsLits(Clause) - 1);
  else if (Indice <= la)
    clause_SetNumOfAnteLits(Clause, clause_NumOfAnteLits(Clause) - 1);
  else
    clause_SetNumOfSuccLits(Clause, clause_NumOfSuccLits(Clause) - 1);
  /* Mark the weight as undefined */
  Clause->weight = clause_WEIGHTUNDEFINED;
}


void clause_DeleteLiteral(CLAUSE Clause, int Indice, FLAGSTORE Flags,
			  PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An unshared clause, a literals index, a flag store,
           and a precedence.
  RETURNS: Nothing.
  EFFECT:  The literal at position <Indice> is deleted from <Clause>.
           In contrast to the function clause_DeleteLiteralNN
	   the clause is reinitialized afterwards.
  MEMORY:  The memory of the literal with the 'Indice' and memory
           of its atom is freed.
***************************************************************/
{
  clause_DeleteLiteralNN(Clause, Indice);
  clause_ReInit(Clause, Flags, Precedence);
}


void clause_DeleteLiterals(CLAUSE Clause, LIST Indices, FLAGSTORE Flags,
			   PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An unshared clause, a list of literal indices a
           flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  The literals given by <Indices> are deleted.
           The clause is reinitialized afterwards.
  MEMORY:  The memory of the literals with the 'Indice' and
           memory of its atom is freed.
***************************************************************/
{
  LITERAL *NewLits;
  intptr_t     i; 
  int j, nc, na, ns, lc, la, olength, nlength;

#ifdef CHECK
  LIST Scan;
  if (!list_IsSetOfPointers(Indices)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_DeleteLiterals:");
    misc_ErrorReport(" list contains duplicate indices.");
    misc_FinishErrorReport();
  }
  /* check the literal indices */
  for (Scan = Indices; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    i = (intptr_t) list_Car(Scan);
    if (i < 0 || i > clause_LastLitIndex(Clause)) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In clause_DeleteLiterals:");
      misc_ErrorReport(" literal index %d is out ", i);
      misc_ErrorReport(" of bounds");
      misc_FinishErrorReport();
    }
  }
#endif

  nc = 0;
  na = 0;
  ns = 0;
  lc = clause_LastConstraintLitIndex(Clause);
  la = clause_LastAntecedentLitIndex(Clause);

  olength = clause_Length(Clause);
  nlength = olength - list_Length(Indices);

  if (nlength != 0)
    NewLits = (LITERAL*) memory_Malloc(sizeof(LITERAL) * nlength);
  else
    NewLits = (LITERAL*) NULL;

  for (i=clause_FirstLitIndex(), j=clause_FirstLitIndex(); i < olength; i++)
 
    if (list_PointerMember(Indices, (POINTER) i))
      clause_LiteralDelete(clause_GetLiteral(Clause,i));
    else {

      NewLits[j++] = clause_GetLiteral(Clause,i);

      if (i <= lc)
	nc++;
      else if (i <= la)
	na++;
      else
	ns++;
    }
  clause_FreeLitArray(Clause);
  Clause->literals = NewLits;

  clause_SetNumOfConsLits(Clause, nc);
  clause_SetNumOfAnteLits(Clause, na);
  clause_SetNumOfSuccLits(Clause, ns);

  clause_ReInit(Clause, Flags, Precedence);
}


/**************************************************************/
/* Clause Comparisons                                         */
/**************************************************************/

BOOL clause_IsHornClause(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: The boolean value TRUE if 'Clause' is a hornclause
           FALSE else.
  ***************************************************************/
{
#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_IsHornClause:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif
  return (clause_NumOfSuccLits(Clause) <= 1);
}


BOOL clause_HasTermSortConstraintLits(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause,
  RETURNS: TRUE iff there is at least one sort constraint atom having
           a term as its argument
***************************************************************/
{
  int i,n;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_HasTermSortConstraintLits:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif
  
  n = clause_LastConstraintLitIndex(Clause);

  for (i = clause_FirstConstraintLitIndex(Clause); i <= n; i++)
    if (!term_AllArgsAreVar(clause_GetLiteralAtom(Clause,i)))
      return TRUE;

  return FALSE;
}

BOOL clause_HasOnlySpecDomArgs(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: TRUE iff all non sort constraint literals are of the 
           form x=a, x=y, a=b.
***************************************************************/
{
  int i,n;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_HasOnlySpecDomArgs:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  n = clause_Length(Clause);

  for (i = clause_FirstAntecedentLitIndex(Clause); i < n; i++)
    if (!fol_IsEquality(clause_GetLiteralAtom(Clause,i)) ||
	!(term_IsVariable(term_FirstArgument(clause_GetLiteralAtom(Clause,i))) ||
	  term_IsConstant(term_FirstArgument(clause_GetLiteralAtom(Clause,i)))) ||
	!(term_IsVariable(term_SecondArgument(clause_GetLiteralAtom(Clause,i))) ||
	  term_IsConstant(term_SecondArgument(clause_GetLiteralAtom(Clause,i)))))
      return FALSE;

  return TRUE;
}

BOOL clause_HasSolvedConstraint(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: The boolean value TRUE if 'Clause' has a solved
           constraint, i.e. only variables as sort arguments,
           or only equality literals with variable and constant
           arguments, e.g., x=a, x=y
	   FALSE else.
***************************************************************/
{
  int  i,c;
  LIST CVars, LitVars;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_HasSolvedConstraint:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  CVars = list_Nil();
  c     = clause_NumOfConsLits(Clause);

  if (c == 0)
    return TRUE;

  if (clause_HasTermSortConstraintLits(Clause) || clause_HasOnlySpecDomArgs(Clause))
    return FALSE;

  for (i = 0; i < c; i++)
    CVars = list_NPointerUnion(term_VariableSymbols(clause_GetLiteralAtom(Clause, i)), CVars);

  if (i == c) {
    c       = clause_Length(Clause);
    LitVars = list_Nil();

    for ( ; i < c; i++)
      LitVars = list_NPointerUnion(LitVars, term_VariableSymbols(clause_GetLiteralAtom(Clause, i)));
    
    if (list_Empty(CVars = list_NPointerDifference(CVars, LitVars))) {
      list_Delete(LitVars);
      return TRUE;
    }
    list_Delete(LitVars);
  }

  list_Delete(CVars);

  return FALSE;
}


BOOL clause_HasSelectedLiteral(CLAUSE Clause, FLAGSTORE Flags,
			       PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, a flag store and a precedence.
  RETURNS: The boolean value TRUE iff <Clause> has a selected literal
***************************************************************/
{
  int  i,negs;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_HasSelectedLiteral:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  negs = clause_LastAntecedentLitIndex(Clause);

  for (i=clause_FirstAntecedentLitIndex(Clause); i <= negs; i++)
    if (clause_LiteralGetFlag(clause_GetLiteral(Clause,i), LITSELECT))
      return TRUE;

  return FALSE;
}


BOOL clause_IsDeclarationClause(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: The boolean value TRUE, if 'Clause' has only variables
           as arguments in constraint literals.
***************************************************************/
{
  int     i,length;
  LITERAL Lit;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_IsDeclarationClause:");
    misc_ErrorReport("\n Illegal input.");
    misc_FinishErrorReport();
  }
#endif
  
  if (!clause_HasSolvedConstraint(Clause))
    return FALSE;

  length = clause_Length(Clause);

  for (i=clause_FirstSuccedentLitIndex(Clause);i<length;i++) {
    Lit = clause_GetLiteral(Clause,i);
    if (clause_LiteralIsMaximal(Lit) &&
	symbol_IsBaseSort(term_TopSymbol(clause_LiteralSignedAtom(Lit))))
      return TRUE;
  }

  return FALSE;
}


BOOL clause_IsSortTheoryClause(CLAUSE Clause, FLAGSTORE Flags,
			       PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, a flag store and a precedence.
  RETURNS: The boolean value TRUE, if 'Clause' has only variables
           as arguments in constraint literals, no antecedent literals
	   and exactly one monadic succedent literal.
***************************************************************/
{
  LITERAL Lit;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_IsSortTheoryClause:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif
  
  if (clause_NumOfAnteLits(Clause) > 0 ||
      clause_NumOfSuccLits(Clause) > 1 ||
      !clause_HasSolvedConstraint(Clause))
    return FALSE;

  Lit = clause_GetLiteral(Clause,clause_FirstSuccedentLitIndex(Clause));
  if (symbol_IsBaseSort(term_TopSymbol(clause_LiteralSignedAtom(Lit))))
    return TRUE;

  return FALSE;
}

BOOL clause_IsPotentialSortTheoryClause(CLAUSE Clause, FLAGSTORE Flags,
					PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, a flag store and a precedence.
  RETURNS: The boolean value TRUE, if 'Clause' has monadic literals
           only variables as arguments in antecedent/constraint literals,
	   no other antecedent literals and exactly one monadic succedent
	   literal.
***************************************************************/
{
  LITERAL Lit;
  int     i;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_IsPotentialSortTheoryClause:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif
  
  if (clause_NumOfSuccLits(Clause) != 1)
    return FALSE;

  for (i=clause_FirstLitIndex();i<clause_FirstSuccedentLitIndex(Clause);i++) {
    Lit = clause_GetLiteral(Clause,i);
    if (!symbol_IsBaseSort(term_TopSymbol(clause_LiteralAtom(Lit))) ||
	!term_IsVariable(term_FirstArgument(clause_LiteralAtom(Lit))))
      return FALSE;
  }

  Lit = clause_GetLiteral(Clause,clause_FirstSuccedentLitIndex(Clause));
  if (symbol_IsBaseSort(term_TopSymbol(clause_LiteralSignedAtom(Lit))))
    return TRUE;

  return FALSE;
}


BOOL clause_HasOnlyVarsInConstraint(CLAUSE Clause, FLAGSTORE Flags,
				    PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, a flag store and a precedence.
  RETURNS: The boolean value TRUE, if 'Clause' has only variables
           as arguments in constraint literals.
***************************************************************/
{
  int  i,nc;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_HasOnlyVarsInConstraint:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif

  nc = clause_NumOfConsLits(Clause);

  for (i = 0; i < nc && term_AllArgsAreVar(clause_GetLiteralAtom(Clause,i)); i++)
    /* empty */;

  return (i == nc);
}


BOOL clause_HasSortInSuccedent(CLAUSE Clause, FLAGSTORE Flags,
			       PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, a flag store and a precedence.
  RETURNS: The boolean value TRUE, if 'Clause' has a maximal succedent
           sort literal; FALSE, else.
***************************************************************/
{
  LITERAL Lit;
  int     i,l;
  BOOL    result = FALSE;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_HasSortInSuccedent:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif
 
  l  = clause_Length(Clause);

  for (i = clause_FirstSuccedentLitIndex(Clause); i < l && !result ; i++) {
    Lit = clause_GetLiteral(Clause, i);
    result = (symbol_Arity(term_TopSymbol(clause_LiteralAtom(Lit))) == 1);
  }
  return result;
}


BOOL clause_LitsHaveCommonVar(LITERAL Lit1, LITERAL Lit2)
/**************************************************************
  INPUT:   Two literals.
  RETURNS: The boolean value TRUE, if 'Lit1' and 'Lit2' have
           common variables, FALSE, else.
***************************************************************/
{
  LIST Vars1, Vars2;
  BOOL Result;

  Vars1  = term_VariableSymbols(clause_LiteralAtom(Lit1));
  Vars2  = term_VariableSymbols(clause_LiteralAtom(Lit2));
  Result = list_HasIntersection(Vars1, Vars2);
  list_Delete(Vars1);
  list_Delete(Vars2);

  return Result;
}

BOOL clause_VarsOfClauseAreInTerm(CLAUSE Clause, TERM Term)
/**********************************************************
 * INPUT:  A clause and a term
 * OUTPUT: TRUE, if vars(<Clause>) is a subset of vars(<Term>)
 *         else FALSE
 * EFFECT: modifies ord_VARCOUNT        
 **********************************************************/
{
   SYMBOL MaxVarClause, MaxVarTerm;
   LITERAL Lit;
   TERM Atom;
   int i, n;

   MaxVarClause = clause_MaxVar(Clause);
   MaxVarTerm = term_MaxVar(Term);

   if(MaxVarClause > MaxVarTerm)
           return FALSE;

   n = clause_Length(Clause);

   for (i = 0; i <= MaxVarTerm; i++) {
    ord_VARCOUNT[i][0] = 0;
    ord_VARCOUNT[i][1] = 0;
  }
 
  for(i = clause_FirstLitIndex(); i < n; i++) {
      Lit  = clause_GetLiteral(Clause, i);
      Atom = clause_LiteralAtom(Lit);
      
      ord_CompareCountVars(Atom, 0);
  }

  ord_CompareCountVars(Term, 1);

  for (i = 0; i <= MaxVarTerm; i++) {
   if(ord_VARCOUNT[i][1] == 0 && ord_VARCOUNT[i][0] != 0)
           return FALSE;
  }

  return TRUE;
}


BOOL clause_LiteralIsStrictMaximalLiteralSkolem(CLAUSE Clause, int i, 
                                                FLAGSTORE Flags, PRECEDENCE Precedence)
/**********************************************************
 * INPUT:  A clause and a literal
 * OUTPUT: TRUE, if Literal <i> of <Clause> is strictly maximal in <Clause>. 
 *         Consider variables to be (skolem) constants. 
 * EFFECT: modifies ord_VARCOUNT 
 * NOTE:   We only need to compare STRICTLY MAXIMAL Literals
 *         of <Clause> with <Lit>
 **********************************************************/
{
    int ri, last;
    
    last = clause_LastLitIndex(Clause);
    for (ri = clause_FirstLitIndex(); ri <= last; ri++) {
        if(ri != i &&
           clause_LiteralGetFlag(clause_GetLiteral(Clause, ri),STRICTMAXIMAL) &&
           ord_LiteralCompareAux(clause_GetLiteralTerm(Clause, i), 
                                 clause_LiteralGetOrderStatus(clause_GetLiteral(Clause,i)),
                                 clause_GetLiteralTerm(Clause, ri),                                 
                                 clause_LiteralGetOrderStatus(clause_GetLiteral(Clause,ri)),
			         TRUE, TRUE, Flags, Precedence) != ord_GREATER_THAN )
                return FALSE;
    }
    return TRUE;
}

BOOL    clause_IsTransitivityAxiomExt(CLAUSE Clause, SYMBOL* symb, BOOL* twisted)
/**********************************************************
 * INPUT:  A clause, boolean flag, pointer to symbol
           and a pointer to a flag.
 * OUTPUT: TRUE, if the clause has the form of a transitivity axiom
 *         (a) x S y, y S z -> x S z 
 *            or
 *         (b) y S z , x S y -> x S z
 *         FALSE otherwise
 *         
 *         If the function returns TRUE:
 *           The *symb will be assing the symbol S.
 *           The *twisted will indicate that case (b) and not (a) occured
 **********************************************************/
{
  TERM t1, t2, t3;
  SYMBOL s1, s2, s3;
  SYMBOL s1a,s1b,s2a,s2b,s3a,s3b;
  int i;

  /* check the clause size and "shape" */
  
  if (clause_NumOfConsLits(Clause) != 0 ||
      clause_NumOfAnteLits(Clause) != 2 ||
      clause_NumOfSuccLits(Clause) != 1 )
    return FALSE;

  /* check the literals' top symbols and collect the terms */

  i  = clause_FirstAntecedentLitIndex(Clause);
  t1 = clause_LiteralAtom(clause_GetLiteral(Clause,i));
  s1 = term_TopSymbol(t1);

  if (symbol_Arity(s1) != 2) 
    return FALSE;
  
  i++;
  t2 = clause_LiteralAtom(clause_GetLiteral(Clause,i));
  s2 = term_TopSymbol(t2);
    
  if (!symbol_Equal(s1,s2))
    return FALSE;
  
  i = clause_FirstSuccedentLitIndex(Clause);
  t3 = clause_LiteralAtom(clause_GetLiteral(Clause,i));
  s3 = term_TopSymbol(t3);
  
  if (!symbol_Equal(s1,s3))
      return FALSE;
    
  /* check that the arguments are variables and collect them */
  s1a = term_TopSymbol(term_FirstArgument(t1));
  if (!symbol_IsVariable(s1a))
    return FALSE;
  s1b = term_TopSymbol(term_SecondArgument(t1));
  if (!symbol_IsVariable(s1b))
    return FALSE;

  s2a = term_TopSymbol(term_FirstArgument(t2));
  if (!symbol_IsVariable(s2a))
    return FALSE;
  s2b = term_TopSymbol(term_SecondArgument(t2));
  if (!symbol_IsVariable(s2b))
    return FALSE;
  
  s3a = term_TopSymbol(term_FirstArgument(t3));
  if (!symbol_IsVariable(s3a))
    return FALSE;
  s3b = term_TopSymbol(term_SecondArgument(t3));
  if (!symbol_IsVariable(s3b))
    return FALSE;

  /* check the variable correspondence - the two negative literals may be in any order!*/

  if (symbol_Equal(s1a,s3a) && symbol_Equal(s1b,s2a) && symbol_Equal(s2b,s3b)) {
    *twisted = FALSE;
    *symb = s1;
    return TRUE;
  }

  if (symbol_Equal(s2a,s3a) && symbol_Equal(s2b,s1a) && symbol_Equal(s1b,s3b)) {
    *twisted = TRUE;
    *symb = s1;
    return TRUE;
  }

  return FALSE; 
}

BOOL clause_IsIrreflexivityAxiomExt(CLAUSE Clause, SYMBOL* Symbol)
/**********************************************************
 * INPUT:  A clause, pointers to symbol.           
 * OUTPUT: True if Clause represents a irreflexivity axiom
 *        for symbol Symbol
 **********************************************************/
{
  TERM atom;
  SYMBOL top,v1,v2;

  /* one negative literal*/
  if (clause_NumOfConsLits(Clause) != 0 ||
      clause_NumOfAnteLits(Clause) != 1 ||
      clause_NumOfSuccLits(Clause) != 0 )
    return FALSE;

  atom = clause_LiteralAtom(clause_GetLiteral(Clause,clause_FirstAntecedentLitIndex(Clause)));
  top = term_TopSymbol(atom);

  /* binary predicate at the top*/
  if (symbol_Arity(top) != 2) 
    return FALSE;
    
  /* both arguments are variables */
  v1 = term_TopSymbol(term_FirstArgument(atom));
  if (!symbol_IsVariable(v1))
    return FALSE;
  
  v2 = term_TopSymbol(term_SecondArgument(atom));
  if (!symbol_IsVariable(v2))
    return FALSE;
  
  /* actually they should be the same */
  if (!symbol_Equal(v1,v2))
    return FALSE;

  *Symbol = top;
  
  return TRUE;
}

BOOL clause_IsTotalityAxiomExt(CLAUSE Clause, SYMBOL* Symbol) 
/**********************************************************
 * INPUT:  A clause, pointers to symbol.           
 * OUTPUT: True if Clause represents a totality axiom
 *         for symbol Symbol
 **********************************************************/
{
  TERM atom1, atom2, atom3, tatom;  
  SYMBOL top1,top2,top3, ttop;
  SYMBOL v1,v2,t1,t2;
  int i;
  
  /* one positive literals*/
  if (clause_NumOfConsLits(Clause) != 0 ||
      clause_NumOfAnteLits(Clause) != 0 ||
      clause_NumOfSuccLits(Clause) != 3 )
  return FALSE;
  
  /* each has ... */
  i = clause_FirstSuccedentLitIndex(Clause);
  atom1 = clause_LiteralAtom(clause_GetLiteral(Clause,i));
  top1 = term_TopSymbol(atom1);
  
  /* .. binary top .. */
  if (symbol_Arity(top1) != 2) 
    return FALSE;
  
  /* .. variable areguments .. */
  v1 = term_TopSymbol(term_FirstArgument(atom1));
  if (!symbol_IsVariable(v1))
    return FALSE;
    
  v2 = term_TopSymbol(term_SecondArgument(atom1));
  if (!symbol_IsVariable(v2))
    return FALSE;
    
  /* .. the two variables are different .. */
  if (symbol_Equal(v1,v2))
    return FALSE;
     
  i++;  
  atom2 = clause_LiteralAtom(clause_GetLiteral(Clause,i));
  top2 = term_TopSymbol(atom2);
  
  /* .. binary top .. */
  if (symbol_Arity(top2) != 2) 
    return FALSE;
  
  /* .. also variables and the same as last time .. */
  t1 = term_TopSymbol(term_FirstArgument(atom2));
  if (!symbol_Equal(t1,v1) && !symbol_Equal(t1,v2))
    return FALSE;
    
  t2 = term_TopSymbol(term_SecondArgument(atom2));
  if (!symbol_Equal(t2,v1) && !symbol_Equal(t2,v2))
    return FALSE;
    
  /* .. the two variables are different .. */
  if (symbol_Equal(t1,t2))
    return FALSE;
        
  i++;  
  atom3 = clause_LiteralAtom(clause_GetLiteral(Clause,i));
  top3 = term_TopSymbol(atom3);
  
  /* .. binary top .. */  
  if (symbol_Arity(top3) != 2) 
    return FALSE;
    
  /* .. also variables and the same as last time .. */
  t1 = term_TopSymbol(term_FirstArgument(atom3));
  if (!symbol_Equal(t1,v1) && !symbol_Equal(t1,v2))
    return FALSE;
    
  t2 = term_TopSymbol(term_SecondArgument(atom3));
  if (!symbol_Equal(t2,v1) && !symbol_Equal(t2,v2))
    return FALSE;

  /* .. the two variables are different .. */
  if (symbol_Equal(t1,t2))
    return FALSE;
    
  /* one of them is equality predicate - let's shift that one to front */
  if (symbol_Equal(top1,fol_Equality())) {
    /* no op*/
  } else if (symbol_Equal(top2,fol_Equality())) {
    ttop = top1; top1 = top2; top2 = ttop;
    tatom = atom1; atom1 = atom2; atom2 = tatom;
  } else if (symbol_Equal(top3,fol_Equality())) {
    ttop = top1; top1 = top3; top3 = ttop;
    tatom = atom1; atom1 = atom3; atom3 = tatom;
  } else {
    return FALSE;  /* no equality predicate */
  }
   
  /* the other two share the same non-equality top */
  if (!symbol_Equal(top2, top3) || symbol_Equal(top2,fol_Equality()))
    return FALSE;
   
  /*the other two are not the same*/
  if (symbol_Equal(term_TopSymbol(term_FirstArgument(atom2)),
                   term_TopSymbol(term_FirstArgument(atom3))))
    return FALSE;
  
  *Symbol = top2;
  
  return TRUE;
}

/**************************************************************/
/* Clause Input and Output Functions                          */
/**************************************************************/

void clause_Print(CLAUSE Clause)
/**************************************************************
  INPUT:   A Clause.
  RETURNS: Nothing.
  SUMMARY: The clause is printed to stdout.
***************************************************************/
{
  RULE Origin;
  LITERAL Lit;
  int i,c,a,s;
  ord_RESULT ord;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_Print:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif

  if (!clause_Exists(Clause))
    fputs("(CLAUSE)NULL", stdout);
  else {
    printf("%zd",clause_Number(Clause));
    
    Origin = clause_Origin(Clause);
    printf("[%zd:", clause_SplitLevel(Clause));

#ifdef CHECK
    if (Clause->splitfield_length <= 1)
      fputs("0.", stdout);
    else
      for (i=Clause->splitfield_length-1; i > 0; i--)
	printf("%lu.", Clause->splitfield[i]);
    if (Clause->splitfield_length == 0)
      putchar('1');
    else
      printf("%lu", (Clause->splitfield[0] | 1));
    printf(":%c%c:%c:%zu:%zu:%.1f:", clause_GetFlag(Clause, CONCLAUSE) ? 'C' : 'A',
	   clause_GetFlag(Clause, WORKEDOFF) ? 'W' : 'U',
	   clause_GetFlag(Clause, NOPARAINTO) ? 'N' : 'P',
	   clause_Weight(Clause), clause_Depth(Clause), clause_SplitPotential(Clause));
#endif
    
    clause_PrintOrigin(Clause);
    
    if (Origin == INPUTCLAUSE) {
      ;
    } else  {      
      putchar(':');
      clause_PrintParentClauses(Clause);
    }
    putchar(']');

    c = clause_NumOfConsLits(Clause);
    a = clause_NumOfAnteLits(Clause);
    s = clause_NumOfSuccLits(Clause);

    for (i = 0; i < c; i++) {
      putchar(' ');
      Lit = clause_GetLiteral(Clause, i);
      clause_LiteralPrintUnsigned(Lit);
    }
    fputs(" || ", stdout);

    a += c;
    for ( ; i < a; i++) {

      Lit = clause_GetLiteral(Clause, i);
      clause_LiteralPrintUnsigned(Lit);
      if (clause_LiteralIsMaximal(Lit)) {
	putchar('*');
	if (clause_LiteralIsOrientedEquality(Lit))
	  putchar('*');
  if (symbol_HasProperty(term_TopSymbol(clause_LiteralAtom(Lit)), TRANSITIVE)
      && ((ord = clause_LiteralGetOrderStatus(Lit)) != ord_UNCOMPARABLE)) {
    switch (ord) {
      case ord_SMALLER_THAN:
        putchar('r');
        break;
      case ord_GREATER_THAN:
        putchar('l');
        break;
      default:
        /*putchar('e');*/
        break;
    }
  }      
      }
      if (clause_LiteralGetFlag(Lit,LITSELECT))
	putchar('+');
      if (i+1 < a)
	putchar(' ');
    }
    fputs(" -> ",stdout);

    s += a;
    for ( ; i < s; i++) {

      Lit = clause_GetLiteral(Clause, i);
      clause_LiteralPrintUnsigned(Lit);
      if (clause_LiteralIsMaximal(Lit)) {
	putchar('*');
	if (clause_LiteralIsOrientedEquality(Lit))
	  putchar('*');
  if (symbol_HasProperty(term_TopSymbol(clause_LiteralAtom(Lit)), TRANSITIVE)
      && ((ord = clause_LiteralGetOrderStatus(Lit)) != ord_UNCOMPARABLE)) {
    switch (ord) {
      case ord_SMALLER_THAN:
        putchar('r');
        break;
      case ord_GREATER_THAN:
        putchar('l');
        break;
      default:
        /*putchar('e');*/
        break;
    }
  }    
      }
#ifdef CHECK
      if (clause_LiteralGetFlag(Lit,LITSELECT)) {
	misc_StartErrorReport();
	misc_ErrorReport("\n In clause_Print: Clause has selected positive literal.\n");
	misc_FinishErrorReport();
      }
#endif
      if (i+1 < s)
	putchar(' ');
    }
    putchar('.');
  }
}

void clause_PrintSpecial(CLAUSE Clause)
/**************************************************************
  INPUT:   A Clause, where parents are the pointers to clauses
           as used in the tableau module
  RETURNS: Nothing.
  SUMMARY: The clause is printed to stdout. For DEBUGGING!
***************************************************************/
{
  RULE Origin;
  LITERAL Lit;
  int i,j,c,a,s;
  ord_RESULT ord;

#ifdef CHECK
  if (!clause_IsUnorderedClause(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_PrintSpecial:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif

  if (!clause_Exists(Clause))
    fputs("(CLAUSE)NULL", stdout);
  else {
    printf("%zd",clause_Number(Clause));
    
    Origin = clause_Origin(Clause);
    printf("[%zd:", clause_SplitLevel(Clause));

    fputs("0", stdout);
    for (i = Clause->splitfield_length-1; i >= 0; i--)
      if (Clause->splitfield[i] != 0) {
	for (j = 0; j<=sizeof(SPLITFIELDENTRY)*CHAR_BIT-1; j++)
	  if (Clause->splitfield[i] & ((SPLITFIELDENTRY)1 << j))
	    printf(",%zd", i*sizeof(SPLITFIELDENTRY)*CHAR_BIT+j);
      }
	

    printf(":%c%c:%c:%zd:%zd:%.1f:", clause_GetFlag(Clause, CONCLAUSE) ? 'C' : 'A',
	   clause_GetFlag(Clause, WORKEDOFF) ? 'W' : 'U',
	   clause_GetFlag(Clause, NOPARAINTO) ? 'N' : 'P',
	   clause_Weight(Clause), clause_Depth(Clause), clause_SplitPotential(Clause));
    
    clause_PrintOrigin(Clause);
    
    if (Origin == INPUTCLAUSE) {
      ;
    } else  {      
      putchar(':');
      clause_PrintParentClausesSpecial(Clause);
    }
    putchar(']');

    c = clause_NumOfConsLits(Clause);
    a = clause_NumOfAnteLits(Clause);
    s = clause_NumOfSuccLits(Clause);

    for (i = 0; i < c; i++) {
      putchar(' ');
      Lit = clause_GetLiteral(Clause, i);
      clause_LiteralPrintUnsigned(Lit);
    }
    fputs(" || ", stdout);

    a += c;
    for ( ; i < a; i++) {

      Lit = clause_GetLiteral(Clause, i);
      clause_LiteralPrintUnsigned(Lit);
      if (clause_LiteralIsMaximal(Lit)) {
	putchar('*');
	if (clause_LiteralIsOrientedEquality(Lit))
	  putchar('*');
  if (symbol_HasProperty(term_TopSymbol(clause_LiteralAtom(Lit)), TRANSITIVE)
      && ((ord = clause_LiteralGetOrderStatus(Lit)) != ord_UNCOMPARABLE)) {
    switch (ord) {
      case ord_SMALLER_THAN:
        putchar('r');
        break;
      case ord_GREATER_THAN:
        putchar('l');
        break;
      default:
        /*putchar('e');*/
        break;
    }
  }      
      }
      if (clause_LiteralGetFlag(Lit,LITSELECT))
	putchar('+');
      if (i+1 < a)
	putchar(' ');
    }
    fputs(" -> ",stdout);

    s += a;
    for ( ; i < s; i++) {

      Lit = clause_GetLiteral(Clause, i);
      clause_LiteralPrintUnsigned(Lit);
      if (clause_LiteralIsMaximal(Lit)) {
	putchar('*');
	if (clause_LiteralIsOrientedEquality(Lit))
	  putchar('*');
  if (symbol_HasProperty(term_TopSymbol(clause_LiteralAtom(Lit)), TRANSITIVE)
      && ((ord = clause_LiteralGetOrderStatus(Lit)) != ord_UNCOMPARABLE)) {
    switch (ord) {
      case ord_SMALLER_THAN:
        putchar('r');
        break;
      case ord_GREATER_THAN:
        putchar('l');
        break;
      default:
        /*putchar('e');*/
        break;
    }
  }    
      }
#ifdef CHECK
      if (clause_LiteralGetFlag(Lit,LITSELECT)) {
	misc_StartErrorReport();
	misc_ErrorReport("\n In clause_Print: Clause has selected positive literal.\n");
	misc_FinishErrorReport();
      }
#endif
      if (i+1 < s)
	putchar(' ');
    }
    putchar('.');
  }
}

void clause_PrintSplitfield(CLAUSE Clause, NAT MaxLevel)
{
  int i,j;
  uintptr_t field;
  short first = 1;

  for(i=1; i<=MaxLevel; i++) {
    j = clause_ComputeSplitFieldAddress(i, &field);
    if((Clause->splitfield[field] & ((SPLITFIELDENTRY)1 << j)) != 0) {
      if(first==1) {
        printf("%d", i);
        first=0;
      } else {
        printf(".%d", i);
      }
    }
  }
}

void clause_PrintSplitfieldOnly(SPLITFIELD SF, int SF_length)
{
  int i,j;
  NAT k;
  short first = 1;

  for(i=0; i<SF_length; i++) {
    for(j=0; j<sizeof(SPLITFIELDENTRY) * CHAR_BIT; j++) {
      if((SF[i] & ((SPLITFIELDENTRY)1 << j)) != 0) {
        k = (i * sizeof(SPLITFIELDENTRY) * CHAR_BIT)+j;
        if(first==1) {
          printf("%zu", k);
          first=0;
        } else {
          printf(".%zu", k);
        }
      }
    }
  }
}

void clause_PrintMaxLitsOnly(CLAUSE Clause, FLAGSTORE Flags,
			     PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, a flag store and a precedence.
  RETURNS: Nothing.
  SUMMARY:
***************************************************************/
{
  int i,c,a,s;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_PrinMaxLitsOnly:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif

  c = clause_NumOfConsLits(Clause);
  a = clause_NumOfAnteLits(Clause);
  s = clause_NumOfSuccLits(Clause);

  for (i = 0; i < c; i++) {
    if (clause_LiteralIsMaximal(clause_GetLiteral(Clause, i)))
      clause_LiteralPrint(clause_GetLiteral(Clause, i));
    if (clause_LiteralGetFlag(clause_GetLiteral(Clause, i),STRICTMAXIMAL)) {
      clause_LiteralPrint(clause_GetLiteral(Clause, i));
      fputs("(strictly)", stdout);
    }
  }
  fputs(" || ", stdout);
  
  a += c;
  for ( ; i < a; i++) {
    if (clause_LiteralIsMaximal(clause_GetLiteral(Clause, i)))
      clause_LiteralPrint(clause_GetLiteral(Clause, i));
    if (clause_LiteralGetFlag(clause_GetLiteral(Clause, i),STRICTMAXIMAL)) {
      clause_LiteralPrint(clause_GetLiteral(Clause, i));
      fputs("(strictly)", stdout);
    }
  }
  fputs(" -> ", stdout);

  s += a;
  for ( ; i < s; i++) {
    if (clause_LiteralIsMaximal(clause_GetLiteral(Clause, i)))
      clause_LiteralPrint(clause_GetLiteral(Clause, i));
    if (clause_LiteralGetFlag(clause_GetLiteral(Clause, i),STRICTMAXIMAL)) {
      clause_LiteralPrint(clause_GetLiteral(Clause, i));
      fputs("(strictly)", stdout);
    }
  }
  puts(".");  /* with newline */
}


void clause_FPrint(FILE* File, CLAUSE Clause)
/**************************************************************
  INPUT:   A file and a clause.
  RETURNS: Nothing.
  SUMMARY: Prints any clause to the file 'File'.
  CAUTION: Uses the term_Output functions.
***************************************************************/
{
  int i, c, a, s;

  c = clause_NumOfConsLits(Clause);
  a = clause_NumOfAnteLits(Clause);
  s = clause_NumOfSuccLits(Clause);

  for (i = 0; i < c; i++)
    term_FPrint(File, clause_GetLiteralAtom(Clause, i));

  fputs(" || ", stdout);

  a += c;
  for ( ; i < a; i++)
    term_FPrint(File, clause_GetLiteralAtom(Clause, i));

  fputs(" -> ", stdout);

  s += a;
  for ( ; i < s; i++)
    term_FPrint(File, clause_GetLiteralAtom(Clause, i));
  
  putc('.', File);
}


void clause_ListPrint(LIST ClauseList)
/**************************************************************
  INPUT:   A list of clauses.
  RETURNS: Nothing.
  SUMMARY: Prints the clauses to stdout.
  CAUTION: Uses the clause_Print function.
***************************************************************/
{
  while (!(list_Empty(ClauseList))) {
    clause_Print(list_First(ClauseList));
    ClauseList = list_Cdr(ClauseList);
    if (!list_Empty(ClauseList))
      putchar('\n');
  }
}


void clause_PrintParentClauses(CLAUSE Clause)
/**************************************************************
  INPUT:   A Clause.
  RETURNS: Nothing.
  SUMMARY: Prints the clauses parentclauses and -literals to stdout.
***************************************************************/
{
  LIST Scan1,Scan2;
  
  if (!list_Empty(clause_ParentClauses(Clause))) {
    
    Scan1 = clause_ParentClauses(Clause);
    Scan2 = clause_ParentLiterals(Clause);
    printf("%zd.%zd", (intptr_t)list_Car(Scan1), (intptr_t)list_Car(Scan2));
    
    for (Scan1 = list_Cdr(Scan1), Scan2 = list_Cdr(Scan2);
	 !list_Empty(Scan1);
	 Scan1 = list_Cdr(Scan1), Scan2 = list_Cdr(Scan2))
      printf(",%zd.%zd", (intptr_t)list_Car(Scan1), (intptr_t)list_Car(Scan2));
  }
}

void clause_PrintParentClausesSpecial(CLAUSE Clause)
/**************************************************************
  INPUT:   A Clause where parents are clause pointers.
  RETURNS: Nothing.
  SUMMARY: Prints the clauses parentclauses and -literals to stdout.
***************************************************************/
{
  LIST Scan1,Scan2;
  
  if (!list_Empty(clause_ParentClauses(Clause))) {
    
    Scan1 = clause_ParentClauses(Clause);
    Scan2 = clause_ParentLiterals(Clause);
    printf("%zd.%.zd", clause_Number((CLAUSE)list_Car(Scan1)), (intptr_t)list_Car(Scan2));
    
    for (Scan1 = list_Cdr(Scan1), Scan2 = list_Cdr(Scan2);
	 !list_Empty(Scan1);
	 Scan1 = list_Cdr(Scan1), Scan2 = list_Cdr(Scan2))
      printf(",%zd.%zd", clause_Number((CLAUSE)list_Car(Scan1)), (intptr_t)list_Car(Scan2));
  }
}


RULE clause_GetOriginFromString(const char* RuleName)
/**************************************************************
  INPUT:   A string containing the abbreviated name of a rule.
  RETURNS: The RULE corresponding to the <RuleName>.
***************************************************************/
{
  if      (string_Equal(RuleName, "App")) return CLAUSE_DELETION;
  else if (string_Equal(RuleName, "EmS")) return EMPTY_SORT;
  else if (string_Equal(RuleName, "SoR")) return SORT_RESOLUTION;
  else if (string_Equal(RuleName, "EqR")) return EQUALITY_RESOLUTION;
  else if (string_Equal(RuleName, "EqF")) return EQUALITY_FACTORING;
  else if (string_Equal(RuleName, "MPm")) return MERGING_PARAMODULATION;
  else if (string_Equal(RuleName, "SpR")) return SUPERPOSITION_RIGHT;
  else if (string_Equal(RuleName, "SPm")) return PARAMODULATION;
  else if (string_Equal(RuleName, "OPm")) return ORDERED_PARAMODULATION;
  else if (string_Equal(RuleName, "SpL")) return SUPERPOSITION_LEFT;
  else if (string_Equal(RuleName, "Res")) return GENERAL_RESOLUTION;
  else if (string_Equal(RuleName, "SHy")) return SIMPLE_HYPER;
  else if (string_Equal(RuleName, "OHy")) return ORDERED_HYPER;
  else if (string_Equal(RuleName, "URR")) return UR_RESOLUTION;
  else if (string_Equal(RuleName, "Fac")) return GENERAL_FACTORING;
  else if (string_Equal(RuleName, "Spt")) return SPLITTING;
  else if (string_Equal(RuleName, "Inp")) return INPUTCLAUSE;
  else if (string_Equal(RuleName, "Rew")) return REWRITING;
  else if (string_Equal(RuleName, "CRw")) return CONTEXTUAL_REWRITING;
  else if (string_Equal(RuleName, "Con")) return CONDENSING;
  else if (string_Equal(RuleName, "AED")) return ASSIGNMENT_EQUATION_DELETION;
  else if (string_Equal(RuleName, "Obv")) return OBVIOUS_REDUCTIONS;
  else if (string_Equal(RuleName, "SSi")) return SORT_SIMPLIFICATION;
  else if (string_Equal(RuleName, "MRR")) return MATCHING_REPLACEMENT_RESOLUTION;
  else if (string_Equal(RuleName, "UnC")) return UNIT_CONFLICT;
  else if (string_Equal(RuleName, "Def")) return DEFAPPLICATION;
  else if (string_Equal(RuleName, "Ter")) return TERMINATOR;
  else if (string_Equal(RuleName, "OCh")) return ORDERED_CHAINING;
  else if (string_Equal(RuleName, "NCh")) return NEGATIVE_CHAINING;
  else if (string_Equal(RuleName, "CRe")) return COMPOSITION_RESOLUTION;
  else {
    misc_StartErrorReport();
    misc_ErrorReport("\nIn clause_GetOriginFromString: Unknown clause origin '%s'.", RuleName);
    misc_FinishErrorReport();
    return CLAUSE_DELETION; /* Just for the compiler, code is not reachable */
  }
}

void clause_FPrintOrigin(FILE* File, CLAUSE Clause)
/**************************************************************
  INPUT:   A Clause.
  RETURNS: Nothing.
  SUMMARY: Prints the clause's origin to the file.
***************************************************************/
{
  RULE Result;

  Result = clause_Origin(Clause);

  switch(Result) {
  case CLAUSE_DELETION:                 fputs("App", File); break;
  case EMPTY_SORT:                      fputs("EmS", File); break;
  case SORT_RESOLUTION:                 fputs("SoR", File); break;
  case EQUALITY_RESOLUTION:             fputs("EqR", File); break;
  case EQUALITY_FACTORING:              fputs("EqF", File); break;
  case MERGING_PARAMODULATION:          fputs("MPm", File); break;
  case SUPERPOSITION_RIGHT:             fputs("SpR", File); break;
  case PARAMODULATION:                  fputs("SPm", File); break;
  case ORDERED_PARAMODULATION:          fputs("OPm", File); break;
  case SUPERPOSITION_LEFT:              fputs("SpL", File); break;
  case GENERAL_RESOLUTION:              fputs("Res", File); break;
  case SIMPLE_HYPER:                    fputs("SHy", File); break;
  case ORDERED_HYPER:                   fputs("OHy", File); break;
  case UR_RESOLUTION:                   fputs("URR", File); break;
  case GENERAL_FACTORING:               fputs("Fac", File); break;
  case SPLITTING:                       fputs("Spt", File); break;
  case INPUTCLAUSE:                     fputs("Inp", File); break;
  case REWRITING:                       fputs("Rew", File); break;
  case CONTEXTUAL_REWRITING:            fputs("CRw", File); break;
  case CONDENSING:                      fputs("Con", File); break;
  case ASSIGNMENT_EQUATION_DELETION:    fputs("AED", File); break;
  case OBVIOUS_REDUCTIONS:              fputs("Obv", File); break;
  case SORT_SIMPLIFICATION:             fputs("SSi", File); break;
  case MATCHING_REPLACEMENT_RESOLUTION: fputs("MRR", File); break;
  case UNIT_CONFLICT:                   fputs("UnC", File); break;
  case DEFAPPLICATION:                  fputs("Def", File); break;
  case TERMINATOR:                      fputs("Ter", File); break;
  case TEMPORARY:                       fputs("Temporary", File); break;
  case ORDERED_CHAINING:                fputs("OCh", File); break;
  case NEGATIVE_CHAINING:               fputs("NCh", File); break;
  case COMPOSITION_RESOLUTION:          fputs("CRe", File); break;
  default:
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_FPrintOrigin: Clause has no origin.");
    misc_FinishErrorReport();
  }
}


void clause_PrintOrigin(CLAUSE Clause)
/**************************************************************
  INPUT:   A Clause.
  RETURNS: Nothing.
  SUMMARY: Prints the clauses origin to stdout.
***************************************************************/
{
  clause_FPrintOrigin(stdout, Clause);
}


void clause_PrintVerbose(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A Clause, a flag store and a precedence.
  RETURNS: Nothing.
  SUMMARY: Prints almost all the information kept in the
           clause structure.
***************************************************************/
{
  int c,a,s;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_PrintVerbose:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif

  c = clause_NumOfConsLits(Clause);
  a = clause_NumOfAnteLits(Clause);
  s = clause_NumOfSuccLits(Clause);

  printf(" c = %d a = %d s = %d", c,a,s);
  printf(" Weight : %zd", clause_Weight(Clause));
  printf(" Depth  : %zd", clause_Depth(Clause));
  printf(" %s %s ",
	 (clause_GetFlag(Clause,WORKEDOFF) ? "WorkedOff" : "Usable"),
	 (clause_GetFlag(Clause,CLAUSESELECT) ? "Selected" : "NotSelected"));

  clause_Print(Clause);
}


CLAUSE clause_GetNumberedCl(int number, LIST ClList)
/**************************************************************
  INPUT:   
  RETURNS: 
  CAUTION: 
***************************************************************/
{
  while (!list_Empty(ClList) &&
	 clause_Number((CLAUSE)list_Car(ClList)) != number)
    ClList = list_Cdr(ClList);
  
  if (list_Empty(ClList))
    return NULL;
  else
    return list_Car(ClList);
}

 BOOL clause_NumberLower(CLAUSE A, CLAUSE B)
{
  return (BOOL) (clause_Number(A) < clause_Number(B));
}

LIST clause_NumberSort(LIST List)
/**************************************************************
  INPUT:   A list of clauses.
  RETURNS: The same list where the elements are sorted wrt their number.
  CAUTION: Destructive.
***************************************************************/
{
  return list_Sort(List, (BOOL (*) (POINTER, POINTER)) clause_NumberLower);
}


LIST clause_NumberDelete(LIST List, int Number)
/**************************************************************
  INPUT:   A list of clauses and an integer.
  RETURNS: The same list where the clauses with <Number> are deleted.
  CAUTION: Destructive.
***************************************************************/
{
  LIST Scan1,Scan2;
  
  for (Scan1 = List; !list_Empty(Scan1); )
    if (clause_Number(list_Car(Scan1))==Number) {
      
      Scan2 = Scan1;
      Scan1 = list_Cdr(Scan1);
      List  = list_PointerDeleteOneElement(List, list_Car(Scan2));
    } else
      Scan1 = list_Cdr(Scan1);

  return List;
}


static NAT clause_NumberOfMaxLits(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: The number of maximal literals in a clause.
***************************************************************/
{
  NAT Result,i,n;

  Result = 0;
  n      = clause_Length(Clause);

  for (i = clause_FirstAntecedentLitIndex(Clause); i < n; i++)
    if (clause_LiteralIsMaximal(clause_GetLiteral(Clause,i)))
      Result++;

  return Result;
}

/* Unused ! */
NAT clause_NumberOfMaxAntecedentLits(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause.
  RETURNS: The number of maximal antecedent literals in a clause.
***************************************************************/
{
  NAT Result,i,n;

  Result = 0;
  n      = clause_LastAntecedentLitIndex(Clause);

  for (i = clause_FirstAntecedentLitIndex(Clause); i <= n; i++)
    if (clause_LiteralIsMaximal(clause_GetLiteral(Clause,i)))
      Result++;

  return Result;
}


void clause_SelectLiteral(CLAUSE Clause, FLAGSTORE Flags)
/**************************************************************
  INPUT:   A clause and a flag store.
  RETURNS: Nothing.
  EFFECT:  If the clause contains more than 2 maximal literals,
           at least one antecedent literal, the literal with
	   the highest weight is selected or if selected predicates
           are set, the first (from left to right) literal with
           selected predicate
           Or the user has determined literals to be selected that
           will then be chosen if they occur in the antecedent.
***************************************************************/
{
  if (clause_HasSolvedConstraint(Clause) &&
      !clause_GetFlag(Clause,CLAUSESELECT) &&
      clause_NumOfAnteLits(Clause) > 0 &&
      flag_GetFlagIntValue(Flags, flag_SELECT) != flag_SELECTOFF &&
      (flag_GetFlagIntValue(Flags, flag_SELECT) != flag_SELECTIFSEVERALMAXIMAL || clause_NumberOfMaxLits(Clause) > 1)) {
    NAT     i,n;
    LITERAL Lit;

    Lit = (LITERAL)NULL;
    n   = clause_LastAntecedentLitIndex(Clause);
    i   = clause_FirstAntecedentLitIndex(Clause);

    if (flag_GetFlagIntValue(Flags, flag_SELECT) == flag_SELECTFROMLIST) {
      for ( ; i <= n; i++)
	if (symbol_HasProperty(term_TopSymbol(clause_LiteralAtom(clause_GetLiteral(Clause,i))), SELECTED)) {
	  Lit = clause_GetLiteral(Clause,i);
	  i   = n + 1;
	}
    }
    else {
    
      Lit = clause_GetLiteral(Clause,i);
      i++;      

      if (symbol_HasProperty(term_TopSymbol(clause_LiteralAtom(Lit)), SELECTED))
	i = n + 1;
    
      for ( ; i <= n; i++)
	if (symbol_HasProperty(term_TopSymbol(clause_LiteralAtom(clause_GetLiteral(Clause,i))), SELECTED)) {
	  Lit = clause_GetLiteral(Clause,i);
	  i   = n + 1;
	}
	else
	  if (clause_LiteralWeight(Lit)
	      < clause_LiteralWeight(clause_GetLiteral(Clause,i)))
	    Lit = clause_GetLiteral(Clause,i);
    }

    if (Lit != (LITERAL)NULL) {    
      clause_LiteralSetFlag(Lit,LITSELECT);
      clause_SetFlag(Clause,CLAUSESELECT);
    }
  }
}


void clause_SetSpecialFlags(CLAUSE Clause, BOOL SortDecreasing, FLAGSTORE Flags,
			    PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a flag indicating whether all equations are
           sort decreasing, a flag store and a precedence.
  RETURNS: void.
  EFFECT:  If the clause is a sort theory clause and its declaration
           top symbol is a set declaration sort, i.e., it occurred in a
	   declaration right from the beginning, the paramodulation/superposition
	   steps into the clause are forbidden by setting the
	   NOPARAINTO clause flag
***************************************************************/
{
  if (SortDecreasing &&
      clause_IsSortTheoryClause(Clause, Flags, Precedence) &&
      symbol_HasProperty(term_TopSymbol(clause_GetLiteralTerm(Clause,clause_FirstSuccedentLitIndex(Clause))),
			 DECLSORT))
    clause_SetFlag(Clause,NOPARAINTO);
}


BOOL clause_ContainsPotPredDef(CLAUSE Clause, FLAGSTORE Flags,
			       PRECEDENCE Precedence, NAT* Index, LIST* Pair)
/**************************************************************
  INPUT:   A clause, a flag store, a precedence and a pointer to an index.
  RETURNS: TRUE iff a succedent literal of the clause is a predicate
           having only variables as arguments, the predicate occurs only
	   once in the clause and no other variables but the predicates'
	   occur.
	   In that case Index is set to the index of the predicate and
	   Pair contains two lists : the literals for which positive
	   occurrences must be found and a list of literals for which negative
	   occurrences must be found for a complete definition.
***************************************************************/
{
  NAT i;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_ContainsPotPredDef:");
    misc_ErrorReport("\n Illegal input. Input not a clause.");
    misc_FinishErrorReport();
  }
#endif

  /* Iterate over all succedent literals */
  for (i=clause_FirstSuccedentLitIndex(Clause); i < clause_Length(Clause); i++) {
    LITERAL lit;
    TERM atom;
    LIST pair;

    lit = clause_GetLiteral(Clause, i);
    atom = clause_LiteralSignedAtom(lit);
    if (symbol_IsPredicate(term_TopSymbol(atom))) {
      LIST l;
      BOOL ok;
      ok = TRUE;
      pair = list_PairCreate(list_Nil(), list_Nil());
      
      /* Make sure all arguments of predicate are variables */
      for (l=term_ArgumentList(atom); !list_Empty(l); l=list_Cdr(l)) {
	if (!term_IsStandardVariable((TERM) list_Car(l))) {
	  ok = FALSE;
	  break;
	}
      }
      if (ok) {
	/* Make sure predicate occurs only once in clause */
	NAT j, count;
	count = 0;
	for (j=0; (j < clause_Length(Clause)) && (count < 2); j++) {
	  TERM t;
	  t = clause_GetLiteralAtom(Clause, j);
	  if (symbol_Equal(term_TopSymbol(t), term_TopSymbol(atom)))
	    count++;
	}
	if (count > 1)
	  ok = FALSE;
      }
      if (ok) {
	/* Build lists of positive and negative literals */
	/* At the same time check if other variables than those among
	   the predicates arguments are found */
	NAT j;
	LIST predvars, vars;
	predvars = fol_FreeVariables(atom);
	
	/* Build list of literals for which positive occurrences are required */
	for (j=0; (j < clause_FirstSuccedentLitIndex(Clause)) && ok; j++) {
	  list_Rplaca(pair, list_Cons(clause_GetLiteralAtom(Clause, j), list_PairFirst(pair)));
	  vars = fol_FreeVariables(clause_GetLiteralTerm(Clause, j));
	  for (l = vars; !list_Empty(l); l = list_Cdr(l)) {
	    if (!term_ListContainsTerm(predvars, list_Car(l))) {
	      ok = FALSE;
	      break;
	    }
	  }
	  list_Delete(vars);
	}
	
	/* Build list of literals for which negative occurrences are required */
	for (j = clause_FirstSuccedentLitIndex(Clause);
	     (j < clause_Length(Clause)) && ok; j++) {
	  if (j != i) {
	    list_Rplacd(pair, list_Cons(clause_GetLiteralAtom(Clause, j), list_PairSecond(pair)));
	    vars = fol_FreeVariables(clause_GetLiteralAtom(Clause, j));
	    for (l=vars; !list_Empty(l); l=list_Cdr(l)) {
	      if (!term_ListContainsTerm(predvars, list_Car(l))) {
		ok = FALSE;
		break;
	      }
	    }
	    list_Delete(vars);
	  }
	}
	list_Delete(predvars);
      }

      if (ok) {
	*Index = i;
	*Pair = pair;
	return TRUE;
      }
      else {
	list_Delete(list_PairFirst(pair));
	list_Delete(list_PairSecond(pair));
	list_PairFree(pair);
      }
    }
  }
  return FALSE;
}

BOOL clause_IsPartOfDefinition(CLAUSE Clause, TERM Predicate, intptr_t *Index,
			       LIST Pair)
/**************************************************************
  INPUT:   A clause, a term, a pointer to an int and a pair of term lists.
  RETURNS: TRUE iff the predicate occurs among the negative literals of
           the clause and the other negative and positive literals are found
	   in the pairs' lists.
	   In that case they are removed from the lists.
	   Index is set to the index of the defined predicate in Clause.
***************************************************************/
{
  NAT predindex, i;
  BOOL ok;

  ok = TRUE;

  /* Check whether Predicate is among antecedent or constraint literals */
  for (predindex=clause_FirstLitIndex();
       predindex < clause_FirstSuccedentLitIndex(Clause);
       predindex++)
    if (term_Equal(Predicate, clause_GetLiteralAtom(Clause, predindex)))
      break;
  if (predindex == clause_FirstSuccedentLitIndex(Clause))
    return FALSE;
  *Index = predindex;

  /* Check if other negative literals are required for definition */
  for (i=clause_FirstLitIndex();
       (i < clause_FirstSuccedentLitIndex(Clause)) && ok; i++) {
    if (i != predindex)
      if (!term_ListContainsTerm((LIST) list_PairSecond(Pair),
				 clause_GetLiteralAtom(Clause, i)))
	ok = FALSE;
  }

  /* Check if positive literals are required for definition */
  for (i=clause_FirstSuccedentLitIndex(Clause);
       (i < clause_Length(Clause)) && ok; i++)
    if (!term_ListContainsTerm((LIST) list_PairFirst(Pair),
			       clause_GetLiteralAtom(Clause, i)))
      ok = FALSE;
  
  if (!ok)
    return FALSE;
  else {
    /* Complement for definition found, remove literals from pair lists */
    for (i=0; i < clause_FirstSuccedentLitIndex(Clause); i++)
      if (i != predindex)
	list_Rplacd(Pair,
		    list_DeleteElement((LIST) list_PairSecond(Pair),
				       clause_GetLiteralAtom(Clause, i),
				       (BOOL (*)(POINTER, POINTER)) term_Equal));
    for (i=clause_FirstSuccedentLitIndex(Clause); i < clause_Length(Clause); i++)
      list_Rplaca(Pair,
		  list_DeleteElement((LIST) list_PairFirst(Pair),
				     clause_GetLiteralAtom(Clause, i),
				     (BOOL (*)(POINTER, POINTER)) term_Equal));
    return TRUE;
  }
}

void clause_FPrintRule(FILE* File, CLAUSE Clause)
/**************************************************************
  INPUT:   A file and a clause.
  RETURNS: Nothing.
  SUMMARY: Prints any term of the clause to file in rule format.
  CAUTION: Uses the term_Output functions.
***************************************************************/
{
  int  i,n;
  TERM Literal;
  LIST scan,antecedent,succedent,constraints;

  n = clause_Length(Clause);

  constraints = list_Nil();
  antecedent  = list_Nil();
  succedent   = list_Nil();

  for (i = 0; i < n; i++) {
    Literal = clause_GetLiteralTerm(Clause,i);
    if (symbol_Equal(term_TopSymbol(Literal),fol_Not())) {
      if (symbol_Arity(term_TopSymbol(fol_Atom(Literal))) == 1 &&
	  symbol_IsVariable(term_TopSymbol(term_FirstArgument(fol_Atom(Literal)))))
	constraints = list_Cons(Literal,constraints);
      else
	antecedent = list_Cons(Literal,antecedent);
    }
    else
      succedent = list_Cons(Literal,succedent);
  }

  for (scan = constraints; !list_Empty(scan); scan = list_Cdr(scan)) {
    term_FPrint(File, fol_Atom(list_Car(scan)));
    putc(' ', File);
  }
  fputs("||", File);
  for (scan = antecedent; !list_Empty(scan); scan = list_Cdr(scan)) {
    putc(' ', File);
    term_FPrint(File,fol_Atom(list_Car(scan)));
    if (list_Empty(list_Cdr(scan)))
      putc(' ', File);
  }
  fputs("->", File);
  for (scan = succedent; !list_Empty(scan); scan = list_Cdr(scan)) {
    putc(' ', File);
    term_FPrint(File,list_Car(scan));
  }
  fputs(".\n", File);
  
  list_Delete(antecedent);
  list_Delete(succedent);
  list_Delete(constraints);
}


void clause_FPrintOtter(FILE* File, CLAUSE clause)
/**************************************************************
  INPUT:   A file and a clause.
  RETURNS: Nothing.
  SUMMARY: Prints any clause to File.
  CAUTION: Uses the other clause_Output functions.
***************************************************************/
{
  int     n,j;
  LITERAL Lit;
  TERM    Atom;
  
  n = clause_Length(clause);

  if (n == 0)
    fputs(" $F ", File);
  else {
    for (j = 0; j < n; j++) {
      Lit  = clause_GetLiteral(clause,j);
      Atom = clause_LiteralAtom(Lit);
      if (clause_LiteralIsNegative(Lit))
	putc('-', File);
      if (fol_IsEquality(Atom)) {
	if (clause_LiteralIsNegative(Lit))
	  putc('(', File);
	term_FPrintOtterPrefix(File,term_FirstArgument(Atom));
	fputs(" = ", File);
	term_FPrintOtterPrefix(File,term_SecondArgument(Atom));
	if (clause_LiteralIsNegative(Lit))
	  putc(')', File);
      }
      else
	term_FPrintOtterPrefix(File,Atom);
      if (j <= (n-2))
	fputs(" | ", File);
    }
  }

  fputs(".\n", File);
}


void clause_FPrintCnfDFG(FILE* File, BOOL OnlyProductive, LIST Axioms,
			 LIST Conjectures, FLAGSTORE Flags,
			 PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A file, a list of axiom clauses and a list of conjecture clauses.
           A flag indicating whether only potentially productive clauses should
	   be printed.
	   A flag store.
	   A precedence.
  RETURNS: Nothing.
  SUMMARY: Prints a  the respective clause lists to <File> dependent
           on <OnlyProductive>.
***************************************************************/
{
  LIST   scan;
  CLAUSE Clause;

  if (Axioms) {
    fputs("list_of_clauses(axioms, cnf).\n", File);
    for (scan=Axioms;!list_Empty(scan);scan=list_Cdr(scan)) {
      Clause = (CLAUSE)list_Car(scan);
      if (!OnlyProductive ||
	  (clause_HasSolvedConstraint(Clause) &&
	   !clause_HasSelectedLiteral(Clause, Flags, Precedence)))
	clause_FPrintDFG(File,Clause,FALSE);
    }
    fputs("end_of_list.\n\n", File);
  }

  if (Conjectures) {
    fputs("list_of_clauses(conjectures, cnf).\n", File);
    for (scan=Conjectures;!list_Empty(scan);scan=list_Cdr(scan)) {
      Clause = (CLAUSE)list_Car(scan);
      if (!OnlyProductive ||
	  (clause_HasSolvedConstraint(Clause) &&
	   !clause_HasSelectedLiteral(Clause, Flags, Precedence)))
	clause_FPrintDFG(File,Clause,FALSE);
    }
    fputs("end_of_list.\n\n", File);
  }
}

void clause_FPrintCnfDFGProof(FILE* File, BOOL OnlyProductive, LIST Axioms,
				 LIST Conjectures, FLAGSTORE Flags,
				 PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A file, a list of axiom clauses and a list of conjecture clauses.
           A flag indicating whether only potentially productive clauses should
	   be printed.
	   A flag store.
	   A precedence.
  RETURNS: Nothing.
  SUMMARY: Prints a  the respective clause lists to <File> dependent
           on <OnlyProductive>.
***************************************************************/
{
  LIST   scan;
  CLAUSE Clause;

  if (Axioms) {
    fputs("list_of_clauses(axioms, cnf).\n", File);
    for (scan=Axioms;!list_Empty(scan);scan=list_Cdr(scan)) {
      Clause = (CLAUSE)list_Car(scan);
      if (!OnlyProductive ||
	  (clause_HasSolvedConstraint(Clause) &&
	   !clause_HasSelectedLiteral(Clause, Flags, Precedence)))
	clause_FPrintDFGProof(File,Clause,FALSE);
    }
    fputs("end_of_list.\n\n", File);
  }

  if (Conjectures) {
    fputs("list_of_clauses(conjectures, cnf).\n", File);
    for (scan=Conjectures;!list_Empty(scan);scan=list_Cdr(scan)) {
      Clause = (CLAUSE)list_Car(scan);
      if (!OnlyProductive ||
	  (clause_HasSolvedConstraint(Clause) &&
	   !clause_HasSelectedLiteral(Clause, Flags, Precedence)))
	clause_FPrintDFGProof(File,Clause,FALSE);
    }
    fputs("end_of_list.\n\n", File);
  }
}


static void clause_FPrintDescription(FILE* File, const char* Name,
				     const char* Author, const char* Status,
				     const char* Description)
{
  fputs("list_of_descriptions.\n", File);
  fprintf(File, "name(%s).\n", Name);
  fprintf(File, "author(%s).\n", Author);
  fprintf(File, "status(%s).\n", Status);
  fprintf(File, "description(%s).\n", Description);
  fputs("end_of_list.\n", File);
}

static void clause_FPrintClauseFomulaRelationForClauses(FILE* File, LIST Clauses, HASHMAP ClauseToTermLabelList)
/**************************************************************
  INPUT:   A file, a list of axioms, a list of conjectures, 
           and a relationship between clauses and formala names.
  RETURNS: Nothing.
  SUMMARY: Prints relationship between clauses and formula names
           to <File>.
***************************************************************/
{
  LIST Scan, ValueList;
  		
  for (Scan=Clauses; !list_Empty(Scan); Scan=list_Cdr(Scan)) {
    ValueList = hm_Retrieve(ClauseToTermLabelList, list_Car(Scan));
    if (!list_Empty(ValueList)) {
      fprintf(File, "(%zd,", clause_Number(list_Car(Scan)));
      fputs(list_Car(ValueList), File);
      ValueList = list_Cdr(ValueList);
      while (!list_Empty(ValueList)) {
        fputs(",", File);
        fputs(list_Car(ValueList), File);
        ValueList = list_Cdr(ValueList);
      }
      fputs(")", File);
      if (!list_Empty(list_Cdr(Scan)))
        fputs(",\n   ", File);
    }
  }	
}

static void clause_FPrintClauseFormulaRelation(FILE* File, LIST Axioms,
					       LIST Conjectures, HASHMAP ClauseToTermLabelList)
/**************************************************************
  INPUT:   A file, a list of axioms, a list of conjectures, 
           and a relationship between clauses and formala names.
  RETURNS: Nothing.
  SUMMARY: Prints relationship between clauses and formula names
           to <File>.
***************************************************************/
{
  fputs("set_ClauseFormulaRelation(", File);
  clause_FPrintClauseFomulaRelationForClauses(File, Axioms, ClauseToTermLabelList);
  if (!list_Empty(Conjectures) && !list_Empty(Axioms))
    fputs(",\n   ", File);
  clause_FPrintClauseFomulaRelationForClauses(File, Conjectures, ClauseToTermLabelList);
  fputs(").\n\n\n", File);
}

static void clause_FPrintSelectedSymbols(FILE* File)
/**************************************************************
  INPUT:   A file.
  RETURNS: Nothing.
  SUMMARY: Prints selected symbols to <File>.
***************************************************************/
{
  LIST Scan, SelectedSymbols;

  SelectedSymbols = symbol_GetAllSymbolsWithProperty(SELECTED);
  
  /* Print the selected symbols. */
  if (!list_Empty(SelectedSymbols)) {
    fputs("set_selection(", File);
    
    for (Scan = SelectedSymbols; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
      SYMBOL Symbol;
	  
      Symbol = (SYMBOL)list_Car(Scan);
      fputs(symbol_Name(Symbol), File);

      /* If there are more selected symbols following, add a ',' after the symbol. */
      if (!list_Empty(list_Cdr(Scan)))
        putc(',', File);
    }
    fputs(").\n\n\n", File);

    list_Free(SelectedSymbols);
  }
}         

void clause_FPrintSettings(FILE* File, LIST Axioms,
				  LIST Conjectures, FLAGSTORE Flags,
				  PRECEDENCE Precedence, HASHMAP ClauseToTermLabelList)
/**************************************************************
  INPUT:   A file, a list of axioms, a list of conjectures, 
           a flag store, a precedence, a relationship 
           between clauses and formula names.
  RETURNS: Nothing.
  SUMMARY: Prints SPASS settings to <File>.
***************************************************************/
{
  fputs("list_of_settings(SPASS).\n{*\n", File);
  if (ClauseToTermLabelList != (HASHMAP)NULL)
    clause_FPrintClauseFormulaRelation(File, Axioms, Conjectures, ClauseToTermLabelList);
  
  /* Print just the necessary flags. */
  if (Flags != NULL) {
    flag_FPrintFlag(File, Flags, flag_ORD);
    flag_FPrintFlag(File, Flags, flag_SELECT);
    fputs("\nset_flag(RInput,0).",File);
    flag_FPrintFlagWithValue(File, Flags, flag_SPLITS, flag_SPLITSOFF, ""); /* spliting is forced off by default */
    fputs("\n\n", File);
  
    /* Print the selected symbols. */
    if(flag_GetFlagIntValue(Flags, flag_SELECT) != flag_SELECTOFF)
      clause_FPrintSelectedSymbols(File);
  }
  
  if(Precedence != NULL)
    symbol_FPrintPrecedence(File, Precedence);

  fputs("*}\nend_of_list.\n\n", File);
}

static void clause_FPrintClauses(FILE* File, BOOL OnlyProductive,
                                 BOOL PrintConjectures, LIST Clauses, 
                                 FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A file, a flag indicating whether only potentially 
           productive clauses should be printed, a flag 
           indicating whether only conjectures or only axioms 
           should be printed, a list of clauses, an optional 
           flag store and an optional precedence.
  RETURNS: Nothing.
  SUMMARY: Prints (productive) axiom or conjecture clauses to 
           <File>.
***************************************************************/
{
  LIST Scan;
	
  for (Scan=Clauses;!list_Empty(Scan);Scan=list_Cdr(Scan)) {
    CLAUSE Clause;
  	
    Clause = (CLAUSE) list_Car(Scan);
  	
    if (!OnlyProductive ||
        (clause_HasSolvedConstraint(Clause) &&
	 !clause_HasSelectedLiteral(Clause, Flags, Precedence)))
      if (PrintConjectures == clause_GetFlag(Clause,CONCLAUSE))
        clause_FPrintDFG(File,Clause,FALSE);
  }
}

void clause_FPrintCnfDFGProblem(FILE* File, BOOL OnlyProductive, 
                                const char* Name, const char* Author, 
                                const char* Status, const char* Description, 
                                LIST Clauses, LIST Conjectures, 
                                FLAGSTORE Flags, PRECEDENCE Precedence, 
                                HASHMAP  ClauseToTermLabelList, 
                                BOOL IsModel, BOOL PrintSettings)
/**************************************************************
  INPUT:   A file, a flag indicating whether only potentially 
           productive clauses should be printed, the problem's 
           name, author, status and description to be included 
           in the description block given as strings, a list 
           of clauses, an optional additional list of 
           conjectures, an optional flag store, an optional 
           precedence, an optional relationship between 
           clauses and formula names, whether the problem 
           is a model, and whether the settings should be printed.
  RETURNS: Nothing.
  SUMMARY: Prints a complete DFG problem clause file to <File>.
***************************************************************/
{
  fputs("begin_problem(Unknown).\n\n", File);
  clause_FPrintDescription(File, Name,  Author, Status,  Description);
  putc('\n', File);

  fputs("list_of_symbols.\n", File);
  symbol_FPrintDFGSignature(File);
  fputs("end_of_list.\n\n", File);

  /* Since Clauses can hold both axioms and conjectures, it's
   * printed in both cases, once just the axioms, the other time
   * just the conjectures, except when we're printing a model.
   * If we are printing a model, the conjectures are printed 
   * along with the axioms, to keep prior behaviour.
   */
  fputs("list_of_clauses(axioms, cnf).\n", File);
  clause_FPrintClauses(File, OnlyProductive, FALSE, 
                       Clauses, Flags, Precedence);
  if(IsModel)
    clause_FPrintClauses(File, OnlyProductive, TRUE,
                         Clauses, Flags, Precedence);
  fputs("end_of_list.\n\n", File);

  fputs("list_of_clauses(conjectures, cnf).\n", File);
  if(!IsModel)
    clause_FPrintClauses(File, OnlyProductive, TRUE,
                         Clauses, Flags, Precedence);
  clause_FPrintClauses(File, OnlyProductive, TRUE,
                       Conjectures, Flags, Precedence);
  fputs("end_of_list.\n\n", File);

  if(PrintSettings)
    clause_FPrintSettings(File, Clauses, Conjectures, Flags, 
                          Precedence, ClauseToTermLabelList);
  
  fputs("\nend_problem.\n\n", File);
}

void clause_FPrintCnfFormulasDFGProblem(FILE* File, BOOL OnlyProductive,
					const char* Name, const char* Author,
					const char* Status,
					const char* Description, LIST Axioms,
					LIST Conjectures, FLAGSTORE Flags,
					PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A file, a list of axiom clauses and a list of conjecture clauses.
           A flag indicating whether only potentially productive clauses should
	   be printed.
	   A bunch of strings that are printed to the description.
	   A flag store.
	   A precedence.
  RETURNS: Nothing.
  SUMMARY: Prints the respective clause lists as a complete DFG formulae output
           to <File>.
***************************************************************/
{
  LIST   scan;
  CLAUSE Clause;

  fputs("begin_problem(Unknown).\n\n", File);
  clause_FPrintDescription(File, Name,  Author, Status,  Description);
  fputs("\nlist_of_symbols.\n", File);
  symbol_FPrintDFGSignature(File);
  fputs("end_of_list.\n\n", File);

  if (Axioms) {
    fputs("list_of_formulae(axioms).\n", File);
    for (scan=Axioms; !list_Empty(scan); scan=list_Cdr(scan)) {
      Clause = (CLAUSE)list_Car(scan);
      if (!OnlyProductive ||
	  (clause_HasSolvedConstraint(Clause) &&
	   !clause_HasSelectedLiteral(Clause, Flags, Precedence)))
	clause_FPrintFormulaDFG(File,Clause,FALSE);
    }
    fputs("end_of_list.\n\n", File);
  }

  if (Conjectures) {
    fputs("list_of_formulae(conjectures).\n", File);
    for (scan=Conjectures; !list_Empty(scan); scan=list_Cdr(scan)) {
      Clause = (CLAUSE)list_Car(scan);
      if (!OnlyProductive ||
	  (clause_HasSolvedConstraint(Clause) &&
	   !clause_HasSelectedLiteral(Clause, Flags, Precedence)))
	clause_FPrintFormulaDFG(File,Clause,FALSE);
    }
    fputs("end_of_list.\n\n", File);
  }

  fputs("list_of_settings(SPASS).\n{*\n", File);
  fol_FPrintPrecedence(File, Precedence);
  fputs("\n*}\nend_of_list.\n\nend_problem.\n\n", File);
}

void clause_FPrintCnfOtter(FILE* File, LIST Clauses, FLAGSTORE Flags)
/**************************************************************
  INPUT:   A file, a list of clauses and a flag store.
  RETURNS: Nothing.
  SUMMARY: Prints the clauses to <File> in a format readable by Otter.
***************************************************************/
{
  LIST   scan;
  int    i;
  BOOL   Equality;
  CLAUSE Clause;

  Equality = FALSE;

  for (scan=Clauses;!list_Empty(scan) && !Equality; scan=list_Cdr(scan)) {
    Clause = (CLAUSE)list_Car(scan);
    for (i=clause_FirstAntecedentLitIndex(Clause);i<clause_Length(Clause);i++)
      if (fol_IsEquality(clause_GetLiteralAtom(Clause,i))) {
	Equality = TRUE;
	i = clause_Length(Clause);
      }
  }

  fol_FPrintOtterOptions(File, Equality,
			 flag_GetFlagIntValue(Flags, flag_TDFG2OTTEROPTIONS));

  if (Clauses) {
    fputs("list(usable).\n", File);
    if (Equality)
      fputs("x=x.\n", File);
    for (scan=Clauses;!list_Empty(scan);scan=list_Cdr(scan))
      clause_FPrintOtter(File,list_Car(scan));
    fputs("end_of_list.\n\n", File);
  }
}


void clause_FPrintCnfDFGDerivables(FILE* File, LIST Clauses, BOOL Type)
/**************************************************************
  INPUT:   A file, a list of clauses and a bool flag Type.
  RETURNS: Nothing.
  SUMMARY: If <Type> is true then all axiom clauses in <Clauses> are
	   written to <File>. Otherwise all conjecture clauses in
	   <Clauses> are written to <File>.
***************************************************************/
{
  CLAUSE Clause;

  while (Clauses) {
    Clause = (CLAUSE)list_Car(Clauses);
    if ((Type && !clause_GetFlag(Clause,CONCLAUSE)) ||
	(!Type && clause_GetFlag(Clause,CONCLAUSE)))
      clause_FPrintDFG(File,Clause,FALSE);
    Clauses = list_Cdr(Clauses);
  }
}


void clause_FPrintDFGStep(FILE* File, CLAUSE Clause, BOOL Justif)
/**************************************************************
  INPUT:   A file, a clause and a boolean value.
  RETURNS: Nothing.
  SUMMARY: Prints any clause together with a label (the clause number)
	   to File. If <Justif> is TRUE also the labels of the parent
	   clauses are printed.
  CAUTION: Uses the other clause_Output functions.
***************************************************************/
{
  int     n,j;
  LITERAL Lit;
  TERM    Atom;
  LIST    Variables,Iter;
  
  n = clause_Length(Clause);

  fputs("  step(", File);
  fprintf(File, "%zd,", clause_Number(Clause));

  Variables = list_Nil();

  for (j = 0; j < n; j++)
    Variables =
      list_NPointerUnion(Variables,
	term_VariableSymbols(clause_GetLiteralAtom(Clause,j)));

  if (!list_Empty(Variables)) {
    symbol_FPrint(File, fol_All());
    fputs("([", File);
    for (Iter = Variables; !list_Empty(Iter); Iter = list_Cdr(Iter)) {
      symbol_FPrint(File, (SYMBOL) list_Car(Iter));
      if (!list_Empty(list_Cdr(Iter)))
	putc(',', File);
    }
    fputs("],", File);
  }
  
  symbol_FPrint(File, fol_Or());
  putc('(', File);

  for (j = 0; j < n; j++) {
    Lit  = clause_GetLiteral(Clause,j);
    Atom = clause_LiteralSignedAtom(Lit);
    term_FPrintPrefix(File,Atom);
    if (j+1 < n)
      putc(',', File);
  }
  if (n==0)
    symbol_FPrint(File,fol_False());

  if (!list_Empty(Variables)) {
    list_Delete(Variables);
    putc(')', File);
  }
  fputs("),", File);
  clause_FPrintOrigin(File, Clause);

  fputs(",[", File);
  for (Iter = clause_ParentClauses(Clause);
      !list_Empty(Iter);
      Iter = list_Cdr(Iter)) {
    fprintf(File, "%zd", (intptr_t)list_Car(Iter));
    if (!list_Empty(list_Cdr(Iter)))
      putc(',', File);
  }
  putc(']', File);
  fprintf(File, ",[splitlevel:%zd]", clause_SplitLevel(Clause));
  
  fputs(").\n", File);
}

void clause_FPrintDFGProof(FILE* File, CLAUSE Clause, BOOL Justif)
/**************************************************************
  INPUT:   A file, a clause and a boolean value.
  RETURNS: Nothing.
  SUMMARY: Prints any clause together with a label (the clause number)
	   to File. If Justif is TRUE also the labels of the parent
	   clauses are printed.
  CAUTION: Uses the other clause_Output functions.
***************************************************************/
{
  int     n,j;
  LITERAL Lit;
  TERM    Atom;
  LIST    Variables,Iter;
  
  n = clause_Length(Clause);

  fputs("  clause(", File);
  Variables = list_Nil();

  for (j = 0; j < n; j++)
    Variables =
      list_NPointerUnion(Variables,
	term_VariableSymbols(clause_GetLiteralAtom(Clause,j)));

  if (!list_Empty(Variables)) {
    symbol_FPrint(File, fol_All());
    fputs("([", File);
    for (Iter = Variables; !list_Empty(Iter); Iter = list_Cdr(Iter)) {
      symbol_FPrint(File, (SYMBOL) list_Car(Iter));
      if (!list_Empty(list_Cdr(Iter)))
	putc(',', File);
    }
    fputs("],", File);
  }
  
  symbol_FPrint(File, fol_Or());
  putc('(', File);

  for (j = 0; j < n; j++) {
    Lit  = clause_GetLiteral(Clause,j);
    Atom = clause_LiteralSignedAtom(Lit);
    term_FPrintPrefix(File,Atom);
    if (j+1 < n)
      putc(',', File);
  }
  if (n==0)
    symbol_FPrint(File,fol_False());

  if (!list_Empty(Variables)) {
    list_Delete(Variables);
    putc(')', File);
  }
  fprintf(File, "),%zd", clause_Number(Clause));

  if (Justif) {
    putc(',', File);
    clause_FPrintOrigin(File, Clause);
    fputs(",[", File);
    for (Iter = clause_ParentClauses(Clause);
	!list_Empty(Iter);
	Iter = list_Cdr(Iter)) {
      fprintf(File, "%zd", (intptr_t)list_Car(Iter));
      if (!list_Empty(list_Cdr(Iter)))
	putc(',', File);
    }
    putc(']', File);
    fprintf(File, ",%zd", clause_SplitLevel(Clause));
  }
  
  fputs(").\n", File);
}

void clause_FPrintDFG(FILE* File, CLAUSE Clause, BOOL Justif)
/**************************************************************
  INPUT:   A file, a clause and a boolean value.
  RETURNS: Nothing.
  SUMMARY: Prints any clause together with a label (the clause number)
	   to File. If Justif is TRUE also the labels of the parent
	   clauses are printed.
  CAUTION: Uses the other clause_Output functions.
***************************************************************/
{
  int     n,j,c,a,s;
  LITERAL Lit;
  LIST    Iter;
  
  fputs("  clause(", File);
  
  n = clause_Length(Clause);
  if (n==0)
    symbol_FPrint(File,fol_False());
  else {
    c = clause_NumOfConsLits(Clause);
    a = clause_NumOfAnteLits(Clause);
    s = clause_NumOfSuccLits(Clause);

    for (j = 0; j < c; j++) {
      putchar(' ');
      Lit = clause_GetLiteral(Clause, j);
      clause_LiteralFPrintUnsigned(File, Lit);
      if (j+1 < c)
	    putc(' ', File);
    }
    fputs(" || ", File);

    a += c;
    for ( ; j < a; j++) {
      Lit = clause_GetLiteral(Clause, j);
      clause_LiteralFPrintUnsigned(File, Lit);     
      if (clause_LiteralGetFlag(Lit,LITSELECT))
	    putc('+', File);
      if (j+1 < a)
	    putc(' ', File);
    }
    fputs(" -> ",File);

    s += a;
    for ( ; j < s; j++) {
      Lit = clause_GetLiteral(Clause, j);
      clause_LiteralFPrintUnsigned(File, Lit);
#ifdef CHECK
      if (clause_LiteralGetFlag(Lit,LITSELECT)) {
	    misc_StartErrorReport();
	    misc_ErrorReport("\n In clause_FPrintDFG: Clause has selected positive literal.\n");
	    misc_FinishErrorReport();
      }
#endif
      if (j+1 < s)
	    putc(' ', File);
    }
  }

  fprintf(File, ",%zd", clause_Number(Clause));

  if (Justif) {
    putc(',', File);
    clause_FPrintOrigin(File, Clause);
    fputs(",[", File);
    for (Iter = clause_ParentClauses(Clause);
	!list_Empty(Iter);
	Iter = list_Cdr(Iter)) {
      fprintf(File, "%zd", (intptr_t)list_Car(Iter));
      if (!list_Empty(list_Cdr(Iter)))
	putc(',', File);
    }
    putc(']', File);
    fprintf(File, ",%zd", clause_SplitLevel(Clause));
  }
  
  fputs(").\n", File);
}

void clause_FPrintFormulaDFG(FILE* File, CLAUSE Clause, BOOL Justif)
/**************************************************************
  INPUT:   A file, a clause and a boolean value.
  RETURNS: Nothing.
  SUMMARY: Prints any clause together with a label (the clause number)
	   as DFG Formula to File. If Justif is TRUE also the labels of the
	   parent clauses are printed.
  CAUTION: Uses the other clause_Output functions.
***************************************************************/
{
  int     n,j;
  LITERAL Lit;
  TERM    Atom;
  LIST    Variables,Iter;
  
  n = clause_Length(Clause);

  fputs("  formula(", File);
  Variables = list_Nil();

  for (j = 0; j < n; j++)
    Variables =
      list_NPointerUnion(Variables,
	term_VariableSymbols(clause_GetLiteralAtom(Clause,j)));

  if (!list_Empty(Variables)) {
    symbol_FPrint(File, fol_All());
    fputs("([", File);
    for (Iter = Variables; !list_Empty(Iter); Iter = list_Cdr(Iter)) {
      symbol_FPrint(File, (SYMBOL) list_Car(Iter));
      if (!list_Empty(list_Cdr(Iter)))
	putc(',', File);
    }
    fputs("],", File);
  }
  
  if (n>1) {
    symbol_FPrint(File, fol_Or());
    putc('(', File);
  }

  for (j = 0; j < n; j++) {
    Lit  = clause_GetLiteral(Clause,j);
    Atom = clause_LiteralSignedAtom(Lit);
    term_FPrintPrefix(File,Atom);
    if (j+1 < n)
      putc(',', File);
  }
  if (n==0)
    symbol_FPrint(File,fol_False());

  if (!list_Empty(Variables)) {
    list_Delete(Variables);
    putc(')', File);
  }

  if (n>1)
    fprintf(File, "),%zd", clause_Number(Clause));
  else
    fprintf(File, ",%zd", clause_Number(Clause));

  if (Justif) {
    putc(',', File);
    clause_FPrintOrigin(File, Clause);
    fputs(",[", File);
    for (Iter = clause_ParentClauses(Clause);
	 !list_Empty(Iter);
	 Iter = list_Cdr(Iter)) {
      fprintf(File, "%zd", (intptr_t)list_Car(Iter));
      if (!list_Empty(list_Cdr(Iter)))
	putc(',', File);
    }
    putc(']', File);
    fprintf(File, ",%zd", clause_SplitLevel(Clause));
  }
  
  fputs(").\n", File);
}


void clause_CheckAux(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence, BOOL VarIsConst)
/**************************************************************
  INPUT:   A clause, a flag store and a precedence and a flag.
  RETURNS: Nothing.
  EFFECT:  Checks whether the clause is in a proper state. If
           not, a core is dumped.
  CAUTION: If <VarIsConst> is set to TRUE variables of
           <Clause> are interpreted as constants
***************************************************************/
{
  CLAUSE Copy;
  if (!clause_Exists(Clause))
    return;
  
  if (!clause_IsClauseAux(Clause, Flags, Precedence, VarIsConst)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_Check: Clause not consistent !\n");
    misc_FinishErrorReport();
  }

  Copy = clause_Copy(Clause);

  if(VarIsConst)
        clause_PrecomputeOrderingAndReInitSkolem(Copy, Flags, Precedence);
  else
        clause_PrecomputeOrderingAndReInit(Copy, Flags, Precedence);

  if ((clause_Weight(Clause) != clause_Weight(Copy)) ||
      (clause_MaxVar(Clause) != clause_MaxVar(Copy))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_Check: Weight or maximal variable not properly set.\n");
    misc_FinishErrorReport();
  }
  clause_Delete(Copy);
}

void clause_Check(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  Checks whether the clause is in a proper state. If
           not, a core is dumped.
***************************************************************/
{
        clause_CheckAux(Clause, Flags, Precedence, FALSE);
}


void clause_CheckSkolem(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  Checks whether the clause is in a proper state. If
           not, a core is dumped.
***************************************************************/
{
        clause_CheckAux(Clause, Flags, Precedence, TRUE);
}

/* The following are output procedures for clauses with parent pointers */


void clause_PParentsFPrintParentClauses(FILE* File, CLAUSE Clause, BOOL ParentPts)
/**************************************************************
  INPUT:   A file, a clause and a boolean flag indicating whether
           the clause's parents are given by numbers or pointers.
  RETURNS: Nothing.
  SUMMARY: Prints the clauses parent clauses and -literals to the file.
           If <ParentPts> is TRUE the parent clauses are defined
           by pointers, else by numbers.
***************************************************************/
{
  LIST Scan1,Scan2;
  int  length;
  intptr_t  ParentNum;
  
  if (!list_Empty(clause_ParentClauses(Clause))) {
    
    Scan1 = clause_ParentClauses(Clause);
    Scan2 = clause_ParentLiterals(Clause);
    
    if (ParentPts)
      ParentNum = clause_Number(list_Car(Scan1));
    else
      ParentNum = (intptr_t)list_Car(Scan1);

    fprintf(File, "%zd.%zd", ParentNum, (intptr_t)list_Car(Scan2));
    
    if (!list_Empty(list_Cdr(Scan1))) {
      
      length = list_Length(Scan1) - 2;
      putc(',', File);
      Scan1 = list_Cdr(Scan1);
      Scan2 = list_Cdr(Scan2);
      
      if (ParentPts)
	ParentNum = clause_Number(list_Car(Scan1));
      else
	ParentNum = (intptr_t)list_Car(Scan1);

      fprintf(File, "%zd.%zd", ParentNum, (intptr_t)list_Car(Scan2));
      
      for (Scan1 = list_Cdr(Scan1), Scan2 = list_Cdr(Scan2);
	   !list_Empty(Scan1);
	   Scan1 = list_Cdr(Scan1), Scan2 = list_Cdr(Scan2)) {
	
	length -= 2;
	
	if (ParentPts)
	  ParentNum = clause_Number(list_Car(Scan1));
	else
	  ParentNum = (intptr_t)list_Car(Scan1);

	fprintf(File, ",%zd.%zd", ParentNum, (intptr_t)list_Car(Scan2));
      }
    }
  }
}

void clause_PParentsLiteralFPrintUnsigned(FILE* File, LITERAL Literal)
/**************************************************************
  INPUT:   A Literal.
  RETURNS: Nothing.
  SUMMARY:
***************************************************************/
{
#ifdef CHECK
  if (!clause_LiteralIsLiteral(Literal)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In clause_PParentsLiteralFPrintUnsigned:");
    misc_ErrorReport("\n Illegal input. Input not a literal.");
    misc_FinishErrorReport();
  }
#endif

  term_FPrintPrefix(File, clause_LiteralAtom(Literal));
  fflush(stdout);
}

void clause_PParentsFPrintGen(FILE* File, CLAUSE Clause, BOOL ParentPts)
/**************************************************************
  INPUT:   A file, a clause, a boolean flag.
  RETURNS: Nothing.
  EFFECTS: Prints the clause to file in SPASS format. If <ParentPts>
           is TRUE, the parents of <Clause> are interpreted as pointers.
***************************************************************/
{
  LITERAL Lit;
  int i,c,a,s;

  if (!clause_Exists(Clause))
    fputs("(CLAUSE)NULL", File);
  else {
    fprintf(File, "%zd",clause_Number(Clause));
    
    fprintf(File, "[%zd:", clause_SplitLevel(Clause));

#ifdef CHECK
    if (Clause->splitfield_length <= 1)
      fputs("0.", File);
    else
      for (i=Clause->splitfield_length-1; i > 0; i--)
	fprintf(File, "%lu.", Clause->splitfield[i]);
    if (Clause->splitfield_length == 0)
      putc('1', File);
    else
      fprintf(File, "%lu", (Clause->splitfield[0] | 1));
    fprintf(File,":%c%c:%c:%zu:%zu:", clause_GetFlag(Clause, CONCLAUSE) ? 'C' : 'A',
	   clause_GetFlag(Clause, WORKEDOFF) ? 'W' : 'U',
	   clause_GetFlag(Clause, NOPARAINTO) ? 'N' : 'P',
	   clause_Weight(Clause), clause_Depth(Clause));
#endif
    
    clause_FPrintOrigin(File, Clause);
    
    if (!list_Empty(clause_ParentClauses(Clause))) {
      putc(':', File);
      clause_PParentsFPrintParentClauses(File, Clause, ParentPts);
    }
    putc(']', File);

    c = clause_NumOfConsLits(Clause);
    a = clause_NumOfAnteLits(Clause);
    s = clause_NumOfSuccLits(Clause);

    for (i = 0; i < c; i++) {
      Lit = clause_GetLiteral(Clause, i);
      clause_PParentsLiteralFPrintUnsigned(File, Lit);
      if (i+1 < c)
	putc(' ', File);
    }
    fputs(" || ", File);

    a += c;
    for ( ; i < a; i++) {

      Lit = clause_GetLiteral(Clause, i);
      clause_PParentsLiteralFPrintUnsigned(File, Lit);
      if (clause_LiteralIsMaximal(Lit)) {
	putc('*', File);
	if (clause_LiteralIsOrientedEquality(Lit))
	  putc('*', File);
      }
      if (clause_LiteralGetFlag(Lit,LITSELECT))
	putc('+', File);
      if (i+1 < a)
	putc(' ', File);
    }
    fputs(" -> ",File);

    s += a;
    for ( ; i < s; i++) {

      Lit = clause_GetLiteral(Clause, i);
      clause_PParentsLiteralFPrintUnsigned(File, Lit);
      if (clause_LiteralIsMaximal(Lit)) {
	putc('*', File);
	if (clause_LiteralIsOrientedEquality(Lit))
	  putc('*', File);
      }
#ifdef CHECK
      if (clause_LiteralGetFlag(Lit, LITSELECT)) {
	misc_StartErrorReport();
	misc_ErrorReport("\n In clause_PParentsFPrintGen: Clause has selected positive literal.\n");
	misc_FinishErrorReport();
      }
#endif
      if (i+1 < s)
	putc(' ', File);
    }
    putc('.', File);
  }
}

void clause_PParentsFPrint(FILE* File, CLAUSE Clause)
/**************************************************************
  INPUT:   A file handle and a clause.
  RETURNS: Nothing.
  EFFECTS: Prints out the clause to file in SPASS output format
***************************************************************/
{
  clause_PParentsFPrintGen(File, Clause, TRUE);
}

void clause_PParentsListFPrint(FILE* File, LIST L)
/**************************************************************
 INPUT:   A file handle, a list of clauses with parent pointers
 RETURNS: Nothing.
 EFFECTS: Print the list to <file>.
***************************************************************/
{
  while (!list_Empty(L)) {
    clause_PParentsFPrint(File, list_Car(L));
    putc('\n', File);
    L = list_Cdr(L);
  }
}


void clause_PParentsPrint(CLAUSE Clause)
/**************************************************************
 INPUT:   A clause with parent pointers
 RETURNS: Nothing.
 EFFECTS: The clause is printed to stdout.
***************************************************************/
{
  clause_PParentsFPrint(stdout, Clause);
}

void clause_PParentsListPrint(LIST L)
/**************************************************************
 INPUT:   A file handle, a list of clauses with parent pointers
 RETURNS: Nothing.
 EFFECTS: Print the clause list to stdout.
***************************************************************/
{
  clause_PParentsListFPrint(stdout, L);
}

LIST clause_MergeMaxLitLists(LIST List1, LIST List2, FLAGSTORE Flags, PRECEDENCE Precedence) 
/**************************************************************
  INPUT:   Two sorted lists <List1> and <List2> of pairs (maxlit,clause)
           the lists are sorted occording to the max literals of the clauses
  RETURNS: The merged list ordered with respect to the maximum literals
***************************************************************/
{

  LIST Scan1, Scan2, Result, ResultStart;

  if (list_Empty(List1))
    return List2;

  if (list_Empty(List2))
    return List1;

  if (ord_IsSmallerThan(ord_LiteralCompare((TERM)list_PairFirst(list_Car(List1)), 
					 ord_UNCOMPARABLE, /* prop literals */
					 (TERM)list_PairFirst(list_Car(List2)), ord_UNCOMPARABLE,
					 TRUE,  Flags, Precedence))) {
    ResultStart = List1;
    Scan1       = list_Cdr(List1);
    Scan2       = List2;
  }
  else {
    ResultStart = List2;
    Scan1       = List1;
    Scan2       = list_Cdr(List2);
  }

  /* Result is the last element of the merged list. */

  Result = ResultStart;

  while (!list_Empty(Scan1) && !list_Empty(Scan2)) {

    if (ord_IsSmallerThan(ord_LiteralCompare((TERM)list_PairFirst(list_Car(Scan1)), 
					 ord_UNCOMPARABLE, /* prop literals */
					 (TERM)list_PairFirst(list_Car(Scan2)), ord_UNCOMPARABLE,
					 TRUE,  Flags, Precedence))) {
      list_Rplacd(Result,Scan1);
      Scan1  = list_Cdr(Scan1);
    }
    else {
      list_Rplacd(Result,Scan2);
      Scan2  = list_Cdr(Scan2);
    }
    Result = list_Cdr(Result);
  }

  if (list_Empty(Scan1))
    list_Rplacd(Result, Scan2);
  else
    list_Rplacd(Result, Scan1);

  return ResultStart;
}



LIST clause_MergeSortByMaxLit(LIST L, FLAGSTORE Flags, PRECEDENCE Precedence) 
/**************************************************************
  INPUT:   A list of pairs (maxlit,clause) a flagstore and precedence
           maxlit are terms
  RETURNS: The list sorted with respect the max literals
***************************************************************/
{
  LIST Result; 

  if (!list_Empty(L) && !list_Empty(list_Cdr(L))) {
    LIST  lowerhalf;
    LIST  greaterhalf;

    LIST *lowerhalfptr;
    LIST *greaterhalfptr;

    lowerhalfptr = &lowerhalf;
    greaterhalfptr = &greaterhalf;

    list_Split(L, lowerhalfptr, greaterhalfptr);
    lowerhalf   = clause_MergeSortByMaxLit(lowerhalf, Flags, Precedence);
    greaterhalf = clause_MergeSortByMaxLit(greaterhalf, Flags, Precedence);

    Result = clause_MergeMaxLitLists(lowerhalf, greaterhalf, Flags, Precedence);
  }
  else {
    Result = L;
  }

  return Result;
}

BOOL clause_IsPropModelProductive(CLAUSE Clause, HASHMAP hash)
/**************************************************************
 INPUT:   A propositional clause and a hash storing true literals; it is assumed
          that clause has a strictly maximal positive literal
          and that all smaller literals are decided
 RETURNS: TRUE iff the clause is productive
 EFFECTS: None.
***************************************************************/
{
  int i;

  for (i=clause_FirstAntecedentLitIndex(Clause);i<=clause_LastAntecedentLitIndex(Clause);i++)
    if (list_Empty(hm_Retrieve(hash,(POINTER)term_TopSymbol(clause_LiteralAtom(clause_GetLiteral(Clause,i))))))
      return FALSE;
  for (i=clause_FirstSuccedentLitIndex(Clause);i<=clause_LastSuccedentLitIndex(Clause);i++)
    if (!clause_LiteralIsMaximal(clause_GetLiteral(Clause,i)) &&
	!list_Empty(hm_Retrieve(hash,(POINTER)term_TopSymbol(clause_LiteralAtom(clause_GetLiteral(Clause,i))))))
      return FALSE;
  return TRUE;
}

LIST clause_ComputePropModel(LIST Clauses, FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
 INPUT:   A saturated, satisfiable, non-redundant (at least using subsumption) 
          list of propositional clases.
 RETURNS: Returns a list of literals as terms representing the model.
 EFFECTS: None.
***************************************************************/
{
  LIST    Model,Scan;
  CLAUSE  Clause;
  TERM    Literal;
  int     i;
  HASHMAP hash;

  Model = list_Nil();

  for(Scan=Clauses;!list_Empty(Scan);Scan=list_Cdr(Scan)) {
    Clause = list_Car(Scan);
    for(i=clause_FirstLitIndex();i<=clause_LastLitIndex(Clause);i++) {
      if (clause_LiteralIsMaximal(clause_GetLiteral(Clause,i))) { /* we are prop so ord is total ! */
	Model = list_Cons(list_PairCreate(clause_LiteralSignedAtom(clause_GetLiteral(Clause,i)), Clause), 
			  Model);
	i = clause_LastLitIndex(Clause);
      }
    }
  }

  Model = clause_MergeSortByMaxLit(Model, Flags, Precedence);

  /* printf("\n Sorted Clauses:"); */
  /* for(Scan=Model;!list_Empty(Scan);Scan=list_Cdr(Scan)) { */
  /*   printf("\n"); */
  /*   clause_Print(list_PairSecond(list_Car(Scan))); */
  /* } */
  
  Clauses = Model;
  Model   = list_Nil();
  hash    = hm_Create(4, hm_PointerHash, hm_PointerEqual, FALSE); 
             /* putting an item in hash means its truth value is decided */
  Scan    = Clauses;
  while(!list_Empty(Scan)) {
    Literal = list_PairFirst(list_Car(Scan));
    if (fol_IsNegativeLiteral(Literal)) {
      if (list_Empty(hm_Retrieve(hash, (POINTER)term_TopSymbol(term_FirstArgument(Literal)))))
	Model = list_Cons(term_Copy(Literal), Model);
      while(!list_Empty(Scan) && term_Equal(Literal,list_PairFirst(list_Car(Scan))))
	Scan = list_Cdr(Scan);
    }
    else {
      while(!list_Empty(Scan) && !clause_IsPropModelProductive(list_PairSecond(list_Car(Scan)), hash) &&
	    term_Equal(Literal,list_PairFirst(list_Car(Scan))))
	Scan = list_Cdr(Scan);
      if (!list_Empty(Scan) && clause_IsPropModelProductive(list_PairSecond(list_Car(Scan)), hash))
	hm_Insert(hash, (POINTER)term_TopSymbol(Literal), Literal);
      Model = list_Cons(term_Copy(Literal), Model);
      while(!list_Empty(Scan) && term_Equal(Literal,list_PairFirst(list_Car(Scan))))
	Scan = list_Cdr(Scan);
    }
  }  
  list_DeleteWithElement(Clauses, (void (*)(POINTER))list_PairFree);
  hm_Delete(hash);
  return Model;
}

void clause_PrintPropModel(LIST Clauses, FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
 INPUT:   A saturated, satisfiable, non-redundant (at least using subsumption) 
          list of propositional clases.
 RETURNS: Nothing.
 EFFECTS: Prints the model with respect to the <Clauses> to <stdout>.
***************************************************************/
{
  LIST Model,Scan;
  TERM Literal;
  Model = clause_ComputePropModel(Clauses, Flags, Precedence);
  fputs("SPASS Minimal Model: ", stdout);
  Scan  = Model;
  while(!list_Empty(Scan)) {
    Literal = (TERM)list_Car(Scan);
    if (fol_IsNegativeLiteral(Literal)) {
      putc('-',stdout);
      term_PrintPrefix(term_FirstArgument(Literal));
    }
    else
      term_PrintPrefix(Literal);
    Scan = list_Cdr(Scan);
    if (!list_Empty(Scan))
      putc(',',stdout);
  }
  list_DeleteWithElement(Model, (void (*)(POINTER))term_Delete);
}
