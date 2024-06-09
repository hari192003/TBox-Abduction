/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                                                        * */
/* *          Module: rescmp                                * */
/* *                                                        * */
/* *                                                        * */
/* *          Compare two SPASS results (clause sets )      * */
/* *                                                        * */
/* *  Copyright (C) 1998, 1999, 2000, 2001                  * */
/* *  This program is free software; you can redistribute   * */
/* *  it and/or modify it under the terms of the FreeBSD    * */
/* *  Licence.                                              * */
/* *                                                        * */
/* *  This program is distributed in the hope that it will  * */
/* *  be useful, but WITHOUT ANY WARRANTY; without even     * */
/* *  the implied warranty of MERCHANTABILITY or FITNESS    * */
/* *  FOR A PARTICULAR PURPOSE.  See the LICENCE file       * */  
/* *  for more details.                                     * */
/* *  MPI fuer Informatik                                   * */
/* *                                                        * */
/* *                                                        * */
/* $Revision: 1.11 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2016-03-23 13:37:10 $                             * */
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


/* $RCSfile: rescmp.c,v $ */

#include "cmdline.h"
#include "list.h"
#include "clause.h"
#include "subsumption.h"
#include "dfg.h"

#include "description.h"


#define RESCMP__VERSION "0.45"

static LIST  rescmp_ClausesFromFile(const char*, FLAGSTORE, PRECEDENCE, DFGDESCRIPTION);
static BOOL  rescmp_Lists(LIST*, LIST*, LIST*, LIST*, const char**, const char**);
static BOOL  rescmp_FindImage(CLAUSE, LIST, CLAUSE*, LIST*);
static void  rescmp_MoveElement(LIST*, LIST, LIST*);


int main(int argc, const char* argv[]) 
/**************************************************************
  INPUT:   The command line args: possibly an option (-v),
           and two file names.
  RETURNS: Nothing
  EFFECTS: Prints out whether each clause in file1 mutually subsumes
           with a clause in file2, and in verbose mode
	   information about which clauses could not be
	   mapped.
***************************************************************/
{
  LIST       Result1, Result2;
  LIST       Unmapped1, Mapped2;
  BOOL       Verbose;
  const char *File1, *File2;  
  FLAGSTORE  Flags;
  PRECEDENCE Precedence;
  DFGDESCRIPTION Description;
  
  memory_Init(memory__UNLIMITED);
  symbol_Init(TRUE);
  stack_Init();
  term_Init();
  flag_Init(flag_RESCMP);

  /* Use a fresh flag store with default values */
  Flags = flag_CreateStore();
  flag_InitStoreByDefaults(Flags);
  Precedence = symbol_CreatePrecedence();
  Description = desc_Create();

  fol_Init(TRUE, Precedence);
  cont_Init();
  unify_Init();
  subs_Init();
  clause_Init();
  cmdlne_Init();

  dfg_ParserInit();

  Unmapped1 = list_Nil();
  Mapped2   = list_Nil();
  

  if (!cmdlne_Read(argc, argv) ||
      (cmdlne_GetInputFile() == (char*)NULL || cmdlne_GetOutputFile() == (char*)NULL )) {
    fputs("\n\t          rescmp Version ", stdout);
    fputs(RESCMP__VERSION, stdout);
    puts("\n\t       Usage: rescmp [-v] <input-file1> <input-file2> \n");
    puts("See the man page or the postscript documentation for more details.");
    return EXIT_FAILURE;
  }
  
  if (!cmdlne_SetFlags(Flags))
    return EXIT_FAILURE;

  File1   = cmdlne_GetInputFile();
  File2   = cmdlne_GetOutputFile();
  Verbose = flag_GetFlagIntValue(Flags, flag_RESCMPV);

  /* read files */

  if (Verbose) { 
    fputs("Reading file '", stdout);
    fputs(File1, stdout);
    fputs("'...", stdout); 
    fflush(stdout);
  }
  Result1 = rescmp_ClausesFromFile(File1, Flags, Precedence, Description);
  if (Verbose) {
    puts("Ok.");
    fflush(stdout);
  }

  desc_Delete(Description);
  Description = desc_Create();
  
  if (Verbose) { 
    fputs("Reading file '", stdout);
    fputs(File2, stdout);
    fputs("'...", stdout); 
    fflush(stdout);
  }
  Result2 = rescmp_ClausesFromFile(File2, Flags, Precedence, Description);

  if (Verbose) { 
    puts("Ok."); 
    fflush(stdout);
  }

  /* compute difference */
  
  if (rescmp_Lists(&Result1, &Result2, &Unmapped1, &Mapped2, &File1, &File2))
    puts("Results are equal.");
  else {
    puts("Results differ.");

    if (Verbose) {
      fputs("Unmapped clauses in file '", stdout);
      fputs(File1, stdout);
      fputs("': ", stdout);
      if (list_Empty(Unmapped1))
	fputs("None.", stdout);
      else {
	putchar('\n');
	clause_ListPrint(Unmapped1);
      }
      fputs("\n\nUnmapped clauses in file '", stdout);
      fputs(File2, stdout);
      fputs("': ", stdout);
      if (list_Empty(Result2))
	fputs("None.", stdout);
      else {
	putchar('\n');
	clause_ListPrint(Result2);
      }
      putchar('\n');
    }
  }

  /* clean up */
  clause_DeleteClauseList(Result1);
  list_Delete(Unmapped1); 
  clause_DeleteClauseList(Result2);
  clause_DeleteClauseList(Mapped2);

  flag_DeleteStore(Flags);
  symbol_DeletePrecedence(Precedence);
  desc_Delete(Description);

  unify_Free();
  subs_Free();
  cont_Free();
  fol_Free();
  cmdlne_Free();
  symbol_FreeAllSymbols();  
  
#ifdef CHECK
  memory_Print();
#endif
  return 0;
}

static BOOL rescmp_Lists(LIST* L1, LIST* L2, LIST *Unmapped1, LIST *Mapped2,
			 const char** File1, const char** File2)
/**************************************************************
  INPUT:   Four clause lists by reference, two file names by reference. 
  RETURNS: For each clause c1 in list L1, a corresponding clause  c2 
           in L2 is searched. If found, c2 is prepended to Mapped2
	   and removed from L2, if not, c1 is prepended to Unmapped1. 

	   Therefore: 
	   L2 contains on return all clauses that have
	   NOT been the image of some clause in L1,

	   Mapped2 contains all clauses that HAVE been the image
	   of some clause in L1,

	   L1 remains unchanged, and

	   Unmapped1 contains all clauses in L1 that have not been
	   mapped on some clause in L2.

	   TRUE is returned if all clauses in both lists were
	   mapped, that is, Empty(Unmapped1) && Empty(L2).

	   The lists may be swapped so that length(L1) > Length(L2),
	   in which case the filenames are also swapped.
	   
  EFFECTS:  L2 is destructively changed as described above.
***************************************************************/
{
  LIST Tmp1;
  LIST Rest; /* contains rest of clause list to be mapped */
  LIST Pred; /* contains predecessor of an image for a mapped clause */ 
  CLAUSE c1,c2;
  const char* Tmp2;

  /* order lists so that L2, the list that is always fully searched, is the smaller list */
 
  if (list_Length(*L2) > list_Length(*L1)) {
    Tmp1   = *L1; 
    *L1    = *L2; 
    *L2    = Tmp1; 
    Tmp2   = *File1; 
    *File1 = *File2; 
    *File2 = Tmp2;
  }

  Rest = *L1;

  while (!list_Empty(Rest)) {
    
    c1 = list_Car(Rest);

    if (rescmp_FindImage(c1,*L2, &c2,&Pred)) {
      /* if an image is found, dequeue element from */
      /* image list, put it in 'Mapped2' */
      rescmp_MoveElement(L2, Pred, Mapped2);
    }
    else {
      /* otherwise, prepend mapped element to 'Mapped1' */ 
      *Unmapped1 = list_Cons(c1, *Unmapped1);
    }
    /* next clause */
    Rest = list_Cdr(Rest); 
  }
  
  if (list_Empty(*Unmapped1) && list_Empty(*L2))
    return TRUE;
  else
    return FALSE;
}


static BOOL rescmp_FindImage(CLAUSE c1, LIST l, CLAUSE* c2, LIST* Pred)
/**************************************************************
  INPUT:   A clause to be mapped, a list to be search for an image,
           by reference a clause (the image on return) and a list,
  RETURNS: Searches a clause c2 in list l such that
           c1 subs c2, c2 subs c1. Returns TRUE iff such
	   a clause is found. In this case, *c2 points to the
	   clause, and *Pred is the sublist of l that contains
	   *c2 as its second element. (So that the calling
	   function can efficiently dequeue c2.)
	   
  EFFECTS: None 
  MEMORY:  None
***************************************************************/
{    
  BOOL Mapped;
  int l1;

  Mapped = FALSE;
  l1 = clause_Length(c1);
  *Pred = list_Nil();

  while (!Mapped && !list_Empty(l)) {
    *c2 = list_Car(l);
    
    /* check subsumption on clauses with same length */

    if (clause_Length(*c2) == l1) {
      if (subs_Subsumes(c1,*c2, -1,-1)) {
	if (subs_Subsumes(*c2,c1, -1,-1)) 
	  Mapped = TRUE;
      }
    }
    if (!Mapped) {
      *Pred = l;
      l = list_Cdr(l);
    }
  }

  return Mapped;
}


static void rescmp_MoveElement(LIST* L1, LIST Pred, LIST* L2) 
/*********************************************************************
  INPUT:   Three lists.
  EFFECTS: Move element c=list_Car(list_Cdr(Pred)) (Pred is a sublist
           of L1) with predecessor Pred from list L1 to list L2. 
	   Adjust L1 if c is first element (i.e., Pred is empty list). 
           A list node in L1 is freed, one is added to L2.
**********************************************************************/
{ 
  POINTER El;

  if (list_Empty(Pred))
    El = list_Car(*L1);
  else
    El = list_Car(list_Cdr(Pred));

  *L2 = list_Cons(El, *L2);

  if (list_Empty(Pred)) {
    /* remove first element */
    Pred = *L1;                      /* use Pred as temporary */
    *L1  = list_Cdr(*L1);
    memory_Free(Pred, sizeof(LIST_NODE));
  }
  else {
    /* remove other element */
    list_DequeueNext(Pred);
  }
}


static LIST rescmp_ClausesFromFile(const char *Filename, FLAGSTORE Flags,
				   PRECEDENCE Precedence, DFGDESCRIPTION Description)
/*********************************************************************
  INPUT:   A Filename, a flag store and a precedence.
  RETURNS: A list of the clauses contained in the file <Filename>.
  EFFECTS: Allocates memory for result.
**********************************************************************/
{
  LIST  Clauses;
  FILE* File;
  LIST  Axioms, Conjectures, SortDecl, UserPrecedence, UserSelection,
        ClAxRelation;
  BOOL  HasPlainClauses;
  char* IncludePath;

  Axioms         = list_Nil();
  Conjectures    = list_Nil();
  SortDecl       = list_Nil();
  UserPrecedence = list_Nil();
  UserSelection  = list_Nil();
  ClAxRelation   = list_Nil();

  IncludePath = flag_GetFlagStringValue(Flags,flag_IncludePath);   
  
  File    = misc_OpenFile(Filename, "r");
  Clauses = dfg_DFGParser(File,IncludePath,Flags,Precedence,Description,&Axioms,&Conjectures,
			  &SortDecl, &UserPrecedence, &UserSelection, 
			  &ClAxRelation, &HasPlainClauses);
  misc_CloseFile(File, Filename);

  if (!list_Empty(Axioms) || !list_Empty(Conjectures)) {
    misc_StartUserErrorReport();
    misc_UserErrorReport("\n Error: file %s contains formulas.\n", Filename);
    misc_FinishUserErrorReport();
  }
    
  dfg_DeleteFormulaPairList(SortDecl);
  list_Delete(UserPrecedence);
  list_Delete(UserSelection);
  dfg_DeleteClAxRelation(ClAxRelation);

  return Clauses;
}
