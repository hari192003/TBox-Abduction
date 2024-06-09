/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *              DIMACS2DFG                                * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


/**************************************************************/
/* *  This program is free software; you can redistribute   * */
/* *  it and/or modify it under the terms of the FreeBSD    * */
/* *  Licence.                                              * */
/* *                                                        * */
/* *  This program is distributed in the hope that it will  * */
/* *  be useful, but WITHOUT ANY WARRANTY; without even     * */
/* *  the implied warranty of MERCHANTABILITY or FITNESS    * */
/* *  FOR A PARTICULAR PURPOSE.  See the LICENCE file       * */ 
/* *  for more details.                                     * */
/**************************************************************/

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "memory.h"
#include "term.h"
#include "clause.h"
#include "foldfg.h"
#include "symbol.h"
#include "cmdline.h"

#include "tptp.h"
#include "dimparser.h"

#define DIMACS2DFG__VERSION "0.3"

/*-----------------------------------------------------------------------------*/

static LIST dim_PrepareDFGProblem(ARRAY*, ARRAY*, PRECEDENCE);
static void dim_FPrintDFGProblem(FILE*, DFGDESCRIPTION, LIST);

/*-----------------------------------------------------------------------------*/

int main(int argc, const char* argv[]) {

  ARRAY clauses, variables;
  LIST Axioms;
  PRECEDENCE Precedence;
  FLAGSTORE  Flags;
  DFGDESCRIPTION Description;
  FILE* InFile;
  FILE* OutFile;
  int i;

  /*Set up modules*/
  
  memory_Init(memory__UNLIMITED);
  symbol_Init(TRUE);
  stack_Init();
  flag_Init(flag_SPASS);
  array_Init();

  Flags = flag_CreateStore();
  flag_InitStoreByDefaults(Flags);

  Precedence = symbol_CreatePrecedence();
  term_Init();
  fol_Init(TRUE, Precedence);
  clause_Init();
  cmdlne_Init(flag_SPASS);

  if (!cmdlne_Read(argc, argv) ||
      cmdlne_GetInputFile() == (char*)NULL || 
      cmdlne_GetOutputFile() == (char*)NULL) {
    /* print options */
    fputs("\n\t          dimacs2dfg Version ", stdout);
    fputs(DIMACS2DFG__VERSION, stdout);
    puts("\n\t       Usage: dimacs2dfg <input-file> <output-file>\n");

    return EXIT_FAILURE;
  }

  if (!cmdlne_SetFlags(Flags))
    return EXIT_FAILURE;

  /*Set up variables*/   
  variables   = array_Create(100,50);  
  clauses     = array_Create(10,50);
  Description = desc_Create();
  /*TODO: fill in the description somehow?*/
  
  InFile = misc_OpenFile(cmdlne_GetInputFile(),"r");
  OutFile = misc_OpenFile(cmdlne_GetOutputFile(),"w"); 
  
  /*Parse the thing*/
  dimparser_Parse(InFile,&variables,&clauses);

  /* Fill it into spass */
  Axioms = dim_PrepareDFGProblem(&variables,&clauses,Precedence);    

  /* print it */
  dim_FPrintDFGProblem(OutFile,Description,Axioms);

  misc_CloseFile(InFile,cmdlne_GetInputFile());
  misc_CloseFile(OutFile,cmdlne_GetOutputFile());

  /* Free the variables */
  desc_Delete(Description);
  list_DeleteWithElement(Axioms, tptp_DeleteLabelTermPair);
  array_Delete(variables);
  for(i = 0; i < array_GetSize(clauses); i++)
    array_Delete((ARRAY)array_GetElement(clauses,i));
  array_Delete(clauses);
  
  /*Free the modules*/
  array_Free();
  cmdlne_Free();
  flag_DeleteStore(Flags);
  symbol_DeletePrecedence(Precedence);
  fol_Free();
  symbol_FreeAllSymbols();
  #ifdef CHECK
  memory_Print();
  #endif
  
  return EXIT_SUCCESS;
}

static SYMBOL dim_getVarSymb(int varindex, ARRAY variables) {
  int var;
  char* number;
  char* name;
  SYMBOL s;
  
  var = array_GetElement(variables,varindex);
  number = string_IntToString(var);
  name = string_Conc("p",number);
  s = symbol_Lookup(name);  
  string_StringFree(number);
  string_StringFree(name);
  
  return s;
}

static LIST dim_PrepareDFGProblem(ARRAY* variables, ARRAY* clauses, PRECEDENCE Precedence) {
  LIST Result;
  int i,j, var, lit, varidx;
  char* number;
  char* name;
  
  Result = list_Nil();
  
  /*Register that names as they were read from the file*/
  for(i = 0; i < array_GetSize(*variables); i++) {
    var = array_GetElement(*variables,i);
    number = string_IntToString(var);
    name = string_Conc("p",number);
    symbol_CreatePredicate(name,0,symbol_STATLEX,Precedence);
    string_StringFree(number);
    string_StringFree(name);
  }

  /*Now prepare the clauses*/
  for (i = array_GetSize(*clauses)-1; i >=0; i--) {  /* starting form the back will result in the right order in the end*/
    ARRAY clause;
    LIST or_args;
    
    clause = (ARRAY)array_GetElement(*clauses,i);
    or_args = list_Nil(); /*the resulting term will be a big OR from the individual literals*/
    
    number = string_IntToString(i);
    name   = string_Conc("clause",number);
    string_StringFree(number);
      
    for (j = array_GetSize(clause)-1; j >=0; j--) {
      TERM literal;
      
      lit = array_GetElement(clause,j);
      varidx = abs(lit)-1;
      literal = term_Create(dim_getVarSymb(varidx,*variables),list_Nil());
      if (lit < 0)
        literal = term_Create(fol_Not(),list_List(literal));

      or_args = list_Cons(literal,or_args);
    }    
    Result = list_Cons(list_PairCreate(name,term_Create(fol_Or(),or_args)),Result);
  }

  return Result;
}

static void dim_FPrintClauseList(FILE* File, LIST Formulas, const char* Name)
/**************************************************************
  INPUT:   A file, a list of formulas, a name.
  EFFECTS: Print a list formulas in DFG format, with given list name.
 **************************************************************/
{
  LIST scan;

  fputs("list_of_clauses(", File);
  fputs(Name, File);
  fputs(",cnf).\n", File);
  for (scan = Formulas; !list_Empty(scan); scan= list_Cdr(scan)) {
    LIST formula_pair;
  
    formula_pair = list_Car(scan);
  
    fputs("\tclause(", File);
    fol_FPrintDFG(File, list_PairSecond(formula_pair));
    fputs(",", File);
    fputs((char*)list_PairFirst(formula_pair), File);
    fputs(").\n", File);
  }
  fputs("end_of_list.\n\n", File);
}

 const char* Null2Empty(char * str) {
	return str ? str : "";
}

void dim_FPrintDFGProblem(FILE* File, DFGDESCRIPTION Description, LIST Axioms)
/**************************************************************
  INPUT:   A file, some info strings, list of formulae
  EFFECTS: Prints a complete DFG file containing these lists.
**************************************************************/
{
  fputs("begin_problem(Unknown).\n\n", File);
  
  fputs("list_of_descriptions.\n", File);
  fprintf(File,"name({*%s*}).\n",Null2Empty(Description->name));
  fprintf(File,"author({*%s*}).\n",Null2Empty(Description->author));
  if (Description->version)
    fprintf(File,"version({*%s*}).\n",Null2Empty(Description->version));
  if (Description->logic)
    fprintf(File,"logic({*%s*}).\n",Null2Empty(Description->logic));

  fprintf(File,"status(%s).\n",desc_StatusString(Description));

  fprintf(File,"description({*%s*}).\n",Null2Empty(Description->description));
  if (Description->date)
    fprintf(File,"date({*%s*}).\n",Null2Empty(Description->date));
  fputs("end_of_list.\n\n", File);
  
  fputs("list_of_symbols.\n", File);
  symbol_FPrintDFGSignature(File);
  fputs("end_of_list.\n\n", File);

  dim_FPrintClauseList(File, Axioms, "axioms");
     
  fputs("end_problem.\n", File);
}
