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
/* $Revision: 1.15 $                                        * */
/* $State: Exp $                                            * */
/* $Date: 2011-11-27 10:57:35 $                             * */
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


/* $RCSfile: clause.h,v $ */

#ifndef _CLAUSE_
#define _CLAUSE_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "sharing.h"
#include "foldfg.h"
#include "order.h"
#include "subst.h"
#include "flags.h"
#include "symbol.h"
#include "hashmap.h"

/**************************************************************/
/* Data Structures and Constants                              */
/**************************************************************/

/* Means weight of literal or clause is undefined */
extern const NAT clause_WEIGHTUNDEFINED;

extern int clause_CLAUSECOUNTER;

typedef enum {MAXIMAL=1, STRICTMAXIMAL=2, LITSELECT=4} MAXFLAG;

typedef enum {CLAUSE_DELETION, EMPTY_SORT, SORT_RESOLUTION,
	      EQUALITY_RESOLUTION, EQUALITY_FACTORING, MERGING_PARAMODULATION,
	      PARAMODULATION, ORDERED_PARAMODULATION,
	      SUPERPOSITION_RIGHT,  SUPERPOSITION_LEFT,
	      SIMPLE_HYPER, ORDERED_HYPER, UR_RESOLUTION,
	      GENERAL_RESOLUTION, GENERAL_FACTORING, SPLITTING, INPUTCLAUSE,
	      CONDENSING, ASSIGNMENT_EQUATION_DELETION, OBVIOUS_REDUCTIONS,
	      SORT_SIMPLIFICATION, REWRITING, CONTEXTUAL_REWRITING,
	      MATCHING_REPLACEMENT_RESOLUTION, UNIT_CONFLICT, DEFAPPLICATION,
	      TERMINATOR, TEMPORARY, ORDERED_CHAINING, NEGATIVE_CHAINING, COMPOSITION_RESOLUTION
        
} RULE;

typedef unsigned long SPLITFIELDENTRY;
typedef SPLITFIELDENTRY* SPLITFIELD;

typedef enum  {WORKEDOFF=1,CLAUSESELECT=2,DOCCLAUSE=4,CONCLAUSE=8,BLOCKED=16,
	       NOPARAINTO=32, MARKED=64, HIDDEN=128, MARK=256, TRANSAX=512} CLAUSE_FLAGS;

/* WORKEDOFF set if it is a worked off clause */
/* MARK is used for marking */
/* TRANSAX means that the clause has the form of transitivity axiom */

/* As there are a lot of implications a clauses properties may have */
/* for the prover, this information should be kept with the clause. */
/* That for a flagfield is foreseen, most likely an integer used    */
/* like the sort-Bitfield existing for term, now used for varoccs.  */

typedef struct CLAUSE_HELP{
  int      clausenumber;
  NAT      weight;     /* The sum of the weight of all literals */
  NAT      depth;      /* The depth of the clause in the derivation  */
  NAT      validlevel; /* Level of splitting where clause is valid. */
  float    splitpotential; /* -1 if not splittable, 0 if not tested, 
			      k>0 if splittable encoding reduction potential of split at start */
  SPLITFIELD splitfield;
  unsigned splitfield_length;
  
  LIST     parentCls, parentLits; /* Parents clauses' clause and lit numbers.*/
  NAT      flags;
  SYMBOL   maxVar;     /* The maximal variable symbol in the clause */
  struct LITERAL_HELP{
    NAT    maxLit;     /* for clause intern literal ordering */
    NAT    weight;     /* weight of the <atomWithSign> below */    
    ord_RESULT ord_stat; /* ordering status for special binary predicates used by chaining */
                         /* We also encode orientedness of equality predicates here:
                            equality is oriented iff (ord_stat == ord_GREATER_THAN).
                            Note that orienting means switching sides of arguments of equlity
                            when it can be established that (ord_stat == ord_SMALLER_THAN).
                         */
    struct CLAUSE_HELP *owningClause;
    TERM   atomWithSign; /* Pointer to the term, where an unshared
			    Term for the sign of negative literals
			    is supplied additionally.              */
  } **literals;        /* An Array of (c+a+s) literalpointers in this order. */
  int      c;          /* number of constraint literals */
  int      a;          /* number of antecedent literals */
  int	   s;          /* number of succedent literals */
  RULE    origin;
} *CLAUSE, CLAUSE_NODE;

typedef struct LITERAL_HELP *LITERAL, LITERAL_NODE;


/**************************************************************/
/* Functions Prototypes                                       */
/**************************************************************/

/**************************************************************/
/* Functions on clauses and literals creation and deletion.   */
/**************************************************************/

void    clause_Init(void);

CLAUSE  clause_CreateBody(int);
CLAUSE  clause_Create(LIST, LIST, LIST, FLAGSTORE, PRECEDENCE);
CLAUSE  clause_CreateSkolem(LIST, LIST, LIST, FLAGSTORE, PRECEDENCE);
CLAUSE  clause_CreateCrude(LIST, LIST, LIST, BOOL);
CLAUSE  clause_CreateUnnormalized(LIST, LIST, LIST);
CLAUSE  clause_CreateFromLiterals(LIST, BOOL, BOOL, BOOL, FLAGSTORE, PRECEDENCE);
CLAUSE  clause_CreateFromLiteralLists(LIST, LIST, LIST, BOOL, TERM);
void    clause_Delete(CLAUSE);

LITERAL clause_LiteralCreate(TERM, CLAUSE);
LITERAL clause_LiteralCreateNegative(TERM, CLAUSE);        /* Unused */
void    clause_LiteralDelete(LITERAL);

LIST    clause_CopyConstraint(CLAUSE);
LIST    clause_CopyAntecedentExcept(CLAUSE, int);
LIST    clause_CopySuccedent(CLAUSE);
LIST    clause_CopySuccedentExcept(CLAUSE, int);


/**************************************************************/
/* Functions to use the sharing for clauses and literals.     */
/**************************************************************/

void    clause_InsertIntoSharing(CLAUSE, SHARED_INDEX, FLAGSTORE, PRECEDENCE);
void    clause_DeleteFromSharing(CLAUSE, SHARED_INDEX, FLAGSTORE, PRECEDENCE);

NAT     clause_GetNumberOfInstances(TERM, SHARED_INDEX);

void    clause_MakeUnshared(CLAUSE, SHARED_INDEX);
void    clause_MoveSharedClause(CLAUSE, SHARED_INDEX, SHARED_INDEX, FLAGSTORE, PRECEDENCE);
void    clause_DeleteSharedLiteral(CLAUSE, int, SHARED_INDEX, FLAGSTORE, PRECEDENCE);

void    clause_LiteralInsertIntoSharing(LITERAL, SHARED_INDEX);
void    clause_LiteralDeleteFromSharing(LITERAL, SHARED_INDEX);  /* Used only in clause.c */

void    clause_DeleteClauseList(LIST);
void    clause_DeleteSharedClauseList(LIST, SHARED_INDEX, FLAGSTORE, PRECEDENCE);
void    clause_DeleteAllIndexedClauses(SHARED_INDEX, FLAGSTORE, PRECEDENCE); /* Necessary? */
void    clause_PrintAllIndexedClauses(SHARED_INDEX);       /* For Debugging */
LIST    clause_AllIndexedClauses(SHARED_INDEX);

/**************************************************************/
/* Clause Comparisons                                         */
/**************************************************************/

BOOL    clause_IsHornClause(CLAUSE);
int     clause_CompareAbstract(CLAUSE, CLAUSE);

/**************************************************************/
/* Clause and literal Input and Output Functions              */
/**************************************************************/

void    clause_Print(CLAUSE);
void    clause_PrintSpecial(CLAUSE);
void    clause_PrintSplitfield(CLAUSE, NAT);
void    clause_PrintSplitfieldOnly(SPLITFIELD, int);
void    clause_PrintVerbose(CLAUSE, FLAGSTORE, PRECEDENCE);
void    clause_PrintMaxLitsOnly(CLAUSE, FLAGSTORE, PRECEDENCE); /* For Debugging */
void    clause_FPrint(FILE*, CLAUSE);                /* For Debugging */
void    clause_FPrintRule(FILE*, CLAUSE);
void    clause_FPrintOtter(FILE*, CLAUSE);           /* Unused */
void    clause_FPrintCnfDFG(FILE* , BOOL, LIST, LIST, FLAGSTORE, PRECEDENCE);
void    clause_FPrintCnfDFGProof(FILE* , BOOL, LIST, LIST, FLAGSTORE, PRECEDENCE);
void    clause_FPrintCnfDFGProblem(FILE* , BOOL, const char*, const char*, const char*, const char*, LIST, LIST, FLAGSTORE, PRECEDENCE, HASHMAP, BOOL, BOOL);
void    clause_FPrintCnfFormulasDFGProblem(FILE* , BOOL, const char*, const char*, const char*, const char*, LIST, LIST, FLAGSTORE, PRECEDENCE); /* unused */
void    clause_FPrintCnfDFGDerivables(FILE*, LIST, BOOL);
void    clause_FPrintDFG(FILE*, CLAUSE, BOOL);
void    clause_FPrintDFGProof(FILE*, CLAUSE, BOOL);
void    clause_FPrintDFGStep(FILE*, CLAUSE, BOOL);
void    clause_FPrintFormulaDFG(FILE*, CLAUSE, BOOL);
void    clause_FPrintCnfOtter(FILE*, LIST, FLAGSTORE);
void    clause_FPrintSettings(FILE*, LIST, LIST, FLAGSTORE, PRECEDENCE, HASHMAP);

void    clause_LiteralPrint(LITERAL);                /* For Debugging */
void    clause_LiteralListPrint(LIST);               /* For Debugging */
void    clause_LiteralPrintUnsigned(LITERAL);        /* For Debugging */
void    clause_LiteralPrintSigned(LITERAL);          /* For Debugging */
void    clause_LiteralFPrint(FILE*, LITERAL);        /* For Debugging */
void    clause_LiteralFPrintUnsigned(FILE*, LITERAL);/* For Debugging */

void    clause_ListPrint(LIST);
void    clause_PrintPropModel(LIST, FLAGSTORE, PRECEDENCE);

void    clause_PrintParentClauses(CLAUSE);           /* For Debugging */
void    clause_PrintParentClausesSpecial(CLAUSE);    /* For Debugging */
void    clause_PrintOrigin(CLAUSE);                  /* For Debugging */
void    clause_FPrintOrigin(FILE*, CLAUSE);

/**************************************************************/
/* Specials                                                   */
/**************************************************************/

void    clause_SetSplitDataFromFather(CLAUSE Result, CLAUSE Father);
void    clause_SetSplitDataFromList(CLAUSE Result, LIST List);
void    clause_SetSplitDataFromParents(CLAUSE Result, CLAUSE Mother, CLAUSE Father);
void    clause_UpdateSplitField(CLAUSE C1, CLAUSE C2);
BOOL    clause_LiteralIsSort(LITERAL L);
TERM    clause_LiteralAtom(LITERAL L);

CLAUSE  clause_Copy(CLAUSE);
LITERAL clause_LiteralCopy(LITERAL);

LIST    clause_CopyClauseList(LIST List);


void    clause_DeleteLiteral(CLAUSE, int, FLAGSTORE, PRECEDENCE);
void    clause_DeleteLiteralNN(CLAUSE, int);
void    clause_DeleteLiterals(CLAUSE, LIST, FLAGSTORE, PRECEDENCE);  /* Unused */
LIST    clause_GetLiteralSubSetList(CLAUSE, int, int, FLAGSTORE, PRECEDENCE);
void    clause_ReplaceLiteralSubSet(CLAUSE, int, int, LIST, FLAGSTORE, PRECEDENCE);
void    clause_FixLiteralOrder(CLAUSE, FLAGSTORE, PRECEDENCE);

SYMBOL  clause_AtomMaxVar(TERM);
void    clause_SetMaxLitFlags(CLAUSE, FLAGSTORE, PRECEDENCE);
void    clause_SetMaxLitFlagsSkolem(CLAUSE, FLAGSTORE, PRECEDENCE);
void    clause_SetNativeMaxLitFlags(CLAUSE, FLAGSTORE, PRECEDENCE);
SYMBOL  clause_LiteralMaxVar(LITERAL);        /* Used only in clause.c */
SYMBOL  clause_SearchMaxVar(CLAUSE);
void    clause_UpdateMaxVar(CLAUSE);

BOOL clause_DLcontainsRedundantLiteral(CLAUSE Clause);

void    clause_RenameVarsBiggerThan(CLAUSE, SYMBOL);
void    clause_Normalize(CLAUSE);
void    clause_SetSortConstraint(CLAUSE, BOOL, FLAGSTORE, PRECEDENCE);
void    clause_SubstApply(SUBST, CLAUSE);
void    clause_ReplaceVariable(CLAUSE, SYMBOL, TERM);
void    clause_PrecomputeOrdering(CLAUSE, FLAGSTORE, PRECEDENCE);
void    clause_PrecomputeOrderingSkolem(CLAUSE, FLAGSTORE, PRECEDENCE);
NAT     clause_NumberOfVarOccs(CLAUSE);
NAT     clause_NumberOfSymbolOccurrences(CLAUSE, SYMBOL);
NAT     clause_ComputeWeight(CLAUSE, FLAGSTORE);
NAT     clause_LiteralComputeWeight(LITERAL, FLAGSTORE);
NAT     clause_ComputeTermDepth(CLAUSE);
NAT     clause_MaxTermDepthClauseList(LIST);
NAT     clause_ComputeSize(CLAUSE);
BOOL    clause_WeightCorrect(CLAUSE, FLAGSTORE, PRECEDENCE);   /* Unused */

LIST    clause_MoveBestLiteralToFront(LIST, SUBST, SYMBOL,
				      BOOL (*)(LITERAL, NAT, LITERAL, NAT));


LIST    clause_InsertWeighed(CLAUSE, LIST, FLAGSTORE, PRECEDENCE);
LIST    clause_ListSortWeighed(LIST);

BOOL    clause_HasTermSortConstraintLits(CLAUSE);
BOOL    clause_HasOnlySpecDomArgs(CLAUSE);
BOOL    clause_HasSolvedConstraint(CLAUSE);
BOOL    clause_HasSelectedLiteral(CLAUSE, FLAGSTORE, PRECEDENCE);
BOOL    clause_IsDeclarationClause(CLAUSE);
BOOL    clause_IsSortTheoryClause(CLAUSE, FLAGSTORE, PRECEDENCE);
BOOL    clause_IsPartOfDefinition(CLAUSE, TERM, intptr_t*, LIST);
BOOL    clause_IsPotentialSortTheoryClause(CLAUSE, FLAGSTORE, PRECEDENCE);
BOOL    clause_HasOnlyVarsInConstraint(CLAUSE, FLAGSTORE, PRECEDENCE);
BOOL    clause_HasSortInSuccedent(CLAUSE, FLAGSTORE, PRECEDENCE);
BOOL    clause_VarsOfClauseAreInTerm(CLAUSE, TERM);
BOOL    clause_ContainsPotPredDef(CLAUSE, FLAGSTORE, PRECEDENCE, NAT*, LIST*);
BOOL    clause_LitsHaveCommonVar(LITERAL, LITERAL);

void    clause_SelectLiteral(CLAUSE, FLAGSTORE);
void    clause_SetSpecialFlags(CLAUSE,BOOL, FLAGSTORE, PRECEDENCE);

BOOL    clause_LiteralIsLiteral(LITERAL);
BOOL    clause_IsClause(CLAUSE, FLAGSTORE, PRECEDENCE);
BOOL    clause_IsClauseSkolem(CLAUSE, FLAGSTORE, PRECEDENCE);
BOOL    clause_IsUnorderedClause(CLAUSE);
BOOL    clause_ContainsPositiveEquations(CLAUSE);
BOOL    clause_ContainsNegativeEquations(CLAUSE);
int     clause_ContainsFolAtom(CLAUSE,BOOL*,BOOL*,BOOL*,BOOL*);
BOOL    clause_ContainsVariables(CLAUSE);
BOOL    clause_ContainsFunctions(CLAUSE);
BOOL    clause_ContainsSymbol(CLAUSE, SYMBOL);
void    clause_ContainsSortRestriction(CLAUSE,BOOL*,BOOL*);
BOOL    clause_ImpliesFiniteDomain(CLAUSE);
BOOL    clause_ImpliesNonTrivialDomain(CLAUSE);
LIST    clause_FiniteMonadicPredicates(LIST);

CLAUSE  clause_GetNumberedCl(int, LIST);
LIST    clause_NumberSort(LIST);
LIST    clause_NumberDelete(LIST,int);
void    clause_Check(CLAUSE, FLAGSTORE, PRECEDENCE);
void    clause_CheckSkolem(CLAUSE, FLAGSTORE, PRECEDENCE);

void    clause_DeleteFlatFromIndex(CLAUSE, st_INDEX);
void    clause_InsertFlatIntoIndex(CLAUSE, st_INDEX);
void    clause_DeleteClauseListFlatFromIndex(LIST, st_INDEX);

RULE    clause_GetOriginFromString(const char*);

void    clause_CountSymbols(CLAUSE);

LIST    clause_ComputePropModel(LIST, FLAGSTORE, PRECEDENCE);

LIST    clause_ListOfPredicates(CLAUSE);
LIST    clause_ListOfConstants(CLAUSE);
LIST    clause_ListOfVariables(CLAUSE);
LIST    clause_ListOfFunctions(CLAUSE);

BOOL    clause_LiteralIsStrictMaximalLiteralSkolem(CLAUSE, int, FLAGSTORE, PRECEDENCE);

BOOL    clause_IsTransitivityAxiomExt(CLAUSE, SYMBOL*, BOOL*);
BOOL    clause_IsIrreflexivityAxiomExt(CLAUSE, SYMBOL*);
BOOL    clause_IsTotalityAxiomExt(CLAUSE, SYMBOL*);

BOOL clause_IsTransitivityAxiom(CLAUSE Clause); 

/* special output functions for clauses with parent pointers */
void clause_PParentsFPrint(FILE*, CLAUSE);
void clause_PParentsListFPrint(FILE*, LIST L);
void clause_PParentsPrint(CLAUSE);
void clause_PParentsListPrint(LIST);
void clause_PParentsFPrintGen(FILE*, CLAUSE, BOOL);


/**************************************************************/
/* Inline Functions                                           */
/**************************************************************/

/**************************************************************/
/* Accessing Literals 1                                       */
/**************************************************************/

TERM clause_LiteralSignedAtom(LITERAL L);
CLAUSE clause_LiteralOwningClause(LITERAL L);
void clause_LiteralSetOwningClause(LITERAL L, CLAUSE C);
void clause_LiteralSetOrderStatus(LITERAL L, ord_RESULT OS);
ord_RESULT clause_LiteralGetOrderStatus(LITERAL L);
NAT clause_LiteralWeight(LITERAL L);
void clause_UpdateLiteralWeight(LITERAL L, FLAGSTORE Flags);
void clause_LiteralFlagReset(LITERAL L);
void clause_LiteralFlagResetAndKeepSelFlag(LITERAL L);
 
BOOL clause_LiteralGetFlag(LITERAL L, MAXFLAG Flag);
void clause_LiteralSetFlag(LITERAL L, MAXFLAG Flag);
void clause_LiteralClearFlag(LITERAL L, MAXFLAG Flag);
BOOL clause_LiteralIsMaximal(LITERAL L);
BOOL clause_LiteralIsOrientedEquality(LITERAL L);
BOOL clause_LiteralIsNotOrientedEquality(LITERAL L);


/**************************************************************/
/* Literal Comparison 1                                       */
/**************************************************************/
BOOL clause_LiteralIsNegative(LITERAL L);
BOOL clause_LiteralIsPositive(LITERAL L);
BOOL clause_LiteralsAreComplementary(LITERAL L1, LITERAL L2);
BOOL clause_HyperLiteralIsBetter(LITERAL Dummy1, NAT S1,
						   LITERAL Dummy2, NAT S2);

/**************************************************************/
/* Accessing Literals 2                                       */
/**************************************************************/
SYMBOL clause_LiteralPredicate(LITERAL L);
BOOL clause_LiteralIsPredicate(LITERAL L);
BOOL clause_LiteralIsEquality(LITERAL L);
void clause_LiteralSetAtom(LITERAL L, TERM A);
void clause_LiteralSetNegAtom(LITERAL L, TERM A);
void clause_LiteralSetPosAtom(LITERAL L, TERM A);
void clause_NLiteralSetLiteral(LITERAL L, TERM LIT);

/**************************************************************/
/* Memory management                                          */
/**************************************************************/
void clause_LiteralFree(LITERAL L);

/**************************************************************/
/* Functions to access literals.                                 */
/**************************************************************/

LITERAL clause_GetLiteral(CLAUSE C, intptr_t Index);
void clause_SetLiteral(CLAUSE C, int Index, LITERAL L);
TERM clause_GetLiteralTerm(CLAUSE C, int Index);
TERM clause_GetLiteralAtom(CLAUSE C, intptr_t Index);
int clause_NumOfConsLits(CLAUSE Clause);
int clause_NumOfAnteLits(CLAUSE Clause);
int clause_NumOfSuccLits(CLAUSE Clause);
void clause_SetNumOfConsLits(CLAUSE Clause, int Number);
void clause_SetNumOfAnteLits(CLAUSE Clause, int Number);
void clause_SetNumOfSuccLits(CLAUSE Clause, int Number);
int clause_Length(CLAUSE Clause);
int clause_LastLitIndex(CLAUSE Clause);
intptr_t clause_FirstLitIndex(void);
int clause_FirstConstraintLitIndex(CLAUSE Clause);
intptr_t clause_FirstAntecedentLitIndex(CLAUSE Clause);
intptr_t clause_FirstSuccedentLitIndex(CLAUSE Clause);
int clause_LastConstraintLitIndex(CLAUSE Clause);
int clause_LastAntecedentLitIndex(CLAUSE Clause);
int clause_LastSuccedentLitIndex(CLAUSE Clause);
LIST clause_GetLiteralList(CLAUSE Clause);
LIST clause_GetLiteralListExcept(CLAUSE Clause, int Index);

/**************************************************************/
/* Clause Access Macros                                       */
/**************************************************************/
int clause_Counter(void);
void clause_SetCounter(int Value);
int clause_IncreaseCounter(void);
void clause_DecreaseCounter(void);
NAT clause_Depth(CLAUSE Clause);
void clause_SetDepth(CLAUSE Clause, NAT NewDepth);
NAT clause_Weight(CLAUSE Clause);
void clause_UpdateWeight(CLAUSE Clause, FLAGSTORE Flags);
float clause_SplitPotential(CLAUSE Clause);
void clause_SetSplitPotential(CLAUSE Clause, float pot);
intptr_t clause_Number(const CLAUSE Clause);
void clause_SetNumber(CLAUSE Clause, int Number);
void clause_NewNumber(CLAUSE Clause);
NAT clause_SplitLevel(CLAUSE Clause);
BOOL clause_CheckSplitLevel(CLAUSE Clause);
NAT clause_SplitLevelDependencies(CLAUSE Clause);
LIST clause_ParentClauses(CLAUSE Clause);
LIST clause_ParentLiterals(CLAUSE Clause);
SYMBOL clause_MaxVar(CLAUSE Clause);
void clause_SetMaxVar(CLAUSE Clause, SYMBOL Variable);
RULE clause_Origin(CLAUSE Clause);
BOOL clause_Exists(CLAUSE Clause);
BOOL clause_LiteralExists(LITERAL L);
CLAUSE clause_Null(void);
void clause_SetSplitLevel(CLAUSE Clause, NAT Level);
void clause_InitSplitData(CLAUSE C);
void clause_SetSplitField(CLAUSE Clause, SPLITFIELD B,
					    unsigned Length);
NAT  clause_ComputeSplitFieldAddress(NAT n, NAT* field);
void clause_ExpandSplitField(CLAUSE C, NAT Length);
void clause_ClearSplitField(CLAUSE C);
	
void clause_SetSplitFieldBit(CLAUSE Clause, NAT n);
BOOL clause_GetFlag(CLAUSE Clause, CLAUSE_FLAGS Flag);
void clause_SetFlag(CLAUSE Clause, CLAUSE_FLAGS Flag);
void clause_RemoveFlag(CLAUSE Clause, CLAUSE_FLAGS Flag);
void clause_ClearFlags(CLAUSE Clause);
BOOL clause_DependsOnSplitLevel(CLAUSE C, NAT N);
void clause_UpdateSplitDataFromNewSplitting(CLAUSE Result,
							      CLAUSE Father,
							      NAT Level);
void clause_UpdateSplitDataFromPartner(CLAUSE Result,
							 CLAUSE Partner);
void clause_SetParentClauses(CLAUSE Clause, LIST PClauses);
void clause_AddParentClause(CLAUSE Clause, intptr_t PClause);
void clause_SetParentLiterals(CLAUSE Clause, LIST PLits);
void clause_AddParentLiteral(CLAUSE Clause, intptr_t PLit);
BOOL clause_ValidityIsNotSmaller(CLAUSE C1, CLAUSE C2);
BOOL clause_IsMoreValid(CLAUSE C1, CLAUSE C2);
BOOL  clause_CompareAbstractLEQ (CLAUSE Left, CLAUSE Right); 
BOOL clause_IsFromRewriting(CLAUSE Clause);
BOOL clause_IsFromCondensing(CLAUSE Clause);
BOOL clause_IsFromObviousReductions(CLAUSE Clause);
BOOL clause_IsFromSortSimplification(CLAUSE Clause);
BOOL clause_IsFromMatchingReplacementResolution(CLAUSE Clause);
BOOL clause_IsFromClauseDeletion(CLAUSE Clause);
BOOL clause_IsFromEmptySort(CLAUSE Clause);
BOOL clause_IsFromSortResolution(CLAUSE Clause);
BOOL clause_IsFromUnitConflict(CLAUSE Clause);
BOOL clause_IsFromEqualityResolution(CLAUSE Clause);
BOOL clause_IsFromEqualityFactoring(CLAUSE Clause);
BOOL clause_IsFromMergingParamodulation(CLAUSE Clause);
BOOL clause_IsFromSuperpositionRight(CLAUSE Clause);
BOOL clause_IsFromSuperpositionLeft(CLAUSE Clause);
BOOL clause_IsFromGeneralResolution(CLAUSE Clause);
BOOL clause_IsFromGeneralFactoring(CLAUSE Clause);
BOOL clause_IsFromSplitting(CLAUSE Clause);
BOOL clause_IsFromDefApplication(CLAUSE Clause);
BOOL clause_IsFromTerminator(CLAUSE Clause);
BOOL clause_IsTemporary(CLAUSE Clause);
BOOL clause_IsFromInput(CLAUSE Clause);
BOOL clause_IsFromOrderedChaining(CLAUSE Clause);
BOOL clause_IsFromNegativeChaining(CLAUSE Clause);
BOOL clause_IsFromCompositionResolution(CLAUSE Clause);
BOOL clause_HasReducedPredecessor(CLAUSE Clause);
BOOL clause_IsSplitFather(CLAUSE C1, CLAUSE C2);
void clause_SetFromRewriting(CLAUSE Clause);
void clause_SetFromContextualRewriting(CLAUSE Clause);
void clause_SetFromUnitConflict(CLAUSE Clause);
void clause_SetFromCondensing(CLAUSE Clause);
void clause_SetFromAssignmentEquationDeletion(CLAUSE Clause);
void clause_SetFromObviousReductions(CLAUSE Clause);
void clause_SetFromSortSimplification(CLAUSE Clause);
void clause_SetFromMatchingReplacementResolution(CLAUSE Clause);
void clause_SetFromClauseDeletion(CLAUSE Clause);
void clause_SetFromEmptySort(CLAUSE Clause);
void clause_SetFromSortResolution(CLAUSE Clause);
void clause_SetFromEqualityResolution(CLAUSE Clause);
void clause_SetFromEqualityFactoring(CLAUSE Clause);
void clause_SetFromMergingParamodulation(CLAUSE Clause);
void clause_SetFromParamodulation(CLAUSE Clause);
void clause_SetFromOrderedParamodulation(CLAUSE Clause);
void clause_SetFromSuperpositionRight(CLAUSE Clause);
void clause_SetFromSuperpositionLeft(CLAUSE Clause);
void clause_SetFromGeneralResolution(CLAUSE Clause);
void clause_SetFromOrderedHyperResolution(CLAUSE Clause);
void clause_SetFromSimpleHyperResolution(CLAUSE Clause);
void clause_SetFromURResolution(CLAUSE Clause);
void clause_SetFromGeneralFactoring(CLAUSE Clause);
void clause_SetFromSplitting(CLAUSE Clause);
void clause_SetFromDefApplication(CLAUSE Clause);
void clause_SetFromTerminator(CLAUSE Clause);
void clause_SetTemporary(CLAUSE Clause);
void clause_SetFromOrderedChaining(CLAUSE Clause);
void clause_SetFromNegativeChaining(CLAUSE Clause);
void clause_SetFromCompositionResolution(CLAUSE Clause);
void clause_SetFromInput(CLAUSE Clause);
LITERAL clause_FirstConstraintLit(CLAUSE Clause);
LITERAL clause_FirstAntecedentLit(CLAUSE Clause);
LITERAL clause_FirstSuccedentLit(CLAUSE Clause);
LITERAL clause_LastConstraintLit(CLAUSE Clause);
LITERAL clause_LastAntecedentLit(CLAUSE Clause);
LITERAL clause_LastSuccedentLit(CLAUSE Clause);
BOOL clause_HasEmptyConstraint(CLAUSE Clause);
BOOL clause_HasEmptyAntecedent(CLAUSE Clause);
BOOL clause_HasEmptySuccedent(CLAUSE Clause);
BOOL clause_IsGround(CLAUSE Clause);
BOOL clause_IsEmptyClause(CLAUSE C);
intptr_t clause_LiteralGetIndex(LITERAL L);
BOOL clause_LiteralIsFromConstraint(LITERAL Literal);
BOOL clause_LiteralIsFromAntecedent(LITERAL Literal);
BOOL clause_LiteralIsFromSuccedent(LITERAL Literal);
BOOL clause_IsSimpleSortClause(CLAUSE Clause);
BOOL clause_IsSubsortClause(CLAUSE Clause);
BOOL clause_HasSuccLits(CLAUSE Clause);
BOOL clause_HasGroundSuccLit(CLAUSE Clause);
LITERAL clause_GetGroundSuccLit(CLAUSE Clause);
void clause_Free(CLAUSE Clause);
void clause_ReInit(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence);
void clause_ReInitSkolem(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence);
void clause_PrecomputeOrderingAndReInit(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence);
void clause_PrecomputeOrderingAndReInitSkolem(CLAUSE Clause, FLAGSTORE Flags, PRECEDENCE Precedence);
void clause_SetDataFromFather(CLAUSE Result, CLAUSE Father, int i, FLAGSTORE Flags, PRECEDENCE Precedence);
void clause_SetDataFromParents(CLAUSE, CLAUSE, int, CLAUSE, int, FLAGSTORE, PRECEDENCE);
SPLITFIELD clause_GetSplitfield(CLAUSE, unsigned *);

#endif
