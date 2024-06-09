/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                ANALYSIS OF CLAUSE SETS                 * */
/* *                                                        * */
/* *  $Module:   ANALYZE                                    * */ 
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
/* $Revision: 1.4 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-22 09:05:45 $                             * */
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


/* $RCSfile: analyze.h,v $ */
 
#ifndef _ANALYZE_
#define _ANALYZE_

#include "memory.h"
#include "clause.h"
#include "sort.h"
#include "list.h"
#include "graph.h"

/**************************************************************/
/* Data Structures and Constants                              */
/**************************************************************/

typedef enum {
  ana_EQUATIONS,          /* Problem contains any equations at all */
  ana_PEQUATIONS,         /* Problem contains positive equations */
  ana_NEQUATIONS,         /* Problem contains negative equations */
  ana_FUNCTIONS,          /* Problem contains non-constant function symbols */
  ana_PROP,               /* Problem contains propositional variables */
  ana_GROUND,             /* Problem contains non-propositional, non-equational ground atoms */
  ana_NONUNIT,            /* Problem contains non-unit clauses */
  ana_MONADIC,            /* Problem contains non-ground monadic predicates */
  ana_NONMONADIC,         /* Problem contains non-ground n-place predicates, n>1, not equality */
  ana_SORTRES,            /* Problem contains literal not(S(x)) for some S */
  ana_USORTRES,           /* Problem contains literal not(S(t)) for some S */
  ana_FINDOMAIN,          /* Problem contains clause implying a finite domain */
  ana_NONTRIVDOMAIN,      /* Problem contains clause implying a domain of size greater one */
  ana_CONGROUND,          /* Conjecture is ground */
  ana_TRANSITIVITY,       /* Problem contains transitivity axiom for some predicate exploited by chaining */

  ana_PUREEQUATIONAL,     /* Problem is purely equational */
  ana_PUREPROPOSITIONAL,  /* Problem is purely propositional */
  ana_SORTDECEQUATIONS,   /* True if all positive equations are sort decreasing with respect to
                                        the static sort theory contained in the problem */
  ana_SORTMANYEQUATIONS,  /* True if all positive equations have the
                             same sort on left and right hand side with
                             respect to the static sort theory
                             contained in the problem */
                             
  ana_MAXFEATURE            /* ana_MAXFEATURE is a final Dummy */
} ANA_FEATUREID;

typedef struct ANALYZE_HELP {
  BOOL boolarray[ana_MAXFEATURE];       /*all the boolean features of the problem*/

  LIST ana_FINITEMONADICPREDICATES;  /* List of monadic predicates with finite extension */
                 
  NAT  ana_AXIOMCLAUSES;      /* Number of axiom clauses */
  NAT  ana_CONCLAUSES;        /* Number of conjecture clauses */
  NAT  ana_NONHORNCLAUSES;    /* Number of Non-Horn clauses */  
  
  NAT  nontrivclausenumber;
  
  LIST transitivityaxioms;  
  /* In the case that chaining is activated, <transitivityaxioms> is a list of the         */
  /* transitivity axiom clauses that have been removed from the input clause set.          */ 
  /* The axioms are used to extract the clause number for proof documentation and          */
  /* possibly to be printed into the model.                                                */
} ANALYZE_NODE;

typedef ANALYZE_NODE *ANALYZE;

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

BOOL    ana_GetFeature(ANALYZE, ANA_FEATUREID);
BOOL*   ana_GetFeatureAddr(ANALYZE, ANA_FEATUREID);
void    ana_SetFeature(ANALYZE, ANA_FEATUREID, BOOL);
int     ana_GetAxiomClauses(ANALYZE);
int     ana_GetConClauses(ANALYZE);
int     ana_GetNonHornClauses(ANALYZE);
void    ana_IncAxiomClauses(ANALYZE);
void    ana_IncConClauses(ANALYZE);
void    ana_IncNonHornClauses(ANALYZE);
void    ana_SetAxiomClauses(ANALYZE, int);
void    ana_SetConClauses(ANALYZE, int);
void    ana_SetNonHornClauses(ANALYZE, int);
LIST    ana_GetFinMonPredList(ANALYZE);
void    ana_SetFinMonPredList(ANALYZE, LIST);
NAT     ana_NonTrivClauseNumber(ANALYZE);
void    ana_SetNonTrivClauseNumber(ANALYZE, NAT);
void    ana_AddTransAxClause(ANALYZE, CLAUSE);
LIST    ana_GetTransAxClauseList(ANALYZE);
void    ana_SetTransAxClauseList(ANALYZE, LIST);
CLAUSE  ana_LookupTransAxClause(ANALYZE, SYMBOL);
ANALYZE ana_Init(void);
void    ana_Clean(ANALYZE); 
void    ana_Free(ANALYZE);

void ana_AnalyzeProblem(ANALYZE, FLAGSTORE, LIST);
void ana_AnalyzeSortStructure(ANALYZE, LIST, FLAGSTORE, PRECEDENCE);
LIST ana_AnalyzeTransitivity(ANALYZE, LIST);
void ana_AutoConfiguration(ANALYZE, LIST, FLAGSTORE, PRECEDENCE);
void ana_ExploitSortAnalysis(ANALYZE,FLAGSTORE);
void ana_Print(ANALYZE, FLAGSTORE, PRECEDENCE);

#endif
