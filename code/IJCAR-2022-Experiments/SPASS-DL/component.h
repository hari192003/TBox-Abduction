/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *               COMPONENTS OF CLAUSES                    * */
/* *                                                        * */
/* *  $Module:   COMPONENT                                  * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 2000, 2001 MPI fuer Informatik    * */
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
/* $Revision: 1.3 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-22 12:06:19 $                             * */
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


/* $RCSfile: component.h,v $ */

#ifndef _COMPONENT_
#define _COMPONENT_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "list.h"

/**************************************************************/
/* Structures                                                 */
/**************************************************************/

typedef struct cliteral {
  BOOL  used;              /* Flag if the index is already used            */
  intptr_t   litindex;          /* Index of the literal in the original clause  */ 
  LIST  litvarlist;        /* List of variables of the literal             */
} *CLITERAL, CLITERAL_NODE;


typedef struct litptr {
  CLITERAL *litptr;             /* Array of Pointer to literals           */
  int     length;              /* Number of literal in the array *litptr */
} *LITPTR, LITPTR_NODE;

/**************************************************************/
/* Macros                                                     */
/**************************************************************/

BOOL literal_GetUsed(CLITERAL C);

int  literal_GetLitIndex(CLITERAL C);

LIST literal_GetLitVarList(CLITERAL C);

void literal_PutUsed(CLITERAL C,BOOL Bool);

void literal_PutLitIndex(CLITERAL C, int I);

void literal_PutLitVarList(CLITERAL C, LIST L);

CLITERAL litptr_Literal(LITPTR C, intptr_t I);

void litptr_SetLiteral(LITPTR LP, int I, CLITERAL CL);

int litptr_Length(LITPTR C);

void litptr_SetLength(LITPTR C, int n);

void litptr_IncLength(LITPTR C);

void literal_Free(CLITERAL Lit);

/**************************************************************/
/* Functions on a Component and on a Literal                  */
/**************************************************************/

CLITERAL literal_Create(BOOL, int, LIST);
void     literal_Delete(CLITERAL);

LITPTR   litptr_Create(LIST, LIST);
void     litptr_Delete(LITPTR);
void     litptr_Print(LITPTR);
BOOL     litptr_AllUsed(LITPTR);


#endif
