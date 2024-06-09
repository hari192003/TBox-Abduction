/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *       CONVERTER FROM DFG SYNTAX TO DIMACS SYNTAX       * */
/* *                                                        * */
/* *  $Module:   DFG2OTTER                                  * */ 
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
/* $Revision: 1.5 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2016-03-23 13:28:31 $                             * */
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


/* $RCSfile: dfg2dimacs.c,v $ */

/*** MAINLOOP *************************************************/


/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "dfg.h"
#include "foldfg.h"
#include "clause.h"
#include "cmdline.h"
#include "eml.h"
#include "description.h"

#include <errno.h>

void dimacs_PrintCnfDimacs(FILE* Output, LIST Clauses, FLAGSTORE Flags) {  
  fprintf(Output,"p cnf %d %zu\n", symbol_ActIndex()-1, list_Length(Clauses));
  
  while (list_Exist(Clauses)) {
    int i,n;
    CLAUSE Clause = (CLAUSE)list_Car(Clauses);
  
    n = clause_Length(Clause);
    
    for (i = clause_FirstLitIndex(); i < n; i++) {
      LITERAL Lit = clause_GetLiteral(Clause, i);
      if (clause_LiteralIsNegative(Lit))
        fprintf(Output,"-");
      fprintf(Output,"%d ",symbol_Index(clause_LiteralPredicate(Lit)));
    }
    fprintf(Output,"0\n");
  
    Clauses = list_Cdr(Clauses);
  }

}

/**************************************************************/
/* Main Function                                              */
/**************************************************************/

int main(int argc, const char* argv[])
{
  LIST       Scan, Scan1, Clauses, AxiomList,ConjectureList, SortList, 
    UserPrecedence,UserSelection,ClAxRelation;
  FILE       *Output,*Input;
  const char *Filename;
  char *     IncludePath;
  char       ConLabel[] = "Conjecture";
  FLAGSTORE  Flags;
  PRECEDENCE Precedence;
  BOOL       HasPlainClauses;
  DFGDESCRIPTION Description;

  memory_Init(memory__UNLIMITED);
  atexit(memory_FreeAllMem);
  symbol_Init(TRUE);
  stack_Init();
  term_Init();
  flag_Init(flag_SPASS);

  Flags      = flag_CreateStore();
  flag_InitStoreByDefaults(Flags);
  Precedence = symbol_CreatePrecedence();
  Description = desc_Create();
  
  fol_Init(TRUE, Precedence);
  clause_Init();
  cmdlne_Init();

  dfg_ParserInit();

  if (!cmdlne_Read(argc, argv))
    return EXIT_FAILURE;

  if  (!cmdlne_SetFlags(Flags))
    return EXIT_FAILURE;

  if (!flag_GetFlagIntValue(Flags, flag_STDIN) && 
      (cmdlne_GetInputFile() == (char*)NULL || cmdlne_GetOutputFile() == (char*)NULL )) {
    fputs("\n\t          dfg2dimacs ", stdout);
    puts("\n\t       Usage: dfg2dimacs [options] <input-file> <output-file>\n");
    return EXIT_FAILURE;
  }

  AxiomList      = list_Nil();
  ConjectureList = list_Nil();
  SortList       = list_Nil();
  UserPrecedence = list_Nil();
  UserSelection  = list_Nil();
  ClAxRelation   = list_Nil();
  
  IncludePath = flag_GetFlagStringValue(Flags,flag_IncludePath);

  if (!flag_GetFlagIntValue(Flags, flag_STDIN)) {
    Input = misc_OpenFile(cmdlne_GetInputFile(),"r");
    Clauses = dfg_DFGParser(Input,IncludePath,Flags,Precedence,Description,&AxiomList,&ConjectureList,
			    &SortList, &UserPrecedence, &UserSelection, &ClAxRelation, &HasPlainClauses);
    misc_CloseFile(Input,cmdlne_GetInputFile());
    if (flag_GetFlagIntValue(Flags, flag_EML)) {
      misc_StartUserErrorReport();
      misc_UserErrorReport("\n The input file contains EML formulae. Please perfrom a translation to FOL by FLOTTER first!\n\n");
      misc_FinishUserErrorReport();
    }
    AxiomList = list_Nconc(AxiomList, SortList);
    if (!list_Empty(ConjectureList)) { 
      /* Build conjecture formula and negate it: conjectures are taken disjunctively!!*/
      for (Scan = ConjectureList; !list_Empty(Scan); Scan = list_Cdr(Scan))
	list_Rplacd(list_Car(Scan), (LIST)term_Create(fol_Not(), 
						      list_List(list_PairSecond(list_Car(Scan)))));
      if (!list_Empty(list_Cdr(ConjectureList))) {
	for (Scan = ConjectureList; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
	  Scan1 = list_Car(Scan);
	  list_Rplaca(Scan, list_PairSecond(list_Car(Scan)));
	  if (list_PairFirst(Scan1) != NULL) 
	    string_StringFree(list_PairFirst(Scan1));  /* Free the label */
	  list_PairFree(Scan1);                      /* Free the pair */
	}
	ConjectureList = list_List(list_PairCreate(string_StringCopy(ConLabel),
						   term_Create(fol_Or(), ConjectureList)));
      }
    }
    AxiomList = list_Nconc(ConjectureList, AxiomList);    
    Filename  = cmdlne_GetOutputFile();
    Output    = misc_OpenFile(Filename,"w");
    if (list_Empty(AxiomList))
      dimacs_PrintCnfDimacs(Output, Clauses, Flags);
    else {
      misc_StartErrorReport();
      misc_ErrorReport("\n Only clause files are supported. \n");
      misc_FinishErrorReport();    
    }
           
    dfg_DeleteFormulaPairList(AxiomList);
    misc_CloseFile(Output,Filename);
  }
  else {
    Clauses   = dfg_DFGParser(stdin,IncludePath,Flags,Precedence,Description,&AxiomList,&ConjectureList,
			      &SortList, &UserPrecedence, &UserSelection, &ClAxRelation, &HasPlainClauses);
    if (flag_GetFlagIntValue(Flags, flag_EML)) {
      misc_StartUserErrorReport();
      misc_UserErrorReport("\n The input file contains EML formulae. Please perfrom a translation to FOL by FLOTTER first!\n");
      misc_FinishUserErrorReport();
    }
    AxiomList = list_Nconc(AxiomList, SortList);
    if (!list_Empty(ConjectureList)) {
      /* Build conjecture formula and negate it: conjectures are taken disjunctively!!*/
      for (Scan = ConjectureList; !list_Empty(Scan); Scan = list_Cdr(Scan))
	list_Rplacd(list_Car(Scan), (LIST)term_Create(fol_Not(), 
						      list_List(list_PairSecond(list_Car(Scan)))));
      if (!list_Empty(list_Cdr(ConjectureList))) {
	for (Scan = ConjectureList; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
	  Scan1 = list_Car(Scan);
	  list_Rplaca(Scan, list_PairSecond(list_Car(Scan)));
	  if (list_PairFirst(Scan1) != NULL) 
	    string_StringFree(list_PairFirst(Scan1));  /* Free the label */
	  list_PairFree(Scan1);                      /* Free the pair */
	}
	ConjectureList = list_List(list_PairCreate(string_StringCopy(ConLabel),
						   term_Create(fol_Or(), ConjectureList)));
      }
    }
    AxiomList = list_Nconc(ConjectureList, AxiomList);
    Output    = stdout;
    if (list_Empty(AxiomList))
      dimacs_PrintCnfDimacs(Output, Clauses, Flags);
    else {
      misc_StartErrorReport();
      misc_ErrorReport("\n Only clause files are supported. \n");
      misc_FinishErrorReport();    
    }      
    dfg_DeleteFormulaPairList(AxiomList);
  }

  clause_DeleteClauseList(Clauses);
  /*symbol_Dump();*/
  if(flag_GetFlagIntValue(Flags, flag_EML)){
    eml_Free();
  }
  
  flag_DeleteStore(Flags);
  symbol_DeletePrecedence(Precedence);
  list_Delete(UserPrecedence);
  list_Delete(UserSelection);
  dfg_DeleteClAxRelation(ClAxRelation);
  desc_Delete(Description);
  cmdlne_Free();

  fol_Free();
  symbol_FreeAllSymbols();
#ifdef CHECK
  memory_Print();
#endif
  return 0;
}
