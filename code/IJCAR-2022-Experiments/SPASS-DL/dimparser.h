/************************************************************/
/************************************************************/
/**                                                        **/
/**                       PARSER                           **/
/**                                                        **/
/**             Parsing the DIMACs input.                  **/
/**                                                        **/
/**                Author: Martin Suda                     **/
/**                                                        **/
/**                                                        **/
/**  This program is free software; you can redistribute   **/
/**  it and/or modify it under the terms of the FreeBSD    **/
/**  Licence.                                              **/
/**                                                        **/
/**  This program is distributed in the hope that it will  **/
/**  be useful, but WITHOUT ANY WARRANTY; without even     **/
/**  the implied warranty of MERCHANTABILITY or FITNESS    **/
/**  FOR A PARTICULAR PURPOSE.  See the LICENCE file       **/
/**  for more details.                                     **/
/************************************************************/
/************************************************************/

#ifndef _DIMPARSER_    
#define _DIMPARSER_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "array.h"

/**************************************************************/
/* Types                                                      */
/**************************************************************/

typedef enum { UNKNOWN, UNSATISFIABLE, SATISFIABLE } SATRESULT;

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

SATRESULT   dimparser_Parse(FILE*, ARRAY*, ARRAY*);

#endif
