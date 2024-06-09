/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                  PARSER FOR PROLOG SYNTAX              * */
/* *                                                        * */
/* *  $Module:   pro                                        * */ 
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
/* $Revision: 1.6 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-25 06:52:06 $                             * */
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


/* $RCSfile: proparser.y,v $ */

%{

#include "pro.h"
#include "dfg.h"
#include "symbol.h"
#include "term.h"
#include "foldfg.h"
#include "clause.h"


void yyerror(const char*);
int  yylex(void);		/* Defined in proscanner.l */

#define YYERROR_VERBOSE


NAT pro_LINENUMBER;  /* used also in the scanner */

static LIST       pro_RULESLIST;     /* A list of terms representing clauses */
static TERM       pro_QUESTIONTERM;
static PRECEDENCE pro_PRECEDENCE;
static LIST       pro_SYMBOLLIST;
static LIST       pro_VARLIST;

static void   pro_AppendRule(TERM, LIST);
static void   pro_StoreQuestion(LIST);
static TERM   pro_TermCreate(char*, LIST);
static TERM   pro_VarTermCreate(char*);
static TERM   pro_CheckAtom(TERM Atom);
static void   pro_CheckArguments(TERM);
static void   pro_RestartVarList(void);
static SYMBOL pro_VarLookup(char*);

 void pro_AppendFact(TERM Term)
{
  Term = term_Create(fol_Or(), list_List(Term));
  pro_RULESLIST = list_Nconc(pro_RULESLIST, list_List(Term));
  pro_RestartVarList();
}

%}

%union {
  char*     string;
  TERM      term;
  LIST      list;
}

%token PRO_IF PRO_QUESTION
%token <string> PRO_ID PRO_VAR PRO_NUM 
%type  <string>  id
%type  <list>    atomlist termlist
%type  <term>    term atom

%%

/****************/
/* TOP RULE     */
/****************/

prologprogram:	list_of_rules
                | list_of_rules question
;

question:    PRO_QUESTION atomlist '.' { pro_StoreQuestion($2); }
;

list_of_rules:	/* empty */
		| list_of_rules rule_or_fact
;

rule_or_fact:	fact
		| rule
;

fact:		atom '.'		{ pro_AppendFact($1); }
;

rule:		atom PRO_IF atomlist '.' { pro_AppendRule($1,$3); }
;

atomlist:	atom			{ $$ = list_List($1); }
		| atomlist ',' atom	{ $$ = list_Nconc($1,list_List($3)); }
;

atom:		term			{ $$ = pro_CheckAtom($1); }
;

term:		id			{ $$ = pro_TermCreate($1,list_Nil()); }
		| PRO_VAR		{ $$ = pro_VarTermCreate($1); } 
		| PRO_ID '(' termlist ')' { $$ = pro_TermCreate($1, $3); }
;

termlist:	term			{ $$ = list_List($1); }
		| termlist ',' term	{ $$ = list_Nconc($1, list_List($3)); }
;

id:		PRO_ID			{ $$ = $1; }
		| PRO_NUM		{ $$ = $1; }
;


%%

void yyerror(const char *s)
{
  misc_StartUserErrorReport();
  misc_UserErrorReport("\n line %i: %s\n", pro_LINENUMBER, s);
  misc_FinishUserErrorReport();
}


LIST pro_PrologParser(FILE* InputFile, FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An input file in Prolog syntax, a flag store and a precedence,
           that are needed for the initialization of clauses.
  RETURNS: A list of clauses created from the input file.
  EFFECT:  The precedence of new symbols is set in <Precedence>.
  CAUTION: The parser can't handle Prolog files containing lists
           or operators.
***************************************************************/
{
  extern FILE* pro_in;  /* declared in proscanner */
  LIST         Scan;

  pro_in           = InputFile;
  pro_LINENUMBER   = 1;
  pro_QUESTIONTERM = (TERM) NULL;
  pro_RULESLIST    = list_Nil();
  pro_SYMBOLLIST   = list_Nil();
  pro_VARLIST      = list_Nil();
  pro_PRECEDENCE   = Precedence;

  pro_parse();  /* Call the parser */

  /* Cleanup */
  list_Delete(pro_SYMBOLLIST);
  pro_RestartVarList();

  /* Create Clauses */
  for (Scan = pro_RULESLIST; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
    list_Rplaca(Scan, dfg_CreateClauseFromTerm(list_Car(Scan), TRUE, Flags,
					       Precedence));
    clause_UpdateWeight(list_Car(Scan), Flags);
  }
  /* Conjecture clause */
  if (pro_QUESTIONTERM != NULL) {
    CLAUSE Con;

    Con = dfg_CreateClauseFromTerm(pro_QUESTIONTERM, FALSE, Flags, Precedence);
    pro_RULESLIST = list_Nconc(pro_RULESLIST, list_List(Con));
  }
  return pro_RULESLIST;
}


static TERM pro_TermCreate(char* Name, LIST Arguments)
/* Look up the symbol, create the term and check arguments */
{
  SYMBOL s;
  TERM   result;

  s = symbol_Lookup(Name);
  if (s == 0)
    s = symbol_CreateFunction(Name, list_Length(Arguments), symbol_STATLEX,
			      pro_PRECEDENCE);

  /* Check the symbol's arity */
  if (symbol_Arity(s) != list_Length(Arguments)) {
    misc_StartUserErrorReport();
    misc_UserErrorReport("\n line %i: Symbol '%s' was defined with arity %d\n",
                         pro_LINENUMBER, Name, symbol_Arity(s));
    misc_FinishUserErrorReport();
  }

  string_StringFree(Name);    /* was copied by symbol_CreateFunction */

  result = term_Create(s, Arguments);
  pro_CheckArguments(result);
      
  return result;
}

static TERM pro_VarTermCreate(char* Name)
{
  SYMBOL s;

  s = pro_VarLookup(Name);
  return term_Create(s, list_Nil());
}

static TERM pro_CheckAtom(TERM Atom)
{
  SYMBOL s;

  s = term_TopSymbol(Atom);
  if (!symbol_IsPredicate(s)) {
    if (list_PointerMember(pro_SYMBOLLIST, (POINTER)s)) {
      misc_StartUserErrorReport();
      misc_UserErrorReport("\n parse error in line %i:", pro_LINENUMBER);
      misc_UserErrorReport("\n '%s' is not a predicate\n", symbol_Name(term_TopSymbol(Atom)));
      misc_FinishUserErrorReport();
    } else {
      /* First occurrence, so change it to a predicate */
      s = symbol_ChangeType(s, symbol_PREDICATE);
      pro_SYMBOLLIST = list_Cons((POINTER)s, pro_SYMBOLLIST);
      term_RplacTop(Atom, s);
    }
  }
  return Atom;
}

static void pro_CheckArguments(TERM Term)
{
  LIST scan;

  for (scan = term_ArgumentList(Term); !list_Empty(scan); scan = list_Cdr(scan)) {
    SYMBOL argsymbol = term_TopSymbol(list_Car(scan));
    if (symbol_IsPredicate(argsymbol)) {
      misc_StartUserErrorReport();
      misc_UserErrorReport("\n parse error line %i:", pro_LINENUMBER);
      misc_UserErrorReport(" Term has predicate term as argument: ");
      misc_UserErrorReport("%s(...%s...)\n", 
			   symbol_Name(term_TopSymbol(Term)),  symbol_Name(argsymbol));
      misc_FinishUserErrorReport();
    }
    if (!symbol_IsVariable(argsymbol)) {
      /* Add <argsymbol> to the front of the symbol list */
      pro_SYMBOLLIST = list_PointerDeleteOneElement(pro_SYMBOLLIST,
						    (POINTER)argsymbol);
      pro_SYMBOLLIST = list_Cons((POINTER)argsymbol, pro_SYMBOLLIST);
    }
  }
}

static void pro_AppendRule(TERM Positive, LIST Negatives)
{
  LIST scan;

  for (scan = Negatives; !list_Empty(scan); scan = list_Cdr(scan))
    list_Rplaca(scan, term_Create(fol_Not(), list_List(list_Car(scan))));
  Negatives = list_Cons(Positive, Negatives);
  pro_RULESLIST = list_Cons(term_Create(fol_Or(),Negatives), pro_RULESLIST);
  pro_RestartVarList();
}

static void pro_StoreQuestion(LIST List)
{
  TERM term;
  LIST scan;
  for (scan = List; !list_Empty(scan); scan = list_Cdr(scan))
    list_Rplaca(scan, term_Create(fol_Not(), list_List(list_Car(scan))));
  term = term_Create(fol_Or(), List);
  pro_QUESTIONTERM = term;
  pro_RestartVarList();
}

/**************************************************************/
/* Functions for the Variable Table                           */
/**************************************************************/
  
typedef struct {
  char*  name;
  SYMBOL symbol;
} PRO_VARENTRY, *PRO_VARIABLE;

 char* pro_VarName(PRO_VARIABLE Entry)
{
  return Entry->name;
}

 SYMBOL pro_VarSymbol(PRO_VARIABLE Entry)
{
  return Entry->symbol;
}

static SYMBOL pro_VarCreate(char* Name)
{
  PRO_VARIABLE item = (PRO_VARIABLE) memory_Malloc(sizeof(PRO_VARENTRY));

  item->name   = Name;
  item->symbol = symbol_CreateStandardVariable();

  pro_VARLIST = list_Cons(item, pro_VARLIST);

  return item->symbol;
}

static void pro_VarFree(PRO_VARIABLE Entry)
{
  string_StringFree(Entry->name);
  memory_Free(Entry, sizeof(PRO_VARENTRY));
}

static void pro_RestartVarList(void)
{
  list_DeleteWithElement(pro_VARLIST, (void (*)(POINTER))pro_VarFree);
  pro_VARLIST = list_Nil();
  symbol_ResetStandardVarCounter();
}

static SYMBOL pro_VarLookup(char* Name)
/**************************************************************
  INPUT:   A variable name.
  RETURNS: The corresponding variable symbol.
  EFFECT:  If the variable name occurred before, the corresponding
           symbol is returned and the <Name> is freed.
           If the variable name didn't occur
           a new variable is created.
	   Every occurrence of the special variable "_" generates
           a new variable symbol.
***************************************************************/
{
  LIST scan;

  if (!string_Equal(Name, "_")) {
    for (scan = pro_VARLIST; !list_Empty(scan); scan = list_Cdr(scan)) {
      if (string_Equal(Name, pro_VarName(list_Car(scan)))) {
	string_StringFree(Name);
	return pro_VarSymbol(list_Car(scan));
      }
    }
  }
  /* Create a new variable */
  return pro_VarCreate(Name);
}
