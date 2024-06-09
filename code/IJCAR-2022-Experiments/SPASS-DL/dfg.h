/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *              INTERFACE FOR THE DFG PARSER              * */
/* *                                                        * */
/* *  $Module:   DFG                                        * */ 
/* *                                                        * */
/* *  Copyright (C) 1997, 1999, 2000, 2001                  * */
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
/* $Revision: 1.9 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2016-03-23 13:30:31 $                             * */
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


/* $RCSfile: dfg.h,v $ */

#ifndef _DFG_
#define _DFG_

#include <stdio.h>
#include "list.h"
#include "flags.h"
#include "clause.h"

#include "description.h"

/* Parser functions */
void dfg_ParserInit(void); //TODO: add to top.c

void dfg_InitGlobalArrays(void);
void dfg_DeleteGlobalArrays(void);
void dfg_CleanGloabalArrays(void);

void	      dfg_InitExplicitParser(void);
LIST         dfg_DFGParserIncludesExplicit(FILE*, FLAGSTORE, PRECEDENCE, DFGDESCRIPTION, BOOL, LIST*, LIST*, LIST*, LIST*, LIST*, LIST*, LIST*, BOOL*);
ARRAY        dfg_DFGParserIncludesExplicitA(FILE*, FLAGSTORE, PRECEDENCE, DFGDESCRIPTION, BOOL, ARRAY*, ARRAY*, ARRAY*, LIST*, LIST*, LIST*, ARRAY*, BOOL*);

LIST        dfg_DFGParser(FILE*, char *, FLAGSTORE, PRECEDENCE, DFGDESCRIPTION, LIST*, LIST*, LIST*, LIST*, LIST*, LIST*, BOOL*);
LIST        dfg_ProofParser(FILE*, FLAGSTORE, PRECEDENCE, DFGDESCRIPTION);
LIST        dfg_TermParser(FILE*, FLAGSTORE, PRECEDENCE, DFGDESCRIPTION);

/* Misc functions */
void        dfg_DeleteFormulaPairList(LIST);
void        dfg_DeleteFormulaPairArray(ARRAY);
void        dfg_DeleteIncludePairList(LIST);
void        dfg_DeleteIncludePairArray(ARRAY);
void        dfg_StripLabelsFromList(LIST);

void        dfg_DeleteProofList(LIST);
void        dfg_DeleteClAxRelation(LIST);
void        dfg_FilterClausesBySelection(LIST*, LIST*, LIST);
void        dfg_FilterClausesBySelectionA(ARRAY*, LIST*, LIST);

CLAUSE      dfg_CreateClauseFromTerm(TERM, BOOL, FLAGSTORE, PRECEDENCE);
void        dfg_CreateClausesFromTerms(LIST*, LIST*, LIST*, BOOL, FLAGSTORE, PRECEDENCE);
void        dfg_CreateClausesFromTermsA(ARRAY*, ARRAY*, LIST*, LIST*, BOOL, FLAGSTORE, PRECEDENCE);

FILE*       dfg_OpenFile(const char *, char *, char ** const);

#endif
