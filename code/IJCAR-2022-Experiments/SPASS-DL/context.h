/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                VARCONTS FOR VARIABLES                  * */
/* *                                                        * */
/* *  $Module:   VARCONT                                    * */
/* *                                                        * */
/* *  Copyright (C) 1997, 1998, 1999, 2000, 2001            * */
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
/* $Revision: 1.7 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-11-27 11:18:20 $                             * */
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


/* $RCSfile: context.h,v $ */


#define SHOWBINDINGS 0

#ifndef _VARCONT_
#define _VARCONT_


/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "term.h"
#include "symbol.h"
#include "list.h"

/**************************************************************/
/* Structures                                                 */
/**************************************************************/

/* Set 'SHOWBINDINGS' to non-zero value to enable debug output. */
/* #define SHOWBINDINGS 1 */

#define cont__SIZE symbol__MAXVARIABLES

extern int  cont_NOOFVARCONTS;
extern LIST cont_LISTOFVARCONTS;
extern intptr_t  cont_BINDINGS;

/* An array to remember bindings for the variables. The array     */
/* is indexed by the variable index and holds the binding term.   */

typedef struct binding {
  SYMBOL         symbol;
  SYMBOL         renaming;
  TERM           term;
  struct binding *context;
  struct binding *link;
} *VARCONT, VARCONT_NODE;

extern VARCONT cont_LASTBINDING;     /* The last binding made. */
extern VARCONT cont_CURRENTBINDING;  /* Help variable. */

extern SYMBOL cont_INDEXVARSCANNER;

/* Two contexts are allocated by default */

extern VARCONT cont_LEFTVARCONT;
extern VARCONT cont_RIGHTVARCONT;
extern VARCONT cont_INSTANCEVARCONT; /* This context is used as a label only (dummy context) */


VARCONT cont_LeftContext(void);

VARCONT cont_RightContext(void);

VARCONT cont_InstanceContext(void);

/**************************************************************/
/* Binding Functions                                          */
/**************************************************************/

void    cont_BackTrackLastBindingHelp(void);
VARCONT cont_ContextOfBinding(VARCONT);
void    cont_CreateBindingHelp(VARCONT, SYMBOL, VARCONT, TERM);
void    cont_BindingOutput(VARCONT C, SYMBOL Var);

/**************************************************************/
/* A stack for the number of established bindings             */
/**************************************************************/

#define cont__STACKSIZE 5000

typedef int cont_STACK_TYPE[cont__STACKSIZE];

extern cont_STACK_TYPE cont_STACK;
extern int             cont_STACKPOINTER;

/* Stack operations */

 void cont_StackInit(void);

void cont_StackPush(int Entry);

 void cont_StackPop(void);

 int cont_StackPopResult(void);

 void cont_StackNPop(int N);

 intptr_t cont_StackTop(void);

 int cont_StackNthTop(int N);

 void cont_StackRplacTop(int Entry);

 void cont_StackRplacNthTop(int N, int Entry);

 void cont_StackRplacNth(int N, int Entry);

 intptr_t cont_StackBottom(void);

 void cont_StackSetBottom(int Pointer);

 BOOL cont_StackEmpty(int Pointer);

 void cont_StartBinding(void);

 int cont_BindingsSinceLastStart(void);

 void cont_StopAndStartBinding(void);

/**************************************************************/
/* Access                                                     */
/**************************************************************/

 VARCONT cont_Binding(VARCONT C, SYMBOL Var);

 VARCONT cont_BindingLink(VARCONT B);

 void cont_SetBindingLink(VARCONT B, VARCONT L);

 TERM cont_BindingTerm(VARCONT B);

 void cont_SetBindingTerm(VARCONT B, TERM T);

 SYMBOL cont_BindingSymbol(VARCONT B);

 void cont_SetBindingSymbol(VARCONT B, SYMBOL S);

 SYMBOL cont_BindingRenaming(VARCONT B);

 void cont_SetBindingRenaming(VARCONT B, SYMBOL S);

 VARCONT cont_BindingContext(VARCONT B);

 void cont_SetBindingContext(VARCONT B, VARCONT C);

 VARCONT cont_ContextBindingLink(VARCONT C,SYMBOL Var);

 TERM cont_ContextBindingTerm(VARCONT C,SYMBOL Var);

 void cont_SetContextBindingTerm(VARCONT C, SYMBOL Var, TERM t);

 SYMBOL cont_ContextBindingSymbol(VARCONT C,SYMBOL Var);

 SYMBOL cont_ContextBindingRenaming(VARCONT C,SYMBOL Var);

 void cont_SetContextBindingRenaming(VARCONT C, SYMBOL Var,
						       SYMBOL R);
 VARCONT cont_ContextBindingContext(VARCONT C,SYMBOL Var);

/**************************************************************/
/* Predicates                                                 */
/**************************************************************/

 BOOL cont_VarIsBound(VARCONT C, SYMBOL Var);

 BOOL cont_VarIsUsed(VARCONT C, SYMBOL Var);

 BOOL cont_VarIsLinked(VARCONT C, SYMBOL Var);

 BOOL cont_VarIsRenamed(VARCONT C, SYMBOL Var);

 BOOL cont_VarIsClosed(VARCONT C,SYMBOL Var);

 BOOL cont_BindingIsBound(VARCONT B);

 BOOL cont_BindingIsUsed(VARCONT B);

 BOOL cont_IsBoundToVar(VARCONT C);

/**************************************************************/
/* Aux functions for backtracking                             */
/**************************************************************/

 VARCONT cont_LastBinding(void);

 void cont_SetLastBinding(VARCONT B);

 TERM cont_LastBindingTerm(void);

 SYMBOL cont_LastBindingSymbol(void);

 VARCONT cont_LastBindingContext(void);

 BOOL cont_LastIsBound(void);

 BOOL cont_LastIsUsed(void);

 BOOL cont_LastIsClosed(void);

 BOOL cont_IsInContext(VARCONT C, SYMBOL Var, VARCONT B);

/**************************************************************/
/* Initialization                                             */
/**************************************************************/

void cont_InitBinding(VARCONT C, SYMBOL Var);

 void cont_InitContext(VARCONT C);

/**************************************************************/
/* Creation and deletion of contexts                          */
/**************************************************************/

 VARCONT cont_Create(void);
    
 void cont_Delete(VARCONT C);

 void cont_ResetIndexVarScanner(void);

/**************************************************************/
/* Output bindings                                            */
/**************************************************************/

 void cont_PrintCurrentTrail(void);

/**************************************************************/
/* Check Current Trail                                        */
/**************************************************************/

 BOOL cont_OnlyVariablesInCoDomOfCurrentTrail();

/**************************************************************/
/* Close bindings                                             */
/**************************************************************/

 void cont_CloseBindingHelp(VARCONT C, SYMBOL Var);

 void cont_CloseBindingBindingHelp(VARCONT B);

 void cont_CloseBinding(VARCONT C, SYMBOL Var);

 void cont_CloseBinding(VARCONT C, SYMBOL Var);

 void cont_CloseBindingBinding(VARCONT B); 

/**************************************************************/
/* Establish bindings                                         */
/**************************************************************/
 int cont_CreateBinding(VARCONT C, SYMBOL Var, VARCONT CTerm, TERM Term);

 int cont_CreateClosedBinding(VARCONT C, SYMBOL Var);

 int cont_CreateBinding(VARCONT C, SYMBOL Var, VARCONT CTerm, TERM Term);

 int cont_CreateClosedBinding(VARCONT C, SYMBOL Var);

/**************************************************************/
/* Backtracking                                               */
/**************************************************************/

 void cont_BackTrackLastBinding(void);

 int cont_BackTrack(void);

 int cont_StopAndBackTrack(void);

 int cont_BackTrackAndStart(void);

 void cont_Reset(void);

 void cont_BackTrackLastBinding(void);

 int cont_BackTrack(void);

 int cont_StopAndBackTrack(void);

 int cont_BackTrackAndStart(void);

 void cont_Reset(void);

/**************************************************************/
/* Check state of bindings                                    */
/**************************************************************/

#define cont__CHECKSTACKSIZE  1000
#define cont__CHECKSTACKEMPTY 0

typedef POINTER cont_CHECKSTACK_TYPE[cont__CHECKSTACKSIZE];

extern cont_CHECKSTACK_TYPE cont_CHECKSTACK;
extern int                  cont_CHECKSTACKPOINTER;

/* Stack operations */

 void cont_CheckStackInit(void);

 void cont_CheckStackPush(POINTER Entry);

 void cont_CheckStackPop(void);

 POINTER cont_CheckStackPopResult(void);

 void cont_CheckStackNPop(int N);

 POINTER cont_CheckStackTop(void);

 POINTER cont_CheckStackNthTop(int N);

 void cont_CheckStackRplacTop(POINTER Entry);

 void cont_CheckStackRplacNthTop(int N, POINTER Entry);

 void cont_CheckStackRplacNth(int N, POINTER Entry);

 int cont_CheckStackBottom(void);

 void cont_CheckStackSetBottom(int Pointer);

 BOOL cont_CheckStackEmpty(int Pointer);

extern VARCONT cont_STATELASTBINDING; /* Storage to save state of trails. */
extern int     cont_STATEBINDINGS;    /* Storage to save number of current bindings. */
extern int     cont_STATESTACK;       /* Storage to save state of stack. */
extern int     cont_STATETOPSTACK;    /* Storage to save state of the top element of the stack. */

 BOOL cont_CheckLastBinding(VARCONT Check, int Bindings);

 void cont_CheckState(void);

 void cont_SaveState(void);

 BOOL cont_IsContextEmpty(const VARCONT Check);

 SYMBOL cont_NextIndexVariable(const VARCONT IndexContext);

 TERM cont_SymbolDeref(VARCONT* Context, SYMBOL Symbol);


/**************************************************************/
/* Functions for Initialization and Controlling               */
/**************************************************************/
           
void cont_Init(void);
void cont_Check(void);
void cont_Free(void);

/**************************************************************/
/* Functions for Term Equality Test with respect to Bindings  */
/**************************************************************/

BOOL cont_TermEqual(VARCONT, VARCONT, TERM, VARCONT, VARCONT, TERM);
BOOL cont_TermEqualModuloBindings(VARCONT, VARCONT, TERM, VARCONT, TERM);

/**************************************************************/
/* Dereferencing of terms wrt. contexts                       */
/**************************************************************/

TERM cont_Deref(VARCONT, VARCONT*, TERM);

/**************************************************************/
/* Functions for Applying Bindings                            */
/**************************************************************/

TERM   cont_CopyAndApplyBindings(VARCONT, TERM);
TERM   cont_CopyAndApplyBindingsCom(const VARCONT, TERM);

TERM   cont_ApplyBindingsModuloMatching(const VARCONT, TERM, BOOL);
TERM   cont_ApplyBindingsModuloMatchingReverse(const VARCONT, TERM);

BOOL   cont_BindingsAreRenamingModuloMatching(const VARCONT);

/**************************************************************/
/* Misc Functions                                             */
/**************************************************************/

SYMBOL  cont_TermMaxVar(VARCONT, VARCONT, TERM);
NAT     cont_TermSize(VARCONT, VARCONT, TERM);
BOOL    cont_TermContainsSymbol(VARCONT, VARCONT, TERM, SYMBOL);

/**************************************************************/
/* Functions for Output                                       */
/**************************************************************/

void    cont_TermPrintPrefix(VARCONT, VARCONT, TERM);

#endif
