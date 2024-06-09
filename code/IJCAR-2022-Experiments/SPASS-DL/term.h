/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                       TERMS                            * */
/* *                                                        * */
/* *  $Module:   TERM                                       * */
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
/* $Revision: 1.4 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-25 10:05:32 $                             * */
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


/* $RCSfile: term.h,v $ */


#ifndef _TERM_
#define _TERM_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "symbol.h"
#include "stack.h"

/**************************************************************/
/* Structures                                                 */
/**************************************************************/

/* In CHECK mode you can block the stamp for all terms,
   so no other function can use the macro term_IncStamp(),
   which must be used for initialization at the beginning
   of a new traversal */

typedef struct term {
  SYMBOL symbol;
  union {
    LIST termlist;
    struct term* term;
  } super;
  LIST   args;
  NAT    stamp;
  NAT    size;
} *TERM, TERM_NODE;


/* Data Structures and Macros for Marking of variables, used in */
/* all functions extracting variables from terms.               */

extern NAT      term_MARK;
extern POINTER  term_BIND[symbol__MAXVARIABLES][2];

#ifdef CHECK
extern BOOL term_BINDPHASE;
void term_StartBinding(void); 
void term_StopBinding(void);  
BOOL term_InBindingPhase(void);
#else
void term_StartBinding(void);   
void term_StopBinding(void); 
#endif


NAT 	term_NullMark(void);
NAT 	term_BindingMark(SYMBOL Var);
void 	term_SetBindingMark(SYMBOL Var, NAT Mark);
POINTER term_BindingValue(SYMBOL Var);
void 	term_SetBindingValue(SYMBOL Var, POINTER Value);
void 	term_CreateBinding(SYMBOL Var, NAT Mark);
void 	term_ClearBinding(SYMBOL Var);
void 	term_CreateValueBinding(SYMBOL Var, NAT Mark, POINTER Value);
BOOL 	term_VarIsMarked(SYMBOL Var, NAT Mark);
NAT 	term_ActMark(void);
void 	term_NewMark(void);
NAT 	term_OldMark(void);

/**************************************************************/
/* Macros                                                     */
/**************************************************************/

TERM 	term_Null(void);
SYMBOL 	term_TopSymbol(TERM T);
void 	term_RplacTop(TERM T, SYMBOL S);
LIST 	term_SupertermList(TERM T);
void 	term_RplacSupertermList(TERM T, LIST L);
LIST 	term_AtomsLiterals(TERM T);
TERM 	term_Superterm(TERM T);
void 	term_RplacSuperterm(TERM T1, TERM T2);
BOOL 	term_IsVariable(TERM T);
BOOL 	term_IsStandardVariable(TERM T);
BOOL 	term_IsIndexVariable(TERM T);
LIST 	term_ArgumentList(TERM T);
void 	term_RplacArgumentList(TERM T, LIST A);
BOOL 	term_IsComplex(TERM T);
BOOL 	term_IsConstant(TERM T);
BOOL 	term_IsAtom(TERM T);
BOOL 	term_IsDeclaration(TERM Term);
TERM 	term_FirstArgument(TERM T);
void 	term_RplacFirstArgument(TERM T1, TERM T2);
TERM 	term_SecondArgument(TERM T);
void 	term_RplacSecondArgument(TERM T1, TERM T2);
void 	term_Free(TERM T);
BOOL 	term_EqualTopSymbols(TERM T, TERM S);
void 	term_EqualitySwap(TERM T);

/**************************************************************/
/* Macros and Variables for the term's stamp                  */
/**************************************************************/

/* Maximum number of modules that use the term module's stamp */
#define term_MAXSTAMPUSERS  20
extern 	NAT  term_STAMP;
extern 	BOOL term_STAMPBLOCKED;
void 	term_StopStamp(void);
void 	term_StartStamp(void);
void 	term_StopStamp(void);
NAT 	term_TermStamp(TERM T);
void 	term_SetTermStamp(TERM T);
NAT 	term_Size(TERM T);
void 	term_SetSize(TERM T, NAT s);
BOOL 	term_AlreadyVisited(TERM T);
BOOL 	term_HasTermStamp(TERM T);
void  	term_ResetTermStamp(TERM T);
BOOL 	term_StampAlreadyReset(TERM T);


/**************************************************************/
/* Functions on Term Creation And Deletion                    */
/**************************************************************/
void  term_Init(void);
TERM  term_Create(SYMBOL, LIST);
TERM  term_CreateAddFather(SYMBOL, LIST);
TERM  term_CreateStandardVariable(void);
void  term_Delete(TERM);
void  term_DeleteIterative(TERM);

/**************************************************************/
/* Term Comparisons                                           */
/**************************************************************/
BOOL  term_Equal(TERM, TERM);
BOOL  term_EqualIterative(TERM, TERM);               /* Unused */
BOOL  term_VariableEqual(TERM, TERM);
BOOL  term_IsGround(TERM);
BOOL  term_IsTerm(TERM);
BOOL  term_IsTermList(LIST);
BOOL  term_AllArgsAreVar(TERM);
int   term_CompareBySymbolOccurences(TERM, TERM);
int   term_CompareAbstract(TERM, TERM);
BOOL  term_CompareAbstractLEQ(TERM, TERM);


/**************************************************************/
/* Low Level Term Functions                                   */
/**************************************************************/
TERM  term_Copy(TERM);
TERM  term_CopyIterative(TERM);                     /* Unused */
TERM  term_CopyWithEmptyArgListNode(TERM, LIST, LIST*);
void  term_PrintWithEmptyArgListNode(TERM);
BOOL  term_ReplaceSubtermBy(TERM, TERM, TERM);
void  term_ReplaceVariable(TERM, SYMBOL, TERM);
void  term_ExchangeVariable(TERM, SYMBOL, SYMBOL);
BOOL  term_SubstituteVariable(SYMBOL, TERM, TERM);
NAT   term_ComputeSize(TERM);
void  term_InstallSize(TERM);
NAT   term_Depth(TERM);
BOOL  term_ContainsSymbol(TERM, SYMBOL);
BOOL  term_Sharing(TERM);
void  term_AddFatherLinks(TERM);
BOOL  term_FatherLinksEstablished(TERM);
TERM  term_TopLevelTerm(TERM);
BOOL  term_HasPointerSubterm(TERM, TERM);
BOOL  term_HasSubterm(TERM, TERM);
BOOL  term_HasProperSuperterm(TERM, TERM);
TERM  term_FindSubterm(TERM, SYMBOL);
LIST  term_FindAllAtoms(TERM, SYMBOL);
BOOL  term_CheckTerm(TERM);
NAT   term_RootDistance(TERM);
BOOL  term_RootDistanceSmaller(TERM,TERM);
LIST  term_CopyTermList(LIST List);
void  term_CopyTermsInList(LIST List);
void  term_DeleteTermList(LIST List);
BOOL  term_ListContainsTerm(LIST List, TERM Term);
LIST  term_DeleteDuplicatesFromList(LIST List);
LIST  term_DestroyDuplicatesInList(LIST Terms);

/**************************************************************/
/* Term Input and Output Functions                            */
/**************************************************************/

void   term_Print(TERM);
void   term_PrettyPrint(TERM);
void   term_FPrint(FILE*, TERM);
void   term_TermListPrint(LIST);
void   term_TermListFPrint(FILE*, LIST);
void   term_PrintPrefix(TERM);
void   term_FPrintPrefix(FILE*, TERM);
void   term_TermListPrintPrefix(LIST);
void   term_TermListFPrintPrefix(FILE*, LIST);
void   term_FPrintOtterPrefix(FILE*, TERM);
void   term_TermListFPrintOtterPrefix(FILE*, LIST);
void   term_FPrintPosition(FILE*,TERM,TERM);
void term_PrintPosition(TERM TopTerm, TERM Subterm);

/**************************************************************/
/* High Level Term Functions                                  */
/**************************************************************/
void   term_ToCoVariables(TERM);
NAT    term_Bytes(TERM);
void   term_MarkVariables(TERM, NAT);
void   term_CountSymbols(TERM);
LIST   term_ListOfVariables(TERM);
LIST   term_VariableSymbols(TERM);
LIST   term_ListOfAtoms(TERM,SYMBOL);
LIST   term_ListOfConstants(TERM);
LIST   term_ListOfFunctions(TERM);
NAT    term_NumberOfVarOccs(TERM);
NAT    term_NumberOfSymbolOccurrences(TERM, SYMBOL);
BOOL   term_ContainsFunctions(TERM);
BOOL   term_ContainsVariable(TERM,SYMBOL);
SYMBOL term_MaxVar(TERM);
void   term_StartMinRenaming(void);
void   term_StartMaxRenaming(SYMBOL);
TERM   term_Rename(TERM);
SYMBOL term_GetRenamedVarSymbol(SYMBOL);
LIST   term_RenamePseudoLinear(TERM, SYMBOL);
/**************************************************************/
/* Stamp Functions                                            */
/**************************************************************/
/* Currently only in CHECK mode */
#ifdef CHECK
void   term_StartStamp(void);
#endif

void   term_SetTermSubtermStamp(TERM T);

NAT    term_GetStampID(void);
BOOL   term_StampOverflow(NAT);


#endif
