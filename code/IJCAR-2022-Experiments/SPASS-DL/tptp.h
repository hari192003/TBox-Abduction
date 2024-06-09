/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *             INTERFACE FOR THE TPTP PARSER              * */
/* *                                                        * */
/* *  $Module:   TPTP                                       * */ 
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
/* $Revision: 1.4 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-25 10:14:33 $                             * */
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


#ifndef _TPTP_
#define _TPTP_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "list.h"
#include "flags.h"
#include "symbol.h"
#include "description.h"
#include "dfg.h"

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

void   tptp_Init(void);
void   tptp_Free(void);

void   tptp_SetParserOutput(FILE*);     /*to be called prior to calling the parser*/
void   tptp_SetDoTranslateIdents(BOOL); /*to be called prior to calling the parser*/

LIST   tptp_TPTPParserIncludesExplicit(FILE*, FLAGSTORE, PRECEDENCE, DFGDESCRIPTION, LIST*, LIST*, LIST*, LIST*, LIST*, LIST*, LIST*, BOOL*);
LIST   tptp_TPTPParser(FILE*, char *,FLAGSTORE, PRECEDENCE, DFGDESCRIPTION, LIST*, LIST*, LIST*, LIST*, LIST*, LIST*, BOOL*);

BOOL   tptp_WasParsingCNF(void); /*to be read after the parser finishes*/

FILE*  tptp_OpenFile(const char *, char *, char ** const);

void   tptp_DeleteLabelTermPair(POINTER);

#endif
