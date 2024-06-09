/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                ANALYSIS OF CLAUSE SETS                 * */
/* *                                                        * */
/* *  $Module:   ANALYZE                                    * */ 
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
/* $Revision: 1.7 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-22 09:05:45 $                             * */
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


/* $RCSfile: analyze.c,v $ */

#include <stdio.h>

#include "analyze.h"

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

static LIST ana_CalculatePredicatePrecedence(LIST, LIST);
static LIST ana_CalculateFunctionPrecedence(ANALYZE, LIST, LIST, FLAGSTORE);

BOOL ana_GetFeature(ANALYZE Ana, ANA_FEATUREID id)
{
  return Ana->boolarray[id];
}

BOOL* ana_GetFeatureAddr(ANALYZE Ana, ANA_FEATUREID id)
{
  return &(Ana->boolarray[id]);
}

void ana_SetFeature(ANALYZE Ana, ANA_FEATUREID id, BOOL newval)
{
  Ana->boolarray[id] = newval;
}

int ana_GetAxiomClauses(ANALYZE Ana)
{
  return Ana->ana_AXIOMCLAUSES;
}

int ana_GetConClauses(ANALYZE Ana)
{
  return Ana->ana_CONCLAUSES;
}

int ana_GetNonHornClauses(ANALYZE Ana)
{
  return Ana->ana_NONHORNCLAUSES;
}

void ana_IncAxiomClauses(ANALYZE Ana)
{
  Ana->ana_AXIOMCLAUSES++;
}

void ana_IncConClauses(ANALYZE Ana)
{
  Ana->ana_CONCLAUSES++;
}

void ana_IncNonHornClauses(ANALYZE Ana)
{
  Ana->ana_NONHORNCLAUSES++;
}

void ana_SetAxiomClauses(ANALYZE Ana, int newval)
{
  Ana->ana_AXIOMCLAUSES = newval;
}

void ana_SetConClauses(ANALYZE Ana, int newval)
{
  Ana->ana_CONCLAUSES = newval;
}

void ana_SetNonHornClauses(ANALYZE Ana, int newval)
{
  Ana->ana_NONHORNCLAUSES = newval;
}

LIST ana_GetFinMonPredList(ANALYZE Ana)
{
  return Ana->ana_FINITEMONADICPREDICATES;
}

void ana_SetFinMonPredList(ANALYZE Ana, LIST newval)
{
  Ana->ana_FINITEMONADICPREDICATES = newval;
}

NAT ana_NonTrivClauseNumber(ANALYZE Ana)
{
  return Ana->nontrivclausenumber;
}

void ana_SetNonTrivClauseNumber(ANALYZE Ana, NAT Number)
{
  Ana->nontrivclausenumber = Number;
}

void ana_AddTransAxClause(ANALYZE Ana,CLAUSE Ax) {
  Ana->transitivityaxioms = list_Cons(Ax,Ana->transitivityaxioms);
}

LIST ana_GetTransAxClauseList(ANALYZE Ana) {
  return Ana->transitivityaxioms;
}

void ana_SetTransAxClauseList(ANALYZE Ana, LIST NewList) {  
  Ana->transitivityaxioms = NewList;  
}

CLAUSE ana_LookupTransAxClause(ANALYZE Ana, SYMBOL s) {
  LIST Scan;
  
  Scan = Ana->transitivityaxioms;
  
  while (!list_Empty(Scan)) {    
    if (symbol_Equal(term_TopSymbol(clause_GetLiteralAtom(list_Car(Scan),0)),s))
      return list_Car(Scan);

    Scan = list_Cdr(Scan);
  }

  return NULL;
}

ANALYZE ana_Init(void)
{
  ANALYZE Ana;
  
  Ana = (ANALYZE)memory_Malloc(sizeof(ANALYZE_NODE));
  
  Ana->ana_FINITEMONADICPREDICATES = list_Nil();  
  Ana->transitivityaxioms = list_Nil();
  Ana->nontrivclausenumber = 0;
  
  return Ana;  
}

void ana_Clean(ANALYZE Ana) 
{
  list_DeleteWithElement(Ana->transitivityaxioms,(void (*)(POINTER))clause_Delete);
  list_Delete(Ana->ana_FINITEMONADICPREDICATES);  
  
  Ana->ana_FINITEMONADICPREDICATES = list_Nil();  
  Ana->transitivityaxioms = list_Nil();
  Ana->nontrivclausenumber = 0;
}

void ana_Free(ANALYZE Ana)
{
  ana_Clean(Ana);
  memory_Free(Ana, sizeof(ANALYZE_NODE));  
}

void ana_AnalyzeProblem(ANALYZE Ana, FLAGSTORE Flags, LIST Clauses)
/**************************************************************
  INPUT:   An analyze object, flagstore, and a list of clauses.
  RETURNS: Void.
  EFFECT:  Analyzes the clauses and sets the analyze variables.
           Recomputes the weight for the clauses.
	   <Ana> is modified according to clauses: non trivial domain number
	   is set
***************************************************************/
{
  CLAUSE Clause;

  ana_SetFeature(Ana,ana_EQUATIONS,FALSE);
  ana_SetFeature(Ana,ana_PEQUATIONS,FALSE); /* Defaults for properties */
  ana_SetFeature(Ana,ana_NEQUATIONS,FALSE);
  ana_SetFeature(Ana,ana_FUNCTIONS,FALSE);
  ana_SetFeature(Ana,ana_FINDOMAIN,FALSE);
  ana_SetFeature(Ana,ana_NONTRIVDOMAIN,FALSE);
  ana_SetFeature(Ana,ana_MONADIC,FALSE);
  ana_SetFeature(Ana,ana_NONMONADIC,FALSE);
  ana_SetFeature(Ana,ana_PROP,FALSE);
  ana_SetFeature(Ana,ana_GROUND,FALSE);
  ana_SetFeature(Ana,ana_SORTRES,FALSE);
  ana_SetFeature(Ana,ana_USORTRES,FALSE);
  ana_SetFeature(Ana,ana_NONUNIT,FALSE);  
  
  ana_SetFeature(Ana,ana_CONGROUND,TRUE);
  ana_SetFeature(Ana,ana_TRANSITIVITY,FALSE);
  
  ana_SetAxiomClauses(Ana,0);
  ana_SetConClauses(Ana,0);
  ana_SetNonHornClauses(Ana,0);
  
  list_Delete(ana_GetFinMonPredList(Ana));
  ana_SetFinMonPredList(Ana,list_Nil());

  if (list_Empty(Clauses))
    return;

  ana_SetFinMonPredList(Ana,clause_FiniteMonadicPredicates(Clauses));
    
  while (!list_Empty(Clauses)) {
    Clause = (CLAUSE)list_Car(Clauses);
    clause_UpdateWeight(Clause, Flags);

    if (clause_GetFlag(Clause,CONCLAUSE))
      ana_IncConClauses(Ana);
    else
      ana_IncAxiomClauses(Ana);

    if (clause_NumOfSuccLits(Clause) > 1)
      ana_IncNonHornClauses(Ana);

    if (ana_GetFeature(Ana,ana_CONGROUND) && clause_GetFlag(Clause,CONCLAUSE) &&
	clause_MaxVar(Clause) != symbol_GetInitialStandardVarCounter())
      ana_SetFeature(Ana,ana_CONGROUND,FALSE);
    if (!ana_GetFeature(Ana,ana_PEQUATIONS) && clause_ContainsPositiveEquations(Clause)) {
      ana_SetFeature(Ana,ana_PEQUATIONS,TRUE);
    }
    if (!ana_GetFeature(Ana,ana_NEQUATIONS) && clause_ContainsNegativeEquations(Clause)) {
      ana_SetFeature(Ana,ana_NEQUATIONS,TRUE);
    }
    if (!ana_GetFeature(Ana,ana_MONADIC) || !ana_GetFeature(Ana,ana_NONMONADIC) || !ana_GetFeature(Ana,ana_PROP) || !ana_GetFeature(Ana,ana_GROUND))
      clause_ContainsFolAtom(Clause,ana_GetFeatureAddr(Ana,ana_PROP),ana_GetFeatureAddr(Ana,ana_GROUND),ana_GetFeatureAddr(Ana,ana_MONADIC),ana_GetFeatureAddr(Ana,ana_NONMONADIC));

    if (!ana_GetFeature(Ana,ana_FUNCTIONS) && clause_ContainsFunctions(Clause)) {
      ana_SetFeature(Ana,ana_FUNCTIONS,TRUE);
    }
    if (!ana_GetFeature(Ana,ana_FINDOMAIN) && clause_ImpliesFiniteDomain(Clause)) {
      ana_SetFeature(Ana,ana_FINDOMAIN,TRUE);
    }
    if (!ana_GetFeature(Ana,ana_NONTRIVDOMAIN) && clause_ImpliesNonTrivialDomain(Clause)) {
      ana_SetNonTrivClauseNumber(Ana, clause_Number(Clause));
      ana_SetFeature(Ana,ana_NONTRIVDOMAIN,TRUE);
    }
    if (!ana_GetFeature(Ana,ana_NONUNIT) && clause_Length(Clause) > 1) {
      ana_SetFeature(Ana,ana_NONUNIT,TRUE);
    }
    if (!ana_GetFeature(Ana,ana_SORTRES) || !ana_GetFeature(Ana,ana_USORTRES)) 
      clause_ContainsSortRestriction(Clause,ana_GetFeatureAddr(Ana,ana_SORTRES),ana_GetFeatureAddr(Ana,ana_USORTRES));

    if (!ana_GetFeature(Ana,ana_TRANSITIVITY) && clause_IsTransitivityAxiom(Clause))
        ana_SetFeature(Ana,ana_TRANSITIVITY,TRUE);

    Clauses = list_Cdr(Clauses);
  }

  ana_SetFeature(Ana,ana_PUREEQUATIONAL,((ana_GetFeature(Ana,ana_PEQUATIONS) || ana_GetFeature(Ana,ana_NEQUATIONS)) &&
                                        !ana_GetFeature(Ana,ana_MONADIC) &&
                                        !ana_GetFeature(Ana,ana_NONMONADIC) &&
                                        !ana_GetFeature(Ana,ana_PROP) &&
                                        !ana_GetFeature(Ana,ana_GROUND)));    
  ana_SetFeature(Ana,ana_EQUATIONS,(ana_GetFeature(Ana,ana_PEQUATIONS) || ana_GetFeature(Ana,ana_NEQUATIONS)));
  ana_SetFeature(Ana,ana_PUREPROPOSITIONAL,(!ana_GetFeature(Ana,ana_PEQUATIONS) && 
                                            !ana_GetFeature(Ana,ana_NEQUATIONS) &&
                                            !ana_GetFeature(Ana,ana_MONADIC) &&
                                            !ana_GetFeature(Ana,ana_NONMONADIC) &&
                                            ana_GetFeature(Ana,ana_PROP)));
}


void ana_AnalyzeSortStructure(ANALYZE Ana, LIST Clauses, FLAGSTORE Flags,
			      PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A list of clauses, a flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  The sort structure with respect to equations is analyzed.
           It is detected whether all equations are many sorted or
	   sort decreasing.
***************************************************************/
{
  if (ana_GetFeature(Ana,ana_PEQUATIONS) && ana_GetFeature(Ana,ana_SORTRES)) {
    STR Result;
    Result                = sort_AnalyzeSortStructure(Clauses,Flags,Precedence);
    ana_SetFeature(Ana,ana_SORTMANYEQUATIONS,(Result == SORTEQMANY));
    ana_SetFeature(Ana,ana_SORTDECEQUATIONS,(Result == SORTEQMANY || Result == SORTEQDECR));
  }
}

 void ana_ProcessTranstivityAxiom(ANALYZE Ana, CLAUSE Clause, SYMBOL s, BOOL twisted) {  
  /* twisted means the axiom clause premise literals are in non-standard order */  
  if (twisted) {
    LITERAL lit1, lit2;
    lit1 = clause_GetLiteral(Clause,0);
    lit2 = clause_GetLiteral(Clause,1);
    
    clause_SetLiteral(Clause,0,lit2);
    clause_SetLiteral(Clause,1,lit1);
  }   
  
  if (symbol_HasProperty(s,TRANSITIVE)) { /*duplicated axiom*/
    clause_Delete(Clause);
  } else {
    clause_SetFlag(Clause,TRANSAX);   
    ana_AddTransAxClause(Ana,Clause);    
    symbol_AddProperty(s,TRANSITIVE);   
  }
}

LIST ana_AnalyzeTransitivity(ANALYZE Ana, LIST Clauses)
/**************************************************************
  INPUT: An analyze object and a list of problem clauses.
  RETURNS: Updated list of problem clauses.
  EFFECT: The list of clauses is scanned and 
          and if a clause in
          the form of a transitivity axiom is found
          the respective symbol is remembered as transitive.
          
          Such a clause is deleted from the list and stored in 
          a special place in <Ana>
          
          Also, if the clause found is an axiom of the twisted form
          (i.e. y S z , x S y -> x S z)
          it is set right.
***************************************************************/
{  
  SYMBOL s;
  BOOL twisted;  
  LIST   Scan1,Scan2;

  while (!list_Empty(Clauses) && clause_IsTransitivityAxiomExt(list_Car(Clauses),&s,&twisted)) {     
    Scan1 = list_Cdr(Clauses);
    ana_ProcessTranstivityAxiom(Ana,list_Car(Clauses),s,twisted);
    list_Free(Clauses);
    Clauses = Scan1;
  }

  if (list_Empty(Clauses))
    return list_Nil();
  
  Scan2 = Clauses;
  Scan1 = list_Cdr(Clauses);

  while (!list_Empty(Scan1)) {
    if (clause_IsTransitivityAxiomExt(list_Car(Scan1),&s,&twisted)) {
      ana_ProcessTranstivityAxiom(Ana,list_Car(Scan1),s,twisted);   
      list_Rplacd(Scan2, list_Cdr(Scan1));
      list_Free(Scan1);
      Scan1 = list_Cdr(Scan2);
    }
    else {
      Scan2 = Scan1;
      Scan1 = list_Cdr(Scan1);
    }
  }
  return Clauses;
}

void ana_Print(ANALYZE Ana, FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An analyze object, a flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  The results of an analysis stored in the module variables
           is printed to stdout.
***************************************************************/
{
  const char* Horn;

  if (ana_GetNonHornClauses(Ana) == 0)
    Horn = "Horn";
  else
    Horn = "Non-Horn";
  
  if (ana_GetFeature(Ana,ana_MONADIC) && !ana_GetFeature(Ana,ana_NONMONADIC)) {
    printf("\n This is a monadic %s problem",Horn);
    if (ana_GetFeature(Ana,ana_PEQUATIONS) || ana_GetFeature(Ana,ana_NEQUATIONS))
      fputs(" with equality.", stdout);
    else
      fputs(" without equality.", stdout);
  }
  else
    if (ana_GetFeature(Ana,ana_PEQUATIONS) || ana_GetFeature(Ana,ana_NEQUATIONS)) {
      if (ana_GetFeature(Ana,ana_NONMONADIC) || ana_GetFeature(Ana,ana_MONADIC) || ana_GetFeature(Ana,ana_PROP))
	printf("\n This is a first-order %s problem containing equality.",Horn);
      else
	if (ana_GetFeature(Ana,ana_NONUNIT))
	  printf("\n This is a pure equality %s problem.",Horn);
	else
	  fputs("\n This is a unit equality problem.", stdout);
    }
    else
      if (ana_GetFeature(Ana,ana_NONMONADIC) || ana_GetFeature(Ana,ana_MONADIC))
	printf("\n This is a first-order %s problem without equality.",Horn);

  if (ana_GetFeature(Ana,ana_PUREPROPOSITIONAL))
    printf("\n This is a propositional %s problem.",Horn);
  else
    if (ana_GetFeature(Ana,ana_FINDOMAIN) || !ana_GetFeature(Ana,ana_FUNCTIONS)) {
      fputs("\n This is a problem that has, if any, a finite domain model.",
	    stdout);
      if (ana_GetFeature(Ana,ana_FINDOMAIN))
	fputs("\n There is a finite domain clause.", stdout);
      if (!ana_GetFeature(Ana,ana_FUNCTIONS))
	fputs("\n There are no function symbols.", stdout);
    }

  if (ana_GetFeature(Ana,ana_NONTRIVDOMAIN))
    fputs("\n This is a problem that has, if any, a non-trivial domain model.",
	  stdout);
      
  
  if (ana_GetFeature(Ana,ana_SORTRES)) {
    fputs("\n This is a problem that contains sort information.", stdout);
    if (ana_GetFeature(Ana,ana_PEQUATIONS)) {
      if (ana_GetFeature(Ana,ana_SORTMANYEQUATIONS))
	fputs("\n All equations are many sorted.", stdout);
      else {
	if (ana_GetFeature(Ana,ana_SORTDECEQUATIONS))
	  fputs("\n All equations are sort-decreasing.", stdout);
      }
    }
  }

  if (ana_GetConClauses(Ana) > 0 && ana_GetFeature(Ana,ana_CONGROUND) && !ana_GetFeature(Ana,ana_PUREPROPOSITIONAL))
    fputs("\n The conjecture is ground.", stdout);

    if (ana_GetFeature(Ana,ana_TRANSITIVITY))
      fputs("\n The problem contains transitivity axiom(s).", stdout);
    
  if (!list_Empty(ana_GetFinMonPredList(Ana))) {
    LIST Scan;
    fputs("\n The following monadic predicates have finite extensions: ", stdout);
    for (Scan=ana_GetFinMonPredList(Ana);!list_Empty(Scan);Scan=list_Cdr(Scan)) {
      symbol_Print((SYMBOL)list_Car(Scan));
      if (!list_Empty(list_Cdr(Scan)))
	fputs(", ", stdout);
    }
    putchar('.');
  }

  printf("\n Axiom clauses: %d Conjecture clauses: %d",ana_GetAxiomClauses(Ana),ana_GetConClauses(Ana));

  flag_PrintInferenceRules(Flags);
  flag_PrintReductionRules(Flags);
  fputs("\n Extras    : ", stdout);
  if (flag_GetFlagIntValue(Flags, flag_SATINPUT))
    fputs("Input Saturation, ", stdout);
  else
    fputs("No Input Saturation, ", stdout);
  if (flag_GetFlagIntValue(Flags, flag_SELECT) == flag_SELECTOFF)
    fputs("No Selection, ", stdout);
  else if (flag_GetFlagIntValue(Flags, flag_SELECT) == flag_SELECTIFSEVERALMAXIMAL)
    fputs("Dynamic Selection, ", stdout);
  else if (flag_GetFlagIntValue(Flags, flag_SELECT) == flag_SELECTALWAYS)
    fputs("Always Selection, ", stdout);
  else   
    fputs("Selection From List, ", stdout);
  if (flag_GetFlagIntValue(Flags, flag_SPLITS) == flag_SPLITSUNLIMITED)
    fputs("Full Splitting, ", stdout);
  else
    if (flag_GetFlagIntValue(Flags, flag_SPLITS) == flag_SPLITSOFF)
      fputs("No Splitting, ", stdout);
    else
      printf("Maximum of %d Splits, ",flag_GetFlagIntValue(Flags, flag_SPLITS));
  if (flag_GetFlagIntValue(Flags,flag_Transitivity)) {
    fputs("Chaining, ", stdout); /* Meaning: Transitivity Analyzed (labelling symbols and clauses for special use),
                                             Removed Transitivity Axioms,
                                             Subterm Chaining Restriction of Superposition,
                                             Literal Ordering Admissible for Chaining */
  }    
  if (flag_GetFlagIntValue(Flags, flag_FULLRED))
    fputs("Full Reduction, ", stdout);
  else
    fputs("Lazy Reduction, ", stdout);
  printf(" Ratio: %d, FuncWeight: %d, VarWeight: %d",
	 flag_GetFlagIntValue(Flags, flag_WDRATIO),
	 flag_GetFlagIntValue(Flags, flag_FUNCWEIGHT),
	 flag_GetFlagIntValue(Flags, flag_VARWEIGHT));
  fputs("\n Precedence: ", stdout);
  fol_PrintPrecedence(Precedence);
  fputs("\n Ordering  : ", stdout);
  fputs(flag_GetFlagIntValue(Flags, flag_ORD) == flag_ORDKBO ? "KBO" : "RPOS",
	stdout);

  if (flag_GetFlagIntValue(Flags,flag_Transitivity)) {
    LIST Scan;
    
    fputs("\n Chaining predicates: ", stdout);
    Scan = ana_GetTransAxClauseList(Ana);
    while (!list_Empty(Scan)) {
      fputs(symbol_Name(term_TopSymbol(clause_GetLiteralAtom(list_Car(Scan),0))), stdout);
      
      Scan = list_Cdr(Scan);
      
      if (!list_Empty(Scan))
        fputs(", ", stdout);
    }
  }
}


void ana_AutoConfiguration(ANALYZE Ana, LIST Clauses, FLAGSTORE Flags,
			   PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A list of clauses, a flag store and a precedence.
  RETURNS: Nothing.
  EFFECT:  Based on the values of the ana analysis module, an appropriate
           complete configuration of inference, reduction rules and other
	   settings is established.
***************************************************************/
{
  LIST Scan, Functions, Predicates, Constants;

  Functions  = symbol_GetAllFunctions();
  Predicates = fol_GetNonFOLPredicates();

  /* Set precedence */
  Predicates = ana_CalculatePredicatePrecedence(Predicates, Clauses);
  Functions  = ana_CalculateFunctionPrecedence(Ana, Functions, Clauses, Flags);
  Constants  = list_Nil();

  for (Scan=Functions; !list_Empty(Scan); Scan=list_Cdr(Scan))
    if (symbol_IsConstant((SYMBOL)list_Car(Scan)))
      Constants = list_Cons(list_Car(Scan),Constants);
  Functions = list_NPointerDifference(Functions,Constants);
  Constants = list_NReverse(Constants);

  for ( ; !list_Empty(Functions); Functions = list_Pop(Functions))
    symbol_SetIncreasedOrdering(Precedence, (SYMBOL)list_Car(Functions));
  /* Predicates < Functions */
  for ( ; !list_Empty(Predicates); Predicates = list_Pop(Predicates))
    symbol_SetIncreasedOrdering(Precedence, (SYMBOL)list_Car(Predicates));
  /* Constants < Predicates */
  /* Predicates < Functions */
  for ( ; !list_Empty(Constants); Constants = list_Pop(Constants))
    symbol_SetIncreasedOrdering(Precedence, (SYMBOL)list_Car(Constants));

  flag_ClearInferenceRules(Flags);
  flag_ClearReductionRules(Flags);

  flag_SetFlagIntValue(Flags, flag_ROBV,    flag_ROBVON);
  flag_SetFlagIntValue(Flags, flag_RTAUT,   flag_RTAUTSYNTACTIC);
  flag_SetFlagIntValue(Flags, flag_RFSUB,   flag_RFSUBON);
  flag_SetFlagIntValue(Flags, flag_RBSUB,   flag_RBSUBON);
  flag_SetFlagIntValue(Flags, flag_RFMRR,   flag_RFMRRON);
  flag_SetFlagIntValue(Flags, flag_RBMRR,   flag_RBMRRON);
  flag_SetFlagIntValue(Flags, flag_RUNC,    flag_RUNCON);
  flag_SetFlagIntValue(Flags, flag_FULLRED, flag_FULLREDON);
  /*flag_SetFlagIntValue(Flags, flag_FUNCWEIGHT,1);
    flag_SetFlagIntValue(Flags, flag_VARWEIGHT,1);*/
  flag_SetFlagIntValue(Flags, flag_WDRATIO,5);

  if (ana_GetFeature(Ana,ana_TRANSITIVITY)) {
    flag_SetFlagIntValue(Flags,flag_Transitivity,flag_TRANSON);
    flag_SetFlagIntValue(Flags, flag_IChain, flag_CHAININGON);
    if (ana_GetNonHornClauses(Ana) > 0)
      flag_SetFlagIntValue(Flags, flag_ICompRes, flag_COMPOSITIONRESOLUTIONON);       
  }
  
  if (ana_GetFeature(Ana,ana_NEQUATIONS)) {
    flag_SetFlagIntValue(Flags, flag_IEQR, flag_EQUALITYRESOLUTIONON);
    if (ana_GetFeature(Ana,ana_NONUNIT)) {
      if (ana_GetFeature(Ana,ana_NONTRIVDOMAIN))
	flag_SetFlagIntValue(Flags, flag_RAED, flag_RAEDPOTUNSOUND);
      else
	flag_SetFlagIntValue(Flags, flag_RAED, flag_RAEDSOUND);
    }
  }

  if (ana_GetFeature(Ana,ana_PEQUATIONS)) {
    flag_SetFlagIntValue(Flags, flag_ISPR, flag_SUPERPOSITIONRIGHTON);
    flag_SetFlagIntValue(Flags, flag_ISPL, flag_SUPERPOSITIONLEFTON);
    if (ana_GetNonHornClauses(Ana) > 0)
      flag_SetFlagIntValue(Flags, flag_IEQF, flag_EQUALITYFACTORINGON);
    if (ana_GetFeature(Ana,ana_NONUNIT)) 
      flag_SetFlagIntValue(Flags, flag_RCON, flag_RCONON);
    flag_SetFlagIntValue(Flags, flag_RFREW, flag_RFREWON);
    flag_SetFlagIntValue(Flags, flag_RBREW, flag_RBREWON);
  }
  
  if (ana_GetFeature(Ana,ana_SORTRES)) {
    flag_SetFlagIntValue(Flags, flag_SORTS, flag_SORTSMONADICWITHVARIABLE);
    flag_SetFlagIntValue(Flags, flag_IEMS,  flag_EMPTYSORTON);
    flag_SetFlagIntValue(Flags, flag_ISOR,  flag_SORTRESOLUTIONON);
    flag_SetFlagIntValue(Flags, flag_RSSI, flag_RSSION);
    if (!ana_GetFeature(Ana,ana_PEQUATIONS) || ana_GetFeature(Ana,ana_SORTMANYEQUATIONS))
      flag_SetFlagIntValue(Flags, flag_RSST, flag_RSSTON);
  }
  else
    flag_SetFlagIntValue(Flags, flag_SORTS, flag_SORTSOFF);

  if (ana_GetFeature(Ana,ana_MONADIC) || ana_GetFeature(Ana,ana_NONMONADIC)) { /* Problem contains real predicates */
    flag_SetFlagIntValue(Flags, flag_IORE, flag_ORDEREDRESOLUTIONNOEQUATIONS);
    if (ana_GetNonHornClauses(Ana) > 0)
      flag_SetFlagIntValue(Flags, flag_IOFC, flag_FACTORINGONLYRIGHT);
    if (ana_GetFeature(Ana,ana_NONUNIT)) 
      flag_SetFlagIntValue(Flags, flag_RCON, flag_RCONON);
  }

  if (!ana_GetFeature(Ana,ana_FUNCTIONS))
    flag_SetFlagIntValue(Flags, flag_SELECT, flag_SELECTALWAYS);
  else
    if (ana_GetFeature(Ana,ana_NONUNIT))
      flag_SetFlagIntValue(Flags, flag_SELECT, flag_SELECTIFSEVERALMAXIMAL);
    else
      flag_SetFlagIntValue(Flags, flag_SELECT, flag_SELECTOFF);

  if (ana_GetConClauses(Ana) < ana_GetAxiomClauses(Ana) || (ana_GetFeature(Ana,ana_CONGROUND) && !ana_GetFeature(Ana,ana_PUREPROPOSITIONAL)))
    flag_SetFlagIntValue(Flags, flag_SATINPUT, flag_SATINPUTON);
  else
    flag_SetFlagIntValue(Flags, flag_SATINPUT, flag_SATINPUTOFF);

  if (ana_GetNonHornClauses(Ana) > 0)
    flag_SetFlagIntValue(Flags, flag_SPLITS, flag_SPLITSUNLIMITED);
  else
    flag_SetFlagIntValue(Flags, flag_SPLITS, flag_SPLITSOFF);

  /*   if (ana_GetFeature(Ana,ana_PUREPROPOSITIONAL)) */
  /*     flag_SetFlagIntValue(Flags, flag_SPLITHEURISTIC, flag_SPLITHEURISTICALWAYS); */
}


void ana_ExploitSortAnalysis(ANALYZE Ana, FLAGSTORE Flags)
/**************************************************************
  INPUT:   A flag store.
  EFFECT:  If all equations are many sorted and or no positive
           equations occur at all and the problem contains sort
	   information, static soft typing is activated.
***************************************************************/
{
  if (ana_GetFeature(Ana,ana_SORTRES) && (!ana_GetFeature(Ana,ana_PEQUATIONS) || ana_GetFeature(Ana,ana_SORTMANYEQUATIONS)))
    flag_SetFlagIntValue(Flags, flag_RSST, flag_RSSTON);
}


static LIST ana_CalculatePredicatePrecedence(LIST Predicates, LIST Clauses)
/**************************************************************
  INPUT:   A list of predicates and a list of clauses.
  RETURNS: A list of predicate symbols, which should be used
           for setting the symbol precedence. The list is sorted
           in descending order, that means predicates with highest
           precedence come first.
  EFFECT:  Analyze the clause list to build a directed graph G where
           the predicates are vertices. There's an edge (P,Q) in
           G iff a clause exists where P is a negative literal
           and Q is a positive literal and P != Q. Apply DFS to
           find the strongly connected components of this graph.
	   The <Predicates> list is deleted.
  CAUTION: The predicate list must contain ALL predicates
           occurring in the clause list!
***************************************************************/
{
  GRAPH  graph;
  LIST   result, scan;
  int    i, j;
  NAT    count;
  SYMBOL s;

  /* clause_ListPrint(Clauses); DBG */

  if (list_Empty(Predicates)) {
    return Predicates;
  }

  graph = graph_Create();

  /* First create the nodes: one node for every predicate symbol. */
  for ( ; !list_Empty(Predicates); Predicates = list_Pop(Predicates))
    graph_AddNode(graph, symbol_Index((SYMBOL)list_Car(Predicates)));

  /* Now scan the clause clause list to create the edges */
  /* An edge (P,Q) means P is smaller than Q */
  for (scan = Clauses; !list_Empty(scan); scan = list_Cdr(scan)) {
    CLAUSE c = list_Car(scan);

    for (i = clause_FirstLitIndex(); i < clause_FirstSuccedentLitIndex(c); i++) {
      SYMBOL negPred = term_TopSymbol(clause_GetLiteralAtom(c, i));
      if (!symbol_Equal(negPred, fol_Equality())) { /* negative predicate */
	for (j = clause_FirstSuccedentLitIndex(c); j < clause_Length(c); j++) {
	  SYMBOL posPred = term_TopSymbol(clause_GetLiteralAtom(c, j));
	  if (!symbol_Equal(posPred, fol_Equality()) && /* positive predicate */
	      negPred != posPred) {  /* No self loops! */
	    graph_AddEdge(graph_GetNode(graph, symbol_Index(negPred)),
			  graph_GetNode(graph, symbol_Index(posPred)));
	  }
	}
      }
    }
  }

  /* graph_Print(graph); fflush(stdout); DBG */

  /* Calculate the strongly connected components of the graph */
  count = graph_StronglyConnectedComponents(graph);

  /* Now create the precedence list by scanning the nodes.        */
  /* If there's a link between two strongly connected components  */
  /* c1 and c2 then component_num(c1) > component_num(c2), so the */
  /* following code creates a valid precedence list in descending */
  /* order.                                                       */
  result = list_Nil();
  for (i = count - 1; i >= 0; i--) {
    for (scan = graph_Nodes(graph); !list_Empty(scan); scan = list_Cdr(scan)) {
      GRAPHNODE n = list_Car(scan);
      if (graph_NodeCompNum(n) == i) {
	/* The symbol represented by the node <<n> belongs to component <i> */
	s = symbol_GetSigSymbol(graph_NodeNumber(n));
	result = list_Cons((POINTER)s, result);
      }
    }
  }

  /* putchar('\n');
     for (scan = result; !list_Empty(scan); scan = list_Cdr(scan)) {
     s = (SYMBOL) list_Car(scan);
     symbol_Print(s);
     putchar(' ');
     }
     putchar('\n'); fflush(stdout); DBG */

  graph_Delete(graph);

  return result;
}


/* We use the node info to store the degree of the node */
 NAT ana_NodeDegree(GRAPHNODE Node)
{
  return (NAT)graph_NodeInfo(Node);
}


 void ana_IncNodeDegree(GRAPHNODE Node)
{
  graph_NodeSetInfo(Node, (POINTER)(ana_NodeDegree(Node)+1));
}

static BOOL ana_NodeGreater(GRAPHNODE N1, GRAPHNODE N2)
/**************************************************************
  INPUT:   Two graph nodes.
  RETURNS: TRUE, if N1 is greater than N2.
  EFFECT:  This function is used to sort the node list
           of the graph in ana_CalculateFunctionPrecedence.
***************************************************************/
{
  return (symbol_Arity(symbol_GetSigSymbol(graph_NodeNumber(N1))) >
	  symbol_Arity(symbol_GetSigSymbol(graph_NodeNumber(N2))));
}


static BOOL ana_BidirectionalDistributivity(LIST SymbolPairs)
/**************************************************************
  INPUT:   A list of symbol pairs defining distributivity.
  RETURNS: TRUE, if the list contains two pairs (s1, s2) and (s2, s1)
           FALSE otherwise.
  EFFECT:  This function is used to detect symbols that are distributive
           in both directions, logical OR and AND for example.
***************************************************************/
{
  LIST scan, actPair, nextPair;

  for ( ; !list_Empty(SymbolPairs); SymbolPairs = list_Cdr(SymbolPairs)) {
    actPair = list_Car(SymbolPairs);
    /* If actPair = (s1, s2), check whether there's a pair (s2, s1) in list */
    for (scan = list_Cdr(SymbolPairs); !list_Empty(scan); scan = list_Cdr(scan)) {
      nextPair = list_Car(scan);
      if (symbol_Equal((SYMBOL)list_PairFirst(actPair),(SYMBOL)list_PairSecond(nextPair)) &&
	  symbol_Equal((SYMBOL)list_PairSecond(actPair),(SYMBOL)list_PairFirst(nextPair)))
	return TRUE;
    }
  }
  return FALSE;
}


static LIST ana_CalculateFunctionPrecedence(ANALYZE Ana, LIST Functions, LIST Clauses,
					    FLAGSTORE Flags)
/**************************************************************
  INPUT:   An analyze object, a list of functions, a list of clauses and 
           a flag store.
  RETURNS: A list of function symbols, which should be used
           for setting the symbol precedence. The list is sorted
           in descending order, that means function with highest
           precedence come first.
  EFFECT:  Analyzes the clauses to build a directed graph G with
           function symbol as nodes. An edge (f,g) or in G means
           f should have lower precedence than g.
           An edge (f,g) or (g,f) is created if there's an equation
           equal(f(...), g(...)) in the clause list.
	   The direction of the edge depends on the degree of the
           nodes and the symbol arity.
	   Then find the strongly connected components of this
           graph.
           The "Ordering" flag will be set in the flag store.
  CAUTION: The value of "ana_PEQUATIONS" must be up to date.
***************************************************************/
{
  GRAPH     graph;
  GRAPHNODE n1, n2;
  LIST      result, scan, scan2, distrPairs;
  int       i, j;
  SYMBOL    s, Add, Mult;

  if (list_Empty(Functions))
    return Functions;   /* Problem contains no functions */
  else if (!ana_GetFeature(Ana,ana_PEQUATIONS)) {
    Functions = list_NumberSort(Functions, (NAT (*)(POINTER)) symbol_PositiveArity);
    return Functions;
  }

  graph = graph_Create();
  /* First create the nodes: one node for every function symbol. */
  for (; !list_Empty(Functions); Functions = list_Pop(Functions))
    graph_AddNode(graph, symbol_Index((SYMBOL)list_Car(Functions)));

  /* Now sort the node list wrt descending symbol arity. */
  graph_SortNodes(graph, ana_NodeGreater);

  /* A list of pairs (add, multiply) of distributive symbols */
  distrPairs = list_Nil();

  /* Now add undirected edges: there's an undirected edge between  */
  /* two nodes if the symbols occur as top symbols in a positive   */
  /* equation. */
  for (scan = Clauses; !list_Empty(scan); scan = list_Cdr(scan)) {
    CLAUSE c = list_Car(scan);
    for (i = clause_FirstSuccedentLitIndex(c);
	 i <= clause_LastSuccedentLitIndex(c); i++) {
      if (clause_LiteralIsEquality(clause_GetLiteral(c, i))) {
	/* Consider only positive equations */
	TERM t1, t2;

	if (fol_DistributiveEquation(clause_GetLiteralAtom(c,i), &Add, &Mult)) {
	  /* Add a pair (Add, Mult) to <distrTerms> */
	  distrPairs = list_Cons(list_PairCreate((POINTER)Add, (POINTER)Mult),
				 distrPairs);
	  /*fputs("\nDISTRIBUTIVITY: ", stdout);
	    term_PrintPrefix(clause_GetLiteralAtom(c,i));
	    fputs(" Add=", stdout); symbol_Print(Add);
	    fputs(" Mult=", stdout); symbol_Print(Mult); fflush(stdout); DBG */
	}

	t1 = term_FirstArgument(clause_GetLiteralAtom(c, i));
	t2 = term_SecondArgument(clause_GetLiteralAtom(c, i));

	if  (!term_IsVariable(t1) && !term_IsVariable(t2) &&
	     !term_EqualTopSymbols(t1, t2) &&  /* No self loops! */
	     !term_HasSubterm(t1, t2) &&       /* No subterm property */
	     !term_HasSubterm(t2, t1)) {
	  n1 = graph_GetNode(graph, symbol_Index(term_TopSymbol(t1)));
	  n2 = graph_GetNode(graph, symbol_Index(term_TopSymbol(t2)));
	  /* Create an undirected edge by adding two directed edges */
	  graph_AddEdge(n1, n2);
	  graph_AddEdge(n2, n1);
	  /* Use the node info for the degree of the node */
	  ana_IncNodeDegree(n1);
	  ana_IncNodeDegree(n2);
	}
      }
    }
  }
  
  /* putchar('\n');
     for (scan = graph_Nodes(graph); !list_Empty(scan); scan = list_Cdr(scan)) {
     n1 = list_Car(scan);
     printf("(%s,%d,%u), ",
     symbol_Name(symbol_GetSigSymbol(graph_NodeNumber(n1))),
     graph_NodeNumber(n1), ana_NodeDegree(n1));
     }
     graph_Print(graph); fflush(stdout); DBG */

  graph_DeleteDuplicateEdges(graph);
  
  /* Transform the undirected graph into a directed graph. */
  for (scan = graph_Nodes(graph); !list_Empty(scan); scan = list_Cdr(scan)) {
    n1 = list_Car(scan);
    result = list_Nil(); /* Collect edges from n1 that shall be deleted */ 
    for (scan2 = graph_NodeNeighbors(n1); !list_Empty(scan2);
	 scan2 = list_Cdr(scan2)) {
      int a1, a2;
      n2 = list_Car(scan2);
      /* Get the node degrees in the undirected graph with multiple edges */
      i  = ana_NodeDegree(n1);
      j  = ana_NodeDegree(n2);
      a1 = symbol_Arity(symbol_GetSigSymbol(graph_NodeNumber(n1)));
      a2 = symbol_Arity(symbol_GetSigSymbol(graph_NodeNumber(n2)));

      if (i > j || (i==j && a1 >= a2)) {
	/* symbol2 <= symbol1, so remove edge n1 -> n2 */
	result = list_Cons(n2, result);
      }
      if (i < j || (i==j && a1 <= a2)) {
	/* symbol1 <= symbol2, so remove edge n2 -> n1 */
	graph_DeleteEdge(n2, n1);
      }
      /* NOTE: If (i==j && a1==a2) both edges are deleted! */
    }
    /* Now delete edges from n1 */
    for ( ; !list_Empty(result); result = list_Pop(result))
      graph_DeleteEdge(n1, list_Car(result));
  }

  if (!list_Empty(distrPairs) && !ana_BidirectionalDistributivity(distrPairs)) {
    /* Enable RPO ordering, otherwise the default KBO will be used. */
    flag_SetFlagIntValue(Flags, flag_ORD, flag_ORDRPOS);
  }

  /* Now examine the list of distribute symbols */
  /* since they've highest priority.                  */
  for ( ; !list_Empty(distrPairs); distrPairs = list_Pop(distrPairs)) {
    scan = list_Car(distrPairs); /* A pair (Add, Mult) */
    /* Addition */
    n1 = graph_GetNode(graph,
		       symbol_Index((SYMBOL)list_PairFirst(scan)));
    /* Multiplication */
    n2 = graph_GetNode(graph, 
		       symbol_Index((SYMBOL)list_PairSecond(scan)));
    /* Remove any edges between n1 and n2 */
    graph_DeleteEdge(n1, n2);
    graph_DeleteEdge(n2, n1);
    /* Add one edge Addition -> Multiplication */
    graph_AddEdge(n1, n2);
    list_PairFree(scan);
  }

  /* fputs("\n------------------------",stdout);
     graph_Print(graph); fflush(stdout); DBG */

  /* Calculate the strongly connected components of the graph. */
  /* <i> is the number of SCCs. */
  i = graph_StronglyConnectedComponents(graph);

  /* Now create the precedence list by scanning the nodes.        */
  /* If there's a link between two strongly connected components  */
  /* c1 and c2 then component_num(c1) > component_num(c2), so the */
  /* following code creates a valid precedence list in descending */
  /* order.                                                       */
  result = list_Nil();
  while (i-- > 0) {   /* for i = numberOfSCCs -1 dowto 0 */
    for (scan = graph_Nodes(graph); !list_Empty(scan); scan = list_Cdr(scan)) {
      n1 = list_Car(scan);
      if (graph_NodeCompNum(n1) == i) {
	/* The symbol represented by the node <n> belongs to component <i> */
	s = symbol_GetSigSymbol(graph_NodeNumber(n1));
	result = list_Cons((POINTER)s, result);
      }
    }
  }

  /* putchar('\n');
     for (scan = result; !list_Empty(scan); scan = list_Cdr(scan)) {
     s = (SYMBOL) list_Car(scan);
     symbol_Print(s);
     fputs(" > ", stdout);
     }
     putchar('\n'); fflush(stdout); DBG */

  graph_Delete(graph);

  return result;
}
