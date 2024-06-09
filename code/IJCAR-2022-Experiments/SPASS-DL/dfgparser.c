/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                  PARSER FOR DFG SYNTAX                 * */
/* *                                                        * */
/* *  $Module:   DFG                                        * */ 
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
/* $Revision: 1.15 $                                        * */
/* $State: Exp $                                            * */
/* $Date: 2016-03-23 16:54:37 $                             * */
/* $Author: weidenb $                                      * */
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




#include "dfgparser.h"


/* Used for delayed parsing of plain clauses */
typedef struct {
  LIST constraint;
  LIST antecedent;
  LIST succedent;
  TERM selected;
} DFG_PLAINCLAUSEHELP, *DFG_PLAINCLAUSE;

static DFGDESCRIPTION     dfg_DESC;

static ARRAY              dfg_AXIOMARRAY;
static ARRAY              dfg_CONJECARRAY; 

static LIST               dfg_SORTDECLLIST;
static ARRAY              dfg_INCLUDEARRAY;

/* symbol precedence explicitly defined by user */
static LIST               dfg_USERPRECEDENCE;
static LIST               dfg_USERSELECTION;
static LIST               dfg_CLAXRELATION;
static LIST               dfg_PLAINCLAXAX;
static LIST               dfg_PLAINCLAXCON;
static LIST               dfg_CLAXAXIOMS;

static ARRAY              dfg_AXCLAUSES;
static ARRAY              dfg_CONCLAUSES;
static ARRAY              dfg_PLAINAXCLAUSES; /* array of DFG_PLAINCLAUSE */
static ARRAY              dfg_PLAINCONCLAUSES; /* array of DFG_PLAINCLAUSE */

static ARRAY              dfg_DECLARATIONS;

static ARRAY              dfg_TERM_STACK; /* array used for parsing terms */
static ARRAY              dfg_FORMULA_STACK;/* array used for parsing formulas*/

static LIST               dfg_PROOFLIST;     /* list_of_proofs */
static LIST               dfg_TERMLIST;      /* list_of_terms  */


static TERM               dfg_SELECTED_LITERAL; /* last selected literal */
static BOOL               dfg_IGNORE;      /* tokens are ignored while TRUE */
static BOOL		  dfg_IGNORESETTINGS; /* issue a warning for included file with SPASS settings */
static FLAGSTORE          dfg_FLAGS;
static PRECEDENCE         dfg_PRECEDENCE;

/* used in iascanner */
NAT                        dfg_LINENUMBER;

/* used also in the scanner */
BOOL			   dfg_IGNORETEXT;
char*                      dfg_CurrentFilename = NULL;

static void   dfg_SymbolDecl(int, char*, int);
static SYMBOL dfg_Symbol(char*, intptr_t);
static void   dfg_TranslPairDecl(char*, char*);

TERM dfg_TermCreate(char* Name, LIST Arguments)
/***************************************************************
  INPUT:   A string <Name> and a list of terms <Arguments>.
  RETURNS: Does a symbol lookup for <Name> and creates out of
           the found or otherwise created new symbol and <Arguments>
           a new term.
  CAUTION: Deletes the string <Name>.
***************************************************************/
{
  SYMBOL s;
  NAT    arity;
  arity = list_Length(Arguments);
  s = dfg_Symbol(Name, arity); /* Frees the string */
  if (!symbol_IsVariable(s) && !symbol_IsFunction(s)) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportHeader(& token.pos);
    misc_UserErrorReport(" is not a function.\n");
    misc_FinishUserErrorReport();
  }
  return term_Create(s, Arguments);
}

TERM dfg_AtomCreate(char* Name, LIST Arguments)
/* Look up the symbol, check its arity and create the atom term */
{
  SYMBOL s;
  s = dfg_Symbol(Name, list_Length(Arguments)); /* Frees the string */
  if (symbol_IsVariable(s) || !symbol_IsPredicate(s)) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportHeader(& token.pos);
    misc_UserErrorReport(" Symbol is not a predicate.\n");
    misc_FinishUserErrorReport();
  }
  return term_Create(s, Arguments);
}

void dfg_DeleteStringList(LIST List)
{
  list_DeleteWithElement(List, (void (*)(POINTER)) string_StringFree);
}

/**************************************************************/
/* Functions that handle symbols with unspecified arity       */
/**************************************************************/

/* The symbol list holds all symbols whose arity wasn't       */
/* specified in the symbol declaration section.               */
/* If the arity of a symbol was not specified in this section */
/* it is first set to 0. If the symbol occurs with always the */
/* same arity 'A' the arity of this symbol is set to 'A'.     */
static LIST   dfg_SYMBOLLIST;

static void dfg_SymAdd(SYMBOL);
static void dfg_SymCheck(SYMBOL, NAT);
static void dfg_SymCleanUp(void);

/**************************************************************/
/* Functions that handle variable names                       */
/**************************************************************/

/* List of quantified variables in the current input formula. */
/* This list is used to find symbols that by mistake weren't  */
/* declared in the symbol declaration section                 */
/* --> free variables                                         */
/* This is a list of lists, since each time a quantifier is   */
/* reached, a new list is added to the global list.           */
typedef struct {
  char*  name;
  SYMBOL symbol;
} DFG_VARENTRY, *DFG_VAR;

static HASHMAP dfg_VARSCOPE;
static LIST    dfg_VARLIST;
static BOOL    dfg_VARDECL;

static void   dfg_VarStart(void);
static void   dfg_VarStop(void);
static void   dfg_VarBacktrack(void);
static void   dfg_VarCheck(void);
static SYMBOL dfg_VarLookup(char*,pos_t);
static DFG_PLAINCLAUSE dfg_PlainClauseCreate(void);
static void dfg_PlainClauseFree(DFG_PLAINCLAUSE Clause);




static void dfg_Init(FILE* Input, FLAGSTORE Flags, PRECEDENCE Precedence, DFGDESCRIPTION Description)
/**************************************************************
  INPUT:   The input file stream for the parser, a flag store,
           a precedence and a problem description store.
  RETURNS: Nothing.
  EFFECT:  The parser and scanner are initialized.
           The parser will use the flag store and the precedence
	   to memorize the settings from the input file.
***************************************************************/
{
  dfg_SORTDECLLIST     = list_Nil();
  dfg_USERPRECEDENCE   = list_Nil();
  dfg_USERSELECTION    = list_Nil();
  dfg_CLAXRELATION     = list_Nil();
  dfg_PLAINCLAXAX      = list_Nil();
  dfg_PLAINCLAXCON     = list_Nil();

  dfg_CLAXAXIOMS       = list_Nil();

  dfg_PROOFLIST        = list_Nil();
  dfg_TERMLIST         = list_Nil();
  dfg_SYMBOLLIST       = list_Nil();
  dfg_VARLIST          = list_Nil();
  dfg_VARSCOPE         = hm_Create(4,(HM_GET_HASH_FUNCTION)hm_StringHash,
				   (HM_KEY_EQUAL_FUNCTION)string_Equal,FALSE);

  dfg_SELECTED_LITERAL = (TERM) NULL;
  dfg_VARDECL          = FALSE;
  dfg_IGNORE           = FALSE;
  dfg_FLAGS            = Flags;
  dfg_PRECEDENCE       = Precedence;
  dfg_DESC             = Description;

  flag_SetFlagIntValue(dfg_FLAGS, flag_EML, flag_EMLOFF);
}



static void dfg_Free(FLAGSTORE Flags, PRECEDENCE Precedence, DFGDESCRIPTION Description)
/**************************************************************
  INPUT:   The a flag store,
           a precedence and a problem description store.
  RETURNS: Nothing.
  EFFECT:  The parser and scanner are freed.
***************************************************************/
{
  hm_Delete(dfg_VARSCOPE);
}

void dfg_InitGlobalArrays(void)
/**************************************************************
  INPUT:   Nothing
  RETURNS: Nothing.
  EFFECT:  The global arrays are initialized.
***************************************************************/
{
  dfg_AXIOMARRAY      = array_Create(256,100);
  dfg_CONJECARRAY     = array_Create(256,100); 
  dfg_AXCLAUSES       = array_Create(256,100);
  dfg_CONCLAUSES      = array_Create(256,100);
  dfg_PLAINAXCLAUSES  = array_Create(256,100);
  dfg_PLAINCONCLAUSES = array_Create(256,100);
  dfg_DECLARATIONS    = array_Create(256,100);
  dfg_INCLUDEARRAY    = array_Create(256,100);

  dfg_TERM_STACK      = array_Create(16,100);
  dfg_FORMULA_STACK   = array_Create(16,100);
}

void dfg_CleanGlobalArrays(void)
/**************************************************************
  INPUT:   Nothing
  RETURNS: Nothing.
  EFFECT:  The global arrays are cleaned.
***************************************************************/
{
  dfg_AXIOMARRAY      = array_Clean(dfg_AXIOMARRAY);
  dfg_CONJECARRAY     = array_Clean(dfg_CONJECARRAY); 
  dfg_AXCLAUSES       = array_Clean(dfg_AXCLAUSES);
  dfg_CONCLAUSES      = array_Clean(dfg_CONCLAUSES);
  dfg_PLAINAXCLAUSES  = array_Clean(dfg_PLAINAXCLAUSES);
  dfg_PLAINCONCLAUSES = array_Clean(dfg_PLAINCONCLAUSES);
  dfg_DECLARATIONS    = array_Clean(dfg_DECLARATIONS);
  dfg_INCLUDEARRAY    = array_Clean(dfg_INCLUDEARRAY);
}

void dfg_DeleteGlobalArrays(void)
/**************************************************************
  INPUT:   Nothing
  RETURNS: Nothing.
  EFFECT:  The global arrays are deleted.
***************************************************************/
{
  array_Delete(dfg_AXIOMARRAY);
  array_Delete(dfg_CONJECARRAY); 
  array_Delete(dfg_AXCLAUSES);
  array_Delete(dfg_CONCLAUSES);
  array_Delete(dfg_PLAINAXCLAUSES);
  array_Delete(dfg_PLAINCONCLAUSES);
  array_Delete(dfg_DECLARATIONS);
  array_Delete(dfg_INCLUDEARRAY);

  array_Delete(dfg_TERM_STACK);
  array_Delete(dfg_FORMULA_STACK);
}

void dfg_DeleteClAxRelation(LIST ClAxRelation)
{
  LIST Scan1, Scan2;

  for (Scan1=ClAxRelation;!list_Empty(Scan1);Scan1=list_Cdr(Scan1)) {
    for (Scan2=list_Cdr(list_Car(Scan1));!list_Empty(Scan2);Scan2=list_Cdr(Scan2))
      string_StringFree((char *)list_Car(Scan2));
    list_Delete(list_Car(Scan1));
  }
  list_Delete(ClAxRelation);
}

FILE* dfg_OpenFile(const char * FileName, char* IncludePath, char ** const DiscoveredName)
/**************************************************************
  INPUT: A string filename, a string IncludePath,
           and a pointer to char pointer
           to hold the fullpath name of the file 
           that was opened in the end (can be null).
  RETURNS: A opened file.
  EFFECT:
  Opens a file for reading.
  The file is searched for using the extended search mechanism.
  If the parameter IncludePath has non-trivial value, its value is used,
  otherwise we use the SPASSINPUT environment variable.  
***************************************************************/
{ 
  if(dfg_CurrentFilename != NULL){
    string_StringFree(dfg_CurrentFilename);
    dfg_CurrentFilename = NULL;
  }
  dfg_CurrentFilename = string_StringCopy(FileName);
  if (IncludePath != NULL && strlen(IncludePath)>0) 
    return misc_OpenFileExt(FileName,"r",IncludePath,DiscoveredName);
  else
    return misc_OpenFileEnv(FileName,"r","SPASSINPUT",DiscoveredName);
}

static LIST dfg_TestList; /* list of strings used in nonmember tests */
	
static BOOL dfg_LabelFormulaPairIsNonmember(POINTER pair) {
  if (list_PairFirst(pair) == NULL)
    return TRUE;
	
  return !list_Member(dfg_TestList,list_PairFirst(pair),(BOOL (*)(POINTER, POINTER))string_Equal);
}

static void dfg_DeleteLabelTermPair(POINTER pair)	{
  term_Delete(list_PairSecond(pair));
  if (list_PairFirst(pair) != NULL)
    string_StringFree(list_PairFirst(pair));  /* Free the label */
  list_PairFree(pair);
}


void dfg_FilterClausesBySelection(LIST* Clauses, LIST* ClAxRelation, LIST selection)
/**************************************************************
  INPUT: A pointer to a clause list, a pointer to a ClAxRelation list
         and a char* list.
  RETURNS: Nothing.
  EFFECT: The clause list is filtered in such a way that clauses
          with names not in the selection list are deleted.
          The name for the clauses is found in the ClAxRelation structure.
          And the corresponding entry in ClAxRelation is also 
          deleted if the clause is.
          The clause list and the ClAxRelation list are assumed to be coherent.
  NOTE: The coherence is assured when the lists come from
        an included file, because in includes settings sections are forbidden
        and so the user cannot specify ClAxRelation herself.
***************************************************************/
{
  /* result lists */
  LIST RClauses;
  LIST RClAxRelation;

  /* pointers to last element in the result */
  LIST RLClauses = list_Nil();
  LIST RLClAxRelation = list_Nil();

  RClauses = list_Nil();
  RClAxRelation = list_Nil();

  /*
    Performs parallel tranversal of Clauses and ClAxRelation.
    Either deleting from both or keeping both for the result.
  */

  while (!list_Empty(*Clauses)) {  
    CLAUSE Clause;
    LIST ClAx;
    LIST Labels;
    char* Label;

    /* current list elements */
    LIST CClause;
    LIST CClAxRelation;

    CClause = *Clauses;
    CClAxRelation = *ClAxRelation;

    Clause = (CLAUSE) list_Car(CClause);

#ifdef CHECK
    if (list_Empty(CClAxRelation)) {
      misc_StartUserErrorReport();
      misc_ErrorReport("\n In dfg_FilterClausesBySelection: ClAxRelation too short!\n");
      misc_FinishUserErrorReport();
    }
#endif

    ClAx = (LIST) list_Car(CClAxRelation);

#ifdef CHECK
    if ((intptr_t)list_Car(ClAx) != clause_Number(Clause)) {
      misc_StartUserErrorReport();
      misc_ErrorReport("\n In dfg_FilterClausesBySelection: Incompatible ClAxRelation!\n");
      misc_FinishUserErrorReport();
    }
#endif

    Labels = list_Cdr(ClAx);
    if (list_Empty(Labels)) 
      Label = NULL;
    else
      Label = (char*)list_Car(Labels);

    /*and we can already move on*/
    *Clauses = list_Cdr(*Clauses);
    *ClAxRelation = list_Cdr(*ClAxRelation);

    if (!Label || !list_Member(selection,Label,(BOOL (*)(POINTER, POINTER))string_Equal)) {
      /* deleting */
      if (RClauses) { /*if RClauses exists so does RLClauses and RLClAxRelation*/
        list_Rplacd(RLClauses,*Clauses);
        list_Rplacd(RLClAxRelation,*ClAxRelation);
      }

      clause_Delete(Clause);
      list_DeleteWithElement(Labels,(void (*)(POINTER))string_StringFree);
      list_Free(ClAx);
      list_Free(CClause);
      list_Free(CClAxRelation);

    } else {
      /* keeping */
      if (!RClauses) {
        /* the result will contain at least one element */
        RClauses = CClause;
        RClAxRelation = CClAxRelation;
      }

      /* if we delete the next, whis will need to be updated */
      RLClauses = CClause;
      RLClAxRelation = CClAxRelation;
    }
  }

  *Clauses = RClauses;
  *ClAxRelation = RClAxRelation;
}


void dfg_FilterClausesBySelectionA(ARRAY* Clauses, LIST* ClAxRelation, LIST selection)
/**************************************************************
  INPUT: A pointer to a clause list, a pointer to a ClAxRelation list
         and a char* list.
  RETURNS: Nothing.
  EFFECT: The clause list is filtered in such a way that clauses
          with names not in the selection list are deleted.
          The name for the clauses is found in the ClAxRelation structure.
          And the corresponding entry in ClAxRelation is also 
          deleted if the clause is.
          The clause list and the ClAxRelation list are assumed to be coherent.
  NOTE: The coherence is assured when the lists come from
        an included file, because in includes settings sections are forbidden
        and so the user cannot specify ClAxRelation herself.
***************************************************************/
{
  /* result lists */

  LIST RClAxRelation;

  /* pointers to last element in the result */

  LIST RLClAxRelation;
  int i, j;


  RClAxRelation  = list_Nil();
  RLClAxRelation = list_Nil();
  /*
    Performs parallel tranversal of Clauses and ClAxRelation.
    Either deleting from both or keeping both for the result.
  */

  j = 0;

  for(i = 0; i<array_GetSize(*Clauses); i++) {  
    CLAUSE Clause;
    LIST ClAx;
    LIST Labels;
    char* Label;

    /* current list elements */
    LIST CClAxRelation;

    CClAxRelation = *ClAxRelation;

    Clause = (CLAUSE)array_GetElement(*Clauses,i);

#ifdef CHECK
    if (list_Empty(CClAxRelation)) {
      misc_StartUserErrorReport();
      misc_ErrorReport("\n In dfg_FilterClausesBySelectionA: ClAxRelation too short!\n");
      misc_FinishUserErrorReport();
    }
#endif

    ClAx = (LIST) list_Car(CClAxRelation);
#ifdef CHECK
    if ((intptr_t)list_Car(ClAx) != clause_Number(Clause)) {
      misc_StartUserErrorReport();
      misc_ErrorReport("\n In dfg_FilterClausesBySelectionA: Incompatible ClAxRelation!\n");
      misc_FinishUserErrorReport();
    }
#endif

    Labels = list_Cdr(ClAx);
    if (list_Empty(Labels)) 
      Label = NULL;
    else
      Label = (char*)list_Car(Labels);

    /*and we can already move on*/
    
    *ClAxRelation = list_Cdr(*ClAxRelation);

    if (!Label || !list_Member(selection,Label,(BOOL (*)(POINTER, POINTER))string_Equal)) {
      /* deleting */
      if (RClAxRelation) { /*if RClauses exists so does RLClauses and RLClAxRelation*/
    
	list_Rplacd(RLClAxRelation,*ClAxRelation);
      }

      clause_Delete(Clause);
      list_DeleteWithElement(Labels,(void (*)(POINTER))string_StringFree);
      list_Free(ClAx);
      list_Free(CClAxRelation);

      *Clauses = array_AddAtIndex(*Clauses,i,(intptr_t)NULL);

    } else {
      /* keeping */
      if (!RClAxRelation) {
        /* the result will contain at least one element */

        RClAxRelation = CClAxRelation;
      }
      /* if we delete the next, whis will need to be updated */

      RLClAxRelation = CClAxRelation;
    }
  }

  *Clauses = array_PointerDeleteElement(*Clauses,(intptr_t)NULL);
  *ClAxRelation = RClAxRelation;
}

LIST dfg_DFGParser(FILE* File, char * IncludePath, FLAGSTORE Flags, PRECEDENCE Precedence, DFGDESCRIPTION Description,
		   LIST* Axioms, LIST* Conjectures, LIST* Declarations,
		   LIST* UserDefinedPrecedence, LIST* UserDefinedSelection,
		   LIST* ClAxRelation, BOOL* HasPlainClauses)
/**************************************************************
  A stub around the real parser that takes care of following
  include directives.
  
  NOTE: This function is almost the same as tptp_TPTPParser form tptpparser.y
    any change in its implementation should propably be also propagated there.
***************************************************************/
{
  ARRAY Clauses, AxiomsA, Axs, ConjecturesA, Conjs, Cls, Decl, Includes;
  LIST FilesRead, Return;
  DFGDESCRIPTION description_dummy;
  int i;

  dfg_InitGlobalArrays();

  Clauses      = array_Create(256,100);
  AxiomsA      = array_Create(256,100);
  ConjecturesA = array_Create(256,100);
  Axs          = array_Create(256,100);
  Conjs        = array_Create(256,100);
  Decl         = array_Create(256,100);
  Includes     = array_Create(256,100);

  FilesRead    = list_Nil();	

  dfg_IGNORESETTINGS= FALSE;

  Cls = dfg_DFGParserIncludesExplicitA(File,Flags,Precedence,Description,FALSE,&AxiomsA,&ConjecturesA,&Decl,UserDefinedPrecedence,UserDefinedSelection,ClAxRelation,&Includes,HasPlainClauses);

  Clauses = array_Nconc(Clauses,Cls);

  for (i = 0; i<array_GetSize(Includes); i++) {
    LIST pair;
    char* filename;
    LIST selection;
  
    pair      = (LIST)array_GetElement(Includes,i);
    filename  = (char*)list_PairFirst(pair);
    selection = (LIST)list_PairSecond(pair);
    list_PairFree(pair);
		
    if (list_Member(FilesRead,filename,(BOOL (*)(POINTER, POINTER))string_Equal)) {
      misc_UserWarning("File %s already included, skipped!\n",filename);
      string_StringFree(filename);
    } else {
      LIST  UDS, CAR;
      BOOL HPC;
      FILE* FileToInclude;      
				
      FileToInclude = dfg_OpenFile(filename, IncludePath, NULL);

      UDS = CAR = list_Nil();

      dfg_IGNORESETTINGS = TRUE; 
      description_dummy = desc_Create();
      Cls = dfg_DFGParserIncludesExplicitA(FileToInclude, Flags, Precedence, description_dummy, TRUE, &Axs, &Conjs, &Decl, UserDefinedPrecedence, &UDS, &CAR, &Includes, &HPC);
      desc_Delete(description_dummy);

      if (list_Exist(selection)) { /*use the selection to filter Axs and Conjs */
        dfg_FilterClausesBySelectionA(&Cls,&CAR,selection);
      
        dfg_TestList = selection;
        Axs = array_DeleteElementIfFree(Axs,(BOOL (*)(intptr_t))dfg_LabelFormulaPairIsNonmember,(void (*)(intptr_t))dfg_DeleteLabelTermPair);
	Conjs = array_DeleteElementIfFree(Conjs,(BOOL (*)(intptr_t))dfg_LabelFormulaPairIsNonmember,(void (*)(intptr_t))dfg_DeleteLabelTermPair);
      }

      Clauses = array_Nconc(Clauses, Cls); 

      AxiomsA = array_Nconc(AxiomsA,Axs);
      ConjecturesA = array_Nconc(ConjecturesA,Conjs);

      /* "Summing up" UserDefinedSelection and ClAxRelation from all the files included. */
      *UserDefinedSelection = list_Nconc(*UserDefinedSelection, UDS);
      /*
	No explicit (user specified) ClAxRelation coming in dfg from includes!      
	*ClAxRelation = list_Nconc(*ClAxRelation, CAR);
	*/
      dfg_DeleteClAxRelation(CAR);

      /*The whole thing has plain clauses only if all the files have!*/			
      if (!HPC)
	*HasPlainClauses = FALSE;

      misc_CloseFile(FileToInclude,filename);
      FilesRead = list_Cons(filename,FilesRead);
    }

    list_DeleteWithElement(selection,(void (*)(POINTER))string_StringFree);		
  }
		
  list_DeleteWithElement(FilesRead,(void (*)(POINTER))string_StringFree);

  *Axioms       = array_ToList(AxiomsA);
  *Conjectures  = array_ToList(ConjecturesA);
  *Declarations = array_ToList(Decl);

  array_Delete(Axs);
  array_Delete(Conjs);
  array_Delete(Includes);
  array_Delete(AxiomsA);
  array_Delete(ConjecturesA);
  array_Delete(Decl);

  Return = array_ToList(Clauses);
  array_Delete(Clauses);

  dfg_DeleteGlobalArrays();

  return Return;
}

void dfg_InitExplicitParser() 
/**************************************************************
  This function should be called 
  prior to calling dfg_DFGParserIncludesExplicit
  from outside the dfg module.
***************************************************************/
{
  dfg_IGNORESETTINGS = FALSE; 
}

void dfg_CreateClausesFromTerms(LIST* Axioms, LIST* Conjectures,
				LIST* ClAxRelation, BOOL BuildClAx,
				FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   Pointers to two lists of term-label pairs,
            pointer to ClAx list,
            boolean flag and
            a flag store and a precedence.
  EFFECT:  Applies dfg_CreateClauseFromTerm to the 
           pairs in the lists treating the firsts as axioms 
           and second as conjectures, releasing the pair struct
           and the labels.
           If BuildClAx is set the labels are not deleted,
           but instead used to record the relation 
           between the clauses and their original names
           in the ClAxRelation structure
           (otherwise ClAxRelation remains intact).
***************************************************************/
{
  LIST    scan, tupel;
  TERM    clauseTerm;
  CLAUSE  clause;
  LIST    ClAxContribution, labels;

  ClAxContribution = list_Nil();
 
  /* Remove clause labels and create clauses from the terms */
  for (scan = *Axioms; !list_Empty(scan); scan = list_Cdr(scan)) {
    tupel = list_Car(scan);
    clauseTerm = list_PairSecond(tupel);
    clause = dfg_CreateClauseFromTerm(clauseTerm,TRUE, Flags, Precedence);
    list_Rplaca(scan, clause);

    if (BuildClAx) {
      if (list_PairFirst(tupel) != NULL)
        labels = list_List(list_PairFirst(tupel));
      else
        labels = list_Nil();
      
      ClAxContribution = list_Cons(list_Cons((POINTER)clause_Number(clause),labels),ClAxContribution);
    } else {
      if (list_PairFirst(tupel) != NULL)        /* Label is defined */
        string_StringFree(list_PairFirst(tupel));  /* Delete the label */
    }
  
    list_PairFree(tupel);
  }
  /* Since dfg_CreateClauseFromTerm() returns NULL for trivial tautologies */
  /* we now delete those NULL pointers from the clause list.               */
  *Axioms = list_PointerDeleteElement(*Axioms, NULL);
  for (scan = *Conjectures; !list_Empty(scan); scan = list_Cdr(scan)) {
    tupel = list_Car(scan);
    clauseTerm = list_PairSecond(tupel);
    clause = dfg_CreateClauseFromTerm(clauseTerm,FALSE, Flags, Precedence);
    list_Rplaca(scan, clause);

    if (BuildClAx) {
      if (list_PairFirst(tupel) != NULL)
        labels = list_List(list_PairFirst(tupel));
      else
        labels = list_Nil();
      
      ClAxContribution = list_Cons(list_Cons((POINTER)clause_Number(clause),labels),ClAxContribution);
    } else {
      if (list_PairFirst(tupel) != NULL)        /* Label is defined */
        string_StringFree(list_PairFirst(tupel));  /* Delete the label */
    }         
  
    list_PairFree(tupel);
  }
  /* Since dfg_CreateClauseFromTerm() returns NULL for trivial tautologies */
  /* we now delete those NULL pointers from the clause list.               */
  *Conjectures = list_PointerDeleteElement(*Conjectures, NULL);

  if (BuildClAx) {
    /* appending the contribution to the end! */
    *ClAxRelation = list_Nconc(*ClAxRelation,list_NReverse(ClAxContribution));
  }
}

void dfg_CreateClausesFromTermsA(ARRAY* Axioms, ARRAY* Conjectures,
				LIST* ClAxContrAx, LIST* ClAxContrCon, BOOL BuildClAx,
				FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   Pointers to two arrays of term-label pairs,
            pointer to return two ClAx lists,
            boolean flag and
            a flag store and a precedence.
  EFFECT:  Applies dfg_CreateClauseFromTerm to the 
           pairs in the lists treating the firsts as axioms 
           and second as conjectures, releasing the pair struct
           and the labels.
           If BuildClAx is set the labels are not deleted,
           but instead used to record the relation 
           between the clauses and their original names
           in the two ClAx lists.
***************************************************************/
{
  LIST    tupel;
  TERM    clauseTerm;
  CLAUSE  clause;
  LIST    labels;
  int i;
 
  /* Remove clause labels and create clauses from the terms */
  for (i = 0; i<array_GetSize(*Axioms); i++) {
    tupel = (LIST)array_GetElement(*Axioms,i);
    clauseTerm = list_PairSecond(tupel);
    clause = dfg_CreateClauseFromTerm(clauseTerm,TRUE, Flags, Precedence);
    array_SetElement(*Axioms, i, (intptr_t)clause);

    if (BuildClAx) {
      if (list_PairFirst(tupel) != NULL)
        labels = list_List(list_PairFirst(tupel));
      else
        labels = list_Nil();
      
      *ClAxContrAx = list_Cons(list_Cons((POINTER)clause_Number(clause),labels),*ClAxContrAx);
    } else {
      if (list_PairFirst(tupel) != NULL)        /* Label is defined */
        string_StringFree(list_PairFirst(tupel));  /* Delete the label */
    }
  
    list_PairFree(tupel);
  }
  /* Since dfg_CreateClauseFromTerm() returns NULL for trivial tautologies */
  /* we now delete those NULL pointers from the clause list.               */
  *Axioms = array_PointerDeleteElement(*Axioms, (intptr_t)NULL);
  for (i = 0; i<array_GetSize(*Conjectures); i++) {
    tupel = (LIST)array_GetElement(*Conjectures,i);
    clauseTerm = list_PairSecond(tupel);
    clause = dfg_CreateClauseFromTerm(clauseTerm,FALSE, Flags, Precedence);
    array_SetElement(*Conjectures, i, (intptr_t)clause);

    if (BuildClAx) {
      if (list_PairFirst(tupel) != NULL)
        labels = list_List(list_PairFirst(tupel));
      else
        labels = list_Nil();
      
      *ClAxContrCon = list_Cons(list_Cons((POINTER)clause_Number(clause),labels),*ClAxContrCon);
    } else {
      if (list_PairFirst(tupel) != NULL)        /* Label is defined */
        string_StringFree(list_PairFirst(tupel));  /* Delete the label */
    }         
  
    list_PairFree(tupel);
  }
  /* Since dfg_CreateClauseFromTerm() returns NULL for trivial tautologies */
  /* we now delete those NULL pointers from the clause list.               */
  *Conjectures = array_PointerDeleteElement(*Conjectures, (intptr_t)NULL);
}

static void dfg_SubSort(char* Name1, char* Name2, pos_t pos1, pos_t pos2)
/**************************************************************
  INPUT:   Two sort symbol names and the positions of the two 
           sorts in the input file.
  RETURNS: Nothing.
  EFFECT:  This functions adds the formula
           forall([U], implies(Name1(U), Name2(U)))
	   to the list of axiom formulas. Both <Name1> and <Name2>
	   are deleted.
***************************************************************/
{
  SYMBOL s1, s2;
  TERM   varterm, t1, t2, term;

  s1 = dfg_Symbol(Name1, 1);   /* Should be unary predicates */
  s2 = dfg_Symbol(Name2, 1);
  if (!symbol_IsPredicate(s1)) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportF(&pos1,"Symbol is not a sort predicate.");
    misc_FinishUserErrorReport();
  }
  if (!symbol_IsPredicate(s2)) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportF(&pos2,"Symbol is not a sort predicate.");
    misc_FinishUserErrorReport();
  }

  varterm = term_Create(symbol_CreateStandardVariable(), list_Nil());
  symbol_ResetStandardVarCounter();
  
  t1   = term_Create(s1, list_List(varterm));
  t2   = term_Create(s2, list_List(term_Copy(varterm)));
  term = term_Create(fol_Implies(), list_Cons(t1, list_List(t2)));
  term = fol_CreateQuantifier(fol_All(), list_List(term_Copy(varterm)),
			      list_List(term));
  dfg_DECLARATIONS = array_Add(dfg_DECLARATIONS,
			       (intptr_t)list_PairCreate(NULL,term));
}


LIST dfg_DFGParserIncludesExplicit(FILE* File, FLAGSTORE Flags, PRECEDENCE Precedence,
				   DFGDESCRIPTION Description, BOOL AppendImplicitClAx,
				   LIST* Axioms, LIST* Conjectures, LIST* SortDecl,
				   LIST* UserDefinedPrecedence, LIST* UserDefinedSelection,
				   LIST* ClAxRelation, LIST* Includes, BOOL* HasPlainClauses)
/**************************************************************
  INPUT:   The input file containing clauses or formulae in DFG syntax,
           a flag store, a precedence used to memorize settings
	       from the file, and Description to store problem description.
           Boolean flag determinig whether the implicit ClAxRelation
           from Clauses should be appended to the explicit one 
           coming from settings.
           Axioms, Conjectures, SortDecl, UserDefinedPrecedence
           UserDefinedSelection, and ClAxRelation, and Includes are
	   pointers to lists used as return values.
           HasPlainClauses is a return value to indicate if
           the problem had clauses in the plain format.
  RETURNS: The list of clauses from File.
  EFFECT:  Reads formulae and clauses from the input file.
           The axioms, conjectures, sort declarations, user-defined
	   precedences, and includes are appended to the respective lists,
       the lists are not deleted!
	   The Includes list contains pairs (filename, selection),
	   where selection is a list of names of formulas chosen to include.
	   All lists except the returned clause list contain pairs
	   (label, term), where <label> may be NULL, if no
	   label was specified for that term.
	   <UserDefinedPrecedence> contains symbols sorted by decreasing
	   precedence. This list will only be changed, if the precedence
	   is explicitly defined in the input file. This can be done
	   by the 'set_precedence' flag in the SPASS settings list in
	   the DFG input file.
           <UserDefinedSelection> is set to a list of predicates to be
           selected in clause inferences by the 'set_selection' flag
           <ClAxRelation> is set to a clause, axiom relation where the
           clauses are given by their number, the axioms by name (string)
  CAUTION: The weight of the clauses is not correct and the literals
           are not oriented!
***************************************************************/
{
  ARRAY AxiomsA;
  ARRAY ConjecturesA;
  ARRAY DeclA;
  ARRAY IncludesA;
  ARRAY ClausesA;
  LIST  Return;

  dfg_InitGlobalArrays();

  AxiomsA      = array_Create(256,100);
  ConjecturesA = array_Create(256,100);
  DeclA        = array_Create(256,100);
  IncludesA    = array_Create(256,100);

  ClausesA = dfg_DFGParserIncludesExplicitA(File,Flags,Precedence,Description,AppendImplicitClAx,
					    &AxiomsA,&ConjecturesA,&DeclA,UserDefinedPrecedence,
					    UserDefinedSelection,ClAxRelation,&IncludesA,HasPlainClauses);

  *Axioms       = array_ToList(AxiomsA);
  *Conjectures  = array_ToList(ConjecturesA);
  *SortDecl     = array_ToList(DeclA);
  *Includes     = array_ToList(IncludesA);
  
  array_Delete(IncludesA);
  array_Delete(AxiomsA);
  array_Delete(ConjecturesA);
  array_Delete(DeclA);

  Return = array_ToList(ClausesA);

  dfg_DeleteGlobalArrays();

  return Return;
}


ARRAY dfg_DFGParserIncludesExplicitA(FILE* File, FLAGSTORE Flags, PRECEDENCE Precedence, DFGDESCRIPTION Description, BOOL AppendImplicitClAx, ARRAY* Axioms, ARRAY* Conjectures, ARRAY* Declarations,  LIST* UserDefinedPrecedence, LIST* UserDefinedSelection, LIST* ClAxRelation, ARRAY* Includes, BOOL* HasPlainClauses)
/**************************************************************
  INPUT:   The input file containing clauses or formulae in DFG syntax,
           a flag store, a precedence used to memorize settings
	       from the file, and Description to store problem description.
           Boolean flag determinig whether the implicit ClAxRelation
           from Clauses should be appended to the explicit one 
           coming from settings.
           Axioms, Conjectures, Declarations, and Includes are
	   pointers to arrays used as return values.
           UserDefinedPrecedence, UserDefinedSelection, and 
           ClAxRelation are pointers to lists used as return values.
           HasPlainClauses is a return value to indicate if
           the problem had clauses in the plain format.
  RETURNS: The list of clauses from File.
  EFFECT:  Reads formulae and clauses from the input file.
           The axioms, conjectures, sort declarations, user-defined
	   precedences, and includes are appended to the respective lists,
       the lists are not deleted!
	   The Includes list contains pairs (filename, selection),
	   where selection is a list of names of formulas chosen to include.
	   All lists except the returned clause list contain pairs
	   (label, term), where <label> may be NULL, if no
	   label was specified for that term.
	   <UserDefinedPrecedence> contains symbols sorted by decreasing
	   precedence. This list will only be changed, if the precedence
	   is explicitly defined in the input file. This can be done
	   by the 'set_precedence' flag in the SPASS settings list in
	   the DFG input file.
           <UserDefinedSelection> is set to a list of predicates to be
           selected in clause inferences by the 'set_selection' flag
           <ClAxRelation> is set to a clause, axiom relation where the
           clauses are given by their number, the axioms by name (string)
  CAUTION: The weight of the clauses is not correct and the literals
           are not oriented!
***************************************************************/
{
  LIST  ClAxContrAx, ClAxContrCon;
  ARRAY ascan;
  int size, i;

  dfg_Init(File, Flags, Precedence, Description);  /* Initialize the parser and scanner */

  dfg_CleanGlobalArrays();
  
  dfg_parse(File,dfg_CurrentFilename);

  dfg_SymCleanUp();

  ClAxContrAx  = list_Nil();
  ClAxContrCon = list_Nil();

  dfg_CreateClausesFromTermsA(&dfg_AXCLAUSES,&dfg_CONCLAUSES,&ClAxContrAx,&ClAxContrCon,AppendImplicitClAx,Flags,Precedence);

  /* Delete the proof list */
  dfg_DeleteProofList(dfg_PROOFLIST);

  /* Delete the list_of_terms, since it'll be ignored */
  term_DeleteTermList(dfg_TERMLIST);

  if (array_GetSize(dfg_PLAINAXCLAUSES) == 0
      && array_GetSize(dfg_PLAINCONCLAUSES) == 0)
    *HasPlainClauses = FALSE;
  else
    *HasPlainClauses = TRUE;

  dfg_PLAINCLAXAX = list_NReverse(dfg_PLAINCLAXAX);
  ascan = dfg_PLAINAXCLAUSES;
  size  =  array_GetSize(dfg_PLAINAXCLAUSES);
  for(i = 0; i < size ; i++){
    DFG_PLAINCLAUSE clause;
    CLAUSE newclause;
    LIST   labels;

    clause    = (DFG_PLAINCLAUSE)array_GetElement(ascan,i);
    newclause = clause_CreateFromLiteralLists(clause->constraint, clause->antecedent, clause->succedent, FALSE, clause->selected);
    
    dfg_AXCLAUSES = array_Add(dfg_AXCLAUSES,(intptr_t)newclause);
  
    dfg_PlainClauseFree(clause);
    
    
    if(AppendImplicitClAx){
      if (list_Car(dfg_PLAINCLAXAX) != NULL)
	labels = list_List(list_Car(dfg_PLAINCLAXAX));
      else
	labels = list_Nil();
      
      ClAxContrAx = list_Cons(list_Cons((POINTER)clause_Number(newclause),labels),ClAxContrAx); 
    }else{
      if (list_Car(dfg_PLAINCLAXAX) != NULL)        /* Label is defined */
        string_StringFree(list_Car(dfg_PLAINCLAXAX));  /* Delete the label */
    }
    dfg_PLAINCLAXAX = list_Pop(dfg_PLAINCLAXAX);
  } 
  array_Clean(ascan); 
  
  dfg_PLAINCLAXCON = list_NReverse(dfg_PLAINCLAXCON);
  ascan = dfg_PLAINCONCLAUSES;
  size  = array_GetSize(dfg_PLAINCONCLAUSES);
  for(i = 0; i < size ; i++){   
    DFG_PLAINCLAUSE clause;
    CLAUSE newclause;
    LIST   labels;

    clause    = (DFG_PLAINCLAUSE)array_GetElement(ascan,i);
    newclause = clause_CreateFromLiteralLists(clause->constraint, clause->antecedent, clause->succedent, TRUE, clause->selected);

    dfg_CONCLAUSES = array_Add(dfg_CONCLAUSES,(intptr_t)newclause);
     
    dfg_PlainClauseFree(clause);

    if(AppendImplicitClAx){
      if (list_Car(dfg_PLAINCLAXCON) != NULL)
	labels = list_List(list_Car(dfg_PLAINCLAXCON));
      else
	labels = list_Nil();
      
      ClAxContrCon = list_Cons(list_Cons((POINTER)clause_Number(newclause),labels),ClAxContrCon); 
    }else{
      if (list_Car(dfg_PLAINCLAXCON) != NULL)        /* Label is defined */
        string_StringFree(list_Car(dfg_PLAINCLAXCON));  /* Delete the label */
    }
    dfg_PLAINCLAXCON = list_Pop(dfg_PLAINCLAXCON);
  }  
  array_Clean(ascan);

  dfg_AXCLAUSES = array_Nconc(dfg_AXCLAUSES, dfg_CONCLAUSES);

  if (AppendImplicitClAx) {
    /* appending the ClAx contributions to the end! */
    dfg_CLAXRELATION = list_Nconc(dfg_CLAXRELATION,list_NReverse(list_Nconc(ClAxContrCon,ClAxContrAx)));
  }

  *Axioms       = array_Nconc(*Axioms, dfg_AXIOMARRAY);
  *Conjectures  = array_Nconc(*Conjectures, dfg_CONJECARRAY);
  *Includes     = array_Nconc(*Includes, dfg_INCLUDEARRAY);
  *Declarations = array_Nconc(*Declarations, dfg_DECLARATIONS);

  dfg_USERPRECEDENCE     = list_NReverse(dfg_USERPRECEDENCE);
  *UserDefinedPrecedence = list_Nconc(*UserDefinedPrecedence, dfg_USERPRECEDENCE);
  *UserDefinedSelection  = dfg_USERSELECTION;
  *ClAxRelation          = dfg_CLAXRELATION;

  dfg_Free(Flags, Precedence, Description);

  return dfg_AXCLAUSES;
}

LIST dfg_ProofParser(FILE* File, FLAGSTORE Flags, PRECEDENCE Precedence, DFGDESCRIPTION Description)
/**************************************************************
  INPUT:   The input file containing clauses in DFG syntax,
           a flag store and a precedence used to memorize settings
           from the file.
  RETURNS: A list of tuples (label,clause,justificationlist,splitlevel,origin)
           representing a proof.
  EFFECT:  Reads inputs clauses with labels and the proof lists
           from the input file.
           The elements of the list are lists with five items.
	   1. the label (a string) of a clause,
	   2. the clause in TERM format,
           3. the list of justification labels (strings, too),
           4. the split level of the clause,
           5. the origin of the clause (RULE struct from clause.h).
	   Note that the justification list is empty for input
	   clauses.
***************************************************************/
{
  LIST  tupel;
  TERM  term;
  
  dfg_Init(File, Flags, Precedence, Description);  /* Initialize the parser and scanner */
  dfg_InitGlobalArrays();
  
  dfg_parse(File,dfg_CurrentFilename);          /* Invoke the parser */

  dfg_SymCleanUp();

  dfg_PROOFLIST = list_NReverse(dfg_PROOFLIST);

  while (array_GetSize(dfg_CONCLAUSES) > 0) {
    tupel = (LIST)array_Pop(dfg_CONCLAUSES);
    term  = list_PairSecond(tupel);
    if (list_PairFirst(tupel) == NULL) {
      /* Ignore input clauses without label */
      term_Delete(term);
      list_PairFree(tupel);
    } else {
      /* Expand the pair to a tuple                            */
      /* (label,clause,justificationlist, split level, origin) */
      /* For input clauses the justificationlist is empty.     */
      /* Input clauses have split level 0.                     */
      list_Rplacd(tupel, list_Cons(term,list_Cons(list_Nil(),list_Cons(0, list_List((POINTER)INPUTCLAUSE)))));
      dfg_PROOFLIST = list_Cons(tupel,dfg_PROOFLIST);
    }
  }

  while (array_GetSize(dfg_AXCLAUSES) > 0) {
    tupel = (LIST)array_Pop(dfg_AXCLAUSES);
    term  = list_PairSecond(tupel);
    if (list_PairFirst(tupel) == NULL) {
      /* Ignore input clauses without label */
      term_Delete(term);
      list_PairFree(tupel);
    } else {
      /* Expand the pair to a tuple                            */
      /* (label,clause,justificationlist, split level, origin) */
      /* For input clauses the justificationlist is empty.     */
      /* Input clauses have split level 0.                     */
      list_Rplacd(tupel, list_Cons(term,list_Cons(list_Nil(),list_Cons(0, list_List((POINTER)INPUTCLAUSE)))));
      dfg_PROOFLIST = list_Cons(tupel,dfg_PROOFLIST);
    }
  }

  /* Delete the formula lists */
  dfg_DeleteFormulaPairArray(dfg_AXIOMARRAY);
  dfg_DeleteFormulaPairArray(dfg_CONJECARRAY);
  /* Delete includes list*/
  dfg_DeleteIncludePairArray(dfg_INCLUDEARRAY);	  
  /* Delete the list of sort declarations */
  dfg_DeleteFormulaPairArray(dfg_DECLARATIONS);
  dfg_DeleteFormulaPairList(dfg_SORTDECLLIST);
  /* Delete the list_of_terms, since it'll be ignored */
  term_DeleteTermList(dfg_TERMLIST);

  /* Finally append the proof list to the list of input clauses with labels */
  dfg_DeleteGlobalArrays();

  dfg_Free(Flags, Precedence, Description);
 
  return dfg_PROOFLIST;
}


LIST dfg_TermParser(FILE* File, FLAGSTORE Flags, PRECEDENCE Precedence, DFGDESCRIPTION Description)
/**************************************************************
  INPUT:   The input file containing a list of terms in DFG syntax,
           a flag store and a precedence used to memorize settings
	   from the file.
  RETURNS: The list of terms from <File>.
  EFFECT:  Reads terms from the list_of_terms from the input file.
***************************************************************/
{
  
  dfg_Init(File, Flags, Precedence, Description);   /* Initialize the parser and scanner */
  dfg_InitGlobalArrays();
  dfg_parse(File,dfg_CurrentFilename);          /* Invoke the parser */

  dfg_SymCleanUp();

  /* Delete the clause lists */
 
  dfg_DeleteFormulaPairArray(dfg_AXCLAUSES);
  dfg_DeleteFormulaPairArray(dfg_CONCLAUSES);
  /* Delete the formula lists */

 
  dfg_DeleteFormulaPairArray(dfg_AXIOMARRAY);
  dfg_DeleteFormulaPairArray(dfg_CONJECARRAY);
  /* Delete includes list*/
  dfg_DeleteIncludePairArray(dfg_INCLUDEARRAY);	  
  /* Delete the proof list */
  dfg_DeleteProofList(dfg_PROOFLIST);
  /* Delete the list of sort declarations */
  dfg_DeleteFormulaPairList(dfg_SORTDECLLIST);
  dfg_DeleteFormulaPairArray(dfg_DECLARATIONS);

  dfg_DeleteGlobalArrays();
  dfg_Free(Flags, Precedence, Description);
  return dfg_TERMLIST;
}

void dfg_DeleteFormulaPairList(LIST FormulaPairs)
/**************************************************************
  INPUT:   A list of pairs (label, formula).
  RETURNS: Nothing.
  EFFECT:  The list and the pairs with their strings and terms
           are completely deleted.
***************************************************************/
{
  LIST pair;

  for ( ; !list_Empty(FormulaPairs); FormulaPairs = list_Pop(FormulaPairs)) {
    pair = list_Car(FormulaPairs);  /* (label, term) */
    term_Delete(list_PairSecond(pair));
    if (list_PairFirst(pair) != NULL)
      string_StringFree(list_PairFirst(pair));  /* Free the label */
    list_PairFree(pair);
  }
}

void dfg_DeleteFormulaPairArray(ARRAY FormulaPairs)
/**************************************************************
  INPUT:   An array of pairs (label, formula).
  RETURNS: Nothing.
  EFFECT:  The pairs with their strings and terms
           are completely deleted. Array is only cleaned.
***************************************************************/
{
  LIST pair;
  int i;
  for (i = 0 ; i < array_GetSize(FormulaPairs); i++) {
    pair = (LIST)array_GetElement(FormulaPairs,i);  /* (label, term) */
    term_Delete(list_PairSecond(pair));
    if (list_PairFirst(pair) != NULL)
      string_StringFree(list_PairFirst(pair));  /* Free the label */
    list_PairFree(pair);
  }
  array_Clean(FormulaPairs);
}


void dfg_DeleteIncludePairList(LIST IncludePairs) 
/**************************************************************
  INPUT:   A list of pairs (filename, selectionlist).
  RETURNS: Nothing.
  EFFECT:  The list and the pairs with their strings
           are completely deleted.
***************************************************************/
{
  LIST pair;

  for ( ; !list_Empty(IncludePairs); IncludePairs = list_Pop(IncludePairs)) {
    pair = list_Car(IncludePairs);  /* (filename, selectionlist) */

    string_StringFree((char *)list_PairFirst(pair));
    list_DeleteWithElement(list_PairSecond(pair),(void (*)(POINTER))string_StringFree);
    list_PairFree(pair);
  }	
}

void dfg_DeleteIncludePairArray(ARRAY IncludePairs)
/**************************************************************
  INPUT:   An array of pairs (filename, selectionlist).
  RETURNS: Nothing.
  EFFECT:  The pairs with their strings contained in the array 
           are deleted. Array is only cleaned.
***************************************************************/
{
  LIST pair;
  int i;

  for ( i = 0; i <array_GetSize(IncludePairs); i++) {
    pair = (LIST)array_GetElement(IncludePairs,i);  /* (filename, selectionlist) */

    string_StringFree((char *)list_PairFirst(pair));
    list_DeleteWithElement(list_PairSecond(pair),(void (*)(POINTER))string_StringFree);
    list_PairFree(pair);
  }
  array_Clean(IncludePairs);
}

void dfg_StripLabelsFromList(LIST FormulaPairs)
/**************************************************************
  INPUT:   A list of pairs (label, formula).
  RETURNS: Nothing.
  EFFECT:  The pairs are replaced by the respective formula
           and the pairs with their label strings are deleted.
***************************************************************/
{
  LIST pair, scan;

  for (scan = FormulaPairs; !list_Empty(scan); scan = list_Cdr(scan)) {
    pair = list_Car(scan);  /* (label, term) */
    list_Rplaca(scan, list_PairSecond(pair));
    if (list_PairFirst(pair) != NULL)
      string_StringFree(list_PairFirst(pair));  /* Free the label */
    list_PairFree(pair);
  }
}

void dfg_DeleteProofList(LIST Proof)
/**************************************************************
  INPUT:   A list of tuples (label, term, justificationlist, split level).
  RETURNS: Nothing.
  EFFECT:  All memory used by the proof list is freed.
           The labels must NOT be NULL entries!
***************************************************************/
{
  /* Delete the proof list */
  for ( ; !list_Empty(Proof); Proof = list_Pop(Proof)) {
    LIST tupel = list_Car(Proof);
    string_StringFree(list_First(tupel));
    term_Delete(list_Second(tupel));
    dfg_DeleteStringList(list_Third(tupel));
    list_Delete(tupel);
  }
}

/**************************************************************/
/* Static Functions                                           */
/**************************************************************/

static SYMBOL dfg_SymbolDeclAndReturn(int SymbolType, char* Name, int Arity)
/**************************************************************
  INPUT:   The type of a symbol, the name, and the arity.
  RETURNS: The SPASS intern symbol under the input <name>.
  EFFECT:  This function handles the declaration of symbols.
           If <Arity> is -2, it means that the arity of the symbol
	   was not specified, if it is -1 the symbol is declared
	   with arbitrary arity. User defined symbols with arbitrary
           arity are not allowed.
	   The <Name> is deleted.
***************************************************************/
{
  NAT    arity;
  SYMBOL symbol;

  switch (Arity) {
  case -2: /* not specified */
    arity = 0;
    break;
  case -1:
    misc_StartUserErrorReport();
    dfg_UserErrorReportHeader(& token.pos);
    misc_UserErrorReport(" symbols with arbitrary arity are not allowed.\n");
    misc_FinishUserErrorReport();
  default:
    arity = Arity;
  }

  if(((int)arity)<0){
    misc_StartUserErrorReport();
    dfg_UserErrorReportHeader(& token.pos);
    misc_UserErrorReport(" symbols with negativ arity are not allowed.\n");
    misc_FinishUserErrorReport();
  }

  /* Check if this symbol was declared earlier */
  symbol = symbol_Lookup(Name);
  if (symbol != 0) {
    /* Symbol was declared before */
    /* Check if the old and new symbol type are equal */
    if ((SymbolType == DFG_FUNC && !symbol_IsFunction(symbol)) ||
	(SymbolType == DFG_PRDICAT && !symbol_IsPredicate(symbol)) ||
	(SymbolType == DFG_SRT && !symbol_IsPredicate(symbol))) {
      misc_StartUserErrorReport();
      dfg_UserErrorReportHeader(& token.pos);
      misc_UserErrorReport(" symbol %s was already declared as ", symbol_Name(symbol));
      switch (symbol_Type(symbol)) {
      case symbol_CONSTANT:
      case symbol_FUNCTION:
	misc_UserErrorReport("function.\n"); break;
      case symbol_PREDICATE:
	misc_UserErrorReport("predicate.\n"); break;
      case symbol_JUNCTOR:
	misc_UserErrorReport("predefined junctor.\n"); break;
      default:
	misc_UserErrorReport("unknown type.\n");
      }
      misc_FinishUserErrorReport();
    }
    /* Now check the old and new arity if specified */
    if (Arity != -2 && Arity != symbol_Arity(symbol)) {
      misc_StartUserErrorReport();
      dfg_UserErrorReportHeader(& token.pos);
      misc_UserErrorReport(" symbol %s was already declared with arity %d\n",
			   Name, symbol_Arity(symbol));
      misc_FinishUserErrorReport();
    }
  } else {
    /* Symbol was not declared before */
    switch (SymbolType) {
    case DFG_FUNC:
      symbol = symbol_CreateFunction(Name, arity, symbol_STATLEX,dfg_PRECEDENCE);
      break;
    case DFG_PRDICAT:
      symbol = symbol_CreatePredicate(Name, arity,symbol_STATLEX,dfg_PRECEDENCE);
      break;
    case DFG_SRT:
      symbol = symbol_CreatePredicate(Name, arity,symbol_STATLEX,dfg_PRECEDENCE);
      break;
    default:
      symbol = symbol_CreateJunctor(Name, arity, symbol_STATLEX, dfg_PRECEDENCE);
    }
    if (Arity == -2)
      /* Arity wasn't specified so check the arity for each occurrence */
      dfg_SymAdd(symbol);
  }

  string_StringFree(Name);  /* Name was copied */
  return symbol;
}


static void dfg_SymbolDecl(int SymbolType, char* Name, int Arity)
/**************************************************************
  INPUT:   The type of a symbol, the name, and the arity.
  RETURNS: Nothing.
  EFFECT:  This function handles the declaration of symbols.
           If <Arity> is -2, it means that the arity of the symbol
	   was not specified, if it is -1 the symbol is declared
	   with arbitrary arity. User defined symbols with arbitrary
           arity are not allowed.
	   The <Name> is deleted.
***************************************************************/
{
  dfg_SymbolDeclAndReturn(SymbolType,Name,Arity);
}


static SYMBOL dfg_Symbol(char* Name, intptr_t Arity)
/**************************************************************
  INPUT:   The name of a symbol and the actual arity of the symbol.
  RETURNS: The corresponding SYMBOL.
  EFFECT:  This function checks if the <Name> was declared as
           symbol or variable. If not, an error message is printed
	   to stderr.
	   The <Name> is deleted.
***************************************************************/
{
  SYMBOL symbol;

  symbol = symbol_Lookup(Name);
  if (symbol != 0) {
    string_StringFree(Name);
    dfg_SymCheck(symbol, Arity); /* Check the arity */
  } else {
    /* Variable */
    if (Arity > 0) {
      misc_StartUserErrorReport();
      dfg_UserErrorReportHeader(& token.pos);
      misc_UserErrorReport(" Undefined symbol %s.\n",Name);
      misc_FinishUserErrorReport();
    }
    symbol = dfg_VarLookup(Name,token.pos);
  }
  return symbol;
}

static TERM dfg_CreateQuantifier(SYMBOL Symbol, LIST VarTermList, TERM Term)
/**************************************************************
  INPUT:   A quantifier symbol, a list possibly containing sorts,
           and a term.
  RETURNS: The created quantifier term..
***************************************************************/
{
  LIST varlist, sortlist, scan;
  TERM helpterm;

  /* First collect the variable symbols in varlist and the sorts in sortlist */
  varlist = sortlist = list_Nil();
  for ( ; !list_Empty(VarTermList); VarTermList = list_Pop(VarTermList)) {
    helpterm = list_Car(VarTermList);
    if (term_IsVariable(helpterm)) {
      varlist = list_Cons((POINTER)term_TopSymbol(helpterm),varlist);
      term_Delete(helpterm);
    } else {
      SYMBOL var = term_TopSymbol(term_FirstArgument(helpterm));
      varlist  = list_Cons((POINTER)var,varlist);
      sortlist = list_Cons((POINTER)helpterm,sortlist);
    }
  }

  varlist = list_PointerDeleteDuplicates(varlist);
  /* Now create terms from the variables */
  for (scan = varlist; !list_Empty(scan); scan = list_Cdr(scan))
    list_Rplaca(scan, term_Create((SYMBOL)list_Car(scan), list_Nil()));

  if (!list_Empty(sortlist)) {
    if (symbol_Equal(fol_All(), Symbol)) {
      /* The conjunction of all sortterms implies the Term */
      if (symbol_Equal(fol_Or(), term_TopSymbol(Term))) {
	/* Special treatment if <Term> is a term with "or" like */
	/* in clauses: add all sort terms negated to the args    */
	/* of the "or" */
	for (scan = sortlist; !list_Empty(scan); scan = list_Cdr(scan))
	  /* Negate the sort terms */
	  list_Rplaca(scan, term_Create(fol_Not(), list_List(list_Car(scan))));
	sortlist = list_Nconc(sortlist, term_ArgumentList(Term));
	term_RplacArgumentList(Term, sortlist);
      } else {
	/* No "or" term, so build the implication term */
	if (list_Empty(list_Cdr(sortlist))) {
	  /* Only one sort term */
	  list_Rplacd(sortlist, list_List(Term));
	  Term = term_Create(fol_Implies(), sortlist);
	} else {
	  /* More than one sort term */
	  helpterm = term_Create(fol_And(), sortlist);
	  Term = term_Create(fol_Implies(), list_Cons(helpterm, list_List(Term)));
	}
      }
    } else if (symbol_Equal(fol_Exist(), Symbol)) {
      /* Quantify the conjunction of all sort terms and <Term> */
      if (symbol_Equal(fol_And(), term_TopSymbol(Term))) {
	/* Special treatment if <Term> has an "and" as top symbol: */
	/* just add the sort terms to the args of the "and".       */
	sortlist = list_Nconc(sortlist, term_ArgumentList(Term));
	term_RplacArgumentList(Term, sortlist);
      } else {
	sortlist = list_Nconc(sortlist, list_List(Term));
	Term = term_Create(fol_And(), sortlist);
      }
    }
  }
  helpterm = fol_CreateQuantifier(Symbol, varlist, list_List(Term));
  return helpterm;
}


CLAUSE dfg_CreateClauseFromTerm(TERM Clause, BOOL IsAxiom, FLAGSTORE Flags,
				PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause term, a boolean value, a flag store and a precedence.
  RETURNS: The clause term converted to a CLAUSE.
  EFFECT:  This function converts a clause stored as term into an
           EARL clause structure.
	   If 'IsAxiom' is TRUE the clause is treated as axiom
	   clause else as conjecture clause.
           The function deletes the literals "false" and "not(true)"
           if they occur in <Clause>.
	   The contents of the flag store and the precedence are changed
	   because the parser read flag and precedence settings from
  MEMORY:  The clause term is deleted.
***************************************************************/
{
  LIST   literals, scan;
  TERM   literal;
  CLAUSE result;
  
  if (term_TopSymbol(Clause) == fol_All()) {
    /* Remove and free the quantifier and the OR term */
    literals = term_ArgumentList(term_SecondArgument(Clause));
    term_RplacArgumentList(term_SecondArgument(Clause), list_Nil());
  } else {
    /* Remove and free the OR term */
    literals = term_ArgumentList(Clause);
    term_RplacArgumentList(Clause, list_Nil());
  }
  term_Delete(Clause);

  for (scan = literals; !list_Empty(scan); scan = list_Cdr(scan)) {
    literal = (TERM) list_Car(scan);
    if (symbol_IsPredicate(term_TopSymbol(literal))) {  /* Positive literal */
      if (fol_IsFalse(literal)) {
	/* Ignore this literal */
	term_Delete(literal);
	list_Rplaca(scan, NULL); /* Mark the actual list element */
      }
    } else {
      /* Found a negative literal */
      TERM atom;
      atom = term_FirstArgument(literal);
      if (fol_IsTrue(atom)) {
	/* Ignore this literal */
	term_Delete(literal);
	list_Rplaca(literals, NULL); /* Mark the actual list element */
      }
    }
  }
  

  literals = list_PointerDeleteElement(literals, NULL);
  /* Remove the special literals treated above from the list */
  result = clause_CreateFromLiterals(literals, FALSE, !IsAxiom, FALSE, Flags, Precedence);
  /* Don't create sorts! */
  list_Delete(literals);

  return result;
}


static void dfg_SymbolGenerated(SYMBOL SortPredicate, BOOL FreelyGenerated,
				LIST GeneratedBy, pos_t pos_sort,
				pos_t pos_funct)
/**************************************************************
  INPUT:   A sort predicate, a boolean flag, and a list of function
           symbol names.
  RETURNS: Nothing.
  EFFECT:  This function stores the information that the <SortPredicate>
           is generated by the function symbols from the <GeneratedBy>
           list. The list contains only symbol names!
	   The <SortPredicate> AND the symbols from the list get
           the property GENERATED. Additionally the symbols get
	   the property FREELY, if the flag <FreelyGenerated> is TRUE.
***************************************************************/
{
  SYMBOL symbol;
  LIST   scan;

  if (!symbol_IsPredicate(SortPredicate)) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportF(&pos_sort,"Symbol is not a sort predicate.");
    misc_FinishUserErrorReport();
  }
  /* First reset the old information */
  symbol_RemoveProperty(SortPredicate, GENERATED);
  symbol_RemoveProperty(SortPredicate, FREELY);
  list_Delete(symbol_GeneratedBy(SortPredicate));
  /* Now set the new information */
  symbol_AddProperty(SortPredicate, GENERATED);
  if (FreelyGenerated)
    symbol_AddProperty(SortPredicate, FREELY);
  for (scan = GeneratedBy; !list_Empty(scan); scan = list_Cdr(scan)) {
    symbol = symbol_Lookup(list_Car(scan));
    if (symbol == 0) {
      misc_StartUserErrorReport();
      dfg_UserErrorReportF(&pos_funct," undefined symbol %s.",
			   (char*)list_Car(scan));
      misc_FinishUserErrorReport();
    } else if (!symbol_IsFunction(symbol)) { /* must be function or constant */
      misc_StartUserErrorReport();
      dfg_UserErrorReportF(&pos_funct,"Symbol is not a function.");
      misc_FinishUserErrorReport();
    }
    string_StringFree(list_Car(scan));
    list_Rplaca(scan, (POINTER)symbol);  /* change the name list to a symbol list */
    /* Set GENERATED properties for generating symbols */
    symbol_AddProperty(symbol, GENERATED);
    if (FreelyGenerated)
      symbol_AddProperty(symbol, FREELY);
  }
  symbol_SetGeneratedBy(SortPredicate, GeneratedBy);
}

static void dfg_TranslPairDecl(char* PropName, char* FoName)
/**************************************************************
  INPUT:   A propositional name/first order name pair
  RETURNS: Nothing.
  EFFECT:  Updates the appropriate list.
***************************************************************/
{
  SYMBOL PropSymbol, FoSymbol;
  
  PropSymbol = symbol_Lookup(PropName);
  FoSymbol = symbol_Lookup(FoName);
  
  if (PropSymbol == 0) {
    fprintf(stderr, "Line %u: Undefined symbol %s\n", 
	    token.pos.line, PropName); 
    misc_Error(); 
  }
  else if (FoName == 0) {
    fprintf(stderr, "Line %u: Undefined symbol %s\n", 
	    token.pos.line, FoName); 
    misc_Error(); 
  }
  else 
    eml_SetPropFoSymbolAssocList(PropSymbol,list_List((POINTER)FoSymbol));
  string_StringFree(PropName);  /* Names were copied */
  string_StringFree(FoName);  
}

/**************************************************************/
/* Functions for the Symbol Table                             */
/**************************************************************/

typedef struct {
  SYMBOL symbol;
  BOOL   valid;
  int    arity;
} DFG_SYMENTRY, *DFG_SYM;

DFG_SYM dfg_SymCreate(void)
{
  return (DFG_SYM) memory_Malloc(sizeof(DFG_SYMENTRY));
}

void dfg_SymFree(DFG_SYM Entry)
{
  memory_Free(Entry, sizeof(DFG_SYMENTRY));
}


static void dfg_SymAdd(SYMBOL Symbol)
/**************************************************************
  INPUT:   A symbol.
  RETURNS: Nothing.
  EFFECT:  This function adds 'Symbol' to the symbol list.
           The arity of these symbols will be checked every time
	   the symbol occurs.
***************************************************************/
{
  DFG_SYM newEntry;
  newEntry         = dfg_SymCreate();
  newEntry->symbol = Symbol;
  newEntry->valid  = FALSE;
  newEntry->arity  = 0;
  dfg_SYMBOLLIST   = list_Cons(newEntry, dfg_SYMBOLLIST);
}


static void dfg_SymCheck(SYMBOL Symbol, NAT Arity)
/**************************************************************
  INPUT:   A symbol and the current arity of this symbol.
  RETURNS: Nothing.
  EFFECT:  This function compares the previous arity of 'Symbol'
           with the actual 'Arity'. If these values differ
	   the symbol's arity is set to arbitrary.
	   The arity of symbols whose arity was specified in
	   the symbol declaration section is checked and a warning
	   is printed to stderr in case of differences.
***************************************************************/
{
  LIST scan;

  scan = dfg_SYMBOLLIST;

  while (!list_Empty(scan)) {

    DFG_SYM actEntry;

    actEntry = (DFG_SYM) list_Car(scan);

    if (actEntry->symbol == Symbol) {
      if (actEntry->valid) {
	if (actEntry->arity != Arity) {
	  misc_StartUserErrorReport();
	  dfg_UserErrorReportHeader(& token.pos);
	  misc_UserErrorReport(" The actual arity %u", Arity);
	  misc_UserErrorReport(" of symbol %s differs", symbol_Name(Symbol));
	  misc_UserErrorReport(" from the previous arity %u.\n", actEntry->arity);
	  misc_FinishUserErrorReport();
	}
      } else {
	/* Not valid => first time */
	actEntry->arity = Arity;
	actEntry->valid = TRUE;
      }
      return;
    }
    scan = list_Cdr(scan);
  }

  /* Symbol isn't in SymbolList, so its arity was specified.        */
  /* Check if the specified arity corresponds with the actual arity */
  if (symbol_Arity(Symbol) != Arity) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportHeader(& token.pos);
    misc_UserErrorReport(" Symbol %s was declared with arity %u.\n",
			 symbol_Name(Symbol), symbol_Arity(Symbol));
    misc_FinishUserErrorReport();
  }
}


static void dfg_SymCleanUp(void)
/**************************************************************
  INPUT:   None.
  RETURNS: Nothing.
  EFFECT:  This function corrects all symbols whose arity wasn't
           specified in the symbol declaration section but seem
	   to occur with always the same arity.
	   The memory for the symbol list is freed.
***************************************************************/
{
  while (!list_Empty(dfg_SYMBOLLIST)) {
    DFG_SYM actEntry;
    SYMBOL  actSymbol;
    actEntry = (DFG_SYM) list_Car(dfg_SYMBOLLIST);
    actSymbol = actEntry->symbol;
    if (actEntry->arity != symbol_Arity(actSymbol))
      symbol_SetArity(actSymbol, actEntry->arity);

    dfg_SymFree(actEntry);
    dfg_SYMBOLLIST = list_Pop(dfg_SYMBOLLIST);
  }
}


/**************************************************************/
/* Functions for the Variable Table                           */
/**************************************************************/

 char* dfg_VarName(DFG_VAR Entry)
{
  return Entry->name;
}

 SYMBOL dfg_VarSymbol(DFG_VAR Entry)
{
  return Entry->symbol;
}

 DFG_VAR dfg_VarCreate(void)
{
  return (DFG_VAR) memory_Malloc(sizeof(DFG_VARENTRY));
}

static void dfg_VarFree(DFG_VAR Entry)
{
  string_StringFree(Entry->name);
  memory_Free(Entry, sizeof(DFG_VARENTRY));
}

static void dfg_VarStart(void)
{
  dfg_VARLIST   = list_Push(list_Nil(),
			    dfg_VARLIST);
  dfg_VARDECL   = TRUE;
}

static void dfg_VarStop(void)
{
  dfg_VARDECL = FALSE;
}

static void dfg_VarBacktrack(void)
{
  LIST    undo_l;
  DFG_VAR entry;

  undo_l      = (LIST)list_Top(dfg_VARLIST);
  dfg_VARLIST = list_Pop(dfg_VARLIST);

  while(!list_Empty(undo_l)){
    entry = (DFG_VAR)list_Top(undo_l);
    undo_l     = list_Pop(undo_l);
    hm_Remove(dfg_VARSCOPE,(POINTER)dfg_VarName(entry));
    dfg_VarFree(entry);
  }
}

static void dfg_VarCheck(void)
/* Should be called after a complete clause or formula was parsed */
{
  if (!list_Empty(dfg_VARLIST)) {
    misc_StartErrorReport();
    dfg_UserErrorReportHeader(& token.pos);
    misc_ErrorReport("\n In dfg_VarCheck: List of variables should be empty!\n");
    misc_FinishErrorReport();
  }
  symbol_ResetStandardVarCounter();
}

static SYMBOL dfg_VarLookup(char* Name, pos_t pos_sym)
/**************************************************************
  INPUT:   A variable name and its sort or 0 if it is
           a quantified variable.
  RETURNS: The corresponding variable symbol.
  EFFECT:  If the variable name was quantified before, and the sort
           is the same, the corresponding symbol is returned and the
           <Name> is freed.
	   If the variable name was not quantified, and <dfg_VARDECL>
	   is TRUE, a new variable is created, else an error
	   message is printed and the program exits.
***************************************************************/
{
  SYMBOL  symbol;
  DFG_VAR var_E;

  symbol = symbol_Null();

  var_E = (DFG_VAR)hm_Retrieve(dfg_VARSCOPE,(POINTER)Name);

  if (var_E != (DFG_VAR)NULL) {
    /* Found variable */
    string_StringFree(Name);
    symbol = dfg_VarSymbol(var_E);
  } else {
    /* Variable not found */
    if (dfg_VARDECL) {
      DFG_VAR newEntry = dfg_VarCreate();
      newEntry->name   = Name;
      newEntry->symbol = symbol_CreateStandardVariable();
      /* Add <newentry> to the first list in dfg_VARLIST */
      list_Rplaca(dfg_VARLIST, list_Cons(newEntry,list_Car(dfg_VARLIST)));
      /* Add <newentry> to the scope */
      hm_Insert(dfg_VARSCOPE,(POINTER)Name,(POINTER)newEntry);
      symbol = dfg_VarSymbol(newEntry);
    }else{
      misc_StartUserErrorReport();
      dfg_UserErrorReportF(&pos_sym,"Free Variable %s.", Name);
      misc_FinishUserErrorReport();
    }
  }
  return symbol;
}


/**************************************************************/
/* Functions for the plain clauses                            */
/**************************************************************/
static DFG_PLAINCLAUSE dfg_PlainClauseCreate(void)
{
  return (DFG_PLAINCLAUSE) memory_Malloc(sizeof(DFG_PLAINCLAUSEHELP));
}

static void dfg_PlainClauseFree(DFG_PLAINCLAUSE Clause)
{
  list_Delete(Clause->constraint);
  list_Delete(Clause->antecedent);
  list_Delete(Clause->succedent);
  memory_Free(Clause, sizeof(DFG_PLAINCLAUSEHELP));
}



/**************************************************************/
/* Core Functions                                             */
/**************************************************************/

void dfg_ParserInit(void)
{
  string_table_init();
  lexer_init();
}

void dfg_parse_translation_unit(void);


void dfg_parse(FILE* file, char* name)
/**************************************************************
  INPUT:   A file and its name.
  EFFECT:  Runs the parser on the file and fills up the global
           parser variables. 
**************************************************************/
{
  lexer_set_input(file, name);
  dfg_parse_translation_unit();
  if(dfg_CurrentFilename != NULL){
    string_StringFree(dfg_CurrentFilename);
    dfg_CurrentFilename = NULL;
  }
}

 
static BOOL dfg_accept(token_kind_t const kind)
/**************************************************************
  INPUT:   A token kind.
  EFFECT:  If the current token was of the given kind the lexer
           is set to the next token.
  RETURNS: TRUE if the current token was of the given kind.
**************************************************************/
{
  if (token.kind == kind) {
    lexer_next();
    return TRUE;
  } else {
    return FALSE;
  }
}

static void dfg_expectError(char const* const expected)
/**************************************************************
  INPUT:   A string describing which token was expected.
  EFFECT:  Prints an error.
**************************************************************/
{
  misc_StartUserErrorReport();
  dfg_UserErrorReportF(&token.pos, "got '%s', expected %s", token.text, expected);
  misc_FinishUserErrorReport();
}


static void dfg_expect(token_kind_t const kind)
/**************************************************************
  INPUT:   The expected token kind.
  EFFECT:  If the current token is not of the expected kind an
           error is issued. If the current token is of the
           expected kind it is consumed and the lexer is set
           to the next token.
**************************************************************/
{
  if (!dfg_accept(kind)) {
    dfg_expectError(token_kind2str(kind));
  }
}

static char const* dfg_expectAndReturnString(token_kind_t const kind)
/**************************************************************
  INPUT:   The expected token kind.
  EFFECT:  If the current token is not of the expected kind an
           error is issued. If the current token is of the
           expected kind it is consumed and the lexer is set
           to the next token.
  RETURNS: A pointer to the text of the current token.
**************************************************************/
{
  if (token.kind == kind) {
    char const* ret_string = token.text;
    lexer_next();
    return ret_string;
  } else {
    dfg_expectError(token_kind2str(kind));
  }
  return NULL;
}


static BOOL dfg_acceptId(void)
/**************************************************************
  EFFECT:  If the current token was of the given kind the lexer
           is set to the next token.
  RETURNS: TRUE if the current token was of the given kind.
**************************************************************/
{
  if (token.kind == T_IDENTIFIER || token.kind == T_NUMBER) {
    lexer_next();
    return TRUE;
  } else {
    return FALSE;
  }
}

static void dfg_expectId(void)
/**************************************************************
  EFFECT:  If the current token is not of the expected kind an
           error is issued. If the current token is of the
           expected kind it is consumed and the lexer is set
           to the next token.
**************************************************************/
{
  if (!dfg_acceptId()) {
    dfg_expectError(token_kind2str(T_IDENTIFIER));
  }
}

static char const* dfg_expectIdAndReturnString(void)
/**************************************************************
  EFFECT:  If the current token is not an identifier or a number 
           an error is issued. Otherwise the token text is 
           returned!
  RETURNS: A pointer to the text of the current token.
**************************************************************/
{
  if (token.kind == T_IDENTIFIER || token.kind == T_NUMBER) {
    char const* ret_string = token.text;
    lexer_next();
    return ret_string;
  } else {
    dfg_expectError(token_kind2str(T_IDENTIFIER));
  }
  return NULL;
}


static intptr_t dfg_expectAndReturnNumber(token_kind_t const kind)
/**************************************************************
  INPUT:   The expected token kind.
  EFFECT:  If the current token is not of the expected kind an
           error is issued. If the current token is of the
           expected kind it is consumed and the lexer is set
           to the next token.
  RETURNS: The text of the current token transformed into an int.
**************************************************************/
{
  if (token.kind == kind) {
    intptr_t ret_int;
    if(!string_StringToInt(token.text, false, &ret_int)){
      ret_int = 0;
      /*
      misc_StartUserErrorReport();
      dfg_UserErrorReportF(&token.pos, "got '%s', but expected this to be a number", token.text);
      misc_FinishUserErrorReport();*/
    }
    lexer_next();
    return ret_int;
  } else {
    dfg_expectError(token_kind2str(kind));
  }
  return 0;
}


static BOOL dfg_acceptKeyPeriod(token_kind_t const key)
/**************************************************************
  INPUT:   The expected token kind.
  EFFECT:  If the current token is of the expected kind,
           it has to be followed by a period or an error is
           issued. The current token and the next token
           representing the period are both consumed.
  RETURNS: TRUE if the current token is of the expected kind
           and followed by a period.
**************************************************************/
{
  if (dfg_accept(key)) {
    dfg_expect(T_PERIOD);
    return TRUE;
  } else {
    return FALSE;
  }
}

static void dfg_expectKeyPeriod(token_kind_t const key)
/**************************************************************
  INPUT:   The expected token kind.
  EFFECT:  The current token has to be of the expected kind and
           it has to be followed by a period or else an error is
           issued. The current token and the next token
           representing the period are both consumed.
**************************************************************/
{
  dfg_expect(key);
  dfg_expect(T_PERIOD);
}

static char* dfg_acceptKeyText(token_kind_t const key)
/**************************************************************
  INPUT:   The expected token kind.
  EFFECT:  If the current token is of the expected kind,
           it has to be followed by a dfg text, i.e. {*<text>*}.
           or an error is issued. The current token and the
           tokens necessary for {*<text>*} are consumed.
  RETURNS: The string <text>.
**************************************************************/
{
  char* string;
  if (dfg_accept(key)) {
    dfg_expect(T_LPAREN);
    if(token.kind == T_LPARSTAR){
      lexer_lextext();
      string = string_StringCopy(token.text);
      lexer_next();
    }else{
      dfg_expectError(token_kind2str(T_LPARSTAR));
      string = NULL;
    }
    dfg_expectKeyPeriod(T_RPAREN);
    return string;
  }else{
    return NULL;
  }
}

static char* dfg_expectKeyText(token_kind_t const key)
/**************************************************************
  INPUT:   The expected token kind.
  EFFECT:  The current token has to be of the expected kind and
           it has to be followed by a dfg text, i.e. {*<text>*}.
           or an error is issued. The current token and the
           tokens necessary for {*<text>*} are consumed.
  RETURNS: The string <text>.
**************************************************************/
{
  char* string;
  dfg_expect(key);
  dfg_expect(T_LPAREN);
  if(token.kind == T_LPARSTAR){
    lexer_lextext();
    string = string_StringCopy(token.text);
    lexer_next();
  }else{
    dfg_expectError(token_kind2str(T_LPARSTAR));
    string = NULL;
  }
  dfg_expectKeyPeriod(T_RPAREN);
  return string;
}

static DFG_STATE dfg_parse_log_state(void)
/***************************************************************
  EFFECTS: Parse the problem status.
  RETURNS: The problem status.
  RULE:    ::= satisfiable | unsatisfiable | unknown
***************************************************************/
{
  switch (token.kind) {
  case T_satisfiable:
    lexer_next();
    return DFG_SATISFIABLE;
  case T_unsatisfiable:
    lexer_next();
    return DFG_UNSATISFIABLE;
  case T_unknown:
    lexer_next();
    return DFG_UNKNOWNSTATE;
  default:
    dfg_expectError("log state");
    return DFG_UNKNOWNSTATE;
  }
}

static void dfg_parse_description(void)
/***************************************************************
  EFFECTS: Parse and create a description for the current problem!
  RULE:    ::= list_of_descriptions.
               name({* <name> *}).
               author({* <author> *}).
               { version({* <version> *}). }
               { logic({* <logic> *}). }
               status(dfg_parse_log_state).
               description({* <description> *}).
               { date({* <date> *}). }
               end_of_list.
***************************************************************/
{
  dfg_expectKeyPeriod(T_list_of_descriptions);
  dfg_DESC->name        = dfg_expectKeyText(T_name);
  dfg_DESC->author      = dfg_expectKeyText(T_author);
  dfg_DESC->version     = dfg_acceptKeyText(T_version);
  dfg_acceptKeyText(T_logic);
  dfg_expect(T_status);
  dfg_expect(T_LPAREN);
  dfg_DESC->status      = dfg_parse_log_state();
  dfg_expectKeyPeriod(T_RPAREN);
  dfg_DESC->description = dfg_expectKeyText(T_description);
  dfg_DESC->date        = dfg_acceptKeyText(T_date);
  dfg_expectKeyPeriod(T_end_of_list);
}

static void dfg_parse_and_create_sort_sym(void)
/***************************************************************
  EFFECTS: Parse and declare a sort symbol.
           <sort_name> is the name of the sort. 
           The sort is internally realized as a unary predicate!
  RULE:    ::= <sort_name>
  WARNING: <sort_name> must be undeclared and not a keyword
***************************************************************/
{
  char* sort_name = NULL;
  switch (token.kind) {
  case T_IDENTIFIER:
  case T_NUMBER:
    sort_name = string_StringCopy(dfg_expectIdAndReturnString());
    dfg_SymbolDecl(DFG_PRDICAT, sort_name, 1);
    break;
  default:
    dfg_expectError("sort sym");
    break;
  }
}

static void dfg_parse_and_create_fun_sym(void)
/***************************************************************
  EFFECTS: Parse and declare a function symbol.
           <fun_name> is the name of the function.
           The function's arity is either <arity> or it will be
           determined when it is used the first time! 
  RULE:    ::= <fun_name> | (<fun_name>,<arity>) 
  WARNING: <fun_name> must be undeclared and not a keyword
***************************************************************/
{
  char* fun_name;
  switch(token.kind){
  case T_IDENTIFIER:
  case T_NUMBER:
    fun_name = string_StringCopy(dfg_expectIdAndReturnString());
    dfg_SymbolDecl(DFG_FUNC, fun_name, -2);
    break;
  case T_LPAREN:
    lexer_next();
    fun_name       = string_StringCopy(dfg_expectIdAndReturnString());
    dfg_expect(T_COMMA);
    intptr_t arity = dfg_expectAndReturnNumber(T_NUMBER);
    dfg_expect(T_RPAREN);
    dfg_SymbolDecl(DFG_FUNC, fun_name, arity);
    break;
  default:
    dfg_expectError("identifier");
  }
}

static SYMBOL dfg_parse_fun_sym_or_var(void)
/***************************************************************
  EFFECTS: Parse a function or variable name and find the
           corresponding symbol.
  RETURNS: The symbol declared for <name>
  RULE:    ::= <name>
  WARNING: <name> must be a declared function symbol or a 
           variable name inside the variable scope!
           Otherwise ERROR!
***************************************************************/
{
  pos_t  start_position;
  char const*  fun_name;
  SYMBOL fun_sym;
  start_position = token.pos;
  fun_name       = dfg_expectIdAndReturnString();
  fun_sym        = symbol_Lookup(fun_name);
  if (fun_sym == 0) {
    fun_sym = dfg_VarLookup(string_StringCopy(fun_name),start_position);
  }
  if (!symbol_IsFunction(fun_sym) && !symbol_IsVariable(fun_sym)) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportF(&start_position,"Symbol %s is declared but not as a function or variable as required.", fun_name);
    misc_FinishUserErrorReport();
  }
  return fun_sym;
}


static void dfg_parse_and_create_pred_sym(void)
/***************************************************************
  EFFECTS: Parse and declare a predicate symbol.
           <pred_name> is the name of the predicate.
           The predicate's arity is either <arity> or it will be
           determined when it is used the first time! 
  RULE:    ::= <pred_name> | (<pred_name>,<arity>) 
  WARNING: <pred_name> must be undeclared and not a keyword
***************************************************************/
{
  char* pred_name;
  switch(token.kind){
  case T_IDENTIFIER:
  case T_NUMBER:
    pred_name = string_StringCopy(dfg_expectIdAndReturnString());
    dfg_SymbolDecl(DFG_PRDICAT, pred_name, -2);
    break;
  case T_LPAREN:
    lexer_next();
    pred_name      = string_StringCopy(dfg_expectIdAndReturnString());
    dfg_expect(T_COMMA);
    intptr_t arity = dfg_expectAndReturnNumber(T_NUMBER);
    dfg_expect(T_RPAREN);
    dfg_SymbolDecl(DFG_PRDICAT, pred_name, arity);
    break;
  default:
    dfg_expectError("identifier");
  }
}

static SYMBOL dfg_parse_declared_pred_sym(void)
/***************************************************************
  EFFECTS: Parse a predicate name and find the corresponding symbol.
  RETURNS: The predicate symbol declared for <pred_name>
  RULE:    ::= <pred_name>
  WARNING: <pred_name> must be a declared predicate symbol!
           Otherwise ERROR!
***************************************************************/
{
  pos_t  start_position = token.pos;
  char const* pred_name = dfg_expectIdAndReturnString();
  SYMBOL pred_sym       = symbol_Lookup(pred_name);
  if (pred_sym == 0) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportF(&start_position,"Undefined symbol %s. Should be a predicate symbol.", pred_name);
    misc_FinishUserErrorReport();
  }
  if (!symbol_IsPredicate(pred_sym)) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportF(&start_position,"Symbol %s is declared but not as a predicate as it should be.", pred_name);
    misc_FinishUserErrorReport();
  }
  return pred_sym;
}


static void dfg_parse_and_create_operator_sym(void)
/***************************************************************
  EFFECTS: Parse and declare an operator symbol.
           <op_name> is the name of the operator.
           The operator's arity is either <arity> or it will be
           determined when it is used the first time! 
  RULE:    ::= <op_name> | (<op_name>,<arity>) 
  WARNING: <op_name> must be undeclared and not a keyword
           Although an operator symbol can be declared, it can't 
           be used in formulae.
***************************************************************/
{
  char* pred_name;
  switch(token.kind){
  case T_IDENTIFIER:
  case T_NUMBER:
    pred_name = string_StringCopy(dfg_expectIdAndReturnString());
    dfg_SymbolDecl(DFG_OPERAT, pred_name, -2);
    break;
  case T_LPAREN:
    lexer_next();
    pred_name      = string_StringCopy(dfg_expectIdAndReturnString());
    dfg_expect(T_COMMA);
    intptr_t arity = dfg_expectAndReturnNumber(T_NUMBER);
    dfg_expect(T_RPAREN);
    dfg_SymbolDecl(DFG_OPERAT, pred_name, arity);
    break;
  default:
    dfg_expectError("identifier");
  }
}


static void dfg_parse_and_create_quantifier_sym(void)
/***************************************************************
  EFFECTS: Parse and declare a quantifier symbol.
           <quant_name> is the name of the quantifier.
           The quantifier's arity is either <arity> or it will be
           determined when it is used the first time! 
  RULE:    ::= <quant_name> | (<quant_name>,<arity>) 
  WARNING: <quant_name> must be undeclared and not a keyword
           Although a quantifier symbol can be declared, it can't 
           be used in formulae.
***************************************************************/
{
  char* pred_name;
  switch(token.kind){
  case T_IDENTIFIER:
  case T_NUMBER:
    pred_name = string_StringCopy(dfg_expectIdAndReturnString());
    dfg_SymbolDecl(DFG_QUANTIF, pred_name, -2);
    break;
  case T_LPAREN:
    lexer_next();
    pred_name      = string_StringCopy(dfg_expectIdAndReturnString());
    dfg_expect(T_COMMA);
    intptr_t arity = dfg_expectAndReturnNumber(T_NUMBER);
    dfg_expect(T_RPAREN);
    dfg_SymbolDecl(DFG_QUANTIF, pred_name, arity);
    break;
  default:
    dfg_expectError("identifier");
  }
}


static void dfg_parse_symbol_list(void)
/***************************************************************
  EFFECTS: Parse and declare a list of symbols.
  RULE:    ::= list_of_symbols.
               {functions[ dfg_parse_and_create_fun_sym {, dfg_parse_and_create_fun_sym } ].}
               {predicates[ dfg_parse_and_create_pred_sym {, dfg_parse_and_create_pred_sym } ].}
               {sorts[ dfg_parse_and_create_sort_sym {, dfg_parse_and_create_sort_sym } ].}
               {operators[ dfg_parse_and_create_operator_sym {, dfg_parse_and_create_operator_sym } ].}
               {quantifiers[ dfg_parse_and_create_quantifier_sym {, dfg_parse_and_create_quantifier_sym } ].}
               {translpairs[ (ID,ID) {, (ID,ID) } ].}
               end_of_list.
***************************************************************/
{
  if (dfg_acceptKeyPeriod(T_list_of_symbols)) {
    if (dfg_accept(T_functions)) {
      dfg_expect(T_LBRACKET);
      do {
	dfg_parse_and_create_fun_sym();
      } while (dfg_accept(T_COMMA));
      dfg_expectKeyPeriod(T_RBRACKET);
    }

    if (dfg_accept(T_predicates)) {
      dfg_expect(T_LBRACKET);
      do {
	dfg_parse_and_create_pred_sym();
      } while (dfg_accept(T_COMMA));
      dfg_expectKeyPeriod(T_RBRACKET);
    }

    if (dfg_accept(T_sorts)) {
      dfg_expect(T_LBRACKET);
      do {
	dfg_parse_and_create_sort_sym();
      } while (dfg_accept(T_COMMA));
      dfg_expectKeyPeriod(T_RBRACKET);
    }

    if (dfg_accept(T_operators)) {
      dfg_expect(T_LBRACKET);
      do {
	dfg_parse_and_create_operator_sym();
      } while (dfg_accept(T_COMMA));
      dfg_expectKeyPeriod(T_RBRACKET);
    }

    if (dfg_accept(T_quantifiers)) {
      dfg_expect(T_LBRACKET);
      do {
	dfg_parse_and_create_quantifier_sym();
      } while (dfg_accept(T_COMMA));
      dfg_expectKeyPeriod(T_RBRACKET);
    }

    if (dfg_accept(T_translpairs)) {
      dfg_expect(T_LBRACKET);
      do {
	char* first_name;
	char* second_name;
	dfg_expect(T_LPAREN);
	first_name = string_StringCopy(dfg_expectIdAndReturnString());
	dfg_expect(T_COMMA);
	second_name= string_StringCopy(dfg_expectIdAndReturnString());
	dfg_expect(T_RPAREN);
	dfg_TranslPairDecl(first_name, second_name);
      } while (dfg_accept(T_COMMA));
      dfg_expectKeyPeriod(T_RBRACKET);
    }

    dfg_expectKeyPeriod(T_end_of_list);
  }
}


static void dfg_parse_gensort_declaration(void)
/***************************************************************
  EFFECTS: Parse a sort generator declaration, i.e.,
           the list of functions parsed are the generators for 
           the sort <sort>.
  RULE:    ::= sort(<sort>, <function> {, <function>} ).
  WARNING: <sort> must be declared as a sort symbol and
           <function>s must be declared as function symbols.
***************************************************************/
{
  char*  first_name;
  char*  second_name;
  LIST   func_list;
  BOOL   isFreely;
  pos_t  pos_sort, pos_funct;

  dfg_expect(T_sort);

  pos_sort = token.pos;

  first_name = string_StringCopy(dfg_expectIdAndReturnString());

  isFreely = dfg_accept(T_freely);

  pos_funct = token.pos;

  dfg_expect(T_generated);
  dfg_expect(T_by);

  dfg_expect(T_LBRACKET);

  func_list = list_Nil();

  do {
    second_name = string_StringCopy(dfg_expectIdAndReturnString());
    func_list = list_Cons((POINTER)second_name,func_list);
  } while (dfg_accept(T_COMMA));
  dfg_expect(T_RBRACKET);

  dfg_SymbolGenerated(dfg_Symbol(first_name,1), isFreely, func_list,
		      pos_sort, pos_funct);
}


static void dfg_parse_subsort_declaration(void)
/***************************************************************
  EFFECTS: Parse a subsort declaration (sort1 is subsort of sort2).
           This functions adds the formula
           forall([U], implies(<sort1>(U), <sort2>(U)))
	   to the list of axiom formulas
  RULE:    ::= subsort(<sort1>,<sort2>).
  WARNING: <sort1> and <sort2> must be declared as sorts.
***************************************************************/
{
  char* first_name;
  char* second_name;
  pos_t  pos1, pos2;

  dfg_expect(T_subsort);

  dfg_expect(T_LPAREN);

  pos1 = token.pos;
  first_name = string_StringCopy(dfg_expectIdAndReturnString());

  dfg_expect(T_COMMA);

  pos2 = token.pos;
  second_name = string_StringCopy(dfg_expectIdAndReturnString());

  dfg_expectKeyPeriod(T_RPAREN);

  dfg_SubSort(first_name,second_name,pos1,pos2);
}

static void dfg_parse_predicate_declaration()
/***************************************************************
  EFFECTS: Parse a predicate declaration. This has no effect on
           SPASS.
  RULE:    ::= predicate(ID,ID).
***************************************************************/
{
  dfg_expect(T_predicate);
  dfg_expect(T_LPAREN);
  dfg_expectId();
  dfg_expect(T_COMMA);

  do {
    dfg_expectId();
  } while (dfg_accept(T_COMMA));
  dfg_expectKeyPeriod(T_RPAREN);
}

static LIST dfg_parse_qvar_list(void);
static TERM dfg_parse_atom(void);

static void dfg_parse_forall_declaration()
/***************************************************************
  EFFECTS: Parse a forall declaration. The forall declaration is 
           added as a TERM to dfg_DECLARATIONS.
  RULE:    ::=  forall( dfg_parse_qvar_list , dfg_parse_atom ).
***************************************************************/
{
  TERM term;
  LIST qvar_list;

  dfg_expect(T_forall);
  dfg_expect(T_LPAREN);
  qvar_list     = dfg_parse_qvar_list();
  dfg_expect(T_COMMA);
  term = dfg_parse_atom();
  dfg_expectKeyPeriod(T_RPAREN);

  term = dfg_CreateQuantifier(fol_All(),qvar_list,term);
  dfg_DECLARATIONS = array_Add(dfg_DECLARATIONS,
			       (intptr_t)list_PairCreate(NULL,term));
  dfg_VarBacktrack();
  dfg_VarCheck();
}

static void dfg_parse_atom_declaration()
/***************************************************************
  EFFECTS: Parse an atom declaration. The atom is added as a TERM
           to dfg_DECLARATIONS.
  RULE:    ::=  dfg_parse_atom .
***************************************************************/
{
  TERM term;
  term = dfg_parse_atom();
  dfg_expect(T_PERIOD);
  dfg_DECLARATIONS = array_Add(dfg_DECLARATIONS,
			       (intptr_t)list_PairCreate(NULL,term));
}


static void dfg_parse_declaration_list(void)
/***************************************************************
  EFFECTS: Parse a list of declarations.
  RULE:    ::=  {list_of_declarations.
	         { dfg_parse_gensort_declaration |
	           dfg_parse_subsort_declaration |
	           dfg_parse_predicate_declaration |
	           dfg_parse_forall_declaration |
	           dfg_parse_atom_declaration }^*
                 end_of_list.}
***************************************************************/
{
  if (dfg_acceptKeyPeriod(T_list_of_declarations)) {
    for (;;) {
      switch (token.kind) {
      case T_sort:
	dfg_parse_gensort_declaration();
	break;

      case T_subsort:
	dfg_parse_subsort_declaration();
	break;

      case T_predicate:
	dfg_parse_predicate_declaration();
	break;

      case T_forall:
	dfg_parse_forall_declaration();
	break;

      case T_end_of_list:
	dfg_expectKeyPeriod(T_end_of_list);
	return;

      default:
	dfg_parse_atom_declaration();
      }
    }
  }
}

static BOOL dfg_parse_origin_type_is_axioms(void)
/***************************************************************
  EFFECTS: Parse whether the formulae list is a list of axioms
           or conjectures.
  RETURNS: TRUE iff the list is a list of axioms
  RULE:    ::= axioms | conjectures
***************************************************************/
{
  switch (token.kind) {
  case T_axioms:
    lexer_next();
    return TRUE;
  case T_conjectures:
    lexer_next();
    return FALSE;
  default:
    dfg_expectError("origin type ('axioms' or 'conjectures')");
  }
  return FALSE;
}

static TERM dfg_parse_qvar(void)
/***************************************************************
  EFFECTS: Parse a (sorted) variable as argument for
           a quantifier.
  RETURNS: The parsed variable as a TERM
  RULE:    ::= <variable> | <sort>(<variable>)
  WARNING: <sort> must correspond to a declared sort symbol.
           <variable> must not be declared as a symbol.
           <variable> can already be in the scope of variables!
           If this is the case, the last declaration is ignored
           until this new scope closes.
***************************************************************/
{
  pos_t  start_position = token.pos;
  SYMBOL sort_sym, var_sym;
  char*  var_name;
  char*  sort_name;

  if (!dfg_IGNORE) {
    var_name = string_StringCopy(dfg_expectIdAndReturnString());
  }else{
    dfg_expectId();
    var_name = NULL;
  }
  
  if (dfg_accept(T_LPAREN)) {
    if (!dfg_IGNORE) {
      sort_name = var_name;
      sort_sym  = dfg_Symbol(sort_name,1);
      if (!symbol_IsPredicate(sort_sym)) {
	misc_StartUserErrorReport();
	dfg_UserErrorReportF(&start_position,"Symbol is not a sort.");
	misc_FinishUserErrorReport();
      } 
    }else{
      sort_sym = 0;
    }

    start_position = token.pos;
    if (!dfg_IGNORE) {
      var_name  = string_StringCopy(dfg_expectIdAndReturnString());
      var_sym   = dfg_Symbol(var_name,0);
      if (!symbol_IsVariable(var_sym)) {
	misc_StartUserErrorReport();
	dfg_UserErrorReportF(&start_position,"Symbol is not a variable.");
	misc_FinishUserErrorReport();
      }
    }else{
      dfg_expectId();
      var_sym = 0;
    }

    dfg_expect(T_RPAREN);

    if (!dfg_IGNORE) {      
      return term_Create(sort_sym, list_List(term_Create(var_sym,list_Nil())));
    }
  }else{
    if (!dfg_IGNORE) {
      var_sym   = dfg_Symbol(var_name,0);
      if (!symbol_IsVariable(var_sym)) {
	misc_StartUserErrorReport();
	dfg_UserErrorReportF(&start_position,"Symbol is not a variable.");
	misc_FinishUserErrorReport();
      }
    }else{
      var_sym = 0;
    }

    if (!dfg_IGNORE) {
      return term_Create(var_sym, list_Nil());
    }
  }

  return 0;
}

static LIST dfg_parse_qvar_list(void)
/***************************************************************
  EFFECTS: Parse a list of (sorted) variables as argument for
           a quantifier.
  RETURNS: The parsed variables as a LIST of TERMs
  RULE:    ::= [ dfg_parse_qvar {, dfg_parse_qvar}^* ]
  WARNING: Every call of this function opens a new variable scope!
           The scope has to be closed externally with dfg_VarBacktrack()!
***************************************************************/
{
  TERM   var_term;
  LIST   list1;

  dfg_expect(T_LBRACKET);

  dfg_VarStart();
  list1 = list_Nil();
  do {
    var_term = dfg_parse_qvar();
    list1    = list_Cons((POINTER)var_term,list1);
  } while (dfg_accept(T_COMMA));
  dfg_expect(T_RBRACKET);

  dfg_VarStop();

  return list1;
}

static TERM dfg_parse_term(void)
/***************************************************************
  EFFECTS: Parse a term
  RETURNS: Parsed term as a TERM
  RULE:    ::= ID( dfg_parse_term {, dfg_parse_term}^* ) |
               ID
  WARNING: ID must correspond to an existing function symbol
***************************************************************/
{
  pos_t*   intermed_pos;
  pos_t    help_pos;
  TERM     current_TOS, arg_term;
  SYMBOL   fun_sym, TOS_top_sym;
  LIST     arg_list, pair;
  ARRAY    term_stack;
  BOOL     Overjump;

  Overjump = FALSE;

  term_stack  = dfg_TERM_STACK;
  current_TOS = (TERM)NULL;

  do{
    switch (token.kind) {
    case T_IDENTIFIER:
    case T_NUMBER:
      if(!Overjump){
	help_pos      = token.pos;
	fun_sym       = dfg_parse_fun_sym_or_var();
	if(symbol_IsVariable(fun_sym)){
	  arg_term = term_Create(fun_sym,list_Nil());
	}else if(dfg_accept(T_LPAREN)){
	  current_TOS   = term_Create(fun_sym,list_Nil());
	  fun_sym       = 0;
	  intermed_pos  = (pos_t*)memory_Malloc(sizeof(pos_t));
	  *intermed_pos = help_pos;
	  pair          = list_PairCreate((POINTER)current_TOS,
					 (POINTER)intermed_pos);
	  term_stack    = array_Add(term_stack,(intptr_t)pair);
	  break;
	}else{
	  dfg_SymCheck(fun_sym,0);
	  arg_term = term_Create(fun_sym,list_Nil());
	}
      }
      if(array_GetSize(term_stack) == 0){
	dfg_TERM_STACK = term_stack;
	if(dfg_IGNORE){
	  term_Delete(arg_term);
	  return NULL;
	}
	return arg_term;
      }
      
      TOS_top_sym = term_TopSymbol(current_TOS);
      arg_list    = term_ArgumentList(current_TOS);
      term_RplacArgumentList(current_TOS,list_Cons(arg_term,arg_list));
      Overjump = FALSE;
    case T_RPAREN:
      while(token.kind == T_RPAREN){
	help_pos = token.pos;
	lexer_next();
	
	TOS_top_sym  = term_TopSymbol(current_TOS);
	arg_list     = term_ArgumentList(current_TOS);

	arg_term    = current_TOS;	  
	pair        = (LIST)array_Pop(term_stack);
	memory_Free(list_PairSecond(pair),sizeof(pos_t));
	list_PairFree(pair);
	term_RplacArgumentList(arg_term,list_NReverse(arg_list));

	arg_list = term_ArgumentList(arg_term);
	dfg_SymCheck(term_TopSymbol(arg_term),list_Length(arg_list));

	if(array_GetSize(term_stack) == 0){
	  dfg_TERM_STACK = term_stack;
	  if(dfg_IGNORE){
	    term_Delete(arg_term);
	    return NULL;
	  }
	  return arg_term;
	}
	  
	current_TOS = (TERM)list_PairFirst((LIST)array_Top(term_stack));
	TOS_top_sym = term_TopSymbol(current_TOS);
	arg_list    = term_ArgumentList(current_TOS);
	term_RplacArgumentList(current_TOS,list_Cons(arg_term,arg_list));
      }
      help_pos = token.pos;
      if(!dfg_accept(T_COMMA)){
	TOS_top_sym  = term_TopSymbol(current_TOS);
	arg_list     = term_ArgumentList(current_TOS);
	pair         = (LIST)array_Top(term_stack);
	misc_StartUserErrorReport();
	dfg_UserErrorReportF((pos_t*)list_PairSecond(pair), "expected ',' or ')' at position (line %d,column %d) to conclude parsing of term '%s' but found '%s'", help_pos.line, help_pos.col, symbol_Arity(TOS_top_sym) - list_Length(arg_list), symbol_Name(TOS_top_sym), token.text);
	misc_FinishUserErrorReport();
      }
      
      TOS_top_sym  = term_TopSymbol(current_TOS);
      arg_list     = term_ArgumentList(current_TOS);
      break;
    default:
      dfg_expectError("term (constant, variable, or function(...))");
      break;
    }
  }while(array_GetSize(term_stack) > 0);
    /*This point should never be reached!*/
    misc_StartUserErrorReport();
  misc_UserErrorReport("Unreachable position in dfg_parse_term reached! Probably error in parser itself.");
  misc_FinishUserErrorReport();
  return NULL;
}

static TERM dfg_parse_atom(void)
/***************************************************************
  EFFECTS: Parse an atom
  RETURNS: Parsed atom as a TERM
  RULE:    ::= true |
               false |
               equal( dfg_parse_term , dfg_parse_term ) |
               ID( dfg_parse_term {, dfg_parse_term}^* ) |
               ID
  WARNING: ID must correspond to an existing predicate symbol
***************************************************************/
{
  TERM         arg1, arg2, atom;
  SYMBOL       pred_sym;
  LIST         list1;
  int          arity;
  pred_sym      = 0;
  switch(token.kind){
  case T_equal:
    pred_sym = fol_Equality();
    lexer_next();
    dfg_expect(T_LPAREN);
    arg1 = dfg_parse_term();
    dfg_expect(T_COMMA);
    arg2 = dfg_parse_term();
    dfg_expect(T_RPAREN);
    if(!dfg_IGNORE){
      atom = term_Create(pred_sym, list_Cons(arg1,list_List(arg2)));
    }else{
      atom = NULL;
    }
    return atom;
    break;
  case T_true:
    lexer_next();
    if(!dfg_IGNORE){
      atom = term_Create(fol_True(), list_Nil());
    }else{
      atom = NULL;
    }
    return atom;
  case T_false:
    lexer_next();
    if(!dfg_IGNORE){
      atom = term_Create(fol_False(), list_Nil());
    }else{
      atom = NULL;
    }
    return atom;
  case T_IDENTIFIER:
  case T_NUMBER:
    pred_sym = dfg_parse_declared_pred_sym();
    list1 = list_Nil();
    if(dfg_accept(T_LPAREN)){
      do{
	arg1 = dfg_parse_term();
	if(!dfg_IGNORE){
	  list1 = list_Cons(arg1,list1);
	}
      }while(dfg_accept(T_COMMA));
      dfg_expect(T_RPAREN);
      if(!dfg_IGNORE){
	list1 = list_NReverse(list1);
      }
    }
    if(!dfg_IGNORE){
      arity = list_Length(list1);
      dfg_SymCheck(pred_sym,arity);	
    }

    if(dfg_IGNORE){
      return NULL;
    }
    return term_Create(pred_sym,list1);
  default:
    dfg_expectError("atom term (equal(...), true, false, predicate(), etc.)");
    break;
  }
  return NULL;
}

static TERM dfg_parse_literal(void)
/***************************************************************
  EFFECTS: Parse a literal
  RETURNS: Parsed literal as a TERM
  RULE:    ::= not( dfg_parse_atom ) | dfg_parse_atom
***************************************************************/
{
  if(dfg_accept(T_not)){
    dfg_expect(T_LPAREN);
    TERM atom = dfg_parse_atom();
    dfg_expect(T_RPAREN);
    if(!dfg_IGNORE){
      return term_Create(fol_Not(),list_List(atom));
    }else{
      return NULL;
    }
  }else{
    return dfg_parse_atom();
  }
}



static TERM dfg_parse_formula()
/***************************************************************
  EFFECTS: Parse a formula
  RETURNS: Parsed formula as a TERM
  RULE:    ::= forall(dfg_parse_qvar_list, dfg_parse_formula ) |
               exists(dfg_parse_qvar_list, dfg_parse_formula ) |
               or(dfg_parse_formula {, dfg_parse_formula}* ) |
               and(dfg_parse_formula {, dfg_parse_formula}* ) |
               true |
               false |
               not( dfg_parse_formula ) |
               implies( dfg_parse_formula , dfg_parse_formula ) |
               implied( dfg_parse_formula , dfg_parse_formula ) |
               equiv( dfg_parse_formula , dfg_parse_formula ) |
               nand( dfg_parse_formula , dfg_parse_formula ) |
               nor( dfg_parse_formula , dfg_parse_formula ) |
               xor( dfg_parse_formula , dfg_parse_formula ) |
               ID |
               equal( dfg_parse_term , dfg_parse_term ) |
               ID( dfg_parse_term {, dfg_parse_term} )
  WARNING: ID must correspond to an existing predicate symbol
***************************************************************/
{
  pos_t*  intermed_pos;
  pos_t   help_pos;
  TERM    current_TOS, arg_term, help_term;
  SYMBOL  quant_sym, junct_sym, TOS_top_sym;
  LIST    arg_list, qvar_list, pair, help_list;

  ARRAY   formula_stack;  

  formula_stack = dfg_FORMULA_STACK;
  current_TOS   = NULL;

  do{
    switch (token.kind) {
    case T_forall:
      quant_sym = fol_All();
      intermed_pos  = (pos_t*)memory_Malloc(sizeof(pos_t));
      *intermed_pos = token.pos;
      lexer_next();
      dfg_expect(T_LPAREN);
      qvar_list     = dfg_parse_qvar_list();
      dfg_expect(T_COMMA);
      current_TOS   = term_Create(quant_sym,list_List(term_Create(fol_Varlist(),qvar_list)));
      pair          = list_PairCreate((POINTER)current_TOS,
				      (POINTER)intermed_pos);
      formula_stack = array_Add(formula_stack,(intptr_t)pair);
      break;
    case T_exists:
      quant_sym = fol_Exist();
      intermed_pos  = (pos_t*)memory_Malloc(sizeof(pos_t));
      *intermed_pos = token.pos;
      lexer_next();
      dfg_expect(T_LPAREN);
      qvar_list     = dfg_parse_qvar_list();
      dfg_expect(T_COMMA);
      current_TOS   = term_Create(quant_sym,list_List(term_Create(fol_Varlist(),qvar_list)));
      pair          = list_PairCreate((POINTER)current_TOS,
				      (POINTER)intermed_pos);
      formula_stack = array_Add(formula_stack,(intptr_t)pair);
      break;
    case T_not:
    case T_or:
    case T_and:
    case T_implies:
    case T_implied:
    case T_equiv:
    case T_nor:
    case T_nand:
    case T_xor:

      switch(token.kind){
      case T_not:
	junct_sym = fol_Not();
	break;
      case T_or:
	junct_sym = fol_Or();
	break;
      case T_and:
	junct_sym = fol_And();
	break;
      case T_implies:
	junct_sym = fol_Implies();
	break;
      case T_implied:
	junct_sym = fol_Implied();
	break;
      case T_equiv:
	junct_sym = fol_Equiv();
	break;
      case T_nor:
	junct_sym = fol_Nor();
	break;
      case T_nand:
	junct_sym = fol_Nand();
	break;
      case T_xor:
	junct_sym = fol_Xor();
	break;
      default:
	/*should never be reached*/
	break;
      }
      current_TOS   = term_Create(junct_sym,list_Nil());
      junct_sym     = 0;
      intermed_pos  = (pos_t*)memory_Malloc(sizeof(pos_t));
      *intermed_pos = token.pos;
      pair          = list_PairCreate((POINTER)current_TOS,
				      (POINTER)intermed_pos);
      formula_stack = array_Add(formula_stack,(intptr_t)pair);
      lexer_next();
      dfg_expect(T_LPAREN);
      break;
    case T_true:
    case T_false:
    case T_equal:
    case T_IDENTIFIER:
    case T_NUMBER:
      arg_term    = dfg_parse_atom();
      if(array_GetSize(formula_stack) == 0){
	dfg_FORMULA_STACK = formula_stack;
	return arg_term;
      }
      TOS_top_sym = term_TopSymbol(current_TOS);
      arg_list    = term_ArgumentList(current_TOS);
      if(list_Length(arg_list) < symbol_Arity(TOS_top_sym) ||
	 symbol_Arity(TOS_top_sym) == symbol_ArbitraryArity()){
	term_RplacArgumentList(current_TOS,list_Cons(arg_term,arg_list));
      }else{
	pair = (LIST)array_Top(formula_stack);
	misc_StartUserErrorReport();
	dfg_UserErrorReportF((pos_t*)list_PairSecond(pair), "term '%s', has too many arguments!", symbol_Name(TOS_top_sym));
	misc_FinishUserErrorReport();
      }
    case T_RPAREN:
      while(token.kind == T_RPAREN){
	help_pos = token.pos;
	lexer_next();
	
	TOS_top_sym  = term_TopSymbol(current_TOS);
	arg_list     = term_ArgumentList(current_TOS);
	if(list_Length(arg_list) == symbol_Arity(TOS_top_sym) ||
	   symbol_Arity(TOS_top_sym) == symbol_ArbitraryArity()){
	  arg_term    = current_TOS;	  
	  pair        = (LIST)array_Pop(formula_stack);
	  memory_Free(list_PairSecond(pair),sizeof(pos_t));
	  list_PairFree(pair);

	  term_RplacArgumentList(arg_term,list_NReverse(arg_list));

	  if(fol_IsQuantifier(TOS_top_sym)){
	    dfg_VarBacktrack();
	    /*Get quantifier symbol*/
	    quant_sym = term_TopSymbol(arg_term);

	    /*Get formula inside quantifier*/
	    help_list = term_ArgumentList(arg_term);
	    term_RplacArgumentList(arg_term,list_Nil());
	    term_Delete(arg_term);
	    help_term = (TERM)list_Car(help_list);
	    qvar_list = term_ArgumentList(help_term);

	    /*Get quantified variables*/
	    help_list = list_Pop(help_list);
	    arg_term = (TERM)list_Car(help_list);
	    list_Delete(help_list);


	    /*Clean-up rest*/
	    term_RplacArgumentList(help_term,list_Nil());
	    term_Delete(help_term);

	    /*Create actually quantified term (nec. because of sorts)*/
	    arg_term = dfg_CreateQuantifier(quant_sym, qvar_list, arg_term);
	  }
	  if(array_GetSize(formula_stack) == 0){
	    dfg_FORMULA_STACK = formula_stack;
	    return arg_term;
	  }
	  
	  current_TOS = (TERM)list_PairFirst((LIST)array_Top(formula_stack));
	  TOS_top_sym = term_TopSymbol(current_TOS);
	  arg_list    = term_ArgumentList(current_TOS);
	  if( list_Length(arg_list) < symbol_Arity(TOS_top_sym) ||
	      symbol_Arity(TOS_top_sym) == symbol_ArbitraryArity() ){
	    term_RplacArgumentList(current_TOS,list_Cons(arg_term,arg_list));
	  }else{
	    pair = (LIST)array_Top(formula_stack);
	    misc_StartUserErrorReport();
	    dfg_UserErrorReportF((pos_t*)list_PairSecond(pair), "term '%s', has too many arguments!", symbol_Name(TOS_top_sym));
	    misc_FinishUserErrorReport();
	  }
	}else{
	  TOS_top_sym  = term_TopSymbol(current_TOS);
	  pair         = (LIST)array_Top(formula_stack);
	  misc_StartUserErrorReport();
	  dfg_UserErrorReportF((pos_t*)list_PairSecond(pair), "')' at position (line %d,column %d) concluded parsing of term '%s' with %d arguments missing!", help_pos.line, help_pos.col, symbol_Name(TOS_top_sym), symbol_Arity(TOS_top_sym) - list_Length(arg_list));
	  misc_FinishUserErrorReport();
	}
      }

      help_pos = token.pos;
      if(!dfg_accept(T_COMMA)){
	TOS_top_sym  = term_TopSymbol(current_TOS);
	arg_list     = term_ArgumentList(current_TOS);
	pair         = (LIST)array_Top(formula_stack);
	misc_StartUserErrorReport();
	dfg_UserErrorReportF((pos_t*)list_PairSecond(pair), "expected ',' at position (line %d,column %d) and %d more arguments to conclude parsing of term '%s' but found '%s'", help_pos.line, help_pos.col, symbol_Arity(TOS_top_sym) - list_Length(arg_list), symbol_Name(TOS_top_sym), token.text);
	misc_FinishUserErrorReport();
      }

      TOS_top_sym  = term_TopSymbol(current_TOS);
      arg_list     = term_ArgumentList(current_TOS);
      if(list_Length(arg_list) >= symbol_Arity(TOS_top_sym) &&
	 symbol_Arity(TOS_top_sym) != symbol_ArbitraryArity()){
	pair         = (LIST)array_Top(formula_stack);
	misc_StartUserErrorReport();
	dfg_UserErrorReportF((pos_t*)list_PairSecond(pair), "term '%s', has already enough arguments but token ',' at position (line %d,column %d) suggests that more will follow!", symbol_Name(TOS_top_sym), help_pos.line, help_pos.col);
	misc_FinishUserErrorReport();
      }
      break;
    default:
      dfg_expectError("formula term (forall(...), and(...), etc.)");
      break;
    }
  }while(array_GetSize(formula_stack) > 0);
    /*This point should never be reached!*/
    misc_StartUserErrorReport();
  misc_UserErrorReport("Unreachable position in dfg_parse_formula reached! Probably error in parser itself.");
  misc_FinishUserErrorReport();
  return NULL;
}


static char* dfg_parse_label(void)
/***************************************************************
  EFFECTS: Parse a label to identify a formula.
  RETURNS: the label as a string
  RULE:    ::= ID
***************************************************************/
{
  char* label;
  switch (token.kind) {
  case T_IDENTIFIER:
  case T_NUMBER:
    label = string_StringCopy(token.text);
    lexer_next();
    return label;
    break;
  default:
    dfg_expectError("label (identifier or number)");
    break;
  }
  return NULL;
}

static void dfg_parse_formula_list(void)
/***************************************************************
  EFFECTS: Parse a list of formulae.
           A formula can be extended by a <label> (nec. for 
           proof mode and increases readability of output)
           Depending on <isAxioms> store the formulae to 
           dfg_AXIOMARRAY or dfg_CONJECARRAY.
  RULE:    ::= list_of_formulae(dfg_parse_origin_type_is_axioms,
                                        EML).
               { formula( dfg_parse_formula {, dfg_parse_label} ). }^*
               end_of_list.
***************************************************************/
{
  BOOL  isAxiom;
  TERM  formula;
  char* label;
  LIST  pair;

  while (dfg_accept(T_list_of_formulae)) {
    dfg_expect(T_LPAREN);
    isAxiom = dfg_parse_origin_type_is_axioms();
    dfg_expectKeyPeriod(T_RPAREN);
    while (dfg_accept(T_formula)) {
      dfg_expect(T_LPAREN);
      formula = dfg_parse_formula();
      if (dfg_accept(T_COMMA)) {
	label = dfg_parse_label();
      }else{
	label = NULL;
      }
      dfg_expectKeyPeriod(T_RPAREN);


      pair = list_PairCreate(label, formula);
      if(isAxiom)
	dfg_AXIOMARRAY = array_Add(dfg_AXIOMARRAY,(intptr_t)pair);
      else
	dfg_CONJECARRAY = array_Add(dfg_CONJECARRAY,(intptr_t)pair);

      dfg_VarCheck();
    }
    dfg_expectKeyPeriod(T_end_of_list);
  }
}

static BOOL dfg_parse_special_type_isEml(void)
/***************************************************************
  EFFECTS: Determines what type of special formula list is parsed
  RETURNS: TRUE if it is an eml formula
  RULE:    ::= eml
***************************************************************/
{
  switch (token.kind) {
  case T_eml:
    lexer_next();
    if(!flag_GetFlagIntValue(dfg_FLAGS, flag_EML)){
      flag_SetFlagIntValue(dfg_FLAGS, flag_EML, flag_EMLON);
      eml_Init(dfg_PRECEDENCE);
    }
    return TRUE;
    break;
  default:
    dfg_expectError("special type (eml)");
    break;
  }
  return FALSE;
}

static TERM dfg_parse_prop_term(void);

static TERM dfg_parse_rel_term(void)
/***************************************************************
  EFFECTS: Parse a rel-formula
  RETURNS: Rel-formula as TERM
  RULE:    ::= or(dfg_parse_rel_term {, dfg_parse_rel_term}* ) |
               and(dfg_parse_rel_term {, dfg_parse_rel_term}* ) |
               sum(dfg_parse_rel_term {, dfg_parse_rel_term}* ) |
               comp(dfg_parse_rel_term {, dfg_parse_rel_term}* ) |
               true |
               false |
               id  |
               div |
               not( dfg_parse_rel_term ) |
               conv( dfg_parse_rel_term ) |
               implies( dfg_parse_rel_term , dfg_parse_rel_term ) |
               implied( dfg_parse_rel_term , dfg_parse_rel_term ) |
               equiv( dfg_parse_rel_term , dfg_parse_rel_term ) |
               nand( dfg_parse_rel_term , dfg_parse_rel_term ) |
               nor( dfg_parse_rel_term , dfg_parse_rel_term ) |
               xor( dfg_parse_rel_term , dfg_parse_rel_term ) |
               ranrestr( dfg_parse_rel_term , dfg_parse_prop_term ) |
               domrestr( dfg_parse_rel_term , dfg_parse_prop_term ) |
               test( dfg_parse_prop_term ) |
               ID
  WARNING: ID must correspond to an existing predicate symbol
***************************************************************/
{
  SYMBOL prop_symb;
  TERM   argument;
  LIST   argslist;
  char*  id;

  prop_symb = 0;

  switch (token.kind) {
  case T_or:
  case T_and:
    switch (token.kind) {
    case T_or:
      prop_symb = fol_Or();
      break;
    case T_and:
      prop_symb = fol_And();
      break;
    default:
      prop_symb = 0;
      //unreachable
    }

    lexer_next();
    argslist = list_Nil();
    if (dfg_accept(T_LPAREN)) {
      do {
	argument = dfg_parse_rel_term();
	argslist = list_Cons(argument,argslist);
      } while (dfg_accept(T_COMMA));
      dfg_expect(T_RPAREN);
    }
    argslist = list_NReverse(argslist);
    return term_Create(prop_symb,argslist);

    break;
  case T_true:
  case T_false:
    switch (token.kind) {
    case T_true:
      prop_symb = fol_True();
      break;
    case T_false:
      prop_symb = fol_False();
      break;
    default:
      prop_symb = 0;
      //unreachable
    }

    lexer_next();
    return term_Create(prop_symb,list_Nil());
    break;
  case T_not:
    prop_symb = fol_Not();
    lexer_next();
    dfg_expect(T_LPAREN);
    argslist = list_List(dfg_parse_rel_term());
    dfg_expect(T_RPAREN);
    return term_Create(prop_symb,argslist);
    break;
  case T_implies:
  case T_implied:
  case T_xor:
  case T_nor:
  case T_nand:
  case T_equiv:
    // prop_symbol
    switch (token.kind) {
    case T_implies:
      prop_symb = fol_Implies();
      break;
    case T_implied:
      prop_symb = fol_Implied();
      break;
    case T_equiv:
      prop_symb = fol_Equiv();
      break;
    case T_xor:
      prop_symb = fol_Xor();
      break;
    case T_nor:
      prop_symb = fol_Nor();
      break;
    case T_nand:
      prop_symb = fol_Nand();
      break;
    default:
      prop_symb = 0;
      //unreachable
    }

    lexer_next();
    
    dfg_expect(T_LPAREN);
    argument = dfg_parse_rel_term();
    dfg_expect(T_COMMA);
    argslist = list_List(dfg_parse_rel_term());
    dfg_expect(T_RPAREN);
    argslist = list_Cons(argument,argslist);
    return term_Create(prop_symb,argslist);
    break;
  case T_IDENTIFIER:
    if(string_Equal(token.text,"id")){
      prop_symb = eml_Id();
      lexer_next();
      return term_Create(prop_symb,list_Nil());
      break;
    }else if(string_Equal(token.text,"div")){
      prop_symb = eml_Div();
      lexer_next();
      return term_Create(prop_symb,list_Nil());
      break;
    }else if(string_Equal(token.text,"conv")){
      prop_symb = eml_Conv();
      lexer_next();
      dfg_expect(T_LPAREN);
      argslist = list_List(dfg_parse_rel_term());
      dfg_expect(T_RPAREN);
      return term_Create(prop_symb,argslist);
      break;
    }else if(string_Equal(token.text,"test")){
      lexer_next();
      dfg_expect(T_LPAREN);
      argslist = list_List(dfg_parse_prop_term());
      dfg_expect(T_RPAREN);
      return term_Create(eml_Test(),argslist);
      break;
    }else if(string_Equal(token.text,"sum")){
      prop_symb = eml_Sum();
      lexer_next();
      argslist = list_Nil();
      if (dfg_accept(T_LPAREN)) {
	do {
	  argument = dfg_parse_rel_term();
	  argslist = list_Cons(argument,argslist);
	} while (dfg_accept(T_COMMA));
	dfg_expect(T_RPAREN);
      }
      argslist = list_NReverse(argslist);
      return term_Create(prop_symb,argslist);
    }else if(string_Equal(token.text,"comp")){
      prop_symb = eml_Comp();
      lexer_next();
      argslist = list_Nil();
      if (dfg_accept(T_LPAREN)) {
	do {
	  argument = dfg_parse_rel_term();
	  argslist = list_Cons(argument,argslist);
	} while (dfg_accept(T_COMMA));
	dfg_expect(T_RPAREN);
      }
      argslist = list_NReverse(argslist);
      return term_Create(prop_symb,argslist);
    }else if(string_Equal(token.text,"domrestr")){
      prop_symb = eml_DomRestr();
      lexer_next();
      dfg_expect(T_LPAREN);
      argument = dfg_parse_rel_term();
      dfg_expect(T_COMMA);
      argslist = list_List(dfg_parse_prop_term());
      dfg_expect(T_RPAREN);
      argslist = list_Cons(argument,argslist);
      return term_Create(prop_symb,argslist);
      break;
    }else if(string_Equal(token.text,"ranrestr")){
      prop_symb = eml_RanRestr();
      lexer_next();
      dfg_expect(T_LPAREN);
      argument = dfg_parse_rel_term();
      dfg_expect(T_COMMA);
      argslist = list_List(dfg_parse_prop_term());
      dfg_expect(T_RPAREN);
      argslist = list_Cons(argument,argslist);
      return term_Create(prop_symb,argslist);
      break;
    }
  case T_NUMBER:
    id = string_StringCopy(token.text);
    lexer_next();
    return dfg_AtomCreate(id, list_Nil());
    break;

  default:
    dfg_expectError("prop term");
  }
  return NULL;
}


static TERM dfg_parse_prop_term(void)
/***************************************************************
  EFFECTS: Parse a prop-formula
  RETURNS: Prop-formula as TERM
  RULE:    ::= or(dfg_parse_prop_term {, dfg_parse_prop_term}* ) |
               and(dfg_parse_prop_term {, dfg_parse_prop_term}* ) |
               true |
               false |
               not( dfg_parse_prop_term ) |
               implies( dfg_parse_prop_term , dfg_parse_prop_term ) |
               implied( dfg_parse_prop_term , dfg_parse_prop_term ) |
               equiv( dfg_parse_prop_term , dfg_parse_prop_term ) |
               nand( dfg_parse_prop_term , dfg_parse_prop_term ) |
               nor( dfg_parse_prop_term , dfg_parse_prop_term ) |
               xor( dfg_parse_prop_term , dfg_parse_prop_term ) |
               box( dfg_parse_rel_term , dfg_parse_prop_term ) |
               all( dfg_parse_rel_term , dfg_parse_prop_term ) |
               dia( dfg_parse_rel_term , dfg_parse_prop_term ) |
               some( dfg_parse_rel_term , dfg_parse_prop_term ) |
               domain( dfg_parse_rel_term ) |
               range( dfg_parse_rel_term ) |
               ID
  WARNING: ID must correspond to an existing predicate symbol
***************************************************************/
{
  SYMBOL prop_symb;
  TERM   argument;
  LIST   argslist;
  char*  id;

  prop_symb = 0;

  switch (token.kind) {
  case T_or:
  case T_and:
    switch (token.kind) {
    case T_or:
      prop_symb = fol_Or();
      break;
    case T_and:
      prop_symb = fol_And();
      break;
    default:
      prop_symb = 0;
      //unreachable
    }

    lexer_next();
    argslist = list_Nil();
    if (dfg_accept(T_LPAREN)) {
      do {
	argument = dfg_parse_prop_term();
	argslist = list_Cons(argument,argslist);
      } while (dfg_accept(T_COMMA));
      dfg_expect(T_RPAREN);
    }
    argslist = list_NReverse(argslist);
    return term_Create(prop_symb,argslist);

    break;
  case T_true:
  case T_false:
    switch (token.kind) {
    case T_true:
      prop_symb = fol_True();
      break;
    case T_false:
      prop_symb = fol_False();
      break;
    default:
      prop_symb = 0;
      //unreachable
    }

    lexer_next();
    return term_Create(prop_symb,list_Nil());
    break;
  case T_not:
    lexer_next();
    dfg_expect(T_LPAREN);
    argslist = list_List(dfg_parse_prop_term());
    dfg_expect(T_RPAREN);
    return term_Create(fol_Not(),argslist);
    break;
  case T_implies:
  case T_implied:
  case T_equiv:
  case T_xor:
  case T_nor:
  case T_nand:
    // prop_symbol
    switch (token.kind) {
    case T_implies:
      prop_symb = fol_Implies();
      break;
    case T_implied:
      prop_symb = fol_Implied();
      break;
    case T_equiv:
      prop_symb = fol_Equiv();
      break;
    case T_xor:
      prop_symb = fol_Xor();
      break;
    case T_nor:
      prop_symb = fol_Nor();
      break;
    case T_nand:
      prop_symb = fol_Nand();
      break;
    default:
      prop_symb = 0;
      //unreachable
    }

    lexer_next();
    
    dfg_expect(T_LPAREN);
    argument = dfg_parse_prop_term();
    dfg_expect(T_COMMA);
    argslist = list_List(dfg_parse_prop_term());
    dfg_expect(T_RPAREN);
    argslist = list_Cons(argument,argslist);
    return term_Create(prop_symb,argslist);
    break;
  case T_IDENTIFIER:
    if(string_Equal(token.text,"box") || string_Equal(token.text,"all")){
      prop_symb = eml_Box();
      lexer_next();
      dfg_expect(T_LPAREN);
      argument = dfg_parse_rel_term();
      dfg_expect(T_COMMA);
      argslist = list_List(dfg_parse_prop_term());
      dfg_expect(T_RPAREN);
      argslist = list_Cons(argument,argslist);
      return term_Create(prop_symb,argslist);
      break;
    }else if(string_Equal(token.text,"dia") || string_Equal(token.text,"some")){
      prop_symb = eml_Dia();
      lexer_next();
      dfg_expect(T_LPAREN);
      argument = dfg_parse_rel_term();
      dfg_expect(T_COMMA);
      argslist = list_List(dfg_parse_prop_term());
      dfg_expect(T_RPAREN);
      argslist = list_Cons(argument,argslist);
      return term_Create(prop_symb,argslist);
      break;
    }else if(string_Equal(token.text,"domain")){
      prop_symb = eml_Domain();
      lexer_next();
      dfg_expect(T_LPAREN);
      argslist = list_List(dfg_parse_rel_term());
      dfg_expect(T_RPAREN);
      return term_Create(prop_symb,argslist);
      break;
    }else if(string_Equal(token.text,"range")){
      prop_symb = eml_Range();
      lexer_next();
      dfg_expect(T_LPAREN);
      argslist = list_List(dfg_parse_rel_term());
      dfg_expect(T_RPAREN);
      return term_Create(prop_symb,argslist);
      break;
    }
  case T_NUMBER:
    id = string_StringCopy(token.text);
    lexer_next();
    return dfg_AtomCreate(id, list_Nil());
    break;
  default:
    dfg_expectError("prop term");
  }
  return NULL;
}

static void dfg_parse_special_formula_list(void)
/***************************************************************
  EFFECTS: Parse a list of special formulae. Currently this
           is restricted to EML formulae.
           Depending on <isAxioms> store the formulae to 
           dfg_AXIOMARRAY or dfg_CONJECARRAY
  WARNING: dfg_parse_rel_terms are internally transformed into
           dfg_parse_prop_terms by surrounding them with
           box(not(rel_term),False)
  RULE:    ::= list_of_special_formulae(dfg_parse_origin_type_is_axioms,
                                        EML).
               { formula( dfg_parse_formula ). | 
                 prop_formula( dfg_parse_prop_term ). | 
                 rel_formula( dfg_parse_rel_term ). }^*
               end_of_list.
***************************************************************/
{
  BOOL  isAxioms;
  TERM  formula;
  char* label;
  LIST  pair;

  formula = NULL; 

  while (dfg_accept(T_list_of_special_formulae)) {
    dfg_expect(T_LPAREN);
    isAxioms = dfg_parse_origin_type_is_axioms();
    dfg_expect(T_COMMA);
    dfg_parse_special_type_isEml();
    dfg_expectKeyPeriod(T_RPAREN);

    while (token.kind != T_end_of_list) {
      switch (token.kind) {
      case T_formula:
	lexer_next();
	dfg_expect(T_LPAREN);
	formula = dfg_parse_formula();
	break;
      case T_prop_formula:
	lexer_next();
	dfg_expect(T_LPAREN);
	formula = dfg_parse_prop_term();
	break;
      case T_rel_formula:
	lexer_next();
	dfg_expect(T_LPAREN);
	formula = dfg_parse_rel_term();
	formula = term_Create(eml_Box(), 
			      list_Cons(term_Create(fol_Not(),
						    list_List(formula)), 
					list_List(term_Create(fol_False(),
							      list_Nil()))));
	break;
      default:
	dfg_expectError("end_of_list");
      }
      if (dfg_accept(T_COMMA)) {
	label = dfg_parse_label();
      }else{
	label = NULL;
      }

      pair = list_PairCreate(label, formula);
      
      if(isAxioms)
	dfg_AXIOMARRAY = array_Add(dfg_AXIOMARRAY,(intptr_t)pair);
      else
	dfg_CONJECARRAY = array_Add(dfg_CONJECARRAY,(intptr_t)pair);
      
      dfg_VarCheck();

      dfg_expectKeyPeriod(T_RPAREN);
    }

    dfg_expectKeyPeriod(T_end_of_list);
  }
}

static TERM dfg_parse_cnf_clause_body(void)
/***************************************************************
  EFFECTS: Parse a clause body. (disjunction of literals)
  RETURNS: The clause body as a TERM.
  RULE:    ::= or(dfg_parse_literal {, dfg_parse_literal})
***************************************************************/
{
  TERM literal;
  LIST literalslist;
  dfg_expect(T_or);
  dfg_expect(T_LPAREN);
  literalslist = list_Nil();
  do {
    literal      = dfg_parse_literal();
    literalslist = list_Cons(literal,literalslist);
  } while (dfg_accept(T_COMMA));

  literalslist = list_NReverse(literalslist);
  dfg_expect(T_RPAREN);

  return term_Create(fol_Or(),literalslist);
}

static TERM dfg_parse_cnf_clause(void)
/***************************************************************
  EFFECTS: Parse a CNF Clause. Variable sorts in <qvar_list> are 
           handled in dfg_CreateQuantifier!
  RETURNS: The CNF Clause as a TERM.
  RULE:    ::= forall(dfg_parse_qvar_list,
                      dfg_parse_cnf_clause_body)
***************************************************************/
{
  TERM body;
  LIST qvar_list;

  dfg_expect(T_forall);
  dfg_expect(T_LPAREN);
  qvar_list = dfg_parse_qvar_list();
  dfg_expect(T_COMMA);
  body      = dfg_parse_cnf_clause_body();
  dfg_expect(T_RPAREN);
  dfg_VarBacktrack();
  return dfg_CreateQuantifier(fol_All(),qvar_list,body);
	
}


static TERM dfg_parse_dnf_clause_body(void)
/***************************************************************
  EFFECTS: Parse a dnf clause body. (conjunction of literals)
  RETURNS: The clause body as a TERM.
  RULE:    ::= and(dfg_parse_literal {, dfg_parse_literal})
***************************************************************/
{
  TERM literal;
  LIST literalslist;
  dfg_expect(T_and);
  dfg_expect(T_LPAREN);
  literalslist = list_Nil();
  do {
    literal      = dfg_parse_literal();
    literalslist = list_Cons(literal,literalslist);
  } while (dfg_accept(T_COMMA));

  literalslist = list_NReverse(literalslist);
  dfg_expect(T_RPAREN);

  return term_Create(fol_And(),literalslist);
}

static TERM dfg_parse_dnf_clause(void)
/***************************************************************
  EFFECTS: Parse a DNF Clause. Variable sorts in <qvar_list> are 
           handled in dfg_CreateQuantifier!
  RETURNS: The DNF clause as a TERM.
  RULE:    ::= exists(dfg_parse_qvar_list,
                      dfg_parse_dnf_clause_body)
***************************************************************/
{
  TERM body;
  LIST qvar_list;

  dfg_expect(T_exists);
  dfg_expect(T_LPAREN);
  qvar_list = dfg_parse_qvar_list();
  dfg_expect(T_COMMA);
  body      = dfg_parse_dnf_clause_body();
  dfg_expect(T_RPAREN);
  dfg_VarBacktrack();
  return dfg_CreateQuantifier(fol_Exist(),qvar_list,body);
	
}

static LIST dfg_parse_term_ws_list(BOOL isSelectable)
/***************************************************************
  INPUT:   A boolean that is true if we allow the user to 
           select an atom (indicated with '+').
  EFFECTS: Parse a list of literals for a plain clause. At most 
           one atom can be selected by the user!
  RETURNS: The list of literals as a LIST of TERMs.
  RULE:    ::= { dfg_parse_literal { + } }^*
***************************************************************/
{
  TERM literal;
  LIST litlist;

  litlist = list_Nil();

  for(;;){
    if(token.kind == T_PIPE_PIPE || token.kind == T_MINUS_GREATER ||
       token.kind == T_COMMA || token.kind == T_RPAREN){
      break;
    }

    literal = dfg_parse_literal();
    litlist = list_Cons(literal,litlist);

    if(dfg_accept(T_PLUS)){
      if(!isSelectable) {
	misc_StartUserErrorReport();
	dfg_UserErrorReportHeader(& token.pos);
	misc_UserErrorReport("In this sequence no literal can be selected");
	misc_FinishUserErrorReport();
      }

      if(dfg_SELECTED_LITERAL != NULL) {
	misc_StartUserErrorReport();
	dfg_UserErrorReportHeader(& token.pos);
	misc_UserErrorReport("Trying to select two literals in a clause.");
	misc_FinishUserErrorReport();
      }
      dfg_SELECTED_LITERAL = literal;
    }
  }

  return list_NReverse(litlist);
  
}

static LIST dfg_parse_atom_ws_list(BOOL isSelectable)
/***************************************************************
  INPUT:   A boolean that is true if we allow the user to 
           select an atom (indicated with '+').
  EFFECTS: Parse a list of atoms for a plain clause. At most 
           one atom can be selected by the user!
  RETURNS: The list of atoms as a LIST of TERMs.
  RULE:    ::= { dfg_parse_atom { + } }^*
***************************************************************/
{
  TERM atom;
  LIST atomlist;

  atomlist = list_Nil();

  for(;;){
    if(token.kind == T_PIPE_PIPE || token.kind == T_MINUS_GREATER ||
       token.kind == T_COMMA || token.kind == T_RPAREN){
      break;
    }

    atom     = dfg_parse_atom();
    atomlist = list_Cons(atom,atomlist);

    if(dfg_accept(T_PLUS)){
      if(!isSelectable) {
	misc_StartUserErrorReport();
	dfg_UserErrorReportHeader(& token.pos);
	misc_UserErrorReport("In this sequence no atom can be selected");
	misc_FinishUserErrorReport();
      }

      if(dfg_SELECTED_LITERAL != NULL) {
	misc_StartUserErrorReport();
	dfg_UserErrorReportHeader(& token.pos);
	misc_UserErrorReport("Trying to select two atoms in a clause.");
	misc_FinishUserErrorReport();
      }
      dfg_SELECTED_LITERAL = atom;
    }
  }

  return list_NReverse(atomlist);
  
}

static TERM dfg_parse_clause(BOOL isAxiom)
/***************************************************************
  INPUT:   A boolean that is true if the clause is an axiom
           and false if it's a conjecture.
  EFFECTS: parse a clause. This clause is either of the form:
           1) forall([x,...],or(l1(x),l2(x)...))
           2) or(l1,l2,...) where l1, l2, etc. are (negated) 
              predicates of arity 0
           3) A || B |-> C , where A,B, and C are lists
              of atoms.
           A clause of type 3) is stored in either 
           dfg_PLAINAXCLAUSES or dfg_PLAINCONCLAUSES
  RULE:    ::= dfg_parse_cnf_clause       |
               dfg_parse_cnf_clause_body  |
               dfg_parse_term_ws_list || dfg_parse_term_ws_list 
               -> dfg_parse_term_ws_list 
***************************************************************/
{
  DFG_PLAINCLAUSE clause;
  switch (token.kind) {
  case T_forall:
    return dfg_parse_cnf_clause();
    break;
  case T_or:
    return dfg_parse_cnf_clause_body();
    break;
  default:
    clause = dfg_PlainClauseCreate();

    dfg_VarStart();
    
    clause->constraint = dfg_parse_term_ws_list(FALSE);
    dfg_expect(T_PIPE_PIPE);
    clause->antecedent = dfg_parse_atom_ws_list(TRUE);
    dfg_expect(T_MINUS_GREATER);
    clause->succedent  = dfg_parse_atom_ws_list(FALSE);
    
    dfg_VarStop();

    dfg_VarBacktrack();

    clause->selected     = dfg_SELECTED_LITERAL;
    dfg_SELECTED_LITERAL = (TERM) NULL;

    if(isAxiom)
      dfg_PLAINAXCLAUSES  = array_Add(dfg_PLAINAXCLAUSES,(intptr_t)clause);
    else
      dfg_PLAINCONCLAUSES = array_Add(dfg_PLAINCONCLAUSES,(intptr_t)clause);
    break;
  }
  return NULL;
}

static void dfg_parse_clause_list(void)
/***************************************************************
  EFFECTS: parse a list of clauses.
           Depending on dfg_parse_origin_type_is_axioms
           these clauses are axioms or conjectures and are
           then saved to dfg_AXCLAUSES or dfg_CONCLAUSES, 
           respectively. Plain clauses are stored in 
           dfg_PLAINAXCLAUSES or dfg_PLAINCONCLAUSES,
  RULE:    ::= list_of_clauses(dfg_parse_origin_type_is_axioms,cnf).
               clause(dfg_parse_clause {, <label>}). |
               list_of_clauses(dfg_parse_origin_type_is_axioms,dnf).
               clause(dfg_parse_dnf_clause {, <label>}).
***************************************************************/
{
  BOOL  isAxiom;
  TERM  clause;
  char* label;
  LIST  pair;

  while (dfg_accept(T_list_of_clauses)) {
    dfg_expect(T_LPAREN);
    isAxiom = dfg_parse_origin_type_is_axioms();
    dfg_expect(T_COMMA);
    if(dfg_accept(T_cnf)){
      dfg_expectKeyPeriod(T_RPAREN);
      
      while (dfg_accept(T_clause)) {
	dfg_expect(T_LPAREN);
	clause = dfg_parse_clause(isAxiom);
	
	if (dfg_accept(T_COMMA)) {
	  label = dfg_parse_label();
	}else{
	  label = NULL;
	}
	
	if(clause == NULL){
	  if(isAxiom){
	    dfg_PLAINCLAXAX  = list_Cons(label,dfg_PLAINCLAXAX);
	  }else{
	    dfg_PLAINCLAXCON = list_Cons(label,dfg_PLAINCLAXCON);
	  }
	}else{
	  pair = list_PairCreate(label,clause);
	  if(isAxiom)
	    dfg_AXCLAUSES  = array_Add(dfg_AXCLAUSES,(intptr_t)pair);
	  else
	    dfg_CONCLAUSES = array_Add(dfg_CONCLAUSES,(intptr_t)pair);
	}
	dfg_VarCheck();
	dfg_expectKeyPeriod(T_RPAREN);
      }

    }else if(dfg_accept(T_dnf)){
      dfg_expectKeyPeriod(T_RPAREN);
      printf("\n PARSER: WARNING! SPASS parses DNF clauses but directly throws them away! \n");
      while (dfg_accept(T_clause)) {
	dfg_expect(T_LPAREN);
	switch (token.kind) {
	case T_exists:
	  term_Delete(dfg_parse_dnf_clause());
	  break;
	case T_and:
	  term_Delete(dfg_parse_dnf_clause_body());
	  break;
	default:
	  dfg_expectError("either exists(...) or and(...)");
	}

	clause = NULL;
	
	if (dfg_accept(T_COMMA)) {
	  label = dfg_parse_label();
	  
	}else{
	  label = NULL;
	}
	
	if(clause == NULL){
	  if( label != NULL){
	    string_StringFree(label);
	  }
	}

	dfg_VarCheck();
	dfg_expectKeyPeriod(T_RPAREN);
      }
    }else{
      dfg_expectError("either cnf or dfg");
    }

    dfg_expectKeyPeriod(T_end_of_list);
  }
}

static char* dfg_parse_reference(void)
/***************************************************************
  EFFECTS: parse the label for the resulting formula of a proof step
  RULE:    ::= <identifier> | <number>
***************************************************************/
{
  char* reference;
  switch (token.kind) {
  case T_IDENTIFIER:
  case T_NUMBER:
    reference = string_StringCopy(token.text);
    lexer_next();
    return reference;
    break;

  default:
    dfg_expectError("reference (identifier or number)");
    break;
  }

  return NULL;
}

static TERM dfg_parse_result(void)
/***************************************************************
  EFFECTS: parse a cnf clause a the result of a proof step
  RULE:    ::= dfg_parse_cnf_clause |
               dfg_parse_cnf_clause_body
***************************************************************/
{
  if(token.kind == T_forall){
    return dfg_parse_cnf_clause();
  }else if(token.kind == T_or){
    return dfg_parse_cnf_clause_body();
  }
  dfg_expectError("cnf clause (i.e.: forall(...) or or(...))");
  return NULL;

}

static char* dfg_parse_rule_appl(void)
/***************************************************************
  EFFECTS: parse a proof rule name
  RULE:    ::= <identifier> | <number> | ...
***************************************************************/
{
  char* rule;
  switch (token.kind) {
  case T_IDENTIFIER:
  case T_NUMBER:
  case T_App:
  case T_SpL:
  case T_SpR:
  case T_EqF:
  case T_Rew:
  case T_Obv:
  case T_EmS:
  case T_SoR:
  case T_EqR:
  case T_MPm:
  case T_SPm:
  case T_OPm:
  case T_SHy:
  case T_OHy:
  case T_URR:
  case T_Fac:
  case T_Spt:
  case T_Inp:
  case T_Con:
  case T_SSi:
  case T_UnC:
  case T_Ter:
  case T_Res:
  case T_CRW:
  case T_AED:
  case T_MRR:
  case T_Def:
    rule = string_StringCopy(token.text);
    lexer_next();
    return rule;
    break;

  default:
    dfg_expectError("rule application");
    break;
  }
  return NULL;
}

static char* dfg_parse_parent(void)
/***************************************************************
  EFFECTS: parse a parent (a formula label as prerequisites
           for a proof step)
  RULE:    ::= <identifier> | <number>
***************************************************************/
{
  char* parent;
  switch (token.kind) {
  case T_IDENTIFIER:
  case T_NUMBER:
    parent = string_StringCopy(token.text);
    lexer_next();
    return parent;
    break;

  default:
    dfg_expectError("parent");
    break;
  }

  return NULL;
}

static LIST dfg_parse_parent_list(void)
/***************************************************************
  EFFECTS: parse a list of parents (formula labels as prerequisites
           for a proof step)
  RULE:    ::= [parent {, parent}^* ]
***************************************************************/
{
  char* parent;
  LIST  parents;
  dfg_expect(T_LBRACKET);
  parents = list_Nil();
  do {
    parent  = dfg_parse_parent();
    parents = list_Cons(parent,parents);
  } while (dfg_accept(T_COMMA));
  dfg_expect(T_RBRACKET);

  parents = list_NReverse(parents);
  return parents;
}

static BOOL dfg_parse_key_is_splitlevel(void)
/***************************************************************
  EFFECTS: parse a key for the associated list of splitlevels.
           This key is either 'splitlevel' or an arbitrary ID
  RULE:    ::= splitlevel | <identifier> | <number>
***************************************************************/
{
  switch (token.kind) {
  case T_IDENTIFIER:
  case T_NUMBER:
    lexer_next();
    return FALSE;
    break;
  case T_splitlevel:
    lexer_next();
    return TRUE;
    break;
  default:
    dfg_expectError("key (splitlevel)");
    break;
  }
  return FALSE;
}

static intptr_t dfg_parse_value(void)
{
  return dfg_expectAndReturnNumber(T_NUMBER);
}

static intptr_t dfg_parse_assoc_list(void)
/***************************************************************
  EFFECTS: parse an associated list of splitlevels for a parent
           list.
  RULE:    ::=   [dfg_parse_key_is_splitlevel : <number>
                  {, dfg_parse_key_is_splitlevel : <number>}]
***************************************************************/
{
  BOOL     isSplitlevel;
  intptr_t value, retvalue;
  retvalue = -1;
  dfg_expect(T_LBRACKET);
  do {
    isSplitlevel = dfg_parse_key_is_splitlevel();
    dfg_expect(T_COLON);
    value        = dfg_parse_value();
    if(isSplitlevel){
      retvalue   = value;
    }
  } while (dfg_accept(T_COMMA));
  dfg_expect(T_RBRACKET);
  return retvalue;
}

static void dfg_parse_proof_list(void)
/***************************************************************
  EFFECTS: parse a SPASS proof via resolution. A proof consists 
           of "steps" which consist of a
           - reference:   the label for the resulting formula
           - clause:      the resulting formula
           - rule_appl:   the name of the rule applied to get this formula,
           - parent_list: a list of labels of derived clauses that
                          are prerequisites for this step
           - assoc_list:  the split level for the parents
           Everything is stored in dfg_PROOFLIST
  RULE:    ::=   list_of_proof(SPASS).
                 {step(dfg_parse_reference,
                       dfg_parse_result,
                       dfg_parse_rule_appl,
                       dfg_parse_parent_list
                       {, dfg_parse_assoc_list})}^*
                 end_of_list.
***************************************************************/
{
  char*       reference;
  char*       rule;
  TERM        result;
  LIST        parents;
  intptr_t   splitlevel;
  while (dfg_accept(T_list_of_proof)) {
    dfg_expect(T_LPAREN);
    dfg_expect(T_SPASS);
    dfg_expectKeyPeriod(T_RPAREN);
    while (dfg_accept(T_step)) {
      dfg_expect(T_LPAREN);
      reference  = dfg_parse_reference();
      dfg_expect(T_COMMA);
      result     = dfg_parse_result();
      dfg_expect(T_COMMA);
      rule       = dfg_parse_rule_appl();
      dfg_expect(T_COMMA);
      parents    = dfg_parse_parent_list();
      if (dfg_accept(T_COMMA)) {
	splitlevel = dfg_parse_assoc_list();
      }else{
	splitlevel = 0;
      }
      dfg_expectKeyPeriod(T_RPAREN);


      if (reference!=NULL && result!=NULL && !list_Empty(parents)) {
	LIST tupel;
	RULE Rule = clause_GetOriginFromString(rule);
	string_StringFree(rule);
    
	/* Build a tuple (label,clause,parentlist,split level,origin) */
	tupel = list_Cons((POINTER)splitlevel,list_List((POINTER)Rule));
	tupel = list_Cons(reference,list_Cons(result,list_Cons(parents,tupel)));
	dfg_PROOFLIST = list_Cons(tupel, dfg_PROOFLIST);
      } else {
	/* ignore clauses with incomplete data */
	if (reference  != NULL) string_StringFree(reference);
	if (result != NULL) term_Delete(result);
	if (rule  != NULL)  string_StringFree(rule);
	dfg_DeleteStringList(parents);
      }
      dfg_VarCheck();

    }
    dfg_expectKeyPeriod(T_end_of_list);
  }
}

static void dfg_parse_logical_part(void)
/***************************************************************
  EFFECTS: parses the logical parts of the dfg input language.
           This includes symbol lists, declarations, formulas,
           special formulas (eml, etc.), and clauses
  RULE:    ::=   dfg_parse_symbol_list
                 dfg_parse_declaration_list
                 dfg_parse_formula_list
                 dfg_parse_special_formula_list
                 dfg_parse_clause_list
                 dfg_parse_proof_list 
***************************************************************/
{
  dfg_parse_symbol_list();
  dfg_parse_declaration_list();
  dfg_parse_formula_list();
  dfg_parse_special_formula_list();
  dfg_parse_clause_list();
  dfg_parse_proof_list();
}

static LIST dfg_parse_fla_selection(void)
/***************************************************************
  EFFECTS: parse a list of formula names for an include file.
           This allows the user to white-list only certain formulas
           in the include file!
  RULE:    ::=  {<label>}^*
***************************************************************/
{
  LIST  fla_selection;
  char* fla;
  fla_selection = list_Nil();
  dfg_expect(T_LBRACKET);
  do {
    fla = string_StringCopy(dfg_expectIdAndReturnString());
    fla_selection = list_Cons(fla, fla_selection);
  } while (dfg_accept(T_COMMA));
  dfg_expect(T_RBRACKET);

  return list_NReverse(fla_selection);
}

static void dfg_parse_includes(void)
/***************************************************************
  EFFECTS: parse include files that have to be parsed after 
           all remaining files in dfg_INCLUDEARRAY are read.
           If fla_selection is not omitted it contains a list 
           of formula names/labels. Only these formulas are included!
  RULE:    ::=  {list_of_includes.
                 include(<filename> {, dfg_parse_fla_selection})
                }^*
***************************************************************/
{
  LIST  includes;
  LIST  fla_selection;
  char* filename;

  if(dfg_acceptKeyPeriod(T_list_of_includes)) {
    includes = list_Nil();
    while (dfg_accept(T_include)) {
      dfg_expect(T_LPAREN);
      filename = string_StringCopy(dfg_expectAndReturnString(T_FILENAME));
      if (dfg_accept(T_COMMA)) {
	fla_selection = dfg_parse_fla_selection();
      }else{
	fla_selection = list_Nil();
      }
      dfg_expectKeyPeriod(T_RPAREN);
      dfg_INCLUDEARRAY = array_Add(dfg_INCLUDEARRAY,(intptr_t)list_PairCreate(filename,fla_selection));
    }
    dfg_expectKeyPeriod(T_end_of_list);
  }
}

static void dfg_parse_setting_entry(void)
/***************************************************************
  EFFECTS: parse a general setting entry also called hypothesis. 
           this setting has no effect at all.
  RULE:    ::=   hypothesis[<label> {, <label>}^*].
***************************************************************/
{
  char* label;
  LIST  sett_entry;
  dfg_expect(T_hypothesis);
  dfg_expect(T_LBRACKET);
  sett_entry = list_Nil();
  do {
    label = dfg_parse_label();
    sett_entry = list_Cons(label,sett_entry);
  } while (dfg_accept(T_COMMA));
  dfg_expectKeyPeriod(T_RBRACKET);
  dfg_DeleteStringList(sett_entry);
}


static void dfg_parse_set_flag(void)
/***************************************************************
  EFFECTS: parse a flag <flag_name> and set it to <value> 
  RULE:    ::=   set_flag(<flag_name>,<value>) 
***************************************************************/
{
  int      flag;
  intptr_t number;
  char*    id;
  dfg_expect(T_set_flag);
  dfg_expect(T_LPAREN);
  id = string_StringCopy(dfg_expectIdAndReturnString());
  dfg_expect(T_COMMA);
  number = dfg_expectAndReturnNumber(T_NUMBER);
  dfg_expectKeyPeriod(T_RPAREN);

  flag = flag_Id(id);
  if (flag == -1) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportHeader(& token.pos);
    misc_UserErrorReport("Found unknown flag %s",id);
    misc_FinishUserErrorReport();
  }
  string_StringFree(id);
  flag_SetFlagIntValue(dfg_FLAGS, flag, number);
}

static void dfg_parse_set_prec_entry(void)
/***************************************************************
  EFFECTS: parse a symbol and set its precedence.
           In case 1: <sym_name> make it the largest symbol in
           the precedence ordering
           In case 2: (<sym_name>,<number>,<set_ord>) make 
           <sym_name> the largest symbol in the precedence ordering, 
           set its weight to <number>, and ordering properties
           (l = lexicographic ordering)
           (m = multiset ordering)
           (r = reverse-lexicographic ordering)
           In both cases store symbol in dfg_UserPrecedence
  RULE:    ::=   <sym_name> | (<sym_name>,<number>,set_ord)
  set_ord  ::=  l | m | r 
***************************************************************/
{
  SYMBOL      sym;
  char*       id;
  char const* id_stat;
  intptr_t    number;
  if(dfg_accept(T_LPAREN)){
    id = string_StringCopy(dfg_expectIdAndReturnString());

    sym = symbol_Lookup(string_StringCopy(id));
    if (sym == 0) {
      misc_StartUserErrorReport();
      dfg_UserErrorReportHeader(& token.pos);
      misc_UserErrorReport("Undefined symbol %s",id);
      misc_UserErrorReport(" in precedence setting.\n"); 
      misc_FinishUserErrorReport(); 
    }
    string_StringFree(id);

    dfg_expect(T_COMMA);

    number = dfg_expectAndReturnNumber(T_NUMBER);

    dfg_expect(T_COMMA);

    id_stat = dfg_expectIdAndReturnString();
    if(id_stat[1] != '\0' ||
       (id_stat[0]!='l' && id_stat[0]!='m' && id_stat[0]!='r')) {
      misc_StartUserErrorReport();
      dfg_UserErrorReportHeader(& token.pos);
      misc_UserErrorReport("Invalid symbol status %s",id_stat);
      misc_UserErrorReport(" in precedence list.");
      misc_FinishUserErrorReport();
    }
    int ord;
    switch (id[0]) {
    case 'm': ord = ORDMUL;   break;
    case 'r': ord = ORDRIGHT; break;
    default:  ord = 0;
    }

    dfg_expect(T_RPAREN);

    symbol_SetIncreasedOrdering(dfg_PRECEDENCE, sym);
    dfg_USERPRECEDENCE = list_Cons((POINTER)sym, dfg_USERPRECEDENCE);
    symbol_SetWeight(sym, number);
    if (ord != 0)
      symbol_AddProperty(sym, ord);

  }else{
    id  = string_StringCopy(dfg_expectIdAndReturnString());
    sym = symbol_Lookup(string_StringCopy(id));
    if (sym == 0) {
      misc_StartUserErrorReport();
      dfg_UserErrorReportHeader(& token.pos);
      misc_UserErrorReport("Undefined symbol %s",id);
      misc_UserErrorReport(" in precedence setting.\n"); 
      misc_FinishUserErrorReport(); 
    }
    string_StringFree(id);
    symbol_SetIncreasedOrdering(dfg_PRECEDENCE, sym);
    dfg_USERPRECEDENCE = list_Cons((POINTER)sym, dfg_USERPRECEDENCE);
  }
}

static void dfg_parse_set_precedence(void)
/***************************************************************
  EFFECTS: parse setting determining symbol precedence
  RULE:    ::=   set_precedence( dfg_parse_set_prec_entry 
                                 {, dfg_parse_set_prec_entry}^*)
***************************************************************/
{
  dfg_expect(T_set_precedence);
  dfg_expect(T_LPAREN);
  do{
    dfg_parse_set_prec_entry();
  }while(dfg_accept(T_COMMA));

  dfg_expectKeyPeriod(T_RPAREN);
}

static void dfg_parse_set_selection_item(void)
/***************************************************************
  EFFECTS: parse a predicate and make it a predicate selected by
           the user
           (this information is stored in dfg_USERSELECTION)
  RULE:    ::=   <pred_name>
***************************************************************/
{
  SYMBOL s;
  char*  id;
  id = string_StringCopy(dfg_expectIdAndReturnString());
  s  = symbol_Lookup(string_StringCopy(id));
  if (s == 0) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportHeader(& token.pos);
    misc_UserErrorReport("Undefined symbol %s ", id);
    misc_UserErrorReport(" in selection setting.\n"); 
    misc_FinishUserErrorReport(); 
  }

  if (!symbol_IsPredicate(s)) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportHeader(& token.pos);
    misc_UserErrorReport("Symbol %s isn't a predicate", 
			 string_StringCopy(token.text));
    misc_UserErrorReport(" in selection setting.\n");
    misc_FinishUserErrorReport();
  }
  string_StringFree(id);
  dfg_USERSELECTION = list_Cons((POINTER)s, dfg_USERSELECTION);
}

static void dfg_parse_set_selection(void)
/***************************************************************
  EFFECTS: parse setting determining user selected predicates
  RULE:    ::=   set_selection( dfg_parse_set_selection_item 
                              {, dfg_parse_set_selection_item}^*)
***************************************************************/
{
  dfg_expect(T_set_selection);
  dfg_expect(T_LPAREN);
  do{
    dfg_parse_set_selection_item();
  }while(dfg_accept(T_COMMA));
  dfg_expectKeyPeriod(T_RPAREN);
}

static void dfg_parse_set_DomPred(void)
/***************************************************************
  EFFECTS: parse the setting that determines whether a predicate 
           is dominating
  RULE:    ::=   set_DomPred(<pred_name> {, <pred_name>}^*)
***************************************************************/
{
  SYMBOL pred_sym;
  LIST   list1;
  dfg_expect(T_set_DomPred);
  dfg_expect(T_LPAREN);
  list1 = list_Nil();
  do{
    pred_sym = dfg_parse_declared_pred_sym();
    list1    = list_Cons((POINTER)pred_sym,list1);
  }while(dfg_accept(T_COMMA));

  for ( ; !list_Empty(list1); list1 = list_Pop(list1)) {
    pred_sym = (SYMBOL)list_Car(list1);
    if (pred_sym == 0) {
      misc_StartUserErrorReport();
      dfg_UserErrorReportHeader(& token.pos);
      misc_UserErrorReport("Undefined symbol");
      misc_UserErrorReport(" in DomPred setting.\n"); 
      misc_FinishUserErrorReport();
    }
    if (!symbol_IsPredicate(pred_sym)) {
      misc_StartUserErrorReport();
      dfg_UserErrorReportHeader(& token.pos);
      misc_UserErrorReport("Symbol %s isn't a predicate",symbol_Name(pred_sym));
      misc_UserErrorReport(" in DomPred setting.\n");
      misc_FinishUserErrorReport();
    }
    string_StringFree(list_Car(list1)); 
    symbol_AddProperty(pred_sym, DOMPRED);
  }

  dfg_expectKeyPeriod(T_RPAREN);
}

static void dfg_parse_set_ClauseFormula_entry(void)
/***************************************************************
  EFFECTS: stores a clause formula relation as an axiom on top of
           dfg_CLAXRELATION [[<number_1>,<label_{1,1}>,...,<label_{1,n}>],
                             [<number_2>,<label_{2,1}>,...,<label_{2,n}>],...]
  RULE:    ::=   (<number_i>{,<label_{i,j}>}^*)
***************************************************************/
{
  intptr_t  number;
  char*     label;
  dfg_expect(T_LPAREN);
  number = dfg_expectAndReturnNumber(T_NUMBER);
  while(dfg_accept(T_COMMA)){
    label          = dfg_parse_label();
    dfg_CLAXAXIOMS = list_Cons((POINTER)label, dfg_CLAXAXIOMS);
  }

  dfg_expect(T_RPAREN);

  dfg_CLAXRELATION = list_Cons(list_Cons((POINTER)number, dfg_CLAXAXIOMS), dfg_CLAXRELATION);
  dfg_CLAXAXIOMS   = list_Nil();
}

static void dfg_parse_set_ClauseFormulaRelation(void)
/***************************************************************
  EFFECTS: parse the clause formula relation setting
  RULE:    ::=   set_ClauseFormulaRelation(
                        dfg_parse_set_ClauseFormula_entry
                        {, dfg_parse_set_ClauseFormula_entry}^+
                 )
***************************************************************/
{
  dfg_expect(T_set_ClauseFormulaRelation);
  dfg_expect(T_LPAREN);

  do{
    dfg_parse_set_ClauseFormula_entry();
  }while(dfg_accept(T_COMMA));
  dfg_expectKeyPeriod(T_RPAREN);
}


static void dfg_parse_settings_sets(void)
/***************************************************************
  EFFECTS: parse one of the five types of SPASS settings
  RULE:    ::=   dfg_parse_set_flag       |
                 dfg_parse_set_precedence |
                 dfg_parse_set_selection  |
                 dfg_parse_set_DomPred    |
                 dfg_parse_set_ClauseFormulaRelation
***************************************************************/
{
  do{
    switch (token.kind) {
    case T_set_flag:
      dfg_parse_set_flag();
      break;
    case T_set_precedence:
      dfg_parse_set_precedence();
      break;
    case T_set_selection:
      dfg_parse_set_selection();
      break;
    case T_set_DomPred:
      dfg_parse_set_DomPred();
      break;
    case T_set_ClauseFormulaRelation:
      dfg_parse_set_ClauseFormulaRelation();
      break;
    default:
      return;
    }
  }while(TRUE);
}


static void dfg_parse_settings(void)
/***************************************************************
  EFFECTS: parses a list of dfg settings (either general or SPASS
           specific)
  RULE:    ::= list_of_general_settings.
                   {dfg_parse_setting_entry}^+
                   end_of_list.  |
               list_of_settings(SPASS).
                   {dfg_parse_settings_set}^+
                   end_of_list.
***************************************************************/
{
  for (;;) {
    switch (token.kind) {
    case T_list_of_general_settings:
      if (dfg_IGNORESETTINGS) {
	misc_StartUserErrorReport();
	misc_UserErrorReport("\n Settings not allowed in included files\n");			
	misc_FinishUserErrorReport();
      }
      dfg_expectKeyPeriod(T_list_of_general_settings);
      do {
	dfg_parse_setting_entry();
      } while (token.kind == T_hypothesis);
      dfg_expectKeyPeriod(T_end_of_list);
      break;

    case T_list_of_settings:
      if (dfg_IGNORESETTINGS) {
	misc_StartUserErrorReport();
	misc_UserErrorReport("\n Settings not allowed in included files\n");			
	misc_FinishUserErrorReport();
      }
      dfg_expect(T_list_of_settings);
      dfg_expect(T_LPAREN);
      dfg_expect(T_SPASS);
      dfg_expectKeyPeriod(T_RPAREN);
      dfg_expect(T_LPARSTAR);

      dfg_parse_settings_sets();

      dfg_expect(T_RPARSTAR);
      dfg_expectKeyPeriod(T_end_of_list);
      break;

    default:
      return;
    }
  }
}

static void dfg_parse_problem(void)
/***************************************************************
  EFFECTS: parses a complete problem in the dfg language and
           initializes symbols, allocates formulas, etc.
  RULE:    ::= begin_problem(Id).
               dfg_parse_description
               dfg_parse_logical_part
               {dfg_parse_includes}^*
               {dfg_parse_settings}^*
               end_problem. 
***************************************************************/
{
  dfg_expect(T_begin_problem);
  dfg_expect(T_LPAREN);
  dfg_expectId();
  dfg_expectKeyPeriod(T_RPAREN);
  dfg_parse_description();
  dfg_parse_logical_part();
  dfg_parse_includes();
  dfg_parse_settings();
  dfg_expectKeyPeriod(T_end_problem);
}

/**
 * Parse a translation unit.
 *
 * @return The statement representing the translation unit.
 */
void dfg_parse_translation_unit(void)
/***************************************************************
  EFFECTS: parses a complete problem in the dfg language and
           initializes symbols, allocates formulas, etc.
  RULE:    ::= dfg_parse_problem
***************************************************************/
{
  lexer_next();
  dfg_parse_problem();
  if (token.kind != T_EOF) {
    misc_StartUserErrorReport();
    dfg_UserErrorReportF(&token.pos, "stray token '%s' at end of translation unit", token.text);
    misc_FinishUserErrorReport();
  }
  return;
}
