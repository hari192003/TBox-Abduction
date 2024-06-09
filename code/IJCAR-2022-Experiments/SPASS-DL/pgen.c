/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *             PROOF GENERATOR MAIN                       * */
/* *                                                        * */
/* *  Copyright (C) 1998, 1999, 2000, 2001                  * */
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
/* $Revision: 1.9 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2016-03-23 13:36:58 $                             * */
/* $Author: weidenb $                                       * */
/* *                                                        * */
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

/* $RCSfile: pgen.c,v $ */

/**************************************************************

   Given a dfg proof file, this program generates
   a set of proof files corresponding to each proof step.

	Options:

	-d    Prefix of generated files
	-t    time limit for proof attempt
	      (to set the TimeLimit option in the
	       file passed to SPASS)
	-s    file suffix of generated proof files
	-q    quiet mode

**************************************************************/

#include "proofcheck.h"
#include "description.h"
#include "cmdline.h"

extern int   pcheck_Timelimit;
extern const char* pcheck_ProofFileSuffix;
extern BOOL  pcheck_Quiet;

extern BOOL pcheck_ClauseCg;
extern BOOL pcheck_GenNamedCg, pcheck_GenRedCg;
extern const char *pcheck_CgName, *pcheck_RedCgName;
extern GRAPHFORMAT pcheck_GraphFormat;

#define PGEN__VERSION "2.0"

int main(int argc, const char* argv[])
{
  /* option defaults */
  /* time allowed for each spass proof check */
  const int  DefaultTimeLimit = 3;
  /* file suffix of generated proof files */
  const char *const DefaultProofFileSuffix = ".dfg";

  LIST       Proof;            /* proof with DFG terms */
  LIST       ConvertedProof;   /* proof with clauses   */
  char       *Filename, *Path;
  const char *FullFilename, *Prefix;
  int        i;
  BOOL       PrefixDefined;
  FILE*      ProofFileHandle;
  TABLEAU    Tableau;
  FLAGSTORE  Flags;
  PRECEDENCE Precedence;
  DFGDESCRIPTION Description;

  /* OPTID   Timelimit_OptId;  set time limit in SPASS flags in generated files */
  /* OPTID   Suffix_OptId;     suffix of generated files */
  /* OPTID   Quiet_OptId;      do not print any comments on progress */
  /* OPTID   Prefix_OptId;     prefix of generated files */
  /* OPTID   JustCg_OptId;     do not generate proof files */

  BOOL    pcheck_JustCg;    /* do not generate any proof files */

  /* OPTID GenRedCg_OptId;      write the reduced tableau to a named file */
  /* OPTID GenNamedCg_OptId;    write the (non-reduced) tableau to a named file */
  /* OPTID ClauseCg_OptId;      do not show clauses in tableau output */
  /* OPTID XvcgFormat_OptId;    print tableau in xvcg format */
  /* OPTID DaVinciFormat_OptId; print tableau in daVinci format */

  /* Initalization */
  memory_Init(memory__UNLIMITED);
  symbol_Init(TRUE);
  stack_Init();
  term_Init();
  flag_Init(flag_SPASS);
  flag_Init(flag_PGEN);

  Flags = flag_CreateStore();
  flag_InitStoreByDefaults(Flags);
  Precedence = symbol_CreatePrecedence();
  Description = desc_Create();

  fol_Init(TRUE, Precedence);
  clause_Init();
  cont_Init();
  unify_Init();
  vec_Init();
  cmdlne_Init();
  subs_Init();

  dfg_ParserInit();

  /* Declare options, set to defaults and read command line */
  /* See *_OptId variable declarations for option semantics */
  /* Timelimit_OptId = opts_Declare("t", opts_REQARGTYPE);*/
  /* Suffix_OptId    = opts_Declare("s", opts_REQARGTYPE);*/
  /* Prefix_OptId    = opts_Declare("d", opts_REQARGTYPE);*/
  /* Quiet_OptId     = opts_Declare("q", opts_NOARGTYPE);*/
  /* JustCg_OptId    = opts_Declare("j", opts_NOARGTYPE);*/

  /* GenNamedCg_OptId    = opts_Declare("n", opts_REQARGTYPE);*/
  /* GenRedCg_OptId      = opts_Declare("r", opts_REQARGTYPE);*/
  /* ClauseCg_OptId      = opts_Declare("c", opts_NOARGTYPE);*/
  /* XvcgFormat_OptId    = opts_Declare("xvcg", opts_NOARGTYPE);*/
  /* DaVinciFormat_OptId = opts_Declare("vinci", opts_NOARGTYPE);*/
  
  pcheck_ProofFileSuffix = DefaultProofFileSuffix;
  pcheck_Timelimit       = DefaultTimeLimit;
  pcheck_Quiet           = FALSE;
  Prefix                 = NULL;
  PrefixDefined          = FALSE;
  pcheck_JustCg          = FALSE;

  pcheck_GenNamedCg      = FALSE;
  pcheck_GenRedCg        = FALSE;
  pcheck_ClauseCg        = TRUE;
  pcheck_GraphFormat     = XVCG;
 

  /* get file name argument */
  if (!cmdlne_Read(argc, argv) ||
      cmdlne_GetInputFile() == (char*)NULL) {
    fputs("\n\t         pgen Version ", stdout);
    fputs(PGEN__VERSION, stdout);
    puts("\nUsage: pgen [ -dstqjnr] [-vinci -xvcg] filename");
    puts("\nSee the man page or the postscript documentation for more details.");
    return EXIT_FAILURE;
  }

  if (!cmdlne_SetFlags(Flags)) {
    return EXIT_FAILURE;
  }


  /* translate command line options into parameters */

  FullFilename = cmdlne_GetInputFile();
  Prefix       = flag_GetFlagStringValue(Flags, flag_PGEND);
  if (Prefix)
    PrefixDefined = TRUE;
  if (flag_GetFlagStringValue(Flags, flag_PGENS) != (char *)NULL)
    pcheck_ProofFileSuffix = flag_GetFlagStringValue(Flags, flag_PGENS);
  pcheck_Quiet      = (flag_GetFlagIntValue(Flags, flag_PGENQ) == flag_ON);
  pcheck_JustCg     = (flag_GetFlagIntValue(Flags, flag_PGENJ) == flag_ON);
  pcheck_GenNamedCg = (flag_GetFlagStringValue(Flags,flag_PGENN) != (char *)NULL);
  pcheck_CgName     = flag_GetFlagStringValue(Flags,flag_PGENN);
  pcheck_GenRedCg   = (flag_GetFlagStringValue(Flags,flag_PGENR) != (char *)NULL);
  pcheck_RedCgName  = flag_GetFlagStringValue(Flags,flag_PGENR);
  if (flag_GetFlagIntValue(Flags, flag_PGENC) == flag_ON)
    pcheck_ClauseCg = FALSE;
  if (flag_GetFlagIntValue(Flags, flag_PGENVINCI) == flag_ON)
    pcheck_GraphFormat = DAVINCI;
  if (flag_GetFlagIntValue(Flags, flag_PGENXVCG) == flag_ON)
    pcheck_GraphFormat = XVCG;
  
  /* derive prefix of proof file from filename */
  /* extract pathname, append prefix parameter, if given. */
  for (i = strlen(FullFilename)-1; i >= 0 && FullFilename[i] != '/' ; i--)
    /* empty */;
  if (i == 0)
    i = -1;
  Path     = string_Prefix(FullFilename,i+1);
  Filename = string_Suffix(FullFilename, i+1);   /* extract file name */
  if (!PrefixDefined)
    Prefix = Path;

  /* read and check proof */
  ProofFileHandle = misc_OpenFile(FullFilename, "r");
  Proof           = dfg_ProofParser(ProofFileHandle, Flags, Precedence, Description);
  misc_CloseFile(ProofFileHandle, FullFilename);

  if (list_Empty(Proof)) {
    misc_StartUserErrorReport();
    misc_UserErrorReport("\n Proof given to pgen is empty.\n");
    misc_FinishUserErrorReport();
  }

  /* Create clauses from the terms */
  ConvertedProof = pcheck_ConvertTermListToClauseList(Proof, Flags, Precedence);

  if (!pcheck_BuildTableauFromProof(ConvertedProof, &Tableau)) {
    misc_StartUserErrorReport();
    misc_UserErrorReport("\n Could not built a closed tableau from the proof.\n");
    misc_FinishUserErrorReport();
  }
  if (pcheck_TableauProof(&Tableau, ConvertedProof)) {
    if (!pcheck_JustCg) {
      pcheck_TableauToProofTask(Tableau, Filename, Prefix);
      puts("\nAll files generated!");
    }
  }
  else
    puts("\nProof not correct.");
  
  tab_Delete(Tableau);
  clause_DeleteClauseList(ConvertedProof);

  /* clean up  */

  string_StringFree(Filename);
  string_StringFree(Path);
  pcheck_DeleteProof(Proof);

  flag_DeleteStore(Flags);
  symbol_DeletePrecedence(Precedence);
  desc_Delete(Description);
  
  subs_Free();
  cmdlne_Free();
  cont_Free();
  unify_Free();
  fol_Free();
  symbol_FreeAllSymbols();

#ifdef CHECK
  memory_Print();
  putchar('\n');
#endif

 return 0;
}


