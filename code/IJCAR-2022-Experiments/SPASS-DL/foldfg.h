/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *              FIRST ORDER LOGIC SYMBOLS                 * */
/* *                                                        * */
/* *  $Module:   FOL      DFG                               * */ 
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
/* $Revision: 1.10 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2016-03-23 13:33:28 $                             * */
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


/* $RCSfile: foldfg.h,v $ */

#ifndef _FOLDFG_
#define _FOLDFG_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "flags.h"
#include "unify.h"
#include "context.h"
#include "term.h"

/**************************************************************/
/* Global Variables and Constants (Only seen by macros)       */
/**************************************************************/

extern SYMBOL  fol_ALL;
extern SYMBOL  fol_EXIST;
extern SYMBOL  fol_AND;
extern SYMBOL  fol_OR;
extern SYMBOL  fol_NOT;
extern SYMBOL  fol_IMPLIES;
extern SYMBOL  fol_IMPLIED;
extern SYMBOL  fol_EQUIV;
extern SYMBOL  fol_VARLIST;
extern SYMBOL  fol_EQUALITY;
extern SYMBOL  fol_NONEQUALITY;
extern SYMBOL  fol_TRUE;
extern SYMBOL  fol_FALSE;

extern SYMBOL  fol_XOR;
extern SYMBOL  fol_NOR;
extern SYMBOL  fol_NAND;

/**************************************************************/
/* Access to the first-order symbols.                         */
/**************************************************************/

SYMBOL fol_All(void);

SYMBOL fol_Exist(void);

SYMBOL fol_And(void);

SYMBOL fol_Or(void);

SYMBOL fol_Not(void);

SYMBOL fol_Implies(void);

SYMBOL fol_Implied(void);

SYMBOL fol_Equiv(void);

SYMBOL fol_Xor(void);

SYMBOL fol_Nor(void);

SYMBOL fol_Nand(void);

SYMBOL fol_Varlist(void);

SYMBOL fol_Equality(void);

SYMBOL fol_NonEquality(void);

SYMBOL fol_True(void);

SYMBOL fol_False(void);

/**************************************************************/
/* Macros                                                     */
/**************************************************************/

  BOOL fol_IsQuantifier(SYMBOL S);

  BOOL fol_IsTrue(TERM S);

  BOOL fol_IsFalse(TERM S);

   LIST fol_QuantifierVariables(TERM T);

  BOOL fol_IsNegativeLiteral(TERM T);

  BOOL fol_IsJunctor(SYMBOL S);

  TERM fol_Atom(TERM Lit);

  BOOL fol_IsEquality(TERM Term);

  BOOL fol_IsAssignment(TERM Term);

  LIST fol_DeleteFalseTermFromList(LIST List);

  LIST fol_DeleteTrueTermFromList(LIST List);

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

void   fol_Init(BOOL, PRECEDENCE);
BOOL   fol_IsLiteral(TERM T);
SYMBOL fol_IsStringPredefined(const char*);
TERM   fol_CreateQuantifier(SYMBOL, LIST, LIST);
TERM   fol_CreateQuantifierAddFather(SYMBOL, LIST, LIST);
LIST   fol_GetNonFOLPredicates(void);
TERM   fol_ComplementaryTerm(TERM);
LIST   fol_GetAssignments(TERM);
void   fol_Free(void);
void   fol_CheckFatherLinks(TERM);
BOOL   fol_FormulaIsClause(TERM);
void   fol_FPrintOtterOptions(FILE*, BOOL, FLAG_TDFG2OTTEROPTIONSTYPE);
void   fol_FPrintOtter(FILE*, LIST, FLAG_TDFG2OTTEROPTIONSTYPE);
void   fol_PrettyPrintDFG(TERM);
void   fol_PrintDFG(TERM);
void   fol_FPrintDFG(FILE*, TERM);
void   fol_FPrintDFGProblem(FILE*, const char*, const char*, const char*, const char*, LIST, LIST);
LIST   fol_GetPrecedence(PRECEDENCE);
void   fol_PrintPrecedence(PRECEDENCE);
void   fol_FPrintPrecedence(FILE*, PRECEDENCE);
LIST   fol_Instances(TERM, TERM);
LIST   fol_Generalizations(TERM, TERM);
TERM   fol_MostGeneralFormula(LIST);
void   fol_NormalizeVars(TERM);
void   fol_NormalizeVarsStartingAt(TERM, SYMBOL);
LIST   fol_FreeVariables(TERM);
LIST   fol_BoundVariables(TERM);
BOOL   fol_VarOccursFreely(TERM,TERM);
BOOL   fol_AssocEquation(TERM, SYMBOL *);
BOOL   fol_DistributiveEquation(TERM, SYMBOL*, SYMBOL*);
void   fol_ReplaceVariable(TERM, SYMBOL, TERM);
void   fol_PrettyPrint(TERM);
LIST   fol_GetSubstEquations(TERM);
TERM   fol_GetBindingQuantifier(TERM, SYMBOL);
int    fol_TermPolarity(TERM, TERM);
BOOL   fol_PolarCheck(TERM, TERM);
void   fol_PopQuantifier(TERM);
void   fol_DeleteQuantifierVariable(TERM,SYMBOL);
void   fol_SetTrue(TERM);
void   fol_SetFalse(TERM);
void   fol_RemoveImplied(TERM);
TERM   fol_RemoveXorNorNand(TERM);
BOOL   fol_PropagateFreeness(TERM);
BOOL   fol_PropagateWitness(TERM);
BOOL   fol_PropagateTautologies(TERM);
BOOL   fol_AlphaEqual(TERM, TERM);
BOOL   fol_VarBoundTwice(TERM);
NAT    fol_Depth(TERM);
BOOL   fol_ApplyContextToTerm(VARCONT, TERM);
BOOL   fol_CheckFormula(TERM);
BOOL   fol_SignatureMatchFormula(TERM, TERM, BOOL);
BOOL   fol_SignatureMatch(TERM, TERM, LIST*, BOOL);

#endif
