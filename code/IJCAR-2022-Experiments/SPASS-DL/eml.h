/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *              EXTENDED MODAL LOGICS ROUTINES            * */
/* *                                                        * */
/* *  $Module:   EML                                        * */ 
/* *                                                        * */
/* *                                                        * */
/* $Revision: 1.4 $                                         * */
/* $State: Exp $                                            * */
/* *  This program is free software; you can redistribute   * */
/* *  it and/or modify it under the terms of the FreeBSD    * */
/* *  Licence.                                              * */
/* *                                                        * */
/* *  This program is distributed in the hope that it will  * */
/* *  be useful, but WITHOUT ANY WARRANTY; without even     * */
/* *  the implied warranty of MERCHANTABILITY or FITNESS    * */
/* *  FOR A PARTICULAR PURPOSE.  See the LICENCE file       * */ 
/* *  for more details.                                     * */
/* $Date: 2011-05-22 13:15:40 $                             * */
/* $Author: weidenb $                                       * */
/* *                                                        * */
/* *             Contact:                                   * */
/* *             Renate Schmidt                             * */
/* *             School of Computer Science                 * */
/* *             University of Manchester                   * */
/* *             Oxford Rd, Manchester M13 9PL, UK          * */
/* *             Email: Renate.Schmidt@manchester.ac.uk     * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


/* $RCSfile: eml.h,v $ */

#ifndef _EMLDFG_
#define _EMLDFG_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "term.h"
#include "list.h"
#include "hasharray.h"

/**************************************************************/
/* Global Variables and Constants (Only seen by macros)       */
/**************************************************************/

extern SYMBOL  eml_APPLYF;
extern SYMBOL  eml_BOX;
extern SYMBOL  eml_COMP;
extern SYMBOL  eml_CONV;
extern SYMBOL  eml_DIA;
extern SYMBOL  eml_DIV;
extern SYMBOL  eml_DOMAIN;
extern SYMBOL  eml_DOMRESTR;
extern SYMBOL  eml_ID;
extern SYMBOL  eml_RANGE;
extern SYMBOL  eml_RANRESTR;
extern SYMBOL  eml_SUM;
extern SYMBOL  eml_TEST;

typedef enum { eml_PROP,
               eml_REL,
	       eml_NDE,
               eml_NARYPROP,
	       eml_NARYNDE,
	       eml_SEMIFNDE, 
               eml_VARSORT
} eml_SYMBOLTYPE;

extern LIST   *eml_PROPINDEXTOFOSYMBOL;
extern SYMBOL eml_SEMIFNDESYMBOLS[symbol__MAXSIGNATURE];            
/* Records which nde symbols have been created, for the semi-functional
 * translation */

#define eml__MAXMODALDEPTH 100

extern SYMBOL eml_NARYSYMBOLS[symbol__MAXSIGNATURE][eml__MAXMODALDEPTH];    
/* Records which symbols of the n-ary signature have been created */

/**************************************************************/
/* Access to the first-order symbols.                         */
/**************************************************************/

  SYMBOL eml_Box(void);

  SYMBOL eml_Dia(void);

  SYMBOL eml_Domain(void);

  SYMBOL eml_Range(void);

  SYMBOL eml_Comp(void);

  SYMBOL eml_Sum(void);

  SYMBOL eml_Conv(void);

  SYMBOL eml_Test(void);

  SYMBOL eml_Id(void);

  SYMBOL eml_Div(void);

  SYMBOL eml_DomRestr(void);

  SYMBOL eml_RanRestr(void);

  SYMBOL eml_ApplyFunct(void);

/**************************************************************/
/* Symbol identification and access                           */
/**************************************************************/

  BOOL eml_IsId(TERM S);

  BOOL eml_IsDiv(TERM S);

  BOOL eml_IsNotSymbol(SYMBOL S);

  BOOL eml_IsImpliesSymbol(SYMBOL S);

  BOOL eml_IsImpliedSymbol(SYMBOL S);

  BOOL eml_IsEquivSymbol(SYMBOL S);

  BOOL eml_IsDiaSymbol(SYMBOL S);

  BOOL eml_IsBoxSymbol(SYMBOL S);

  BOOL eml_IsModalOpSymbol(SYMBOL S);

  BOOL eml_IsRelPropSymbol(SYMBOL S);

  BOOL eml_IsModalLiteral(TERM T);
/* unused */

  BOOL eml_IsPropJunctor(SYMBOL S);

  BOOL eml_IsDesignatedPropSymbol(SYMBOL S);

  BOOL eml_IsNullaryPredicate(SYMBOL S);

  BOOL eml_IsNaryJunctor(SYMBOL S);

  BOOL eml_IsRelationalJunctor(SYMBOL S);

  BOOL eml_IsModalJunctor(SYMBOL S);

  BOOL eml_IsPredefinedPred(SYMBOL S);

  TERM eml_Atom(TERM Lit);

  SYMBOL eml_GetDualSymbol(SYMBOL symbol);
/**************************************************************/
/* Propositional/first order symbol associations              */
/**************************************************************/

  BOOL eml_VarSortType(eml_SYMBOLTYPE SymbolType);


  BOOL eml_NaryType(eml_SYMBOLTYPE SymbolType);

  BOOL eml_SemiFuncNdeType(eml_SYMBOLTYPE SymbolType);

  BOOL eml_RelType(eml_SYMBOLTYPE SymbolType);

  void eml_SetPropNarySymbolAssocList(SYMBOL PropSymbol, 
		int Arity, SYMBOL FoSymbol);

  void eml_SetPropSemiFNdeSymbolAssocList(SYMBOL PropSymbol, 
		int Arity, SYMBOL FoSymbol);

  void eml_SetPropFoSymbolAssocList(SYMBOL PropSymbol, LIST FoSymbols);

 SYMBOL eml_FoQuantAssocWithPropSymbol(SYMBOL PropSymbol);

 SYMBOL eml_FoJunctorAssocWithPropSymbol(SYMBOL PropSymbol);

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

void eml_Init(PRECEDENCE);
void eml_Free(void);
void eml_TranslateToFolMain(LIST*, LIST*, BOOL, FLAGSTORE, PRECEDENCE);
BOOL eml_FormulaContainsEMLTheorySymbol(TERM);

#endif
