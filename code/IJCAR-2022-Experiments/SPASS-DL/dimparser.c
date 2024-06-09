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

#include "dimparser.h"

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

SATRESULT dimparser_Parse(FILE* pFile, ARRAY* variables, ARRAY* clauses)
/**************************************************************
INPUT:   A file to read, 2 empty arrays.
RETURNS: Typically UNKNOWN, but trivial instances can be resolved
          already at parsetime.
SUMMARY: The file gets parsed and the two arrays are filled
         appropriately. Parsing may issue warnings or
         terminate with an error if the input doesn't
         conform to the format.
***************************************************************/
{
  ARRAY clause;
  int c,e,lit,var;
  int vars_announced, clauses_announced, new_name;
  BOOL whole, drop_next;
  SATRESULT result;

  result = UNKNOWN;

  /* Read the header. */
  do
  {
    c = fgetc (pFile);

    if (c == 'c') 
    {  /* comment -> eat the rest of the line */    
      do
      {
        c = fgetc(pFile);
      }
      while ((c != '\n') && (c != EOF));

      if (c == EOF)
      {
        fprintf(stderr,"File not complete!\n");
        exit(EXIT_FAILURE);
      }

      continue; /*we are on the next line -> read next character*/
    }

    if (c != 'p')
    {
      fprintf(stderr,"Invalid format!\n");
      exit(EXIT_FAILURE);
    }

    if (fscanf(pFile," cnf %d %d",&vars_announced,&clauses_announced) != 2)
    {
      fprintf(stderr,"Invalid format!\n");
      exit(EXIT_FAILURE);
    }

    break;

  }
  while (1);

  c = fgetc(pFile);
  if ((c != (int)'\n') && (c != '\r'))
    fprintf(stderr,"Warning: Newline expected(#%d read).\n",c);

  clause = array_Create(10,50);
  whole = TRUE; /* boolean-like: whole clause read? */
  drop_next = FALSE;

  /* Read the clauses */
  while((e = fscanf(pFile,"%d",&lit)) == 1)
  {
    var = abs(lit);

    if (var == 0) 
    { /* other clause read */
      if (array_GetSize(clause) == 0) {
        fprintf(stderr,"Warning: Empty clause read - problem trivially unsatisfiable.\n");
        result = UNSATISFIABLE;        
      }

      if (drop_next)
        array_Delete(clause);
      else
        *clauses = array_Add(*clauses,(intptr_t)clause);
      clause = array_Create(10,50);
    
      whole = TRUE;
      drop_next = FALSE;

      c = fgetc(pFile);
      if ((c != '\n') && (c != '\r') && (c != ' ') )
        fprintf(stderr,"Warning: Newline expected(#%d read).\n",c);
    }
    else
    { /* just a literal */
      int index;
    
      whole = FALSE;
     
      if (!array_ContainsElement(*variables,var,&index)) {
        *variables = array_Add(*variables,var);
        new_name = array_GetSize(*variables); 
      } else {
        new_name = index+1;
      }
      /* in any case: new_name = (index of the var in the array)+1  */   

      /*overwrite lit to reference vars (index+1)*/
      lit = (lit>0) ? new_name : -new_name;

      if (array_ContainsElement(clause,lit,&index)) {
        fprintf(stderr,"Warning: Multiple occurence of literal %d in clause number %d.\n",
          lit,array_GetSize(*clauses)+1);
      } else {
        clause = array_Add(clause,lit);
      }

      if (array_ContainsElement(clause,-lit,&index)) {
        fprintf(stderr,"Clause number %d containes both positive and negative occurence of varible %d.\n"
          "Clause trivially satisfiable - not included.\n",
          array_GetSize(*clauses)+1,var);
        drop_next = TRUE;
      }
    }
  }

  if(e != EOF)
    fprintf(stderr,"Warning: Format error - didn't finish reading.\n");

  if(!whole)
  {
    fprintf(stderr,"Warning: Last clause not properly closed.\n");
    *clauses = array_Add(*clauses,(intptr_t)clause);
  }
  else /*We were closed properly - the clause already prepared for reading next line has to be disposed. */
  {
    array_Delete(clause);
  }

  if (array_GetSize(*clauses) == 0)
    result = SATISFIABLE;

  return result;
}
