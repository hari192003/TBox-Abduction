/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                     SYMBOLS                            * */
/* *                                                        * */
/* *  $Module:   SYMBOL                                     * */ 
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
/* $Revision: 1.14 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2016-03-23 13:39:00 $                             * */
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


/* $RCSfile: symbol.h,v $ */


#ifndef _SYMBOL_
#define _SYMBOL_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "list.h"
#include "strings.h"
#include "hashmap.h"
#include "array.h"

/**************************************************************/
/* Data Structures and Constants                              */
/**************************************************************/

/* Critical values: the maximum length of a symbol string and the */
/* maximum number of different variables (in one term, clause)    */
/* CAUTION: symbol__MAXVARIABLES is the overall number of         */
/*          variables + 1 to provide safe allocation of contexts  */
/*          ... because the first var begins with 1 instead of 0. */

#define symbol__SYMBOLVARMAXLEN 7  

#define symbol__NOOFSTANDARDVAR 2000
#define symbol__NOOFINDEXVAR    2000

#define symbol__MAXSTANDARDVAR  symbol__NOOFSTANDARDVAR
#define symbol__MAXINDEXVAR     (symbol__NOOFSTANDARDVAR + symbol__NOOFINDEXVAR)

#define symbol__NOOFVARIABLES (symbol__NOOFSTANDARDVAR + symbol__NOOFINDEXVAR)
#define symbol__MAXVARIABLES  (symbol__NOOFVARIABLES + 1)

/* Symbol Types, Symbols are represented as integers. In case of */
/* constants, functions, predicates, junctors, the two least     */
/* significant bits encode the type. Variables are just          */
/* positive integers, all other symbols negative integers        */
/* The third least significant bit encodes the status of         */
/* function symbols (lexicographic or multiset)                  */

extern const int symbol_MASK;
extern const int symbol_TYPEMASK;
extern const int symbol_STATMASK;
extern const int symbol_TYPESTATMASK;

extern const int symbol_ARBITRARYARITY;

extern const int symbol_TYPEBITS;
extern const int symbol_STATBITS;
extern const int symbol_TYPESTATBITS;

extern const int symbol_SIGTYPES;

#define symbol_CONSTANT  0 
#define symbol_FUNCTION  1
#define symbol_PREDICATE 2
#define symbol_JUNCTOR   3

#define symbol_STATLEX   0 
#define symbol_STATMUL   1

/* For constants, functions, predicates, junctors, is a special */
/* symbol table available, containing the arity, status and the */
/* print name.                                                  */

typedef intptr_t SYMBOL;
typedef int *PRECEDENCE;

typedef struct signature {
  char         *name;         /* The name of the symbol as a string */
  NAT          length;        /* The length of the name. Needed for efficient printing */
  union {
    int          weight;        /* The weight of the symbol for ordering purposes in case of no sort */
    int          index;         /* In case of a sort the index in the sort array */
  } number;
  int          arity;         /* The arity of the symbol */
  NAT          props;         /* Special Properties of the symbol, e.g. AC, Skolem,... */
  SYMBOL       info;          /* 2 LSB denote Type and 3rd LSB denotes status */
  LIST         generatedBy;
  BOOL         fixed;         /* used when parsing to decide, whether the type of the given symbol has already been fixed*/
  unsigned	DLMark;
} SIGNATURE_NODE, *SIGNATURE;

typedef enum {SKOLEM=1, CUMMUTATIVE=2, ASSOCIATIVE=4, ORDRIGHT=8, ORDMUL=16,
	      DECLSORT=32, DOMPRED=64, ISDEF=128, FREELY=256, GENERATED=512, SELECTED=1024,
	      PREDEFINED=2048, TRANSITIVE=4096
} SPROPERTY;

/* TRANSITIVE: a binary predicate symbol about which we know that behaves transitively in the theory. */
/* SORT: a mondic predicate solely used as a sort and not as a predicate in a formula */

#define symbol__MAXSIGNATURE 800000

extern SIGNATURE *symbol_SIGNATURE;


extern SYMBOL symbol_STANDARDVARCOUNTER;
extern SYMBOL symbol_INDEXVARCOUNTER;

extern int symbol_ACTINDEX;
extern int symbol_ACTSKOLEMFINDEX;
extern int symbol_ACTSKOLEMCINDEX;
extern int symbol_ACTSKOLEMPINDEX;
extern int symbol_ACTSKOLEMAINDEX;

/* For matching of signature symbols */
extern SYMBOL symbol_VARCONT[symbol__MAXSIGNATURE];

/**************************************************************/
/* Specials                                                   */
/**************************************************************/

NAT             symbol_MaxStringLength(void);

void            symbol_ReinitGenericNameCounters(void);

int             symbol_GetIncreasedOrderingCounter(void);

void            symbol_Delete(SYMBOL);
BOOL            symbol_IsSymbol(SYMBOL);
BOOL            symbol_IsFreed(SYMBOL);
void            symbol_Dump(PRECEDENCE);

LIST            symbol_SortByPrecedence(LIST, PRECEDENCE);
void            symbol_RearrangePrecedence(PRECEDENCE, LIST);

void            symbol_LowerSignature(void);

LIST            symbol_GetAllSymbols(void);
LIST            symbol_GetAllPredicates(void);
LIST            symbol_GetAllFunctions(void);
LIST            symbol_GetAllSymbolsWithProperty(SPROPERTY Property);

void            symbol_SetCount(SYMBOL, unsigned long);
unsigned long   symbol_GetCount(SYMBOL);

intptr_t             symbol_Ordering(PRECEDENCE, SYMBOL);

void            symbol_CheckIndexInRange(int);
void            symbol_CheckNoVariable(SYMBOL);

/**************************************************************/
/* Symbol Comparisons                                         */
/**************************************************************/

 BOOL symbol_Equal(SYMBOL A, SYMBOL B);

 BOOL symbol_IsSignature(SYMBOL S);

 int symbol_Type(SYMBOL ActSymbol);

 BOOL symbol_IsJunctor(SYMBOL S);

 BOOL symbol_IsFunction(SYMBOL S);

 BOOL symbol_IsConstant(SYMBOL S);

 BOOL symbol_IsPredicate(SYMBOL S);

 BOOL symbol_IsVariable(SYMBOL S);

 BOOL symbol_IsStandardVariable(SYMBOL S);

 BOOL symbol_IsIndexVariable(SYMBOL S);

 BOOL symbol_IsComplex(SYMBOL S);

 BOOL symbol_IsSuccessor(SYMBOL S, SYMBOL P);

/**************************************************************/
/* Symbol Manipulation                                        */
/**************************************************************/

int symbol_GetInitialStandardVarCounter(void);

int symbol_GetInitialIndexVarCounter(void);

 int symbol_FirstIndexVariable(void);

 int symbol_LastIndexVariable(void);

/* Special predefined symbols            */

#define symbol__NULL 0

 int symbol_MaxVars(void);

 int symbol_MaxConsts(void);

 int symbol_MaxBaseSorts(void);

 int symbol_TypeBits(void);

 int symbol_Null(void);

 int symbol_ActIndex(void);

 void symbol_ResetSkolemIndex(void);


/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *  MEMORY MANAGEMENT                                     * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/

 SIGNATURE symbol_GetSignature(void);


/**************************************************************/
/* Symbol Creation                                            */
/**************************************************************/

 SYMBOL symbol_CreateStandardVariable(void);

 SYMBOL symbol_CreateIndexVariable(void);

 SYMBOL symbol_NextIndexVariable(SYMBOL Variable);

 void symbol_SetStandardVarCounter(SYMBOL Variable);

 SYMBOL symbol_FirstVariable(void);

 BOOL symbol_GreaterVariable(SYMBOL Var1, SYMBOL Var2);

 void symbol_ResetStandardVarCounter(void);

void   symbol_Init(BOOL);
BOOL   symbol_SignatureExists(void);
void   symbol_FreeAllSymbols(void);
SYMBOL symbol_CreateFunction(const char*, int, int, PRECEDENCE);
SYMBOL symbol_CreateSkolemFunction(int, PRECEDENCE);
SYMBOL symbol_CreateSkolemPredicate(int, PRECEDENCE);
SYMBOL symbol_CreatePredicate(const char*, int, int, PRECEDENCE);
SYMBOL symbol_CreateJunctor(const char*, int, int, PRECEDENCE);

/**************************************************************/
/* Symbol Access                                              */
/**************************************************************/

SYMBOL symbol_Lookup(const char*);
SYMBOL symbol_LookupVariable(const char*);

 int symbol_VarIndex(SYMBOL ActSymbol);

 int symbol_NormVar(SYMBOL ActSymbol);


 int symbol_Index(SYMBOL ActSymbol);

 SYMBOL symbol_SignatureSymbol(int ActIndex, int Type, int Status);

 SIGNATURE symbol_Signature(int Index);

 SYMBOL symbol_GetSigSymbol(int Index);

 int symbol_Stat(SYMBOL ActSymbol);

 SYMBOL symbol_ChangeType(SYMBOL S, int Type);

 int symbol_Arity(SYMBOL ActSymbol);

 NAT symbol_PositiveArity(SYMBOL ActSymbol);

 void symbol_SetArity(SYMBOL ActSymbol, int Arity);

 BOOL symbol_Fixed(SYMBOL ActSymbol);

 void symbol_SetFixed(SYMBOL ActSymbol, BOOL fixed);

 int symbol_ArbitraryArity(void);

 const char* symbol_Name(SYMBOL ActSymbol);

 NAT symbol_NameLength(SYMBOL ActSymbol);

 int symbol_Info(SYMBOL ActSymbol);

 int symbol_Weight(SYMBOL ActSymbol);

 void symbol_SetWeight(SYMBOL ActSymbol, int Weight);

void symbol_SetName(SYMBOL, char*);

 LIST symbol_GeneratedBy(SYMBOL S);

 BOOL symbol_IsGeneratedBy(SYMBOL S1, SYMBOL S2);

 void symbol_SetGeneratedBy(SYMBOL S, LIST SymbolList);

 void symbol_SetOrdering(PRECEDENCE P, SYMBOL ActSymbol,
					  int Ordering);

 void symbol_SetIncreasedOrdering(PRECEDENCE P, SYMBOL S);

 BOOL symbol_PrecedenceGreater(PRECEDENCE P, SYMBOL S1, SYMBOL S2);

 BOOL symbol_HasProperty(SYMBOL ActSymbol, SPROPERTY Property);

 void symbol_AddProperty(SYMBOL ActSymbol, SPROPERTY Property);

 void symbol_RemoveProperty(SYMBOL ActSymbol, SPROPERTY Property);

 BOOL symbol_IsPredefined(SYMBOL Symbol);

 BOOL symbol_IsPredefinedPred(SYMBOL Symbol);

 BOOL symbol_IsBaseSort(SYMBOL Symbol);

 void symbol_ClearPrecedence(PRECEDENCE P);

 PRECEDENCE symbol_CreatePrecedence(void);

 void symbol_DeletePrecedence(PRECEDENCE P);

 void symbol_TransferPrecedence(PRECEDENCE Source,
						 PRECEDENCE Target);
  
 LIST symbol_DeleteSymbolFromList(LIST Symbols, SYMBOL S);
  
 void symbol_DeleteSymbolList(LIST Symbols);

 NAT symbol_DLGetMaxSigBound(void);

 void symbol_DLMark(SYMBOL S);

 BOOL symbol_DLIsMarked(SYMBOL S);

 void symbol_DLUnmarkAll();
  
/**************************************************************/
/* Symbol VARCONT                                             */
/**************************************************************/

 BOOL symbol_ContextIsClean(void);

 void symbol_ContextClean(void);

 BOOL symbol_ContextIsMapped(SYMBOL Symbol);

 SYMBOL symbol_ContextGetValue(SYMBOL Symbol);

 void symbol_ContextSetValue(SYMBOL Symbol, SYMBOL Value);

 void symbol_ContextClearValue(SYMBOL Symbol);

 BOOL symbol_ContextIsBound(SYMBOL Symbol);

/**************************************************************/
/* Symbol Output                                              */
/**************************************************************/

void   symbol_Print(SYMBOL);
void   symbol_PrintPrecedence(PRECEDENCE);
void   symbol_FPrintPrecedence(FILE*, PRECEDENCE);
void   symbol_FPrintDFGSignature(FILE*);
void   symbol_FPrint(FILE*, SYMBOL);
void   symbol_FPrintTPTP(FILE*, SYMBOL);
void   symbol_FPrintOtter(FILE*, SYMBOL);
void   symbol_PrintLn(SYMBOL);
void   symbol_PrintAll(void);

#endif
