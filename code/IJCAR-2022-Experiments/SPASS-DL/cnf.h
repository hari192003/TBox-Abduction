/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *               NAIVE CNF TRANSLATOR                     * */
/* *                                                        * */
/* *  $Module:   CNF                                        * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 1998, 1999, 2000            * */
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
/* $Date: 2011-11-27 11:05:41 $                             * */
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


/* $RCSfile: cnf.h,v $ */

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#ifndef _CNF_
#define _CNF_

#include "hashmap.h"
#include "renaming.h"
#include "resolution.h"
#include "search.h"
#include "flags.h"

#include <string.h>

/**************************************************************/
/* Functions                                                 */
/**************************************************************/

BOOL cnf_LabelEqual(const char* l1, const char* l2);

LIST cnf_DeleteDuplicateLabelsFromList(LIST Labels);

TERM        cnf_ApplyDefinitionOnce(TERM, TERM, TERM, TERM, FLAGSTORE);
LIST        cnf_ApplyDefinitionToClause(CLAUSE, TERM, TERM,FLAGSTORE,PRECEDENCE);

BOOL        cnf_ContainsDefinition(TERM, TERM*);
BOOL        cnf_ContainsPredicate(TERM, SYMBOL, TERM*, TERM*, LIST*, LIST*);

TERM        cnf_DeSkolemFormula(LIST);
TERM        cnf_DefConvert(TERM, TERM, TERM*);
void        cnf_FilePrint(TERM, FILE*);
TERM        cnf_DefTargetConvert(TERM, TERM, TERM, LIST, LIST, LIST, LIST,
				 FLAGSTORE, PRECEDENCE, BOOL*);

void        cnf_FilePrintPrefix(TERM, FILE*);
void        cnf_FPrint(TERM, FILE*);
TERM        cnf_Flatten(TERM, SYMBOL);
PROOFSEARCH cnf_Flotter(LIST, LIST, LIST*, LIST*, HASHMAP, HASHMAP, FLAGSTORE,
			PRECEDENCE, LIST*);
void        cnf_Free(FLAGSTORE);

LIST        cnf_HandleDefinition(PROOFSEARCH, LIST, LIST, LIST, LIST);
void        cnf_Init(FLAGSTORE);
TERM        cnf_NegationNormalFormula(TERM);
TERM        cnf_ObviousSimplifications(TERM);

LIST        cnf_QueryFlotter(PROOFSEARCH, TERM, LIST*);
void        cnf_StdoutPrint(TERM);

BOOL        cnf_PropagateSubstEquations(TERM);

BOOL        cnf_HaveProof(LIST, TERM, FLAGSTORE, PRECEDENCE);


#endif
