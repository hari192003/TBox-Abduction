/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                     SUBSTITUTION                       * */
/* *                                                        * */
/* *  $Module:      SUBSTITUTION                            * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 1998, 1999, 2001            * */
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
/* $Date: 2011-11-27 14:36:19 $                             * */
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


/* $RCSfile: subst.h,v $ */

#ifndef _SUBST_
#define _SUBST_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "term.h"
#include "symbol.h"
#include "list.h"

/**************************************************************/
/* Data Structures                                            */
/**************************************************************/

typedef struct subst {
  struct subst *next;
  SYMBOL dom;
  TERM   codomain;
} SUBST_NODE, *SUBST;


SUBST subst_Get(void);

void subst_FreeOneNode(SUBST SL);

/**************************************************************/
/* Include 'unify.h' after SUBST declaration.                 */
/**************************************************************/

#include "unify.h"

/**************************************************************/
/* Functions on Substitutions                                 */
/**************************************************************/

 SUBST subst_Nil(void);

 BOOL subst_Exist(SUBST S);

 BOOL subst_Empty(SUBST S);

 SUBST subst_Next(SUBST S);

 void subst_SetNext(SUBST S, SUBST N);

 SYMBOL subst_Dom(SUBST S);

 TERM subst_Cod(SUBST S);

 SUBST subst_NUnion(SUBST S1,SUBST S2);


/**************************************************************/
/* Functions for Creation and Deletion                        */
/**************************************************************/

SUBST subst_Add(SYMBOL, TERM, SUBST);
void  subst_Delete(SUBST);
void  subst_Free(SUBST);

/**************************************************************/
/* Functions for Applying and Copying                         */
/**************************************************************/

TERM  subst_Term(SYMBOL, SUBST);
TERM  subst_Apply(SUBST, TERM);
SUBST subst_Merge(SUBST, SUBST);
SUBST subst_Compose(SUBST, SUBST);
SUBST subst_Copy(SUBST);
BOOL  subst_MatchTops(const VARCONT, SUBST);
BOOL  subst_BindVar(SYMBOL,SUBST);

/**************************************************************/
/* Functions for Search of Unifiers                           */
/**************************************************************/

BOOL subst_Unify(VARCONT, SUBST);
BOOL subst_IsShallow(SUBST);

/**************************************************************/
/* Functions for Search of Generalizations                    */
/**************************************************************/

BOOL  subst_Match(const VARCONT, SUBST);

/**************************************************************/
/* Functions for Search of Instances                          */
/**************************************************************/

BOOL  subst_MatchReverse(const VARCONT, SUBST);

/**************************************************************/
/* Functions for Search of Variations                         */
/**************************************************************/

BOOL  subst_Variation(const VARCONT, SUBST);

/**************************************************************/
/* Functions for Computation of MSCGs                         */
/**************************************************************/

SUBST subst_ComGen(const VARCONT, SUBST, SUBST*, SUBST*);

/**************************************************************/
/* Functions for Closing Bindings                             */
/**************************************************************/

void  subst_CloseVariables(const VARCONT, SUBST);
SUBST subst_CloseOpenVariables(SUBST);

/**************************************************************/
/* Functions for Extracting Substitutions from Bindings       */
/**************************************************************/

void      subst_ExtractUnifier(const VARCONT, SUBST*, const VARCONT, SUBST*);
void      subst_ExtractUnifierCom(const VARCONT, SUBST*);
SUBST     subst_ExtractMatcher(void);

/**************************************************************/
/* Functions for Debugging                                    */
/**************************************************************/

void  subst_Print(SUBST);

#endif


