/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                INFERENCE RULES                         * */
/* *                                                        * */
/* *  $Module:   INFRULES                                   * */ 
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
/* $Revision: 1.4 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-25 07:51:40 $                             * */
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


/* $RCSfile: rules-inf.h,v $ */

#ifndef _INFRULES_
#define _INFRULES_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "search.h"
#include "rules-split.h"
#include "rules-sort.h"
#include "rules-ur.h"
#include "subst.h"
#include "unify.h"
#include "st.h"
#include "defs.h"

/**************************************************************/
/* Functions                                                  */
/**************************************************************/        

LIST inf_DerivableClauses(PROOFSEARCH, CLAUSE);

LIST inf_EqualityResolution(CLAUSE, BOOL, FLAGSTORE, PRECEDENCE);
LIST inf_EqualityFactoring(CLAUSE, FLAGSTORE, PRECEDENCE);

LIST inf_MergingParamodulation(CLAUSE, SHARED_INDEX, FLAGSTORE, PRECEDENCE);

LIST inf_GenSuperpositionLeft(CLAUSE,SHARED_INDEX,BOOL,BOOL,BOOL,FLAGSTORE, PRECEDENCE);
LIST inf_GenSuperpositionRight(CLAUSE,SHARED_INDEX,BOOL,BOOL,BOOL,FLAGSTORE, PRECEDENCE);

LIST inf_BoundedDepthUnitResolution(CLAUSE, SHARED_INDEX, BOOL, FLAGSTORE, PRECEDENCE);
LIST inf_UnitResolution(CLAUSE, SHARED_INDEX, BOOL, FLAGSTORE, PRECEDENCE);
LIST inf_GeneralResolution(CLAUSE, SHARED_INDEX, BOOL, BOOL, FLAGSTORE, PRECEDENCE);

BOOL inf_HyperLiteralIsBetter(LITERAL, NAT, LITERAL, NAT);
LIST inf_GeneralHyperResolution(CLAUSE, SHARED_INDEX, BOOL, FLAGSTORE, PRECEDENCE);

LIST inf_GeneralFactoring(CLAUSE, BOOL, BOOL, BOOL, FLAGSTORE, PRECEDENCE);

LIST inf_ApplyDefinition(PROOFSEARCH, CLAUSE, FLAGSTORE, PRECEDENCE);

/**************************************************************/
/* Inline Functions                                           */
/**************************************************************/        

LIST inf_Paramodulation(CLAUSE, SHARED_INDEX, FLAGSTORE, PRECEDENCE);
LIST inf_OrderedParamodulation(CLAUSE, SHARED_INDEX, FLAGSTORE, PRECEDENCE);
LIST inf_SuperpositionLeft(CLAUSE, SHARED_INDEX, FLAGSTORE, PRECEDENCE);
LIST inf_SuperpositionRight(CLAUSE, SHARED_INDEX, FLAGSTORE, PRECEDENCE);



#endif

