/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                REDUCTION RULES                         * */
/* *                                                        * */
/* *  $Module:   REDRULES                                   * */
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
/* $Revision: 1.13 $                                        * */
/* $State: Exp $                                            * */
/* $Date: 2011-11-27 12:52:40 $                             * */
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


/* $RCSfile: rules-red.c,v $ */

#include "rules-red.h"

/**************************************************************/
/* previously Inlined Functions                               */
/**************************************************************/
BOOL red_WorkedOffMode(NAT Mode)
{
  return (Mode == red_WORKEDOFF || Mode == red_ALL);
}

BOOL red_OnlyWorkedOffMode(NAT Mode)
{
  return (Mode == red_WORKEDOFF);
}

BOOL red_UsableMode(NAT Mode)
{
  return (Mode == red_USABLE || Mode == red_ALL);
}

BOOL red_AllMode(NAT Mode)
{
  return (Mode == red_ALL);
}

/**************************************************************/
/* Inline Functions ends here                                 */
/**************************************************************/


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  Globals                                               * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


/* Needed for term stamping in red_RewriteRedUnitClause */
static NAT red_STAMPID;
 
const NAT red_USABLE    = 1;
const NAT red_WORKEDOFF = 2;
const NAT red_ALL       = 3;

/**************************************************************
 * Subterm Contextual Rewriting                               *
 **************************************************************/

/* variables for gathering statistical information for contextual rewriting*/
unsigned long red_CRW_CALLS = 0;
unsigned long red_CRW_SUCCESS = 0;

/* fault caching table */
st_INDEX red_CRW_TERM_FAULT_CACHE;

/**************************************************************/
/* FUNTION PROTOTYPES                                         */
/**************************************************************/

static BOOL red_SortSimplification(SORTTHEORY, CLAUSE, NAT, BOOL, FLAGSTORE,
				   PRECEDENCE, CLAUSE*);
static BOOL red_SelectedStaticReductions(PROOFSEARCH, CLAUSE*, CLAUSE*, LIST*,
					 NAT);

BOOL red_CRwGroundSubtermRedundant(PROOFSEARCH, CLAUSE, LIST*, LIST*, LIST, int);

/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  Functions                                             * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/

unsigned long red_GetCRwCalls()
{
  return red_CRW_CALLS;
}

unsigned long red_GetCRwSucceeded()
{
  return red_CRW_SUCCESS;
}

static void red_HandleRedundantIndexedClauses(PROOFSEARCH Search, LIST Blocked,
					      CLAUSE RedClause)
/*********************************************************
  INPUT:   A proof search object, a list <Blocked> of clauses from
           the proof search object and a clause that causes the
	   already indexed clauses in <Blocked> to be redundant.
  RETURNS: Nothing.
**********************************************************/
{
  FLAGSTORE Flags;
  CLAUSE    Clause;
  LIST      Scan;

  Flags = prfs_Store(Search);
  for (Scan = Blocked; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    Clause = (CLAUSE)list_Car(Scan);
    if (prfs_SplitLevelCondition(clause_SplitLevel(RedClause),clause_SplitLevel(Clause),
				 prfs_LastBacktrackLevel(Search)))
      split_DeleteClauseAtLevel(Search, Clause, clause_SplitLevel(RedClause));
    else {
      if (clause_GetFlag(Clause, WORKEDOFF)) {
	if (flag_GetFlagIntValue(Flags, flag_DOCPROOF))
	  prfs_MoveWorkedOffDocProof(Search, Clause);
	else
	  prfs_DeleteWorkedOff(Search, Clause);
      }
      else
	if (flag_GetFlagIntValue(Flags, flag_DOCPROOF))
	  prfs_MoveUsableDocProof(Search, Clause);
	else
	  prfs_DeleteUsable(Search, Clause);
    }
  }
}

static void red_HandleRedundantDerivedClauses(PROOFSEARCH Search, LIST Blocked,
					      CLAUSE RedClause)
/*********************************************************
  INPUT:   A proof search object, a list <Blocked> of clauses from
           the proof search object and a clause that causes the
	   derived clauses in <Blocked> to be redundant.
  RETURNS: Nothing.
**********************************************************/
{
  CLAUSE Clause;
  LIST   Scan;

  for (Scan = Blocked; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    Clause = (CLAUSE)list_Car(Scan);
    if (prfs_SplitLevelCondition(clause_SplitLevel(RedClause),clause_SplitLevel(Clause),
				 prfs_LastBacktrackLevel(Search))) {
      split_KeepClauseAtLevel(Search, Clause, clause_SplitLevel(RedClause));
    }
    else {
      if (flag_GetFlagIntValue(prfs_Store(Search), flag_DOCPROOF))
	prfs_InsertDocProofClause(Search, Clause);
      else
	clause_Delete(Clause);
    }
  }
}


void red_Init(void)
/*********************************************************
  INPUT:   None.
  RETURNS: Nothing.
  EFFECT:  Initializes the Reduction module, in particular
           its stampid to stamp terms and initalizes the
	  compute table for CRw.
**********************************************************/
{
  red_STAMPID = term_GetStampID();
  red_CRW_TERM_FAULT_CACHE = st_IndexCreate();
}

void red_Free(void)
/*********************************************************
 INPUT:   None.
 RETURNS: Nothing.
 EFFECT:  Frees memory allocated by module rules-red
	  in particular the compute table for CRw.
 **********************************************************/
{
  st_IndexDeleteWithElement(red_CRW_TERM_FAULT_CACHE);
}

static void red_DocumentObviousReductions(CLAUSE Clause, LIST Indexes)
/*********************************************************
  INPUT:   A clause and a list of literal indexes removed by
           obvious reductions.
  RETURNS: None
  MEMORY:  The <Indexes> list is consumed.
**********************************************************/
{
  list_Delete(clause_ParentClauses(Clause));
  list_Delete(clause_ParentLiterals(Clause));
  clause_SetParentClauses(Clause, list_Nil());
  clause_SetParentLiterals(Clause, Indexes);

  clause_AddParentClause(Clause,clause_Number(Clause));  /* Has to be done before increasing it! */

  clause_SetNumber(Clause, clause_IncreaseCounter());
  clause_SetFromObviousReductions(Clause);
}

static BOOL red_ObviousReductionsAux(CLAUSE Clause, BOOL Document, FLAGSTORE Flags,
    PRECEDENCE Precedence, CLAUSE *Changed, BOOL VarIsConst)
/**********************************************************
 INPUT:   A clause, a boolean flag for proof
	  documentation, a flag store and a precedence
	  and a flag.
 RETURNS: TRUE iff  obvious reductions are possible.
	  If <Document>  is false the clause is
	  destructively changed,
	  else a reduced copy of  the clause is returned
	  in <*Changed>.
 EFFECT:  Multiple occurrences of the same literal as
	  well as trivial equations are removed.
 CAUTION: if <VarIsConst> is set variables of <Clause>
	  are interpreted as constants
 ********************************************************/
{
  intptr_t i;
  int  j, end;
  LIST Indexes;
  TERM Atom, PartnerAtom;

#ifdef CHECK
  if(!VarIsConst)
  clause_Check(Clause, Flags, Precedence);
  else
  clause_CheckSkolem(Clause, Flags, Precedence);
#endif

  Indexes = list_Nil();
  end = clause_LastAntecedentLitIndex(Clause);

  for (i = clause_FirstConstraintLitIndex(Clause); i <= end; i++)
    {
      Atom = clause_LiteralAtom(clause_GetLiteral(Clause, i));
      if (fol_IsEquality(Atom) && !clause_LiteralIsOrientedEquality(
          clause_GetLiteral(Clause, i)) && term_Equal(term_FirstArgument(Atom),
          term_SecondArgument(Atom)))
        {
          Indexes = list_Cons((POINTER) i, Indexes);
        }
      else
        for (j = i + 1; j <= end; j++)
          {
            PartnerAtom = clause_LiteralAtom(clause_GetLiteral(Clause, j));
            if (term_Equal(PartnerAtom, Atom) || (fol_IsEquality(Atom)
                && fol_IsEquality(PartnerAtom) && term_Equal(
                term_FirstArgument(Atom), term_SecondArgument(PartnerAtom))
                && term_Equal(term_FirstArgument(PartnerAtom),
                    term_SecondArgument(Atom))))
              {
                Indexes = list_Cons((POINTER) i, Indexes);
                j = end;
              }
          }
    }

  end = clause_LastSuccedentLitIndex(Clause);

  for (i = clause_FirstSuccedentLitIndex(Clause); i <= end; i++)
    {
      Atom = clause_LiteralAtom(clause_GetLiteral(Clause, i));
      for (j = i + 1; j <= end; j++)
        {
          PartnerAtom = clause_LiteralAtom(clause_GetLiteral(Clause, j));
          if (term_Equal(PartnerAtom, Atom) || (fol_IsEquality(Atom)
              && fol_IsEquality(PartnerAtom) && term_Equal(term_FirstArgument(
              Atom), term_SecondArgument(PartnerAtom)) && term_Equal(
              term_FirstArgument(PartnerAtom), term_SecondArgument(Atom))))
            {
              Indexes = list_Cons((POINTER) i, Indexes);
              j = end;
            }
        }
    }

  if (clause_Length(Clause) == 1 && clause_NumOfAnteLits(Clause) == 1
      && !list_PointerMember(Indexes, (POINTER) clause_FirstAntecedentLitIndex(
          Clause)) && fol_IsEquality(clause_GetLiteralAtom(Clause,
      clause_FirstAntecedentLitIndex(Clause))))
    {
      cont_StartBinding();
      if (unify_UnifyCom(cont_LeftContext(), term_FirstArgument(
          clause_LiteralAtom(clause_GetLiteral(Clause,
              clause_FirstAntecedentLitIndex(Clause)))), term_SecondArgument(
          clause_LiteralAtom(clause_GetLiteral(Clause,
              clause_FirstAntecedentLitIndex(Clause))))))
        Indexes = list_Cons((POINTER) clause_FirstAntecedentLitIndex(Clause),
            Indexes);
      cont_BackTrack();
    }

  if (!list_Empty(Indexes)) {
    if (flag_GetFlagIntValue(Flags, flag_POBV)) {
      fputs("\nObvious: ", stdout);
      clause_Print(Clause);
      fputs(" ==> ", stdout);
    }      
    if (Document) {
      CLAUSE Copy;
      Copy = clause_Copy(Clause);
      clause_DeleteLiterals(Copy,Indexes, Flags, Precedence);
      red_DocumentObviousReductions(Copy,Indexes); /* Indexes is consumed */
      if (flag_GetFlagIntValue(Flags, flag_POBV))
	clause_Print(Copy);
      *Changed = Copy;
    }
    else {
      clause_DeleteLiterals(Clause,Indexes, Flags, Precedence);
      list_Delete(Indexes);
      if (flag_GetFlagIntValue(Flags, flag_POBV))
	clause_Print(Clause);
    }
    return TRUE;
  }

  return FALSE;
}

static BOOL red_ObviousReductions(CLAUSE Clause, BOOL Document, FLAGSTORE Flags,
				  PRECEDENCE Precedence, CLAUSE *Changed)
/**********************************************************
 INPUT:   A clause, a boolean flag for proof
	  documentation, a flag store and a precedence
 RETURNS: TRUE iff  obvious reductions are possible.
	  If <Document>  is false the clause is
	  destructively changed,
	  else a reduced copy of  the clause is returned
	  in <*Changed>.
 EFFECT:  Multiple occurrences of the same literal as
	  well as trivial equations are removed.
 ********************************************************/
{
  return red_ObviousReductionsAux(Clause, Document, Flags, Precedence, Changed,
      FALSE);
}

static BOOL red_ObviousReductionsSkolem(CLAUSE Clause, BOOL Document, FLAGSTORE Flags,
					PRECEDENCE Precedence, CLAUSE *Changed)
/**********************************************************
 INPUT:   A clause, a boolean flag for proof
	  documentation, a flag store and a precedence
	  a flag.
 RETURNS: TRUE iff  obvious reductions are possible.
	  If <Document>  is false the clause is
	  destructively changed,
	  else a reduced copy of  the clause is returned
	  in <*Changed>.
 EFFECT:  Multiple occurrences of the same literal as
	  well as trivial equations are removed.
 CAUTION: Variables of <Clause> are interpreted as
	  constants.
 ********************************************************/
{
  return red_ObviousReductionsAux(Clause, Document, Flags, Precedence, Changed,
      TRUE);
}


static void red_DocumentCondensing(CLAUSE Clause, LIST Indexes)
/*********************************************************
  INPUT:   A clause and a list of literal indexes removed by condensing.
  RETURNS: Nothing.
  MEMORY:  The <Indexes> list is consumed.
**********************************************************/
{
  list_Delete(clause_ParentClauses(Clause));
  list_Delete(clause_ParentLiterals(Clause));
  clause_SetParentClauses(Clause, list_Nil());
  clause_SetParentLiterals(Clause, Indexes);

  clause_AddParentClause(Clause,clause_Number(Clause));  /* Has to be done before increasing it! */

  clause_SetNumber(Clause, clause_IncreaseCounter());
  clause_SetFromCondensing(Clause);
}


static BOOL red_CondensingAux(CLAUSE Clause, BOOL Document, FLAGSTORE Flags,
			      PRECEDENCE Precedence, CLAUSE *Changed, BOOL VarIsConst)
/**********************************************************
 INPUT:   A non-empty unshared clause, a boolean flag
	  concerning proof documentation, a flag store and
	  a precedence.
 RETURNS: TRUE iff condensing is applicable to <Clause>.
	  If <Document> is false the clause is
	  destructively changed else a condensed copy of
	  the clause is returned in <*Changed>.
 CAUTION: If <VarIsConst> is set to TRUE then variables
	  are assumed to be constants
 ***********************************************************/
{
  LIST Indexes;

#ifdef CHECK
  if ((!VarIsConst && !clause_IsClause(Clause, Flags, Precedence)) ||
      (VarIsConst && !clause_IsClauseSkolem(Clause, Flags, Precedence)) ||
      (*Changed != (CLAUSE)NULL))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_Condensing : ");
      misc_ErrorReport("Illegal input.\n");
      misc_FinishErrorReport();
    }

  if(!VarIsConst)
  clause_Check(Clause, Flags, Precedence);
  else
  clause_CheckSkolem(Clause, Flags, Precedence);
#endif

  Indexes = cond_CondFast(Clause);

  if (!list_Empty(Indexes)) {
    if (flag_GetFlagIntValue(Flags, flag_PCON)) {
      fputs("\nCondensing: ", stdout);
      clause_Print(Clause);
      fputs(" ==> ", stdout);
    }
    if (Document) {
      CLAUSE Copy;
      Copy = clause_Copy(Clause);
      clause_DeleteLiterals(Copy, Indexes, Flags, Precedence);
      red_DocumentCondensing(Copy, Indexes);
      if (flag_GetFlagIntValue(Flags, flag_PCON))
	clause_Print(Copy);
      *Changed = Copy;
    }
    else {
      clause_DeleteLiterals(Clause, Indexes, Flags, Precedence);
      list_Delete(Indexes);
      if (flag_GetFlagIntValue(Flags, flag_PCON))
	clause_Print(Clause);
    }
    return TRUE;
  }
  return FALSE;
}

static BOOL red_Condensing(CLAUSE Clause, BOOL Document, FLAGSTORE Flags,
			   PRECEDENCE Precedence, CLAUSE *Changed)
/**********************************************************
 INPUT:   A non-empty unshared clause, a boolean flag
	  concerning proof documentation, a flag store and
	  a precedence.
 RETURNS: TRUE iff condensing is applicable to <Clause>.
	  If <Document> is false the clause is
	  destructively changed else a condensed copy of
	  the clause is returned in <*Changed>.
 ***********************************************************/
{
  return red_CondensingAux(Clause, Document, Flags, Precedence, Changed, FALSE);
}

static BOOL red_CondensingSkolem(CLAUSE Clause, BOOL Document, FLAGSTORE Flags,
				 PRECEDENCE Precedence, CLAUSE *Changed)
/**********************************************************
 INPUT:   A non-empty unshared clause, a boolean flag
	  concerning proof documentation, a flag store and
	  a precedence.
 RETURNS: TRUE iff condensing is applicable to <Clause>.
	  If <Document> is false the clause is
	  destructively changed else a condensed copy of
	  the clause is returned in <*Changed>.
 CAUTION: Variables are interpreted as constants
 ***********************************************************/
{
  return red_CondensingAux(Clause, Document, Flags, Precedence, Changed, TRUE);
}

static void red_DocumentAssignmentEquationDeletion(CLAUSE Clause, LIST Indexes,
						   NAT NonTrivClauseNumber)
/*********************************************************
  INPUT:   A clause and a list of literal indexes pointing to
           redundant equations and the clause number of a clause
	   implying a non-trivial domain.
  RETURNS: Nothing.
  MEMORY:  The <Indexes> list is consumed.
**********************************************************/
{
  list_Delete(clause_ParentClauses(Clause));
  list_Delete(clause_ParentLiterals(Clause));
  clause_SetParentClauses(Clause, list_Nil());
  clause_SetParentLiterals(Clause, Indexes);

  clause_AddParentClause(Clause,clause_Number(Clause));  /* Has to be done before increasing it! */

  clause_SetNumber(Clause, clause_IncreaseCounter());
  clause_SetFromAssignmentEquationDeletion(Clause);

  if (NonTrivClauseNumber != 0) {  /* Such a clause exists */
    clause_AddParentClause(Clause, NonTrivClauseNumber);
    clause_AddParentLiteral(Clause, 0); /* The non triv clause has exactly one negative literal */
  }
}


static BOOL red_AssignmentEquationDeletion(CLAUSE Clause, FLAGSTORE Flags,
					   PRECEDENCE Precedence, CLAUSE *Changed,
					   NAT NonTrivClauseNumber,
					   BOOL NonTrivDomain)
/**********************************************************
  INPUT:   A non-empty unshared clause, a flag store, a
           precedence, the clause number of a clause
	   implying a  non-trivial domain and a boolean
	   flag indicating whether the current domain has
	   more than one element.
  RETURNS: TRUE iff equations are removed.
           If the <DocProof> flag is false the clause is
	   destructively changed else a copy of the clause 
	   where redundant equations are removed is 
	   returned in <*Changed>.
***********************************************************/
{
  LIST Indexes;              /* List of indexes of redundant equations*/
  NAT  i;
  TERM LeftArg, RightArg;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence) ||
      (*Changed != (CLAUSE)NULL) ||
      (NonTrivDomain && NonTrivClauseNumber == 0) ||
      (!NonTrivDomain && NonTrivClauseNumber > 0)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_AssignmentEquationDeletion: ");
    misc_ErrorReport("Illegal input.\n");
    misc_FinishErrorReport();    
  }
  clause_Check(Clause, Flags, Precedence);
#endif

  Indexes = list_Nil();

  if (clause_ContainsNegativeEquations(Clause)) {
    for (i = clause_FirstAntecedentLitIndex(Clause); i <= clause_LastAntecedentLitIndex(Clause); i++) {
      if (clause_LiteralIsEquality(clause_GetLiteral(Clause,i))) {
	LeftArg  = term_FirstArgument(clause_GetLiteralAtom(Clause,i));
	RightArg = term_SecondArgument(clause_GetLiteralAtom(Clause,i));
	if ((term_IsVariable(LeftArg) &&
	     clause_NumberOfSymbolOccurrences(Clause, term_TopSymbol(LeftArg)) == 1) ||
	    (term_IsVariable(RightArg) &&
	     clause_NumberOfSymbolOccurrences(Clause, term_TopSymbol(RightArg)) == 1))
	  Indexes = list_Cons((POINTER)i, Indexes);
      }
    }
  }

  if (NonTrivDomain && clause_ContainsPositiveEquations(Clause)) {
    for (i = clause_FirstSuccedentLitIndex(Clause); i <= clause_LastSuccedentLitIndex(Clause); i++) {
      if (clause_LiteralIsEquality(clause_GetLiteral(Clause,i))) {
	LeftArg  = term_FirstArgument(clause_GetLiteralAtom(Clause,i));
	RightArg = term_SecondArgument(clause_GetLiteralAtom(Clause,i));
	if ((term_IsVariable(LeftArg) &&
	     clause_NumberOfSymbolOccurrences(Clause, term_TopSymbol(LeftArg)) == 1) ||
	    (term_IsVariable(RightArg) &&
	     clause_NumberOfSymbolOccurrences(Clause, term_TopSymbol(RightArg)) == 1))
	  Indexes = list_Cons((POINTER)i, Indexes);
      }
    }
  }

  if (!list_Empty(Indexes)) {
    if (flag_GetFlagIntValue(Flags, flag_PAED)) {
      fputs("\nAED: ", stdout);
      clause_Print(Clause);
      fputs(" ==> ", stdout);
    }
    if (flag_GetFlagIntValue(Flags, flag_DOCPROOF)) {
      CLAUSE Copy;
      Copy = clause_Copy(Clause);
      clause_DeleteLiterals(Copy, Indexes, Flags, Precedence);
      red_DocumentAssignmentEquationDeletion(Copy, Indexes, NonTrivClauseNumber);
      if (flag_GetFlagIntValue(Flags, flag_PAED))
	clause_Print(Copy);
      *Changed = Copy;
    }
    else {
      clause_DeleteLiterals(Clause, Indexes, Flags, Precedence);
      list_Delete(Indexes);
      if (flag_GetFlagIntValue(Flags, flag_PAED))
	clause_Print(Clause);
    }
    return TRUE;
  }

  return FALSE;
}

BOOL red_ExplicitTautology(CLAUSE Clause, FLAGSTORE Flags, 
				  PRECEDENCE Precedence)
/**********************************************************
  INPUT:   A non-empty clause, a flag store and a
           precedence.
  RETURNS: The boolean value TRUE if <Clause> contains
           true or not(false)
***********************************************************/
{
  int   i,la,n;
  BOOL  Result;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_ExplicitTautology :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(Clause, Flags, Precedence);
#endif
  
  la     = clause_LastAntecedentLitIndex(Clause);
  n      = clause_Length(Clause);
  Result = FALSE;

  for (i = clause_FirstSuccedentLitIndex(Clause); i < n && !Result;  i++) 
    if (symbol_Equal(term_TopSymbol(clause_LiteralAtom(clause_GetLiteral(Clause, i))),fol_True()))
      Result = TRUE;
    
  for (i = clause_FirstLitIndex(); i <= la && !Result; i++)
    if (symbol_Equal(term_TopSymbol(clause_LiteralAtom(clause_GetLiteral(Clause, i))),fol_False()))
      Result = TRUE;

  if (Result && flag_GetFlagIntValue(Flags, flag_PTAUT)) {
    fputs("\nTautology: ", stdout);
    clause_Print(Clause);
  }
  return Result;
}

static BOOL red_TautologyAux(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence,
			     BOOL VarIsConst)
/**********************************************************
 INPUT:   A non-empty clause, a flag store and a
	  precedence.
 RETURNS: The boolean value TRUE if 'Clause' is a
	  tautology.
 CAUTION: If <VarIsConst> is set to TRUE Variables are
	  interpreted as constants
 ***********************************************************/
{
  TERM Atom;
  int i, j, la, n;
  BOOL Result;

#ifdef CHECK
  if ((!VarIsConst && !clause_IsClause(Clause, Flags, Precedence))||
      (VarIsConst && !clause_IsClauseSkolem(Clause, Flags, Precedence)))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_Tautology :");
      misc_ErrorReport(" Illegal input.\n");
      misc_FinishErrorReport();
    }

  if(!VarIsConst)
  clause_Check(Clause, Flags, Precedence);
  else
  clause_CheckSkolem(Clause, Flags, Precedence);

#endif

  la = clause_LastAntecedentLitIndex(Clause);
  n = clause_Length(Clause);
  Result = FALSE;

  for (j = clause_FirstSuccedentLitIndex(Clause); j < n && !Result; j++)
    {

      Atom = clause_LiteralAtom(clause_GetLiteral(Clause, j));

      if (fol_IsEquality(Atom) && !clause_LiteralIsOrientedEquality(
          clause_GetLiteral(Clause, j)) && term_Equal(term_FirstArgument(Atom),
          term_SecondArgument(Atom)))
        Result = TRUE;

      for (i = clause_FirstLitIndex(); i <= la && !Result; i++)
        if (term_Equal(Atom, clause_LiteralAtom(clause_GetLiteral(Clause, i))))
          Result = TRUE;
    }

  if (!Result && flag_GetFlagIntValue(Flags, flag_RTAUT) == flag_RTAUTSEMANTIC
      && clause_NumOfAnteLits(Clause) != 0 && clause_NumOfSuccLits(Clause) != 0)
    {
      Result = cc_Tautology(Clause);
    }

  if (Result && flag_GetFlagIntValue(Flags, flag_PTAUT)) {
    fputs("\nTautology: ", stdout);
    clause_Print(Clause);
  }
  return Result;
}

static BOOL red_Tautology(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence)
/**********************************************************
 INPUT:   A non-empty clause, a flag store and a
	  precedence.
 RETURNS: The boolean value TRUE if 'Clause' is a
	  tautology.
 ***********************************************************/
{
  return red_TautologyAux(Clause, Flags, Precedence, FALSE);
}

static BOOL red_TautologySkolem(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence)
/**********************************************************
 INPUT:   A non-empty clause, a flag store and a
	  precedence.
 RETURNS: The boolean value TRUE if 'Clause' is a
	  tautology.
 CAUTION: Variables are interpreted as constants
 ***********************************************************/
{
  return red_TautologyAux(Clause, Flags, Precedence, TRUE);
}


static LITERAL red_GetMRResLit(LITERAL ActLit, SHARED_INDEX ShIndex)
/**************************************************************
  INPUT:   A literal and an Index.
  RETURNS: The most valid clause with a complementary literal,
           (CLAUSE)NULL, if no such clause exists.
***************************************************************/
{
  LITERAL NextLit;
  int     i;
  CLAUSE  ActClause;
  TERM    CandTerm;
  LIST    LitScan;

  NextLit   = (LITERAL)NULL;
  ActClause = clause_LiteralOwningClause(ActLit);
  i         = clause_LiteralGetIndex(ActLit);
  CandTerm  = st_ExistGen(cont_LeftContext(),
			  sharing_Index(ShIndex),
			  clause_LiteralAtom(ActLit));

  while (CandTerm) { /* First check units */
    if (!term_IsVariable(CandTerm)) { /* Has to be an Atom! */
      LitScan   = sharing_NAtomDataList(CandTerm); /* CAUTION !!! the List is not a copy */
      while (!list_Empty(LitScan)) {
	NextLit = list_Car(LitScan);
	if (clause_LiteralsAreComplementary(ActLit,NextLit))
	  if (clause_Length(clause_LiteralOwningClause(NextLit)) == 1 ||
	      subs_SubsumesBasic(clause_LiteralOwningClause(NextLit),ActClause,
				 clause_LiteralGetIndex(NextLit),i)) {
	    st_CancelExistRetrieval();
	    return NextLit;
	  }
	LitScan = list_Cdr(LitScan);
      }
    }
    CandTerm = st_NextCandidate();
  }
  return (LITERAL)NULL;
}

static void red_DocumentMatchingReplacementResolution(CLAUSE Clause, LIST LitInds,
						      LIST ClauseNums, LIST PLitInds)
/*********************************************************
  INPUT:   A clause, the involved literals indices in <Clause>,
           the literal indices of the reduction literals
	   and the clauses number.
  RETURNS: Nothing.
  MEMORY:  All input lists are consumed.
**********************************************************/
{
  LIST   Scan,Help;

  Help = list_Nil();

  for (Scan=LitInds; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    Help = list_Cons((POINTER)clause_Number(Clause), Help);
                                                /* Has to be done before increasing the clause number! */
  }
  list_Delete(clause_ParentClauses(Clause));
  list_Delete(clause_ParentLiterals(Clause));
  clause_SetParentClauses(Clause, list_Nconc(Help,ClauseNums));
  clause_SetParentLiterals(Clause, list_Nconc(LitInds,PLitInds));
  
  clause_SetNumber(Clause, clause_IncreaseCounter());
  clause_SetFromMatchingReplacementResolution(Clause);
}

static BOOL red_MatchingReplacementResolution(CLAUSE Clause, SHARED_INDEX ShIndex,
					      FLAGSTORE Flags, PRECEDENCE Precedence,
					      CLAUSE *Changed, int Level)
/**************************************************************
  INPUT:   A clause, an Index, a flag store, a precedence and a
           split level indicating the need of a copy if
	   <Clause> is reduced by a clause of higher split
	   level than <Level>.
  RETURNS: TRUE if reduction wrt the indexed clauses was
           possible.
           If the <DocProof> flag is true or the clauses used
	   for reductions have a higher split level then a
	   changed copy is returned in <*Changed>.
	   Otherwise <Clause> is destructively changed.
***************************************************************/
{
  CLAUSE  PClause,Copy;
  LITERAL ActLit,PLit;
  intptr_t     i, j; 
  int			length;
  LIST    ReducedBy,ReducedLits,PLits,Scan1,Scan2;
  BOOL    Document;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence) ||
      (*Changed != (CLAUSE)NULL)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_MatchingReplacementResolution:");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(Clause, Flags, Precedence);
#endif
  
  Copy        = Clause;
  length      = clause_Length(Clause);
  ReducedBy   = list_Nil();
  ReducedLits = list_Nil();
  PLits       = list_Nil();
  i           = clause_FirstLitIndex();
  j           = 0;
  Document    = flag_GetFlagIntValue(Flags, flag_DOCPROOF);
  
  while (i < length) {
    ActLit = clause_GetLiteral(Copy, i);
    
    if (!fol_IsEquality(clause_LiteralAtom(ActLit)) ||   /* Reduce with negative equations. */
	clause_LiteralIsPositive(ActLit)) {
      PLit = red_GetMRResLit(ActLit, ShIndex);
      if (clause_LiteralExists(PLit)) {
	if (list_Empty(PLits) && flag_GetFlagIntValue(Flags, flag_PMRR)) {
	  fputs("\nFMatchingReplacementResolution: ", stdout);
	  clause_Print(Copy);
	}
	PClause     = clause_LiteralOwningClause(PLit);
	ReducedBy   = list_Cons((POINTER)clause_Number(PClause), ReducedBy);
	PLits       = list_Cons((POINTER)clause_LiteralGetIndex(PLit),PLits);
	ReducedLits = list_Cons((POINTER)(i+j), ReducedLits);
	if (Copy == Clause &&
	    (Document || prfs_SplitLevelCondition(clause_SplitLevel(PClause),clause_SplitLevel(Copy),Level)))
	  Copy = clause_Copy(Clause);
	clause_UpdateSplitDataFromPartner(Copy, PClause);	  
	clause_DeleteLiteral(Copy,i, Flags, Precedence);
	length--;
	j++;
      }
      else
	i++;
    }
    else
      i++;
  }
  
  if (!list_Empty(ReducedBy)) {
    if (Document) {
      ReducedBy   = list_NReverse(ReducedBy);
      ReducedLits = list_NReverse(ReducedLits);
      PLits       = list_NReverse(PLits);
      red_DocumentMatchingReplacementResolution(Copy, ReducedLits, ReducedBy, PLits); /* Lists are consumed */
      if (flag_GetFlagIntValue(Flags, flag_PMRR)) {
	fputs(" ==> [ ", stdout);
	for(Scan1=ReducedBy,Scan2=ReducedLits;!list_Empty(Scan1);
	    Scan1=list_Cdr(Scan1),Scan2=list_Cdr(Scan2))
	  printf("%zu.%zu ",(NAT)list_Car(Scan1),(NAT)list_Car(Scan2));
	fputs("] ", stdout);
	clause_Print(Copy);
      }
    }
    else {
      if (flag_GetFlagIntValue(Flags, flag_PMRR)) {
	fputs(" ==> [ ", stdout);
	for(Scan1=ReducedBy,Scan2=ReducedLits;!list_Empty(Scan1);
	    Scan1=list_Cdr(Scan1),Scan2=list_Cdr(Scan2))
	  printf("%zu.%zu ",(NAT)list_Car(Scan1),(NAT)list_Car(Scan2));
	fputs("] ", stdout);
	clause_Print(Copy);
      }
      list_Delete(ReducedBy);
      list_Delete(ReducedLits);
      list_Delete(PLits);
    }
    if (Copy != Clause)
      *Changed = Copy;
    return TRUE;
  }
  return FALSE;
}

static void red_DocumentUnitConflict(CLAUSE Clause, LIST LitInds,
				     LIST ClauseNums, LIST PLitInds)
/*********************************************************
  INPUT:   A clause, the involved literals indices and the clauses number.
  RETURNS: Nothing.
  MEMORY:  All input lists are consumed.
**********************************************************/
{
  list_Delete(clause_ParentClauses(Clause));
  list_Delete(clause_ParentLiterals(Clause));
  clause_SetParentClauses(Clause, list_Nconc(list_List((POINTER)clause_Number(Clause)),ClauseNums));
  clause_SetParentLiterals(Clause, list_Nconc(LitInds,PLitInds));
  
  clause_SetNumber(Clause, clause_IncreaseCounter());
  clause_SetFromUnitConflict(Clause);
}


static BOOL red_UnitConflict(CLAUSE Clause, SHARED_INDEX ShIndex,
			     FLAGSTORE Flags, PRECEDENCE Precedence,
			     CLAUSE *Changed, int Level)
/**************************************************************
  INPUT:   A clause, an Index, a flag store  and a splitlevel
           indicating the need of a copy if <Clause> is reduced
	   by a clause of higher split level than <Level>.
  RETURNS: TRUE if a unit conflict with <Clause> and the 
           clauses in <ShIndex> happened.
           If the <DocProof> flag is true or the clauses used for 
	   reductions have a higher split level then a changed 
	   copy is returned in <*Changed>.
	   Otherwise <Clause> is destructively changed.
***************************************************************/
{
  CLAUSE  PClause,Copy;
  LITERAL ActLit,PLit;
  LIST    Scan;
  TERM    CandTerm;
  BOOL    Document;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence) || (*Changed != (CLAUSE)NULL)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_ForwardUnitConflict :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(Clause, Flags, Precedence);
#endif
  
  if (clause_Length(Clause) == 1) {
    Copy     = Clause;
    Document = flag_GetFlagIntValue(Flags, flag_DOCPROOF);
    ActLit   = clause_GetLiteral(Copy, clause_FirstLitIndex());
    PLit     = (LITERAL)NULL;
    CandTerm = st_ExistUnifier(cont_LeftContext(), sharing_Index(ShIndex), cont_RightContext(),
			       clause_LiteralAtom(ActLit));
    while (PLit == (LITERAL)NULL && CandTerm) {
      if (!term_IsVariable(CandTerm)) {
	Scan = sharing_NAtomDataList(CandTerm); /* CAUTION !!! the List is not a copy */
	while (!list_Empty(Scan)) {
	  PLit = list_Car(Scan);
	  if (clause_LiteralsAreComplementary(ActLit,PLit) &&
	      clause_Length(clause_LiteralOwningClause(PLit)) == 1) {
	    st_CancelExistRetrieval();
	    Scan = list_Nil();
	  }
	  else {
	    PLit = (LITERAL)NULL;
	    Scan = list_Cdr(Scan);
	  }
	}
      }
      if (PLit == (LITERAL)NULL)
	CandTerm = st_NextCandidate();
    }

    if (PLit == (LITERAL)NULL && fol_IsEquality(clause_LiteralAtom(ActLit))) {
      TERM Atom;
      Atom = term_Create(fol_Equality(),list_Reverse(term_ArgumentList(clause_LiteralAtom(ActLit))));
      CandTerm = st_ExistUnifier(cont_LeftContext(), sharing_Index(ShIndex), cont_RightContext(), Atom);
      while (PLit == (LITERAL)NULL && CandTerm) {
	if (!term_IsVariable(CandTerm)) {
	  Scan = sharing_NAtomDataList(CandTerm); /* CAUTION !!! the List is not a copy */
	  while (!list_Empty(Scan)) {
	    PLit = list_Car(Scan);
	    if (clause_LiteralsAreComplementary(ActLit,PLit) &&
		clause_Length(clause_LiteralOwningClause(PLit)) == 1) {
	      st_CancelExistRetrieval();
	      Scan = list_Nil();
	    }
	    else {
	      PLit = (LITERAL)NULL;
	      Scan = list_Cdr(Scan);
	    }
	  }
	}
	if (PLit == (LITERAL)NULL)
	  CandTerm = st_NextCandidate();
      }
      list_Delete(term_ArgumentList(Atom));
      term_Free(Atom);
    }
      
    if (clause_LiteralExists(PLit)) {
      if (flag_GetFlagIntValue(Flags, flag_PUNC)) {
	fputs("\nUnitConflict: ", stdout);
	clause_Print(Copy);
      }
      PClause     = clause_LiteralOwningClause(PLit);
      if (Copy == Clause &&
	  (Document || prfs_SplitLevelCondition(clause_SplitLevel(PClause),clause_SplitLevel(Copy),Level)))
	Copy = clause_Copy(Clause);
      clause_UpdateSplitDataFromPartner(Copy, PClause);	  
      clause_DeleteLiteral(Copy,clause_FirstLitIndex(), Flags, Precedence);
      if (Document) 
	red_DocumentUnitConflict(Copy, list_List((POINTER)clause_FirstLitIndex()), 
				 list_List((POINTER)clause_Number(PClause)), 
				 list_List((POINTER)clause_FirstLitIndex()));
      if (flag_GetFlagIntValue(Flags, flag_PUNC)) {
	printf(" ==> [ %zd.%zd ]", clause_Number(PClause), clause_FirstLitIndex());	
	clause_Print(Copy);
      }
      if (Copy != Clause)
	*Changed = Copy;
      return TRUE;
    }
  }
  return FALSE;
}


static CLAUSE red_ForwardSubsumerAux(CLAUSE RedCl, SHARED_INDEX ShIndex, FLAGSTORE Flags,
				     PRECEDENCE Precedence, LIST Blocked, BOOL VarIsConst)
/**********************************************************
 INPUT:   A pointer to a non-empty clause, an index of
	  clauses, a flag store, a precedence and a list
	  of blocked clauses
 RETURNS: A clause that subsumes <RedCl>, or NULL if no such
	  clause exists and is not member of <Blocked>
 ***********************************************************/
{
  TERM Atom, AtomGen;
  CLAUSE CandCl;
  LITERAL CandLit;
  LIST LitScan;
  int i, lc, fa, la, fs, ls;

#ifdef CHECK
  if ((!VarIsConst && !clause_IsClause(RedCl, Flags, Precedence)) ||
      (VarIsConst && !clause_IsClauseSkolem(RedCl, Flags, Precedence)))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_ForwardSubsumer:");
      misc_ErrorReport(" Illegal input.\n");
      misc_FinishErrorReport();
    }

  if(!VarIsConst)
  clause_Check(RedCl, Flags, Precedence);
  else
  clause_CheckSkolem(RedCl, Flags, Precedence);
#endif

  lc = clause_LastConstraintLitIndex(RedCl);
  fa = clause_FirstAntecedentLitIndex(RedCl);
  la = clause_LastAntecedentLitIndex(RedCl);
  fs = clause_FirstSuccedentLitIndex(RedCl);
  ls = clause_LastSuccedentLitIndex(RedCl);

  for (i = 0; i <= ls; i++)
    {
      Atom = clause_GetLiteralAtom(RedCl, i);
      AtomGen = st_ExistGen(cont_LeftContext(), sharing_Index(ShIndex), Atom);

      while (AtomGen)
        {
          if (!term_IsVariable(AtomGen))
            {
              for (LitScan = sharing_NAtomDataList(AtomGen); !list_Empty(
                  LitScan); LitScan = list_Cdr(LitScan))
                {
                  CandLit = list_Car(LitScan);
                  CandCl = clause_LiteralOwningClause(CandLit);

                  if (CandCl != RedCl && clause_GetLiteral(CandCl,
                      clause_FirstLitIndex()) == CandLit &&
                  /* <CandCl> is not blocked */
                  !list_PointerMember(Blocked, CandCl) &&
                  /* Literals must be from same part of the clause */
                  ((i <= lc && clause_LiteralIsFromConstraint(CandLit)) || (i
                      >= fa && i <= la && clause_LiteralIsFromAntecedent(
                      CandLit)) || (i >= fs && clause_LiteralIsFromSuccedent(
                      CandLit))) && subs_SubsumesBasic(CandCl, RedCl,
                      clause_FirstLitIndex(), i))
                    {
                      st_CancelExistRetrieval();
                      return (CandCl);
                    }
                }
            }
          AtomGen = st_NextCandidate();
        }

      if (fol_IsEquality(Atom) && clause_LiteralIsNotOrientedEquality(
          clause_GetLiteral(RedCl, i)))
        {
          Atom = term_Create(fol_Equality(), list_Reverse(term_ArgumentList(
              Atom)));
          AtomGen = st_ExistGen(cont_LeftContext(), sharing_Index(ShIndex),
              Atom);
          while (AtomGen)
            {
              if (!term_IsVariable(AtomGen))
                {
                  for (LitScan = sharing_NAtomDataList(AtomGen); !list_Empty(
                      LitScan); LitScan = list_Cdr(LitScan))
                    {
                      CandLit = list_Car(LitScan);
                      CandCl = clause_LiteralOwningClause(CandLit);
                      if (CandCl != RedCl && clause_GetLiteral(CandCl,
                          clause_FirstLitIndex()) == CandLit &&
                      /* <CandCl> is not blocked */
                      !list_PointerMember(Blocked, CandCl) &&
                      /* Literals must be from same part of the clause */
                      ((i <= lc && clause_LiteralIsFromConstraint(CandLit))
                          || (i >= fa && i <= la
                              && clause_LiteralIsFromAntecedent(CandLit)) || (i
                          >= fs && clause_LiteralIsFromSuccedent(CandLit)))
                          && subs_SubsumesBasic(CandCl, RedCl,
                              clause_FirstLitIndex(), i))
                        {
                          st_CancelExistRetrieval();
                          list_Delete(term_ArgumentList(Atom));
                          term_Free(Atom);
                          return (CandCl);
                        }
                    }
                }
              AtomGen = st_NextCandidate();
            }
          list_Delete(term_ArgumentList(Atom));
          term_Free(Atom);
        }
    }

  return ((CLAUSE) NULL);
}

static CLAUSE red_ForwardSubsumer(CLAUSE RedCl, SHARED_INDEX ShIndex, FLAGSTORE Flags,
				  PRECEDENCE Precedence, LIST Blocked)
/**********************************************************
 INPUT:   A pointer to a non-empty clause, an index of
	  clauses, a flag store, a precedence and a list
	  of blocked clauses
 RETURNS: A clause that subsumes <RedCl>, or NULL if no such
	  clause exists and is not member of <Blocked>
 ***********************************************************/
{
  return red_ForwardSubsumerAux(RedCl, ShIndex, Flags, Precedence, Blocked,
      FALSE);
}

static CLAUSE red_ForwardSubsumerSkolem(CLAUSE RedCl, SHARED_INDEX ShIndex, FLAGSTORE Flags,
					PRECEDENCE Precedence, LIST Blocked)
/**********************************************************
 INPUT:   A pointer to a non-empty clause, an index of
	  clauses, a flag store, a precedence and a list
	  of blocked clauses
 RETURNS: A clause that subsumes <RedCl>, or NULL if no such
	  clause exists and is not member of <Blocked>
 CAUTION: Variables are interpreted as constants
 ***********************************************************/
{
  return red_ForwardSubsumerAux(RedCl, ShIndex, Flags, Precedence, Blocked,
      TRUE);
}


static CLAUSE red_ForwardSubsumption(CLAUSE RedClause, SHARED_INDEX ShIndex, 
				     FLAGSTORE Flags, PRECEDENCE Precedence)
/**********************************************************
  INPUT:   A clause, an index of clauses, a flag store and
           a precedence.
  RETURNS: The clause <RedClause> is subsumed by in <ShIndex>.
***********************************************************/
{ 
  CLAUSE Subsumer;

#ifdef CHECK
  if (!clause_IsClause(RedClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_ForwardSubsumption:");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RedClause, Flags, Precedence);
#endif

  Subsumer = red_ForwardSubsumer(RedClause, ShIndex, Flags, Precedence, list_Nil());

  if (flag_GetFlagIntValue(Flags, flag_PSUB) && Subsumer) {
    fputs("\nFSubsumption: ", stdout);
    clause_Print(RedClause);
    printf(" by %zd %zd ",clause_Number(Subsumer),clause_SplitLevel(Subsumer));
  }

  return Subsumer;
}


static void red_DocumentRewriting(CLAUSE Clause, intptr_t i, CLAUSE Rule, int ri)
/*********************************************************
  INPUT:   Two clauses and the literal indices involved in the rewrite step.
  RETURNS: Nothing.
  EFFECT:  Documentation in <Clause> is set.
**********************************************************/
{
  list_Delete(clause_ParentClauses(Clause));
  list_Delete(clause_ParentLiterals(Clause));
  clause_SetParentClauses(Clause,
			  list_List((POINTER)clause_Number(Clause)));
  /* Has to be done before increasing the number! */

  clause_SetParentLiterals(Clause, list_List((POINTER)i));
  clause_NewNumber(Clause);
  clause_SetFromRewriting(Clause);

  clause_AddParentClause(Clause,clause_Number(Rule));
  clause_AddParentLiteral(Clause,ri);
}


static void red_DocumentFurtherRewriting(CLAUSE Clause, int i, CLAUSE Rule, int ri)
/*********************************************************
  INPUT:   Two clauses and the literal indices involved in the rewrite step.
  RETURNS: Nothing.
  EFFECT:  Documentation in <Clause> is set.
**********************************************************/
{
  clause_AddParentClause(Clause,
			 (intptr_t) list_Car(list_Cdr(clause_ParentClauses(Clause))));
  clause_AddParentLiteral(Clause, i);
  clause_AddParentClause(Clause, clause_Number(Rule));
  clause_AddParentLiteral(Clause, ri);
}


static BOOL red_RewriteRedUnitClause(CLAUSE RedClause, SHARED_INDEX ShIndex,
				     FLAGSTORE Flags, PRECEDENCE Precedence,
				     CLAUSE *Changed, int Level)
/**************************************************************
  INPUT:   A unit (!) clause, an Index, a flag store, a
           precedence and a split level indicating the need of
	   a copy  if <Clause> is reduced by a clause of higher
	   split level than <Level>.
  RETURNS: TRUE iff rewriting was possible.
           If the <DocProof> flag is true or the split level of
	   the rewrite rule is higher a copy of RedClause that
	   is rewritten wrt. the indexed clauses is returned in 
	   <*Changed>.
           Otherwise the clause is destructively rewritten.
***************************************************************/
{
  TERM   RedAtom, RedTermS;
  int    B_Stack;
  BOOL   Rewritten, Result, Oriented, Renamed, Document;
  TERM   TermS,PartnerEq;
  LIST   EqList,EqScan,LitScan;
  CLAUSE Copy;

#ifdef CHECK
  if (!clause_IsClause(RedClause, Flags, Precedence) ||
      *Changed != (CLAUSE)NULL ||
      clause_Length(RedClause) != 1) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_RewriteRedUnitClause :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RedClause, Flags, Precedence);
#endif

  Result    = FALSE;
  Renamed   = FALSE;
  Copy      = RedClause;
  RedAtom   = clause_GetLiteralAtom(Copy, clause_FirstLitIndex());
  Rewritten = TRUE;
  Document  = flag_GetFlagIntValue(Flags, flag_DOCPROOF);    

  /* Don't apply this rule on constraint or propositional literals */
  if (clause_FirstLitIndex() <= clause_LastConstraintLitIndex(RedClause) ||
      list_Empty(term_ArgumentList(RedAtom)))
    return Result;

  if (term_StampOverflow(red_STAMPID))
    term_ResetTermStamp(clause_LiteralSignedAtom(clause_GetLiteral(RedClause, clause_FirstLitIndex())));
  term_StartStamp();

  while (Rewritten) {
    Rewritten = FALSE;
    B_Stack = stack_Bottom();
    sharing_PushListOnStackNoStamps(term_ArgumentList(RedAtom));
    
    while (!stack_Empty(B_Stack)) {
      RedTermS = (TERM)stack_PopResult();
      TermS    = st_ExistGen(cont_LeftContext(), sharing_Index(ShIndex), RedTermS);
      while (TermS && !Rewritten) {
	EqList = term_SupertermList(TermS);
	for (EqScan = EqList; !list_Empty(EqScan) && !Rewritten;
	     EqScan = list_Cdr(EqScan)) {
	  PartnerEq = list_Car(EqScan);
	  if (fol_IsEquality(PartnerEq)) {
	    CLAUSE  RewriteClause;
	    LITERAL RewriteLit;
	    TERM    Right;

	    if (TermS == term_FirstArgument(PartnerEq))
	      Right = term_SecondArgument(PartnerEq);
	    else
	      Right = term_FirstArgument(PartnerEq);

	    for (LitScan = sharing_NAtomDataList(PartnerEq); 
		 !list_Empty(LitScan) && !Rewritten;
		 LitScan = list_Cdr(LitScan)) {
	      RewriteLit    = list_Car(LitScan);
	      RewriteClause = clause_LiteralOwningClause(RewriteLit);
	      if (clause_LiteralIsPositive(RewriteLit) &&
		  clause_Length(RewriteClause) == 1) {
		Oriented = (clause_LiteralIsOrientedEquality(RewriteLit) && 
			    TermS == term_FirstArgument(PartnerEq));
		if (!Oriented && !clause_LiteralIsOrientedEquality(RewriteLit)) { 
		  Renamed = TRUE;                  /* If oriented, no renaming needed! */
		  term_StartMaxRenaming(clause_MaxVar(RewriteClause));
		  term_Rename(RedAtom); /* Renaming destructive, no extra match needed !! */
		  Oriented = ord_ContGreater(cont_LeftContext(), TermS,
					     cont_LeftContext(), Right,
					     Flags, Precedence);
		  
		  /*if (Oriented) {
		    fputs("\n\n\tRedAtom: ",stdout);term_PrintPrefix(RedAtom);
		    fputs("\n\tSubTerm: ",stdout);term_PrintPrefix(RedTermS);
		    fputs("\n\tGenTerm: ",stdout);term_PrintPrefix(TermS);
		    fputs("\n\tGenRight: ",stdout);term_PrintPrefix(Right);
		    putchar('\n');cont_PrintCurrentTrail();putchar('\n');
		    }*/
		}
		if (Oriented) {
		  TERM   TermT;
		  
		  if (RedClause == Copy &&
		      (Document || 
		       prfs_SplitLevelCondition(clause_SplitLevel(RewriteClause),
						clause_SplitLevel(RedClause),Level))) {
		    Copy    = clause_Copy(RedClause);
		    RedAtom = clause_GetLiteralAtom(Copy, clause_FirstLitIndex());
		  }
		  
		  if (!Result)
		    if (flag_GetFlagIntValue(Flags, flag_PREW)) {
		      fputs("\nFRewriting: ", stdout);
		      clause_Print(Copy);
		      fputs(" ==>[ ", stdout);
		    }
		  
		  if (Document) {
		    if (!Result)
		      red_DocumentRewriting(Copy, clause_FirstLitIndex(),
					    RewriteClause, clause_FirstLitIndex());
		    else
		      red_DocumentFurtherRewriting(Copy, clause_FirstLitIndex(), 
						   RewriteClause, clause_FirstLitIndex());
		  }
		  Result = TRUE;
		  TermT  = cont_ApplyBindingsModuloMatching(cont_LeftContext(), term_Copy(Right), TRUE);
		  if (cont_BindingsAreRenamingModuloMatching(cont_RightContext()))
		    term_SetTermSubtermStamp(TermT);
		  term_ReplaceSubtermBy(RedAtom, RedTermS, TermT);
		  Rewritten = TRUE;
		  clause_UpdateSplitDataFromPartner(Copy, RewriteClause);
		  term_Delete(TermT);
		  stack_SetBottom(B_Stack);
		  
		  if (flag_GetFlagIntValue(Flags, flag_PREW))
		    printf("%zd.%zd ",clause_Number(RewriteClause), clause_FirstLitIndex());
		  clause_UpdateWeight(Copy, Flags);
		}
	      }
	    }
	  }
	}
	if (!Rewritten)
	  TermS = st_NextCandidate();
      }
      st_CancelExistRetrieval();
      if (!Rewritten)
	term_SetTermStamp(RedTermS);
    }
  }
  term_StopStamp(); 

  if (Result) {
    clause_PrecomputeOrderingAndReInit(Copy, Flags, Precedence);
    if (Copy != RedClause)
      clause_PrecomputeOrderingAndReInit(RedClause, Flags, Precedence);
    if (flag_GetFlagIntValue(Flags, flag_PREW)) {
      fputs("] ", stdout);
      clause_Print(Copy);
    }
    if (Copy != RedClause)
      *Changed = Copy;
  }
  else
    if (Renamed)
      clause_PrecomputeOrderingAndReInit(Copy, Flags, Precedence);
	
      
#ifdef CHECK
  clause_Check(Copy, Flags, Precedence);
  clause_Check(RedClause, Flags, Precedence);
#endif
  
  return Result;
}


static BOOL red_RewriteRedClause(CLAUSE RedClause, SHARED_INDEX ShIndex,
				 FLAGSTORE Flags, PRECEDENCE Precedence,
				 CLAUSE *Changed, int Level)
/**************************************************************
  INPUT:   A clause, an Index, a flag store, a precedence and
           a split level indicating the need of a copy if
	   <Clause> is reduced by a clause of higher split
	   level than <Level>.
  RETURNS: NULL, if no rewriting was possible.
           If the <DocProof> flag is true or the split level of
	   the rewrite rule is higher a copy of RedClause
	   that is rewritten wrt. the indexed clauses.
	   Otherwise the clause is destructively rewritten and
	   returned.
***************************************************************/
{
  TERM   RedAtom, RedTermS;
  int    B_Stack;
  int    ci, length;
  BOOL   Rewritten, Result, Document, Oriented;
  TERM   TermS,PartnerEq;
  LIST   EqScan,LitScan;
  CLAUSE Copy;

#ifdef CHECK
  if (!clause_IsClause(RedClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_RewriteRedClause :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RedClause, Flags, Precedence);
#endif
  
  length   = clause_Length(RedClause);
  Document = flag_GetFlagIntValue(Flags, flag_DOCPROOF); 

  if (length == 1)
    return red_RewriteRedUnitClause(RedClause, ShIndex, Flags, Precedence,
				    Changed, Level);

  Result = FALSE;
  Copy   = RedClause;

  /* Don't apply this rule on constraint literals! */
  for (ci = clause_FirstAntecedentLitIndex(RedClause); ci < length; ci++) {
    Rewritten  = TRUE;
    if (!list_Empty(term_ArgumentList(clause_GetLiteralAtom(Copy, ci)))) {
      while (Rewritten) {
	Rewritten = FALSE;
	RedAtom   = clause_GetLiteralAtom(Copy, ci);

	B_Stack = stack_Bottom();
	/* push subterms on stack except variables */
	sharing_PushListReverseOnStack(term_ArgumentList(RedAtom));

	while (!stack_Empty(B_Stack))  {
	  RedTermS = (TERM)stack_PopResult();
	  TermS    = st_ExistGen(cont_LeftContext(), sharing_Index(ShIndex), RedTermS);

	  while (TermS && !Rewritten) {
	    /* A variable can't be greater than any other term, */
	    /* so don't consider any variables here             */
            if (!term_IsVariable(TermS)) {
	      EqScan = term_SupertermList(TermS);

	      for ( ; !list_Empty(EqScan) && !Rewritten;
		    EqScan = list_Cdr(EqScan)) {
		PartnerEq = list_Car(EqScan);
                if (fol_IsEquality(PartnerEq)) {
		  CLAUSE  RewriteClause;
		  LITERAL RewriteLit;
                  TERM Right;
		  int     ri;
		 
                  /* determine the right side of the equation? */
                  if(TermS == term_FirstArgument(PartnerEq))
                        Right = term_SecondArgument(PartnerEq);
                  else
                        Right = term_FirstArgument(PartnerEq);

                  
		  for (LitScan = sharing_NAtomDataList(PartnerEq); 
		       !list_Empty(LitScan) && !Rewritten;
		       LitScan = list_Cdr(LitScan)) {
		    RewriteLit    = list_Car(LitScan);
		    RewriteClause = clause_LiteralOwningClause(RewriteLit);
		    ri            = clause_LiteralGetIndex(RewriteLit);
		   
                    if(clause_LiteralIsPositive(RewriteLit)) {
                      Oriented = (term_FirstArgument(PartnerEq) == TermS &&
			  	  clause_LiteralIsOrientedEquality(RewriteLit));

                      
                      if(!Oriented && !clause_LiteralIsOrientedEquality(RewriteLit) &&
                      /* check if vars(TermS) \superset of vars(Right), otherwise
                       * no orientation is possible; this prevents variable capturing */
                                      ord_CompareVarsSubset(TermS, Right)) {

                         Oriented = ord_ContGreater(cont_LeftContext(), TermS,
					     cont_LeftContext(), Right,
					     Flags, Precedence);
		         
                      }
                      
		      if (Oriented  &&
			  subs_SubsumesBasic(RewriteClause, Copy, ri, ci)) {

                        TERM   TermT;
		      
		        if (RedClause == Copy &&
			  (Document || 
			   prfs_SplitLevelCondition(clause_SplitLevel(RewriteClause),
						    clause_SplitLevel(RedClause),Level))) {
			  Copy    = clause_Copy(RedClause);
			  RedAtom = clause_GetLiteralAtom(Copy, ci);
		        }
		      
		        if (!Result) {
			  if (flag_GetFlagIntValue(Flags, flag_PREW)) {
			    fputs("\nFRewriting: ", stdout);
			    clause_Print(Copy);
			    fputs(" ==>[ ", stdout);
			  }
		        }
		      
		        if (Document) {
			  if (!Result)
			    red_DocumentRewriting(Copy, ci, RewriteClause, ri);
			  else
			    red_DocumentFurtherRewriting(Copy,ci,RewriteClause,ri);
		        }
		        Result = TRUE;
		        /* Since <TermS> is the bigger term of an oriented   */
		        /* equation and all variables in <TermS> are bound,  */
		        /* all variables in the smaller term are bound, too. */
		        /* So the strict version of cont_Apply... will work. */
			/*cont_PrintCurrentTrail();	*/
		        TermT  = cont_ApplyBindingsModuloMatching(cont_LeftContext(),
								term_Copy(Right),
								TRUE);

		        /* No variable renaming is necessary before creation   */
		        /* of bindings and replacement of subterms because all */
		        /* variables of <TermT> are from <RedClause>/<Copy>.   */
		        term_ReplaceSubtermBy(RedAtom, RedTermS, TermT);			
		        Rewritten = TRUE;
		        clause_UpdateSplitDataFromPartner(Copy,RewriteClause);
		        term_Delete(TermT);
		        stack_SetBottom(B_Stack);
		      
		        if (flag_GetFlagIntValue(Flags, flag_PREW))
			  printf("%zd.%d ",clause_Number(RewriteClause), ri);
		        clause_UpdateWeight(Copy, Flags);
                      }
		    }
		  } /* for */
	        }
	      }/* for */
            }
	    if (!Rewritten)
	      TermS = st_NextCandidate();
	  }
	  st_CancelExistRetrieval();
	}
      }
    }
  }
  if (Result) {
    clause_PrecomputeOrderingAndReInit(Copy, Flags, Precedence);
    if (flag_GetFlagIntValue(Flags, flag_PREW)) {
      fputs("] ", stdout);
      clause_Print(Copy);
    }
    if (Copy != RedClause) {
      clause_PrecomputeOrderingAndReInit(RedClause, Flags, Precedence);
      *Changed = Copy;
    }
  }

#ifdef CHECK
  clause_Check(Copy, Flags, Precedence);
  clause_Check(RedClause, Flags, Precedence);
#endif

  return Result;
}


/**************************************************************/
/* FORWARD CONTEXTUAL REWRITING                               */
/**************************************************************/

static BOOL red_LeftTermOfEquationIsStrictlyMaximalTerm(CLAUSE Clause,
							LITERAL Equation,
							FLAGSTORE Flags,
							PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a literal of the clause, that is an
           oriented equation, a flag store and a precedence.
  RETURNS: TRUE, iff the bigger (i.e. left) term of the equation
           is the strictly maximal term of the clause.
	   A term s is strictly maximal in a clause, iff for every atom
	   u=v (A=tt) of the clause s > u and s > v (s > A).
***************************************************************/
{
  int     i, except, last;
  TERM    LeftTerm, Atom;
  LITERAL ActLit;

#ifdef CHECK
  if (!clause_LiteralIsOrientedEquality(Equation)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_LeftTermOfEquationIsStrictlyMaximalTerm: ");
    misc_ErrorReport("literal is not oriented");
    misc_FinishErrorReport();
  }
#endif

  LeftTerm = term_FirstArgument(clause_LiteralSignedAtom(Equation));
  except   = clause_LiteralGetIndex(Equation);

  /* Compare <LeftTerm> with all terms in the clause */
  last = clause_LastLitIndex(Clause);
  for (i = clause_FirstLitIndex() ; i <= last; i++) {
    if (i != except) {
      ActLit = clause_GetLiteral(Clause, i);
      Atom = clause_LiteralAtom(ActLit);
      if (fol_IsEquality(Atom)) {
	/* Atom is an equation */
	if (ord_Compare(LeftTerm, term_FirstArgument(Atom), Flags, Precedence)
	    != ord_GREATER_THAN ||
	    (!clause_LiteralIsOrientedEquality(ActLit) &&
	     ord_Compare(LeftTerm, term_SecondArgument(Atom), Flags, Precedence)
	     != ord_GREATER_THAN))
	  /* Compare only with left (i.e. greater) subterm if the atom is */
	  /* an oriented equation. */
	  return FALSE;
      } else {
	/* Atom is not an equation */
	if (ord_Compare(LeftTerm, Atom, Flags, Precedence) != ord_GREATER_THAN)
	  return FALSE;
      }
    }
  }
  return TRUE;
}

static BOOL red_TermIsStrictlyMaximalTermSkolem(CLAUSE Clause, int except, TERM Term,
						FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
 INPUT:   A clause, a literal index of the clause and a term,
	  a flag store and a precedence.
 RETURNS: TRUE, iff the bigger (i.e. left) term of the equation
	  is the strictly maximal term of the clause.
	  A term s is strictly maximal in a clause, iff for every atom
	  u=v (A=tt) of the clause s > u and s > v (s > A).
 CAUTION: It is assumed that Literal <except> of <Clause> is
	  an equation, and <Term> is the bigger term of
	  this equation
 ***************************************************************/
{
  int i, last;
  TERM Atom;
  LITERAL ActLit;

#ifdef CHECK
  if(!clause_LiteralIsEquality(clause_GetLiteral(Clause, except)))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_TermIsStrictlyMaximalTermSkolem: ");
      misc_ErrorReport("Literal is no equation");
      misc_FinishErrorReport();
    }
#endif

  /* Compare <Term> with all terms in the clause */
  last = clause_LastLitIndex(Clause);
  for (i = clause_FirstLitIndex(); i <= last; i++)
    {
      if (i != except)
        {
          ActLit = clause_GetLiteral(Clause, i);
          Atom = clause_LiteralAtom(ActLit);
          if (fol_IsEquality(Atom))
            {
              /* Atom is an equation */
              if (ord_CompareSkolem(Term, term_FirstArgument(Atom), Flags,
                  Precedence) != ord_GREATER_THAN
                  || (!clause_LiteralIsOrientedEquality(ActLit)
                      && ord_CompareSkolem(Term, term_SecondArgument(Atom),
                          Flags, Precedence) != ord_GREATER_THAN))
                /* Compare only with left (i.e. greater) subterm if the atom is */
                /* an oriented equation. */
                return FALSE;
            }
          else
            {
              /* Atom is not an equation */
              if (ord_CompareSkolem(Term, Atom, Flags, Precedence)
                  != ord_GREATER_THAN)
                return FALSE;
            }
        }
    }
  return TRUE;
}

static BOOL
red_TermIsStrictlyMaximalTerm(CLAUSE Clause, int except, TERM Term,
    FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
 INPUT:   A clause, a literal index of the clause and a term,
	  a flag store and a precedence.
 RETURNS: TRUE, iff the bigger (i.e. left) term of the equation
	  is the strictly maximal term of the clause.
	  A term s is strictly maximal in a clause, iff for every atom
	  u=v (A=tt) of the clause s > u and s > v (s > A).
 CAUTION: It is assumed that Literal <except> of <Clause> is
	  an equation, and <Term> is the larger argument of
	  this equation
 ***************************************************************/
{
  int i, last;
  TERM Atom;
  LITERAL ActLit;

#ifdef CHECK
  if(!clause_LiteralIsEquality(clause_GetLiteral(Clause, except)))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_TermIsStrictlyMaximalTerm: ");
      misc_ErrorReport("Literal is no equation");
      misc_FinishErrorReport();
    }
#endif
  /* Compare <Term> with all terms in the clause */
  last = clause_LastLitIndex(Clause);
  for (i = clause_FirstLitIndex(); i <= last; i++)
    {
      if (i != except)
        {
          ActLit = clause_GetLiteral(Clause, i);
          Atom = clause_LiteralAtom(ActLit);
          if (fol_IsEquality(Atom))
            {
              /* Atom is an equation */
              if (ord_Compare(Term, term_FirstArgument(Atom), Flags, Precedence)
                  != ord_GREATER_THAN || (!clause_LiteralIsOrientedEquality(
                  ActLit) && ord_Compare(Term, term_SecondArgument(Atom),
                  Flags, Precedence) != ord_GREATER_THAN))
                /* Compare only with left (i.e. greater) subterm if the atom is */
                /* an oriented equation. */
                return FALSE;
            }
          else
            {
              /* Atom is not an equation */
              if (ord_Compare(Term, Atom, Flags, Precedence)
                  != ord_GREATER_THAN)
                return FALSE;
            }
        }
    }
    return TRUE;
}

static void red_CRwCalculateAdditionalParents(CLAUSE Reduced,
					      LIST RedundantClauses,
					      CLAUSE Subsumer,
					      intptr_t OriginalClauseNumber)
/**************************************************************
  INPUT:   A clause that was just reduced by forward reduction,
           a list of intermediate clauses that were derived from
	   the original clause, a clause that subsumes <Reduced>
	   (NULL, if <Reduced> is not subsumed), and the clause
	   number of <Reduced> before it was reduced.
  RETURNS: Nothing.
  EFFECT:  This function collects the information about parent
           clauses and parent literals that is necessary for
	   proof documentation for Contextual Rewriting
	   and sets the parent information of <Reduced> accordingly.
	   The clause <Reduced> was derived in several steps
	   C1 -> C2 -> ... Cn -> <Reduced> from some clause C1.
           <RedundantClauses> contains all those clauses C1, ..., Cn.
	   This function first collects the parent information from
	   the clauses C1, C2, ..., Cn, <Reduced>. All those clauses
	   were needed to derive <Reduced>, but for proof documentation
	   of the rewriting step we have to delete the numbers of
	   all clauses C1,...,Cn,Reduced.

	   As a simplification this function doesn't set the
	   correct parent literals. It simply assumes that every
	   reduction step was done by literal 0.
	   This isn't a problem since only the correct parent
	   clause numbers are really needed for proof documentation.
***************************************************************/
{
  LIST Parents, Scan;
  intptr_t  ActNum;
  
  /* First collect all parent clause numbers from the redundant clauses. */
  /* Also add number of <Subsumer> if it exists. */
  Parents = clause_ParentClauses(Reduced);
  clause_SetParentClauses(Reduced, list_Nil());
  for (Scan = RedundantClauses; !list_Empty(Scan); Scan = list_Cdr(Scan))
    Parents = list_Append(clause_ParentClauses(list_Car(Scan)), Parents);
  if (Subsumer != NULL)
    Parents = list_Cons((POINTER)clause_Number(Subsumer), Parents);

  /* Now delete <OriginalClauseNumber> and the numbers of all clauses */
  /* that were derived from it.                                       */
  Parents = list_PointerDeleteElement(Parents, (POINTER) OriginalClauseNumber);
  for (Scan = RedundantClauses; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    ActNum = clause_Number(list_Car(Scan));
    Parents = list_PointerDeleteElement(Parents, (POINTER) ActNum);
  }

  /* Finally set data of result clause <Reduced>. */
  Parents = list_PointerDeleteDuplicates(Parents);
  clause_SetParentClauses(Reduced, Parents);
  /* Build list of literal numbers: in this simple version we just build   */
  /* a list with the same length as the parent clauses containing only the */
  /* literal indices 0. */
  Parents = list_Copy(Parents);
  for (Scan = Parents; !list_Empty(Scan); Scan = list_Cdr(Scan))
    list_Rplaca(Scan, (POINTER)0);
  list_Delete(clause_ParentLiterals(Reduced));
  clause_SetParentLiterals(Reduced, Parents);
}

static BOOL red_LiteralIsDefinitionNotOrientedAndReInit(LITERAL Literal)
/**************************************************************
 INPUT:   A literal.
 RETURNS: TRUE, iff the literal is a definition, i.e. an equation x=t,
	  where x is a variable and x doesn't occur in t.
	  The function needs time O(1), it is independent of the size
	  of the literal.
 CAUTION: The Literal does not have to be oriented
 ***************************************************************/
{
  TERM Atom;

  Atom = clause_LiteralAtom(Literal);
  if (fol_IsEquality(Atom) && ((term_IsVariable(term_FirstArgument(Atom))
      && !term_ContainsSymbol(term_SecondArgument(Atom), term_TopSymbol(
          term_FirstArgument(Atom)))) || (term_IsVariable(term_SecondArgument(
      Atom)) && !term_ContainsSymbol(term_FirstArgument(Atom), term_TopSymbol(
      term_SecondArgument(Atom))))) && !term_VariableEqual(term_FirstArgument(
      Atom), term_SecondArgument(Atom)))
    return TRUE;
  else
    return FALSE;
}

BOOL red_LiteralIsDefinition(LITERAL Literal)
/**************************************************************
  INPUT:   A literal.
  RETURNS: TRUE, iff the literal is a definition, i.e. an equation x=t,
           where x is a variable and x doesn't occur in t.
           The function needs time O(1), it is independent of the size
	   of the literal.
  CAUTION: The orientation of the literal must be correct.
***************************************************************/
{
  TERM Atom;

  Atom = clause_LiteralAtom(Literal);
  if (fol_IsEquality(Atom) &&
      !clause_LiteralIsOrientedEquality(Literal) &&
      (term_IsVariable(term_FirstArgument(Atom)) ||
       term_IsVariable(term_SecondArgument(Atom))) &&
      !term_VariableEqual(term_FirstArgument(Atom),
			  term_SecondArgument(Atom)))
    return TRUE;
  else
    return FALSE;
}

static BOOL red_PropagateDefinitions(CLAUSE Clause, TERM LeadingTerm, FLAGSTORE Flags,
				     PRECEDENCE Precedence)
/**************************************************************
 INPUT:   A clause, a term, a flag store and a precedence.
 RETURNS: TRUE, iff any definitions in <Clause> where propagated,
	  false otherwise.

	  Here, a definitions means a negative literal x=t, where
	  x is a variable and x doesn't occur in t.
	  Definitions are only propagated if all terms in the
	  resulting clause would be smaller than <LeadingTerm>.
	  The flag store and the precedence are only needed for
	  term comparisons with respect to the reduction ordering.
 CAUTION: <Clause> is changed destructively!
 ***************************************************************/
{
  LITERAL Lit;
  TERM Term, Atom;
  SYMBOL Var;
  intptr_t i;
  int  last, j, lj;
  BOOL success, applied;
  LIST litsToRemove;

#ifdef CHECK
  if (!clause_IsClause(Clause, Flags, Precedence))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_PropagateDefinitions: clause is corrupted.");
      misc_FinishErrorReport();
    }
#endif

  applied = FALSE;
  litsToRemove = list_Nil(); /* collect indices of redundant literals */
  last = clause_LastAntecedentLitIndex(Clause);

  for (i = clause_FirstAntecedentLitIndex(Clause); i <= last; i++)
    {
      Lit = clause_GetLiteral(Clause, i);

      if (red_LiteralIsDefinitionNotOrientedAndReInit(Lit))
        {
          /* <Lit> is an equation x=t where the variable x doesn't occur in t. */

          Term = term_FirstArgument(clause_LiteralAtom(Lit));
          if (term_IsVariable(Term))
            {
              Var = term_TopSymbol(Term);
              Term = term_SecondArgument(clause_LiteralAtom(Lit));
            }
          else
            {
              Var
                  = term_TopSymbol(term_SecondArgument(clause_LiteralAtom(Lit)));
            }

          /* Establish variable binding x -> t in context */
#ifdef CHECK
          cont_SaveState();
#endif
          cont_StartBinding();
          cont_CreateBinding(cont_LeftContext(), Var, cont_InstanceContext(),
              Term);

          /* Check that for each literal u=v (A=tt) the conditions             */
          /* u{x->t} < LeadingTerm and v{x->t} < LeadingTerm (A < LeadingTerm) */
          /* hold. */
          success = TRUE;
          Lit = NULL;
          lj = clause_LastLitIndex(Clause);

          for (j = clause_FirstLitIndex(); j <= lj && success; j++)
            {
              if (j != i)
                {
                  success = FALSE;
                  Lit = clause_GetLiteral(Clause, j);
                  Atom = clause_LiteralAtom(Lit);
                  if (fol_IsEquality(Atom))
                    {
                      /* Atom is an equation */
                      if (ord_ContGreater(cont_InstanceContext(), LeadingTerm,
                          cont_LeftContext(), term_FirstArgument(Atom), Flags,
                          Precedence) && (clause_LiteralIsOrientedEquality(Lit)
                          || ord_ContGreater(cont_InstanceContext(),
                              LeadingTerm, cont_LeftContext(),
                              term_SecondArgument(Atom), Flags, Precedence)))
                        /* Compare only with left (i.e. greater) subterm if the atom is */
                        /* an oriented equation. */
                        success = TRUE;
                    }
                  else
                    {
                      /* Atom is not an equation */
                      if (ord_ContGreater(cont_InstanceContext(), LeadingTerm,
                          cont_LeftContext(), Atom, Flags, Precedence))
                        success = TRUE;
                    }
                }
            }

          cont_BackTrack();

#ifdef CHECK
          cont_CheckState();
#endif

          if (success)
            {
              /* Replace variable <Var> in <Clause> by <Term> */
              clause_ReplaceVariable(Clause, Var, Term);
              /* The clause literals aren't reoriented here. For the detection of */
              /* definitions it suffices to know the non-oriented literals in the */
              /* original clause. */
              litsToRemove = list_Cons((POINTER) i, litsToRemove);
              applied = TRUE;
            }
        }
    }

  if (applied)
    {
      /* Now remove the definition literals. */
      clause_DeleteLiterals(Clause, litsToRemove, Flags, Precedence);
      list_Delete(litsToRemove);

      /* Equations have to be reoriented. */
      clause_PrecomputeOrdering(Clause, Flags, Precedence);

#ifdef CHECK
      if (!clause_IsClause(Clause, Flags, Precedence))
        {
          misc_StartErrorReport();
          misc_ErrorReport("\n In red_PropagateDefinitions: clause is corrupted ");
          misc_ErrorReport("after propagation of definitions");
          misc_FinishErrorReport();
        }
#endif
    }

  return applied;
}

static void red_DocumentContextualRewriting(CLAUSE Clause, int i,
					    CLAUSE RuleClause, int ri,
					    LIST AdditionalPClauses,
					    LIST AdditionalPLits)
/**************************************************************
  INPUT:   Two clauses and two literal indices (one per clause),
           and two lists of additional parent clause numbers and
	   parent literals.
  RETURNS: Nothing.
  EFFECT:  <Clause> is rewritten for the first time by
           Contextual Rewriting. This function sets the parent
           clause and parent literal information in <Clause>.
	   <Clause> gets a new clause number.
  CAUTION: The lists are not copied!
***************************************************************/
{
#ifdef CHECK
  if (list_Length(AdditionalPClauses) != list_Length(AdditionalPLits)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_DocumentContextualRewriting: lists of parent ");
    misc_ErrorReport("clauses\n and literals have different length.");
    misc_FinishErrorReport();
  }
#endif

  list_Delete(clause_ParentClauses(Clause));
  list_Delete(clause_ParentLiterals(Clause));
  clause_SetParentClauses(Clause, AdditionalPClauses);
  clause_SetParentLiterals(Clause, AdditionalPLits);
  /* Add the old number of <Clause> as parent clause, */
  /* before it gets a new clause number.              */
  clause_AddParentClause(Clause, clause_Number(Clause));
  clause_AddParentLiteral(Clause, i);
  clause_AddParentClause(Clause, clause_Number(RuleClause));
  clause_AddParentLiteral(Clause, ri);

  clause_NewNumber(Clause);
  clause_SetFromContextualRewriting(Clause);
}


static void red_DocumentFurtherCRw(CLAUSE Clause, int i, CLAUSE RuleClause,
				   int ri, LIST AdditionalPClauses,
				   LIST AdditionalPLits)
/**************************************************************
  INPUT:   Two clauses, two literal indices (one per clause),
           and two lists of additional parent clause numbers and
	   parent literal indices.
  RETURNS: Nothing.
  EFFECT:  <Clause> is a clause, that was rewritten before by
           Contextual Rewriting. This function adds the parent
           clause and parent literal information from one more
           rewriting step to <Clause>. The information is added
           to the front of the respective lists.
  CAUTION: The lists are not copied!
***************************************************************/
{
  int PClauseNum;

#ifdef CHECK
  if (list_Length(AdditionalPClauses) != list_Length(AdditionalPLits)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_DocumentFurtherCRw: lists of parent ");
    misc_ErrorReport("clauses\n and literals have different length.");
    misc_FinishErrorReport();
  }
#endif

  PClauseNum = (intptr_t)list_Second(clause_ParentClauses(Clause));

  clause_SetParentClauses(Clause, list_Nconc(AdditionalPClauses,
					     clause_ParentClauses(Clause)));
  clause_SetParentLiterals(Clause, list_Nconc(AdditionalPLits,
					      clause_ParentLiterals(Clause)));

  clause_AddParentClause(Clause, PClauseNum);
  clause_AddParentLiteral(Clause, i);
  clause_AddParentClause(Clause, clause_Number(RuleClause));
  clause_AddParentLiteral(Clause, ri);
}

static BOOL
red_CRwRecContGrRewLitCondCheck(PROOFSEARCH Search, CLAUSE RedClause,
    int Except, CLAUSE RuleClause, int h, TERM TermSInstance,
    LIST *RewParentCls, LIST* RewParentLits, LIST Blocked, int RecDepth)
/*************************************************************************
 INPUT:   A proof search object, two clauses, two literal indices
	  (one per clause), a mode defining the clause index used
	  for intermediate reductions. <TermSInstance> is a subterm of
	  literal <i> in <RedClause>,
	  a pointer to a list where all parent clauses are stored
	  that are used for rewriting and
	  a pointer to a list where all literals are stored that are used
	  for rewriting
 RETURNS: FALSE, if the tautology check for literal <h> in <RuleClause>
	  failed. TRUE else.
 *************************************************************************/

{
  FLAGSTORE Flags;
  PRECEDENCE Precedence;
  CLAUSE aux;
  LITERAL Lit;
  TERM Atom;
  BOOL Negative;
  LIST NegLits, PosLits;

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
  Lit = clause_GetLiteral(RuleClause, h);
  Atom = clause_LiteralAtom(Lit);
  Negative = clause_LiteralIsNegative(Lit);

#ifdef CRW_CHECK
  printf("\n  ----------\n  ");
  if (Negative)
  printf((h <= clause_LastConstraintLitIndex(RuleClause)) ? "Cons" : "Ante");
  else
  printf("Succ");
#endif

  /* Collect literals for tautology test */
  if (Negative)
    {
      if (h <= clause_LastConstraintLitIndex(RuleClause))
        NegLits = clause_CopyConstraint(RedClause);
      else
        NegLits = clause_CopyAntecedentExcept(RedClause, Except);
      PosLits = list_List(term_Copy(Atom));
    }
  else
    {
      NegLits = list_List(term_Copy(Atom));
      PosLits = clause_CopySuccedentExcept(RedClause, Except);
    }

#ifdef CRW_CHECK
  printf("\nExcept: %d, h: %d",Except,h);
  printf("\nRuleClause: "); clause_Print(RuleClause);
  printf("\nRedClause: "); clause_Print(RedClause);
  printf(" \naux = ");
  fflush(stdout);
#endif

  /* Create clause intermediate temporal clause */
  aux = clause_CreateSkolem(list_Nil(), NegLits, PosLits, Flags, Precedence);
  clause_SetTemporary(aux);
  list_Delete(NegLits);
  list_Delete(PosLits);

#ifdef CRW_CHECK
  clause_Print(aux);
#endif

  /* Invoke tautology test - this is done in red_CRwRecursiveTautology if -RTaut=2 */
  if (!clause_IsEmptyClause(aux) && /* (cc_Tautology(aux) - is done in red_CRwRecursiveTautology if -RTaut=2 */
  /* call recursively recursive contextual rewriting */
  red_CRwGroundSubtermRedundant(Search, aux, RewParentCls, RewParentLits, Blocked,
      RecDepth + 1))
    {

      /* tautology test succeded */
      clause_UpdateSplitDataFromPartner(RuleClause, aux);
#ifdef CRW_CHECK
      printf("\n  aux reduced = ");
      clause_Print(aux);
#endif
      clause_Delete(aux);
      return TRUE;

    }
  else
    {
      /* test failed */

      clause_Delete(aux);
      return FALSE;
    }

}

static BOOL
red_CRwRecContGrRewSideCondCheck(PROOFSEARCH Search, CLAUSE RedClause, int i,
    TERM TermSInstance, CLAUSE RuleCopy, int j, LIST *RewParentCls,
    LIST* RewParentLits, LIST Blocked, int RecDepth)
/**************************************************************
 INPUT:   A proof search object, two clauses, two literal indices
	  (one per clause), <TermSInstance> is a subterm of
	  literal <i> in <RedClause>, Literal <j> of
	  <RuleClause> has to be strictly maximal in <RuleClause>
	  and has to be oriented equality.
	  a pointer to a list where all parent clauses are stored
	  that are used for rewriting and
	  a pointer to a list where all literals are stored that are used
	  for rewriting
 RETURNS: TRUE if all validity checks of contextual rewriting
	  return true. FALSE otherwise.
 ***************************************************************/
{
#ifdef CRW_DEBUG
  FLAGSTORE BackupFlags;
#endif

  FLAGSTORE Flags;
  PRECEDENCE Precedence;
  CLAUSE aux;
  int last, h, OldClauseCounter;
  BOOL Rewrite;

  aux = NULL;
  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
  Rewrite = FALSE;

#ifdef CRW_DEBUG
    {
      /* Create new flag store and save current settings.  Must be improved ****     */
      /* Then turn off flags for printing and contextual rewriting. */
      /* IMPORTANT: the DocProof flag mustn't be changed!       */
      FLAG_ID f;
      BackupFlags = flag_CreateStore();
      flag_TransferAllFlags(Flags, BackupFlags);

      /* HACK: turn on all printing flags for debugging */

      for (f = (FLAG_ID) 0; f < flag_MAXFLAG; f++)
        {
          if (flag_IsPrinting(f))
          flag_SetFlagIntValue(Flags, f, flag_ON);
        }
    }
#endif

  /* Examine all literals of <RuleCopy> except <j> */
  Rewrite = TRUE;
  last = clause_LastLitIndex(RuleCopy);
  OldClauseCounter = clause_Counter();

  for (h = clause_FirstLitIndex(); Rewrite && h <= last; h++)
    {
      if (h != j)
        {
          if (!red_CRwRecContGrRewLitCondCheck(Search, RedClause, i,
              RuleCopy, h, TermSInstance, RewParentCls, RewParentLits, Blocked,
              RecDepth))
            {
              Rewrite = FALSE;
            }
        }
    }
  /* restore clause counter */
  clause_SetCounter(OldClauseCounter);

  /* reset flag store of proof search object and free backup store */
#ifdef CRW_DEBUG
  flag_TransferAllFlags(BackupFlags, Flags);
  flag_DeleteStore(BackupFlags);
#endif

  return Rewrite;
}

BOOL
red_CRwIsTermInIndexModuloVariables(st_INDEX Index, TERM Term)
/*************************************************************
 * INPUT:   A term index and a term
 * RETURN:  TRUE if a term is in the index that is equal to <term>,
 * 			variables are allowed to differ between the terms
 * CAUTION: modifies cont_LeftContext()
 *************************************************************/
{
  TERM CandTerm;

  CandTerm = st_ExistGen(cont_LeftContext(), Index, Term);

  while (CandTerm)
    {

#ifdef CRW_DEBUG
      printf("\nCmpTbl: Term: "); term_Print(Term);
      printf("\nCmpTbl: Cand: "); term_Print(CandTerm);
      cont_PrintCurrentTrail();
#endif
      if (cont_OnlyVariablesInCoDomOfCurrentTrail())
        {
          st_CancelExistRetrieval();
          return TRUE;
        }

      CandTerm = st_NextCandidate();
    }
  st_CancelExistRetrieval();
 return FALSE;
}

static BOOL
red_CRwRecursiveContextualGroundRewritingAux(PROOFSEARCH Search,
    SHARED_INDEX ShIndex, CLAUSE RedClause, int ri, LIST *RewParentCls,
    LIST* RewParentLits, LIST Blocked, int RecDepth)
/*************************************************************
 * INPUT:   A Proofsearch object, an Index, a Clause,
 *          an integer,
 *          a pointer to a list where all parent clauses are stored
 *          that are used for rewriting and
 *          a pointer to a list where all literals are stored that are used
 *          for rewriting
 * RETURN:  TRUE if RedClause could be rewritten, FALSE otherwise
 * EFFECT:  mutually recursive, destructivly changes <RedClause>
 * CAUTION: Variables of <RedClause> are interpreted as constants
 *************************************************************/
{

  TERM RedAtom, RedTermS;
  int B_Stack;
  int last, h;
  BOOL Rewritten, Result, Tautology, Oriented;
  TERM TermS, PartnerEq, TermSCopy, Right, RightCopy;
  LIST Gen, EqScan, LitScan;
  CLAUSE Copy;
  FLAGSTORE Flags;
  PRECEDENCE Precedence;

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);

#ifdef CHECK
  if (!clause_IsClauseSkolem(RedClause, Flags, Precedence))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_CRwRecursiveContRewriting: Illegal input.\n");
      misc_FinishErrorReport();
    }
  clause_CheckSkolem(RedClause, Flags, Precedence);
#endif
  last = clause_LastSuccedentLitIndex(RedClause);

  Result = FALSE;
  Copy = RedClause;

#ifdef CRW_DEBUG
  if( !(stack_POINTER < CRW_CHECK_STACK) )
    {
      misc_StartErrorReport();
      misc_ErrorReport("\nContextual Rewriting: Max recursion depth reached");
      misc_FinishErrorReport();
    }
#endif

  Rewritten = TRUE;
  while (Rewritten)
    {
      Rewritten = FALSE;
      RedAtom = clause_GetLiteralAtom(Copy, ri);

      B_Stack = stack_Bottom();
      /* push subterms on stack except variables */
      sharing_PushListReverseOnStack(term_ArgumentList(RedAtom));

      while (!stack_Empty(B_Stack))
        {
          RedTermS = (TERM) stack_PopResult();
          /* Check if <RedTermS> is in <red_CRW_TERM_FAULT_CACHE>  (fault caching)
           */
          if (!red_CRwIsTermInIndexModuloVariables(red_CRW_TERM_FAULT_CACHE,
              RedTermS))
            {
              Gen = st_GetGen(cont_LeftContext(), sharing_Index(ShIndex),
                  RedTermS);

              for (; !list_Empty(Gen) && !Rewritten; Gen = list_Pop(Gen))
                {
                  TermS = list_Car(Gen);

                  /* A variable can't be greater than any other term, */
                  /* so don't consider any variables here.            */
                  /* TODO: can this be removed, variables are interpreted as constants? */
                  if (!term_IsVariable(TermS))
                    {
                      EqScan = term_SupertermList(TermS);

                      for (; !list_Empty(EqScan) && !Rewritten; EqScan
                          = list_Cdr(EqScan))
                        {
                          PartnerEq = list_Car(EqScan);
                          if (fol_IsEquality(PartnerEq)
                              && ((term_FirstArgument(PartnerEq) == TermS)
                                  || (term_SecondArgument(PartnerEq) == TermS)))
                            {

                              CLAUSE RuleClause;
                              LITERAL RuleLit;
                              intptr_t i;

                              for (LitScan = sharing_NAtomDataList(PartnerEq); !list_Empty(
                                  LitScan) && !Rewritten; LitScan = list_Cdr(
                                  LitScan))
                                {
                                  RuleLit = list_Car(LitScan);
                                  RuleClause = clause_LiteralOwningClause(
                                      RuleLit);
                                  i = clause_LiteralGetIndex(RuleLit);
                                  Tautology = FALSE;
                                  Oriented = FALSE;
#ifdef CRW_CHECK
                                  if (clause_LiteralIsPositive(RuleLit) &&
                                      clause_LiteralGetFlag(RuleLit,STRICTMAXIMAL) &&
                                      !list_PointerMember(Blocked, RuleClause) &&

                                      /* if it is oriented then TermS has to be the left side
                                       * of the equation, namely the larger term */
                                      (!clause_LiteralIsOrientedEquality(RuleLit) ||
                                          (term_FirstArgument(PartnerEq) == TermS)) )
                                    {
                                      printf("\nRecCRw: \n RuleClause: %d ", i);
                                      clause_Print(RuleClause);
                                      printf("\nRedClause:  %d  ", ri);
                                      clause_Print(Copy);
                                    }

#endif

                                  /* Candidate Clause found <RuleClause> - check if it is
                                   * Strongly universally reductive */
                                  if (/* Lit must be positive */
                                  clause_LiteralIsPositive(RuleLit) &&
                                  /* and strict maximal */
                                  clause_LiteralGetFlag(RuleLit, STRICTMAXIMAL)
                                      &&
                                      /* <RuleClause> is not blocked */
                                      !list_PointerMember(Blocked, RuleClause)
                                      &&
                                      /* if <RuleLit> is oriented then <TermS> must be the left side */
                                      (!clause_LiteralIsOrientedEquality(
                                          RuleLit) || (term_FirstArgument(
                                          PartnerEq) == TermS))
                                      && clause_VarsOfClauseAreInTerm(
                                          RuleClause, TermS))
                                    {

                                      CLAUSE RuleCopy;

                                      /* copy <RuleClause> and rename variables in copy */
                                      /* needed to prevent variable capturing */
                                      RuleCopy = clause_Copy(RuleClause);
                                      clause_RenameVarsBiggerThan(RuleCopy,
                                          clause_MaxVar(Copy));

                                      /* which is the left/right term of the equation */
                                      if (TermS
                                          == term_FirstArgument(PartnerEq))
                                        {
                                          TermSCopy
                                              = term_FirstArgument(
                                                  clause_GetLiteralAtom(
                                                      RuleCopy, i));
                                          Right
                                              = term_SecondArgument(PartnerEq);
                                          RightCopy
                                              = term_SecondArgument(
                                                  clause_GetLiteralAtom(
                                                      RuleCopy, i));
                                        }
                                      else
                                        {
                                          TermSCopy
                                              = term_SecondArgument(
                                                  clause_GetLiteralAtom(
                                                      RuleCopy, i));
                                          Right = term_FirstArgument(PartnerEq);
                                          RightCopy
                                              = term_FirstArgument(
                                                  clause_GetLiteralAtom(
                                                      RuleCopy, i));
                                        }

                                      /* Remove parent information of copied clause and mark it as temporary */
                                      list_Delete(
                                          clause_ParentClauses(RuleCopy));
                                      clause_SetParentClauses(RuleCopy,
                                          list_Nil());
                                      list_Delete(clause_ParentLiterals(
                                          RuleCopy));
                                      clause_SetParentLiterals(RuleCopy,
                                          list_Nil());
                                      clause_SetTemporary(RuleCopy);

                                      /* Check if Literal <j> of <RuleClause>
                                       * is Oriented and the left term is the
                                       * matching term */
                                      Oriented
                                          = (clause_LiteralIsOrientedEquality(
                                              RuleLit) && term_FirstArgument(
                                              clause_GetLiteralAtom(RuleClause,
                                                  i)) == TermS);

                                      /* establish bindings */
                                      cont_StartBinding();

                                      if (!unify_MatchBindings(
                                          cont_LeftContext(), TermSCopy,
                                          RedTermS))
                                        {
#ifdef CRW_CHECK
                                          if(Oriented) printf("\nOriented");
                                          printf("\nTermS    :"); term_Print(TermS);
                                          printf("\nTermSCopy:"); term_Print(TermSCopy);
                                          printf("\nRedTermS :"); term_Print(RedTermS);
                                          cont_PrintCurrentTrail();
                                          misc_StartErrorReport();
                                          misc_ErrorReport("\n In red_CRwRecursiveContextualRewritingAux: terms aren't matchable");
                                          misc_FinishErrorReport();
#endif
                                        }

                                      /* if RuleLit is not oriented try to orient in the context where
                                       * variables are interpreted as Skolem constants */
                                      if (!Oriented
                                          && !clause_LiteralIsOrientedEquality(
                                              RuleLit))
                                        {
                                          /* vars(TermSCopy) are superset of vars(RightCopy) because vars(TermS) is a
                                           * superset of vars(RuleClause) and therefore is vars(TermSCopy) a superset of
                                           * vars(RuleCopy) */
                                          Oriented
                                              = ord_ContGreaterSkolemSubst(
                                                  cont_LeftContext(),
                                                  TermSCopy,
                                                  cont_LeftContext(),
                                                  RightCopy, Flags, Precedence);
                                        }
#ifdef CRW_CHECK
                                      if(Oriented)
                                        {
                                          printf("\nRedTermS :"); term_Print(RedTermS);
                                          fflush(stdout);
                                          printf("\nRedAtom  :"); term_Print(RedAtom);
                                          fflush(stdout);
                                          printf("\nTermSCopy:"); term_Print(TermSCopy);
                                          printf("\nRightCopy:"); term_Print(RightCopy);
                                          cont_PrintCurrentTrail();
                                        }
#endif
                                      if (Oriented)
                                        {
                                          Tautology = FALSE;

                                          /* Apply bindings to equation s=t, where s > t. Here the strict version */
                                          /* of cont_Apply... can be applied, because all variables in s and t    */
                                          /* are bound. */
                                          cont_ApplyBindingsModuloMatching(
                                              cont_LeftContext(),
                                              clause_GetLiteralAtom(RuleCopy, i),
                                              TRUE);

                                          /* Check if Unit Rewriting is possible */
                                          if ((clause_Length(Copy) == 1)
                                              && (clause_Length(RuleCopy) == 1))
                                            {
                                              Tautology = TRUE;
                                              cont_BackTrack();
                                            }
                                          /* Check if Non-Unit Rewriting is possible */
                                          else if ((clause_Length(Copy) != 1)
                                              && subs_SubsumesBasic(RuleCopy,
                                                  Copy, i, ri))
                                            {
                                              Tautology = TRUE;
                                              cont_BackTrack();
                                            }
                                          else
                                            {
                                              /* Check whether (u[s\sigma] = v) > ((s=t)\sigma). It suffices to check only positive */
                                              /* equations. All other cases imply the condition. */
                                              if (ord_LiteralCompareAux(
                                                  clause_GetLiteralTerm(Copy,
                                                      ri),
                                                  clause_LiteralGetOrderStatus(
                                                      clause_GetLiteral(Copy,
                                                          ri)),
                                                  clause_GetLiteralTerm(
                                                      RuleCopy, i),
                                                  clause_LiteralGetOrderStatus(
                                                      clause_GetLiteral(
                                                          RuleCopy, i)), TRUE,
                                                  TRUE, Flags, Precedence)
                                                  != ord_GREATER_THAN)
                                                {

                                                  cont_BackTrack();
                                                }
                                              else
                                                {
                                                  int lastlit;

                                                  /* Apply bindings to the rest of <RuleCopy> */
                                                  lastlit
                                                      = clause_LastLitIndex(
                                                          RuleCopy);
                                                  for (h
                                                      = clause_FirstLitIndex(); h
                                                      <= lastlit; h++)
                                                    {
                                                      if (h != i)
                                                        cont_ApplyBindingsModuloMatching(
                                                            cont_LeftContext(),
                                                            clause_GetLiteralAtom(
                                                                RuleCopy, h),
                                                            FALSE);
                                                    }
                                                  if (!red_TermIsStrictlyMaximalTermSkolem(
                                                      RuleCopy, i, TermSCopy,
                                                      Flags, Precedence))
                                                    {
                                                      cont_BackTrack();
                                                    }
                                                  else
                                                    {
                                                      /* Backtrack bindings before reduction rules are invoked */
                                                      cont_BackTrack();

#ifdef CRW_DEBUG
                                                      printf("\n\t------\n\tFCRwRecursive: \n%d  ", i);
                                                      clause_Print(RuleClause);
                                                      printf("\n%d  ", ri);
                                                      clause_Print(Copy);
#endif

                                                      Tautology
                                                          = red_CRwRecContGrRewSideCondCheck(
                                                              Search, Copy, ri,
                                                              RedTermS,
                                                              RuleCopy, i,
                                                              RewParentCls,
                                                              RewParentLits,
                                                              Blocked, RecDepth);
                                                    }

                                                }
                                            }
                                        }
                                      else
                                        { /* if not oriented Backtrack last binding */
                                          cont_BackTrack();
                                        }

                                      if (Tautology)
                                        /* the side conditions are fullfiled so rewrite term */
                                        {
                                          TERM TermT;

                                          if (!Result && 
					      flag_GetFlagIntValue(Flags, flag_PREW) == flag_PREWDET)
                                            {
                                              /* Clause is rewitten for the first time and */
                                              /* printing is turned on. */
                                              fputs("\nFRContRewriting: ",
                                                  stdout);
                                              clause_Print(Copy);
                                              fputs(" ==>[ ", stdout);
                                            }

                                          /* collect rule lit if DocProof is enabled */
                                          if (flag_GetFlagIntValue(Flags, flag_DOCPROOF))
                                            {
                                              *RewParentCls = list_Cons(
                                                  (POINTER) clause_Number(
                                                      RuleClause),
                                                  *RewParentCls);
                                              *RewParentLits = list_Cons(
                                                  (POINTER) i, *RewParentLits);
                                            }

                                          Result = TRUE;

                                          cont_StartBinding();
                                          unify_MatchBindings(
                                              cont_LeftContext(), TermS,
                                              RedTermS);
                                          TermT
                                              = cont_ApplyBindingsModuloMatching(
                                                  cont_LeftContext(),
                                                  term_Copy(Right), TRUE);
                                          cont_BackTrack();

                                          term_ReplaceSubtermBy(RedAtom,
                                              RedTermS, TermT);
                                          Rewritten = TRUE;

                                          /* Set splitting data from parents */
                                          clause_UpdateSplitDataFromPartner(
                                              Copy, RuleCopy);

                                          term_Delete(TermT);
                                          stack_SetBottom(B_Stack);

                                          if (flag_GetFlagIntValue(Flags, flag_PREW) == flag_PREWDET)
                                            printf("%zd.%zd ", clause_Number(
                                                RuleClause), i);
                                          clause_UpdateWeight(Copy, Flags);
                                        }
                                      clause_Delete(RuleCopy);
                                    }

                                } /* for */
                            }
                        }
                    }
                }
              list_Delete(Gen);

              /* RedTermS could not be reduced,
               * insert RedTermS in <red_CRwNotreducableTerms> (fault caching)*/
              if (!Rewritten && !term_IsVariable(RedTermS) &&
                         /* This has to be checked explicitly because the term <RedTermS>
                          * might be inserted into the caching during the recursive calls
                          */
                         !red_CRwIsTermInIndexModuloVariables(red_CRW_TERM_FAULT_CACHE, RedTermS))
                {
#ifdef CHECK
                  /* Check if the fault caching is working properly */
                  /* If RedTermS is in red_CRW_TERM_FAULT_CACHE here then there is something wrong */
                    {
                      LIST chkCand = st_GetInstance(cont_LeftContext(), red_CRW_TERM_FAULT_CACHE, RedTermS);
                      LIST Scan;
                      BOOL check = FALSE;
                      for(Scan = chkCand; !list_Empty(Scan); Scan=list_Cdr(Scan))
                        {
                          if(term_Equal(list_Car(Scan),RedTermS))
                            {
                              check = TRUE;
                            }
                        }
                      list_Delete(chkCand);
                      if(check)
                        {
                          printf("\nError at Term: "); term_Print(RedTermS);
                          st_Print(red_CRW_TERM_FAULT_CACHE, (void (*)(POINTER))term_Print);
                          misc_StartErrorReport();
                          misc_ErrorReport("\nred_CRwRecursiveContextualRewritingAux: Clause is already in Index.");
                          misc_FinishErrorReport();
                        }
                    }
#endif
                    /* Insert <RedTermS> into fault cache */
                    st_EntryCreate(red_CRW_TERM_FAULT_CACHE, term_Copy(RedTermS),
                      RedTermS, cont_LeftContext());
#ifdef CHECK
                    {
                      LIST chkCand = st_GetInstance(cont_LeftContext(), red_CRW_TERM_FAULT_CACHE, RedTermS);
                      LIST Scan;
                      BOOL check = FALSE;
                      for(Scan = chkCand; !list_Empty(Scan); Scan=list_Cdr(Scan))
                        {
                          if(term_Equal(list_Car(Scan),RedTermS))
                            {
                              check = TRUE;
                            }
                        }
                      list_Delete(chkCand);
                      if(!check)
                        {
                          misc_StartErrorReport();
                          misc_ErrorReport("\nred_CRwRecursiveContextualRewritingAux: CRw Index Check failed.");
                          misc_FinishErrorReport();
                        }
                    }
#endif
                }
            }
        } /* while */
    } /* while */
  if (Result)
    {
      clause_PrecomputeOrderingAndReInitSkolem(Copy, Flags, Precedence);
      if (flag_GetFlagIntValue(Flags, flag_PREW) == flag_PREWDET)
        {
          fputs("] ", stdout);
          clause_Print(Copy);
        }
      if (Copy != RedClause)
        {
          clause_PrecomputeOrderingAndReInitSkolem(RedClause, Flags, Precedence);
        }
    }

#ifdef CHECK
  if (Copy != RedClause)
  clause_CheckSkolem(Copy, Flags, Precedence);
  clause_CheckSkolem(RedClause, Flags, Precedence);
#endif

  return Result;
}

static BOOL
red_CRwRecursiveContextualGroundRewriting(PROOFSEARCH Search, SHARED_INDEX ShIndex,
    CLAUSE RedClause, LIST *RewParentCls, LIST* RewParentLits, LIST Blocked,
    int RecDepth)
/*************************************************************
 * INPUT:   A Proofsearch object, an Index, a Clause,
 *          a pointer to a list where all parent clauses are stored
 *          that are used for rewriting and
 *          a pointer to a list where all literals are stored that are used
 *          for rewriting
 * RETURN:  TRUE if <RedClause> could be rewritten in <ShIndex>
 *          else FALSE
 * EFFECT:  mutually recursive, destructively changes <RedClause>
 * CAUTION: <RedClause> is assumed to be a temporary clause
 *************************************************************/
{

  int ri, last;
  BOOL Result;

#ifdef CHECK
  FLAGSTORE Flags;
  PRECEDENCE Precedence;

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);

  if (!clause_IsClauseSkolem(RedClause, Flags, Precedence))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_CRwRecursiveContRewriting: Illegal input.\n");
      misc_FinishErrorReport();
    }

  clause_CheckSkolem(RedClause, Flags, Precedence);

  if( !(stack_POINTER < CRW_CHECK_STACK) )
    {
      misc_StartErrorReport();
      misc_ErrorReport("\nRecursive Contextual Rewriting: Max recursion depth reached");
      misc_FinishErrorReport();
    }

#endif

  last = clause_LastSuccedentLitIndex(RedClause);
  Result = FALSE;

  /* Don't apply this rule on constraint literals! */
  /* First try to reduce the strict maximal literals */
  for (ri = clause_FirstAntecedentLitIndex(RedClause); ri <= last; ri++)
    {
      if (clause_LiteralGetFlag(clause_GetLiteral(RedClause, ri), STRICTMAXIMAL)
          && !list_Empty(
              term_ArgumentList(clause_GetLiteralAtom(RedClause, ri))))
        {
          Result = red_CRwRecursiveContextualGroundRewritingAux(Search, ShIndex,
              RedClause, ri, RewParentCls, RewParentLits, Blocked, RecDepth);
        }
    }
  /* then reduce non strict maximal literals */
  for (ri = clause_FirstAntecedentLitIndex(RedClause); ri <= last && !Result; ri++)
    {
      if (!clause_LiteralGetFlag(clause_GetLiteral(RedClause, ri),
          STRICTMAXIMAL) && !list_Empty(term_ArgumentList(
          clause_GetLiteralAtom(RedClause, ri))))
        {
          Result = red_CRwRecursiveContextualGroundRewritingAux(Search, ShIndex,
              RedClause, ri, RewParentCls, RewParentLits, Blocked, RecDepth);
        }
    }

#ifdef CHECK
  clause_CheckSkolem(RedClause, Flags, Precedence);
#endif

  return Result;
}

BOOL
red_CRwGroundSubtermRedundant(PROOFSEARCH Search, CLAUSE aux, LIST *RewParentCls,
    LIST* RewParentLits, LIST Blocked, int RecDepth)
/*************************************************************
 * INPUT:   A Proofsearch object and a clause <aux>,
 *          a pointer to a list where all parent clauses are stored
 *          that are used for rewriting and
 *          a pointer to a list where all literals are stored that are used
 *          for rewriting
 *          and a integer indicating the current recursion depth (only needed
 *          for debugging purpose
 * RETURN:  TRUE if aux is valid in Search else FALSE
 * CAUTION: <aux> is expected to be temporary
 * EFFECT:  mutually recursive
 *************************************************************/
{
  CLAUSE Copy, Subsumer;
  FLAGSTORE Flags;
  PRECEDENCE Precedence;
  SHARED_INDEX UsIndex, WoIndex;
  BOOL Rewritten;

#ifdef CHECK
  if(!clause_IsTemporary(aux))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_CRwRecursiveTautology: Illegal Input.\n");
      misc_FinishErrorReport();
    }
#endif

#ifdef CRW_DEBUG
  printf("\n\n======================= Recursion Depth %d START",RecDepth);
  printf("\n    RCRw Loop aux= "); clause_Print(aux);
#endif

  UsIndex = prfs_UsableSharingIndex(Search);
  WoIndex = prfs_WorkedOffSharingIndex(Search);

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);

  Rewritten = TRUE;

  while (Rewritten)
    {
      Rewritten = FALSE;
      Copy = (CLAUSE) NULL;
      Subsumer = (CLAUSE) NULL;

      if (clause_IsEmptyClause(aux))
        {
#ifdef CRW_DEBUG
          printf("\n\n======================= Recursion Depth %d END\n",RecDepth);
#endif
          return FALSE;
        }

      if (red_ObviousReductionsSkolem(aux, FALSE, Flags, Precedence, &Copy))
        {
          Rewritten = TRUE;
          clause_PrecomputeOrderingAndReInitSkolem(aux, Flags, Precedence);
        }

      if (red_CondensingSkolem(aux, FALSE, Flags, Precedence, &Copy))
        {
          Rewritten = TRUE;
          clause_PrecomputeOrderingAndReInitSkolem(aux, Flags, Precedence);
        }

      if (red_TautologySkolem(aux, Flags, Precedence))
        {
#ifdef CRW_DEBUG
          printf("\n\n======================= Recursion Depth %d END\n",RecDepth);
#endif
          return TRUE;
        }

      Subsumer = (CLAUSE) NULL;
      if (WoIndex != NULL)
        {
          Subsumer = red_ForwardSubsumerSkolem(aux, WoIndex, Flags, Precedence,
              Blocked);
          if (Subsumer != (CLAUSE) NULL)
            {
              clause_UpdateSplitDataFromPartner(aux, Subsumer);
              if (flag_GetFlagIntValue(Flags, flag_DOCPROOF))
                {
                  /* document subsumer - we set 0 for the literal
                   * since we do not need this information */
                  *RewParentCls = list_Cons((POINTER) clause_Number(Subsumer),
                      *RewParentCls);
                  *RewParentLits = list_Cons((POINTER) 0, *RewParentLits);
                }
#ifdef CRW_DEBUG
              printf("\nRCRw Subsumer: "); clause_Print(Subsumer);
              printf("\n\n======================= Recursion Depth %d END\n",RecDepth);
#endif
              return TRUE;
            }
        }
      if (UsIndex != NULL)
        {
          Subsumer = red_ForwardSubsumerSkolem(aux, UsIndex, Flags, Precedence,
              Blocked);
          if (Subsumer != (CLAUSE) NULL)
            {
              clause_UpdateSplitDataFromPartner(aux, Subsumer);
              if (flag_GetFlagIntValue(Flags, flag_DOCPROOF))
                {
                  /* document subsumer - we set 0 for the literal
                   * since we do not need this information */
                  *RewParentCls = list_Cons((POINTER) clause_Number(Subsumer),
                      *RewParentCls);
                  *RewParentLits = list_Cons((POINTER) 0, *RewParentLits);
                }
#ifdef CRW_DEBUG
              printf("\nRCRw Subsumer: "); clause_Print(Subsumer);
              printf("\n\n======================= Recursion Depth %d END\n",RecDepth);
#endif
              return TRUE;
            }
        }

      /* Invoke RecursiveContextualGroundRewriting recursively */
      if (UsIndex != NULL && !Rewritten)
        {
          if (red_CRwRecursiveContextualGroundRewriting(Search, UsIndex, aux,
              RewParentCls, RewParentLits, Blocked, RecDepth))
            {
              Rewritten = TRUE;
            }

        }
      if (WoIndex != NULL && !Rewritten)
        {
          if (red_CRwRecursiveContextualGroundRewriting(Search, WoIndex, aux,
              RewParentCls, RewParentLits, Blocked, RecDepth))
            {
              Rewritten = TRUE;
            }
        }

    }
#ifdef CRW_DEBUG
  printf("\n\n======================= Recursion Depth %d END\n",RecDepth);
#endif
  return FALSE;
}

static CLAUSE
red_CRwLitCondSubtermContextualRewriting(PROOFSEARCH Search, CLAUSE RedClause,
    int Except, CLAUSE RuleClause, int i, TERM LeadingTerm,
    BOOL DefPropagation, NAT Mode, LIST *RewParentCls, LIST* RewParentLits,
    LIST Blocked)
/**************************************************************
 INPUT:   A proof search object, two clauses, two literal indices
	  (one per clause), a mode defining the clause index used
	  for intermediate reductions, a flag indicating if
	  red_DefinitionPropagation is applied,
	  a pointer to a list where all parent clauses are stored
	  that are used for rewriting and
	  a pointer to a list where all literals are stored that are used
	  for rewriting
 RETURNS: NULL, if the tautology check for literal <i> in <RuleClause>
	  failed.

	  If the test succeeds an auxiliary clause is returned that
	  contains part of the splitting information for the current
	  rewriting step. If the 'DocProof' flag is set, the necessary
	  parent information is set, too.
 MEMORY:  Remember to delete the returned clause!
 ***************************************************************/
{
  FLAGSTORE Flags;
  PRECEDENCE Precedence;
  CLAUSE aux, NewClause;
  LITERAL Lit;
  TERM Atom;
  BOOL DocProof, Negative, Redundant;
  LIST NegLits, PosLits, RedundantList;
  int OrigNum;

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
  DocProof = flag_GetFlagIntValue(Flags, flag_DOCPROOF);

  Lit = clause_GetLiteral(RuleClause, i);
  Atom = clause_LiteralAtom(Lit);
  Negative = clause_LiteralIsNegative(Lit);

#ifdef CHECK
  if ((flag_GetFlagIntValue(Flags, flag_RFREW)> flag_RFREWON) ||
      (flag_GetFlagIntValue(Flags, flag_RBREW)> flag_RBREWON) )
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_UnivRedCRwClauseTautologyCheck: Illegal flags.\n");
      misc_FinishErrorReport();
    }

#endif

#ifdef CRW_CHECK
  printf("\n  ----------\n  ");
  if (Negative)
  printf((i <= clause_LastConstraintLitIndex(RuleClause)) ? "Cons" : "Ante");
  else
  printf("Succ");
  printf(" aux = ");
#endif

  if (i <= clause_LastConstraintLitIndex(RuleClause))
    {

      /* Apply Sort Simplification for constraint literals only */
      NegLits = list_List(term_Copy(Atom));
      aux = clause_Create(NegLits, list_Nil(), list_Nil(), Flags, Precedence);
      clause_SetTemporary(aux);
      list_Delete(NegLits);

#ifdef CRW_CHECK
      clause_Print(aux);
#endif

      NewClause = NULL;
      OrigNum = clause_Number(aux);
      if (red_SortSimplification(prfs_DynamicSortTheory(Search), aux, NAT_MAX,
      DocProof, Flags, Precedence, &NewClause))
        {
          /* Sort Simplification was possible, so the unit clause was reduced */
          /* to the empty clause. */

          /* The splitting information is already set in <aux> or <NewClause>. */
          if (DocProof)
            /* If 'DocProof' is turned on, a copy was created and assigned */
            /* to <NewClause>. */
            red_CRwCalculateAdditionalParents(NewClause, list_Nil(), NULL,
                OrigNum);

          if (NewClause != NULL)
            {
              clause_Delete(aux);
              return NewClause;
            }
          else
            return aux;
        }
      clause_Delete(aux);

#ifdef CRW_CHECK
      printf("\n  Cons aux2 = ");
#endif
    }

  /* Collect literals for tautology test */
  if (Negative)
    {
      if (i <= clause_LastConstraintLitIndex(RuleClause))
        NegLits = clause_CopyConstraint(RedClause);
      else
        NegLits = clause_CopyAntecedentExcept(RedClause, Except);
      PosLits = list_List(term_Copy(Atom));
    }
  else
    {
      NegLits = list_List(term_Copy(Atom));
      PosLits = clause_CopySuccedentExcept(RedClause, Except);
    }

  /* Create clause for tautology test */
  aux = clause_Create(list_Nil(), NegLits, PosLits, Flags, Precedence);
  clause_SetTemporary(aux);
  list_Delete(NegLits);
  list_Delete(PosLits);

#ifdef CRW_CHECK
  clause_Print(aux);
#endif

  /* Apply special reduction. Propagate definitions x=t if for all literals  */
  /* u=v (A=tt) of the resulting clause the conditions holds:                */
  /* LeadingTerm > u{x->t} and LeadingTerm > v{x->t} (LeadingTerm > A{x->t}. */
  if (DefPropagation && red_PropagateDefinitions(aux, LeadingTerm, Flags,
      Precedence))
    {
#ifdef CRW_DEBUG
      printf("\n  After propagation of definitions:\n  aux = ");
      clause_Print(aux);
#endif
    }

  /* Invoke forward reduction and tautology test */
  NewClause = NULL;
  RedundantList = list_Nil();
  OrigNum = clause_Number(aux);
  Redundant = FALSE;

  if (list_Empty(Blocked))
    {
      Redundant = red_SelectedStaticReductions(Search, &aux, &NewClause,
          &RedundantList, Mode);
    }

  clause_SetTemporary(aux);
  /* <aux> was possibly changed by some reductions, so mark it as */
  /* temporary again. */

  /* Invoke tautology test if <aux> isn't redundant. */
  clause_PrecomputeOrderingAndReInitSkolem(aux, Flags, Precedence);
  if (Redundant || (!clause_IsEmptyClause(aux) &&
  /* (cc_Tautology(aux) || -- this is done in red_CRwRecursiveTautology if -RTaut=2*/
	/* Call Recursive Contextual Rewriting */
  	red_CRwGroundSubtermRedundant(Search, aux, RewParentCls, RewParentLits, Blocked,
      0)))
    {

      if (NewClause != NULL)
        /* <aux> is subsumed by <NewClause> */
        clause_UpdateSplitDataFromPartner(aux, NewClause);

      if (DocProof)
        red_CRwCalculateAdditionalParents(aux, RedundantList, NewClause,
            OrigNum);
    }
  else
    {
      /* test failed */
      clause_Delete(aux);
      aux = NULL;
    }

#ifdef CRW_DEBUG
  if (aux != NULL)
    {
      if (NewClause != NULL)
        {
          printf("\n  Subsumer = ");
          clause_Print(NewClause);
        }
      if (!list_Empty(RedundantList))
        {
          printf("\n  RedundantList: ");
          clause_ListPrint(RedundantList);
        }

      printf("\n  aux reduced = ");
      clause_Print(aux);
    }
  printf("\n  ----------");
#endif

  /* Delete list of redundant clauses */
  clause_DeleteClauseList(RedundantList);

  return aux;
}

static BOOL
red_CRwSideCondSubtermContextualRewriting(PROOFSEARCH Search, CLAUSE RedClause, int i,
    			    TERM TermSInstance, CLAUSE RuleCopy, int j,
			    NAT Mode, CLAUSE *Result,
    			    BOOL DefPropagation, LIST *RewParentCls,
			    LIST* RewParentLits, LIST Blocked)
/**************************************************************
 INPUT:   A proof search object, two clauses, two literal indices
 	  (one per clause), <TermSInstance> is a subterm of
 	  literal <i> in <RedClause>, a mode defining the clause
	  index used for intermediate reductions, a pointer
	  to a clause used as return value,
	  a flag indicating if red_DefinitionPropagation is applied,
	  a pointer to a list where all parent clauses are stored
	  that are used for rewriting and
	  a pointer to a list where all literals are stored that are used
	  for rewriting
	  a pointer to a list where all clauses in this list are not
	  considered for recursive application of contextual rewriting
 RETURNS: FALSE, if the clauses failed some validity test
	  In this case <Result> is set to NULL.

 	  TRUE is returned if  all validity tests are passed
	  In some cases <Result> is set to some auxiliary clause.
	  This is done if some clauses from the index were used to
	  reduce the intermediate clauses before the validity test.
	  The auxiliary clause is used to return the necessary splitting
	  information for the current rewriting step.
	  If the <DocProof> flag is true, the information about
	  parent clauses is set in <Result>, too.
 EFFECT:  parents of RuleCopy is changend
 MEMORY:  Remember to delete the <Result> clause if it is not NULL.
 ASSUME:  literal <i> in <RedClause> is greater than literal <j>
	  in <RuleClause>.
 ***************************************************************/
{
  FLAGSTORE Flags, BackupFlags;
  PRECEDENCE Precedence;
  CLAUSE aux;
  int last, h, OldClauseCounter;
  BOOL Rewrite, Document;

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
  *Result = NULL;

  Document = flag_GetFlagIntValue(Flags, flag_DOCPROOF);

  /* Create new flag store and save current settings.  Must be improved ****     */
  /* Then turn off flags for printing and contextual rewriting. */
  /* IMPORTANT: the DocProof flag mustn't be changed!       */
  BackupFlags = flag_CreateStore();
  flag_TransferAllFlags(Flags, BackupFlags);
#ifndef CRW_DEBUG
  flag_ClearPrinting(Flags);
  if (flag_GetFlagIntValue(BackupFlags, flag_PREW)) /* if PREW was set turn on */
    flag_SetFlagIntValue(Flags, flag_PREW, flag_PREWDET);
#else
    { /* HACK: turn on all printing flags for debugging */
      FLAG_ID f;

      for (f = (FLAG_ID) 0; f < flag_MAXFLAG; f++)
        {
          if (flag_IsPrinting(f))
          flag_SetFlagIntValue(Flags, f, flag_ON);
        }
    }
#endif

  /* ATTENTION: we do not apply contextual rewriting recursively*/
  /* because termination is not guaranteed in this procedure */
  flag_SetFlagIntValue(Flags, flag_RBREW, flag_RBREWON);
  flag_SetFlagIntValue(Flags, flag_RFREW, flag_RFREWON);

  if (Document)
    {
      /* Clear Parent information of <RuleCopy>, because
       * the rule clauses used during the recursive call
       * are collected in the parent list of <RuleCopy> */
      list_Delete(clause_ParentClauses(RuleCopy));
      list_Delete(clause_ParentLiterals(RuleCopy));
      clause_SetParentClauses(RuleCopy, list_Nil());
      clause_SetParentLiterals(RuleCopy, list_Nil());
    }

  /* Examine all literals of <RuleCopy> except <j> */
  Rewrite = TRUE;
  last = clause_LastLitIndex(RuleCopy);
  OldClauseCounter = clause_Counter();

  for (h = clause_FirstLitIndex(); Rewrite && h <= last; h++)
    {
      if (h != j)
        {
          aux = red_CRwLitCondSubtermContextualRewriting(Search, RedClause, i, RuleCopy,
              h, TermSInstance, DefPropagation, Mode, RewParentCls,
              RewParentLits, Blocked);
          if (aux == NULL)
            Rewrite = FALSE;
          else
            {
              /* Store splitting data of <aux> in RuleCopy */
              clause_UpdateSplitDataFromPartner(RuleCopy, aux);
              /* Collect additonal parent information, if <DocProof> is turned on */
              if (flag_GetFlagIntValue(Flags, flag_DOCPROOF))
                {

                  clause_SetParentClauses(RuleCopy,
                      list_Nconc(clause_ParentClauses(aux),
                          clause_ParentClauses(RuleCopy)));
                  clause_SetParentLiterals(RuleCopy, list_Nconc(
                      clause_ParentLiterals(aux), clause_ParentLiterals(
                          RuleCopy)));
                  clause_SetParentClauses(aux, list_Nil());
                  clause_SetParentLiterals(aux, list_Nil());
                }
              clause_Delete(aux);
            }
        }
    }
  /* restore clause counter */
  clause_SetCounter(OldClauseCounter);

  /* reset flag store of proof search object and free backup store */
  flag_TransferAllFlags(BackupFlags, Flags);
  flag_DeleteStore(BackupFlags);

  if (Rewrite)
    *Result = RuleCopy;

  return Rewrite;
}

static BOOL
red_NegativeContextualRewriting(PROOFSEARCH Search, CLAUSE RedClause,
    TERM RedTermS, int ri, CLAUSE RuleClause, TERM TermS, int i, NAT Mode,
    CLAUSE *HelpClause, LIST *RewParentCls, LIST* RewParentLits)
/*************************************************************
 * INPUT:  Two Clauses, two literal indexes,
 *         a subterm <RedTermS> of <RedClause>,
 *         a subterm <TermS> of <RuleClause>,
 *         the reduction mode defining the clause index used for intermediate reductions,
 *         a Clause pointer <HelpClause> where split level information
 *         is stored
 *         a list pointer <RewParentCls> where clauses are stored that
 *         are used for validity check
 *         a list pointer <RewParentLits> where literals are stored that
 *         are used for validity check
 * RETURN: TRUE, if Negative Contextual Rewriting of <RedClause>
 *         using <RuleClause> was possible
 * EFFECT: If Negative Contextual Rewriting was possible <RedClause>
 *         is destructively changed
 * ASSUME: Literal <i> of <RuleClause> is a negative equation
 *************************************************************/
{
  LITERAL RuleLit;
  TERM RuleEq, RedAtom;
  FLAGSTORE Flags;
  PRECEDENCE Precedence;
  BOOL Result;
  int h, last;

  RuleLit = clause_GetLiteral(RuleClause, i);
  RuleEq = clause_LiteralAtom(RuleLit);
  RedAtom = clause_GetLiteralAtom(RedClause, ri);

#ifdef CHECK
  if(clause_LiteralIsPositive(RuleLit) || !(fol_IsEquality(RuleEq)) ||
      !fol_IsEquality(RedAtom))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_NegativeContextualRewriting: Illegal input.\n");
      misc_FinishErrorReport();
    }
#endif

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
  Result = FALSE;

  if (clause_LiteralIsNegative(RuleLit) && clause_LiteralIsPositive(
      clause_GetLiteral(RedClause, ri)) && clause_LiteralIsEquality(RuleLit)
      && clause_LiteralIsEquality(clause_GetLiteral(RedClause, ri)) &&
  /* do not consider subterms */
  (term_FirstArgument(RedAtom) == RedTermS || term_SecondArgument(RedAtom)
      == RedTermS) &&
  /* top symbols of left and right sides have to match */
  ((term_TopSymbol(term_FirstArgument(RedAtom)) == term_TopSymbol(
      term_FirstArgument(RuleEq)) && term_TopSymbol(
      term_SecondArgument(RedAtom)) == term_TopSymbol(term_SecondArgument(
      RuleEq))) || (term_TopSymbol(term_SecondArgument(RedAtom))
      == term_TopSymbol(term_FirstArgument(RuleEq)) && term_TopSymbol(
      term_FirstArgument(RedAtom)) == term_TopSymbol(
      term_SecondArgument(RuleEq)))))
    {

      CLAUSE RuleCopy;
      TERM RuleLitCopyAtom, TermSCopy, Right, RightCopy;
      Right = NULL;

#ifdef CRW_DEBUG
      printf("\n NegCRw: \n%d ", i); clause_Print(RuleClause);
      printf("\n%d ", ri); clause_Print(RedClause);
#endif

      /* copy <RuleClause> and rename variables in copy */
      RuleCopy = clause_Copy(RuleClause);
      clause_RenameVarsBiggerThan(RuleCopy, clause_MaxVar(RedClause));

      /* Remove parent information of copied clause and mark it as temporary */
      list_Delete(clause_ParentClauses(RuleCopy));
      clause_SetParentClauses(RuleCopy, list_Nil());
      list_Delete(clause_ParentLiterals(RuleCopy));
      clause_SetParentLiterals(RuleCopy, list_Nil());
      clause_SetTemporary(RuleCopy);

      /* what is the left and what is the right side? */
      if (TermS == term_FirstArgument(RuleEq))
        {
          TermSCopy = term_FirstArgument(clause_GetLiteralAtom(RuleCopy, i));
          Right = term_SecondArgument(RuleEq);
          RightCopy = term_SecondArgument(clause_GetLiteralAtom(RuleCopy, i));

        }
      else
        {
          TermSCopy = term_SecondArgument(clause_GetLiteralAtom(RuleCopy, i));
          Right = term_FirstArgument(RuleEq);
          RightCopy = term_FirstArgument(clause_GetLiteralAtom(RuleCopy, i));
        }
#ifdef CRW_DEBUG
      printf("\nTermS: "); term_Print(TermS); fflush(stdout);
      printf("\nRight: "); term_Print(Right); fflush(stdout);
#endif

      if (!ord_CompareVarsSubset(TermSCopy, RightCopy))
        {
          clause_Delete(RuleCopy);
          return FALSE;
        }

      /* unify */
      cont_StartBinding();

      if (!unify_MatchBindings(cont_LeftContext(), TermSCopy, RedTermS))
        {
#ifdef CHECK
          cont_PrintCurrentTrail();
          misc_StartErrorReport();
          misc_ErrorReport("\n In red_CRwRecursiveTautologyCheck: terms aren't matchable");
          misc_FinishErrorReport();
#endif
        }

      /* apply context to RuleLit */
      cont_ApplyBindingsModuloMatching(cont_LeftContext(),
          clause_GetLiteralAtom(RuleCopy, i), TRUE);

      RuleLitCopyAtom = clause_GetLiteralAtom(RuleCopy, i);
      /* check if terms are equal */
      if ((term_Equal(term_FirstArgument(RedAtom), term_FirstArgument(
          RuleLitCopyAtom)) && term_Equal(term_SecondArgument(RedAtom),
          term_SecondArgument(RuleLitCopyAtom))) || (term_Equal(
          term_FirstArgument(RedAtom), term_SecondArgument(RuleLitCopyAtom))
          && term_Equal(term_SecondArgument(RedAtom), term_FirstArgument(
              RuleLitCopyAtom))))
        {

          /* Apply bindings to the rest of <RuleCopy> */
          last = clause_LastLitIndex(RuleCopy);
          for (h = clause_FirstLitIndex(); h <= last; h++)
            {
              if (h != i)
                cont_ApplyBindingsModuloMatching(cont_LeftContext(),
                    clause_GetLiteralAtom(RuleCopy, h), FALSE);
            }

          cont_BackTrack();
          Result = red_CRwSideCondSubtermContextualRewriting(Search, RedClause, ri, RedTermS,
              RuleCopy, i, Mode, HelpClause, FALSE, RewParentCls,
              RewParentLits, list_Nil());

        }
      else
        {
          cont_BackTrack();
        }

      if (!Result)
        clause_Delete(RuleCopy);
    }
  return Result;
}

BOOL red_UnivRedExistVarBoundInContextExcept(CLAUSE Clause, int Except, VARCONT context)
/**************************************************************
 INPUT:   A clause, an integer and a context
 RETURNS: TRUE, if it exists a variable in <Clause> that is
 	  bound in the <context>
 ***************************************************************/
{
  int i;
  LIST vars, Scan;

  vars = list_Nil();

  for (i = clause_FirstLitIndex(); i <= clause_LastLitIndex(Clause); i++)
    {
      if (i != Except)
        {
          TERM t;

          t = clause_GetLiteralAtom(Clause, i);
          vars = list_Nconc(term_ListOfVariables(t), vars);
        }
    }

  for (Scan = vars; !list_Empty(Scan); Scan = list_Cdr(Scan))
    {
      TERM t;

      t = (TERM) list_Car(Scan);
      if (cont_VarIsBound(context, term_TopSymbol(t)))
        {
          list_Delete(vars);
          return TRUE;
        }

    }
  list_Delete(vars);
  return FALSE;
}

static BOOL
red_CRwSubtermContextualRewriting(PROOFSEARCH Search, CLAUSE RedClause, NAT Mode,
    int Level, CLAUSE *Changed)
/**************************************************************
 INPUT:   A proof search object, a clause to reduce, the
 	  reduction mode which defines the clause set used for
 	  reduction, a split level indicating the need of a copy
 	  if <Clause> is reduced by a clause of higher split level
 	  than <Level>, and a pointer to a clause used as return value.
 RETURNS: TRUE, if contextual rewriting was possible, FALSE otherwise.
 	  If rewriting was possible and the <DocProof> flag is true
 	  or the split level of the rewrite rule is higher than
 	  <Level>, a copy of <RedClause> that is rewritten wrt.
	  the indexed clauses is returned in <*Changed>.
	  Otherwise the clause is destructively rewritten and
	  returned.
 CAUTION: If rewriting wasn't applied, the value of <*Changed>
	  isn't set explicitely in this function.
 ***************************************************************/
{
  TERM RedAtom, RedTermS;
  int B_Stack;
  int ri, first, last, h;
  BOOL Rewritten, Result, Document, Oriented, Valid;
  TERM TermS, PartnerEq, TermSCopy, Right, RightCopy;
  LIST Gen, EqScan, LitScan;
  CLAUSE Copy, RuleCopy;
  FLAGSTORE Flags;
  PRECEDENCE Precedence;
  SHARED_INDEX ShIndex;

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
#ifdef CHECK
  if (!clause_IsClause(RedClause, Flags, Precedence))
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_UnivRedContextualRewriting: Illegal input.\n");
      misc_FinishErrorReport();
    }
  clause_Check(RedClause, Flags, Precedence);
#endif
  /* Select clause index */
  if (red_WorkedOffMode(Mode))
    ShIndex = prfs_WorkedOffSharingIndex(Search);
  else
    ShIndex = prfs_UsableSharingIndex(Search);

  first = clause_FirstAntecedentLitIndex(RedClause);
  last = clause_LastSuccedentLitIndex(RedClause);
  Document = flag_GetFlagIntValue(Flags, flag_DOCPROOF);

  Result = FALSE;
  Copy = RedClause;

  /* Don't apply this rule on constraint literals! */
  for (ri = clause_FirstAntecedentLitIndex(RedClause); ri <= last && ri
      >= first; ri++)
    {
      if (!list_Empty(term_ArgumentList(clause_GetLiteralAtom(Copy, ri))))
        {
          Rewritten = TRUE;
          while (Rewritten && ri >= first)
            {
              Rewritten = FALSE;
              RedAtom = clause_GetLiteralAtom(Copy, ri);
              B_Stack = stack_Bottom();
              /* push subterms on stack except variables */
              sharing_PushListReverseOnStack(term_ArgumentList(RedAtom));

              while (!stack_Empty(B_Stack))
                {
                  RedTermS = (TERM) stack_PopResult();
                  Gen = st_GetGen(cont_LeftContext(), sharing_Index(ShIndex),
                      RedTermS);
                  for (; !list_Empty(Gen) && !Rewritten; Gen = list_Pop(Gen))
                    {
                      TermS = list_Car(Gen);

                      /* A variable can't be greater than any other term, */
                      /* so don't consider any variables here.            */
                      if (!term_IsVariable(TermS))
                        {
                          EqScan = term_SupertermList(TermS);

                          for (; !list_Empty(EqScan) && !Rewritten; EqScan
                              = list_Cdr(EqScan))
                            {
                              PartnerEq = list_Car(EqScan);
                              if (fol_IsEquality(PartnerEq)
                                  && ((term_FirstArgument(PartnerEq) == TermS)
                                      || (term_SecondArgument(PartnerEq)
                                          == TermS)))
                                {
                                  CLAUSE RuleClause, HelpClause;
                                  LIST RewParentCls, RewParentLits;
                                  LITERAL RuleLit;
                                  int i;

                                  for (LitScan = sharing_NAtomDataList(
                                      PartnerEq); !list_Empty(LitScan)
                                      && !Rewritten; LitScan
                                      = list_Cdr(LitScan))
                                    {
                                      RuleLit = list_Car(LitScan);
                                      RuleClause = clause_LiteralOwningClause(
                                          RuleLit);
                                      i = clause_LiteralGetIndex(RuleLit);
                                      HelpClause = NULL;
                                      Oriented = FALSE;
                                      Valid = FALSE;
                                      RewParentCls = list_Nil();
                                      RewParentLits = list_Nil();

                                      if (clause_LiteralIsPositive(RuleLit)
                                          && clause_LiteralIsEquality(RuleLit) &&
                                      /* if RuleLit is oriented then TermS has
                                       * to be the left side of the equation,
                                       * namely the bigger term */
                                      (!clause_LiteralIsOrientedEquality(
                                          RuleLit) || (term_FirstArgument(
                                          PartnerEq) == TermS))
                                      )
                                        {

                                          /* copy <RuleClause> and rename variables in copy */
                                          RuleCopy = clause_Copy(RuleClause);
                                          clause_RenameVarsBiggerThan(RuleCopy,
                                              clause_MaxVar(Copy));

                                          /* what is the left/right side */
                                          if (TermS == term_FirstArgument(
                                              PartnerEq))
                                            {
                                              TermSCopy = term_FirstArgument(
                                                  clause_GetLiteralAtom(
                                                      RuleCopy, i));
                                              Right = term_SecondArgument(
                                                  PartnerEq);
                                              RightCopy = term_SecondArgument(
                                                  clause_GetLiteralAtom(
                                                      RuleCopy, i));
                                            }
                                          else
                                            {
                                              TermSCopy = term_SecondArgument(
                                                  clause_GetLiteralAtom(
                                                      RuleCopy, i));
                                              Right = term_FirstArgument(
                                                  PartnerEq);
                                              RightCopy = term_FirstArgument(
                                                  clause_GetLiteralAtom(
                                                      RuleCopy, i));
                                            }

                                          /* Remove parent information of copied clause and mark it as temporary */
                                          list_Delete(clause_ParentClauses(
                                              RuleCopy));
                                          clause_SetParentClauses(RuleCopy,
                                              list_Nil());
                                          list_Delete(clause_ParentLiterals(
                                              RuleCopy));
                                          clause_SetParentLiterals(RuleCopy,
                                              list_Nil());
                                          clause_SetTemporary(RuleCopy);

                                          /* Check if Literal <j> of <RuleClause> is Oriented and left term is the matching term */
                                          Oriented
                                              = (clause_LiteralIsOrientedEquality(
                                                  RuleLit)
                                                  && term_FirstArgument(
                                                      PartnerEq) == TermS);

                                          /* establish bindings */
                                          cont_StartBinding();

                                          if (!unify_MatchBindings(
                                              cont_LeftContext(), TermSCopy,
                                              RedTermS))
                                            {
#ifdef CHECK
                                              cont_PrintCurrentTrail();
                                              misc_StartErrorReport();
                                              misc_ErrorReport("\n In red_UnivRedContextualRewriting: terms aren't matchable");
                                              misc_FinishErrorReport();
#endif
                                            }

                                          /* try to orient equation in the context */
                                          if (!Oriented
                                              && !clause_LiteralIsOrientedEquality(
                                                  RuleLit)
                                              && ord_CompareVarsSubset(
                                                  TermSCopy, RightCopy))
                                            {

                                              Oriented = ord_ContGreater(
                                                  cont_LeftContext(),
                                                  TermSCopy,
                                                  cont_LeftContext(),
                                                  RightCopy, Flags, Precedence);

                                            }

                                          if (Oriented)
                                            {
                                              Valid = FALSE;

                                              /* Apply bindings to equation s=t, where s > t. Here the strict version */
                                              /* of cont_Apply... can be applied, because all variables in s and t    */
                                              /* are bound. */
                                              cont_ApplyBindingsModuloMatching(
                                                  cont_LeftContext(),
                                                  clause_GetLiteralAtom(
                                                      RuleCopy, i), TRUE);

                                              /* Check if Unit Rewriting is possible */
                                              if ((clause_Length(Copy) == 1)
                                                  && (clause_Length(RuleCopy)
                                                      == 1))
                                                {
                                                  HelpClause = RuleCopy;
                                                  Valid = TRUE;
                                                  cont_BackTrack();

                                                }
                                              /* Check if Non-Unit Rewriting is possible */
                                              else if ((clause_Length(Copy)
                                                  != 1) && subs_SubsumesBasic(
                                                  RuleCopy, Copy, i, ri))
                                                {
                                                  HelpClause = RuleCopy;
                                                  Valid = TRUE;
                                                  cont_BackTrack();
                                                }

                                              /* Check if Contextual Rewriting is possible */
                                              else if(clause_LiteralGetFlag(RuleLit, STRICTMAXIMAL)
                                                      /* Consider only strongly universally
                                                       * reductive clauses as rule clauses */
                                                        && clause_VarsOfClauseAreInTerm(
                                                                RuleClause, TermS) )
                                                {
                                                  /* Check whether E > (s=t)sigma. It suffices to check only positive */
                                                  /* equations. All other cases imply the condition. */
                                                  if (ord_LiteralCompare(
                                                      clause_GetLiteralTerm(
                                                          Copy, ri),
                                                      clause_LiteralGetOrderStatus(
                                                          clause_GetLiteral(
                                                              Copy, ri)),
                                                      clause_GetLiteralTerm(
                                                          RuleCopy, i),
                                                      clause_LiteralGetOrderStatus(
                                                          clause_GetLiteral(
                                                              RuleCopy, i)),                                                      
                                                      TRUE, Flags, Precedence)
                                                      != ord_GREATER_THAN)
                                                    {

                                                      cont_BackTrack();
                                                    }
                                                  else
                                                    {
                                                      int lastlit;
                                                      /* Apply bindings to the rest of <RuleCopy> */
                                                      lastlit
                                                          = clause_LastLitIndex(
                                                              RuleCopy);
                                                      for (h
                                                          = clause_FirstLitIndex(); h
                                                          <= lastlit; h++)
                                                        {
                                                          if (h != i)
                                                            cont_ApplyBindingsModuloMatching(
                                                                cont_LeftContext(),
                                                                clause_GetLiteralAtom(
                                                                    RuleCopy, h),
                                                                FALSE);
                                                        }
                                                      if (!red_TermIsStrictlyMaximalTerm(
                                                          RuleCopy, i,
                                                          TermSCopy, Flags,
                                                          Precedence))
                                                        {
                                                          cont_BackTrack();
                                                        }
                                                      else
                                                        {

                                                          /* Backtrack bindings before recursive reduction rules are invoked */
                                                          cont_BackTrack();
#ifdef CRW_DEBUG
                                                          printf("\nUnivRedCRw: \nRuleClause: %d ", i);
                                                          clause_Print(RuleClause);
                                                          printf(" \nRedClause:  %d ", ri);
                                                          clause_Print(Copy);
                                                          printf("\n----------------------------------------------");
#endif

                                                          red_CRW_CALLS++;
                                                          /* Test side conditions of Subterm Contextual Rewriting */
                                                          Valid
                                                              = red_CRwSideCondSubtermContextualRewriting(
                                                                  Search,
                                                                  Copy,
                                                                  ri,
                                                                  RedTermS,
                                                                  RuleCopy,
                                                                  i,
                                                                  Mode,
                                                                  &HelpClause,
                                                                  TRUE,
                                                                  &RewParentCls,
                                                                  &RewParentLits,
                                                                  list_Nil());
                                                          if (Valid)
                                                            red_CRW_SUCCESS++;
                                                        }

                                                    }
                                                }
                                              else
                                                {
                                                cont_BackTrack();
                                                }
                                              }
                                          else
                                            { /* if not oriented Backtrack last binding */
                                              /* printf("----> Not Oriented."); */
                                              cont_BackTrack();
                                            }
					  /* if side conditions of CRW are fulfilled */
                                          if (Valid)
                                            {
                                              TERM TermT;

                                              /* collect split information */
					      if (RedClause == Copy
                                                  && (Document
                                                      || prfs_SplitLevelCondition(
                                                          clause_SplitLevel(
                                                              RuleClause),
                                                          clause_SplitLevel(
                                                              RedClause), Level)
                                                      || prfs_SplitLevelCondition(
                                                          clause_SplitLevel(
                                                              HelpClause),
                                                          clause_SplitLevel(
                                                              RedClause), Level)))
                                                {

                                                  Copy = clause_Copy(RedClause);
                                                  RedAtom
                                                      = clause_GetLiteralAtom(Copy, ri);
                                                }

                                              if (!Result && 
						  flag_GetFlagIntValue(Flags, flag_PREW) >= flag_ON)
                                                {
                                                  /* Clause is rewritten for the first time and */
                                                  /* printing is turned on. */
                                                  fputs(
                                                      "\nFSubtermContRewriting: ",
                                                      stdout);
                                                  clause_Print(Copy);
                                                  fputs(" ==>[ ", stdout);
                                                }

					      /* document proof steps if <Document> is set */
                                              if (Document)
                                                {
                                                  LIST PClauses, PLits;

                                                  /* Get additional parent information from */
                                                  /* <HelpClause> */
                                                  PClauses = PLits = list_Nil();
                                                  if (HelpClause != NULL)
                                                    {
                                                      PClauses = list_Nconc(
                                                          clause_ParentClauses(
                                                              HelpClause),
                                                          RewParentCls);
                                                      PLits
                                                          = list_Nconc(
                                                              clause_ParentLiterals(
                                                                  HelpClause),
                                                              RewParentLits);
                                                      clause_SetParentClauses(
                                                          HelpClause,
                                                          list_Nil());
                                                      clause_SetParentLiterals(
                                                          HelpClause,
                                                          list_Nil());
                                                    }
                                                  else
                                                    {
                                                      PClauses = RewParentCls;
                                                      PLits = RewParentLits;
                                                    }

                                                  if (!Result)
                                                    red_DocumentContextualRewriting(
                                                        Copy, ri, RuleClause,
                                                        i, PClauses, PLits);
                                                  else
                                                    red_DocumentFurtherCRw(
                                                        Copy, ri, RuleClause,
                                                        i, PClauses, PLits);
                                                }
                                              Result = TRUE;

					      /* Rewrite Term */
                                              cont_StartBinding();
                                              unify_MatchBindings(
                                                  cont_LeftContext(), TermS,
                                                  RedTermS);
                                              TermT
                                                  = cont_ApplyBindingsModuloMatching(
                                                      cont_LeftContext(),
                                                      term_Copy(Right), TRUE);
                                              cont_BackTrack();

                                              term_ReplaceSubtermBy(RedAtom,
                                                  RedTermS, TermT);
                                              Rewritten = TRUE;
                                              /* Set splitting data from parents */
                                              clause_UpdateSplitDataFromPartner(
                                                  Copy, RuleClause);
                                              if (HelpClause != NULL)
                                                {
                                                  /* Store splitting data from intermediate clauses */
                                                  clause_UpdateSplitDataFromPartner(
                                                      Copy, HelpClause);
                                                  /* clause_Delete(HelpClause); */
                                                }
                                              term_Delete(TermT);
                                              stack_SetBottom(B_Stack);

                                              if (flag_GetFlagIntValue(Flags, flag_PREW) >= flag_ON)
                                                {
                                                  printf(
                                                      "%zd.%d ",
                                                      clause_Number(RuleClause),
                                                      i);
                                                  fflush(stdout);
                                                }
                                              clause_UpdateWeight(Copy, Flags);

                                            }
                                          else
                                            {
                                              list_Delete(RewParentCls);
                                              list_Delete(RewParentLits);
                                            }
                                          clause_Delete(RuleCopy);

                                        }
                                      /* Check if Subterm Contextual Literal Elimination can be applied */
                                      else if ((flag_GetFlagIntValue(Flags,
                                          flag_RFREW) == flag_RFREWRNEGCRW)
                                          && clause_LiteralIsNegative(RuleLit)
                                          && clause_LiteralIsEquality(RuleLit)
                                          && clause_LiteralIsPositive(
                                              clause_GetLiteral(Copy, ri))
                                          && clause_LiteralIsEquality(
                                              clause_GetLiteral(Copy, ri))
                                          && red_NegativeContextualRewriting(
                                              Search, Copy, RedTermS, ri,
                                              RuleClause, TermS, i, Mode,
                                              &HelpClause, &RewParentCls,
                                              &RewParentLits))
                                        {

                                          if (RedClause == Copy
                                              && (Document
                                                  || prfs_SplitLevelCondition(
                                                      clause_SplitLevel(
                                                          RuleClause),
                                                      clause_SplitLevel(
                                                          RedClause), Level)
                                                  || prfs_SplitLevelCondition(
                                                      clause_SplitLevel(
                                                          HelpClause),
                                                      clause_SplitLevel(
                                                          RedClause), Level)))
                                            {
                                              Copy = clause_Copy(RedClause);
                                            }

                                          if (!Result && flag_GetFlagIntValue(Flags, flag_PREW) >= flag_ON)
                                            {

                                              /* Clause is rewritten for the first time and */
                                              /* printing is turned on. */
                                              fputs(
                                                  "\nFSubtermContRewriting: ",
                                                  stdout);
                                              clause_Print(Copy);
                                              fputs(" ==>[ ", stdout);
                                            }
                                          if (Document)
                                            {
                                              LIST PClauses, PLits;

                                              /* Get additional parent information from */
                                              /* <HelpClause> */
                                              PClauses = PLits = list_Nil();
                                              if (HelpClause != NULL)
                                                {
                                                  PClauses = list_Nconc(
                                                      clause_ParentClauses(
                                                          HelpClause),
                                                      RewParentCls);
                                                  PLits = list_Nconc(
                                                      clause_ParentLiterals(
                                                          HelpClause),
                                                      RewParentLits);
                                                  clause_SetParentClauses(
                                                      HelpClause, list_Nil());
                                                  clause_SetParentLiterals(
                                                      HelpClause, list_Nil());
                                                }
                                              else
                                                {
                                                  PClauses = RewParentCls;
                                                  PLits = RewParentLits;
                                                }

                                              if (!Result)
                                                red_DocumentContextualRewriting(
                                                    Copy, ri, RuleClause, i,
                                                    PClauses, PLits);
                                              else
                                                red_DocumentFurtherCRw(Copy,
                                                    ri, RuleClause, i,
                                                    PClauses, PLits);
                                            }
                                          Result = TRUE;
					  /* Delete Literal from Clause */
                                          clause_DeleteLiteral(Copy, ri, Flags,
                                              Precedence);

                                          ri--; /* Literal has been deleted */
                                          last--;/* Hence the clause contains one literal less */
                                          Rewritten = TRUE;

                                          /* Set splitting data from parents */
                                          clause_UpdateSplitDataFromPartner(
                                              Copy, RuleClause);
                                          if (HelpClause != NULL)
                                            {

                                              /* Store splitting data from intermediate clauses */
                                              clause_UpdateSplitDataFromPartner(
                                                  Copy, HelpClause);
                                               clause_Delete(HelpClause);
                                            }
                                          stack_SetBottom(B_Stack);

                                          if (flag_GetFlagIntValue(Flags,flag_PREW) >= flag_ON)
                                            printf("%zd.%d ", clause_Number(
                                                RuleClause), i);
                                          clause_UpdateWeight(Copy, Flags);

                                        }
                                      else
                                        {
                                          list_Delete(RewParentCls);
                                          list_Delete(RewParentLits);
                                        }

                                    }
                                }
                            }
                        }
                    }
                  list_Delete(Gen);
                }
            }
        }
    }
  if (Result)
    {
      clause_PrecomputeOrderingAndReInit(Copy, Flags, Precedence);
      if (flag_GetFlagIntValue(Flags, flag_PREW) >= flag_ON)
        {
          fputs("] ", stdout);
          clause_Print(Copy);
        }
      if (Copy != RedClause)
        {
          clause_PrecomputeOrderingAndReInit(RedClause, Flags, Precedence);
          *Changed = Copy;
        }
    }

#ifdef CHECK
  if (Copy != RedClause)
  clause_Check(Copy, Flags, Precedence);
  clause_Check(RedClause, Flags, Precedence);
#endif

  return Result;
}


static CLAUSE red_CRwLitTautologyCheck(PROOFSEARCH Search, CLAUSE RedClause,
				       int Except, CLAUSE RuleClause, int i,
				       TERM LeadingTerm, NAT Mode)
/**************************************************************
  INPUT:   A proof search object, two clauses, two literal indices
           (one per clause), a mode defining the clause index used
	   for intermediate reductions.
  RETURNS: NULL, if the tautology check for literal <i> in <RuleClause>
           failed.

           If the test succeeds an auxiliary clause is returned that
	   contains part of the splitting information for the current
	   rewriting step. If the 'DocProof' flag is set, the necessary
	   parent information is set, too.
  MEMORY:  Remember to delete the returned clause!
***************************************************************/
{
  FLAGSTORE  Flags;
  PRECEDENCE Precedence;
  CLAUSE     aux, NewClause;
  LITERAL    Lit;
  TERM       Atom;
  BOOL       DocProof, Negative, Redundant;
  LIST       NegLits, PosLits, RedundantList;
  int        OrigNum;
  
  Flags      = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
  DocProof   = flag_GetFlagIntValue(Flags, flag_DOCPROOF);
  
  Lit      = clause_GetLiteral(RuleClause, i);
  Atom     = clause_LiteralAtom(Lit);
  Negative = clause_LiteralIsNegative(Lit);

#ifdef CRW_DEBUG
  printf("\n  ----------\n  ");
  if (Negative)
    printf((i <= clause_LastConstraintLitIndex(RuleClause)) ? "Cons" : "Ante");
  else
    printf("Succ");
  printf(" aux = ");
#endif  

  if (i <= clause_LastConstraintLitIndex(RuleClause)) {

    /* Apply Sort Simplification for constraint literals only */
    NegLits = list_List(term_Copy(Atom));
    aux     = clause_Create(NegLits, list_Nil(), list_Nil(), Flags, Precedence);
    clause_SetTemporary(aux);
    list_Delete(NegLits);

#ifdef CRW_DEBUG
    clause_Print(aux);
#endif
    
    NewClause = NULL;
    OrigNum   = clause_Number(aux);
    if (red_SortSimplification(prfs_DynamicSortTheory(Search), aux, NAT_MAX,
			       DocProof, Flags, Precedence, &NewClause)) {
      /* Sort Simplification was possible, so the unit clause was reduced */
      /* to the empty clause. */

      /* The splitting information is already set in <aux> or <NewClause>. */
      if (DocProof)
	/* If 'DocProof' is turned on, a copy was created and assigned */
	/* to <NewClause>. */
	red_CRwCalculateAdditionalParents(NewClause, list_Nil(), NULL, OrigNum);

      if (NewClause != NULL) {
	clause_Delete(aux);
	return NewClause;
      } else
	return aux;
    }
    clause_Delete(aux);

#ifdef CRW_DEBUG
    printf("\n  Cons aux2 = ");
#endif
  }
  
  /* Collect literals for tautology test */
  if (Negative) {
    if (i <= clause_LastConstraintLitIndex(RuleClause))
      NegLits = clause_CopyConstraint(RedClause);
    else
      NegLits = clause_CopyAntecedentExcept(RedClause, Except);
    PosLits = list_List(term_Copy(Atom));
  } else {
    NegLits = list_List(term_Copy(Atom));
    PosLits = clause_CopySuccedentExcept(RedClause, Except);
  }
  
  /* Create clause for tautology test */
  aux = clause_Create(list_Nil(), NegLits, PosLits, Flags, Precedence);
  clause_SetTemporary(aux);
  list_Delete(NegLits);
  list_Delete(PosLits);
  
#ifdef CRW_DEBUG
  clause_Print(aux);
#endif

  /* Apply special reduction. Propagate definitions x=t if for all literals  */
  /* u=v (A=tt) of the resulting clause the conditions holds:                */
  /* LeadingTerm > u{x->t} and LeadingTerm > v{x->t} (LeadingTerm > A{x->t}. */
  if (red_PropagateDefinitions(aux, LeadingTerm, Flags, Precedence)) {
#ifdef CRW_DEBUG
    printf("\n  After propagation of definitions:\n  aux = ");
    clause_Print(aux);
#endif
  }

  /* Invoke forward reduction and tautology test */
  NewClause     = NULL;
  RedundantList = list_Nil();
  OrigNum       = clause_Number(aux);
  Redundant     = red_SelectedStaticReductions(Search, &aux, &NewClause,
					       &RedundantList, Mode);
  clause_SetTemporary(aux);
  /* <aux> was possibly changed by some reductions, so mark it as */
  /* temporary again. */
  
  /* Invoke tautology test if <aux> isn't redundant. */
  if (Redundant || (!clause_IsEmptyClause(aux) && cc_Tautology(aux))) {

    if (NewClause != NULL)
      /* <aux> is subsumed by <NewClause> */
      clause_UpdateSplitDataFromPartner(aux, NewClause);
    
    if (DocProof)
      red_CRwCalculateAdditionalParents(aux, RedundantList, NewClause, OrigNum);
  } else {
    /* test failed */

    clause_Delete(aux);
    aux = NULL;
  }
  
#ifdef CRW_DEBUG
  if (aux != NULL) {
    if (NewClause != NULL) {
      printf("\n  Subsumer = ");
      clause_Print(NewClause);
    }
    if (!list_Empty(RedundantList)) {
      printf("\n  RedundantList: ");
      clause_ListPrint(RedundantList);
    }
    
    printf("\n  aux reduced = ");
    clause_Print(aux);
  } 
  printf("\n  ----------");
#endif
  
  /* Delete list of redundant clauses */
  clause_DeleteClauseList(RedundantList);
  
  return aux;
}


static BOOL red_CRwTautologyCheck(PROOFSEARCH Search, CLAUSE RedClause, int i,
				  TERM TermSInstance, CLAUSE RuleClause,
				  int j, NAT Mode, CLAUSE *Result)
/**************************************************************
  INPUT:   A proof search object, two clauses, two literal indices
           (one per clause), <TermSInstance> is a subterm of
           literal <i> in <RedClause>, a mode defining the clause
	   index used for intermediate reductions, and a pointer
	   to a clause used as return value.
  RETURNS: FALSE, if the clauses failed some tautology test or
           the literal <i> in <RedClause> is not greater than literal
	   <j> in <RedClause> with the substitution <sigma> applied.
	   In this case <Result> is set to NULL.

	   TRUE is returned if the clauses passed all tautology tests
	   and literal <i> in <RedClause> is greater than literal <j>
	   in <RuleClause> with the substitution <sigma> applied.
	   In some cases <Result> is set to some auxiliary clause.
	   This is done if some clauses from the index were used to
	   reduce the intermediate clauses before the tautology test.
	   The auxiliary clause is used to return the necessary splitting
	   information for the current rewriting step.
	   If the <DocProof> flag is true, the information about
	   parent clauses is set in <Result>, too.
  MEMORY:  Remember to delete the <Result> clause if it is not NULL.
***************************************************************/
{
  FLAGSTORE  Flags, BackupFlags;
  PRECEDENCE Precedence;
  CLAUSE     RuleCopy, aux;
  TERM       TermS;
  int        last, h;
  BOOL       Rewrite;

#ifdef CHECK
  if (!clause_LiteralIsOrientedEquality(clause_GetLiteral(RuleClause, j))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_CRwTautologyCheck:");
    misc_ErrorReport(" literal %d in <RuleClause> %d", j,
		     clause_Number(RuleClause));
    misc_ErrorReport(" isn't an oriented equation");
    misc_FinishErrorReport();
  }
#endif

  Flags      = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
  *Result    = NULL;
  
  /* copy <RuleClause> and rename variables in copy */
  RuleCopy = clause_Copy(RuleClause);
  clause_RenameVarsBiggerThan(RuleCopy, clause_MaxVar(RedClause));
  TermS = term_FirstArgument(clause_GetLiteralAtom(RuleCopy, j));

  /* Remove parent information of copied clause and mark it as temporary */
  list_Delete(clause_ParentClauses(RuleCopy));
  clause_SetParentClauses(RuleCopy, list_Nil());
  list_Delete(clause_ParentLiterals(RuleCopy));
  clause_SetParentLiterals(RuleCopy, list_Nil());
  clause_SetTemporary(RuleCopy);

  /* establish bindings */
  cont_StartBinding();
  if (!unify_MatchBindings(cont_LeftContext(), TermS, TermSInstance)) {
#ifdef CHECK
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_CRwTautologyCheck: terms aren't matchable");
    misc_FinishErrorReport();
#endif
  }

  /* Apply bindings to equation s=t, where s > t. Here the strict version */
  /* of cont_Apply... can be applied, because all variables in s and t    */
  /* are bound. */
  cont_ApplyBindingsModuloMatching(cont_LeftContext(),
				   clause_GetLiteralAtom(RuleCopy, j),
				   TRUE);

  /* Check whether E > (s=t)sigma. It suffices to check only positive */
  /* equations. All other cases imply the condition. */
  if (i >= clause_FirstSuccedentLitIndex(RedClause) &&
      clause_LiteralIsEquality(clause_GetLiteral(RedClause, i)) &&
      ord_LiteralCompare(clause_GetLiteralTerm(RedClause, i),
       clause_LiteralGetOrderStatus(clause_GetLiteral(RedClause, i)),
			 clause_GetLiteralTerm(RuleCopy, j), ord_GREATER_THAN, /* we know it has to be oriented */       
			 FALSE, Flags, Precedence) != ord_GREATER_THAN) {
    cont_BackTrack();
    clause_Delete(RuleCopy);
    return FALSE;
  }

  /*  if (subs_SubsumesBasic(RuleClause, RedClause, j, i)) {    Potential improvement, not completely
    cont_BackTrack();                                           developed ....
    return TRUE;
    } else  */
  {
    int OldClauseCounter;
    /* Apply bindings to the rest of <RuleCopy> */
    last = clause_LastLitIndex(RuleCopy);
    for (h = clause_FirstLitIndex(); h <= last; h++) {
      if (h != j)
	cont_ApplyBindingsModuloMatching(cont_LeftContext(),
					 clause_GetLiteralAtom(RuleCopy, h),
					 FALSE);
    }
    
    /* Backtrack bindings before reduction rules are invoked */
    cont_BackTrack();

    /* Create new flag store and save current settings.  Must be improved ****     */
    /* Then turn off flags for printing and contextual rewriting. */
    /* IMPORTANT: the DocProof flag mustn't be changed!       */
    BackupFlags = flag_CreateStore();
    flag_TransferAllFlags(Flags, BackupFlags);
#ifndef CRW_DEBUG
    flag_ClearPrinting(Flags);
#else
    { /* HACK: turn on all printing flags for debugging */
      FLAG_ID f;
      
      for (f = (FLAG_ID) 0; f < flag_MAXFLAG; f++) {
	if (flag_IsPrinting(f))
	  flag_SetFlagIntValue(Flags, f, flag_ON);
      }
    }
#endif

    /* ATTENTION: to apply CRw recursively, uncomment the following */
    /* line and comment out the following two lines! */
    flag_SetFlagIntValue(Flags, flag_RFREW, flag_ON);
    flag_SetFlagIntValue(Flags, flag_RBREW, flag_ON);
    
    /* Examine all literals of <RuleCopy> except <j> */
    Rewrite          = TRUE;
    last             = clause_LastLitIndex(RuleCopy);
    OldClauseCounter = clause_Counter();

    for (h = clause_FirstLitIndex(); Rewrite && h <= last; h++) {
      if (h != j) {
	aux = red_CRwLitTautologyCheck(Search, RedClause, i, RuleCopy, h,
				       TermSInstance, Mode);
	if (aux == NULL)
	  Rewrite = FALSE;
	else {
	  /* Store splitting data of <aux> in RuleCopy */
	  clause_UpdateSplitDataFromPartner(RuleCopy, aux);
	  /* Collect additonal parent information, if <DocProof> is turned on */
	  if (flag_GetFlagIntValue(Flags, flag_DOCPROOF)) {
	    clause_SetParentClauses(RuleCopy,
				    list_Nconc(clause_ParentClauses(aux),
					       clause_ParentClauses(RuleCopy)));
	    clause_SetParentLiterals(RuleCopy,
				     list_Nconc(clause_ParentLiterals(aux),
						clause_ParentLiterals(RuleCopy)));
	    clause_SetParentClauses(aux, list_Nil());
	    clause_SetParentLiterals(aux, list_Nil());
	  }
	  clause_Delete(aux);
	}
      }
    }
    /* restore clause counter */
    clause_SetCounter(OldClauseCounter);

    /* reset flag store of proof search object and free backup store */
    flag_TransferAllFlags(BackupFlags, Flags);
    flag_DeleteStore(BackupFlags);
  }

  if (Rewrite)
    *Result = RuleCopy;
  else
    /* cleanup */
    clause_Delete(RuleCopy);

  return Rewrite;
}





static BOOL red_ContextualRewriting(PROOFSEARCH Search, CLAUSE RedClause,
				    NAT Mode, int Level, CLAUSE *Changed)
/**************************************************************
  INPUT:   A proof search object, a clause to reduce, the
           reduction mode which defines the clause set used for
	   reduction, a split level indicating the need of a copy
	   if <Clause> is reduced by a clause of higher split level
	   than <Level>, and a pointer to a clause used as return value.
  RETURNS: TRUE, if contextual rewriting was possible, FALSE otherwise.
           If rewriting was possible and the <DocProof> flag is true
	   or the split level of the rewrite rule is higher than
	   <Level>, a copy of <RedClause> that is rewritten wrt.
	   the indexed clauses is returned in <*Changed>.
	   Otherwise the clause is destructively rewritten and
	   returned.
  CAUTION: If rewriting wasn't applied, the value of <*Changed>
           isn't set explicitely in this function.
***************************************************************/
{
  TERM         RedAtom, RedTermS;
  int          B_Stack;
  int          ri, last;
  BOOL         Rewritten, Result, Document;
  TERM         TermS, PartnerEq;
  LIST         Gen, EqScan, LitScan;
  CLAUSE       Copy;
  FLAGSTORE    Flags;
  PRECEDENCE   Precedence;
  SHARED_INDEX ShIndex;

  Flags      = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);

#ifdef CHECK
  if (!clause_IsClause(RedClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_ContextualRewriting: Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RedClause, Flags, Precedence);
#endif
  
  /* Select clause index */
  if (red_WorkedOffMode(Mode))
    ShIndex = prfs_WorkedOffSharingIndex(Search);
  else
    ShIndex = prfs_UsableSharingIndex(Search);

  last     = clause_LastSuccedentLitIndex(RedClause);
  Document = flag_GetFlagIntValue(Flags, flag_DOCPROOF); 

  Result = FALSE;
  Copy   = RedClause;

  /* Don't apply this rule on constraint literals! */
  for (ri = clause_FirstAntecedentLitIndex(RedClause); ri <= last; ri++) {
    if (!list_Empty(term_ArgumentList(clause_GetLiteralAtom(Copy, ri)))) {
      Rewritten = TRUE;
      while (Rewritten) {
	Rewritten = FALSE;
	RedAtom   = clause_GetLiteralAtom(Copy, ri);

	B_Stack = stack_Bottom();
	/* push subterms on stack except variables */
	sharing_PushListReverseOnStack(term_ArgumentList(RedAtom));

	while (!stack_Empty(B_Stack))  {
	  RedTermS = (TERM)stack_PopResult();
	  Gen = st_GetGen(cont_LeftContext(), sharing_Index(ShIndex), RedTermS);
	  
	  for ( ; !list_Empty(Gen) && !Rewritten; Gen = list_Pop(Gen)) {
	    TermS = list_Car(Gen);
	    
	    /* A variable can't be greater than any other term, */
	    /* so don't consider any variables here.            */
	    if (!term_IsVariable(TermS)) {
	      EqScan = term_SupertermList(TermS);
	      
	      for ( ; !list_Empty(EqScan) && !Rewritten;
		    EqScan = list_Cdr(EqScan)) {
		PartnerEq = list_Car(EqScan);
		if (fol_IsEquality(PartnerEq) &&
		    (term_FirstArgument(PartnerEq) == TermS)) {
		  CLAUSE  RuleClause, HelpClause;
		  LITERAL RuleLit;
		  int     i;
		  
		  for (LitScan = sharing_NAtomDataList(PartnerEq); 
		       !list_Empty(LitScan) && !Rewritten;
		       LitScan = list_Cdr(LitScan)) {
		    RuleLit    = list_Car(LitScan);
		    RuleClause = clause_LiteralOwningClause(RuleLit);
		    i         = clause_LiteralGetIndex(RuleLit);
		    HelpClause = NULL;
		    
#ifdef CRW_DEBUG
		    if (clause_LiteralIsPositive(RuleLit) &&
			clause_LiteralGetFlag(RuleLit,STRICTMAXIMAL) &&
			clause_LiteralIsOrientedEquality(RuleLit) &&
			red_LeftTermOfEquationIsStrictlyMaximalTerm(RuleClause,
								    RuleLit,
								    Flags,
								    Precedence)) {
		      printf("\n------\nFCRw: %s\n%d  ", red_WorkedOffMode(Mode)
			     ? "WorkedOff" : "Usable", i);
		      clause_Print(RuleClause);
		      printf("\n%d  ", ri);
		      clause_Print(RedClause);
		    }
#endif

		    if (clause_LiteralIsPositive(RuleLit) &&
			clause_LiteralGetFlag(RuleLit,STRICTMAXIMAL) &&
			clause_LiteralIsOrientedEquality(RuleLit) &&
			red_LeftTermOfEquationIsStrictlyMaximalTerm(RuleClause,
								    RuleLit,
								    Flags,
								    Precedence) &&
			red_CRwTautologyCheck(Search, Copy, ri, RedTermS,
					      RuleClause, i, Mode,
					      &HelpClause)) {
		      TERM   TermT;
		      
		      if (RedClause == Copy &&
			  (Document || 
			   prfs_SplitLevelCondition(clause_SplitLevel(RuleClause),
						    clause_SplitLevel(RedClause),Level) ||
			   prfs_SplitLevelCondition(clause_SplitLevel(HelpClause),
						    clause_SplitLevel(RedClause),
						    Level))) {
			Copy    = clause_Copy(RedClause);
			RedAtom = clause_GetLiteralAtom(Copy, ri);
		      }
		      
		      if (!Result && flag_GetFlagIntValue(Flags, flag_PREW) >= flag_ON) {
			/* Clause is rewitten for the first time and */
			/* printing is turned on. */
			fputs("\nFContRewriting: ", stdout);
			clause_Print(Copy);
			fputs(" ==>[ ", stdout);
		      }
		      
		      if (Document) {
			LIST PClauses, PLits;

			/* Get additional parent information from */
			/* <HelpClause> */
			PClauses = PLits = list_Nil();
			if (HelpClause != NULL) {
			  PClauses = clause_ParentClauses(HelpClause);
			  PLits    = clause_ParentLiterals(HelpClause);
			  clause_SetParentClauses(HelpClause, list_Nil());
			  clause_SetParentLiterals(HelpClause, list_Nil());
			} else
			  PClauses = PLits = list_Nil();

			if (!Result)
			  red_DocumentContextualRewriting(Copy, ri,
							  RuleClause, i,
							  PClauses, PLits);
			else
			  red_DocumentFurtherCRw(Copy, ri, RuleClause, i,
						 PClauses, PLits);
		      }
		      Result = TRUE;
		      
		      cont_StartBinding();
		      unify_MatchBindings(cont_LeftContext(), TermS, RedTermS);
		      TermT = cont_ApplyBindingsModuloMatching(cont_LeftContext(),
							       term_Copy(term_SecondArgument(PartnerEq)),
							       TRUE);
		      cont_BackTrack();
		      
		      term_ReplaceSubtermBy(RedAtom, RedTermS, TermT);
		      Rewritten = TRUE;
		      /* Set splitting data from parents */
		      clause_UpdateSplitDataFromPartner(Copy, RuleClause);
		      if (HelpClause != NULL) {
			/* Store splitting data from intermediate clauses */
			clause_UpdateSplitDataFromPartner(Copy, HelpClause);
			clause_Delete(HelpClause);
		      }
		      term_Delete(TermT);
		      stack_SetBottom(B_Stack);
		      
		      if (flag_GetFlagIntValue(Flags, flag_PREW) >= flag_ON)
			printf("%zd.%d ",clause_Number(RuleClause), i);
		      clause_UpdateWeight(Copy, Flags);
		    }
		  }
		}
	      }
	    }
	  }
	  list_Delete(Gen);
	}
      }
    }
  }
  if (Result) {
    clause_PrecomputeOrderingAndReInit(Copy, Flags, Precedence);
    if (flag_GetFlagIntValue(Flags, flag_PREW) >= flag_ON) {
      fputs("] ", stdout);
      clause_Print(Copy);
    }
    if (Copy != RedClause) {
      clause_PrecomputeOrderingAndReInit(RedClause, Flags, Precedence);
      *Changed = Copy;
    }
  }

#ifdef CHECK
  if (Copy != RedClause)
    clause_Check(Copy, Flags, Precedence);
  clause_Check(RedClause, Flags, Precedence);
#endif

  return Result;
}


static LIST red_BackSubsumption(CLAUSE RedCl, SHARED_INDEX ShIndex,
				FLAGSTORE Flags, PRECEDENCE Precedence)
/**********************************************************
  INPUT:   A pointer to a non-empty clause, an index of 
           clauses, a flag store and a precedence.
  RETURNS: The list of clauses that are subsumed by the 
           clause RedCl.
***********************************************************/
{
  TERM    Atom,CandTerm;
  CLAUSE  SubsumedCl;
  LITERAL CandLit;
  LIST    CandLits, Scan, SubsumedList;
  int     i, j, lc, fa, la, fs, l;
  
#ifdef CHECK
  if (!clause_IsClause(RedCl, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_BackSubsumption :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RedCl, Flags, Precedence);
#endif
  
  /* Special case: clause is empty */
  if (clause_IsEmptyClause(RedCl))
    return list_Nil();

  SubsumedList = list_Nil();
  
  lc = clause_LastConstraintLitIndex(RedCl);
  fa = clause_FirstAntecedentLitIndex(RedCl);
  la = clause_LastAntecedentLitIndex(RedCl);
  fs = clause_FirstSuccedentLitIndex(RedCl);
  l  = clause_LastLitIndex(RedCl);
    
  /* Choose the literal with the greatest weight to start the search */
  i          = clause_FirstLitIndex();
  for (j = i + 1; j <= l; j++) {
    if (clause_LiteralWeight(clause_GetLiteral(RedCl, j)) >
	clause_LiteralWeight(clause_GetLiteral(RedCl, i)))
      i = j;
  }
  
  Atom       = clause_GetLiteralAtom(RedCl, i);
  CandTerm   = st_ExistInstance(cont_LeftContext(), sharing_Index(ShIndex), Atom);
    
  while (CandTerm) {
    CandLits = sharing_NAtomDataList(CandTerm);
      
    for (Scan = CandLits; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
      CandLit    = list_Car(Scan);
      SubsumedCl = clause_LiteralOwningClause(CandLit);
      j          = clause_LiteralGetIndex(CandLit);
	
      if (RedCl != SubsumedCl &&
	  /* Literals must be from same part of the clause */
	  ((i<=lc && clause_LiteralIsFromConstraint(CandLit)) ||
	   (i>=fa && i<=la && clause_LiteralIsFromAntecedent(CandLit)) ||
	   (i>=fs && clause_LiteralIsFromSuccedent(CandLit))) &&
	  !list_PointerMember(SubsumedList, SubsumedCl) &&
	  subs_SubsumesBasic(RedCl, SubsumedCl, i, j))
	SubsumedList = list_Cons(SubsumedCl, SubsumedList);
    }
      
    CandTerm = st_NextCandidate();
  }
    
  if (fol_IsEquality(Atom) && 
      clause_LiteralIsNotOrientedEquality(clause_GetLiteral(RedCl, i))) {
    Atom      = term_Create(fol_Equality(),
			    list_Reverse(term_ArgumentList(Atom)));
    CandTerm  = st_ExistInstance(cont_LeftContext(), sharing_Index(ShIndex), Atom);
      
    while (CandTerm) {
      CandLits = sharing_NAtomDataList(CandTerm);
	
      for (Scan = CandLits; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
	CandLit    = list_Car(Scan);
	SubsumedCl = clause_LiteralOwningClause(list_Car(Scan));
	/* if (!clause_GetFlag(SubsumedCl, BLOCKED)) { */
	j          = clause_LiteralGetIndex(list_Car(Scan));
	  
	if ((RedCl != SubsumedCl) && 
	    /* Literals must be from same part of the clause */
	    ((i<=lc && clause_LiteralIsFromConstraint(CandLit)) ||
	     (i>=fa && i<=la && clause_LiteralIsFromAntecedent(CandLit)) ||
	     (i>=fs && clause_LiteralIsFromSuccedent(CandLit))) &&
	    !list_PointerMember(SubsumedList, SubsumedCl) &&
	    subs_SubsumesBasic(RedCl, SubsumedCl, i, j))
	  SubsumedList = list_Cons(SubsumedCl, SubsumedList);
	/* } */
      }
	
      CandTerm = st_NextCandidate();
    }
      
    list_Delete(term_ArgumentList(Atom));
    term_Free(Atom);
  }
      
  if (flag_GetFlagIntValue(Flags, flag_PSUB)) {
    for (Scan = SubsumedList; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
      SubsumedCl = list_Car(Scan);
      fputs("\nBSubsumption: ", stdout);
      clause_Print(SubsumedCl);
      printf(" by %zd ",clause_Number(RedCl));
    }
  }
  return SubsumedList;
}


static LIST red_GetBackMRResLits(CLAUSE Clause, LITERAL ActLit, SHARED_INDEX ShIndex)
/**************************************************************
  INPUT:   A clause, one of its literals and an Index.
  RETURNS: A list of clauses with a complementary literal instance 
           that are subsumed if these literals are ignored.
	   the empty list if no such clause exists.
  MEMORY:  Allocates the needed listnodes.
***************************************************************/
{
  CLAUSE  PClause;
  LITERAL PLit;
  LIST    LitScan, PClLits;
  TERM    CandTerm;
  int     i;

  PClLits   = list_Nil();
  i         = clause_LiteralGetIndex(ActLit);

  CandTerm  = st_ExistInstance(cont_LeftContext(),
			       sharing_Index(ShIndex),
			       clause_LiteralAtom(ActLit));

  while (CandTerm) {

    LitScan = sharing_NAtomDataList(CandTerm); /* CAUTION ! */

    for ( ; !list_Empty(LitScan); LitScan = list_Cdr(LitScan)) {

      PLit    = list_Car(LitScan);
      PClause = clause_LiteralOwningClause(PLit);

      if (PClause != Clause &&
	  clause_LiteralsAreComplementary(ActLit,PLit) &&
	  subs_SubsumesBasic(Clause,PClause,i,clause_LiteralGetIndex(PLit)))
	PClLits = list_Cons(PLit, PClLits);
    }
    
    CandTerm = st_NextCandidate();
  }
  return PClLits;
}


static LIST red_BackMatchingReplacementResolution(CLAUSE RedClause, SHARED_INDEX ShIndex,
						  FLAGSTORE Flags, PRECEDENCE Precedence,
						  LIST* Result)
/**************************************************************
  INPUT:   A clause, a shared index, a flag store, a 
           precedence, and a pointer to a result list.
  RETURNS: The return value itself contains a list of clauses 
           from <ShIndex> that is reducible by <RedClause> via 
	   clause reduction.
	   The return value stored in <*Result> contains the 
	   result of this operation.
	   If the <DocProof> flag is true then the clauses in 
	   <*Result> contain information about the reduction.
***************************************************************/
{
  LIST   Blocked;
  CLAUSE Copy;
  BOOL   Document;

#ifdef CHECK
  if (!clause_IsClause(RedClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_BackMatchingReplacementResolution:");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RedClause, Flags, Precedence);
#endif

  Blocked = list_Nil();
  Document = flag_GetFlagIntValue(Flags, flag_DOCPROOF);

  if (clause_Length(RedClause) == 1) {
    LITERAL ActLit, PLit;
    LIST    LitList, Scan, Iter;
    TERM    CandTerm;
    intptr_t     RedClNum;
    
    ActLit      = clause_GetLiteral(RedClause, clause_FirstLitIndex());

    if (!fol_IsEquality(clause_LiteralAtom(ActLit)) ||  /* Reduce with negative equations too */
	clause_LiteralIsNegative(ActLit)) {
      CLAUSE PClause;
      LIST   PIndL;

      CandTerm = st_ExistInstance(cont_LeftContext(), sharing_Index(ShIndex), clause_LiteralAtom(ActLit));
      RedClNum = clause_Number(RedClause);
      LitList  = list_Nil();

      while (CandTerm) {
	for (Iter = sharing_NAtomDataList(CandTerm); !list_Empty(Iter); Iter = list_Cdr(Iter))
	  if (clause_LiteralsAreComplementary(ActLit,list_Car(Iter)))
	    LitList = list_Cons(list_Car(Iter),LitList);
	CandTerm = st_NextCandidate();
      }

      /* It is important to get all literals first,
	 because there may be several literals in the same clause which can be reduced by <ActLit> */
      
      while (!list_Empty(LitList)) {
	PLit    = list_Car(LitList);
	PIndL   = list_List(PLit);
	PClause = clause_LiteralOwningClause(PLit);
	Blocked = list_Cons(PClause, Blocked);

	if (flag_GetFlagIntValue(Flags, flag_PMRR)) {
	  fputs("\nBMatchingReplacementResolution: ", stdout);
	  clause_Print(PClause);
	  printf(" ==>[ %zd.%zd ] ",clause_Number(RedClause),clause_FirstLitIndex());
	}

	Iter = LitList;
	for (Scan=list_Cdr(LitList);!list_Empty(Scan);Scan=list_Cdr(Scan)) /* Get brothers of PLit */
	  if (PClause == clause_LiteralOwningClause(list_Car(Scan))) {
	    list_Rplacd(Iter,list_Cdr(Scan));
	    list_Rplacd(Scan,PIndL);
	    PIndL = Scan;
	    Scan  = Iter;
	  }
	  else
	    Iter = Scan;
	Iter    = LitList;
	LitList = list_Cdr(LitList);
	list_Free(Iter);
	Copy    = clause_Copy(PClause);
	clause_RemoveFlag(Copy,WORKEDOFF);
	clause_UpdateSplitDataFromPartner(Copy, RedClause);
	for(Scan=PIndL;!list_Empty(Scan);Scan=list_Cdr(Scan))       /* Change lits to indexes */
	  list_Rplaca(Scan,(POINTER)clause_LiteralGetIndex(list_Car(Scan)));
	clause_DeleteLiterals(Copy, PIndL, Flags, Precedence);
	
	if (Document)
	  /* Lists are consumed */
	  red_DocumentMatchingReplacementResolution(Copy, PIndL, list_List((POINTER)RedClNum),
						    list_List((POINTER)clause_FirstLitIndex()));

	else
	  list_Delete(PIndL);	
	
	if (flag_GetFlagIntValue(Flags, flag_PMRR))
	  clause_Print(Copy);
	*Result = list_Cons(Copy, *Result);
      }
    }
    return Blocked;
  } 
  else {
    CLAUSE  PClause;
    LITERAL ActLit, PLit;
    LIST    LitScan,LitList;
    int     length;
    intptr_t	i,RedClNum,PInd;

    RedClNum    = clause_Number(RedClause);
    length      = clause_Length(RedClause);

    for (i = clause_FirstLitIndex(); i < length; i++) {
      ActLit = clause_GetLiteral(RedClause, i);
      
      if (!fol_IsEquality(clause_LiteralAtom(ActLit))) {
	LitList = red_GetBackMRResLits(RedClause, ActLit, ShIndex);
	
	for (LitScan = LitList;!list_Empty(LitScan);LitScan = list_Cdr(LitScan)) {
	  PLit    = list_Car(LitScan);
	  PClause = clause_LiteralOwningClause(PLit);
	  PInd    = clause_LiteralGetIndex(PLit);
	  Copy    = clause_Copy(PClause);
	  if (list_PointerMember(Blocked,PClause)) {
	    if (!flag_GetFlagIntValue(Flags, flag_DOCPROOF))
	      clause_NewNumber(Copy);
	  }
	  else
	    Blocked = list_Cons(PClause, Blocked);
	  clause_RemoveFlag(Copy,WORKEDOFF);
	  clause_UpdateSplitDataFromPartner(Copy, RedClause);
	  clause_DeleteLiteral(Copy, PInd, Flags, Precedence);

	  if (Document)
	    red_DocumentMatchingReplacementResolution(Copy, list_List((POINTER)PInd), 
						      list_List((POINTER)RedClNum), 
						      list_List((POINTER)i));
	  
	  if (flag_GetFlagIntValue(Flags, flag_PMRR)) {
	    fputs("\nBMatchingReplacementResolution: ", stdout);
	    clause_Print(PClause);
	    printf(" ==>[ %zd.%zd ] ",clause_Number(RedClause),i);
	    clause_Print(Copy);
	  }
	  *Result = list_Cons(Copy, *Result);
	}
	list_Delete(LitList);
      }
    }
    return Blocked;
  }
}


static void red_ApplyRewriting(CLAUSE RuleCl, int ri, CLAUSE PartnerClause,
			       int pli, TERM PartnerTermS, FLAGSTORE Flags,
			       PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause to use for rewriting, the index of a 
           positive equality literal where the first equality
	   argument is greater, a clause, the index of a
	   literal with subterm <PartnerTermS> that can be
	   rewritten, a flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  The atom of literal pli in PartnerClause is
           destructively changed !!!
	   The <DocProof> flag is considered.
***************************************************************/
{
  LITERAL PartnerLit;
  TERM    ReplaceTermT, NewAtom;

#ifdef CHECK
  clause_Check(PartnerClause, Flags, Precedence);
  clause_Check(RuleCl, Flags, Precedence);
#endif

  if (flag_GetFlagIntValue(Flags, flag_DOCPROOF))
    red_DocumentRewriting(PartnerClause, pli, RuleCl, ri);

  if (flag_GetFlagIntValue(Flags, flag_PREW)) {
    fputs("\nBRewriting: ", stdout);
    clause_Print(PartnerClause);
    printf(" ==>[ %zd.%d ] ", clause_Number(RuleCl), ri); 
  }

  PartnerLit = clause_GetLiteral(PartnerClause, pli);

  ReplaceTermT =
    cont_ApplyBindingsModuloMatchingReverse(cont_LeftContext(),
			     term_Copy(term_SecondArgument(clause_GetLiteralTerm(RuleCl, ri))));
  
  NewAtom = clause_LiteralSignedAtom(PartnerLit);
  term_ReplaceSubtermBy(NewAtom, PartnerTermS, ReplaceTermT);
  term_Delete(ReplaceTermT);

  clause_PrecomputeOrderingAndReInit(PartnerClause, Flags, Precedence);
  clause_UpdateSplitDataFromPartner(PartnerClause, RuleCl);

  if (flag_GetFlagIntValue(Flags, flag_PREW))
    clause_Print(PartnerClause);
}


static LIST red_LiteralRewriting(CLAUSE RedClause, LITERAL ActLit, int ri,
				 SHARED_INDEX ShIndex, FLAGSTORE Flags,
				 PRECEDENCE Precedence, LIST* Result)
/**************************************************************
  INPUT:   A clause, a positive equality literal where the
           first equality argument is greater, its index, an 
	   index of clauses, a flag store, a precedence and a
	   pointer to a list of clauses that were rewritten.
  RETURNS: The list of clauses from the index that can be 
           rewritten by <ActLit> and <RedClause>.
           The rewritten clauses are stored in <*Result>.
  EFFECT:  The <DocProof> flag is considered.
***************************************************************/
{
  TERM TermS, CandTerm;
  LIST Blocked;

#ifdef CHECK
  if (!clause_LiteralIsLiteral(ActLit) || !clause_LiteralIsEquality(ActLit) ||
      !clause_LiteralIsOrientedEquality(ActLit)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_LiteralRewriting: Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RedClause, Flags, Precedence);
#endif

  Blocked    = list_Nil();
  TermS      = term_FirstArgument(clause_LiteralSignedAtom(ActLit));  /* Vars can't be greater ! */

  CandTerm = st_ExistInstance(cont_LeftContext(), sharing_Index(ShIndex), TermS);

  while (CandTerm) {

    if (!term_IsVariable(CandTerm) &&
	!symbol_IsPredicate(term_TopSymbol(CandTerm))) {
      LIST LitList;
      
      LitList = sharing_GetDataList(CandTerm, ShIndex);

      for ( ; !list_Empty(LitList); LitList = list_Pop(LitList)){
	LITERAL PartnerLit;
	CLAUSE  PartnerClause;
	int     pli;

	PartnerLit    = list_Car(LitList);
	pli           = clause_LiteralGetIndex(PartnerLit);
	PartnerClause = clause_LiteralOwningClause(PartnerLit);

	/* Partner literal must be from antecedent or succedent */
	if (clause_Number(RedClause) != clause_Number(PartnerClause) &&
	    pli >= clause_FirstAntecedentLitIndex(PartnerClause) &&
	    !list_PointerMember(Blocked, PartnerClause) &&
	    subs_SubsumesBasic(RedClause, PartnerClause, ri, pli)) {
	  CLAUSE Copy;

	  Blocked = list_Cons(PartnerClause, Blocked);
	  Copy    = clause_Copy(PartnerClause);
	  clause_RemoveFlag(Copy, WORKEDOFF);
	  red_ApplyRewriting(RedClause, ri, Copy, pli, CandTerm,
			     Flags, Precedence);
	  *Result = list_Cons(Copy, *Result);
	}
      }
    }
    CandTerm = st_NextCandidate();
  }
  return Blocked;
}

void red_NotOrientedLiteralRewritingHelp(CLAUSE RuleClause, TERM ActAtom, int ri,
                                                TERM Left, TERM Right, 
                                                SHARED_INDEX ShIndex,
                                                FLAGSTORE Flags, 
                                                PRECEDENCE Precedence,
					        LIST* Result,
                                                LIST* Blocked)
/**************************************************************
    INPUT:  A clause, an equality term which is not
  	    oriented, an index, the left term and right term 
            of the equality <ActTerm>,
	    a flag store, a precedence,
            a pointer to a list of the clauses after rewriting
            a pointer to a list of clauses that were rewritten.
    RETURN: nothing
    EFFECT: The clauses from the index that can be 
            rewritten by <ActLit> and <RuleClause> are 
            in <Blocked>
            The rewritten clauses are stored in <*Result>.
 **************************************************************/
{
     TERM CandTerm;
     
    
    #ifdef CHECK
    if (!term_IsTerm(ActAtom) 
            || !term_IsTerm(Left) 
            || !term_IsTerm(Right)
	    || symbol_IsPredicate(term_TopSymbol(Left))) {
            
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_NotOrientedLiteralRewritingHelp: Illegal input.\n");
      misc_FinishErrorReport();
    }
    clause_Check(RuleClause, Flags, Precedence);
    #endif
 
  /* a variable can't be greater than any other term, except a ground term */
  /* right hand side variables must be contained in the left side */
  if((!term_IsVariable(Left) || term_IsGround(Right)) &&
      ord_CompareVarsSubset(Left, Right)) {

      CandTerm = st_ExistInstance(cont_LeftContext(), sharing_Index(ShIndex), Left);
      
      while(CandTerm)
     {
        /* if <CandTerm> is no predicate */
        /* try to orient the equation in the context */
        if(!symbol_IsPredicate(term_TopSymbol(CandTerm)) &&
              ord_ContGreater(cont_LeftContext(), Left,
	                      cont_LeftContext(), Right,
				    Flags, Precedence)) {
         LIST LitList;

         LitList = sharing_GetDataList(CandTerm, ShIndex);

      	 for ( ; !list_Empty(LitList); LitList = list_Pop(LitList)){
	    LITERAL PartnerLit;
	    CLAUSE  PartnerClause;
	    int     pli;
	    
	    PartnerLit    = list_Car(LitList);
	    pli           = clause_LiteralGetIndex(PartnerLit);
	    PartnerClause = clause_LiteralOwningClause(PartnerLit);

	    /* Partner literal must be from antecedent or succedent */
	    if (clause_Number(RuleClause) != clause_Number(PartnerClause) &&
	        pli >= clause_FirstAntecedentLitIndex(PartnerClause) &&
	        !list_PointerMember(*Blocked, PartnerClause) &&
	        subs_SubsumesBasic(RuleClause, PartnerClause, ri, pli)) {
	      
	        CLAUSE Copy;
  	        TERM    ReplaceTermT, NewAtom;
	   
	        *Blocked = list_Cons(PartnerClause, *Blocked);
	        Copy    = clause_Copy(PartnerClause);
	        clause_RemoveFlag(Copy, WORKEDOFF);
	      

              #ifdef CHECK
                clause_Check(Copy, Flags, Precedence);
                clause_Check(RuleClause, Flags, Precedence);
              #endif

                if (flag_GetFlagIntValue(Flags, flag_DOCPROOF))
                  red_DocumentRewriting(Copy, pli, RuleClause, ri);

                if (flag_GetFlagIntValue(Flags, flag_PREW)) {
                  fputs("\nBRewriting: ", stdout);
                  clause_Print(Copy);
                  printf(" ==>[ %zd.%d ] ", clause_Number(RuleClause), ri); 
                }

	    
                ReplaceTermT =
                  cont_ApplyBindingsModuloMatchingReverse(cont_LeftContext(),
			     term_Copy(Right));
  
                NewAtom = clause_LiteralSignedAtom(clause_GetLiteral(Copy, pli));
                term_ReplaceSubtermBy(NewAtom, CandTerm, ReplaceTermT);
              
	        term_Delete(ReplaceTermT);

                clause_PrecomputeOrderingAndReInit(Copy, Flags, Precedence);
                clause_UpdateSplitDataFromPartner(Copy, RuleClause);

                if (flag_GetFlagIntValue(Flags, flag_PREW))
                  clause_Print(Copy);	     			     
	       	     
	        *Result = list_Cons(Copy, *Result);
	    }
        } /* for */	 
       }
       CandTerm = st_NextCandidate();
      } /* while */
  } 
}

static LIST red_NotOrientedLiteralRewriting(CLAUSE RuleClause, LITERAL ActLit,
					 int ri,
				 	 SHARED_INDEX ShIndex, FLAGSTORE Flags,
				 	 PRECEDENCE Precedence, LIST* Result)
/**************************************************************
  INPUT:   A clause, a equality literal which is not
  	   oriented, its index, an 
	   index of clauses, a flag store, a precedence and a
	   pointer to a list of clauses that were rewritten.
  RETURNS: The list of clauses from the index that can be 
           rewritten by <ActLit> and <RuleClause>.
           The rewritten clauses are stored in <*Result>.
  EFFECT:  The <DocProof> flag is considered.
  
***************************************************************/
{
  TERM ActAtom;
  LIST Blocked;
  
  #ifdef CHECK
    if (!clause_LiteralIsLiteral(ActLit) || !clause_LiteralIsEquality(ActLit)) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_NotOrientedLiteralRewriting: Illegal input.\n");
      misc_FinishErrorReport();
    }
    clause_Check(RuleClause, Flags, Precedence);
  #endif
  
  Blocked = list_Nil();
  
  ActAtom = clause_LiteralSignedAtom(ActLit);
 
 
  
  red_NotOrientedLiteralRewritingHelp(RuleClause, ActAtom, ri, 
                                                term_FirstArgument(ActAtom), 
                                                term_SecondArgument(ActAtom), 
                                                ShIndex, Flags, Precedence, Result, &Blocked);
 
  red_NotOrientedLiteralRewritingHelp(RuleClause, ActAtom, ri, 
                                                           term_SecondArgument(ActAtom),
                                                           term_FirstArgument(ActAtom), 
                                                           ShIndex, Flags, Precedence, Result, &Blocked);
  
  return Blocked; 
}

static LIST red_BackRewriting(CLAUSE RuleClause, SHARED_INDEX ShIndex,
			      FLAGSTORE Flags, PRECEDENCE Precedence,
			      LIST* Result)
/**************************************************************
  INPUT:   A clause, and Index, a flag store, a precedence and
           a pointer to the list of rewritten clauses.
  RETURNS: A list of clauses that can be rewritten with
           <RuleClause> and the result of this operation is
	   stored in <*Result>.
  EFFECT:  The <DocProof> flag is considered.
***************************************************************/
{
  int     i,length;
  LITERAL ActLit;
  LIST    Blocked;

#ifdef CHECK
  if (!(clause_IsClause(RuleClause, Flags, Precedence))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_BackRewriting :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RuleClause, Flags, Precedence);
#endif
  
  Blocked = list_Nil();
  length  = clause_Length(RuleClause);
  
  for (i=clause_FirstSuccedentLitIndex(RuleClause); i < length; i++) {
    ActLit = clause_GetLiteral(RuleClause, i);
    if (clause_LiteralIsOrientedEquality(ActLit)) {
      Blocked = list_Nconc(red_LiteralRewriting(RuleClause, ActLit, i,
						ShIndex, Flags, Precedence,
						Result),
			   Blocked);
    }
    
    else if(clause_LiteralIsEquality(ActLit)) {
	Blocked = list_Nconc(red_NotOrientedLiteralRewriting(RuleClause, ActLit, i,
						       ShIndex, Flags, Precedence,
						       Result),
			   Blocked);
    }
      
#ifdef CHECK
    if (fol_IsEquality(clause_LiteralSignedAtom(ActLit))) {
      ord_RESULT HelpRes;
      
      HelpRes =
	ord_Compare(term_FirstArgument(clause_LiteralSignedAtom(ActLit)), 
		    term_SecondArgument(clause_LiteralSignedAtom(ActLit)),
		    Flags, Precedence);
	
      if (ord_IsSmallerThan(HelpRes)){ /* For Debugging */
	misc_StartErrorReport();
	misc_ErrorReport("\n In red_BackRewriting:");
	misc_ErrorReport("First Argument smaller than second in RuleClause.\n");
	misc_FinishErrorReport();
      }
    } /* end of if (fol_IsEquality). */
#endif
  } /* end of for 'all succedent literals'. */
  Blocked = list_PointerDeleteDuplicates(Blocked);
  return Blocked;
}


/**************************************************************/
/* BACKWARD CONTEXTUAL REWRITING                              */
/**************************************************************/

static LIST red_BackCRwOnLiteral(PROOFSEARCH Search, CLAUSE RuleClause,
				 LITERAL Lit, int i, NAT Mode, LIST* Result)
/**************************************************************
  INPUT:   A proof search object, a clause that is used to rewrite
           other clauses, a positive literal from the clause,
	   that is a strictly maximal, oriented equation, the index
	   of the literal, a mode defining which clause index
	   is used to find rewritable clauses, and a pointer
           to a list that is used as return value.
	   The left term of the equation has to be the strictly
	   maximal term in the clause, i.e. it is bigger than
	   any other term.
  RETURNS: The list of clauses from the clause index that can be
           rewritten by <Lit> and <RuleClause>.
           The rewritten clauses are stored in <*Result>.
  EFFECT:  The <DocProof> flag is considered.
***************************************************************/
{
  TERM         TermS, CandTerm, ReplaceTermT;
  LIST         Inst, Blocked;
  FLAGSTORE    Flags;
  PRECEDENCE   Precedence;
  SHARED_INDEX ShIndex;

  Flags      = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
  if (red_WorkedOffMode(Mode))
    ShIndex = prfs_WorkedOffSharingIndex(Search);
  else
    ShIndex = prfs_UsableSharingIndex(Search);

#ifdef CHECK
  if (!clause_LiteralIsLiteral(Lit) || !clause_LiteralIsEquality(Lit) ||
      !clause_LiteralGetFlag(Lit, STRICTMAXIMAL) ||
      !clause_LiteralIsOrientedEquality(Lit)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_BackCRwOnLiteral: Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RuleClause, Flags, Precedence);
#endif

  Blocked = list_Nil();
  TermS   = term_FirstArgument(clause_LiteralSignedAtom(Lit));

  /* Get all instances of <TermS> at once. This can't be done iteratively */
  /* since other reduction rules are invoked within the following loop. */
  Inst = st_GetInstance(cont_LeftContext(), sharing_Index(ShIndex), TermS);

  for ( ; !list_Empty(Inst); Inst = list_Pop(Inst)) {
    CandTerm = list_Car(Inst);
    
    if (!term_IsVariable(CandTerm) &&
	!symbol_IsPredicate(term_TopSymbol(CandTerm))) {
      LIST LitList;
      
      LitList = sharing_GetDataList(CandTerm, ShIndex);
      
      for ( ; !list_Empty(LitList); LitList = list_Pop(LitList)){
	LITERAL RedLit;
	CLAUSE  RedClause, HelpClause;
	int     ri;
	
	RedLit     = list_Car(LitList);
	ri         = clause_LiteralGetIndex(RedLit);
	RedClause  = clause_LiteralOwningClause(RedLit);
	HelpClause = NULL;
	
#ifdef CRW_DEBUG
	if (clause_Number(RuleClause) != clause_Number(RedClause) &&
	    ri >= clause_FirstAntecedentLitIndex(RedClause) &&
	    !list_PointerMember(Blocked, RedClause)) {
	  printf("\n------\nBCRw: %s\n%d  ", red_WorkedOffMode(Mode) ?
		 "WorkedOff" : "Usable", i);
	  clause_Print(RuleClause);
	  printf("\n%d  ", ri);
	  clause_Print(RedClause);
	}
#endif

	/* Partner literal must be from antecedent or succedent */
	if (clause_Number(RuleClause) != clause_Number(RedClause) &&
	    ri >= clause_FirstAntecedentLitIndex(RedClause) &&
	    /* Check that clause wasn't already rewritten by this literal.   */
	    /* Necessary because then the old version of the clause is still */
	    /* in the index, but the rewritten version in not in the index.  */
	    !list_PointerMember(Blocked, RedClause) &&
	    red_CRwTautologyCheck(Search, RedClause, ri, CandTerm,
				  RuleClause, i, Mode, &HelpClause)) {
	  CLAUSE Copy;
	  
	  /* The <PartnerClause> has to be copied because it's indexed. */
	  Blocked = list_Cons(RedClause, Blocked);
	  Copy    = clause_Copy(RedClause);
	  clause_RemoveFlag(Copy, WORKEDOFF);
	 	  	  
	  /* Establish bindings */
	  cont_StartBinding();
	  if (!unify_MatchBindings(cont_LeftContext(), TermS, CandTerm)) {
#ifdef CHECK
	    misc_StartErrorReport();
	    misc_ErrorReport("\n In red_BackCRwOnLiteral: terms aren't ");
	    misc_ErrorReport("matchable.");
	    misc_FinishErrorReport();
#endif
	  }
	  /* The variable check in cont_ApplyBindings... is turned on here */
	  /* because all variables is s are bound, and s > t. So all       */
	  /* variables in t are bound, too. */
	  ReplaceTermT =
	    cont_ApplyBindingsModuloMatching(cont_LeftContext(),
					     term_Copy(term_SecondArgument(clause_GetLiteralTerm(RuleClause, i))),
					     TRUE);
	  cont_BackTrack();

	  /* Modify copied clause */
	  term_ReplaceSubtermBy(clause_GetLiteralAtom(Copy, ri), CandTerm,
				ReplaceTermT);
	  term_Delete(ReplaceTermT);
	  
	  /* Proof documentation */
	  if (flag_GetFlagIntValue(Flags, flag_DOCPROOF)) {
	    LIST PClauses, PLits;

	    if (HelpClause != NULL) {
	      /* Get additional parent clauses and literals from the */
	      /* tautology check. */
	      PClauses = clause_ParentClauses(HelpClause);
	      PLits    = clause_ParentLiterals(HelpClause);
	      clause_SetParentClauses(HelpClause, list_Nil());
	      clause_SetParentLiterals(HelpClause, list_Nil());
	    } else
	      PClauses = PLits = list_Nil();
	    
	    red_DocumentContextualRewriting(Copy, ri, RuleClause, i,
					    PClauses, PLits);
	  }

	  /* Set splitting data according to all parents */
	  clause_UpdateSplitDataFromPartner(Copy, RuleClause);
	  if (HelpClause != NULL) {
	    clause_UpdateSplitDataFromPartner(Copy, HelpClause);
	    clause_Delete(HelpClause);
	  }

	  clause_PrecomputeOrderingAndReInit(Copy, Flags, Precedence);
	  
	  if (flag_GetFlagIntValue(Flags, flag_PREW) >= flag_ON) {
	    fputs("\nBContRewriting: ", stdout);
	    clause_Print(RedClause);
	    printf(" ==>[ %zd.%d ] ", clause_Number(RuleClause), i);
	    clause_Print(Copy);
	  }
	  	  
	  *Result = list_Cons(Copy, *Result);
	}
      }
    }
  }

  return Blocked;
}


static LIST red_BackContextualRewriting(PROOFSEARCH Search, CLAUSE RuleClause,
					NAT Mode, LIST* Result)
/**************************************************************
  INPUT:   A proof search object, a clause that is used to rewrite
           other clauses, a mode flag that indicates which clause
           index is used to find rewritable clauses, and a pointer
           to a list that is used as return value.
  RETURNS: A list of clauses that can be reduced
           with Contextual Rewriting with <RuleClause>.
           The clauses resulting from the rewriting steps are
	   stored in <*Result>.
  EFFECT:  The <DocProof> flag is considered. Every rewritable clause
           is copied before rewriting is applied! This has to be done,
	   because the rewritable clauses are indexed.
***************************************************************/
{
  BOOL       found;
  int        i, ls;
  LITERAL    Lit;
  LIST       Blocked;
  FLAGSTORE  Flags;
  PRECEDENCE Precedence;

  Flags      = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);

#ifdef CHECK
  if (!clause_IsClause(RuleClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_BackContextualRewriting: Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RuleClause, Flags, Precedence);
#endif
  
  Blocked = list_Nil();
  ls      = clause_LastSuccedentLitIndex(RuleClause);
  found   = FALSE;

  for (i = clause_FirstSuccedentLitIndex(RuleClause); i <= ls && !found; i++) {
    Lit = clause_GetLiteral(RuleClause, i);
    if (clause_LiteralIsOrientedEquality(Lit) &&
	clause_LiteralGetFlag(Lit, STRICTMAXIMAL) &&
	red_LeftTermOfEquationIsStrictlyMaximalTerm(RuleClause, Lit, Flags,
						    Precedence)) {
      Blocked = list_Nconc(red_BackCRwOnLiteral(Search, RuleClause, Lit, i,
						Mode, Result),
			   Blocked);
      /* Stop loop: there's only one strictly maximal term per clause */
      found = TRUE;
    }
  }

  Blocked = list_PointerDeleteDuplicates(Blocked);
  return Blocked;
}


static LIST red_BackOnLiteralAuxSubtermContextualRewriting(PROOFSEARCH Search, CLAUSE RuleClause,
							   LITERAL Lit, int i, TERM TermS, TERM Right, NAT Mode, LIST* Result)
/**************************************************************
 INPUT:   A proof search object, a clause, a positive equational literal
	  from <RuleClause> that is used for rewrtiting,
	  a mode defining which clause index
	  is used to find rewritable clauses, two terms and a pointer
	  to a list
 RETURNS: The list of clauses from the clause index that can be
	  rewritten with literal <Lit> of <RuleClause>.
	  The rewritten clauses are stored in <*Result>.
 EFFECT:  If <DocProof> flag set then parent information is stored.
 ASSUME:  <Lit> is an equational literal from
	  <RuleClause>, <TermS> and <Right> are the arguments
	  of <Lit>
 ***************************************************************/
{
  TERM CandTerm, ReplaceTermT, TermSCopy, RightCopy, ActAtom;
  LIST Inst, Blocked;
  FLAGSTORE Flags;
  PRECEDENCE Precedence;
  SHARED_INDEX ShIndex;
  BOOL Validity;

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
  ActAtom = clause_LiteralSignedAtom(Lit);
  Validity = FALSE;

  if (red_WorkedOffMode(Mode))
    ShIndex = prfs_WorkedOffSharingIndex(Search);
  else
    ShIndex = prfs_UsableSharingIndex(Search);

#ifdef CHECK
  if (!clause_LiteralIsLiteral (Lit) || !clause_LiteralIsEquality (Lit) ||
      clause_GetLiteral(RuleClause, i) != Lit ||
      !clause_LiteralGetFlag (Lit, STRICTMAXIMAL) ||
      (!(term_FirstArgument (clause_LiteralAtom (Lit)) == TermS) &&
          !(term_SecondArgument (clause_LiteralAtom (Lit)) == TermS)) ||
      (!(term_FirstArgument (clause_LiteralAtom (Lit)) == Right) &&
          !(term_SecondArgument (clause_LiteralAtom (Lit)) == Right)))
    {
      misc_StartErrorReport ();
      misc_ErrorReport ("\n In red_BackCRwOnLiteral: Illegal input.\n");
      misc_FinishErrorReport ();
    }
  clause_Check (RuleClause, Flags, Precedence);
#endif

  Blocked = list_Nil();
  if (!term_IsVariable(TermS) && ord_CompareVarsSubset(TermS, Right))
    {
      /* Get all instances of <TermS> at once. This can't be done iteratively */
      /* since other reduction rules are invoked within the following loop. */
      Inst = st_GetInstance(cont_LeftContext(), sharing_Index(ShIndex), TermS);

      for (; !list_Empty(Inst); Inst = list_Pop(Inst))
        {
          CandTerm = list_Car(Inst);

          /* Establish bindings */
          cont_StartBinding();
          if (!unify_MatchBindings(cont_LeftContext(), TermS, CandTerm))
            {
#ifdef CHECK
              misc_StartErrorReport();
              misc_ErrorReport("\n In red_BackCRwOnLiteral: terms aren't ");
              misc_ErrorReport("matchable.");
              misc_FinishErrorReport();
#endif
            }

          /* Check if <Lit> is oriented or can be oriented in the current context */
          if ((clause_LiteralIsOrientedEquality(Lit) && term_FirstArgument(
              ActAtom) == TermS) || ord_ContGreater(cont_LeftContext(), TermS,
              cont_LeftContext(), Right, Flags, Precedence))
            {
              LIST LitList;

              LitList = sharing_GetDataList(CandTerm, ShIndex);

              cont_BackTrack();

              /* process list of candidate literals */
              for (; !list_Empty(LitList); LitList = list_Pop(LitList))
                {
                  LITERAL RedLit;
                  CLAUSE RedClause, HelpClause, RuleCopy;
                  LIST RewParentCls, RewParentLits;
                  int ri;

                  RedLit = list_Car(LitList);
                  ri = clause_LiteralGetIndex(RedLit);
                  RedClause = clause_LiteralOwningClause(RedLit);
                  HelpClause = NULL;
                  Validity = FALSE;

                  RewParentCls = list_Nil();
                  RewParentLits = list_Nil();

#ifdef CRW_DEBUG
                  if (clause_Number(RuleClause) != clause_Number(RedClause) &&
                      ri >= clause_FirstAntecedentLitIndex(RedClause) &&
                      !list_PointerMember(Blocked, RedClause))
                    {
                      printf("\n------\nBRCRw: %s\n%d  ", red_WorkedOffMode(Mode) ?
                          "WorkedOff" : "Usable", i);
                      clause_Print(RuleClause);
                      printf("\n%d  ", ri);
                      clause_Print(RedClause);
                    }
#endif
                  /* copy <RuleClause> and rename variables in copy */
                  RuleCopy = clause_Copy(RuleClause);
                  clause_RenameVarsBiggerThan(RuleCopy,
                      clause_MaxVar(RedClause));

                  /* what is the left/right side */
                  if (TermS == term_FirstArgument(ActAtom))
                    {
                      TermSCopy = term_FirstArgument(clause_GetLiteralAtom(
                          RuleCopy, i));
                      RightCopy = term_SecondArgument(clause_GetLiteralAtom(
                          RuleCopy, i));
                    }
                  else
                    {
                      TermSCopy = term_SecondArgument(clause_GetLiteralAtom(
                          RuleCopy, i));
                      RightCopy = term_FirstArgument(clause_GetLiteralAtom(
                          RuleCopy, i));
                    }

                  /* Remove parent information of copied clause and mark it as temporary */
                  list_Delete(clause_ParentClauses(RuleCopy));
                  clause_SetParentClauses(RuleCopy, list_Nil());
                  list_Delete(clause_ParentLiterals(RuleCopy));
                  clause_SetParentLiterals(RuleCopy, list_Nil());
                  clause_SetTemporary(RuleCopy);

                  cont_StartBinding();
                  if (!unify_MatchBindings(cont_LeftContext(), TermSCopy,
                      CandTerm))
                    {
#ifdef CHECK
                      cont_PrintCurrentTrail();
                      misc_StartErrorReport();
                      misc_ErrorReport("\n In red_BackUnivRedCRwOnLiteralHelp: terms aren't matchable");
                      misc_FinishErrorReport();
#endif
                    }

                  /* Partner literal must be from antecedent or succedent */
                  if (clause_Number(RuleClause) != clause_Number(RedClause)
                      && ri >= clause_FirstAntecedentLitIndex(RedClause) &&
                  /* Check that <RedClause> is not in the <Blocked> list */
                  /* Necessary because then the old version of the clause is still */
                  /* in the index, but the rewritten version is not in the index.  */
                  !list_PointerMember(Blocked, RedClause))
                    {

                      /* Apply bindings to equation s=t, where s > t. Here the strict version */
                      /* of cont_Apply... can be applied, because all variables in s and t    */
                      /* are bound. */
                      cont_ApplyBindingsModuloMatching(cont_LeftContext(),
                          clause_GetLiteralAtom(RuleCopy, i), TRUE);

                      /* Check whether E > (s=t)sigma. It suffices to check only positive */
                      /* equations. All other cases imply the condition. */
                      if (ord_LiteralCompare(clause_GetLiteralTerm(RedClause,ri), 
                          clause_LiteralGetOrderStatus(clause_GetLiteral(RedClause, ri)),
                          clause_GetLiteralTerm(RuleCopy, i),
                          clause_LiteralGetOrderStatus(clause_GetLiteral(RuleCopy, i)),
                          TRUE, Flags, Precedence) != ord_GREATER_THAN)
                        {

                          cont_BackTrack();
                        }
                      else
                        {
                          int lastlit, h;

                          /* Apply bindings to the rest of <RuleCopy> */
                          lastlit = clause_LastLitIndex(RuleCopy);
                          for (h = clause_FirstLitIndex(); h <= lastlit; h++)
                            {
                              if (h != i)
                                cont_ApplyBindingsModuloMatching(
                                    cont_LeftContext(), clause_GetLiteralAtom(
                                        RuleCopy, h), FALSE);
                            }
                          if (!red_TermIsStrictlyMaximalTerm(RuleCopy, i,
                              TermSCopy, Flags, Precedence))
                            {
                              cont_BackTrack();
                            }
                          else
                            {
                              /* Backtrack bindings before reduction rules are invoked */
                              cont_BackTrack();

                              /* Do not consider <RedClause> for Recursive Contextual Ground Rewriting */
                              Blocked = list_Cons(RedClause, Blocked);
                              Validity = red_CRwSideCondSubtermContextualRewriting(Search,
                                  RedClause, ri, CandTerm, RuleCopy, i, Mode,
                                  &HelpClause, TRUE, &RewParentCls,
                                  &RewParentLits, Blocked);
#ifdef CHECK
                              if(list_Empty(Blocked) || list_Car(Blocked) != RedClause)
                                {
                                  misc_StartErrorReport();
                                  misc_ErrorReport("\n In red_BackUnivRedCRwOnLiteralHelp: list changed.\n");
                                  misc_FinishErrorReport();
                                }
#endif
                              Blocked = list_Pop(Blocked);
                            }
                        }
                    }
                  else
                    {
                      cont_BackTrack();
                    }

                  if (Validity)
                    {
                      CLAUSE Copy;

                      /* The <PartnerClause> has to be copied because it's indexed. */
                      Blocked = list_Cons(RedClause, Blocked);
                      Copy = clause_Copy(RedClause);
                      clause_RemoveFlag(Copy, WORKEDOFF);

                      /* Establish bindings */
                      cont_StartBinding();
                      if (!unify_MatchBindings(cont_LeftContext(), TermS,
                          CandTerm))
                        {
#ifdef CHECK
                          misc_StartErrorReport();
                          misc_ErrorReport("\n In red_BackCRwOnLiteral: terms aren't ");
                          misc_ErrorReport("matchable.");
                          misc_FinishErrorReport();
#endif
                        }
                      /* The variable check in cont_ApplyBindings... is turned on here */
                      /* because all variables is s are bound, and s > t. So all       */
                      /* variables in t are bound, too. */
                      ReplaceTermT = cont_ApplyBindingsModuloMatching(
                          cont_LeftContext(), term_Copy(Right), TRUE);
                      cont_BackTrack();

                      /* Modify copied clause */
                      term_ReplaceSubtermBy(clause_GetLiteralAtom(Copy, ri),
                          CandTerm, ReplaceTermT);
                      term_Delete(ReplaceTermT);

                      /* Proof documentation */
                      if (flag_GetFlagIntValue(Flags, flag_DOCPROOF))
                        {
                          LIST PClauses, PLits;

                          if (HelpClause != NULL)
                            {
                              /* Get additional parent clauses and literals from the */
                              /* tautology check. */
                              PClauses = list_Nconc(clause_ParentClauses(
                                  HelpClause), RewParentCls);
                              PLits = list_Nconc(clause_ParentLiterals(
                                  HelpClause), RewParentLits);
                              clause_SetParentClauses(HelpClause, list_Nil());
                              clause_SetParentLiterals(HelpClause, list_Nil());
                            }
                          else
                            {
                              PClauses = RewParentCls;
                              PLits = RewParentLits;
                            }

                          red_DocumentContextualRewriting(Copy, ri, RuleClause,
                              i, PClauses, PLits);
                        }

                      /* Set splitting data according to all parents */
                      clause_UpdateSplitDataFromPartner(Copy, RuleClause);
                      if (HelpClause != NULL)
                        {
                          clause_UpdateSplitDataFromPartner(Copy, HelpClause);
                          clause_Delete(HelpClause);
                        }

                      clause_PrecomputeOrderingAndReInit(Copy, Flags, Precedence);

                      if (flag_GetFlagIntValue(Flags, flag_PREW) >= flag_ON)
                        {
                          fputs("\nBUnivRedContRewriting: ", stdout);
                          clause_Print(RedClause);
                          printf(" ==>[ %zd.%d ] ", clause_Number(RuleClause), i);
                          clause_Print(Copy);
                        }

                      *Result = list_Cons(Copy, *Result);
                    }
                  else
                    {
                      clause_Delete(RuleCopy);
                      list_Delete(RewParentCls);
                      list_Delete(RewParentLits);
                    }
                }
            }
          else
            {
              cont_BackTrack();
            }
        }
    }

  return Blocked;
}

static LIST
red_BackCRwLitElimination(PROOFSEARCH Search, CLAUSE RuleClause,
    LITERAL RuleLit, int i, NAT Mode, LIST* Result)
/**************************************************************
 INPUT:   A Proofsearch object, a clause, an equality literal which is
	  not necessarily oriented, its index in <RuleClause>,
	  two terms, a mode
	  and a pointer to a list of clauses
 RETURNS: The list of clauses from the index that can be
	  rewritten by <RuleLit> and <RuleClause>.
	  The rewritten clauses are stored in <*Result>.
 EFFECT:  If <DocProof> flag set then parent information is stored
 ASSUME:  <RuleLit> is an equational literal from
	  <RuleClause>, <TermS> and <Right> are the arguments
	  of <RuleLit>
 ***************************************************************/
{
  CLAUSE RuleCopy;
  TERM CandTerm, ActAtom, ActAtomCopy;
  LIST Inst, Blocked;
  FLAGSTORE Flags;
  PRECEDENCE Precedence;
  SHARED_INDEX ShIndex;
  BOOL Tautology;
  int h, last;

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);
  ActAtom = clause_LiteralAtom(RuleLit);
  Tautology = FALSE;

  if (red_WorkedOffMode(Mode))
    ShIndex = prfs_WorkedOffSharingIndex(Search);
  else
    ShIndex = prfs_UsableSharingIndex(Search);

#ifdef CHECK
  if (!clause_LiteralIsLiteral(RuleLit) || !clause_LiteralIsEquality(RuleLit) ||
      !clause_LiteralIsNegative(RuleLit) ||
      clause_GetLiteral(RuleClause, i) != RuleLit)
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_BackUnivRedNegativeCRwOnLiteral: Illegal input.\n");
      misc_FinishErrorReport();
    }
  clause_Check(RuleClause, Flags, Precedence);
#endif

  Blocked = list_Nil();

  /* Get all instances of <TermS> at once. This can't be done iteratively */
  /* since other reduction rules are invoked within the following loop. */
  Inst = st_GetInstance(cont_LeftContext(), sharing_Index(ShIndex), ActAtom);

  for (; !list_Empty(Inst); Inst = list_Pop(Inst))
    {
      LIST LitList;

      CandTerm = list_Car(Inst);

      LitList = sharing_GetDataList(CandTerm, ShIndex);

      for (; !list_Empty(LitList); LitList = list_Pop(LitList))
        {
          if (clause_LiteralIsPositive(list_Car(LitList))
              && clause_LiteralIsEquality(list_Car(LitList)))
            {

              LITERAL RedLit;
              CLAUSE RedClause, HelpClause;
              LIST RewParentCls, RewParentLits;
              int ri;

              RedLit = list_Car(LitList);
              ri = clause_LiteralGetIndex(RedLit);
              RedClause = clause_LiteralOwningClause(RedLit);
              HelpClause = NULL;
              Tautology = FALSE;

              RewParentCls = list_Nil();
              RewParentLits = list_Nil();

#ifdef CRW_DEBUG
              if (clause_Number(RuleClause) != clause_Number(RedClause) &&
                  !list_PointerMember(Blocked, RedClause))
                {
                  printf("\n------\nBNegCRw: %s\n%d  ", red_WorkedOffMode(Mode) ?
                      "WorkedOff" : "Usable", i);
                  clause_Print(RuleClause);
                  printf("\n%d  ", ri);
                  clause_Print(RedClause);
                }
#endif

              if (clause_Number(RuleClause) != clause_Number(RedClause)
                  && !list_PointerMember(Blocked, RedClause))
                {

                  RuleCopy = clause_Copy(RuleClause);
                  ActAtomCopy = clause_GetLiteralAtom(RuleCopy, i);
                  clause_RenameVarsBiggerThan(RuleCopy,
                      clause_MaxVar(RedClause));

                  /* unify */
                  cont_StartBinding();

                  if (!unify_MatchBindings(cont_LeftContext(), ActAtomCopy,
                      CandTerm))
                    {
#ifdef CHECK
                      cont_PrintCurrentTrail();
                      misc_StartErrorReport();
                      misc_ErrorReport("\n In red_CRwRecursiveTautologyCheck: terms aren't matchable");
                      misc_FinishErrorReport();
#endif
                    }

                  /* Apply bindings to  <RuleCopy> */
                  last = clause_LastLitIndex(RuleCopy);
                  for (h = clause_FirstLitIndex(); h <= last; h++)
                    {
                      cont_ApplyBindingsModuloMatching(cont_LeftContext(),
                          clause_GetLiteralAtom(RuleCopy, h), FALSE);
                    }

                  cont_BackTrack();
#ifdef CRW_DEBUG
                  printf("\nRuleCopy: "); clause_Print(RuleCopy);
                  printf("\nCandTerm: "); term_Print(CandTerm);
#endif

                  /* Block <RedClause> for recursive Side Condition check */
                  Blocked = list_Cons(RedClause, Blocked);

                  /* Check side conditions of Subterm Contextual Literal Elimination */
                  if (red_CRwSideCondSubtermContextualRewriting(Search, RedClause, ri,
                      CandTerm, RuleCopy, i, Mode, &HelpClause, FALSE,
                      &RewParentCls, &RewParentLits, Blocked))
                    {
                      CLAUSE Copy;

                      /* copy RedClause because it is indexed */
                      Copy = clause_Copy(RedClause);
                      clause_RemoveFlag(Copy, WORKEDOFF);
                      /* Eliminate Literal from clause */
                      clause_DeleteLiteral(Copy, ri, Flags, Precedence);

                      /* Proof documentation */
                      if (flag_GetFlagIntValue(Flags, flag_DOCPROOF))
                        {
                          LIST PClauses, PLits;

                          if (HelpClause != NULL)
                            {
                              /* Get additional parent clauses and literals from the */
                              /* tautology check. */
                              PClauses = list_Nconc(clause_ParentClauses(
                                  HelpClause), RewParentCls);
                              PLits = list_Nconc(clause_ParentLiterals(
                                  HelpClause), RewParentLits);
                              clause_SetParentClauses(HelpClause, list_Nil());
                              clause_SetParentLiterals(HelpClause, list_Nil());
                            }
                          else
                            {
                              PClauses = RewParentCls;
                              PLits = RewParentLits;
                            }

                          red_DocumentContextualRewriting(Copy, ri, RuleClause,
                              i, PClauses, PLits);
                        }

                      /* Set splitting data according to all parents */
                      clause_UpdateSplitDataFromPartner(Copy, RuleClause);
                      if (HelpClause != NULL)
                        {
                          clause_UpdateSplitDataFromPartner(Copy, HelpClause);
                          clause_Delete(HelpClause);
                        }

                      clause_PrecomputeOrderingAndReInit(Copy, Flags, Precedence);

                      if (flag_GetFlagIntValue(Flags, flag_PREW) >= flag_ON)
                        {
                          fputs("\nBUnivRedNegContRewriting: ", stdout);
                          clause_Print(RedClause);
                          printf(" ==>[ %zd.%d ] ", clause_Number(RuleClause), i);
                          clause_Print(Copy);
                        }

                      *Result = list_Cons(Copy, *Result); /* save rewritten clause */

                    }
                  else
                    {
#ifdef CHECK
                      if(list_Empty(Blocked) || list_Car(Blocked) != RedClause)
                        {
                          misc_StartErrorReport();
                          misc_ErrorReport("\n In red_BackUnivRedNegativeCRwOnLiteral: Illegal list.\n");
                          misc_FinishErrorReport();
                        }
#endif

                      /* remove <RedClause> from <Blocked> list if literal
                       * elimination was not possible
                       */
                      Blocked = list_Pop(Blocked);
                      clause_Delete(RuleCopy);
                      list_Delete(RewParentCls);
                      list_Delete(RewParentLits);
                    }
                }
            }
        }
    }

  return Blocked;
}

static LIST
red_BackOnLiteralSubtermContextualRewriting(PROOFSEARCH Search, CLAUSE RuleClause,
    LITERAL ActLit, int i, NAT Mode, LIST* Result)
/**************************************************************
 INPUT:   A Proofsearch object, a clause, an equality literal which is
	  not necessarily oriented, a mode flag that indicates which clause
	  index is used to find rewritable clauses,
	  and a pointer to a list of clauses
 RETURNS: The list of clauses from the index that can be
	  rewritten with literal <ActLit> of <RuleClause>.
	  The rewritten clauses are stored in <*Result>.
 EFFECT:  If <DocProof> flag set then parent information is stored.
 ASSUME:  <ActLit> is literal <i> of <RuleClause>
 ***************************************************************/
{
  TERM ActAtom;
  LIST Blocked;

#ifdef CHECK
  if (!clause_LiteralIsLiteral(ActLit) || !clause_LiteralIsEquality(ActLit) ||
      clause_GetLiteral(RuleClause, i) != ActLit )
    {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_BackOnLiteralSubtermContextualRewriting: Illegal input.\n");
      misc_FinishErrorReport();
    }
  clause_Check(RuleClause, prfs_Store(Search), prfs_Precedence(Search));
#endif

  Blocked = list_Nil();

  ActAtom = clause_LiteralSignedAtom(ActLit);

  /* Try to use literal s = t for rewriting*/
  Blocked = red_BackOnLiteralAuxSubtermContextualRewriting(Search, RuleClause, ActLit, i,
      term_FirstArgument(ActAtom), term_SecondArgument(ActAtom), Mode, Result);

  /* Try to use literal t = s for rewriting */
  Blocked = list_Nconc(red_BackOnLiteralAuxSubtermContextualRewriting(Search, RuleClause,
      ActLit, i, term_SecondArgument(ActAtom), term_FirstArgument(ActAtom),
      Mode, Result), Blocked);

  return Blocked;
}

static LIST
red_BackCRwSubtermContextualRewriting(PROOFSEARCH Search, CLAUSE RuleClause,
    NAT Mode, LIST* Result)
/**************************************************************
 INPUT:   A proof search object, a clause that is used to rewrite
	  other clauses, a mode flag that indicates which clause
	  index is used to find rewritable clauses, and a pointer
	  to a list that is used as return value.
 RETURNS: A list of clauses that can be reduced
	  with Contextual Rewriting with <RuleClause>.
	  The clauses resulting from the rewriting steps are
	  stored in <*Result>.
 EFFECT:  The <DocProof> flag is considered. Every rewritable clause
	  is copied before rewriting is applied! This has to be done,
	  because the rewritable clauses are indexed.
 ***************************************************************/
{
  int i, ls;
  LITERAL Lit;
  LIST Blocked;
  FLAGSTORE Flags;
  PRECEDENCE Precedence;

  Flags = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);

#ifdef CHECK
  if (!clause_IsClause (RuleClause, Flags, Precedence))
    {
      misc_StartErrorReport ();
      misc_ErrorReport
      ("\n In red_BackContextualRewriting: Illegal input.\n");
      misc_FinishErrorReport ();
    }
  clause_Check (RuleClause, Flags, Precedence);
#endif

  Blocked = list_Nil();
  ls = clause_LastSuccedentLitIndex(RuleClause);

  /* Subterm Contextual Rewriting */
#ifdef CRW_DEBUG
  printf("\nBackCRw: \n"); clause_Print(RuleClause);
#endif

  for (i = clause_FirstSuccedentLitIndex(RuleClause); i <= ls; i++)
    {
      Lit = clause_GetLiteral(RuleClause, i);
      if (clause_LiteralIsEquality(Lit) && clause_LiteralGetFlag(Lit,
          STRICTMAXIMAL) && clause_VarsOfClauseAreInTerm(RuleClause,
          term_FirstArgument(clause_LiteralAtom(Lit))))
        {
          Blocked = list_Nconc(red_BackOnLiteralSubtermContextualRewriting(Search, RuleClause,
              Lit, i, Mode, Result), Blocked);
        }
    }
  /* Subterm Contextual Literal Elimination */
  if (flag_GetFlagIntValue(Flags, flag_RBREW) == flag_RBREWRNEGCRW)
    {
#ifdef CRW_DEBUG
      printf("\nBackNegCRw: \n"); clause_Print(RuleClause);
#endif

      ls = clause_LastAntecedentLitIndex(RuleClause);

      for (i = clause_FirstAntecedentLitIndex(RuleClause); i <= ls; i++)
        {
          Lit = clause_GetLiteral(RuleClause, i);
          if (clause_LiteralIsEquality(Lit))
            {
              Blocked = list_Nconc(red_BackCRwLitElimination(Search,
                  RuleClause, Lit, i, Mode, Result), Blocked);
            }
        }

    }

  Blocked = list_PointerDeleteDuplicates(Blocked);
  return Blocked;
}

static void red_DocumentSortSimplification(CLAUSE Clause, LIST Indexes,
					   LIST Clauses)
/*********************************************************
  INPUT:   A clause and the literal indices and clauses
           involved in sort simplification.
  RETURNS: Nothing.
  MEMORY:  Consumes the input lists.
**********************************************************/
{
  LIST Scan,Declarations,Self;

  Declarations = list_Nil();
  Self         = list_Nil();

  list_Delete(clause_ParentClauses(Clause));
  list_Delete(clause_ParentLiterals(Clause));  
  
  for(Scan=Indexes;!list_Empty(Scan);Scan=list_Cdr(Scan))
    Self = list_Cons((POINTER)clause_Number(Clause),Self);
  
  for(Scan=Clauses;!list_Empty(Scan);Scan=list_Cdr(Scan)) {
    Declarations = list_Cons((POINTER)clause_FirstSuccedentLitIndex(list_Car(Scan)),Declarations);
    list_Rplaca(Scan,(POINTER)clause_Number(list_Car(Scan)));
  }

  clause_SetParentLiterals(Clause, list_Nconc(Indexes,Declarations));
  clause_SetParentClauses(Clause, list_Nconc(Self,Clauses));

  clause_SetNumber(Clause, clause_IncreaseCounter());
  clause_SetFromSortSimplification(Clause);
}


static BOOL red_SortSimplification(SORTTHEORY Theory, CLAUSE Clause, NAT Level,
				   BOOL Document, FLAGSTORE Flags,
				   PRECEDENCE Precedence, CLAUSE *Changed)
/**********************************************************
  INPUT:   A sort theory, a clause, the last backtrack
           level of the current proof search, a boolean
	   flag concerning proof documentation, a flag
	   store and a precedence.
  RETURNS: TRUE iff sort simplification was possible.
           If <Document> is true or the split level of the
	   used declaration clauses requires copying a 
	   simplified copy of the clause is returned in 
	   <*Changed>.
	   Otherwise the clause is destructively
	   simplified.
***********************************************************/
{ 
  if (Theory != (SORTTHEORY)NULL) {
    TERM      Atom,Term;
    SOJU      SortPair;
    SORT      TermSort,LitSort;
    LIST      Indexes,NewClauses,Clauses,Scan;
    intptr_t  i,j;
    int       lc,OldSplitLevel;
    CLAUSE    Copy;
    BOOL      SubSort;

#ifdef CHECK
    if (!clause_IsClause(Clause, Flags, Precedence)) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_SortSimplification :");
      misc_ErrorReport(" Illegal input.\n");
      misc_FinishErrorReport();
    }
    clause_Check(Clause, Flags, Precedence);
#endif

    lc            = clause_LastConstraintLitIndex(Clause);
    i             = clause_FirstLitIndex();
    j             = 0;
    OldSplitLevel = clause_SplitLevel(Clause);
    Copy          = Clause;
    Indexes       = list_Nil();
    Clauses       = list_Nil();
  
    while (i <= lc) {
      
      Atom        = clause_LiteralAtom(clause_GetLiteral(Copy, i));    
      Term        = term_FirstArgument(Atom);
      SortPair    = sort_ComputeSort(Theory, Term, Copy, i,
				       Flags, Precedence);
      TermSort    = sort_PairSort(SortPair);
      NewClauses  = list_Copy(sort_PairClauses(SortPair));
      LitSort     = sort_TheorySortOfSymbol(Theory,term_TopSymbol(Atom));
      
      SubSort     = FALSE;

      NewClauses  = list_Nconc(sort_TheoryIsSubsort(Theory, TermSort, LitSort, &SubSort), NewClauses);
      
      if (SubSort){
	if (j == 0 && flag_GetFlagIntValue(Flags, flag_PSSI)) {
	  
	  fputs("\nSortSimplification: ", stdout);
	  clause_Print(Copy);
	  
	  fputs(" ==>[ ", stdout);
	}
	
	for (Scan = NewClauses; !list_Empty(Scan); Scan = list_Cdr(Scan)){
	  if (Clause == Copy  && 
	      (Document ||  
	       prfs_SplitLevelCondition(clause_SplitLevel(list_Car(Scan)),OldSplitLevel,Level))) 
	    Copy   = clause_Copy(Clause);
         
	  clause_UpdateSplitDataFromPartner(Copy, list_Car(Scan));
		  
	  if (flag_GetFlagIntValue(Flags, flag_PSSI))
	    printf("%zd ",clause_Number(list_Car(Scan)));

	}
	
	  if (Document)
	    Indexes = list_Cons((POINTER)(i+j), Indexes);
	 
	  clause_DeleteLiteral(Copy, i, Flags, Precedence);
	  Clauses = list_Nconc(NewClauses, Clauses);
	  j++;
	  lc--;
      }	 
   
      else {
	list_Delete(NewClauses);
	i++;
      }
      

      sort_DeleteSortPair(SortPair);
      sort_Delete(LitSort);
 }


#ifdef CHECK
    clause_Check(Copy, Flags, Precedence);
#endif

    if (j > 0) {
      if (Document)
	red_DocumentSortSimplification(Copy,Indexes,Clauses);
      else
	list_Delete(Clauses);
      clause_ReInit(Copy, Flags, Precedence);
      if (flag_GetFlagIntValue(Flags, flag_PSSI)) {
	fputs("] ", stdout);
	clause_Print(Copy);
      }
      if (Copy != Clause)
	*Changed = Copy;
      return TRUE;
    }
  }

  return FALSE;
}

static void red_ExchangeClauses(CLAUSE *RedClause, CLAUSE *Copy, LIST *Result)
/**********************************************************
  INPUT:   Two pointers to clauses and a pointer to a list.
  RETURNS: Nothing.
  EFFECT:  If *Copy is NULL, nothing is done. Otherwise *RedClause
           is added to the list, *Copy is assigned to *RedClause,
	   and NULL is assigned to *Copy.
***********************************************************/
{
  if (*Copy) {
    *Result    = list_Cons(*RedClause,*Result);
    *RedClause = *Copy;
    *Copy      = (CLAUSE)NULL;
  }
}
     


static BOOL red_SimpleStaticReductions(CLAUSE *RedClause, FLAGSTORE Flags,
				       PRECEDENCE Precedence, LIST* Result)
/**********************************************************
  INPUT:   A clause (by reference), a flag store and a
           precedence.
  RETURNS: TRUE if <*RedClause> is redundant.
	   If the <DocProof> flag is false and no copying is necessary
	   with respect to splitting, the clause is destructively changed,
	   otherwise (intermediate) copies are made and returned in <*Result>.
  EFFECT:  Used reductions are tautology deletion and 
           obvious reductions.
***********************************************************/
{
  CLAUSE Copy;
  BOOL   DocProof;

#ifdef CHECK
  if (!clause_IsClause(*RedClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_SimpleStaticReductions :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(*RedClause, Flags, Precedence);
#endif

  Copy = (CLAUSE)NULL;
  DocProof = flag_GetFlagIntValue(Flags, flag_DOCPROOF);

  if (flag_GetFlagIntValue(Flags, flag_RTAUT) != flag_RTAUTOFF &&
      red_Tautology(*RedClause, Flags, Precedence))
    return TRUE;

  if (flag_GetFlagIntValue(Flags, flag_ROBV)) {
    red_ObviousReductions(*RedClause, DocProof, Flags, Precedence, &Copy);
    red_ExchangeClauses(RedClause, &Copy, Result);
  }

  if (flag_GetFlagIntValue(Flags, flag_RCON)) {
    red_Condensing(*RedClause, DocProof, Flags, Precedence, &Copy);
    red_ExchangeClauses(RedClause, &Copy, Result);
  }

  return FALSE;    
}

  


static BOOL red_StaticReductions(PROOFSEARCH Search, CLAUSE *Clause,
				 CLAUSE *Subsumer, LIST* Result, NAT Mode)
/**********************************************************
  INPUT:   A proof search object, a clause (by reference) to be reduced, 
           a shared index of clauses and the mode of the reductions,
	   determining which sets (Usable, WorkedOff) in <Search>
	   are considered for reductions.
  RETURNS: TRUE iff the clause is redundant.
	   If the <DocProof> flag is false and no copying is necessary
	   with respect  to splitting, the clause is destructively changed,
	   otherwise (intermediate) copies are made and returned in <*Result>.
           If <Clause> gets redundant with respect to forward subsumption,
	   the subsuming clause is returned in <*Subsumer>.
  EFFECT:  Used reductions are tautology deletion, obvious reductions,
           forward subsumption, forward rewriting, forward contextual
	   rewriting, forward matching replacement resolution,
	   sort simplification, unit conflict and static soft typing.
	   Depending on <Mode>, then clauses are reduced with respect
	   to WorkedOff or  Usable Clauses.
***********************************************************/
{ 
  CLAUSE       Copy;
  BOOL         Redundant;
  SHARED_INDEX Index;
  FLAGSTORE    Flags;
  PRECEDENCE   Precedence;

  Flags      = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);

#ifdef CHECK
  if (!clause_IsClause(*Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_StaticReductions:");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(*Clause, Flags, Precedence);
#endif

  Index     = (red_OnlyWorkedOffMode(Mode) ? 
	       prfs_WorkedOffSharingIndex(Search) : prfs_UsableSharingIndex(Search));
  Copy      = (CLAUSE)NULL;
  Redundant = red_SimpleStaticReductions(Clause, Flags, Precedence, Result);

  if (Redundant)
    return Redundant;

  /* Assignment Equation Deletion */
  if (flag_GetFlagIntValue(Flags, flag_RAED) != flag_RAEDOFF &&
      red_AssignmentEquationDeletion(*Clause, Flags, Precedence, &Copy,
				     ana_NonTrivClauseNumber(prfs_GetAnalyze(Search)), 
				     (flag_GetFlagIntValue(Flags, flag_RAED) == flag_RAEDPOTUNSOUND))) {
    red_ExchangeClauses(Clause, &Copy, Result);
    if (clause_IsEmptyClause(*Clause))
      return FALSE;
  }

  /* Subsumption */
  if (flag_GetFlagIntValue(Flags, flag_RFSUB)) {
    *Subsumer = red_ForwardSubsumption(*Clause, Index, Flags, Precedence);
    if ((Redundant = (*Subsumer != (CLAUSE)NULL)))
      return Redundant;
  }

  /* Forward Rewriting and Forward Contextual Rewriting */
 if (((flag_GetFlagIntValue(Flags, flag_RFREW) == flag_RFREWON) &&
	red_RewriteRedClause(*Clause, Index, Flags, Precedence, 
		&Copy, prfs_LastBacktrackLevel(Search))) || ((flag_GetFlagIntValue(Flags,
      flag_RFREW) == flag_RFREWCRW) && (red_RewriteRedClause(*Clause, Index,
      Flags, Precedence, &Copy, prfs_LastBacktrackLevel(Search))
      || red_ContextualRewriting(Search, *Clause, Mode,
          prfs_LastBacktrackLevel(Search), &Copy))) || ((flag_GetFlagIntValue(
      Flags, flag_RFREW) == flag_RFREWRCRW) && red_CRwSubtermContextualRewriting(
      Search, *Clause, Mode, prfs_LastBacktrackLevel(Search), &Copy))
      || ((flag_GetFlagIntValue(Flags, flag_RFREW) == flag_RFREWRNEGCRW)
          && red_CRwSubtermContextualRewriting(Search, *Clause, Mode,
              prfs_LastBacktrackLevel(Search), &Copy))) {
    red_ExchangeClauses(Clause, &Copy, Result);
    Redundant = red_SimpleStaticReductions(Clause, Flags, Precedence, Result);
    if (Redundant)
      return Redundant;
    if (clause_IsEmptyClause(*Clause))
      return FALSE;
    if (flag_GetFlagIntValue(Flags, flag_RFSUB)) {
      *Subsumer = red_ForwardSubsumption(*Clause, Index, Flags, Precedence);
      if ((Redundant = (*Subsumer != (CLAUSE)NULL)))
	return Redundant;
    }
  }

  /* Sort Simplification */
  if (red_OnlyWorkedOffMode(Mode) && flag_GetFlagIntValue(Flags, flag_RSSI)) { 
    red_SortSimplification(prfs_DynamicSortTheory(Search), *Clause,
			   prfs_LastBacktrackLevel(Search),
			   flag_GetFlagIntValue(Flags, flag_DOCPROOF),
			   Flags, Precedence, &Copy);
    red_ExchangeClauses(Clause, &Copy, Result);
    if (clause_IsEmptyClause(*Clause))
      return FALSE;    
  }

  /* Matching Replacement Resolution */
  if (flag_GetFlagIntValue(Flags, flag_RFMRR)) {
    red_MatchingReplacementResolution(*Clause, Index, Flags, Precedence, 
				      &Copy, prfs_LastBacktrackLevel(Search));
    red_ExchangeClauses(Clause, &Copy, Result);
    if (clause_IsEmptyClause(*Clause))
      return FALSE;
  }

  /* Unit Conflict */
  if (flag_GetFlagIntValue(Flags, flag_RUNC)) {
    red_UnitConflict(*Clause, Index, Flags, Precedence,
		     &Copy, prfs_LastBacktrackLevel(Search));
    red_ExchangeClauses(Clause, &Copy, Result);
    if (clause_IsEmptyClause(*Clause))
      return FALSE;
  }

  /* Static Soft Typing */
  if (red_OnlyWorkedOffMode(Mode) && flag_GetFlagIntValue(Flags, flag_RSST))
    Redundant = red_ClauseDeletion(prfs_StaticSortTheory(Search),*Clause,
				   Flags, Precedence);
  
#ifdef CHECK
  clause_Check(*Clause, Flags, Precedence);
#endif

  return Redundant;
}

static BOOL red_SelectedStaticReductions(PROOFSEARCH Search, CLAUSE *Clause, 
					 CLAUSE *Subsumer, LIST* Result,
					 NAT Mode)
/**********************************************************
  INPUT:   A proof search object, a clause (by reference) to be reduced,
           and the mode of the reductions, determining which sets
	   (Usable, WorkedOff) in <Search> are considered for reductions.
  EFFECT:  Used reductions are tautology deletion, obvious reductions,
           forward subsumption, forward rewriting, forward matching
	   replacement resolution, sort simplification, unit conflict
	   and static soft typing.
	   Depending on <Mode>, the clauses are reduced with respect
	   to WorkedOff and/or Usable Clauses.
  RETURNS: TRUE iff the clause is redundant.
	   If the <DocProof> flag is false and no copying is necessary
	   with respect to splitting, the clause is destructively changed,
	   otherwise (intermediate) copies are made and returned in <*Result>.
           If <Clause> gets redundant with respect to forward subsumption,
	   the subsuming clause is returned in <*Subsumer>.
***********************************************************/
{ 
  CLAUSE       Copy;
  BOOL         Redundant ,Rewritten, Tried, RecContextualRew, ContextualRew,
      StandardRew, RecNegContextualRew;
  SHARED_INDEX WoIndex,UsIndex;
  FLAGSTORE    Flags;
  PRECEDENCE   Precedence;

  Flags      = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);

#ifdef CHECK
  if (!clause_IsClause(*Clause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_SelectedStaticReductions:");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(*Clause, Flags, Precedence);
#endif

  WoIndex = (SHARED_INDEX)NULL;
  UsIndex = (SHARED_INDEX)NULL;
  if (red_WorkedOffMode(Mode))
    WoIndex   = prfs_WorkedOffSharingIndex(Search);
  if (red_UsableMode(Mode))
    UsIndex   = prfs_UsableSharingIndex(Search);
  Copy      = (CLAUSE)NULL;
  Redundant = red_SimpleStaticReductions(Clause, Flags, Precedence, Result);

  if (Redundant)
    return Redundant;

  if (flag_GetFlagIntValue(Flags, flag_RAED) != flag_RAEDOFF &&
      red_AssignmentEquationDeletion(*Clause, Flags, Precedence, &Copy,
				     ana_NonTrivClauseNumber(prfs_GetAnalyze(Search)), 
				     (flag_GetFlagIntValue(Flags, flag_RAED)==flag_RAEDPOTUNSOUND))) {
    red_ExchangeClauses(Clause, &Copy, Result);
    if (clause_IsEmptyClause(*Clause))
      return FALSE;
  }

  if (flag_GetFlagIntValue(Flags, flag_RFSUB)) {
    *Subsumer = (CLAUSE)NULL;
    if (WoIndex != NULL) {
      *Subsumer = red_ForwardSubsumption(*Clause, WoIndex, Flags, Precedence);
      if (*Subsumer != (CLAUSE)NULL)
	return TRUE;
    }
    if (UsIndex != NULL) {
      *Subsumer = red_ForwardSubsumption(*Clause, UsIndex, Flags, Precedence);
      if (*Subsumer != (CLAUSE)NULL)
	return TRUE;
    }
  }

  StandardRew = (flag_RFREWON == flag_GetFlagIntValue(Flags, flag_RFREW));
  ContextualRew = (flag_RFREWCRW == flag_GetFlagIntValue(Flags, flag_RFREW));
  RecContextualRew = (flag_RFREWRCRW == flag_GetFlagIntValue(Flags, flag_RFREW));
  RecNegContextualRew = (flag_RFREWRNEGCRW == flag_GetFlagIntValue(Flags,
      flag_RFREW));

  Rewritten = (StandardRew || ContextualRew || RecContextualRew
      || RecNegContextualRew);
  Tried = FALSE;
  while (Rewritten)
    {
      Rewritten = FALSE;

      if (WoIndex != NULL && (((StandardRew || ContextualRew)
          && red_RewriteRedClause(*Clause, WoIndex, Flags, Precedence, &Copy,
              prfs_LastBacktrackLevel(Search))) || (ContextualRew
          && (red_ContextualRewriting(Search, *Clause, red_WORKEDOFF,
              prfs_LastBacktrackLevel(Search), &Copy))) || (RecContextualRew
          && (red_CRwSubtermContextualRewriting(Search, *Clause, red_WORKEDOFF,
              prfs_LastBacktrackLevel(Search), &Copy))) || (RecNegContextualRew
          && (red_CRwSubtermContextualRewriting(Search, *Clause, red_WORKEDOFF,
              prfs_LastBacktrackLevel(Search), &Copy))))) 
        {
          Rewritten = TRUE;
      red_ExchangeClauses(Clause, &Copy, Result);
      Redundant = red_SimpleStaticReductions(Clause, Flags, Precedence, Result);
      if (Redundant)
	return Redundant;
      if (clause_IsEmptyClause(*Clause))
	return FALSE;
      if (flag_GetFlagIntValue(Flags, flag_RFSUB)) {
	*Subsumer = (CLAUSE)NULL;
	*Subsumer = red_ForwardSubsumption(*Clause, WoIndex,
					   Flags, Precedence);
	if (*Subsumer != (CLAUSE)NULL)
	  return TRUE;       /* Clause is redundant */
	if (UsIndex != NULL) {
	  *Subsumer = red_ForwardSubsumption(*Clause, UsIndex,
					     Flags, Precedence);
	  if (*Subsumer != (CLAUSE)NULL)
	    return TRUE;
	}
      }
    }

    if (UsIndex != NULL && (!Tried || Rewritten) && (((StandardRew
        || ContextualRew) && red_RewriteRedClause(*Clause, UsIndex, Flags,
        Precedence, &Copy, prfs_LastBacktrackLevel(Search)))
        || (ContextualRew && (red_ContextualRewriting(Search, *Clause,
            red_USABLE, prfs_LastBacktrackLevel(Search), &Copy)))
        || (RecContextualRew && (red_CRwSubtermContextualRewriting(Search,
            *Clause, red_USABLE, prfs_LastBacktrackLevel(Search), &Copy)))
        || (RecNegContextualRew && (red_CRwSubtermContextualRewriting(Search,
            *Clause, red_USABLE, prfs_LastBacktrackLevel(Search), &Copy)))))
      {
  
      Rewritten = TRUE;
      red_ExchangeClauses(Clause, &Copy, Result);
      Redundant = red_SimpleStaticReductions(Clause, Flags,
					     Precedence, Result);
      if (Redundant)
	return Redundant;
      if (clause_IsEmptyClause(*Clause))
	return FALSE;
      if (flag_GetFlagIntValue(Flags, flag_RFSUB)) {
	*Subsumer = (CLAUSE)NULL;
	if (WoIndex != NULL)
	  *Subsumer = red_ForwardSubsumption(*Clause, WoIndex,
					     Flags, Precedence);
	if (*Subsumer != (CLAUSE)NULL)
	  return TRUE;
	*Subsumer = red_ForwardSubsumption(*Clause, UsIndex,
					   Flags, Precedence);
	if (*Subsumer != (CLAUSE)NULL)
	  return TRUE;
      }
    }

    Tried = TRUE;
  } /* end of while(Rewritten) */


  if (flag_GetFlagIntValue(Flags, flag_RSSI)) { 
    red_SortSimplification(prfs_DynamicSortTheory(Search), *Clause,
			   prfs_LastBacktrackLevel(Search),
			   flag_GetFlagIntValue(Flags, flag_DOCPROOF),
			   Flags, Precedence, &Copy);
    red_ExchangeClauses(Clause, &Copy, Result);
    if (clause_IsEmptyClause(*Clause))
      return FALSE; 
  }
     
  if (flag_GetFlagIntValue(Flags, flag_RFMRR)) {
    if (WoIndex)
      red_MatchingReplacementResolution(*Clause, WoIndex, Flags, Precedence,
					&Copy, prfs_LastBacktrackLevel(Search));
    red_ExchangeClauses(Clause, &Copy, Result);
    if (clause_IsEmptyClause(*Clause))
      return FALSE;
    if (UsIndex)
      red_MatchingReplacementResolution(*Clause, UsIndex, Flags, Precedence,
					&Copy, prfs_LastBacktrackLevel(Search));
    red_ExchangeClauses(Clause, &Copy, Result);
    if (clause_IsEmptyClause(*Clause))
      return FALSE;
  }

  if (flag_GetFlagIntValue(Flags, flag_RUNC)) {
    if (WoIndex)
      red_UnitConflict(*Clause, WoIndex, Flags, Precedence,
		       &Copy, prfs_LastBacktrackLevel(Search));
    red_ExchangeClauses(Clause, &Copy, Result);
    if (clause_IsEmptyClause(*Clause))
      return FALSE;
    if (UsIndex)
      red_UnitConflict(*Clause, UsIndex, Flags, Precedence,
		       &Copy, prfs_LastBacktrackLevel(Search));
    red_ExchangeClauses(Clause, &Copy, Result);
    if (clause_IsEmptyClause(*Clause))
      return FALSE;
  }

  if (flag_GetFlagIntValue(Flags, flag_RSST))
    Redundant = red_ClauseDeletion(prfs_StaticSortTheory(Search),*Clause,
				   Flags, Precedence);
  
#ifdef CHECK
  clause_Check(*Clause, Flags, Precedence);
#endif

  return Redundant;
}


CLAUSE red_ReductionOnDerivedClause(PROOFSEARCH Search, CLAUSE Clause,
				    NAT Mode)
/**************************************************************
  INPUT:   A proof search object, a derived clause and a mode
	   indicating which indexes should be used for reductions.
  RETURNS: The non-redundant clause after reducing <Clause>,
           NULL if <Clause> is redundant.
  EFFECT:  Clauses probably generated, but redundant are kept according
           to the <DocProof> flag and the split level of involved clauses.
	   depending on <Mode>, then clauses are reduced
	   with respect to WorkedOff and/or Usable Clauses.
***************************************************************/
{
  CLAUSE RedClause;
  LIST   Redundant;

#ifdef CHECK
  cont_SaveState();
#endif

  Redundant = list_Nil();
  RedClause = (CLAUSE)NULL;

  if (red_StaticReductions(Search,&Clause,&RedClause,&Redundant,Mode)) {
    /* Clause is redundant */
    red_HandleRedundantDerivedClauses(Search, Redundant, Clause);
    list_Delete(Redundant);
    if (RedClause &&
	prfs_SplitLevelCondition(clause_SplitLevel(RedClause),clause_SplitLevel(Clause),
				 prfs_LastBacktrackLevel(Search))) {
      split_KeepClauseAtLevel(Search,Clause,clause_SplitLevel(RedClause));
    }
    else
      if (flag_GetFlagIntValue(prfs_Store(Search), flag_DOCPROOF))
	prfs_InsertDocProofClause(Search,Clause);
      else
	clause_Delete(Clause);
    Clause = (CLAUSE)NULL;
  }
  else {
    red_HandleRedundantDerivedClauses(Search, Redundant, Clause);
    list_Delete(Redundant);
  }

#ifdef CHECK
  cont_CheckState();
#endif

  return Clause;
}

CLAUSE red_CompleteReductionOnDerivedClause(PROOFSEARCH Search, CLAUSE Clause,
					    NAT Mode)
/**************************************************************
  INPUT:   A proof search object, a derived clause and a mode determining
	   which clauses to consider for reduction.
  RETURNS: The non-redundant clause after reducing <Clause>,
           NULL if <Clause> is redundant.
  EFFECT:  Clauses probably generated, but redundant are kept according
           to the <DocProof> flag and the split level of involved clauses.
	   The clause is reduced with respect to all indexes determined
	   by <Mode>
***************************************************************/
{
  CLAUSE RedClause;
  LIST   Redundant;

#ifdef CHECK
  cont_SaveState();
#endif

  Redundant = list_Nil();
  RedClause = (CLAUSE)NULL;

  if (red_SelectedStaticReductions(Search,&Clause,&RedClause,&Redundant,Mode)) {
    /* <Clause> is redundant */
    red_HandleRedundantDerivedClauses(Search, Redundant, Clause);
    list_Delete(Redundant);
    if (RedClause && 
	prfs_SplitLevelCondition(clause_SplitLevel(RedClause),clause_SplitLevel(Clause),
				 prfs_LastBacktrackLevel(Search))) {
      split_KeepClauseAtLevel(Search, Clause, clause_SplitLevel(RedClause));
    }
    else
      if (flag_GetFlagIntValue(prfs_Store(Search), flag_DOCPROOF))
	prfs_InsertDocProofClause(Search, Clause);
      else
	clause_Delete(Clause);
    Clause = (CLAUSE)NULL;
  }
  else {
    red_HandleRedundantDerivedClauses(Search, Redundant, Clause);
    list_Delete(Redundant);
  }

#ifdef CHECK
  cont_CheckState();
#endif

  return Clause;
}


LIST red_BackReduction(PROOFSEARCH Search, CLAUSE Clause, NAT Mode)
/**************************************************************
  INPUT:   A proof search object, a clause and a mode flag.
  RETURNS: A list of reduced clauses in usable and worked-off
           (depending on <Mode>) in <Search> with respect to <Clause>.
	   The original clauses that become redundant are either deleted
	   or kept for proof documentation or splitting.
  EFFECT:  The original clauses that become redundant are either deleted
	   or kept for proof documentation or splitting.
***************************************************************/
{
  LIST Result, Redundant;
  FLAGSTORE  Flags;
  PRECEDENCE Precedence;

#ifdef CHECK
  cont_SaveState();
#endif

  Result    = list_Nil();
  Flags      = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);

  /* Subsumption */
  if (flag_GetFlagIntValue(Flags, flag_RBSUB)) {
    Redundant = list_Nil();
    if (red_WorkedOffMode(Mode))
      Redundant = red_BackSubsumption(Clause,
				      prfs_WorkedOffSharingIndex(Search),
				      Flags, Precedence);
    if (red_UsableMode(Mode))
      Redundant = list_Nconc(Redundant,
			     red_BackSubsumption(Clause,
						 prfs_UsableSharingIndex(Search),
						 Flags, Precedence));
    red_HandleRedundantIndexedClauses(Search, Redundant, Clause);
    list_Delete(Redundant);
  }

  /* Matching Replacement Resolution */
  if (flag_GetFlagIntValue(Flags, flag_RBMRR)) {
    Redundant = list_Nil();
    if (red_WorkedOffMode(Mode))
      Redundant = red_BackMatchingReplacementResolution(Clause,
							prfs_WorkedOffSharingIndex(Search),
							Flags, Precedence, &Result);
    if (red_UsableMode(Mode))
      Redundant = list_Nconc(Redundant,
			     red_BackMatchingReplacementResolution(Clause,
								   prfs_UsableSharingIndex(Search),
								   Flags, Precedence, &Result));
    red_HandleRedundantIndexedClauses(Search, Redundant, Clause);
    list_Delete(Redundant);
  }

  /* Standard Rewriting */
  if (flag_GetFlagIntValue(Flags, flag_RBREW)) {
    Redundant = list_Nil();
    if (red_WorkedOffMode(Mode))
      Redundant = red_BackRewriting(Clause,prfs_WorkedOffSharingIndex(Search),
				    Flags, Precedence, &Result);
    if (red_UsableMode(Mode))
      Redundant = list_Nconc(Redundant,
			     red_BackRewriting(Clause,
					       prfs_UsableSharingIndex(Search),
					       Flags, Precedence, &Result));

    red_HandleRedundantIndexedClauses(Search, Redundant, Clause);
    list_Delete(Redundant);
  }

  /* Contextual Rewriting */
  if (flag_GetFlagIntValue(Flags, flag_RBREW) == flag_RBREWCRW) {
    Redundant = list_Nil();
    if (red_WorkedOffMode(Mode))
      Redundant = red_BackContextualRewriting(Search, Clause, red_WORKEDOFF,
					      &Result);
    if (red_UsableMode(Mode))
      Redundant = list_Nconc(Redundant,
			     red_BackContextualRewriting(Search, Clause,
							 red_USABLE, &Result));

    red_HandleRedundantIndexedClauses(Search, Redundant, Clause);
    list_Delete(Redundant);
  }

  /* Backwards Subterm Contextual Rewriting */
  if (flag_GetFlagIntValue(Flags, flag_RBREW) == flag_RBREWRCRW)
    {
      Redundant = list_Nil();
      if (red_WorkedOffMode(Mode))
        Redundant = red_BackCRwSubtermContextualRewriting(Search, Clause,
            red_WORKEDOFF, &Result);
      if (red_UsableMode(Mode))
        Redundant = list_Nconc(Redundant, red_BackCRwSubtermContextualRewriting(
            Search, Clause, red_USABLE, &Result));

      red_HandleRedundantIndexedClauses(Search, Redundant, Clause);
      list_Delete(Redundant);
    }

  /* Backwards Contextual Literal Elimination */
  if (flag_GetFlagIntValue(Flags, flag_RBREW) == flag_RBREWRNEGCRW)
    {
      Redundant = list_Nil();
      if (red_WorkedOffMode(Mode))
        Redundant = red_BackCRwSubtermContextualRewriting(Search, Clause,
            red_WORKEDOFF, &Result);
      if (red_UsableMode(Mode))
        Redundant = list_Nconc(Redundant, red_BackCRwSubtermContextualRewriting(
            Search, Clause, red_USABLE, &Result));

      red_HandleRedundantIndexedClauses(Search, Redundant, Clause);
      list_Delete(Redundant);
    }

#ifdef CHECK
  cont_CheckState();
#endif

  return Result;
}


 LIST red_MergeClauseListsByWeight(LIST L1, LIST L2)
/**************************************************************
  INPUT:   Two lists of clauses, sorted by weight.
  RETURNS: 
  EFFECT:  
***************************************************************/
{
  return list_NNumberMerge(L1, L2, (NAT (*)(POINTER))clause_Weight);
}


LIST red_CompleteReductionOnDerivedClauses(PROOFSEARCH Search,
					   LIST DerivedClauses, NAT Mode,
					   int Bound, NAT BoundMode,
					   int *BoundApplied)
/**************************************************************
  INPUT:   A proof search object, a list of newly derived (unshared) clauses,
	   a mode determining which clause lists to consider for reduction,
	   a bound and a bound mode to cut off generated clauses.
  RETURNS: A list of empty clauses that may be derived during the
           reduction process.
           <*BoundApplied> is set to the mode dependent value of the
	   smallest clause if a clause is deleted because of a bound.
  EFFECT:  The <DerivedClauses> are destructively reduced and reduced clauses
           from the indexes are checked out and all finally reduced clauses
	   are checked into the indexes. Depending on <Mode> either the
	   WorkedOff, Usable or both indexes are considered.
	   The <DocProof> Flag is considered.
***************************************************************/
{
  LIST   EmptyClauses,NewClauses,Scan;
  NAT    ClauseBound;
  CLAUSE Clause;
  FLAGSTORE Flags;

#ifdef CHECK
  cont_SaveState();
#endif

  EmptyClauses   = list_Nil();
  DerivedClauses = clause_ListSortWeighed(DerivedClauses);
  ClauseBound    = 0;
  Flags          = prfs_Store(Search);

  while (!list_Empty(DerivedClauses)) {
    Clause     = list_NCar(&DerivedClauses);
    if (prfs_SplitStackEmpty(Search))        /* Otherwise splitting not compatible with bound deletion */
      Clause = red_CompleteReductionOnDerivedClause(Search, Clause, Mode);

    if (Clause != NULL && flag_GetFlagIntValue(Flags, flag_DLSTOP)) {
      if(clause_DLcontainsRedundantLiteral(Clause)){
	misc_StartUserErrorReport();
	misc_UserErrorReport("\nLoop detected:");
	clause_Print(Clause);
	fputs("\n", stdout);
	misc_FinishUserErrorReport();
      }
    }

    if (Clause != NULL && flag_GetFlagIntValue(Flags, flag_DLSIGBOUND)&& !clause_IsFromInput(Clause) &&
	!clause_IsFromSplitting(Clause)) {
      NAT termDepth = clause_ComputeTermDepth(Clause);
      NAT DLMaxDepth=symbol_DLGetMaxSigBound();
      if (termDepth > DLMaxDepth) {
	if (flag_GetFlagIntValue(Flags, flag_PBDC)) {
	  fputs("\nDeleted by DLbound: ", stdout);
	  clause_Print(Clause);
	}
	clause_Delete(Clause);
	if (*BoundApplied == -1 || termDepth < *BoundApplied)
	  *BoundApplied = termDepth;
	Clause = (CLAUSE)NULL;
      }
    }


    if (Clause != NULL && flag_GetFlagIntValue(Flags, flag_BOUNDVARS) != flag_BOUNDVARSNONE) {
      if ((clause_SearchMaxVar(Clause) - symbol_GetInitialStandardVarCounter()) > flag_GetFlagIntValue(Flags, flag_BOUNDVARS)) {
	if (flag_GetFlagIntValue(Flags, flag_PBDC)) {
	  fputs("\nDeleted by var bound: ", stdout);
	  clause_Print(Clause);
	}
	clause_Delete(Clause);
	Clause = (CLAUSE)NULL;
      }
    }

    if (Clause != NULL && BoundMode != flag_BOUNDMODEUNLIMITED &&
	Bound != flag_BOUNDSTARTUNLIMITED && !clause_IsFromInput(Clause) &&
	!clause_IsFromSplitting(Clause)) {
      switch (BoundMode) {
      case flag_BOUNDMODERESTRICTEDBYWEIGHT:
	ClauseBound = clause_Weight(Clause);
	break;
      case flag_BOUNDMODERESTRICTEDBYDEPTH:
	ClauseBound = clause_ComputeTermDepth(Clause);
	break;
      default:
	misc_StartUserErrorReport();
	misc_UserErrorReport("\n Error while applying bound restrictions:");
	misc_UserErrorReport("\n You selected an unknown bound mode.\n");
	misc_FinishUserErrorReport();
      }
      if (ClauseBound > Bound) {
	if (flag_GetFlagIntValue(Flags, flag_PBDC)) {
	  fputs("\nDeleted by bound: ", stdout);
	  clause_Print(Clause);
	}
	clause_Delete(Clause);
	if (*BoundApplied == -1 || ClauseBound < *BoundApplied)
	  *BoundApplied = ClauseBound;
	Clause = (CLAUSE)NULL;
      }
    }

    if (Clause != (CLAUSE)NULL &&       /* For clauses below bound, splitting is */
	!prfs_SplitStackEmpty(Search))  /* compatible with bound deletion */
      Clause = red_CompleteReductionOnDerivedClause(Search, Clause, Mode);
	
    if (Clause) {	
      prfs_IncKeptClauses(Search);
      if (flag_GetFlagIntValue(Flags, flag_PKEPT)) {
	fputs("\nKept: ", stdout); 
	clause_Print(Clause);
      }
      if (clause_IsEmptyClause(Clause))
	EmptyClauses = list_Cons(Clause,EmptyClauses);
      else {
	NewClauses = red_BackReduction(Search, Clause, Mode);
	prfs_IncDerivedClauses(Search, list_Length(NewClauses));
	if (flag_GetFlagIntValue(Flags, flag_PDER))
	  for (Scan=NewClauses; !list_Empty(Scan); Scan=list_Cdr(Scan)) {
	    fputs("\nDerived: ", stdout); 
	    clause_Print(list_Car(Scan));
	  }
	NewClauses = split_ExtractEmptyClauses(NewClauses,&EmptyClauses);

	prfs_InsertUsableClause(Search,Clause,TRUE);
	NewClauses = list_NumberSort(NewClauses, (NAT (*) (POINTER)) clause_Weight);
	DerivedClauses = red_MergeClauseListsByWeight(DerivedClauses,NewClauses);
      }
    }
  }

#ifdef CHECK
  cont_CheckState();
#endif

  return EmptyClauses;
}



static CLAUSE red_CDForwardSubsumer(CLAUSE RedCl, st_INDEX Index,
				    FLAGSTORE Flags, PRECEDENCE Precedence)
/**********************************************************
  INPUT:   A pointer to a non-empty clause, an index of 
           clauses, a flag store and a precedence.
  RETURNS: The first clause from the Approx Set which 
           subsumes 'RedCl'.
***********************************************************/
{
  TERM   Atom,AtomGen;
  LIST   LitScan;
  int    i,length;
  CLAUSE CandCl;

#ifdef CHECK
  if (!clause_IsClause(RedCl, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_CDForwardSubsumer :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RedCl, Flags, Precedence);
#endif

  length  = clause_Length(RedCl);

  for (i = 0; i < length; i++) {
    Atom     = clause_GetLiteralAtom(RedCl, i);
    AtomGen  = st_ExistGen(cont_LeftContext(), Index, Atom);

    while (AtomGen) {
      for (LitScan = term_SupertermList(AtomGen); 
	   !list_Empty(LitScan); LitScan = list_Cdr(LitScan)) {
	CandCl = clause_LiteralOwningClause(list_Car(LitScan));

	if (clause_GetLiteral(CandCl,clause_FirstLitIndex()) == (LITERAL)list_Car(LitScan) &&
	    subs_Subsumes(CandCl, RedCl, clause_FirstLitIndex(), i)) {
	  st_CancelExistRetrieval();
	  return CandCl;
	}
      }
      AtomGen = st_NextCandidate();  
    }
  }
  return (CLAUSE)NULL;
}


static BOOL red_CDForwardSubsumption(CLAUSE RedClause, st_INDEX Index,
				     FLAGSTORE Flags, PRECEDENCE Precedence)
/**********************************************************
  INPUT:   A clause, an index of clauses, a flag store and
           a precedence.
  RETURNS: The boolean value TRUE if the clause is subsumed
           by an indexed clause, if so, the clause is deleted,
	   either really or locally.
***********************************************************/
{ 
  BOOL   IsSubsumed;
  CLAUSE Subsumer;

#ifdef CHECK
  if (!clause_IsClause(RedClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_CDForwardSubSumption :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RedClause, Flags, Precedence);
#endif
  IsSubsumed = FALSE;
  Subsumer   = red_CDForwardSubsumer(RedClause, Index, Flags, Precedence);

  if (clause_Exists(Subsumer)) {
    IsSubsumed = TRUE;

    if (flag_GetFlagIntValue(Flags, flag_DOCSST) &&
	flag_GetFlagIntValue(Flags, flag_PSUB)) {
      fputs("\nFSubsumption:", stdout);
      clause_Print(RedClause);
      printf(" by %zd ",clause_Number(Subsumer));
    }
  }
  return IsSubsumed;
}


static void red_CDBackSubsumption(CLAUSE RedCl, FLAGSTORE Flags,
				  PRECEDENCE Precedence,
				  LIST* UsListPt, LIST* WOListPt,
				  st_INDEX Index)
/**********************************************************
  INPUT:   A pointer to a non-empty clause, a flag store, 
           a precedence, and an index of clauses.
  RETURNS: Nothing.
***********************************************************/
{
  TERM   Atom,AtomInst;
  CLAUSE SubsumedCl;
  LIST   Scan, SubsumedList;

#ifdef CHECK
  if (!clause_IsClause(RedCl, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_CDBackupSubSumption :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(RedCl, Flags, Precedence);
#endif

  SubsumedList = list_Nil();

  if (!clause_IsEmptyClause(RedCl)) {
    Atom     = clause_GetLiteralAtom(RedCl, clause_FirstLitIndex());
    AtomInst = st_ExistInstance(cont_LeftContext(), Index, Atom);
    
    while(AtomInst) {
      for (Scan = term_SupertermList(AtomInst); !list_Empty(Scan); Scan = list_Cdr(Scan)) {
	SubsumedCl = clause_LiteralOwningClause(list_Car(Scan));
	if ((RedCl != SubsumedCl) &&
	    subs_Subsumes(RedCl, SubsumedCl, clause_FirstLitIndex(), 
			  clause_LiteralGetIndex(list_Car(Scan))) &&
	    !list_PointerMember(SubsumedList, SubsumedCl))
	  SubsumedList = list_Cons(SubsumedCl, SubsumedList);
      }
      AtomInst = st_NextCandidate();
    }
    
    for (Scan = SubsumedList; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
      SubsumedCl = list_Car(Scan);

      if (flag_GetFlagIntValue(Flags, flag_DOCSST) && flag_GetFlagIntValue(Flags, flag_PSUB)) {
	fputs("\nBSubsumption: ", stdout);
	clause_Print(SubsumedCl);
	printf(" by %zd ",clause_Number(RedCl));
      }
	     

      if (clause_GetFlag(SubsumedCl,WORKEDOFF)) {
	*WOListPt = list_PointerDeleteOneElement(*WOListPt, SubsumedCl);
      }else {
	*UsListPt = list_PointerDeleteOneElement(*UsListPt, SubsumedCl);
      }
      clause_DeleteFlatFromIndex(SubsumedCl, Index);
    }
    list_Delete(SubsumedList);
  }
}


static LIST red_CDDerivables(SORTTHEORY Theory, CLAUSE GivenClause,
			     FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A sort theory, a clause, a flag store and a
           precedence.
  RETURNS: A list of clauses derivable from <GivenClause> and
           the declaration clauses in <Theory>.
***************************************************************/
{
  LIST ListOfDerivedClauses;

#ifdef CHECK
  if (!(clause_IsClause(GivenClause, Flags, Precedence))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_CDDeriveables :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
  clause_Check(GivenClause, Flags, Precedence);
#endif
  
  if (clause_HasTermSortConstraintLits(GivenClause))
    ListOfDerivedClauses = inf_ForwardSortResolution(GivenClause,
						     sort_TheoryIndex(Theory),
						     Theory, TRUE,
						     Flags, Precedence);
  else
    ListOfDerivedClauses = inf_ForwardEmptySort(GivenClause,
						sort_TheoryIndex(Theory),
						Theory, TRUE,
						Flags, Precedence);
  
  return ListOfDerivedClauses;
}


static BOOL red_CDReduce(SORTTHEORY Theory, CLAUSE RedClause,
			 FLAGSTORE Flags, PRECEDENCE Precedence, 
			 LIST *ApproxUsListPt, LIST *ApproxWOListPt,
			 st_INDEX Index)
/**************************************************************
  INPUT:   A sort theory, an unshared clause, a flag store, 
           a precedence, their index and two pointers to the
	   sort reduction subproof usable and worked off list.
  RETURNS: TRUE iff <RedClause> is redundant with respect to 
           clauses in the index or theory.
  EFFECT:  <RedClause> is destructively changed.
           The <DocProof> flag is changed temporarily.
***************************************************************/
{
  CLAUSE Copy;

#ifdef CHECK
  clause_Check(RedClause, Flags, Precedence);
#endif

  Copy = (CLAUSE)NULL; /* Only needed for interface */
  
  red_ObviousReductions(RedClause, FALSE, Flags, Precedence, &Copy);
  red_SortSimplification(Theory, RedClause, NAT_MAX, FALSE,
			 Flags, Precedence, &Copy);
  
  if (clause_IsEmptyClause(RedClause))
    return FALSE;

  red_Condensing(RedClause, FALSE, Flags, Precedence, &Copy);

  if (red_CDForwardSubsumption(RedClause, Index, Flags, Precedence))
    return TRUE;
  else {			/* RedClause isn't subsumed! */
    red_CDBackSubsumption(RedClause, Flags, Precedence,
			  ApproxUsListPt, ApproxWOListPt, Index);
    clause_InsertFlatIntoIndex(RedClause, Index);
    *ApproxUsListPt = list_Cons(RedClause, *ApproxUsListPt);
  }

#ifdef CHECK
  clause_Check(RedClause, Flags, Precedence);
  if (Copy != (CLAUSE)NULL) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In red_CDReduce :");
    misc_ErrorReport(" Illegal input.\n");
    misc_FinishErrorReport();
  }
    
#endif

  return FALSE;
}


BOOL red_ClauseDeletion(SORTTHEORY Theory, CLAUSE RedClause, FLAGSTORE Flags,
			PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A sort theory, a clause (unshared), a flag store
           and a precedence.
  RETURNS: TRUE iff the sort constraint of the clause is 
           unsolvable with respect to the sort theory.
***************************************************************/
{
  if (Theory != (SORTTHEORY)NULL) {
    CLAUSE   ConstraintClause, GivenClause;
    LIST     ApproxUsableList, ApproxWOList, EmptyClauses, ApproxDerivables, Scan;
    int      i,nc, Count, OldClauseCounter;
    st_INDEX Index;

#ifdef CHECK
    if (!(clause_IsClause(RedClause, Flags, Precedence))) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_ClauseDeletion :");
      misc_ErrorReport(" Illegal input.\n");
      misc_FinishErrorReport();
    }
    clause_Check(RedClause, Flags, Precedence);
#endif

    if (clause_HasEmptyConstraint(RedClause) || !flag_GetFlagIntValue(Flags, flag_RSST))
      return FALSE;
 
    if (flag_GetFlagIntValue(Flags, flag_DOCSST)) {
      fputs("\n\nStatic Soft Typing tried on: ", stdout);
      clause_Print(RedClause);
    }

    Index            = st_IndexCreate();
    Scan             = list_Nil();
    nc               = clause_NumOfConsLits(RedClause);
    OldClauseCounter = clause_Counter();

    /* Make constraint clause, insert it into the Approx-usable-list: */

    for (i = clause_FirstLitIndex(); i < nc; i++)
      Scan = list_Cons(term_Copy(clause_LiteralAtom(
						    clause_GetLiteral(RedClause, i))), Scan);

    Scan             = list_NReverse(Scan);
    ConstraintClause = clause_Create(Scan, list_Nil(), list_Nil(),
				     Flags, Precedence);
    list_Delete(Scan);
    clause_InitSplitData(ConstraintClause);
    clause_AddParentClause(ConstraintClause, clause_Number(RedClause));
    clause_AddParentLiteral(ConstraintClause, clause_FirstLitIndex());
    clause_SetFromClauseDeletion(ConstraintClause);
    clause_InsertFlatIntoIndex(ConstraintClause, Index);
    ApproxUsableList = list_List(ConstraintClause);
    ApproxWOList     = list_Nil();

    /* fputs("\nConstraint clause: ",stdout); clause_Print(ConstraintClause); */

    /* Now the lists are initialized, the subproof is started: */

    EmptyClauses = list_Nil();
    Count        = 0;

    if (flag_GetFlagIntValue(Flags, flag_DOCSST)) {
      puts("\n*************** Static Soft Typing Subproof: ***************");
      puts("The usable list:");
      clause_ListPrint(ApproxUsableList);
      puts("\nThe worked-off list:");
      clause_ListPrint(ApproxWOList);
      /* fputs("\nAll indexed clauses: ", stdout);
	 clause_PrintAllIndexedClauses(ShIndex); */
    }
    while (!list_Empty(ApproxUsableList) && list_Empty(EmptyClauses)) {
      GivenClause      = list_Car(ApproxUsableList); 
      clause_SetFlag(GivenClause,WORKEDOFF);

      if (flag_GetFlagIntValue(Flags, flag_DOCSST)) {
	fputs("\n\tSubproof Given clause: ", stdout);
	clause_Print(GivenClause); fflush(stdout);
      }
      ApproxWOList     = list_Cons(GivenClause, ApproxWOList);
      ApproxUsableList = list_PointerDeleteOneElement(ApproxUsableList,GivenClause);
      ApproxDerivables = red_CDDerivables(Theory,GivenClause, Flags, Precedence);
      ApproxDerivables = split_ExtractEmptyClauses(ApproxDerivables, &EmptyClauses);
    
      if (!list_Empty(EmptyClauses)) { /* Exit while loop! */
	if (flag_GetFlagIntValue(Flags, flag_DOCSST)) {
	  fputs("\nStatic Soft Typing not successful: ", stdout);
	  clause_Print(list_Car(EmptyClauses));
	}
	clause_DeleteClauseList(ApproxDerivables); 
	ApproxDerivables = list_Nil();
      }
      else {
	CLAUSE DerClause;
	for (Scan = ApproxDerivables; !list_Empty(Scan) && list_Empty(EmptyClauses);
	     Scan = list_Cdr(Scan)) {
	  DerClause = (CLAUSE)list_Car(Scan);
	  if (red_CDReduce(Theory, DerClause, Flags, Precedence,
			   &ApproxUsableList, &ApproxWOList, Index)) 
	    clause_Delete(DerClause);
	  else{
	    Count++;
	    if (flag_GetFlagIntValue(Flags, flag_DOCSST)) {
	      putchar('\n');
	      clause_Print(DerClause);
	    }
	    if (clause_IsEmptyClause(DerClause))
	      EmptyClauses = list_Cons(DerClause,EmptyClauses);
	  }
	  list_Rplaca(Scan,(CLAUSE)NULL);
	}

	if (!list_Empty(EmptyClauses)) {
	  if (flag_GetFlagIntValue(Flags, flag_DOCSST)) {
	    fputs(" Static Soft Typing not successful!", stdout);
	    clause_Print(list_Car(EmptyClauses));
	  }
	  clause_DeleteClauseList(ApproxDerivables);    /* There still may be clauses in the list */
	  ApproxDerivables = list_Nil();
	}
	else {
	  list_Delete(ApproxDerivables); 
	  ApproxDerivables = list_Nil();
	}
      }
    }

    if (!list_Empty(EmptyClauses)) {
      if (flag_GetFlagIntValue(Flags, flag_DOCSST)) {
	puts("\nStatic Soft Typing failed, constraint solvable.");
	puts("************  Static Soft Typing Subproof finished. ************");
      }
    }
    else
      if (flag_GetFlagIntValue(Flags, flag_PSST)) {
	fputs("\nStatic Soft Typing deleted: ", stdout);
	clause_Print(RedClause);
      }

    /* Cleanup */
    clause_DeleteClauseListFlatFromIndex(ApproxUsableList, Index);
    clause_DeleteClauseListFlatFromIndex(ApproxWOList, Index);
    st_IndexDelete(Index);
    clause_SetCounter(OldClauseCounter);

    if (!list_Empty(EmptyClauses)) {
      clause_DeleteClauseList(EmptyClauses);
      return FALSE;
    } 

    return TRUE;

#ifdef CHECK
    clause_Check(RedClause, Flags, Precedence);
#endif
  }
  return FALSE;
}

LIST red_SatUnit(PROOFSEARCH Search, LIST ClauseList)
/*********************************************************
  INPUT:   A proof search object and a list of unshared clauses.
  RETURNS: A possibly empty list of empty clauses.
  EFFECT:  Does a shallow saturation of the conclauses depending on the
           flag_SATUNIT flag.
	   The <DocProof> flag is considered.
**********************************************************/
{
  CLAUSE Given,Clause;
  LIST   Scan, Derivables, EmptyClauses, BackReduced;
  NAT    n, Derived;
  FLAGSTORE  Flags;
  PRECEDENCE Precedence;

  Flags        = prfs_Store(Search);
  Precedence   = prfs_Precedence(Search);
  Derived      = flag_GetFlagIntValue(Flags, flag_CNFPROOFSTEPS);
  EmptyClauses = list_Nil();

  ClauseList = clause_ListSortWeighed(ClauseList);
  
  while (!list_Empty(ClauseList) && list_Empty(EmptyClauses)) {
    Given = (CLAUSE)list_NCar(&ClauseList);
    Given = red_ReductionOnDerivedClause(Search, Given, red_USABLE);
    if (Given) {
      if (clause_IsEmptyClause(Given))
	EmptyClauses = list_List(Given);
      else {
	BackReduced = red_BackReduction(Search, Given, red_USABLE);

	if (Derived != 0) {
	  Derivables = inf_BoundedDepthUnitResolution(Given,
						      prfs_UsableSharingIndex(Search),
						      FALSE, Flags, Precedence);
	  n          = list_Length(Derivables);
	  if (n > Derived)
	    Derived = 0;
	  else
	    Derived = Derived - n;
	}
	else 
	  Derivables = list_Nil();

	Derivables  = list_Nconc(BackReduced,Derivables);
	Derivables  = split_ExtractEmptyClauses(Derivables, &EmptyClauses);
    
	prfs_InsertUsableClause(Search, Given,TRUE);

	for(Scan = Derivables; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
	  Clause     = (CLAUSE)list_Car(Scan);
	  clause_SetDepth(Clause,0);
	}
	ClauseList = list_Nconc(ClauseList,Derivables);
	Derivables = list_Nil();
      }
    }
  }
  for(Scan = ClauseList; !list_Empty(Scan); Scan = list_Cdr(Scan))
    prfs_InsertUsableClause(Search, list_Car(Scan),TRUE);
  list_Delete(ClauseList);
  return EmptyClauses;
}

static CLAUSE red_SpecialInputReductions(CLAUSE Clause, FLAGSTORE Flags,
					 PRECEDENCE Precedence)
/*********************************************************
  INPUT:   A clause and a flag store.
  RETURNS: The clause where the logical constants TRUE, FALSE
           are removed.
  EFFECT:  The clause is destructively changed.
**********************************************************/
{
  intptr_t  i;
  int     	end;
  LIST    	Indexes;
  TERM    	Atom;

#ifdef CHECK
  clause_Check(Clause, Flags, Precedence);
#endif

  Indexes = list_Nil();
  end     = clause_LastAntecedentLitIndex(Clause);
    
  for (i = clause_FirstConstraintLitIndex(Clause); i <= end; i++) {
    Atom = clause_LiteralAtom(clause_GetLiteral(Clause,i));
    if (fol_IsTrue(Atom)) 
      Indexes = list_Cons((POINTER)i,Indexes);
  }

  end    = clause_LastSuccedentLitIndex(Clause);
    
  for (i = clause_FirstSuccedentLitIndex(Clause); i <= end; i++) {
    Atom = clause_LiteralAtom(clause_GetLiteral(Clause,i));
    if (fol_IsFalse(Atom)) 
      Indexes = list_Cons((POINTER)i,Indexes);
  }
  
  if (!list_Empty(Indexes)) {
    clause_DeleteLiterals(Clause,Indexes, Flags, Precedence);
    list_Delete(Indexes);
  }

  return Clause;
}


LIST red_ReduceInput(PROOFSEARCH Search, LIST ClauseList)
/*********************************************************
  INPUT:   A proof search object and a list of unshared clauses.
  RETURNS: A list of empty clauses.
  EFFECT:  Interreduces the clause list and inserts the clauses into <Search>.
	   Keeps track of derived and kept clauses.
	   Time limits and the DocProof flag are considered.
**********************************************************/
{
  CLAUSE Given;
  LIST   Scan, EmptyClauses, BackReduced;
  FLAGSTORE  Flags;
  PRECEDENCE Precedence;

  Flags        = prfs_Store(Search);
  Precedence   = prfs_Precedence(Search);
  EmptyClauses = list_Nil();
  ClauseList   = clause_ListSortWeighed(list_Copy(ClauseList));
  ClauseList   = split_ExtractEmptyClauses(ClauseList, &EmptyClauses);
  
  while (!list_Empty(ClauseList) && list_Empty(EmptyClauses) &&
	 (flag_GetFlagIntValue(Flags,flag_TIMELIMIT) == flag_TIMELIMITUNLIMITED ||
	  flag_GetFlagIntValue(Flags, flag_TIMELIMIT) > clock_GetSeconds(clock_OVERALL))) {
    Given = (CLAUSE)list_NCar(&ClauseList);
#ifdef CHECK
    if (!clause_IsClause(Given, Flags, Precedence)) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In red_ReduceInput :");
      misc_ErrorReport(" Illegal input.\n");
      misc_FinishErrorReport();
    }
#endif 
    Given = red_SpecialInputReductions(Given, Flags, Precedence);
    Given = red_ReductionOnDerivedClause(Search, Given, red_USABLE);
   if (Given) {
      if (!red_ExplicitTautology(Given,Flags,Precedence)) {
	prfs_IncKeptClauses(Search);
	if (clause_IsEmptyClause(Given))
	  EmptyClauses = list_List(Given);
	else {
	  BackReduced = red_BackReduction(Search, Given, red_USABLE);
	  prfs_IncDerivedClauses(Search, list_Length(BackReduced));
	  BackReduced = split_ExtractEmptyClauses(BackReduced, &EmptyClauses);
	  prfs_InsertUsableClause(Search, Given, FALSE);   /* Sort at end of function */
	  BackReduced = clause_ListSortWeighed(BackReduced);
	  ClauseList = red_MergeClauseListsByWeight(ClauseList, BackReduced);
	}
      }
      else
	clause_Delete(Given);
    }
  }
  for(Scan = ClauseList; !list_Empty(Scan); Scan = list_Cdr(Scan))
    if (red_ExplicitTautology(list_Car(Scan),Flags,Precedence))
      clause_Delete(list_Car(Scan));
    else
      prfs_InsertUsableClause(Search, list_Car(Scan),FALSE); /* Sort at end of function */
  list_Delete(ClauseList);
  prfs_SetUsableClauses(Search, clause_ListSortWeighed(prfs_UsableClauses(Search))); /* Sort Usable */
  return EmptyClauses;
}


LIST red_SatInput(PROOFSEARCH Search) 
/*********************************************************
  INPUT:   A proof search object.
  RETURNS: A list of derived empty clauses.
  EFFECT:  Does a saturation from the conjectures into the axioms/conjectures
	   Keeps track of derived and kept clauses. Keeps track of a possible
	   time limit.
	   Considers the Usable clauses in <Search> and a possible time limit.
**********************************************************/
{
  CLAUSE     Given;
  LIST       Scan, ClauseList, Derivables, EmptyClauses;
  int        n;
  FLAGSTORE  Flags;
  PRECEDENCE Precedence;

  Flags        = prfs_Store(Search);
  Precedence   = prfs_Precedence(Search);

  EmptyClauses = list_Nil();
  ClauseList   = list_Nil();
  Scan         = prfs_UsableClauses(Search);
  n            = list_Length(Scan);

  while(!list_Empty(Scan) &&
	n > 0 &&
	(flag_GetFlagIntValue(Flags,flag_TIMELIMIT) == flag_TIMELIMITUNLIMITED ||
	 flag_GetFlagIntValue(Flags, flag_TIMELIMIT) > clock_GetSeconds(clock_OVERALL))) {
    Given = (CLAUSE)list_Car(Scan);
    if (clause_GetFlag(Given,CONCLAUSE)) {
      Derivables = inf_BoundedDepthUnitResolution(Given,
						  prfs_UsableSharingIndex(Search),
						  FALSE, Flags, Precedence);
      n         -= list_Length(Derivables);
      ClauseList = list_Nconc(Derivables,ClauseList);
    }
    Scan = list_Cdr(Scan);
  }
  
  prfs_IncDerivedClauses(Search, list_Length(ClauseList));
  EmptyClauses = red_ReduceInput(Search, ClauseList);
  list_Delete(ClauseList);
  ClauseList   = list_Nil();
  if (list_Empty(EmptyClauses)) {
    Scan=prfs_UsableClauses(Search);
    while (!list_Empty(Scan) &&
	   n > 0 &&
	   (flag_GetFlagIntValue(Flags,flag_TIMELIMIT)==flag_TIMELIMITUNLIMITED ||
	    flag_GetFlagIntValue(Flags, flag_TIMELIMIT) > clock_GetSeconds(clock_OVERALL))) {
      Given = (CLAUSE)list_Car(Scan);
      if (clause_GetFlag(Given,CONCLAUSE) &&  clause_IsFromInput(Given)) {
	Derivables = inf_BoundedDepthUnitResolution(Given,
						    prfs_UsableSharingIndex(Search), 
						    TRUE, Flags, Precedence);
	n         -= list_Length(Derivables);
	ClauseList = list_Nconc(Derivables,ClauseList);
      }
      Scan = list_Cdr(Scan);
    }
    prfs_IncDerivedClauses(Search, list_Length(ClauseList));
    EmptyClauses = red_ReduceInput(Search, ClauseList);
    list_Delete(ClauseList);
  }
  return EmptyClauses;
}

void red_CheckSplitSubsumptionCondition(PROOFSEARCH Search)
/*********************************************************
  INPUT:   A proof search object.
  EFFECT:  For all deleted clauses in the split stack, it
           is checked whether they are subsumed by some
	   existing clause. If they are not, a core is dumped.
	   Used for debugging.
**********************************************************/
{
  LIST   Scan1,Scan2;
  CLAUSE Clause;
  FLAGSTORE  Flags;
  PRECEDENCE Precedence;

  Flags      = prfs_Store(Search);
  Precedence = prfs_Precedence(Search);

  for (Scan1=prfs_SplitStack(Search);!list_Empty(Scan1);Scan1=list_Cdr(Scan1))
    for (Scan2=prfs_SplitDeletedClauses(list_Car(Scan1));!list_Empty(Scan2);Scan2=list_Cdr(Scan2)) {
      Clause = (CLAUSE)list_Car(Scan2);
      if (!red_ForwardSubsumer(Clause, prfs_WorkedOffSharingIndex(Search),
			       Flags, Precedence,list_Nil()) &&
	  !red_ForwardSubsumer(Clause, prfs_UsableSharingIndex(Search),
			       Flags, Precedence, list_Nil()) &&
	  !red_ClauseDeletion(prfs_StaticSortTheory(Search),Clause,
			      Flags, Precedence)) {
	misc_StartErrorReport();
	misc_ErrorReport("\n In red_CheckSplitSubsumptionCondition: No clause found implying ");
	clause_Print(Clause);
	misc_ErrorReport("\n Current Split: ");
	prfs_PrintSplit(list_Car(Scan1));
	misc_FinishErrorReport();
      }
    }
}
