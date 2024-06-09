/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                 PROOF DOCUMENTATION                    * */
/* *                                                        * */
/* *  $Module:   DOCPROOF                                   * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 1998, 2000, 2001            * */
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
/* $Revision: 1.4 $                                     * */
/* $State: Exp $                                            * */
/* $Date: 2011-11-27 11:47:43 $                             * */
/* $Author: weidenb $                                         * */
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


/* $RCSfile: doc-proof.h,v $ */

#ifndef _DOC_PROOF_
#define _DOC_PROOF_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "clause.h"
#include "st.h"
#include "sharing.h"
#include "search.h"
#include "doc-proof.h"
#include "proofcheck.h"

/**************************************************************/
/* Data Structures and Constants                              */
/**************************************************************/

extern int dp_DEPTH;


/**************************************************************/
/* Macros                                                     */
/**************************************************************/ 

int dp_ProofDepth(void);

void dp_SetProofDepth(int Depth);

/**************************************************************/
/* Functions                                                  */
/**************************************************************/        

void dp_Init(void);
LIST dp_PrintProof(PROOFSEARCH, LIST, const char*, HASHMAP);

#endif
