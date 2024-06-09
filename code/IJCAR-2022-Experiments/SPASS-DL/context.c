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
/* $Revision: 1.6 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-11-27 11:18:18 $                             * */
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


/* $RCSfile: context.c,v $ */

#include "context.h"
/*******************************************************
*Previously inlined functions                          *  
********************************************************/

VARCONT cont_LeftContext(void)
{
  return cont_LEFTVARCONT;
}

VARCONT cont_RightContext(void)
{
  return cont_RIGHTVARCONT;
}

VARCONT cont_InstanceContext(void)
{
  return cont_INSTANCEVARCONT;
}

 void cont_StackInit(void)
{
  cont_STACKPOINTER = 1;
}

 void cont_StackPush(int Entry)
{
#ifdef CHECK
  if (cont_STACKPOINTER >= cont__STACKSIZE) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_StackPush: Context stack overflow!\n");
    misc_FinishErrorReport();
  }
#endif
  
  cont_STACK[cont_STACKPOINTER++] = Entry;
}

 void cont_StackPop(void)
{
  --cont_STACKPOINTER;
}

 int cont_StackPopResult(void)
{
  return cont_STACK[--cont_STACKPOINTER];
}

 void cont_StackNPop(int N)
{
  cont_STACKPOINTER -= N;
}

 intptr_t cont_StackTop(void)
{
  return cont_STACK[cont_STACKPOINTER - 1];
}

 int cont_StackNthTop(int N)
{
  return cont_STACK[cont_STACKPOINTER - (1 + N)];
}

 void cont_StackRplacTop(int Entry)
{
  cont_STACK[cont_STACKPOINTER - 1] = Entry;
}

 void cont_StackRplacNthTop(int N, int Entry)
{
  cont_STACK[cont_STACKPOINTER - (1 + N)] = Entry;
}

 void cont_StackRplacNth(int N, int Entry)
{
  cont_STACK[N] = Entry;
}

 intptr_t cont_StackBottom(void)
{
  return cont_STACKPOINTER;
}

 void cont_StackSetBottom(int Pointer)
{
  cont_STACKPOINTER = Pointer;
}

 BOOL cont_StackEmpty(int Pointer)
{
  return cont_STACKPOINTER == Pointer;
}


 void cont_StartBinding(void)
{
  cont_StackPush(cont_BINDINGS);

  cont_BINDINGS = 0;
}

 int cont_BindingsSinceLastStart(void)
{
  return cont_BINDINGS;
}

 void cont_StopAndStartBinding(void)
{
  cont_StackRplacTop(cont_StackTop() + cont_BINDINGS);

  cont_BINDINGS = 0;
}

VARCONT cont_Binding(VARCONT C, SYMBOL Var)
{
  return &(C)[Var];
}

 VARCONT cont_BindingLink(VARCONT B)
{
  return B->link;
}

void cont_SetBindingLink(VARCONT B, VARCONT L)
{
  B->link = L;
}

 TERM cont_BindingTerm(VARCONT B)
{
  return B->term;
}

 void cont_SetBindingTerm(VARCONT B, TERM T)
{
  B->term = T;
}

SYMBOL cont_BindingSymbol(VARCONT B)
{
  return B->symbol;
}

 void cont_SetBindingSymbol(VARCONT B, SYMBOL S)
{
  B->symbol = S;
}

 SYMBOL cont_BindingRenaming(VARCONT B)
{
  return B->renaming;
}

 void cont_SetBindingRenaming(VARCONT B, SYMBOL S)
{
  B->renaming = S;
}

 VARCONT cont_BindingContext(VARCONT B)
{
  return B->context;
}

 void cont_SetBindingContext(VARCONT B, VARCONT C)
{
  B->context = C;
}

 VARCONT cont_ContextBindingLink(VARCONT C,SYMBOL Var)
{
  return C[Var].link;
}

 TERM cont_ContextBindingTerm(VARCONT C,SYMBOL Var)
{
  return C[Var].term;
}

 void cont_SetContextBindingTerm(VARCONT C, SYMBOL Var, TERM t)
{
  C[Var].term = t;
}

 SYMBOL cont_ContextBindingSymbol(VARCONT C,SYMBOL Var)
{
  return C[Var].symbol;
}

 SYMBOL cont_ContextBindingRenaming(VARCONT C,SYMBOL Var)
{
  return C[Var].renaming;
}

 void cont_SetContextBindingRenaming(VARCONT C, SYMBOL Var,
						       SYMBOL R)
{
  C[Var].renaming = R;
}

 VARCONT cont_ContextBindingContext(VARCONT C,SYMBOL Var)
{
  return C[Var].context;
}

/**************************************************************/
/* Predicates                                                 */
/**************************************************************/

 BOOL cont_VarIsBound(VARCONT C, SYMBOL Var)
{
  return cont_ContextBindingTerm(C,Var) != (TERM) NULL;
}

 BOOL cont_VarIsUsed(VARCONT C, SYMBOL Var)
{
  return cont_ContextBindingContext(C,Var) != (VARCONT) NULL;
}

 BOOL cont_VarIsLinked(VARCONT C, SYMBOL Var)
{
  return cont_ContextBindingLink(C,Var) != (VARCONT) NULL;
}

 BOOL cont_VarIsRenamed(VARCONT C, SYMBOL Var)
{
  return cont_ContextBindingRenaming(C, Var) != symbol_Null();
}

 BOOL cont_VarIsClosed(VARCONT C,SYMBOL Var)
{
  return !cont_VarIsBound(C,Var) && cont_VarIsUsed(C,Var);
}

 BOOL cont_BindingIsBound(VARCONT B)
{
  return cont_BindingTerm(B) != (TERM) NULL;
}

 BOOL cont_BindingIsUsed(VARCONT B)
{
  return cont_BindingContext(B) != (VARCONT) NULL;
}

 BOOL cont_IsBoundToVar(VARCONT C)
{
  return term_IsStandardVariable(cont_BindingTerm(C));
}

/**************************************************************/
/* Aux functions for backtracking                             */
/**************************************************************/

 VARCONT cont_LastBinding(void)
{
  return cont_LASTBINDING;
}

 void cont_SetLastBinding(VARCONT B)
{
  cont_LASTBINDING = B;
}

 TERM cont_LastBindingTerm(void)
{
  return cont_BindingTerm(cont_LastBinding());
}

 SYMBOL cont_LastBindingSymbol(void)
{
  return cont_BindingSymbol(cont_LastBinding());
}

 VARCONT cont_LastBindingContext(void)
{
  return cont_BindingContext(cont_LastBinding());
}

 BOOL cont_LastIsBound(void)
{
  return cont_BindingIsBound(cont_LastBinding());
}

 BOOL cont_LastIsUsed(void)
{
  return cont_LastBindingContext() != (VARCONT) NULL;
}

 BOOL cont_LastIsClosed(void)
{
  return !cont_LastIsBound() && cont_LastIsUsed();
}

 BOOL cont_IsInContext(VARCONT C, SYMBOL Var, VARCONT B)
{
  return cont_Binding(C, Var) == B;
}

/**************************************************************/
/* Initialization                                             */
/**************************************************************/

 void cont_InitContext(VARCONT C)
{
  int i;

  for (i = 0; i < cont__SIZE; i++)
    cont_InitBinding(C, i);
}

/**************************************************************/
/* Creation and deletion of contexts                          */
/**************************************************************/

VARCONT cont_Create(void)
{
  VARCONT Result;

  Result = (VARCONT)memory_Malloc(cont__SIZE*sizeof(VARCONT_NODE));

  cont_InitContext(Result);

  cont_LISTOFVARCONTS = list_Cons(Result, cont_LISTOFVARCONTS);
  cont_NOOFVARCONTS++;

  return Result;
}
    
void cont_Delete(VARCONT C)
{
#ifdef CHECK
  if ((cont_NOOFVARCONTS == 0) ||
      !list_PointerMember(cont_LISTOFVARCONTS, C)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_Delete: Context %ld not registered.\n",
		     (unsigned long)C);
    misc_FinishErrorReport();
  }
#endif

  cont_LISTOFVARCONTS = list_PointerDeleteOneElement(cont_LISTOFVARCONTS, C);

  cont_NOOFVARCONTS--;

  memory_Free(C, cont__SIZE*sizeof(VARCONT_NODE));
}

 void cont_ResetIndexVarScanner(void)
{
  cont_INDEXVARSCANNER = symbol_GetInitialIndexVarCounter();
}

/**************************************************************/
/* Output bindings                                            */
/**************************************************************/

void cont_PrintCurrentTrail(void)
{
  fputs("\nPrint bindings:", stdout);
  cont_CURRENTBINDING = cont_LastBinding();
  while (cont_CURRENTBINDING) {
    cont_BindingOutput(cont_ContextOfBinding(cont_CURRENTBINDING),
		       cont_BindingSymbol(cont_CURRENTBINDING));
    cont_CURRENTBINDING = cont_BindingLink(cont_CURRENTBINDING);
    if (cont_CURRENTBINDING)
      putchar('\n');
  }
  fflush(stdout);
}

/**************************************************************/
/* Check Current Trail                                        */
/**************************************************************/

BOOL cont_OnlyVariablesInCoDomOfCurrentTrail()
{
  cont_CURRENTBINDING = cont_LastBinding();

 while(cont_CURRENTBINDING){
#ifdef CHECK
   if(cont_BindingSymbol(cont_CURRENTBINDING) != cont_ContextBindingSymbol(cont_ContextOfBinding(cont_CURRENTBINDING),
                                                                           cont_BindingSymbol(cont_CURRENTBINDING)))
     {
       misc_StartErrorReport();
       misc_ErrorReport("\n In cont_OnlyVariablesInCoDomOfCurrentTrail: Wrong Trail.\n");
       misc_FinishErrorReport();

     }
#endif
   if(symbol_IsStandardVariable(cont_BindingSymbol(cont_CURRENTBINDING)) &&
       !cont_IsBoundToVar(cont_CURRENTBINDING))
   {
     return FALSE;
   }
   cont_CURRENTBINDING = cont_BindingLink(cont_CURRENTBINDING);
 }
 return TRUE;
}

/**************************************************************/
/* Close bindings                                             */
/**************************************************************/

void cont_CloseBindingHelp(VARCONT C, SYMBOL Var)
{
  cont_SetContextBindingTerm(C, Var, NULL);
}

void cont_CloseBindingBindingHelp(VARCONT B)
{
  cont_SetBindingTerm(B, NULL);
}

#if SHOWBINDINGS
void cont_CloseBinding(VARCONT C, SYMBOL Var)
{
  fputs("\nClose binding from ", stdout);
  cont_BindingOutput(C, Var);
  cont_CloseBindingHelp(C, Var);
}
#else
 void cont_CloseBinding(VARCONT C, SYMBOL Var)
{
  cont_CloseBindingHelp(C, Var);
}
#endif

 void cont_CloseBindingBinding(VARCONT B) {
  cont_CloseBindingBindingHelp(B);
}

/**************************************************************/
/* Establish bindings                                         */
/**************************************************************/

#if SHOWBINDINGS

 int cont_CreateBinding(VARCONT C, SYMBOL Var, VARCONT CTerm, TERM Term)
{
  cont_CreateBindingHelp(C,Var,CTerm,Term);
  fputs("\nEstablish binding from ", stdout);
  cont_BindingOutput(C, Var);
  return ++cont_BINDINGS;
}

 int cont_CreateClosedBinding(VARCONT C, SYMBOL Var)
{
  cont_CreateBindingHelp(C, Var, C, NULL);
  fputs("\nEstablish closed binding from ", stdout);
  cont_BindingOutput(C,Var);
  return ++cont_BINDINGS;
}

#else

 int cont_CreateBinding(VARCONT C, SYMBOL Var, VARCONT CTerm, TERM Term)
{
  cont_CreateBindingHelp(C,Var,CTerm,Term);
  return ++cont_BINDINGS;
}

 int cont_CreateClosedBinding(VARCONT C, SYMBOL Var)
{
  cont_CreateBindingHelp(C, Var, C, NULL);
  return ++cont_BINDINGS;
}

#endif

/**************************************************************/
/* Backtracking                                               */
/**************************************************************/

#if SHOWBINDINGS

void cont_BackTrackLastBinding(void)
{
  VARCONT LastContext;
  SYMBOL  LastSymbol;

  LastContext = cont_ContextOfBinding(cont_LastBinding());
  LastSymbol  = cont_LastBindingSymbol();
  fputs("\nBacktrack binding from ", stdout);
  cont_BindingOutput(LastContext, LastSymbol);
  cont_BackTrackLastBindingHelp();
}

int cont_BackTrack(void)
{
  printf("\nBacktrack %d bindings:", cont_BINDINGS);

  while (cont_BINDINGS > 0)
    cont_BackTrackLastBinding();

  if (!cont_StackEmpty(0))
    cont_BINDINGS = cont_StackPopResult();

  fflush(stdout);
  return 0;
}

int cont_StopAndBackTrack(void)
{
#ifdef CHECK
  if (cont_BINDINGS > 0) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_StopAndBackTrack: Bindings not reset!\n");
    misc_FinishErrorReport();
  } else if (cont_StackEmpty(0)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_StopAndBackTrack: No bindings on stack!\n");
    misc_FinishErrorReport();
  }
#endif
  cont_BINDINGS = cont_StackPopResult();

  printf("\nStop and Backtrack %d bindings:", cont_BINDINGS);

  while (cont_BINDINGS > 0)
    cont_BackTrackLastBinding();

  fflush(stdout);
  return 0;
}

int cont_BackTrackAndStart(void)
{
  printf("\nBacktrack %d bindings:", cont_BINDINGS);

  while (cont_BINDINGS > 0)
    cont_BackTrackLastBinding();

  fflush(stdout);
  return 0;
}

void cont_Reset(void)
{
  fputs("\nReset bindings:", stdout);
  while (cont_LastBinding())
    cont_BackTrackLastBinding();

  cont_BINDINGS = 0;
  cont_StackInit();
  cont_ResetIndexVarScanner();
  fflush(stdout);
}

#else

void cont_BackTrackLastBinding(void)
{
  cont_BackTrackLastBindingHelp();
}

int cont_BackTrack(void)
{
  while (cont_BINDINGS > 0)
    cont_BackTrackLastBinding();

  if (!cont_StackEmpty(0))
    cont_BINDINGS = cont_StackPopResult();

  return 0;
}

int cont_StopAndBackTrack(void)
{
#ifdef CHECK
  if (cont_BINDINGS > 0) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_StopAndBackTrack: Bindings not reset!\n");
    misc_FinishErrorReport();
  } else if (cont_StackEmpty(0)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_StopAndBackTrack: No bindings on stack!\n");
    misc_FinishErrorReport();
  }
#endif
  cont_BINDINGS = cont_StackPopResult();

  while (cont_BINDINGS > 0)
    cont_BackTrackLastBinding();

  return 0;
}

int cont_BackTrackAndStart(void)
{
  while (cont_BINDINGS > 0)
    cont_BackTrackLastBinding();

  return 0;
}

void cont_Reset(void)
{
  while (cont_LastBinding())
    cont_BackTrackLastBinding();

  cont_BINDINGS = 0;
  cont_StackInit();
  cont_ResetIndexVarScanner();
}

#endif

/* Stack operations */

void cont_CheckStackInit(void)
{
  cont_CHECKSTACKPOINTER = cont__CHECKSTACKEMPTY;
}

 void cont_CheckStackPush(POINTER Entry)
{
#ifdef CHECK
  if (cont_CHECKSTACKPOINTER >= cont__STACKSIZE) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_CheckStackPush: Context check stack overflow!\n");
    misc_FinishErrorReport();
  }
#endif
  
  cont_CHECKSTACK[cont_CHECKSTACKPOINTER++] = Entry;
}

void cont_CheckStackPop(void)
{
  --cont_CHECKSTACKPOINTER;
}

 POINTER cont_CheckStackPopResult(void)
{
  return cont_CHECKSTACK[--cont_CHECKSTACKPOINTER];
}

 void cont_CheckStackNPop(int N)
{
  cont_CHECKSTACKPOINTER -= N;
}

POINTER cont_CheckStackTop(void)
{
  return cont_CHECKSTACK[cont_CHECKSTACKPOINTER - 1];
}

 POINTER cont_CheckStackNthTop(int N)
{
  return cont_CHECKSTACK[cont_CHECKSTACKPOINTER - (1 + N)];
}

 void cont_CheckStackRplacTop(POINTER Entry)
{
  cont_CHECKSTACK[cont_CHECKSTACKPOINTER - 1] = Entry;
}

 void cont_CheckStackRplacNthTop(int N, POINTER Entry)
{
  cont_CHECKSTACK[cont_CHECKSTACKPOINTER - (1 + N)] = Entry;
}

 void cont_CheckStackRplacNth(int N, POINTER Entry)
{
  cont_CHECKSTACK[N] = Entry;
}

 int cont_CheckStackBottom(void)
{
  return cont_CHECKSTACKPOINTER;
}

 void cont_CheckStackSetBottom(int Pointer)
{
  cont_CHECKSTACKPOINTER = Pointer;
}

BOOL cont_CheckStackEmpty(int Pointer)
{
  return cont_CHECKSTACKPOINTER == Pointer;
}

extern VARCONT cont_STATELASTBINDING; /* Storage to save state of trails. */
extern int     cont_STATEBINDINGS;    /* Storage to save number of current bindings. */
extern int     cont_STATESTACK;       /* Storage to save state of stack. */
extern int     cont_STATETOPSTACK;    /* Storage to save state of the top element of the stack. */

BOOL cont_CheckLastBinding(VARCONT Check, int Bindings)
{
  VARCONT Scan;
  BOOL    Result;

  Scan = cont_LastBinding();

  while (Bindings > 0) {
    Scan = cont_BindingLink(Scan);
    Bindings--;
  }
  if (Check == Scan)
    Result = TRUE;
  else
    Result = FALSE;

  return Result;
}

void cont_CheckState(void)
{
  if (cont_CheckStackEmpty(cont__CHECKSTACKEMPTY)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_CheckState: No states saved.\n");
    misc_FinishErrorReport();
  }

  cont_STATETOPSTACK    = (intptr_t)cont_CheckStackPopResult();
  cont_STATESTACK       = (intptr_t)cont_CheckStackPopResult();
  cont_STATEBINDINGS    = (intptr_t)cont_CheckStackPopResult();
  cont_STATELASTBINDING = (VARCONT)cont_CheckStackPopResult();

  if ((cont_STATELASTBINDING != cont_LastBinding()) ||
      (cont_STATEBINDINGS != cont_BINDINGS) ||
      (!cont_StackEmpty(cont_STATESTACK)) ||
      (cont_STATETOPSTACK != cont_StackTop())) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_CheckState: State of contexts does not match saved state.");
    misc_ErrorReport("\nTrail: Saved state: %ld; current state: %ld.",
		     (long)cont_STATELASTBINDING, (long)cont_LastBinding());
    misc_ErrorReport("\nNumber of bindings: Saved state: %d; current state: %d.",
		     cont_STATEBINDINGS, cont_BINDINGS);
    misc_ErrorReport("\nBinding stack pointer: Saved state: %d; current state: %d.",
		     cont_STATESTACK, cont_StackBottom());
    misc_ErrorReport("\nNumber of bindings on top of stack: Saved state: %d; current state: %d.\n\n",
		     cont_STATETOPSTACK, cont_StackTop());
    misc_FinishErrorReport();
  }
}

 void cont_SaveState(void)
{
  cont_CheckStackPush((POINTER)cont_LastBinding());
  cont_CheckStackPush((POINTER)cont_BINDINGS);
  cont_CheckStackPush((POINTER)cont_StackBottom());
  cont_CheckStackPush((POINTER)cont_StackTop());
}

 BOOL cont_IsContextEmpty(const VARCONT Check)
{
  int i;

  for (i = 0; i < cont__SIZE; i++)
    if (cont_VarIsBound(Check, i) ||
	cont_VarIsUsed(Check, i) ||
	cont_VarIsLinked(Check, i) ||
	cont_VarIsRenamed(Check, i))
      return FALSE;

  return TRUE;
}

/**************************************************************/
/* Generation of index variables                              */
/**************************************************************/

SYMBOL cont_NextIndexVariable(const VARCONT IndexContext)
{
  if (symbol_Equal(cont_INDEXVARSCANNER, symbol_LastIndexVariable()))
    cont_INDEXVARSCANNER = symbol_CreateIndexVariable();
  else
    for (;;) {
      cont_INDEXVARSCANNER = symbol_NextIndexVariable(cont_INDEXVARSCANNER);
      if (!cont_VarIsUsed(IndexContext, cont_INDEXVARSCANNER)) 
	break;
      else 
	if (symbol_Equal(cont_INDEXVARSCANNER, symbol_LastIndexVariable())) {
	  cont_INDEXVARSCANNER = symbol_CreateIndexVariable();
	  break;
	}
    }
  return cont_INDEXVARSCANNER;
}

/**************************************************************/
/* Dereferencing of terms wrt. contexts                       */
/**************************************************************/

 TERM cont_SymbolDeref(VARCONT* Context, SYMBOL Symbol)
/**************************************************************
  INPUT:   A call-by-ref context and a variable symbol.
  RETURNS: The dereferenced term and the corresponding context,
           NULL if the symbol is not bound
  SUMMARY: Dereferences bindings of variables.
  CAUTION: In general, the context of the returned term
           is different to the input context.
***************************************************************/
{
  TERM Term;

  Term = (TERM)NULL;

  while (symbol_IsVariable(Symbol) && *Context != cont_InstanceContext()) {

    if (cont_VarIsBound(*Context, Symbol)) {
      VARCONT HelpContext;

      HelpContext = cont_ContextBindingContext(*Context, Symbol);
      Term        = cont_ContextBindingTerm(*Context, Symbol);
      *Context    = HelpContext;
      Symbol      = term_TopSymbol(Term);
    } 
    else
      return Term;
  }
  return Term;
}

/**************************************************************/
/* Global Variables                                           */
/**************************************************************/

int     cont_NOOFVARCONTS;
LIST    cont_LISTOFVARCONTS;
intptr_t     cont_BINDINGS;

SYMBOL  cont_INDEXVARSCANNER;

VARCONT cont_LASTBINDING;     /* The last binding made. */
VARCONT cont_CURRENTBINDING;  /* Help variable. */

VARCONT cont_LEFTVARCONT;
VARCONT cont_RIGHTVARCONT;
VARCONT cont_INSTANCEVARCONT;

cont_STACK_TYPE cont_STACK;
int             cont_STACKPOINTER;

cont_CHECKSTACK_TYPE cont_CHECKSTACK;
int                  cont_CHECKSTACKPOINTER;

VARCONT cont_STATELASTBINDING; /* Storage to save state of trails. */
int     cont_STATEBINDINGS;    /* Storage to save number of current bindings. */
int     cont_STATESTACK;       /* Storage to save state of stack. */
int     cont_STATETOPSTACK;    /* Storage to save state of the top element of the stack. */


/**************************************************************/
/* ********************************************************** */
/* *							    * */
/* *  INITIALIZATION           			            * */
/* *							    * */
/* ********************************************************** */
/**************************************************************/


void cont_Init(void)
/**********************************************************
  INPUT:   None.
  RETURNS: None.
  EFFECT:  Initializes the unify module.
********************************************************/
{
  cont_LASTBINDING     = (VARCONT)NULL;

  cont_ResetIndexVarScanner();

  cont_NOOFVARCONTS    = 0;
  cont_LISTOFVARCONTS  = list_Nil();
  cont_BINDINGS        = 0;

  cont_INSTANCEVARCONT = (VARCONT)memory_Malloc(sizeof(VARCONT_NODE));

  cont_LEFTVARCONT     = cont_Create();
  cont_RIGHTVARCONT    = cont_Create();

  cont_StackInit();
  cont_StackPush(0);
  cont_StackPop();
}


void cont_Check(void)
/**********************************************************
  INPUT:   None.
  RETURNS: None.
  EFFECT:  Frees internal structures of the unify module.
********************************************************/
{
#ifdef CHECK
  if (cont_LASTBINDING || (cont_BINDINGS != 0) ||
      !symbol_Equal(cont_INDEXVARSCANNER,
		    symbol_GetInitialIndexVarCounter())) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_Check: There are variable bindings not reset.\n");
    misc_FinishErrorReport();
  }
#endif
}


void cont_Free(void)
/**********************************************************
  INPUT:   None.
  RETURNS: None.
  EFFECT:  Frees internal structures of the unify module.
********************************************************/
{
  cont_Check();

  while (cont_NOOFVARCONTS > 0)
    cont_Delete(list_Car(cont_LISTOFVARCONTS)); /* Decreases NOOFVARCONTS */

  cont_BINDINGS = 0;

  memory_Free(cont_INSTANCEVARCONT, sizeof(VARCONT_NODE));
}

void cont_InitBinding(VARCONT C, SYMBOL Var)
/**********************************************************
  INPUT:   A context and a variable.
  RETURNS: Nothing.
  EFFECT:  Initializes a binding of variable in the 
           given context.
********************************************************/
{
  cont_CURRENTBINDING = cont_Binding(C, Var);
  cont_SetBindingLink(cont_CURRENTBINDING, (VARCONT)NULL);
  cont_SetBindingTerm(cont_CURRENTBINDING, (TERM)NULL);
  cont_SetBindingSymbol(cont_CURRENTBINDING, Var);
  cont_SetBindingRenaming(cont_CURRENTBINDING, symbol_Null());
  cont_SetBindingContext(cont_CURRENTBINDING, (VARCONT)NULL);
}



/**************************************************************/
/* ********************************************************** */
/* *							    * */
/* *  TERM EQUALITY WITH RESPECT TO BOUND VARIABLES         * */
/* *							    * */
/* ********************************************************** */
/**************************************************************/


BOOL cont_TermEqual(VARCONT GlobalContext1, VARCONT TermContext1, TERM Term1, 
                    VARCONT GlobalContext2, VARCONT TermContext2, TERM Term2)
/*********************************************************
  INPUT:   Two terms and two local contexts for the terms and
           two global contexts
  RETURNS: TRUE iff the two terms are equal, where
           variables are interpreted with respect to
	   the bindings in the contexts.
  CAUTION: Variables of <Term1> and <Term2> are bound in 
           <TermContext1> and <TermContext2> respectively and
           the index variables are bound in <GlobalContext1>
           and <GlobalContext2> respectively.
********************************************************/
{
#ifdef CHECK
  if (!(term_IsTerm(Term1) && term_IsTerm(Term2))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_TermEqual: Input terms are corrupted.\n");
    misc_FinishErrorReport();
  }
#endif

  Term1 = cont_Deref(GlobalContext1,&TermContext1,Term1);
  Term2 = cont_Deref(GlobalContext2,&TermContext2,Term2);

  if (!term_EqualTopSymbols(Term1, Term2))
    return FALSE;
  else if (term_ArgumentList(Term1)) {
    LIST Scan1, Scan2;
    for (Scan1=term_ArgumentList(Term1), Scan2=term_ArgumentList(Term2);
	 list_Exist(Scan1) && list_Exist(Scan2);
	 Scan1=list_Cdr(Scan1), Scan2=list_Cdr(Scan2))
      if (!cont_TermEqual(GlobalContext1, TermContext1,list_Car(Scan1), 
                            GlobalContext2, TermContext2,list_Car(Scan2)))
	return FALSE;
    return (list_Empty(Scan1) ? list_Empty(Scan2) : FALSE);
  } else
    return TRUE;
}


BOOL cont_TermEqualModuloBindings(VARCONT IndexContext, VARCONT CtL, TERM TermL,
				  VARCONT CtR, TERM TermR)
/*********************************************************
  INPUT:   Two contexts, two terms.
  RETURNS: The boolean value TRUE if the terms are equal.
  CAUTION: EQUAL FUNCTION- OR PREDICATE SYMBOLS SHARE THE
           SAME ARITY. THIS IS NOT VALID FOR JUNCTORS!
*******************************************************/
{   
#ifdef CHECK
  if (!(term_IsTerm(TermL) && term_IsTerm(TermR))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_TermEqualModuloBindings: Input terms are corrupted.\n");
    misc_FinishErrorReport();
  }
#endif

  while (term_IsVariable(TermL)) {
    SYMBOL TermTop;

    TermTop = term_TopSymbol(TermL);

    if (symbol_IsIndexVariable(TermTop))
      CtL = IndexContext;
    else if (CtL == cont_InstanceContext())
      break;

    if (cont_VarIsBound(CtL, TermTop)) {
      VARCONT CHelp;

      CHelp = cont_ContextBindingContext(CtL, TermTop);
      TermL = cont_ContextBindingTerm(CtL, TermTop);
      CtL   = CHelp;
    } else
      break;
  }

  while (term_IsVariable(TermR)) {
    SYMBOL TermTop;

    TermTop = term_TopSymbol(TermR);

    if (symbol_IsIndexVariable(TermTop))
      CtR = IndexContext;
    else if (CtR == cont_InstanceContext())
      break;

    if (cont_VarIsBound(CtR, TermTop)) {
      VARCONT CHelp;

      CHelp = cont_ContextBindingContext(CtR, TermTop);
      TermR = cont_ContextBindingTerm(CtR, TermTop);
      CtR   = CHelp;
    } else
      break;
  }

  if (!term_EqualTopSymbols(TermL, TermR))
    return FALSE;
  else 
    if (term_IsVariable(TermL)) {
      if (CtL == CtR)
	return TRUE;
      else
	return FALSE;
    }
    else 
      if (term_IsComplex(TermL)) {
	LIST ScanL, ScanR;
	
	for (ScanL=term_ArgumentList(TermL), ScanR=term_ArgumentList(TermR);
	     list_Exist(ScanL) && list_Exist(ScanR);
	     ScanL=list_Cdr(ScanL), ScanR=list_Cdr(ScanR))
	  if (!cont_TermEqualModuloBindings(IndexContext, CtL, list_Car(ScanL),
					    CtR, list_Car(ScanR)))
	    return FALSE;
	
	return (list_Empty(ScanL) ? list_Empty(ScanR) : FALSE);
	
      } 
      else
	return TRUE;
}


/**************************************************************/
/* ********************************************************** */
/* *							    * */
/* *  APPLY BINDINGS                                        * */
/* *							    * */
/* ********************************************************** */
/**************************************************************/


TERM cont_CopyAndApplyBindings(VARCONT TermContext, TERM Term)
{
  while (term_IsVariable(Term)) {
    SYMBOL TermTop;

    TermTop = term_TopSymbol(Term);

    if (cont_VarIsBound(TermContext, TermTop)) {
      VARCONT HelpContext;

      HelpContext = cont_ContextBindingContext(TermContext, TermTop);
      Term        = cont_ContextBindingTerm(TermContext, TermTop);
      TermContext = HelpContext;
    } else
      break;
  }

  if (term_IsComplex(Term)) {
    LIST Scan, ArgumentList;
    for (Scan = ArgumentList = list_Copy(term_ArgumentList(Term));
	 !list_Empty(Scan);
	 Scan = list_Cdr(Scan))
      list_Rplaca(Scan, cont_CopyAndApplyBindings(TermContext, list_Car(Scan)));
    return term_Create(term_TopSymbol(Term), ArgumentList);
  } else 
    return term_Create(term_TopSymbol(Term), list_Nil());
}

TERM cont_SymbolApplyBindings(VARCONT TermContext, SYMBOL Symbol)
/**************************************************************
  INPUT:   A call-by-ref context and a variable symbol.
  RETURNS: The recursively dereferenced term and the corresponding context,
           NULL if the symbol is not bound
  SUMMARY: Dereferences bindings of variables.
  CAUTION: In general, the context of the returned term
           is different to the input context.
***************************************************************/
{
  TERM Term;

  Term = (TERM)NULL;

  while (symbol_IsVariable(Symbol)) {

    if (cont_VarIsBound(TermContext, Symbol)) {
      VARCONT HelpContext;

      HelpContext = cont_ContextBindingContext(TermContext, Symbol);
      Term        = cont_ContextBindingTerm(TermContext, Symbol);
      TermContext = HelpContext;
      Symbol      = term_TopSymbol(Term);
    } else
      break;
  }

  if (Term != (TERM)NULL && term_IsComplex(Term)) {
    LIST Scan, ArgumentList;
    for (Scan = ArgumentList = list_Copy(term_ArgumentList(Term));
	 !list_Empty(Scan);
	 Scan = list_Cdr(Scan))
      list_Rplaca(Scan, cont_CopyAndApplyBindings(TermContext, list_Car(Scan)));
    return term_Create(term_TopSymbol(Term), ArgumentList);
  }
  
  return Term;
}


TERM cont_CopyAndApplyBindingsCom(const VARCONT Context, TERM Term)
{
  while (term_IsVariable(Term) && cont_VarIsBound(Context, term_TopSymbol(Term)))
    Term = cont_ContextBindingTerm(Context, term_TopSymbol(Term));

  if (term_IsComplex(Term)) {
    LIST Scan, ArgumentList;
    for (Scan = ArgumentList = list_Copy(term_ArgumentList(Term));
	 !list_Empty(Scan);
	 Scan = list_Cdr(Scan))
      list_Rplaca(Scan, cont_CopyAndApplyBindingsCom(Context, list_Car(Scan)));
    return term_Create(term_TopSymbol(Term), ArgumentList);
  } else 
    return term_Create(term_TopSymbol(Term), list_Nil());
}


TERM cont_ApplyBindingsModuloMatching(const VARCONT Context, TERM Term,
				      BOOL VarCheck)
/**********************************************************
  INPUT:   A context, a term, and a boolean flag.
  RETURNS: <Term> is destructively changed with respect to
           established bindings in the context.
	   If <VarCheck> is true, all variables in <Term>
	   must be bound in the context. When compiled with
	   "CHECK" on, this condition is in fact checked.
	   This function only makes sense after a matching operation.
***********************************************************/
{
  TERM   RplacTerm;
  LIST   Arglist;
  SYMBOL Top;

#ifdef CHECK
  if (VarCheck && symbol_IsVariable(term_TopSymbol(Term)) &&
      !cont_VarIsBound(Context, term_TopSymbol(Term))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_ApplyBindingsModuloMatching:");
    misc_ErrorReport(" Used in forbidden context.\n");
    misc_FinishErrorReport();
  }
#endif

  Top = term_TopSymbol(Term);

  if (symbol_IsVariable(Top)) {
    
    if (cont_VarIsBound(Context, Top)) {
      RplacTerm = cont_ContextBindingTerm(Context, Top);
      Arglist   = term_CopyTermList(term_ArgumentList(RplacTerm));
      term_RplacTop(Term, term_TopSymbol(RplacTerm));
      term_DeleteTermList(term_ArgumentList(Term));
      term_RplacArgumentList(Term, Arglist); 
    }
  }
  else {
    
    for (Arglist = term_ArgumentList(Term);
	 !list_Empty(Arglist);
	 Arglist = list_Cdr(Arglist))
      cont_ApplyBindingsModuloMatching(Context, list_Car(Arglist), VarCheck);
  }     

  return Term;
}


static TERM cont_CopyAndApplyIndexVariableBindings(const VARCONT Context, TERM Term)
{
  SYMBOL TermTop;

#ifdef CHECK
  if (symbol_IsIndexVariable(term_TopSymbol(Term)) &&
      !cont_VarIsBound(Context, term_TopSymbol(Term))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_CopyAndApplyIndexVariableBindings:");
    misc_ErrorReport(" Expected bound index variable.");
    misc_FinishErrorReport();
  }
#endif

  TermTop = term_TopSymbol(Term);

  while (symbol_IsIndexVariable(TermTop)) {
    if (cont_VarIsBound(Context, TermTop)) {
      Term    = cont_ContextBindingTerm(Context, TermTop);
      TermTop = term_TopSymbol(Term);
    }
  }

  if (term_IsComplex(Term)) {
    LIST Scan, ArgumentList;
    for (Scan = ArgumentList = list_Copy(term_ArgumentList(Term));
	 !list_Empty(Scan);
	 Scan = list_Cdr(Scan))
      list_Rplaca(Scan, cont_CopyAndApplyIndexVariableBindings(Context, list_Car(Scan)));
    return term_Create(TermTop, ArgumentList);
  } else 
    return term_Create(TermTop, list_Nil());
}


TERM cont_ApplyBindingsModuloMatchingReverse(const VARCONT Context, TERM Term)
/**********************************************************
  INPUT:   A term.
  RETURNS: <Term> is destructively changed with respect to
           established bindings in the leftmost context. This
           function only make sense after a matching operation (reverse).
***********************************************************/
{
  TERM   RplacTerm;
  LIST   Arglist;
  SYMBOL Top;

#ifdef CHECK
  if (symbol_IsVariable(term_TopSymbol(Term)) &&
      !cont_VarIsBound(Context, term_TopSymbol(Term))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_ApplyBindingsModuloMatchingReverse:");
    misc_ErrorReport(" Used in forbidden context.\n");
    misc_FinishErrorReport();
  }
#endif
    
  Top = term_TopSymbol(Term);

  if (symbol_IsVariable(Top)) {
    
    if (cont_VarIsBound(Context, Top)) {
      RplacTerm =
	cont_CopyAndApplyIndexVariableBindings(Context,
						  cont_ContextBindingTerm(Context, Top));
      term_RplacTop(Term, term_TopSymbol(RplacTerm));
      term_DeleteTermList(term_ArgumentList(Term));
      term_RplacArgumentList(Term, term_ArgumentList(RplacTerm));
      term_Free(RplacTerm);
    }
  }
  else {
    
    for (Arglist = term_ArgumentList(Term); !list_Empty(Arglist);
	 Arglist = list_Cdr(Arglist))
      cont_ApplyBindingsModuloMatchingReverse(Context, list_Car(Arglist));
  }     

  return Term;
}


BOOL cont_BindingsAreRenamingModuloMatching(const VARCONT RenamingContext)
{
  VARCONT Context;

#ifdef CHECK
  if (!cont_IsContextEmpty(RenamingContext)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_BindingsAreRenamingModuloMatching:");
    misc_ErrorReport(" Renaming context contains bindings.\n");
    misc_FinishErrorReport();
  }
#endif

  cont_StartBinding();

  Context = cont_LastBinding();

  while (Context) {
    
    if (!symbol_IsIndexVariable(cont_BindingSymbol(Context))) {
      SYMBOL CodomainSymbol;

      CodomainSymbol = term_TopSymbol(cont_BindingTerm(Context));

      if (symbol_IsVariable(CodomainSymbol)) {
	if (cont_VarIsRenamed(RenamingContext, CodomainSymbol)) {
	  cont_BackTrack();
	  return FALSE;
	} else {
	  cont_CreateBinding(RenamingContext, CodomainSymbol, NULL, NULL);
	  cont_SetContextBindingRenaming(RenamingContext, CodomainSymbol, CodomainSymbol);
	}
      } else {
	cont_BackTrack();
	return FALSE;
      }
    }

    Context = cont_BindingLink(Context);
  }

  cont_BackTrack();
  return TRUE;
}


/**************************************************************/
/* ********************************************************** */
/* *							    * */
/* *  MISC FUNCTIONS                                        * */
/* *							    * */
/* ********************************************************** */
/**************************************************************/

void cont_CreateBindingHelp(VARCONT C, SYMBOL Var, VARCONT CTerm, TERM Term)
{
#ifdef CHECK
  if (cont_VarIsBound(C, Var)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_CreateBindingHelp: Variable already bound.\n");
    misc_FinishErrorReport();
  }
#endif

  cont_CURRENTBINDING = cont_Binding(C,Var);
  cont_SetBindingTerm(cont_CURRENTBINDING, Term);
  cont_SetBindingContext(cont_CURRENTBINDING, CTerm);
  cont_SetBindingLink(cont_CURRENTBINDING, cont_LastBinding());
  cont_SetLastBinding(cont_CURRENTBINDING);
}

VARCONT cont_ContextOfBinding(VARCONT B)
{
  VARCONT Result;
  LIST    Scan;

  for (Result = NULL, Scan = cont_LISTOFVARCONTS;
       list_Exist(Scan);
       Scan = list_Cdr(Scan)) {
    if (cont_IsInContext(list_Car(Scan), cont_BindingSymbol(B), B)) {
      Result = list_Car(Scan);
      break;
    }
  }

#ifdef CHECK
  if (Result == NULL) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_ContextOfBinding: Unknown context.\n");
    misc_FinishErrorReport();
  }
#endif

  return Result;
}

void cont_BindingOutput(VARCONT C, SYMBOL Var)
{
  symbol_Print(cont_ContextBindingSymbol(C, Var));
  putchar(':');
  symbol_Print(Var);

  fputs(" -> ", stdout);

  if (cont_VarIsBound(C, Var)) {
    term_PrintPrefix(cont_ContextBindingTerm(C, Var));
  } else
    fputs("unbound", stdout);

  fputs(" in ", stdout);

  if (cont_VarIsUsed(C, Var)) {
    printf("%ld", (unsigned long)cont_ContextBindingContext(C, Var));
  } else
    fputs("NULL (unused)", stdout);

  fputs(". ", stdout);

  if (cont_VarIsClosed(C, Var)) {
    fputs("(closed)", stdout);
  }

  if (!cont_VarIsBound(C, Var) &&
      !cont_VarIsUsed(C, Var)) {
    fputs(",(free)", stdout);
  }

  if (cont_VarIsRenamed(C, Var)) {
    fputs(",(renamed): ", stdout);
    symbol_Print(Var);
    fputs(" -> ", stdout);
    symbol_Print(cont_ContextBindingRenaming(C, Var));
  } 
  
  fflush(stdout);
}

void cont_BackTrackLastBindingHelp(void)
{
  cont_CURRENTBINDING = cont_LastBinding();
  cont_SetLastBinding(cont_BindingLink(cont_CURRENTBINDING));
  cont_SetBindingTerm(cont_CURRENTBINDING, NULL);
  cont_SetBindingContext(cont_CURRENTBINDING, NULL);
  cont_SetBindingRenaming(cont_CURRENTBINDING, symbol_Null());
  cont_SetBindingLink(cont_CURRENTBINDING, NULL);

  cont_BINDINGS--;
}

SYMBOL cont_TermMaxVar(VARCONT GlobalContext, VARCONT TermContext, TERM Term)
/*********************************************************
  INPUT:   A global Context where Index variables are bound 
           a context and a term
  RETURNS: The maximal variable in <Term> with respect to
           the bindings in <Context>
  CAUTION: Variables of <Term1> are bound in 
           <TermContext1>  and
           the index variables are bound in <GlobalContext1>
           
********************************************************/
{
  LIST   scan;
  SYMBOL result;

#ifdef CHECK
  if (!term_IsTerm(Term)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In cont_TermMaxVar: Input term is corrupted.\n");
    misc_FinishErrorReport();
  }
#endif

  Term   = cont_Deref(GlobalContext,&TermContext,Term);
  result = symbol_Null();

  if (term_IsStandardVariable(Term)) {
    if (term_TopSymbol(Term) > result)
      result = term_TopSymbol(Term);
  } else {
    for (scan = term_ArgumentList(Term); !list_Empty(scan); scan = list_Cdr(scan)) {
      SYMBOL max = cont_TermMaxVar(GlobalContext, TermContext, list_Car(scan));
      
      if (max > result)
	result = max;
    }
  }

  return result;
}


NAT cont_TermSize(VARCONT GlobalContext, VARCONT TermContext, TERM Term)
/*********************************************************
  INPUT:   A global context where index variables are bound
           a context and a term.
  RETURNS: The number of symbols in <Term> with respect to
           the bindings in <Context>
  CAUTION: Variables of <Term1> are bound in 
           <TermContext1>  and
           the index variables are bound in <GlobalContext1>
********************************************************/
{
  NAT  result;
  LIST scan;

  Term = cont_Deref(GlobalContext, &TermContext, Term);
  result = 1;
  for (scan = term_ArgumentList(Term); !list_Empty(scan); scan = list_Cdr(scan))
    result += cont_TermSize(GlobalContext, TermContext, list_Car(scan));

  return result;
}


BOOL cont_TermContainsSymbol(VARCONT GlobalContext, VARCONT TermContext, TERM Term, 
                                SYMBOL Symbol)
/*********************************************************
  INPUT:   A global context where index varibles are bound
           a context, a term and a symbol.
  RETURNS: TRUE, if <Symbol> occurs in <Term> with respect to
           the bindings in <Context>, FALSE otherwise.
********************************************************/
{
  LIST scan;

  Term = cont_Deref(GlobalContext, &TermContext, Term);

  if (symbol_Equal(term_TopSymbol(Term), Symbol))
    return TRUE;
  else
    for (scan = term_ArgumentList(Term); !list_Empty(scan); scan = list_Cdr(scan)) {
      if (cont_TermContainsSymbol(GlobalContext, TermContext, list_Car(scan), Symbol))
	return TRUE;
    }

  return FALSE;
}

TERM cont_Deref(VARCONT GlobalContext, VARCONT* TermContext, TERM Term)
/******************************************************************
  INPUT:      A global context where the Index variables are bound,
              a term <Term> and a call-by-ref context for <Term>.
  RETURNS:    The dereferenced term and the corresponding context.
  SUMMARY:    Dereferences bindings of variables.
  CAUTION:    In general, the context of the returned term <TermContext>
              is different to the input context. 
  ASSUMPTION: All Index variables occuring in <Term> have to be 
              bound in <GlobalContext>,
              no Index variable is mapped to another index variable
*******************************************************************/
{

  if(term_IsIndexVariable(Term)) {
   
    SYMBOL TermTop;
    TermTop = term_TopSymbol(Term);

    #ifdef CHECK
    if(!cont_VarIsBound(GlobalContext, TermTop) ||
       term_IsIndexVariable(cont_ContextBindingTerm(GlobalContext, TermTop))) {
       misc_StartErrorReport();
       misc_ErrorReport("\ncont_Deref: Illegal Context!");
       misc_FinishErrorReport();
    }
    #endif
        
    Term         = cont_ContextBindingTerm(GlobalContext, TermTop);
    *TermContext = cont_ContextBindingContext(GlobalContext, TermTop);
    
    }
        
  while (term_IsVariable(Term) && *TermContext != cont_InstanceContext()) {
    SYMBOL TermTop;

    TermTop = term_TopSymbol(Term);

    if (cont_VarIsBound(*TermContext, TermTop)) {
      VARCONT HelpContext;

      HelpContext = cont_ContextBindingContext(*TermContext, TermTop);
      Term        = cont_ContextBindingTerm(*TermContext, TermTop);
      *TermContext    = HelpContext;
    } 
    else
      return Term;
  }

  return Term;
}

/**************************************************************/
/* ********************************************************** */
/* *							    * */
/* *  OUTPUT             			            * */
/* *							    * */
/* ********************************************************** */
/**************************************************************/


void cont_TermPrintPrefix(VARCONT GlobalContext, VARCONT TermContext, TERM Term)
/**************************************************************
  INPUT:   A global context where index variables are bound,
           a context and a term.
  RETURNS: none.
  SUMMARY: Prints the term modulo the context to stdout. 
  CAUTION: Variables of <Term1> are bound in 
           <TermContext1>  and
           the index variables are bound in <GlobalContext1>
***************************************************************/
{
  Term = cont_Deref(GlobalContext,&TermContext, Term);

  symbol_Print(term_TopSymbol(Term));

  if (term_IsComplex(Term)) {
    LIST List;

    putchar('(');

    for (List = term_ArgumentList(Term); !list_Empty(List);
	 List = list_Cdr(List)) {
      cont_TermPrintPrefix(GlobalContext, TermContext, list_Car(List));

      if (!list_Empty(list_Cdr(List)))
	putchar(',');
    }

    putchar(')');
  }
}
