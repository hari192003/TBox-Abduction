/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                   RENAMING                             * */
/* *                                                        * */
/* *  $Module:   REN                                        * */ 
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
/* $Revision: 1.4 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-25 06:59:13 $                             * */
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


/* $RCSfile: renaming.h,v $ */


#ifndef _SPASS_RENAMING_
#define _SPASS_RENAMING_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "misc.h"
#include "foldfg.h"
#include "unify.h"
#include "vector.h"

/**************************************************************/
/* Data Structures and Constants                              */
/**************************************************************/

typedef struct {
  TERM   hit;
  LIST   matches;
  BOOL   general;
  int    polarity;
} *RENAMING, RENAMING_NODE;

/* <hit>       is the formula that has a positive benefit              */
/* <matches>   are further matches of <hit> in the overall formula     */
/* <general>   is TRUE iff the <hit> formula must not be replaced but  */
/*             is a generalzation of the matches formulae that are to  */
/*             be replaced                                             */
/* <polarity>  is the most general polarity of <hit> and all <matches> */

 
/**************************************************************/
/* Functions                                                 */
/**************************************************************/

int      ren_OverallPolarity(RENAMING ren);
TERM     ren_Hit(RENAMING ren);
LIST     ren_Matches(RENAMING ren);
BOOL     ren_General(RENAMING ren);
void     ren_SetMatches(RENAMING ren, LIST matches);
void     ren_SetHit(RENAMING ren, TERM hit);
void     ren_SetOverallPolarity(RENAMING ren, int polarity);
void     ren_SetGeneral(RENAMING ren, BOOL general);
RENAMING ren_Create(TERM hit, LIST matches, int polarity);
void     ren_Delete(RENAMING ren);


/**************************************************************/
/* Function Prototypes                                        */
/**************************************************************/

void ren_Init(void);
TERM ren_Rename(TERM, PRECEDENCE, LIST*,BOOL, BOOL, FLAG_CNFRENAMINGTYPE);
void ren_PrettyPrint(RENAMING);

#endif
