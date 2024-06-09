/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                INFERENCE RULES                         * */
/* *                                                        * */
/* *  $Module:   INFRULES                                   * */
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
/* $Revision: 1.6 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-25 07:51:40 $                             * */
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


/* $RCSfile: rules-inf.c,v $ */

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "rules-inf.h"

/**************************************************************/
/* previously Inlined Functions                               */
/**************************************************************/
LIST inf_Paramodulation(CLAUSE GivenClause,
					  SHARED_INDEX ShIndex,
					  FLAGSTORE Flags,
					  PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, an Index, usually the WorkedOffIndex, a 
           flag store and a precedence.
  RETURNS: A list of clauses derivable from the GivenClause by 
           paramodulation wrt. the Index. 
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{ 
  return list_Nconc(inf_GenSuperpositionLeft(GivenClause, ShIndex, FALSE,
					     FALSE, FALSE, Flags, Precedence),
		    inf_GenSuperpositionRight(GivenClause, ShIndex, FALSE,
					      FALSE, FALSE, Flags, Precedence));
}

LIST inf_OrderedParamodulation(CLAUSE GivenClause,
						 SHARED_INDEX ShIndex,
						 FLAGSTORE Flags,
						 PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, an Index, usually the WorkedOffIndex, a 
           flag store and a precedence.
  RETURNS: A list of clauses derivable from the Givenclause by 
           ordered paramodulation wrt. the Index. 
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  return list_Nconc(inf_GenSuperpositionLeft(GivenClause, ShIndex, TRUE,
					     FALSE, FALSE, Flags, Precedence),
		    inf_GenSuperpositionRight(GivenClause, ShIndex, TRUE,
					      FALSE, FALSE, Flags, Precedence));
}

LIST inf_SuperpositionLeft(CLAUSE GivenClause,
					     SHARED_INDEX ShIndex,
					     FLAGSTORE Flags,
					     PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, an Index, usually the WorkedOffIndex, a
           flag store, and a precedence.
  RETURNS: A list of clauses derivable from the Givenclause by 
           superposition left wrt. the Index. 
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  return inf_GenSuperpositionLeft(GivenClause,ShIndex,TRUE,TRUE,FALSE,Flags, Precedence);
}

LIST inf_SuperpositionRight(CLAUSE GivenClause,
					      SHARED_INDEX ShIndex,
					      FLAGSTORE Flags,
					      PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, an Index, usually the WorkedOffIndex, a 
           flag store and a precedence.
  RETURNS: A list of clauses derivable from the Givenclause by 
           superposition right wrt. the Index. 
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  return inf_GenSuperpositionRight(GivenClause,ShIndex,TRUE,TRUE,FALSE,Flags, Precedence);
}

/**************************************************************/
/* Inline Functions ends here                                 */
/**************************************************************/



/**************************************************************/
/* Some auxiliary functions for testing postconditions        */
/**************************************************************/

static BOOL inf_LitMax(CLAUSE Clause, int i, int j, SUBST Subst, BOOL Strict,
		       FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, the index of a maximal literal, another 
           literal index, a substitution, a boolean flag, a 
	   flag store and a precedence.
  RETURNS: If <Strict>=FALSE the function returns TRUE iff the
           literal at index <i> is still maximal in the 
	   instantiated clause.
	   If <Strict>=TRUE the function returns TRUE iff the
	   literal is STRICTLY maximal in the instantiated 
	   clause. The literal at index j is omitted at literal 
	   comparison.
	   However, setting j to a negative number ensures that
	   all literals are compared with the literal at 
	   index <i>.
  CAUTION: DON'T call this function with a clause with selected
           literals!
***************************************************************/
{
  TERM       Max, LitTerm;
  LITERAL    Lit;
  ord_RESULT Compare;
  int        k, l;

#ifdef CHECK
  if (!clause_LiteralIsMaximal(clause_GetLiteral(Clause, i)) ||
      (Strict &&
       !clause_LiteralGetFlag(clause_GetLiteral(Clause, i), STRICTMAXIMAL))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_LitMax: Literal %d isn't %smaximal.",
		     i, Strict ? "strictly " : "");
    misc_FinishErrorReport();
  }
  if (i < clause_FirstAntecedentLitIndex(Clause) ||
      i > clause_LastSuccedentLitIndex(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_LitMax: Literal index %d is out of range.", i);
    misc_FinishErrorReport();
  }
  /* If literal <i> is selected, there's no need to check for maximality, */
  /* if <i> isn't selected, but there're other literals selected,         */
  /* inferences with literal <i> are forbidden.                           */ 
  if (clause_GetFlag(Clause, CLAUSESELECT)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_LitMax: There're selected literals.", i);
    misc_FinishErrorReport();
  }    
#endif

  Lit = clause_GetLiteral(Clause, i);
  /* Check necessary condition */
  if (!clause_LiteralIsMaximal(Lit) ||
      (Strict && !clause_LiteralGetFlag(Lit, STRICTMAXIMAL)))
    return FALSE;
  /* Only antecedent and succedent literals are compared, so if there's */
  /* only one such literal, it's maximal.                               */
  /* If the substitution is empty, the necessary condition tested above */
  /* is sufficient, too.                                                */
  if ((clause_NumOfAnteLits(Clause) + clause_NumOfSuccLits(Clause) == 1) ||
      subst_Empty(Subst))
    return TRUE;

  l   = clause_LastSuccedentLitIndex(Clause);
  Max = subst_Apply(Subst,term_Copy(clause_GetLiteralTerm(Clause,i)));

  for (k = clause_FirstAntecedentLitIndex(Clause); k <= l; k++)
    if (k != i && k != j &&
	clause_LiteralIsMaximal(clause_GetLiteral(Clause, k))) {
      /* Only compare with maximal literals, since for every non-maximal */
      /* literal, there's at least one maximal literal, that is bigger.  */
      LitTerm = subst_Apply(Subst,term_Copy(clause_GetLiteralTerm(Clause,k)));
      Compare = ord_LiteralCompare(Max,				   
           clause_LiteralGetOrderStatus(clause_GetLiteral(Clause,i)),
				   LitTerm,				   
           clause_LiteralGetOrderStatus(clause_GetLiteral(Clause,k)),
				   TRUE, Flags, Precedence);
      if (Compare == ord_SmallerThan() || (Strict && Compare == ord_Equal())) {
	term_Delete(Max);
	term_Delete(LitTerm);
	return FALSE;
      }
      term_Delete(LitTerm);
    }
  term_Delete(Max);

  return TRUE;
}


static BOOL inf_LiteralsMax(CLAUSE Clause, int i, SUBST Subst,
			    CLAUSE PartnerClause, int j, SUBST PartnerSubst,
			    FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   The parents of a resolution inference, the respective
           literal indices and substitutions, a flag store and 
	   a precedence.
  RETURNS: TRUE iff the positive/negative literals are still
           strictly maximal/maximal in the instantiated clause.
	   If a negative literal is selected, no comparison
	   is made.
***************************************************************/
{
#ifdef CHECK
  if ((clause_GetFlag(Clause, CLAUSESELECT) &&
       !clause_LiteralGetFlag(clause_GetLiteral(Clause,i),LITSELECT)) ||
      (clause_GetFlag(PartnerClause, CLAUSESELECT) &&
       !clause_LiteralGetFlag(clause_GetLiteral(PartnerClause,j),LITSELECT))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_LiteralsMax: Another literal is selected.");
    misc_FinishErrorReport();
  }
#endif

  if (!clause_GetFlag(Clause, CLAUSESELECT) &&
      !inf_LitMax(Clause,i,-1,Subst,
		  i>clause_LastAntecedentLitIndex(Clause),Flags, Precedence))
    return FALSE;
  if (!clause_GetFlag(PartnerClause, CLAUSESELECT) &&
      !inf_LitMax(PartnerClause,j,-1,PartnerSubst,
		  j>clause_LastAntecedentLitIndex(PartnerClause), Flags, Precedence))
    return FALSE;

  return TRUE;
}


static BOOL inf_LitMaxWith2Subst(CLAUSE Clause, int i, int j, SUBST Subst2,
				 SUBST Subst1, BOOL Strict, FLAGSTORE Flags,
				 PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, the index of a maximal literal, another 
           literal index, two substitutions, a boolean flag, 
	   a flag store and a precedence.
  RETURNS: In contrast to the function inf_LitMax this function 
           compares the literals with respect to the composition 
	   of the two substitutions Subst2 ° Subst1.
	   If <Strict>=FALSE the function returns TRUE iff the
           literal at index <i> is still maximal in the 
	   instantiated clause.
	   If <Strict>=TRUE the function returns TRUE iff the
	   literal is STRICTLY maximal in the instantiated 
	   clause.
	   The literal at index j is omitted at literal 
	   comparison.
	   However, setting j to a negative number ensures that
	   all literals are compared with the literal at 
	   index <i>.
  CAUTION: DON'T call this function with a clause with selected
           literals!
***************************************************************/
{
  TERM       Max, LitTerm;
  LITERAL    Lit;
  ord_RESULT Compare;
  int        k, l;

#ifdef CHECK
  if (!clause_LiteralIsMaximal(clause_GetLiteral(Clause, i)) ||
      (Strict &&
       !clause_LiteralGetFlag(clause_GetLiteral(Clause, i), STRICTMAXIMAL))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_LitMaxWith2Subst: Literal %d isn't %smaximal.",
		     i, Strict ? "strictly " : "");
    misc_FinishErrorReport();
  }
  if (i < clause_FirstAntecedentLitIndex(Clause) ||
      i > clause_LastSuccedentLitIndex(Clause)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_LitMaxWith2Subst: Literal index %d is out of range.", i);
    misc_FinishErrorReport();
  }
  /* If literal <i> is selected, there's no need to check for maximality, */
  /* if <i> isn't selected, but there're other literals selected,         */
  /* inferences with literal <i> are forbidden.                           */ 
  if (clause_GetFlag(Clause, CLAUSESELECT)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_LitMaxWith2Subst: There're selected literals.", i);
    misc_FinishErrorReport();
  }    
#endif

  Lit = clause_GetLiteral(Clause, i);
  /* Check necessary condition */
  if (!clause_LiteralIsMaximal(Lit) ||
      (Strict && !clause_LiteralGetFlag(Lit, STRICTMAXIMAL)))
    return FALSE;
  /* Only antecedent and succedent literals are compared, so if there's    */
  /* only one such literal, it's maximal.                                  */
  /* If both substitutions are empty, the necessary condition tested above */
  /* is sufficient, too.                                                   */
  if ((clause_NumOfAnteLits(Clause) + clause_NumOfSuccLits(Clause) == 1) ||
      (subst_Empty(Subst1) && subst_Empty(Subst2)))
    return TRUE;

  l   = clause_LastSuccedentLitIndex(Clause);
  Max = subst_Apply(Subst1, term_Copy(clause_GetLiteralTerm(Clause,i)));
  Max = subst_Apply(Subst2, Max);

  for (k = clause_FirstAntecedentLitIndex(Clause); k <= l; k++)
    if (k != i && k != j &&
	clause_LiteralIsMaximal(clause_GetLiteral(Clause, k))) {
      /* Only compare with maximal literals, since for every non-maximal */
      /* literal, there's at least one maximal literal, that is bigger.  */
      LitTerm = subst_Apply(Subst1,term_Copy(clause_GetLiteralTerm(Clause,k)));
      LitTerm = subst_Apply(Subst2, LitTerm);
      Compare = ord_LiteralCompare(Max,				   
           clause_LiteralGetOrderStatus(clause_GetLiteral(Clause,i)),
				   LitTerm,				   
           clause_LiteralGetOrderStatus(clause_GetLiteral(Clause,k)),
				   TRUE, Flags, Precedence);
      if (Compare == ord_SmallerThan() || (Strict && Compare == ord_Equal())) {
	term_Delete(Max);
	term_Delete(LitTerm);
	return FALSE;
      }
      term_Delete(LitTerm);
    }
  term_Delete(Max);

  return TRUE;
}


static BOOL inf_LiteralsMaxWith2Subst(CLAUSE Clause, int i, CLAUSE PartnerClause,
				      int j, SUBST Subst2, SUBST Subst1,
				      FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   The parents of a resolution inference, the 
           respective literal indices and substitutions, a 
	   flag store and a precedence.
  RETURNS: In contrast to the function inf_LiteralsMax
           the composition Subst2 ° Subst1 is applied to both
	   clauses.
	   The function returns TRUE iff the positive/negative 
	   literals are still strictly maximal/maximal in the 
	   instantiated clause.
	   If a negative literal is selected, no comparison
	   is made.
***************************************************************/
{
#ifdef CHECK
  if ((clause_GetFlag(Clause, CLAUSESELECT) &&
       !clause_LiteralGetFlag(clause_GetLiteral(Clause,i),LITSELECT)) ||
      (clause_GetFlag(PartnerClause, CLAUSESELECT) &&
       !clause_LiteralGetFlag(clause_GetLiteral(PartnerClause,j),LITSELECT))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_LiteralsMaxWith2Subst: Another literal is selected.");
    misc_FinishErrorReport();
  }
#endif

  if (!clause_GetFlag(Clause, CLAUSESELECT) &&
      !inf_LitMaxWith2Subst(Clause, i, -1, Subst2, Subst1,
			    i>clause_LastAntecedentLitIndex(Clause), Flags, Precedence))
    return FALSE;
  if (!clause_GetFlag(PartnerClause, CLAUSESELECT) &&
      !inf_LitMaxWith2Subst(PartnerClause, j, -1, Subst2, Subst1,
		 j>clause_LastAntecedentLitIndex(PartnerClause), Flags, Precedence))
    return FALSE;

  return TRUE;
}


/**************************************************************/
/* Inference rules                                            */
/**************************************************************/

LIST inf_EqualityResolution(CLAUSE GivenClause, BOOL Ordered, FLAGSTORE Flags,
			    PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause and a flag determining whether ordering
           constraints apply.
	   For <Ordered>=TRUE the function makes Equality Resolution
	   inferences, for <Ordered>=FALSE Reflexivity Resolution
	   inferences are made.
	   A flag store.
	   A precedence.
  RETURNS: A list of clauses inferred from the GivenClause by
           Equality/Reflexivity Resolution.
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  LIST    Result;
  LITERAL ActLit;
  int     i, last;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_EqualityResolution: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  if (clause_HasEmptyAntecedent(GivenClause) ||
      !clause_HasSolvedConstraint(GivenClause)) {
    return list_Nil();
  }

  Result          = list_Nil();
  last            = clause_LastAntecedentLitIndex(GivenClause);
  
  for (i = clause_FirstAntecedentLitIndex(GivenClause); i <= last; i++) {
    ActLit = clause_GetLiteral(GivenClause, i);
    
    if (clause_LiteralIsEquality(ActLit) &&
	(clause_LiteralGetFlag(ActLit,LITSELECT) ||
	 (!clause_GetFlag(GivenClause,CLAUSESELECT) &&
	  (!Ordered || clause_LiteralIsMaximal(ActLit))))) {
      TERM Atom;
      
      Atom = clause_GetLiteralAtom(GivenClause, i);
      
      cont_Check();
      if (unify_UnifyCom(cont_LeftContext(),
			 term_FirstArgument(Atom), 
			 term_SecondArgument(Atom))) {
	SUBST  mgu;
	CLAUSE NewClause;
	int    j, k, bound;
	
	subst_ExtractUnifierCom(cont_LeftContext(), &mgu);
	/* Check postcondition */
	if (clause_LiteralGetFlag(ActLit,LITSELECT) ||
	    !Ordered || inf_LitMax(GivenClause, i, -1, mgu, 
				   FALSE, Flags, Precedence)) {
	  
	  NewClause = clause_CreateBody(clause_Length(GivenClause) - 1);
	  clause_SetNumOfConsLits(NewClause, clause_NumOfConsLits(GivenClause));
	  clause_SetNumOfAnteLits(NewClause,
				  (clause_NumOfAnteLits(GivenClause) - 1));
	  clause_SetNumOfSuccLits(NewClause, clause_NumOfSuccLits(GivenClause));
	  
	  bound = clause_LastLitIndex(GivenClause);
	  /* j iterates over the given clause, k iterates over the new one */
	  for (j = k = clause_FirstLitIndex(); j <= bound; j++) {
	    if (j != i) {
	      clause_SetLiteral(NewClause, k, 
	        clause_LiteralCreate(subst_Apply(mgu,
	          term_Copy(clause_GetLiteralTerm(GivenClause,j))),NewClause));
	      k++;
	    }
	  }
	  clause_SetDataFromFather(NewClause, GivenClause, i, Flags, Precedence);
	  clause_SetFromEqualityResolution(NewClause);
	  
	  Result = list_Cons(NewClause, Result);
	}
	subst_Delete(mgu);
      }
      cont_Reset();
    }	/* end of if 'ActLit is maximal'. */
  } /*end of for 'all literals'. */
  return(Result);
}

static CLAUSE inf_ApplyEqualityFactoring(CLAUSE Clause, TERM Left, TERM Right,
					 int i, int j, SUBST Subst,
					 FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, two terms, two indices in the clause,
           a substitution, a flag store and a precedence.
  RETURNS: A new clause, where <Left>=<Right> is added as antecedent atom,
           the <i>th literal is deleted, the <j>th kept and
	   <Subst> is applied to a copy of <clause>.
***************************************************************/
{
  CLAUSE NewClause;
  TERM   Atom;
  int    k,c,a,s;

  NewClause = clause_CreateBody(clause_Length(Clause));

  c = clause_LastConstraintLitIndex(Clause);
  clause_SetNumOfConsLits(NewClause, clause_NumOfConsLits(Clause));
  a = clause_LastAntecedentLitIndex(Clause);
  clause_SetNumOfAnteLits(NewClause, clause_NumOfAnteLits(Clause) + 1);
  s = clause_LastSuccedentLitIndex(Clause);
  clause_SetNumOfSuccLits(NewClause, clause_NumOfSuccLits(Clause) - 1);

  for (k = clause_FirstLitIndex(); k <= c; k++) {
    clause_SetLiteral(NewClause, k, 
      clause_LiteralCreate(subst_Apply(Subst,
	term_Copy(clause_GetLiteralTerm(Clause, k))),NewClause));
  }

  for ( ; k <= a; k++) {
    clause_SetLiteral(NewClause, k, 
      clause_LiteralCreate(subst_Apply(Subst,
        term_Copy(clause_GetLiteralTerm(Clause, k))),NewClause));
  }

  Atom = term_Create(fol_Equality(),
		     list_Cons(term_Copy(Left),list_List(term_Copy(Right))));

  clause_SetLiteral(NewClause, k, clause_LiteralCreate(
    term_Create(fol_Not(), list_List(subst_Apply(Subst,Atom))), NewClause));

  a = 1; /* Shift */

  for ( ; k <= s; k++) {
    if (k == i)
      a = 0;
    else {
      clause_SetLiteral(NewClause, (k + a),
	clause_LiteralCreate(subst_Apply(Subst,
	  term_Copy(clause_GetLiteralTerm(Clause, k))),NewClause));
    }
  }

  clause_AddParentClause(NewClause, clause_Number(Clause));
  clause_AddParentLiteral(NewClause, j);
  clause_SetDataFromFather(NewClause, Clause, i, Flags, Precedence);

  clause_SetFromEqualityFactoring(NewClause);

  return NewClause;
}


static BOOL inf_EqualityFactoringApplicable(CLAUSE Clause, int i, int j,
              TERM Left, TERM Right, TERM PartnerLeft, TERM PartnerRight, 
              SUBST Subst,
					    FLAGSTORE Flags,
					    PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause <Clause>, the index <i> of a maximal 
           equality literal in <Clause> <j>, <Left> and 
	   <Right> are the left and right side of the literal, 
	   where <Right> is not greater than <Left> wrt the 
	   ordering, a unifier <Subst>, a flag store and a 
	   precedence.
  RETURNS: TRUE iff the literal at index <i> is strictly 
           maximal in the instantiated clause and <Right> is 
	   not greater than or equal to <Left> after 
	   application of the substitution.
	   Otherwise, the function returns FALSE.
***************************************************************/
{
  ord_RESULT Help;
  TERM LeftS, RightS, PartnerLeftS, PartnerRightS;  

  RightS = subst_Apply(Subst, term_Copy(Right));
  PartnerRightS = subst_Apply(Subst, term_Copy(PartnerRight));
  
  /* The cross-test */
  if ((Help = ord_Compare(RightS, PartnerRightS, Flags, Precedence)) == ord_SmallerThan()) {
      term_Delete(RightS);
      term_Delete(PartnerRightS);
      return FALSE;
    }
  
  /* Literal oriented? */
  if (!clause_LiteralIsOrientedEquality(clause_GetLiteral(Clause, i))) {
    LeftS  = subst_Apply(Subst, term_Copy(Left));
    if ((Help = ord_Compare(LeftS, RightS, Flags, Precedence)) == ord_SmallerThan()
        || Help == ord_Equal()) {
      term_Delete(LeftS);
      term_Delete(RightS);
      term_Delete(PartnerRightS);
      return FALSE;
    }
    term_Delete(LeftS);    
  }

  term_Delete(RightS);
  
  /* Partner literal oriented? */
  if (!clause_LiteralIsOrientedEquality(clause_GetLiteral(Clause, j))) {
    PartnerLeftS  = subst_Apply(Subst, term_Copy(PartnerLeft));
    
    if ((Help = ord_Compare(PartnerLeftS, PartnerRightS, Flags, Precedence)) == ord_SmallerThan()
        || Help == ord_Equal()) {
      term_Delete(PartnerLeftS);
      term_Delete(PartnerRightS);      
      return FALSE;
    }
    term_Delete(PartnerLeftS);
  }

  term_Delete(PartnerRightS);

  /* Literal maximal? */
  return inf_LitMax(Clause, i, -1, Subst, FALSE, Flags, Precedence);
}


LIST inf_EqualityFactoring(CLAUSE GivenClause, FLAGSTORE Flags,
			   PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a flag store and a precedence.
  RETURNS: A list of clauses derivable from 'GivenClause' by EF.
***************************************************************/
{
  LIST    Result;
  LITERAL ActLit;
  int     i, j, last;
  SUBST   mgu;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_EqualityFactoring: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  if (clause_HasEmptySuccedent(GivenClause) ||
      clause_GetFlag(GivenClause, CLAUSESELECT) ||
      !clause_HasSolvedConstraint(GivenClause)) {
    return list_Nil();
  }

  Result = list_Nil();
  
  last = clause_LastSuccedentLitIndex(GivenClause);
  
  for (i = clause_FirstSuccedentLitIndex(GivenClause); i <= last; i++) {
    
    ActLit = clause_GetLiteral(GivenClause, i);
    
    if (clause_LiteralIsMaximal(ActLit) &&
	clause_LiteralIsEquality(ActLit)) {
      TERM    Atom, Left, Right;
      LITERAL PartnerLit;
      
      Atom  = clause_LiteralAtom(ActLit);
      Left  = term_FirstArgument(Atom);
      Right = term_SecondArgument(Atom);
      
      for (j = clause_FirstSuccedentLitIndex(GivenClause); j <= last; j++) {
	PartnerLit = clause_GetLiteral(GivenClause, j);
	if (i != j && clause_LiteralIsEquality(PartnerLit)) {
	  /* i==j can be excluded since this inference would either generate */
	  /* a copy of the given clause (if one side of the equality is      */
	  /* unified with itself), or generate a tautology (if different     */
	  /* sides of the equality are unified).                             */
	  TERM PartnerAtom, PartnerLeft, PartnerRight;
	  
	  PartnerAtom  = clause_LiteralAtom(PartnerLit);
	  PartnerLeft  = term_FirstArgument(PartnerAtom);
	  PartnerRight = term_SecondArgument(PartnerAtom);

	  /* try <Left> and <PartnerLeft> */
	  cont_Check();
	  if (ord_IsNotGreaterThan(ord_Compare(PartnerRight,Right,Flags,Precedence)) && /* no duplicates */
	      unify_UnifyCom(cont_LeftContext(), Left, PartnerLeft)) {
	    subst_ExtractUnifierCom(cont_LeftContext(), &mgu);
	    if (inf_EqualityFactoringApplicable(GivenClause, i, j,
						Left, Right, PartnerLeft, PartnerRight,
						mgu, Flags, Precedence))
	      Result = list_Cons(inf_ApplyEqualityFactoring(GivenClause,Right,
							    PartnerRight,i,j,
							    mgu,Flags, 
							    Precedence),
				 Result);
	    subst_Delete(mgu);
	  }
	  cont_Reset();

	  /* try <Left> and <PartnerRight> */
	  cont_Check();
	  if (!clause_LiteralIsOrientedEquality(PartnerLit) &&
        ord_IsNotGreaterThan(ord_Compare(PartnerLeft,Right,Flags,Precedence)) &&
        unify_UnifyCom(cont_LeftContext(), Left, PartnerRight)) {
	    subst_ExtractUnifierCom(cont_LeftContext(), &mgu);
	    
	    if (inf_EqualityFactoringApplicable(GivenClause, i, j,
						Left, Right, PartnerRight, PartnerLeft,
						mgu, Flags, Precedence))
	      Result = list_Cons(inf_ApplyEqualityFactoring(GivenClause,Right,
							    PartnerLeft,i,j,
							    mgu,Flags, 
							    Precedence),
				 Result);
	    subst_Delete(mgu);
	  }
	  cont_Reset();

	  if (!clause_LiteralIsOrientedEquality(ActLit)) {
	    /* try <Right> and <PartnerLeft> */
	    cont_Check();
	    if (ord_IsNotGreaterThan(ord_Compare(PartnerRight,Left,Flags,Precedence)) &&
          unify_UnifyCom(cont_LeftContext(), Right, PartnerLeft)) {
	      subst_ExtractUnifierCom(cont_LeftContext(), &mgu);
	      
	      if (inf_EqualityFactoringApplicable(GivenClause, i, j,
						  Right, Left, PartnerLeft, PartnerRight,
						  mgu, Flags, Precedence))
		Result = list_Cons(inf_ApplyEqualityFactoring(GivenClause,Left,
							      PartnerRight,i,j,
							      mgu,Flags,
							      Precedence),
				   Result);
	      subst_Delete(mgu);
	    }
	    cont_Reset();
	    
	    /* try <Right> and <PartnerRight> */
	    cont_Check();
	    if (!clause_LiteralIsOrientedEquality(PartnerLit) &&
          ord_IsNotGreaterThan(ord_Compare(PartnerLeft,Left,Flags,Precedence)) &&
          unify_UnifyCom(cont_LeftContext(), Right, PartnerRight)) {
	      subst_ExtractUnifierCom(cont_LeftContext(), &mgu);
	      
	      if (inf_EqualityFactoringApplicable(GivenClause, i, j,
						  Right, Left, PartnerRight, PartnerLeft,
						  mgu, Flags, Precedence))
		Result = list_Cons(inf_ApplyEqualityFactoring(GivenClause,Left,
							      PartnerLeft,i,j,
							      mgu,Flags,
							      Precedence),
				   Result);
	      subst_Delete(mgu);
	    }
	    cont_Reset();
	  }
	}
      }
    }
  }
  return Result;
}


/* START of block with new term replacement */

static BOOL inf_NAllTermsRplac(TERM Term, TERM TestTerm, TERM RplacTerm,
			       SUBST Subst)
/**************************************************************
  INPUT:   Three terms, a substitution and an integer.
           All occurrences of <TestTerm> in <Term> are replaced
           by <RplacTerm>. The substitution <Subst> is applied to <Term>.
  RETURNS: TRUE, if TestTerm was replaced by RplacTerm,
           FALSE otherwise.
  EFFECT:  <Term> is destructively changed!
***************************************************************/
{
  LIST ArgListNode;
  BOOL Replaced;
  int  Bottom;

  Replaced = FALSE;

  /* check if whole term must be replaced */
  if (term_Equal(Term, TestTerm)) {
    term_RplacTop(Term,term_TopSymbol(RplacTerm));
    ArgListNode = term_ArgumentList(Term);
    term_RplacArgumentList(Term, term_CopyTermList(term_ArgumentList(RplacTerm)));
    term_DeleteTermList(ArgListNode);
    return TRUE;
  }

  if (term_IsVariable(Term))
    subst_Apply(Subst, Term);

  /* if not, scan whole term.  */
  if (!list_Empty(term_ArgumentList(Term))) {

    Bottom = stack_Bottom();
    stack_Push(term_ArgumentList(Term));

    while (!stack_Empty(Bottom)) {
      ArgListNode = stack_Top();
      Term        = (TERM)list_Car(ArgListNode);
      stack_RplacTop(list_Cdr(ArgListNode));

      if (term_Equal(Term, TestTerm)) {
	Replaced = TRUE;
	list_Rplaca(ArgListNode, term_Copy(RplacTerm));
	term_Delete(Term);
      }      
      else {
	if (term_IsComplex(Term))
	  stack_Push(term_ArgumentList(Term));
	else if (term_IsVariable(Term))
	  subst_Apply(Subst,Term);
      }

      /* remove empty lists (corresponding to scanned terms) */
      while (!stack_Empty(Bottom) && list_Empty(stack_Top()))
	stack_Pop();
    }
  }
  return Replaced;
}


static TERM inf_AllTermsRplac(TERM Term, TERM TestTerm, TERM RplacTerm,
			      SUBST Subst)
/**************************************************************
  INPUT:   Three terms, a substitution.
           All occurrences of <TestTerm> in A COPY of <Term> are replaced
           by <RplacTerm>. The substitution <Subst> is applied to 
	   the copy of <Term>. If no occurrence is found,
	   NULL is returned.
	   This function is not destructive
	   like NAllTermRplac.
  RETURNS: TRUE, if TestTerm was replaced by RplacTerm,
           FALSE otherwise.
***************************************************************/
{ 
  TERM ActTerm = term_Copy(Term);
  
  if (!inf_NAllTermsRplac(ActTerm,TestTerm, RplacTerm, Subst )) {
    term_Delete(ActTerm);
    ActTerm = NULL;
  }
 
  return(ActTerm);
}

static TERM inf_AllTermsSideRplacs(TERM Term, TERM TestTerm, TERM RplacTerm,
				   SUBST Subst, BOOL Right)
/**************************************************************
  INPUT:   Three terms, a substitution and a boolean flag. 
           <Term> is typically an equality term.
  RETURNS: If <TestTerm> occurs in the right (Right=TRUE) or
           left side (Right=FALSE) of <Term>:

           A copy of the term where all occurrences of <TestTerm>
	   in the ENTIRE <Term> are replaced by <RplacTerm> and
	   the substitution <Subst> is applied to all other subterms.

	   If <TestTerm> does not occur in the right/left side of
	   <Term>, NULL is returned.

	   In non-equality terms, The 'sides' correspond to the 
	   first and second argument of the term.
***************************************************************/
{
  TERM   ActTerm = term_Copy(Term); 
  TERM   ReplSide, OtherSide; /* ReplSide is the side in which terms are
				 replaced */

  if (Right) { 
    ReplSide  = term_SecondArgument(ActTerm);
    OtherSide = term_FirstArgument(ActTerm);
  }
  else { 
    ReplSide  = term_FirstArgument(ActTerm);
    OtherSide = term_SecondArgument(ActTerm);
  }

  if (inf_NAllTermsRplac(ReplSide, TestTerm, RplacTerm, Subst))
    /* If <TestTerm> occurs in <ReplSide> also replace it in <OtherSide>. */
    inf_NAllTermsRplac(OtherSide, TestTerm, RplacTerm, Subst);
  else { 
     term_Delete(ActTerm);
     ActTerm = NULL;
  }

  return ActTerm;
}


static TERM inf_AllTermsRightRplac(TERM Term, TERM TestTerm, TERM RplacTerm,
				   SUBST Subst)
/**************************************************************
  INPUT:   Three terms, a substitution.
           <Term> is typically an equality term.
  RETURNS: See inf_AllTermSideRplac with argument
           'Right' set to TRUE
**************************************************************/
{
  return(inf_AllTermsSideRplacs(Term, TestTerm, RplacTerm, Subst, TRUE));
}


static TERM inf_AllTermsLeftRplac(TERM Term, TERM TestTerm, TERM RplacTerm,
				  SUBST Subst)
/**************************************************************
  INPUT:   Three terms, a substitution.
           <Term> is typically an equality term.
  RETURNS: See inf_AllTermSideRplac with argument
           'Right' set to FALSE.
***************************************************************/
{
  return(inf_AllTermsSideRplacs(Term, TestTerm, RplacTerm, Subst, FALSE));
}


/*  END of block with new term replacement */


static CLAUSE inf_ApplyGenSuperposition(CLAUSE Clause, int ci, SUBST Subst,
					CLAUSE PartnerClause, int pci,
					SUBST PartnerSubst, TERM SupAtom,
					BOOL Right, BOOL OrdPara, BOOL MaxPara,
					FLAGSTORE Flags, PRECEDENCE Precedence)
/************************************************************** 
  INPUT:  Two clauses where a generalized superposition inference can be
          applied using the positive equality literal <i> from <Clause> with
          subst <Subst> using the literal <j> from <PartnerClause> with subst
          <PartnerSubst> where SupAtom is a derivable atom. Returns
	  NULL if SupAtom is NULL.

	   Right is TRUE if the inference is a superposition right inference
	   Right is FALSE if the inference is a superposition left inference,

	   where the inference is selected by MaxPara and OrdPara:
	   (see also inf_GenSuperpositionLeft)
	   
	   OrdPara=TRUE, MaxPara=TRUE 
	   -> Superposition (Left or Right)

	   OrdPara=TRUE, MaxPara=FALSE
	   -> ordered Paramodulation

	   OrdPara=FALSE, MaxPara=FALSE
	   -> simple Paramodulation

	   OrdPara=FALSE, MaxPara=TRUE
	   -> not defined

	   A flag store.
	   A precedence.
  RETURNS: The new clause.
  MEMORY:  Memory for the new clause is allocated.
***************************************************************/
{
  CLAUSE NewClause;
  int    j,lc,la,ls,pls,pla,plc,help;

#ifdef CHECK
  if (!OrdPara && MaxPara) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_ApplyGenSuperposition : Illegal inference");
    misc_ErrorReport("\n rule selection, OrdPara=FALSE and MaxPara=TRUE.");
    misc_FinishErrorReport();
  }
  if (SupAtom == NULL) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_ApplyGenSuperposition: Atom is NULL.");
    misc_FinishErrorReport();
    return clause_Null();
  }
#endif

  pls = clause_LastSuccedentLitIndex(PartnerClause);
  pla = clause_LastAntecedentLitIndex(PartnerClause);
  plc = clause_LastConstraintLitIndex(PartnerClause);

  ls  = clause_LastSuccedentLitIndex(Clause);
  la  = clause_LastAntecedentLitIndex(Clause);
  lc  = clause_LastConstraintLitIndex(Clause);


  NewClause = clause_CreateBody(clause_Length(Clause) - 1 +
				clause_Length(PartnerClause));

  clause_SetNumOfConsLits(NewClause, (clause_NumOfConsLits(Clause) +
				      clause_NumOfConsLits(PartnerClause)));
  clause_SetNumOfAnteLits(NewClause, (clause_NumOfAnteLits(Clause) +
				      clause_NumOfAnteLits(PartnerClause)));
  clause_SetNumOfSuccLits(NewClause, ((clause_NumOfSuccLits(Clause) -1)+
				      clause_NumOfSuccLits(PartnerClause)));

  /* First set the literals from the Clause : */
    
  for (j = clause_FirstLitIndex(); j <= lc; j++) {
    clause_SetLiteral(NewClause, j, 
      clause_LiteralCreate(subst_Apply(Subst, term_Copy(
       	clause_GetLiteralTerm(Clause, j))),NewClause));
  }

  /* help = number of literals to leave empty */
  help = clause_NumOfConsLits(PartnerClause);

  for ( ; j <= la; j++) {
    clause_SetLiteral(NewClause, (j + help), 
      clause_LiteralCreate(subst_Apply(Subst, term_Copy(
    	clause_GetLiteralTerm(Clause, j))),NewClause));
  }

  /* help = number of literals to leave empty */
  help += clause_NumOfAnteLits(PartnerClause);

  for ( ; j <= ls; j++) {
    if (j != ci) {
      /* The literal used in the inference isn't copied */
      clause_SetLiteral(NewClause, (j + help), 
       	clause_LiteralCreate(subst_Apply(Subst,
	  term_Copy(clause_GetLiteralTerm(Clause, j))),NewClause));

    } else {
      /*the index has to be decreased to avoid an empty literal! */
      help--;
    }
  }

  /* Now we consider the PartnerClause : */

  /* help = number of already set constraint (Clause) literals */
  help = clause_NumOfConsLits(Clause);

  for (j = clause_FirstLitIndex(); j <= plc; j++) {
    clause_SetLiteral(NewClause, (j + help), 
      clause_LiteralCreate(subst_Apply(PartnerSubst,
        term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));
  }

  /* help = number of already set constraint and antecedent Given-literals */
  help += clause_NumOfAnteLits(Clause);

  for ( ; j <= pla; j++) {
    if (j != pci) {
      clause_SetLiteral(NewClause, (j + help), 
	clause_LiteralCreate(subst_Apply(PartnerSubst,
	  term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));
    } else {
      /* The PartnerLit is modified appropriately!   */
      clause_SetLiteral(NewClause, (j + help), clause_LiteralCreate(
	term_Create(fol_Not(),list_List(SupAtom)), NewClause));
    }
  }


  /* help = number of already set Given-literals */
  help = clause_Length(Clause) - 1;

  for ( ; j <= pls; j++) {
    if (j != pci) {
      /* The PartnerLit isn't copied! */
      clause_SetLiteral(NewClause, (j + help), 
	clause_LiteralCreate(subst_Apply(PartnerSubst,
	  term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));

    } else {
      /* The PartnerLit is modified appropriately!   */
      clause_SetLiteral(NewClause, (j + help),
			clause_LiteralCreate(SupAtom, NewClause));
    }
  }
    
  /* 
   *  Set inference type. Note that, in the case of (ordered) paramodulation,
   *  we do not distinguish which side was paramodulated into, as compared
   *  to the case of superposition. 
   */

  if (OrdPara && MaxPara) {
    if (Right)
      clause_SetFromSuperpositionRight(NewClause);
    else
      clause_SetFromSuperpositionLeft(NewClause);
  } 
  else if (OrdPara && !MaxPara)
    clause_SetFromOrderedParamodulation(NewClause);
  else 
    clause_SetFromParamodulation(NewClause);

  clause_SetDataFromParents(NewClause, PartnerClause, pci, Clause, ci, Flags,
			    Precedence);

  return NewClause;
}


/* START of block with new superposition right rule */

static LIST inf_GenLitSPRight(CLAUSE Clause, TERM Left, TERM Right, int i,
			      SHARED_INDEX ShIndex, BOOL OrdPara, BOOL MaxPara,
			      FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause (unshared) with a positive equality literal
           at position <i> where <Left> and <Right> are the arguments
	   of just that literal and <Right> is not greater wrt. the
	   ordering than <Left>,
	   two boolean flags for controlling inference
           preconditions (see inf_GenSuperpositionRight),
	   a flag store and a precedence.
  RETURNS: A list of clauses derivable with the literals owning
           clause by general superposition right wrt. the Index.
	   (see inf_GenSuperpositionRight for selection of inference
	   rule by OrdPara/MaxPara)
  MEMORY:  The list of clauses is extended, where memory for the
           list and the clauses is allocated.
***************************************************************/
{
  LIST Result, Terms;

  Result  = list_Nil();
  Terms   = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
			  cont_RightContext(), Left);

  for ( ; !list_Empty(Terms); Terms = list_Pop(Terms)) {
    LIST Lits;
    TERM Term;

    Term = (TERM)list_First(Terms);

    if (!term_IsVariable(Term) && !symbol_IsPredicate(term_TopSymbol(Term))) {

      Lits = sharing_GetDataList(Term, ShIndex);

      for ( ; !list_Empty(Lits); Lits = list_Pop(Lits)) {
	LITERAL PartnerLit;
	TERM    PartnerAtom;
	CLAUSE  PartnerClause;
	int     pli;

	PartnerLit    = (LITERAL)list_Car(Lits);
	PartnerAtom   = clause_LiteralAtom(PartnerLit);
	pli           = clause_LiteralGetIndex(PartnerLit);
	PartnerClause = clause_LiteralOwningClause(PartnerLit);

	if (!clause_GetFlag(PartnerClause,CLAUSESELECT) &&
	    (!MaxPara || clause_LiteralGetFlag(PartnerLit,STRICTMAXIMAL)) &&
	    !clause_GetFlag(PartnerClause,NOPARAINTO) &&
	    clause_LiteralIsPositive(PartnerLit) &&
	    clause_HasSolvedConstraint(PartnerClause)) {

	  SUBST  Subst, PartnerSubst;
	  TERM   NewLeft,NewRight;
	  SYMBOL PartnerMaxVar;
	  TERM   SupAtom;

	  SupAtom = (TERM)NULL;

	  PartnerMaxVar = clause_MaxVar(PartnerClause);
	  NewLeft       = Left;
	  clause_RenameVarsBiggerThan(Clause, PartnerMaxVar);
	  cont_Check();
	  unify_UnifyNoOC(cont_LeftContext(), Left, cont_RightContext(), Term);
	  subst_ExtractUnifier(cont_LeftContext(), &Subst, cont_RightContext(), &PartnerSubst);
	  cont_Reset();
	  if (!MaxPara ||
	      inf_LiteralsMax(Clause, i, Subst, PartnerClause, pli,
			      PartnerSubst, Flags, Precedence)) {
	    NewRight = subst_Apply(Subst, term_Copy(Right));
	    if (OrdPara &&
		!clause_LiteralIsOrientedEquality(clause_GetLiteral(Clause,i)))
	      NewLeft  = subst_Apply(Subst, term_Copy(Left));
	    if (!OrdPara ||
		      NewLeft == Left ||       /* TRUE, if oriented */
		      ord_Compare(NewLeft,NewRight, Flags, Precedence) != ord_SmallerThan()) {
          
	      if (!MaxPara) {
          SupAtom = inf_AllTermsRplac(PartnerAtom, Term,NewRight, PartnerSubst);
	      } else if (clause_LiteralIsPredicate(PartnerLit)) {
          if (symbol_HasProperty(term_TopSymbol(PartnerAtom), TRANSITIVE)) {
            ord_RESULT Compare;
                        
            Compare = clause_LiteralGetOrderStatus(PartnerLit);

            if (Compare == ord_UNCOMPARABLE) {
              TERM NewPartnerLeft, NewPartnerRight;
                           
              NewPartnerLeft = subst_Apply(PartnerSubst,
                                           term_Copy(term_FirstArgument(PartnerAtom)));
              NewPartnerRight = subst_Apply(PartnerSubst,
                                            term_Copy(term_SecondArgument(PartnerAtom)));

              Compare = ord_Compare(NewPartnerLeft,NewPartnerRight, Flags, Precedence);

              term_Delete(NewPartnerLeft);
              term_Delete(NewPartnerRight);
            }
            
            switch (Compare) {              
              case ord_SMALLER_THAN:
                SupAtom = inf_AllTermsRightRplac(PartnerAtom,Term,NewRight,PartnerSubst);
                break;
              case ord_GREATER_THAN:
                SupAtom = inf_AllTermsLeftRplac(PartnerAtom,Term,NewRight,PartnerSubst);
                break;
              default: /*This way we could merge it with the equality case,
                        but we can also consider replacing only right term if the result is ord_EQUAL ??? */
                SupAtom = inf_AllTermsRplac(PartnerAtom,Term,NewRight,PartnerSubst);
            }

          } else { /*The predicate is not equatily, but also not a special or we don't want to restrict by subterm chaining*/
            SupAtom = inf_AllTermsRplac(PartnerAtom, Term, NewRight, PartnerSubst);
          }        
        } else {        
          /* Superposition and <PartnerLit> is equality */
          if (clause_LiteralIsOrientedEquality(PartnerLit))
            SupAtom = inf_AllTermsLeftRplac(PartnerAtom, Term,NewRight, PartnerSubst);
          else {
            TERM NewPartnerLeft,NewPartnerRight;
            NewPartnerLeft  =
              subst_Apply(PartnerSubst, 
              term_Copy(term_FirstArgument(PartnerAtom)));
            NewPartnerRight =
              subst_Apply(PartnerSubst, 
              term_Copy(term_SecondArgument(PartnerAtom)));
            switch (ord_Compare(NewPartnerLeft,NewPartnerRight,
                    Flags, Precedence)) {
            case ord_SMALLER_THAN:
              SupAtom = inf_AllTermsRightRplac(PartnerAtom,Term,
                        NewRight,PartnerSubst);
              break;
            case ord_GREATER_THAN:
              SupAtom = inf_AllTermsLeftRplac(PartnerAtom,Term,
                       NewRight,PartnerSubst);
              break;
            default:
              SupAtom = inf_AllTermsRplac(PartnerAtom,Term,
                   NewRight,PartnerSubst);
            }
            term_Delete(NewPartnerLeft);
            term_Delete(NewPartnerRight);
          }
	      }

	      if (SupAtom != NULL)
		Result =
		  list_Cons(inf_ApplyGenSuperposition(Clause, i, Subst,
						      PartnerClause, pli,
						      PartnerSubst, SupAtom,
						      TRUE, OrdPara, MaxPara,
						      Flags, Precedence), 
			    Result);
	    }
	    if (NewLeft != Left)
	      term_Delete(NewLeft);
	    term_Delete(NewRight);
	  }
	  subst_Delete(Subst);
	  subst_Delete(PartnerSubst);
	}
      }
    }
  }
  return Result;
}


static LIST inf_GenSPRightEqTransToGiven(CLAUSE Clause, int i, BOOL Left,
				    SHARED_INDEX ShIndex, BOOL OrdPara,
				    BOOL MaxPara, BOOL Unit, FLAGSTORE Flags,
				    PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An unshared clause, the index of a succedent literal
           that is an equality (or transitive) literal 
           and a boolean value which argument to use:
	   If Left==TRUE then the left argument is used
	   otherwise the right argument.
	   Three boolean flags for controlling inference
           preconditions (see inf_GenSuperpositionRight).
	   A flag store.
	   A precedence.
  RETURNS: A list of clauses derivable from general superposition right on the
           GivenCopy  wrt. the Index.
           (see inf_GenSuperpositionRight for selection of inference
	   rule by OrdPara/MaxPara)
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  LIST    Result, TermList, Parents;
  int     Bottom;
  LITERAL Lit;
  TERM    Atom, Term, PartnerTerm, PartnerEq;

  Result = list_Nil();
  Lit    = clause_GetLiteral(Clause,i);
  Atom   = clause_LiteralAtom(Lit);

#ifdef CHECK
  {
    if ((!fol_IsEquality(Atom) &&
            (!symbol_HasProperty(term_TopSymbol(Atom), TRANSITIVE) ||
             ( Left && (clause_LiteralGetOrderStatus(Lit) == ord_SMALLER_THAN)) ||
             (!Left && (clause_LiteralGetOrderStatus(Lit) == ord_GREATER_THAN)))) ||
        (MaxPara && clause_LiteralIsOrientedEquality(Lit) && !Left) ||
        (MaxPara && !clause_LiteralGetFlag(Lit, STRICTMAXIMAL))) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In inf_GenSPRightEqTransToGiven: Illegal input.");
      misc_FinishErrorReport();
    }
  }
#endif

  Bottom = stack_Bottom();
  if (fol_IsEquality(Atom)) {
    if (Left) /* Top Level considered in inf_GenLitSPRight (for equality) */ 
      sharing_PushListOnStack(term_ArgumentList(term_FirstArgument(Atom)));      
    else
      sharing_PushListOnStack(term_ArgumentList(term_SecondArgument(Atom)));      
   } else {
     if (Left) /* For general transitive predicate we have to take Top Level as well */
        sharing_PushOnStack(term_FirstArgument(Atom));
      else
        sharing_PushOnStack(term_SecondArgument(Atom));
   }

  while (!stack_Empty(Bottom)) {
    Term = (TERM)stack_PopResult();
    if (!term_IsVariable(Term)) {
      /* Superposition into variables is not necessary */
      TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
			       cont_RightContext(), Term);
      for ( ; !list_Empty(TermList); TermList = list_Pop(TermList)) {
	PartnerTerm = (TERM)list_Car(TermList);
	for (Parents = term_SupertermList(PartnerTerm);
	     !list_Empty(Parents); Parents = list_Cdr(Parents)) {
	  PartnerEq = (TERM)list_Car(Parents);
	  if (fol_IsEquality(PartnerEq)) {
	    CLAUSE  PartnerClause;
	    LITERAL PartnerLit;
	    LIST    Scl;
	    int     j;
	    for (Scl = sharing_NAtomDataList(PartnerEq);
		 !list_Empty(Scl); Scl = list_Cdr(Scl)) {
	      PartnerLit    = (LITERAL)list_Car(Scl);
	      j             = clause_LiteralGetIndex(PartnerLit);
	      PartnerClause = clause_LiteralOwningClause(PartnerLit);
	      if (!clause_GetFlag(PartnerClause,CLAUSESELECT) &&
		  (!MaxPara ||
		   clause_LiteralGetFlag(PartnerLit,STRICTMAXIMAL)) &&
		  (!OrdPara || PartnerTerm==term_FirstArgument(PartnerEq) ||
		   !clause_LiteralIsOrientedEquality(PartnerLit)) &&
		  clause_LiteralIsPositive(PartnerLit) &&
		  clause_Number(PartnerClause) != clause_Number(Clause) &&
		  (!Unit || clause_Length(PartnerClause) == 1) &&
		  clause_HasSolvedConstraint(PartnerClause)) {
		/* We exclude the same clause since that inference will be */
		/* made by the "forward" function inf_GenLitSPRight.       */
		SYMBOL MaxVar;
		SUBST  Subst, PartnerSubst;

		MaxVar = clause_MaxVar(PartnerClause);
		clause_RenameVarsBiggerThan(Clause, MaxVar);
		cont_Check();
		unify_UnifyNoOC(cont_LeftContext(), Term, cont_RightContext(),
				PartnerTerm);
		subst_ExtractUnifier(cont_LeftContext(), &Subst,
				     cont_RightContext(), &PartnerSubst);
		cont_Reset();
		if (!MaxPara ||
		    inf_LiteralsMax(Clause, i, Subst, PartnerClause, j,
				    PartnerSubst, Flags, Precedence)) {
		  TERM PartnerLeft,PartnerRight;
		  BOOL Check, PartnerCheck;
		  PartnerLeft = PartnerRight = NULL;
		  PartnerCheck = Check = TRUE;
		  if (OrdPara &&
		      !clause_LiteralIsOrientedEquality(PartnerLit)) {
		    /* Check post condition for partner literal */
		    if (PartnerTerm == term_FirstArgument(PartnerEq))
		      PartnerRight = term_SecondArgument(PartnerEq);
		    else
		      PartnerRight = term_FirstArgument(PartnerEq);
		    PartnerLeft  = subst_Apply(PartnerSubst,
					       term_Copy(PartnerTerm));
		    PartnerRight = subst_Apply(PartnerSubst,
					       term_Copy(PartnerRight));
		    PartnerCheck = (ord_Compare(PartnerLeft, PartnerRight,
						Flags, Precedence)
				    != ord_SmallerThan());
		  }
		  if (PartnerCheck &&
		      MaxPara && !clause_LiteralIsOrientedEquality(Lit)) {
		    /* Check post condition for literal in given clause */
		    TERM NewLeft, NewRight;
		    if (Left) {
		      NewLeft  = term_FirstArgument(Atom);
		      NewRight = term_SecondArgument(Atom);
		    } else {
		      NewLeft  = term_SecondArgument(Atom);
		      NewRight = term_FirstArgument(Atom);
		    }
		    NewLeft  = subst_Apply(Subst, term_Copy(NewLeft));
		    NewRight = subst_Apply(Subst, term_Copy(NewRight));
		    Check = (ord_Compare(NewLeft, NewRight, Flags, Precedence)
			     != ord_SmallerThan());
		    term_Delete(NewLeft);
		    term_Delete(NewRight);
		  }
		  if (Check && PartnerCheck) {
		    /* Make inference only if both tests were successful */
		    TERM SupAtom;
		    SupAtom = NULL;
		    if (PartnerRight == NULL) {
		      if (PartnerTerm==term_FirstArgument(PartnerEq))
			PartnerRight = term_SecondArgument(PartnerEq);
		      else
			PartnerRight = term_FirstArgument(PartnerEq);
		      PartnerRight = subst_Apply(PartnerSubst,
						 term_Copy(PartnerRight));
		    }
		    if (Left)
		      SupAtom = inf_AllTermsLeftRplac(Atom, Term,
						      PartnerRight, Subst);
		    else 
		      SupAtom = inf_AllTermsRightRplac(Atom, Term,
						       PartnerRight, Subst);
#ifdef CHECK
		    if (SupAtom == NULL) {
		      misc_StartErrorReport();
		      misc_ErrorReport("\n In inf_GenSPRightEqTransToGiven:");
		      misc_ErrorReport(" replacement wasn't possible.");
		      misc_FinishErrorReport();
		    }
#endif
		    Result =
		      list_Cons(inf_ApplyGenSuperposition(PartnerClause, j,
							  PartnerSubst, Clause,
							  i, Subst, SupAtom,
							  TRUE,OrdPara,MaxPara,
							  Flags, Precedence), 
				Result);
		  }
		  if (PartnerLeft != term_Null())
		    term_Delete(PartnerLeft);
		  if (PartnerRight != term_Null())
		    term_Delete(PartnerRight);
		}
		subst_Delete(Subst);
		subst_Delete(PartnerSubst);
	      }
	    }
	  }
	}
      }
    }
  }
  return Result;
}


static LIST inf_GenSPRightLitToGiven(CLAUSE Clause, int i, TERM Atom,
				     SHARED_INDEX ShIndex, BOOL OrdPara,
				     BOOL MaxPara, BOOL Unit, FLAGSTORE Flags,
				     PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An unshared clause, the index of a succedent literal
           that is not an equality literal and its atom,
	   three boolean flags for controlling inference
           preconditions (see inf_GenSuperpositionRight),
	   a flag store and a precedence.
  RETURNS: A list of clauses derivable from general superposition right on the
           GivenCopy  wrt. the Index.
           (see inf_GenSuperpositionRight for selection of inference
	   rule by OrdPara/MaxPara)
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  LIST    Result, TermList, ParentList;
  int     Bottom;
  TERM    Term, PartnerTerm, PartnerEq;

  Result = list_Nil();

  Bottom = stack_Bottom();
  sharing_PushListOnStack(term_ArgumentList(Atom));

  while (!stack_Empty(Bottom)) {
    Term = (TERM)stack_PopResult();
    if (!term_IsVariable(Term)) {
      /* Superposition into variables is not necessary */
      TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
			       cont_RightContext(), Term);
      for ( ; !list_Empty(TermList); TermList=list_Pop(TermList)) {
	PartnerTerm = (TERM)list_Car(TermList);
	for (ParentList = term_SupertermList(PartnerTerm);
	     !list_Empty(ParentList); ParentList = list_Cdr(ParentList)) {
	  PartnerEq = (TERM)list_Car(ParentList);
	  if (fol_IsEquality(PartnerEq)) {
	    CLAUSE  PartnerClause;
	    LITERAL PartnerLit;
	    LIST    Scl;
	    int     j;
	    for (Scl = sharing_NAtomDataList(PartnerEq);
		 !list_Empty(Scl); Scl = list_Cdr(Scl)) {
	      PartnerLit    = (LITERAL)list_Car(Scl);
	      j             = clause_LiteralGetIndex(PartnerLit);
	      PartnerClause = clause_LiteralOwningClause(PartnerLit);
	      if (!clause_GetFlag(PartnerClause,CLAUSESELECT) &&
		  (!MaxPara ||
		   clause_LiteralGetFlag(PartnerLit,STRICTMAXIMAL)) &&
		  (!OrdPara || PartnerTerm==term_FirstArgument(PartnerEq) ||
		   !clause_LiteralIsOrientedEquality(PartnerLit)) &&
		  clause_LiteralIsPositive(PartnerLit) &&
		  clause_Number(PartnerClause) != clause_Number(Clause) && 
		  (!Unit || clause_Length(PartnerClause) == 1) &&
		  clause_HasSolvedConstraint(PartnerClause)) {
		/* We exclude the same clause since that inference will be */
		/* made by the "forward" function inf_GenLitSPRight.       */
		SYMBOL MaxVar;
		TERM   PartnerLeft,PartnerRight;
		SUBST  Subst, PartnerSubst;
		TERM   SupAtom;

		SupAtom = (TERM)NULL;
		MaxVar      = clause_MaxVar(PartnerClause);
		clause_RenameVarsBiggerThan(Clause,MaxVar);
		cont_Check();
		unify_UnifyNoOC(cont_LeftContext(), Term,
				cont_RightContext(),PartnerTerm);
		subst_ExtractUnifier(cont_LeftContext(), &Subst,
				     cont_RightContext(),&PartnerSubst);
		cont_Reset();

		if (!MaxPara ||
		    inf_LiteralsMax(Clause, i, Subst, PartnerClause, j,
				    PartnerSubst, Flags, Precedence)) {
		  PartnerLeft = subst_Apply(PartnerSubst,
					    term_Copy(PartnerTerm));
		  if (PartnerTerm == term_FirstArgument(PartnerEq))
		    PartnerRight =
		      subst_Apply(PartnerSubst,
				  term_Copy(term_SecondArgument(PartnerEq)));
		  else
		    PartnerRight =
		      subst_Apply(PartnerSubst,
				  term_Copy(term_FirstArgument(PartnerEq)));

		  if (!OrdPara ||
		      clause_LiteralIsOrientedEquality(PartnerLit) ||
		      ord_Compare(PartnerLeft,PartnerRight, Flags, Precedence)
		      != ord_SmallerThan()) {
		    SupAtom = inf_AllTermsRplac(Atom,Term,PartnerRight,Subst);
#ifdef CHECK
		    if (SupAtom == NULL) {
		      misc_StartErrorReport();
		      misc_ErrorReport("\n In inf_GenSPRightLitToGiven:");
		      misc_ErrorReport(" replacement wasn't possible.");
		      misc_FinishErrorReport();
		    }
#endif
		    Result =
		      list_Cons(inf_ApplyGenSuperposition(PartnerClause, j,
							  PartnerSubst, Clause,
							  i, Subst, SupAtom,
							  TRUE,OrdPara,MaxPara,
							  Flags, Precedence),
				Result);
		  
		  }
		  term_Delete(PartnerLeft);
		  term_Delete(PartnerRight);
		}
		subst_Delete(Subst);
		subst_Delete(PartnerSubst);
	      }
	    }
	  }
	}
      }
    }
  }
  return Result;
}


static LIST inf_GenSPRightToGiven(CLAUSE Clause, int i, SHARED_INDEX ShIndex, 
				  BOOL OrdPara, BOOL MaxPara, BOOL Unit,
				  FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An unshared clause, the index of a succedent literal
           and an index of shared clauses,
           three boolean flags for controlling inference
           preconditions (see inf_GenSuperpositionRight),
	   a flag store and a precedence.
  RETURNS: A list of clauses derivable from superposition right on the
           GivenCopy  wrt. the Index.
	   (see inf_GenSuperpositionRight for selection of inference
	   rule by OrdPara/MaxPara)
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  TERM Atom;
  LIST Result;
  
#ifdef CHECK
  if (clause_GetFlag(Clause, NOPARAINTO) ||
      clause_GetFlag(Clause, CLAUSESELECT) ||
      (MaxPara &&
       !clause_LiteralGetFlag(clause_GetLiteral(Clause,i), STRICTMAXIMAL))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GenSPRightToGiven: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  Result  = list_Nil();
  Atom    = clause_LiteralAtom(clause_GetLiteral(Clause,i));
  
  if (fol_IsEquality(Atom)) {
    Result = list_Nconc(inf_GenSPRightEqTransToGiven(Clause,i,TRUE,ShIndex,OrdPara,
						     MaxPara,Unit,Flags, Precedence),
			Result);
    
    if (!MaxPara || (fol_IsEquality(Atom) && !clause_LiteralIsOrientedEquality(clause_GetLiteral(Clause,i))))                
      /* For SPm and OPm always try other direction, for SpR try it */
      /* only if the literal is not oriented.                       */
      Result = list_Nconc(inf_GenSPRightEqTransToGiven(Clause, i, FALSE, ShIndex,
						       OrdPara,MaxPara,Unit,
						       Flags, Precedence),
			  Result);
  } 
  else 
    if (symbol_HasProperty(term_TopSymbol(Atom), TRANSITIVE)) {
      switch(clause_LiteralGetOrderStatus(clause_GetLiteral(Clause,i))) {      
      case ord_SMALLER_THAN:
        Result = list_Nconc(inf_GenSPRightEqTransToGiven(Clause, i, FALSE, ShIndex,
							 OrdPara,MaxPara,Unit,Flags, Precedence),Result);
        break;
      case ord_GREATER_THAN:
        Result = list_Nconc(inf_GenSPRightEqTransToGiven(Clause,i,TRUE,ShIndex,
							 OrdPara,MaxPara,Unit,Flags, Precedence),Result);
        break;
      default: /* Replacing only into the right term if the status is ord_EQUAL ??? */
        Result = list_Nconc(inf_GenSPRightLitToGiven(Clause,i,Atom,ShIndex,
						     OrdPara,MaxPara,Unit,Flags, Precedence),Result);
      }             
    } else
      Result = list_Nconc(inf_GenSPRightLitToGiven(Clause,i,Atom,ShIndex,
						   OrdPara,MaxPara,Unit,
						   Flags, Precedence),
			  Result);
  
  return Result;
}


LIST inf_GenSuperpositionRight(CLAUSE GivenClause, SHARED_INDEX ShIndex,
			       BOOL OrdPara, BOOL MaxPara, BOOL Unit,
			       FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause and an Index, usually the WorkedOffIndex,
           three boolean flags for controlling inference
           preconditions, a flag store and a precedence.
  RETURNS: A list of clauses derivable from the given clause by 
           superposition right wrt. the Index.

	   OrdPara=TRUE, MaxPara=TRUE 
	   -> Superposition Right

	   OrdPara=TRUE, MaxPara=FALSE
	   -> ordered Paramodulation

	   OrdPara=FALSE, MaxPara=FALSE
	   -> simple Paramodulation

	   OrdPara=FALSE, MaxPara=TRUE
	   -> not defined

	   If <Unit>==TRUE the clause with the maximal equality 
	   additionally must be a unit clause.
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  LIST    Result;
  TERM    Atom;
  CLAUSE  Copy;
  int     i, n;
  LITERAL ActLit;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GenSuperpositionRight: Illegal input.");
    misc_FinishErrorReport();
  }
  if (!OrdPara && MaxPara) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GenSuperpositionRight: Illegal inference");
    misc_ErrorReport("\n rule selection, OrdPara=FALSE & MaxPara=TRUE.");
    misc_FinishErrorReport();
  }
#endif

  if (clause_GetFlag(GivenClause,CLAUSESELECT) ||
      clause_HasEmptySuccedent(GivenClause) ||
      !clause_HasSolvedConstraint(GivenClause))
    return list_Nil();

  Result = list_Nil();

  Copy = clause_Copy(GivenClause);
  n    = clause_LastSuccedentLitIndex(Copy);
  
  for (i = clause_FirstSuccedentLitIndex(Copy); i <= n; i++) {
    
    ActLit = clause_GetLiteral(Copy, i);
    Atom   = clause_LiteralSignedAtom(ActLit);
    
    if (!MaxPara ||
	clause_LiteralGetFlag(ActLit,STRICTMAXIMAL)) {
      if (fol_IsEquality(Atom) &&
	  (!Unit || clause_Length(GivenClause) == 1)) {
	
	Result = list_Nconc(inf_GenLitSPRight(Copy, term_FirstArgument(Atom), 
					      term_SecondArgument(Atom), i,
					      ShIndex,OrdPara,MaxPara,Flags,
					      Precedence), 
			    Result);
	
	if (!OrdPara ||
	    !clause_LiteralIsOrientedEquality(ActLit))
	  Result = list_Nconc(inf_GenLitSPRight(Copy,
						term_SecondArgument(Atom), 
						term_FirstArgument(Atom), i,
						ShIndex,OrdPara,MaxPara,Flags,
						Precedence),
			      Result);
      }
      if (!clause_GetFlag(Copy, NOPARAINTO))
	Result = list_Nconc(inf_GenSPRightToGiven(Copy, i, ShIndex, OrdPara,
						  MaxPara,Unit,Flags,Precedence),
			    Result);
    }
  }
  clause_Delete(Copy);

  return Result;
}



/* END of block with new superposition right rule */


static LIST inf_ApplyMParamod(CLAUSE C1, CLAUSE C2, int i, int j, int k,
			      TERM u_tau, TERM v, TERM s2, TERM t,
			      TERM v2_sigma, SUBST tau, SUBST rho,
			      FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   Two clauses, <i> is a literal index in <C1>, <j> and <k>
           are literal indices in <C2>, <u_tau> with <rho> applied
	   is used as left side of the two new literals, <v> with
	   subterm <s2> replaced by <t> is used as right side of
	   the first new literal, <v2_sigma> as right side of the
	   second new literal, two substitutions and a flag store.
	   The substitution <rho> is applied after <tau>.
	   A flag store.
	   A precedence.
  RETURNS: A list containing one clause derived from the given
           clauses and literals by merging paramodulation.
  MEMORY:  Memory for the list and the clause is allocated.
***************************************************************/
{
  CLAUSE newClause;
  TERM   u_sigma;
  int    m, lc, la, ls, pls, pla, plc, help;

  pls = clause_LastSuccedentLitIndex(C2);
  pla = clause_LastAntecedentLitIndex(C2);
  plc = clause_LastConstraintLitIndex(C2);

  ls  = clause_LastSuccedentLitIndex(C1);
  la  = clause_LastAntecedentLitIndex(C1);
  lc  = clause_LastConstraintLitIndex(C1);

  newClause = clause_CreateBody(clause_Length(C1) + clause_Length(C2) - 1);

  clause_SetNumOfConsLits(newClause, (clause_NumOfConsLits(C1) +
				      clause_NumOfConsLits(C2)));
  clause_SetNumOfAnteLits(newClause, (clause_NumOfAnteLits(C1) +
				      clause_NumOfAnteLits(C2)));
  clause_SetNumOfSuccLits(newClause, (clause_NumOfSuccLits(C1) - 1 +
				      clause_NumOfSuccLits(C2)));

  for (m = clause_FirstLitIndex(); m <= lc; m++)
    clause_SetLiteral(newClause, m,
      clause_LiteralCreate(subst_Apply(rho, subst_Apply(tau,
	term_Copy(clause_GetLiteralTerm(C1, m)))), newClause));

  /* help = number of literals to leave empty */
  help = clause_NumOfConsLits(C2);

  for ( ; m <= la; m++)
    clause_SetLiteral(newClause, (m + help),
      clause_LiteralCreate(subst_Apply(rho, subst_Apply(tau,
	term_Copy(clause_GetLiteralTerm(C1, m)))), newClause));

  /* help = number of literals to leave empty */
  help += clause_NumOfAnteLits(C2);

  for ( ; m <= ls; m++) {
    if (m != i)
      /* The literal used in the inference isn't copied */
      clause_SetLiteral(newClause, (m + help), 
	clause_LiteralCreate(subst_Apply(rho, subst_Apply(tau,
	  term_Copy(clause_GetLiteralTerm(C1, m)))),newClause));
    else
      /* the index has to be decreased to avoid an empty literal! */
      help--;
  }

  /* Now we consider the PartnerClause : */
  
  /* help = number of already set constraint (Clause) literals */
  help = clause_NumOfConsLits(C1);
  
  for (m = clause_FirstLitIndex(); m <= plc; m++)
    clause_SetLiteral(newClause, (m + help), 
      clause_LiteralCreate(subst_Apply(rho, subst_Apply(tau,
	term_Copy(clause_GetLiteralTerm(C2, m)))),newClause));
  
  /* help = number of already set constraint and antecedent Given-literals */
  help += clause_NumOfAnteLits(C1);
  
  for ( ; m <= pla; m++)
    clause_SetLiteral(newClause, (m + help), 
      clause_LiteralCreate(subst_Apply(rho, subst_Apply(tau,
        term_Copy(clause_GetLiteralTerm(C2, m)))),newClause));

  /* help = number of already set literals */
  help += clause_NumOfSuccLits(C1) - 1;
  /*help = clause_Length(Clause) - 1;*/

  u_sigma = subst_Apply(rho, term_Copy(u_tau));

  for ( ; m <= pls; m++) {
    TERM newAtom;

    if (m == j) {
      /* The first partner literal is modified appropriately! */
      TERM right;
      if (v == s2)
	/* Necessary because term_ReplaceSubtermBy doesn't treat top level */
	right = term_Copy(t);
      else {
	right = term_Copy(v);
	term_ReplaceSubtermBy(right, s2, t);
      }		
      newAtom = term_Create(fol_Equality(), list_Cons(term_Copy(u_sigma),
		  list_List(subst_Apply(rho, subst_Apply(tau, right)))));
    } else if (m == k) {
      /* The second partner lit is modified appropriately!    */
      newAtom = term_Create(fol_Equality(), list_Cons(term_Copy(u_sigma),
	          list_List(term_Copy(v2_sigma))));
    } else {
      /* Apply substitutions to all other literals */
      newAtom = subst_Apply(rho, subst_Apply(tau,
                  term_Copy(clause_GetLiteralTerm(C2, m))));
    }

    clause_SetLiteral(newClause, (m + help),
		      clause_LiteralCreate(newAtom, newClause));
   }

  term_Delete(u_sigma);

  clause_SetFromMergingParamodulation(newClause);

  clause_AddParentClause(newClause, clause_Number(C2));
  clause_AddParentLiteral(newClause, k);
  clause_SetDataFromParents(newClause, C2, j, C1, k, Flags, Precedence);

  return list_List(newClause);
}


static LIST inf_Lit2MParamod(CLAUSE C1, CLAUSE C2, int i, int j, TERM s, TERM t,
			     TERM s2, TERM v, TERM u_tau, SUBST tau,
			     FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   Two clauses, the index <i> of a strict maximal literal
           in <C1>, the index <j> of a strict maximal index in <C2>,
	   <s> and <t> are from literal <i>, <s2> is a subterm
	   of <v>, <v> is from literal <j>, <u_tau> is <u> with
	   substitution <tau> applied, a flag store, and a precedence.
	   <u_tau> must be greater than <v>tau wrt. the ordering.
  RETURNS: A list of clauses derivable from the given data
           by merging paramodulation.
	   This function searches <C2> for a second positive literal
	   u'=v', where u' is unifiable with <u_tau>.
  MEMORY:  Memory is allocated for the list and the clauses.
***************************************************************/
{
  LIST result;
  int  k, last;

  result = list_Nil();

  /* Now find the 3rd literal u' = v' in C2 */
  last = clause_LastSuccedentLitIndex(C2);  /* Last index */
  for (k = clause_FirstSuccedentLitIndex(C2); k <= last; k++) {
    LITERAL partnerLit2  = clause_GetLiteral(C2, k);
    TERM    partnerAtom2 = clause_LiteralSignedAtom(partnerLit2);

    if (k != j && fol_IsEquality(partnerAtom2)) {
      /* partnerLit2: u' = v'  or  v' = u' */
      SUBST      rho;
      TERM       pLeft2, pRight2, s_sigma, t_sigma, v2_sigma;
      ord_RESULT ordResult;
      BOOL       checkPassed;

      pLeft2  = subst_Apply(tau, term_Copy(term_FirstArgument(partnerAtom2)));
      pRight2 = subst_Apply(tau, term_Copy(term_SecondArgument(partnerAtom2)));
      
      /* First try unification with left side */
      cont_Check();

      if (unify_UnifyCom(cont_LeftContext(), u_tau, pLeft2)) {
	subst_ExtractUnifierCom(cont_LeftContext(), &rho);

	s_sigma = t_sigma = (TERM) NULL;
	checkPassed = TRUE;

	if (!clause_LiteralIsOrientedEquality(clause_GetLiteral(C1,i))) {
	  s_sigma = subst_Apply(rho, subst_Apply(tau, term_Copy(s)));
	  t_sigma = subst_Apply(rho, subst_Apply(tau, term_Copy(t)));
	  ordResult = ord_Compare(s_sigma, t_sigma, Flags, Precedence);
	  if (ordResult == ord_SmallerThan() || ordResult == ord_Equal())
	    checkPassed = FALSE;
	}

	if (checkPassed && inf_LiteralsMaxWith2Subst(C1,i,C2,j,rho,tau,
						     Flags, Precedence)) {
	  v2_sigma = subst_Apply(rho, term_Copy(pRight2));
	  result = list_Nconc(inf_ApplyMParamod(C1,C2,i,j,k,u_tau,v,s2,
						t,v2_sigma,tau,rho,
						Flags, Precedence),
			      result);
	  term_Delete(v2_sigma);
	}
	/* Now cleanup */
	if (s_sigma != NULL) {      /* Also t_sigma != NULL */
	  term_Delete(s_sigma);
	  term_Delete(t_sigma);
	}
	subst_Delete(rho);
      }

      cont_Reset();

      /* Now try unification with right side */
      if (unify_UnifyCom(cont_LeftContext(), u_tau, pRight2)) {
	subst_ExtractUnifierCom(cont_LeftContext(), &rho);

	s_sigma = t_sigma = (TERM) NULL;
	checkPassed = TRUE;

	if (!clause_LiteralIsOrientedEquality(clause_GetLiteral(C1,i))) {
	  s_sigma = subst_Apply(rho, subst_Apply(tau, term_Copy(s)));
	  t_sigma = subst_Apply(rho, subst_Apply(tau, term_Copy(t)));
	  ordResult = ord_Compare(s_sigma, t_sigma, Flags, Precedence);
	  if (ordResult == ord_SmallerThan() || ordResult == ord_Equal())
	    checkPassed = FALSE;
	}

	if (checkPassed && inf_LiteralsMaxWith2Subst(C1,i,C2,j,rho,tau,
						     Flags, Precedence)) {
	  v2_sigma = subst_Apply(rho, term_Copy(pLeft2));
	  result = list_Nconc(inf_ApplyMParamod(C1,C2,i,j,k,u_tau,v,s2,
						t,v2_sigma,tau,rho,
						Flags, Precedence),
			      result);
	  term_Delete(v2_sigma);
	}
	/* Now cleanup */
	if (s_sigma != NULL) {      /* Also t_sigma != NULL */
	  term_Delete(s_sigma);
	  term_Delete(t_sigma);
	}
	subst_Delete(rho);
      }

      cont_Reset();

      term_Delete(pLeft2);
      term_Delete(pRight2);
    }   /* k != j */
  }     /* for k */
  
  return result;
}  
  

static LIST inf_LitMParamod(CLAUSE Clause, int i, BOOL Turn,
			    SHARED_INDEX ShIndex, FLAGSTORE Flags,
			    PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause with a strict maximal equality literal at 
           position <i>, a boolean value, a shared index, a 
	   flag store and a precedence.
	   If <Turn> is TRUE, the left and right term of the
	   equality are exchanged.
  RETURNS: A list of clauses derivable from the given clause
           by merging paramodulation.
	   This function searches a second clause with at least
	   two positive equalities, where the first equation
	   has a subterm s', that is unifiable with the left
	   (or right) side of the given equation.
***************************************************************/
{
  LIST    result, unifiers, literals;
  LITERAL actLit;
  TERM    s, t, help;

  actLit = clause_GetLiteral(Clause, i);
  s      = term_FirstArgument(clause_LiteralSignedAtom(actLit));
  t      = term_SecondArgument(clause_LiteralSignedAtom(actLit));
  if (Turn) {
    /* Exchange s and t */
    help = s;
    s    = t;
    t    = help;
  }
  result = list_Nil();

  unifiers = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
			   cont_RightContext(), s);
  
  for ( ; !list_Empty(unifiers); unifiers = list_Pop(unifiers)) {
    TERM s2 = (TERM) list_Car(unifiers);     /* Unifiable with s */

    if (!term_IsVariable(s2) && !term_IsAtom(s2)) {
      for (literals = sharing_GetDataList(s2, ShIndex);
	   !list_Empty(literals);
	   literals = list_Pop(literals)) {
	LITERAL partnerLit    = (LITERAL) list_Car(literals);  /* u = v[s'] */
	CLAUSE  partnerClause = clause_LiteralOwningClause(partnerLit);
	TERM    partnerAtom   = clause_LiteralAtom(partnerLit);
	int     pli           = clause_LiteralGetIndex(partnerLit);

	if (!clause_GetFlag(partnerClause, CLAUSESELECT) &&
	    clause_LiteralGetFlag(partnerLit, STRICTMAXIMAL) &&
	    clause_LiteralIsPositive(partnerLit) &&   /* succedent literal */
	    clause_LiteralIsEquality(partnerLit) &&
	    clause_NumOfSuccLits(partnerClause) > 1 && /* > 1 pos. literals */
	    clause_HasSolvedConstraint(partnerClause)) {
	  TERM partnerLeft    = term_FirstArgument(partnerAtom);
	  TERM partnerRight   = term_SecondArgument(partnerAtom);
	  BOOL inPartnerRight = term_HasPointerSubterm(partnerRight, s2);

	  if (!clause_LiteralIsOrientedEquality(partnerLit) || inPartnerRight){
	    /* Don't do this if u=v is oriented and s2 is not a subterm of v */
	    TERM       newPLeft, newPRight;
	    SUBST      tau;
	    SYMBOL     partnerMaxVar;
	    ord_RESULT ordResult;

	    partnerMaxVar = clause_MaxVar(partnerClause);
	    clause_RenameVarsBiggerThan(Clause, partnerMaxVar);
	    cont_Check();
	    unify_UnifyNoOC(cont_LeftContext(), s, cont_RightContext(), s2);
	    subst_ExtractUnifierCom(cont_LeftContext(), &tau);
	    cont_Reset();

	    newPLeft  = subst_Apply(tau, term_Copy(partnerLeft));
	    newPRight = subst_Apply(tau, term_Copy(partnerRight));
	    if (clause_LiteralIsOrientedEquality(partnerLit))
	      ordResult = ord_GreaterThan();
	    else
	      ordResult = ord_Compare(newPLeft, newPRight, Flags, Precedence);

	    if (inPartnerRight && ord_IsGreaterThan(ordResult)) {
	      /* Take a look at right side */
	      result = list_Nconc(inf_Lit2MParamod(Clause,partnerClause,i,pli,
						   s, t,s2,partnerRight,newPLeft,
						   tau, Flags, Precedence),
				  result);
	    }
	    if (ord_IsSmallerThan(ordResult) &&
		(!inPartnerRight || term_HasPointerSubterm(partnerLeft, s2))) {
	      /* If s2 is not in partnerRight, it MUST be in partnerLeft,
		 else really do the test */
	      /* Take a look at left side */
	      result = list_Nconc(inf_Lit2MParamod(Clause,partnerClause,i,pli,
						   s,t,s2,partnerLeft,newPRight,
						   tau,Flags, Precedence),
				  result);
	    }

	    term_Delete(newPLeft);
	    term_Delete(newPRight);
	    subst_Delete(tau);
	  }
	}
      }  /* for all Literals containing s2 */
    }    /* if s2 isn't a variable */
  }      /* for all unifiers s2 */

  return result;
}


static LIST inf_MParamodLitToGiven(CLAUSE Clause, int j, BOOL Turn,
				   SHARED_INDEX ShIndex, FLAGSTORE Flags,
				   PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause with a strict maximal equality literal at
           position <j>, a boolean value, a shared index a 
	   flag store and a precedence.
	   If <Turn> is TRUE, the left and right term of the
	   equality are exchanged.
  RETURNS: A list of clauses derivable from the given clause
           by merging paramodulation with this literal as second
	   literal of the rule.
***************************************************************/
{
  LIST    result, unifiers, superterms, literals;
  LITERAL actLit;
  TERM    u, v;
  int     bottom;

  if (clause_NumOfSuccLits(Clause) < 2)
    return list_Nil();  /* There must be at least two positive literals */

  actLit = clause_GetLiteral(Clause, j);
  u  = term_FirstArgument(clause_LiteralSignedAtom(actLit));
  v = term_SecondArgument(clause_LiteralSignedAtom(actLit));
  if (Turn) {
    /* Exchange s and t */
    TERM help = u;
    u  = v;
    v = help;
  }
  result = list_Nil();
  bottom = stack_Bottom();

  sharing_PushReverseOnStack(v);  /* Without variables! */

  while (!stack_Empty(bottom)) {
    TERM s2 = (TERM) stack_PopResult();

    for (unifiers = st_GetUnifier(cont_LeftContext(),sharing_Index(ShIndex),
				  cont_RightContext(), s2);
	 !list_Empty(unifiers);
	 unifiers = list_Pop(unifiers)) {
      TERM s = (TERM) list_Car(unifiers);

      for (superterms = term_SupertermList(s);
	   !list_Empty(superterms);
	   superterms = list_Cdr(superterms)) {
	TERM partnerAtom = (TERM) list_Car(superterms);

	if (fol_IsEquality(partnerAtom)) {
	  for (literals = sharing_NAtomDataList(partnerAtom);
	       !list_Empty(literals);
	       literals = list_Cdr(literals)) {
	    LITERAL partnerLit    = (LITERAL) list_Car(literals);
	    CLAUSE  partnerClause = clause_LiteralOwningClause(partnerLit);
	    int     i             = clause_LiteralGetIndex(partnerLit);

	    if (!clause_GetFlag(partnerClause,CLAUSESELECT) &&
		clause_LiteralGetFlag(partnerLit,STRICTMAXIMAL) &&
		clause_LiteralIsPositive(partnerLit) &&
		(s == term_FirstArgument(partnerAtom) ||
		 !clause_LiteralIsOrientedEquality(partnerLit)) &&
		clause_HasSolvedConstraint(partnerClause) &&
		clause_Number(partnerClause) != clause_Number(Clause)) {
	      /* We don't allow self inferences (both clauses having the     */
	      /* same number) here, because they're already made in function */
	      /* inf_LitMParamod.                                            */
	      SYMBOL partnerMaxVar;
	      SUBST  tau;
	      TERM   u_tau, v_tau;
	      BOOL   checkPassed;

	      partnerMaxVar = clause_MaxVar(partnerClause);
	      clause_RenameVarsBiggerThan(Clause, partnerMaxVar);
	      cont_Check();
	      unify_UnifyNoOC(cont_LeftContext(), s, cont_RightContext(), s2);
	      subst_ExtractUnifierCom(cont_LeftContext(), &tau);
	      cont_Reset();

	      u_tau = v_tau = (TERM) NULL;
	      checkPassed = TRUE;

	      /* u_tau must be greater than v_tau */
	      if (!clause_LiteralIsOrientedEquality(actLit)) {
		u_tau = subst_Apply(tau, term_Copy(u));
		v_tau = subst_Apply(tau, term_Copy(v));
		if (ord_Compare(u_tau, v_tau, Flags, Precedence) != ord_GreaterThan())
		  checkPassed = FALSE;
	      }

	      if (checkPassed) {
		/* u_tau > v_tau */
		TERM t;

		if (s == term_FirstArgument(partnerAtom))
		  t = term_SecondArgument(partnerAtom);
		else
		  t = term_FirstArgument(partnerAtom);
		if (u_tau == (TERM)NULL) {
		  u_tau = subst_Apply(tau, term_Copy(u));
		  v_tau = subst_Apply(tau, term_Copy(v));
		}

		result = list_Nconc(inf_Lit2MParamod(partnerClause,Clause,i,j,
						     s,t,s2,v,u_tau, tau,Flags,
						     Precedence),
				    result);
	      }

	      /* Now cleanup */
	      if (u_tau != (TERM)NULL) {
		term_Delete(u_tau);
		term_Delete(v_tau);
	      }
	      subst_Delete(tau);
	      clause_Normalize(Clause);
	    }
	  }
	}   /* partnerAtom is equality */
      }
    }
  }

  return result;
}


LIST inf_MergingParamodulation(CLAUSE GivenClause, SHARED_INDEX ShIndex,
			       FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a shared index, a flag store and a 
           precedence.
  RETURNS: A list of clauses derivable from the given clause
           by merging paramodulation.
  MEMORY:  Memory is allocated for the list and the clauses.
***************************************************************/
{
  LIST   result;
  CLAUSE copy;
  int    last, i;
  
#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_MergingParamodulation: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  if (clause_GetFlag(GivenClause, CLAUSESELECT) ||
      clause_HasEmptySuccedent(GivenClause) ||
      !clause_HasSolvedConstraint(GivenClause))
    return list_Nil();

  result = list_Nil();
  copy   = clause_Copy(GivenClause);
  last   = clause_LastSuccedentLitIndex(copy);

  for (i = clause_FirstSuccedentLitIndex(copy); i <= last; i++) {
    LITERAL actLit = clause_GetLiteral(copy, i);
    TERM    atom   = clause_LiteralSignedAtom(actLit);
    
    if (clause_LiteralGetFlag(actLit, STRICTMAXIMAL) &&
	fol_IsEquality(atom)) {
      
      result = list_Nconc(inf_LitMParamod(copy,i,FALSE,ShIndex,
					  Flags, Precedence), 
			  result);
      /* Assume GivenClause is the second clause of the rule */
      result = list_Nconc(inf_MParamodLitToGiven(copy,i,FALSE,ShIndex,
						 Flags, Precedence),
			  result);
      
      if (!clause_LiteralIsOrientedEquality(actLit)) {
	/* First check rule with left and right side exchanged */
	result = list_Nconc(inf_LitMParamod(copy, i, TRUE, ShIndex, 
					    Flags, Precedence),
			    result);
	/* Now assume GivenClause is the second clause of the rule */
	/* Check with sides exchanged */
	result = list_Nconc(inf_MParamodLitToGiven(copy,i,TRUE,ShIndex,
						   Flags, Precedence),
			    result);
      }
    }
  }  /* for */
  clause_Delete(copy);

  return result;
}


static CLAUSE inf_ApplyGenRes(LITERAL PosLit, LITERAL NegLit, SUBST SubstTermS,
			      SUBST SubstPartnerTermS, FLAGSTORE Flags, 
			      PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause to use for Resolution, the index of a 
           positive non-equality literal, a unifiable literal,
           the substitutions for the terms to unify, a flag 
	   store and a precedence.
  RETURNS: A clause derivable from the literals owning
           clause by Resolution wrt. the Index.
  MEMORY:  Memory for the new clause is allocated.
***************************************************************/
{
  CLAUSE NewClause, GivenClause, PartnerClause;
  int    i,j,lc,la,ls,pi,pls,pla,plc,help,ConNeg,AntNeg; /* p=Partner,l=last */

  PartnerClause = clause_LiteralOwningClause(NegLit);
  GivenClause   = clause_LiteralOwningClause(PosLit);

  pls = clause_LastSuccedentLitIndex(PartnerClause);
  pla = clause_LastAntecedentLitIndex(PartnerClause);
  plc = clause_LastConstraintLitIndex(PartnerClause);

  pi  = clause_LiteralGetIndex(NegLit);

  ls  = clause_LastSuccedentLitIndex(GivenClause);
  la  = clause_LastAntecedentLitIndex(GivenClause);
  lc  = clause_LastConstraintLitIndex(GivenClause);

  i   = clause_LiteralGetIndex(PosLit);

  if (pi <= plc) {
    ConNeg = 1;
    AntNeg = 0;
  }
  else {
    ConNeg = 0;
    AntNeg = 1;
  }

  NewClause = clause_CreateBody((clause_Length(GivenClause) -1) +
				clause_Length(PartnerClause) -1);

  clause_SetNumOfConsLits(NewClause,
			    (clause_NumOfConsLits(GivenClause) +
			     (clause_NumOfConsLits(PartnerClause)-ConNeg)));
  
  clause_SetNumOfAnteLits(NewClause,
			    (clause_NumOfAnteLits(GivenClause) +
			     (clause_NumOfAnteLits(PartnerClause)-AntNeg)));
  
  clause_SetNumOfSuccLits(NewClause,
			  ((clause_NumOfSuccLits(GivenClause) -1)+
			   clause_NumOfSuccLits(PartnerClause)));


  /* First set the literals from the GivenClause : */
  for (j = clause_FirstLitIndex(); j <= lc; j++) {
    clause_SetLiteral(NewClause, j, 
      clause_LiteralCreate(subst_Apply(SubstTermS,
	term_Copy(clause_GetLiteralTerm(GivenClause, j))),NewClause));
  }

  /* help = number of literals to leave empty */
  help = clause_NumOfConsLits(PartnerClause)-ConNeg;
    
  for ( ; j <= la; j++) {
    clause_SetLiteral(NewClause, (j + help), 
      clause_LiteralCreate(subst_Apply(SubstTermS,
	term_Copy(clause_GetLiteralTerm(GivenClause, j))),NewClause));
  }

  /* help = number of literals to leave empty */
  help += clause_NumOfAnteLits(PartnerClause)-AntNeg;
    
  

  for ( ; j <= ls; j++) {
    if (j != i) {
      /* The ActLit isn't copied! */
      clause_SetLiteral(NewClause, (j + help), 
	clause_LiteralCreate(subst_Apply(SubstTermS,
	  term_Copy(clause_GetLiteralTerm(GivenClause, j))),NewClause));

    } else {
      /*the index has to be decreased to avoid an empty literal! */
      help--;
    }
  }

  /* Now we consider the PartnerClause : */

  /* help = number of already set constraint (GivenClause-) literals */
  help = clause_NumOfConsLits(GivenClause);

  for (j = clause_FirstLitIndex(); j <= plc; j++) {
    if (j != pi) {
    clause_SetLiteral(NewClause, (j + help), 
      clause_LiteralCreate(subst_Apply(SubstPartnerTermS,
	term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));
    } else {
      help--;
    }
  }

  /* help = number of already set constraint and antecedent Given-literals */
  help += clause_NumOfAnteLits(GivenClause);

  for ( ; j <= pla; j++) {

    if (j != pi) {
      /* The NegLit isn't copied! */
      clause_SetLiteral(NewClause, (j + help), 
	clause_LiteralCreate(subst_Apply(SubstPartnerTermS,
	  term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));

    } else {
      /* The index has to be shifted as above.  */
      help--;
    }
  }

  /* help = number of already set (GivenClause-) literals */
  help += clause_NumOfSuccLits(GivenClause) - 1;

  for ( ; j <= pls; j++) {
    clause_SetLiteral(NewClause, (j + help), 
      clause_LiteralCreate(subst_Apply(SubstPartnerTermS,
	term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));
  } /* end of NewClause creation (last for loop). */


  clause_SetDataFromParents(NewClause,PartnerClause,pi,GivenClause,i,
			    Flags, Precedence);
  clause_SetFromGeneralResolution(NewClause);

  return(NewClause);
}


LIST inf_GeneralResolution(CLAUSE GivenClause, SHARED_INDEX ShIndex,
			   BOOL Ordered, BOOL Equations, 
			   FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause and an Index, usually the WorkedOffIndex,
           two boolean flags, a flag store and a precedence.
  RETURNS: A list of clauses derivable from the GivenClause by 
           GeneralResolution wrt. the Index.
	   If <Ordered>=TRUE, this function generates ordered
	   resolution inferences (the literals must be selected or
	   (strict) maximal), otherwise it generates standard
	   resolution inferences.
	   If <Equations>=TRUE, equations are allowed for inferences,
	   else no inferences with equations are generated. The
	   default is <Equations>=FALSE..
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  CLAUSE GivenCopy;
  LIST Result;
  LITERAL ActLit;
  TERM Atom;
  int i,n;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GeneralResolution: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  if (!clause_HasSolvedConstraint(GivenClause))
    return list_Nil();
  
  Result = list_Nil();
  GivenCopy = clause_Copy(GivenClause);
  
  if (clause_GetFlag(GivenCopy,CLAUSESELECT))
    n = clause_LastAntecedentLitIndex(GivenCopy);
  else
    n = clause_LastSuccedentLitIndex(GivenCopy);
  
  for (i = clause_FirstAntecedentLitIndex(GivenCopy); i <= n; i++) {
    
    ActLit = clause_GetLiteral(GivenCopy, i);
    Atom   = clause_LiteralAtom(ActLit);
    
    if ((Equations || !fol_IsEquality(Atom)) &&
	(clause_LiteralGetFlag(ActLit,LITSELECT) ||
	 (!clause_GetFlag(GivenCopy,CLAUSESELECT) &&
	  (!Ordered || clause_LiteralIsMaximal(ActLit)))) &&	
	(!Ordered || clause_LiteralIsFromAntecedent(ActLit) || 
	 clause_LiteralGetFlag(ActLit,STRICTMAXIMAL))) {
      /* Positive literals must be strict maximal for ORe,     */
      /* negative literals must be either selected or maximal. */
      LIST TermList;
      BOOL Swapped;

      Swapped = FALSE;

      /* The 'endless' loop may run twice for equations, once for other atoms */
      while (TRUE) {
	TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
				 cont_RightContext(), Atom);
	
	for ( ; !list_Empty(TermList); TermList = list_Pop(TermList)) {
	  LIST LitList;
	  TERM PartnerAtom;
	  
	  PartnerAtom = list_First(TermList);
	  
	  if (!term_IsVariable(PartnerAtom)) {	  
	    LITERAL PartnerLit;
	    int     j;
	    CLAUSE  PartnerClause;
	    
	    for (LitList =  sharing_NAtomDataList(PartnerAtom); 
		 !list_Empty(LitList);  LitList = list_Cdr(LitList)) {
	      PartnerLit    = list_Car(LitList);
	      j             = clause_LiteralGetIndex(PartnerLit);
	      PartnerClause = clause_LiteralOwningClause(PartnerLit);
	      
	      if (clause_LiteralsAreComplementary(PartnerLit,ActLit) &&
		  clause_HasSolvedConstraint(PartnerClause) &&
		  /* Negative literals must be from the antecedent */
		  (clause_LiteralIsPositive(PartnerLit) ||
		   clause_LiteralIsFromAntecedent(PartnerLit)) &&
		  /* Check whether literal is selected or maximal */
		  (clause_LiteralGetFlag(PartnerLit,LITSELECT) ||
		   (!clause_GetFlag(PartnerClause,CLAUSESELECT) &&
		    (!Ordered || clause_LiteralIsMaximal(PartnerLit)))) &&
		  /* Positive literals must be strict maximal for ORe */ 
		  (!Ordered || clause_LiteralIsNegative(PartnerLit) || 
		   clause_LiteralGetFlag(PartnerLit,STRICTMAXIMAL)) &&
		  /* Avoid duplicate self-inferences */
		  (clause_LiteralIsPositive(PartnerLit) ||
		   clause_Number(GivenClause) != clause_Number(PartnerClause))) {
		SUBST  Subst, PartnerSubst;
		SYMBOL MaxVar;
		
		MaxVar = clause_MaxVar(PartnerClause);
		clause_RenameVarsBiggerThan(GivenCopy, MaxVar);

		cont_Check();
		if (!unify_UnifyNoOC(cont_LeftContext(), Atom, cont_RightContext(),
				     PartnerAtom)) {
		  misc_StartErrorReport();
		  misc_ErrorReport("\n In inf_GeneralResolution: Unification failed.");
		  misc_FinishErrorReport();
		}
		subst_ExtractUnifier(cont_LeftContext(), &Subst,
				     cont_RightContext(), &PartnerSubst);
		cont_Reset();
		
		if (!Ordered ||
		    inf_LiteralsMax(GivenCopy, i, Subst, PartnerClause, j,
				    PartnerSubst, Flags, Precedence)) {
		  if (clause_LiteralIsNegative(PartnerLit))
		    Result = list_Cons(inf_ApplyGenRes(ActLit,PartnerLit,Subst,
						       PartnerSubst,
						       Flags, Precedence),
				       Result);
		  else
		    Result = list_Cons(inf_ApplyGenRes(PartnerLit, ActLit, 
						       PartnerSubst,Subst,
						       Flags, Precedence),
				       Result);
		}
		subst_Delete(Subst);
		subst_Delete(PartnerSubst);
	      }
	    } /* end of for (LitList = sharing_NAtomDataList ...). */
	  } /* end of if (!term_IsVariable(PartnerAtom)). */
	} /* end of for (TermList = st_GetUnifier...). */
	if (!Swapped && fol_IsEquality(Atom)) {
	  term_EqualitySwap(Atom);         /* Atom is from copied clause */
	  Swapped = TRUE;
	} else
	  break;
      } /* end of 'endless' loop */
    } /* end of if (clause_LiteralIsMaximal(ActLit)). */
  } /* end of for 'all antecedent and succedent literals'. */
  
  clause_Delete(GivenCopy);
  
  return Result;
}


LIST inf_UnitResolution(CLAUSE GivenClause, SHARED_INDEX ShIndex,
			BOOL Equations, FLAGSTORE Flags,
			PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause and an Index, usually the WorkedOffIndex,
           a boolean flag, a flag store and a precedence.
  RETURNS: A list of clauses derivable from the Givenclause by 
           Unit Resolution wrt. the Index.
	   This function does the same inferences as standard resolution,
	   except that at least one of the clauses must be a unit clause.
	   The involved literals don't have to be maximal.
	   If <Equations>=TRUE, equations are allowed for inferences,
	   else no inferences with equations are made. The
	   default is <Equations>=FALSE..
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  CLAUSE   GivenCopy;
  LIST     Result;
  LITERAL  ActLit;
  TERM     Atom;
  BOOL     GivenIsUnit;
  int      i,n;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_UnitResolution: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  if (!clause_HasSolvedConstraint(GivenClause))
    return list_Nil();
    
  Result = list_Nil();
  
  GivenCopy   = clause_Copy(GivenClause);
  GivenIsUnit = (clause_Length(GivenCopy) == 1);
  
  if (clause_GetFlag(GivenCopy,CLAUSESELECT))
    n = clause_LastAntecedentLitIndex(GivenCopy);
  else
    n = clause_LastSuccedentLitIndex(GivenCopy);
  
  for (i=clause_FirstAntecedentLitIndex(GivenCopy); i <= n; i++) {
    
    ActLit = clause_GetLiteral(GivenCopy, i);
    Atom   = clause_LiteralAtom(ActLit);
    
    if ((Equations || !fol_IsEquality(Atom)) &&
	(clause_LiteralGetFlag(ActLit,LITSELECT) ||
	 !clause_GetFlag(GivenCopy,CLAUSESELECT))) {
      LIST TermList;
      BOOL Swapped;
      
      Swapped = FALSE;

      /* The 'endless' loop runs twice for equations, once for other atoms */
      while (TRUE) {
	TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
				 cont_RightContext(), Atom);
	
	for ( ; !list_Empty(TermList); TermList = list_Pop(TermList)) {
	  LIST LitList;
	  TERM PartnerAtom;
	  
	  PartnerAtom = list_First(TermList);
	  
	  if (!term_IsVariable(PartnerAtom)) {
	    LITERAL PartnerLit;
	    CLAUSE  PartnerClause;
	    
	    for (LitList =  sharing_NAtomDataList(PartnerAtom); 
		 !list_Empty(LitList);  LitList = list_Cdr(LitList)) {
	      PartnerLit    = list_Car(LitList);
	      PartnerClause = clause_LiteralOwningClause(PartnerLit);
	      
	      if ((GivenIsUnit || clause_Length(PartnerClause) == 1) &&
		  clause_LiteralsAreComplementary(PartnerLit,ActLit) &&
		  clause_HasSolvedConstraint(PartnerClause) &&
		  /* Negative literals must be from the antecedent */
		  (clause_LiteralIsPositive(PartnerLit) ||
		   clause_LiteralIsFromAntecedent(PartnerLit)) &&
		  /* Either the literal is selected or no literal is selected */
		  (clause_LiteralGetFlag(PartnerLit,LITSELECT) ||
		   !clause_GetFlag(PartnerClause,CLAUSESELECT))) {
		/* Self-inferences aren't possible, since then the clause must */
		/* be a unit and a single literal can't be both positive and   */
		/* negative.                                                   */
		SUBST  Subst, PartnerSubst;
		SYMBOL MaxVar;
		
		MaxVar = clause_MaxVar(PartnerClause);
		clause_RenameVarsBiggerThan(GivenCopy, MaxVar);
		
		cont_Check();
		if (!unify_UnifyNoOC(cont_LeftContext(), Atom,
				     cont_RightContext(), PartnerAtom)) {
		  misc_StartErrorReport();
		  misc_ErrorReport("\n In inf_UnitResolution: Unification failed.");
		  misc_FinishErrorReport();
		}
		subst_ExtractUnifier(cont_LeftContext(), &Subst,
				     cont_RightContext(), &PartnerSubst);
		cont_Reset();
		
		if (clause_LiteralIsNegative(PartnerLit))
		  Result = list_Cons(inf_ApplyGenRes(ActLit, PartnerLit, Subst,
						     PartnerSubst,
						     Flags, Precedence),
				     Result);
		else
		  Result = list_Cons(inf_ApplyGenRes(PartnerLit, ActLit, 
						     PartnerSubst,Subst,
						     Flags, Precedence),
				     Result);
		subst_Delete(Subst);
		subst_Delete(PartnerSubst);
	      }
	    } /* end of for (LitList = sharing_NAtomDataList ...). */
	  } /* end of if (!term_IsVariable(PartnerAtom)). */
	} /* end of for (TermList = st_GetUnifier...). */
	if (!Swapped && fol_IsEquality(Atom)) {
	  term_EqualitySwap(Atom);        /* Atom is from copied clause */
	  Swapped = TRUE;
	} else
	  break;
      } /* end of 'endless' loop */
    } /* end of if (clause_LiteralIsMaximal(ActLit)). */
  } /* end of for 'all antecedent and succedent literals'. */
  
  clause_Delete(GivenCopy);
  
  return Result;
}

LIST inf_BoundedDepthUnitResolution(CLAUSE GivenClause, SHARED_INDEX ShIndex,
				    BOOL ConClause, FLAGSTORE Flags,
				    PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause and an Index, usually the WorkedOffIndex,
           a flag indicating whether the partner clause must be
	   a conjecture clause, a flag store and a precedence.
  RETURNS: A list of clauses derivable from the Givenclause by 
           bounded depth unit resolution wrt. the Index.
	   This acts similar to inf_UnitResolution, except that
	   it limits the depth of resolvents to the maximum 
	   depth of its parent clauses.
  MEMORY:  A list of clauses is produced, where memory for the 
           list and the clauses is allocated.
***************************************************************/
     /* GivenClause is always a CONCLAUSE */
{
  CLAUSE  GivenCopy;
  LIST    Result;
  LITERAL ActLit;
  TERM    Atom;
  int     i,n,depth;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_BoundedDepthUnitResolution: Illegal input.");
    misc_FinishErrorReport();
  }
  cont_Check();
#endif

  Result    = list_Nil();
  GivenCopy = clause_Copy(GivenClause);
  n         = clause_LastLitIndex(GivenCopy);
  depth     = clause_ComputeTermDepth(GivenCopy);

  for (i = clause_FirstLitIndex(); i <= n; i++) {
    LIST TermList;
    BOOL Swapped;

    ActLit   = clause_GetLiteral(GivenCopy, i);
    Atom     = clause_LiteralAtom(ActLit);
    Swapped  = FALSE;

    /* The 'endless' loop runs twice for equations, once for other atoms */
    while (TRUE) {
      TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
			       cont_RightContext(), Atom);

      for ( ; !list_Empty(TermList); TermList = list_Pop(TermList)) {
	LIST LitList;
	TERM PartnerAtom;
	
	PartnerAtom = list_First(TermList);
	
	if (!term_IsVariable(PartnerAtom)) {
	  LITERAL PartnerLit;
	  CLAUSE  PartnerClause;
	  
	  for (LitList = sharing_NAtomDataList(PartnerAtom);
	       !list_Empty(LitList); LitList = list_Cdr(LitList)) {
	    PartnerLit    = list_Car(LitList);
	    PartnerClause = clause_LiteralOwningClause(PartnerLit);
	    
	    if (clause_LiteralsAreComplementary(PartnerLit,ActLit) &&
		(clause_Length(GivenCopy)==1 || clause_Length(PartnerClause)==1) &&
		(clause_GetFlag(GivenCopy,CONCLAUSE) ||
		 clause_GetFlag(PartnerClause,CONCLAUSE)) &&
		(!ConClause || clause_GetFlag(PartnerClause,CONCLAUSE))) {
	      SUBST  Subst, PartnerSubst;
	      SYMBOL MaxVar;
	      int    maxdepth;
	      CLAUSE Resolvent;
	      
	      maxdepth = misc_Max(depth, clause_ComputeTermDepth(PartnerClause));
	      MaxVar   = clause_MaxVar(PartnerClause);
	      clause_RenameVarsBiggerThan(GivenCopy, MaxVar);
	      
	      cont_Check();
	      if (!unify_UnifyNoOC(cont_LeftContext(), Atom,
				   cont_RightContext(), PartnerAtom)) {
		misc_StartErrorReport();
		misc_ErrorReport("\n In inf_BoundedDepthUnitResolution: Unification failed.");
		misc_FinishErrorReport();
	      }
	      subst_ExtractUnifier(cont_LeftContext(), &Subst,
				   cont_RightContext(), &PartnerSubst);
	      cont_Reset();
	      
	      if (clause_LiteralIsNegative(PartnerLit))
		Resolvent = inf_ApplyGenRes(ActLit, PartnerLit, Subst,
					    PartnerSubst, Flags, Precedence);
	      else
		Resolvent = inf_ApplyGenRes(PartnerLit, ActLit, PartnerSubst,
					    Subst, Flags, Precedence);

	      if (clause_ComputeTermDepth(Resolvent) > maxdepth)
		clause_Delete(Resolvent);
	      else {
		Result = list_Cons(Resolvent,Result);
	      }
	      subst_Delete(Subst);
	      subst_Delete(PartnerSubst);
	    }
	  }
	}
      }
      if (!Swapped && fol_IsEquality(Atom)) {
	term_EqualitySwap(Atom);              /* Given Clause is a copy */
	Swapped = TRUE;
      } else
	break;
    } /* end of 'endless' loop */ 
  }

  clause_Delete(GivenCopy);

  return(Result);
}

static CLAUSE inf_ApplyGeneralFactoring(CLAUSE Clause, NAT i, NAT j,
					SUBST Subst, FLAGSTORE Flags,
					PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause an index in the clause, a substitution a 
           flag store and a precedence.
  RETURNS: A new clause obtained from <Clause> by applying <Subst>
           and deleting literal <j> keeping literal <i>
***************************************************************/
{
  CLAUSE NewClause;

  NewClause = clause_Copy(Clause);
  clause_ClearFlags(NewClause);
  clause_SubstApply(Subst, NewClause);
  
  clause_PrecomputeOrdering(NewClause,Flags,Precedence); /* uncomparable equations may have become comparable */

  clause_DeleteLiteral(NewClause, i, Flags, Precedence);

  list_Delete(clause_ParentClauses(NewClause)); 
  list_Delete(clause_ParentLiterals(NewClause));
  clause_SetParentLiterals(NewClause,list_Nil());
  clause_SetParentClauses(NewClause,list_Nil());

  clause_SetDataFromFather(NewClause, Clause, j, Flags, Precedence);
  clause_SetFromGeneralFactoring(NewClause);

  clause_AddParentClause(NewClause, clause_Number(Clause));
  clause_AddParentLiteral(NewClause, i);

  clause_NewNumber(NewClause);

  return NewClause;
}


LIST inf_GeneralFactoring(CLAUSE GivenClause, BOOL Ordered, BOOL Left,
			  BOOL Equations, FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, three boolean flags, a flag store and a
           precedence.
           If <Ordered>=TRUE, this function generates ordered
	   factoring inferences, otherwise standard factoring
	   inferences.
	   If <Left> is FALSE, this function only makes factoring
	   right inferences, otherwise it also makes factoring left
	   inferences.
	   If <Equations>=TRUE, equations are allowed for inferences,
	   else no inferences with equations are generated. The
	   default is <Equations>=TRUE.
  RETURNS: A list of clauses derivable from <GivenClause> by GF.
***************************************************************/
{
  LIST    Result;
  LITERAL ActLit;
  int     i,j,last;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GeneralFactoring: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  if (!clause_HasSolvedConstraint(GivenClause))
    return list_Nil();

  Result = list_Nil();

  /* Always try Factoring Right inferences */
  last = clause_LastSuccedentLitIndex(GivenClause);
  if (!clause_GetFlag(GivenClause,CLAUSESELECT)) {
    for (i = clause_FirstSuccedentLitIndex(GivenClause); i <= last; i++) {
      ActLit = clause_GetLiteral(GivenClause, i);
      if ((!Ordered || clause_LiteralIsMaximal(ActLit)) &&
	  (Equations || !clause_LiteralIsEquality(ActLit))) {
	TERM    Atom, PartnerAtom;
	LITERAL PartnerLit;
	Atom = clause_LiteralAtom(ActLit);
	for (j = clause_FirstSuccedentLitIndex(GivenClause); j <= last; j++) {
	  if (i != j) {
	    PartnerLit = clause_GetLiteral(GivenClause, j);
	    PartnerAtom = clause_LiteralAtom(PartnerLit);
	    if ((j>i ||(Ordered && !clause_LiteralIsMaximal(PartnerLit))) &&
		term_EqualTopSymbols(Atom, PartnerAtom)) {
	      /* This condition avoids duplicate inferences */
	      cont_Check();
	      if (unify_UnifyCom(cont_LeftContext(), Atom, PartnerAtom)) {
		SUBST mgu;
		subst_ExtractUnifierCom(cont_LeftContext(), &mgu);
		if (!Ordered || inf_LitMax(GivenClause,i,j,mgu,FALSE,
					   Flags, Precedence))
		  Result = list_Cons(inf_ApplyGeneralFactoring(GivenClause,i,j,
							       mgu,Flags, 
							       Precedence),
				     Result);
		subst_Delete(mgu);
	      }
	      cont_Reset();
	      if (fol_IsEquality(Atom) &&  /* PartnerAtom is equality, too */
		  unify_UnifyCom(cont_LeftContext(),
				 term_SecondArgument(Atom), 
				 term_FirstArgument(PartnerAtom)) &&
		  unify_UnifyCom(cont_LeftContext(),
				 term_FirstArgument(Atom), 
				 term_SecondArgument(PartnerAtom))) {
		SUBST mgu;
		subst_ExtractUnifierCom(cont_LeftContext(), &mgu);
		if (!Ordered || inf_LitMax(GivenClause,i,j,mgu,FALSE,
					   Flags, Precedence))
		  Result = list_Cons(inf_ApplyGeneralFactoring(GivenClause,i,j,
							       mgu,Flags, 
							       Precedence),
				     Result);
		subst_Delete(mgu);
	      }
	      cont_Reset();
	    }
	  } 
	}
      }
    }
  }
  /* Try Factoring Left inferences only if <Left>==TRUE */
  if (Left) {
    last = clause_LastAntecedentLitIndex(GivenClause);
    for (i = clause_FirstAntecedentLitIndex(GivenClause); i <= last; i++) {
      ActLit = clause_GetLiteral(GivenClause, i);
      if ((Equations || !clause_LiteralIsEquality(ActLit)) &&
	  (clause_LiteralGetFlag(ActLit,LITSELECT) ||
	   (!clause_GetFlag(GivenClause,CLAUSESELECT) &&
	    (!Ordered || clause_LiteralIsMaximal(ActLit))))) {
	TERM    Atom, PartnerAtom;
	LITERAL PartnerLit;
	Atom = clause_LiteralAtom(ActLit);
	for (j = clause_FirstAntecedentLitIndex(GivenClause);j <= last; j++) {
	  if (i != j) {
	    PartnerLit = clause_GetLiteral(GivenClause, j);
	    PartnerAtom = clause_LiteralAtom(PartnerLit);
	    /* In order to avoid duplicate inferences, we do the following  */
	    /* somewhat "tricky" test. What we want is something like       */
	    /* "if (j>i || j wasn't considered within the outer loop) {...} */
	    /* This lengthy condition can be transformed into the following */
	    /* condition, because only one negative literal is selected.    */
	    /* This implies that the literal at index j can't be selected.  */
	    if ((j>i || clause_LiteralGetFlag(ActLit,LITSELECT) ||
		(Ordered && !clause_LiteralIsMaximal(PartnerLit))) &&
		term_EqualTopSymbols(Atom, PartnerAtom)) {
	      PartnerAtom = clause_LiteralAtom(PartnerLit);
	      cont_Check();
	      if (unify_UnifyCom(cont_LeftContext(), Atom, PartnerAtom)) {
		SUBST mgu;
		subst_ExtractUnifierCom(cont_LeftContext(), &mgu);
		if (!Ordered || clause_LiteralGetFlag(ActLit,LITSELECT) ||
		    inf_LitMax(GivenClause,i,j,mgu,FALSE,Flags, Precedence))
		  Result = list_Cons(inf_ApplyGeneralFactoring(GivenClause,i,j,
							       mgu,Flags,
							       Precedence),
				     Result);
		subst_Delete(mgu);
	      }
	      cont_Reset();
	      if (fol_IsEquality(Atom) && /* PartnerAtom is equality, too */
		  unify_UnifyCom(cont_LeftContext(),
				 term_SecondArgument(Atom), 
				 term_FirstArgument(PartnerAtom)) &&
		  unify_UnifyCom(cont_LeftContext(),
				 term_FirstArgument(Atom), 
				 term_SecondArgument(PartnerAtom))) {
		SUBST mgu;
		subst_ExtractUnifierCom(cont_LeftContext(), &mgu);
		if (!Ordered || clause_LiteralGetFlag(ActLit,LITSELECT) ||
		    inf_LitMax(GivenClause,i,j,mgu,FALSE,Flags, Precedence))
		  Result = list_Cons(inf_ApplyGeneralFactoring(GivenClause,i,j,
							       mgu,Flags,
							       Precedence),
				     Result);
		subst_Delete(mgu);
	      }
	      cont_Reset();
	    }
	  } 
	}
      }
    }
  }
  cont_Check();

  return Result;
}


/***************************************************************/
/* START of code for new Superposition Left rule               */
/***************************************************************/


static LIST inf_GenLitSPLeft(CLAUSE Clause, TERM Left, TERM Right, int i,
			     SHARED_INDEX ShIndex,BOOL OrdPara, BOOL MaxPara,
			     FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause (unshared) with a positive equality literal
           at position <i> where <Left> and <Right> are the 
	   arguments of just that literal, two boolean flags, 
	   a flag store and a precedence.
	   For Ordered Paramodulation and Superposition <Right>
           mustn't be greater wrt. the ordering than <Left>.
	   For Superposition the literal must be strictly maximal.
  RETURNS: A list of clauses derivable with the literals owning
           clause by Superposition Left wrt. the Index.
  MEMORY:  The list of clauses is extended, where memory for the
           list and the clauses is allocated.
***************************************************************/
{
  LIST Result, Terms;

#ifdef CHECK
  if (clause_GetFlag(Clause, CLAUSESELECT) ||
      (OrdPara &&
       clause_LiteralIsOrientedEquality(clause_GetLiteral(Clause,i)) &&
       Left == term_SecondArgument(clause_GetLiteralAtom(Clause,i))) ||
      (MaxPara &&
       !clause_LiteralGetFlag(clause_GetLiteral(Clause,i), STRICTMAXIMAL))) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GenLitSPLeft: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  Result = list_Nil();
  Terms  = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
			 cont_RightContext(), Left);

  for ( ; !list_Empty(Terms); Terms = list_Pop(Terms)) {
    LIST Lits;
    TERM Term;

    Term = (TERM)list_First(Terms);

    if (!term_IsVariable(Term) && !symbol_IsPredicate(term_TopSymbol(Term))) {

      Lits = sharing_GetDataList(Term, ShIndex);

      for ( ; !list_Empty(Lits); Lits = list_Pop(Lits)){

	LITERAL PartnerLit;
	TERM    PartnerAtom;
	CLAUSE  PartnerClause;
	int     pli;

	PartnerLit        = (LITERAL)list_Car(Lits); /* Antecedent Literal ! */
	PartnerAtom       = clause_LiteralAtom(PartnerLit);  
	pli               = clause_LiteralGetIndex(PartnerLit);
	PartnerClause     = clause_LiteralOwningClause(PartnerLit);

	if ((clause_LiteralGetFlag(PartnerLit,LITSELECT) ||
	     (!clause_GetFlag(PartnerClause,CLAUSESELECT) &&
	      (!MaxPara || clause_LiteralIsMaximal(PartnerLit)))) &&
	    clause_LiteralIsNegative(PartnerLit) &&
	    !clause_GetFlag(PartnerClause,NOPARAINTO) &&
	    clause_HasSolvedConstraint(PartnerClause)) {
	  /* If <PartnerClause> has a solved constraint and <PartnerLit> */
	  /* is negative then <PartnerLit> is from the antecedent.       */

	  SUBST  Subst, PartnerSubst;
	  TERM   NewLeft,NewRight;
	  SYMBOL PartnerMaxVar;
	  TERM   SupAtom; 

	  SupAtom = (TERM)NULL;
	  PartnerMaxVar = clause_MaxVar(PartnerClause);
	  NewLeft       = Left;
	  clause_RenameVarsBiggerThan(Clause, PartnerMaxVar);

	  cont_Check();
	  unify_UnifyNoOC(cont_LeftContext(), Left, cont_RightContext(), Term);
	  subst_ExtractUnifier(cont_LeftContext(), &Subst,
			       cont_RightContext(), &PartnerSubst);
	  cont_Reset();

	  if (!MaxPara ||
	      inf_LiteralsMax(Clause, i, Subst, PartnerClause, pli,
			      PartnerSubst, Flags, Precedence)) {
	    NewRight = subst_Apply(Subst, term_Copy(Right));
	    if (OrdPara &&
		!clause_LiteralIsOrientedEquality(clause_GetLiteral(Clause,i)))
	      NewLeft  = subst_Apply(Subst, term_Copy(Left));
	    if (!OrdPara ||
		NewLeft == Left ||
		ord_Compare(NewLeft,NewRight,Flags, Precedence) != ord_SmallerThan()) {
	      if (!MaxPara ||
            (clause_LiteralIsPredicate(PartnerLit) &&            
               (!symbol_HasProperty(term_TopSymbol(PartnerAtom), TRANSITIVE)))) {
          SupAtom = inf_AllTermsRplac(PartnerAtom,Term,NewRight,PartnerSubst);
	      } else {
          /* Superposition and <PartnerLit> is equality (or a special and we want to restrict subterm chaining) */
          if (clause_LiteralIsOrientedEquality(PartnerLit))
            SupAtom = inf_AllTermsLeftRplac(PartnerAtom,Term,NewRight,
                     PartnerSubst);
          else {
            ord_RESULT Compare;            
            TERM NewPartnerLeft,NewPartnerRight;

            Compare = ord_UNCOMPARABLE;
            
            if (clause_LiteralIsPredicate(PartnerLit)) {
              /*Then it is necessarily special ... */
#ifdef CHECK
  if (!symbol_HasProperty(term_TopSymbol(PartnerAtom), TRANSITIVE)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GenLitSPLeft: Nonspecial predicate in superposition branch.");
    misc_FinishErrorReport();
  }
#endif
              /* ... and we can first fetch the order status */
              Compare = clause_LiteralGetOrderStatus(PartnerLit);             
            }

            if (Compare == ord_UNCOMPARABLE) {
              NewPartnerLeft  = subst_Apply(PartnerSubst, 
                      term_Copy(term_FirstArgument(PartnerAtom)));
              NewPartnerRight = subst_Apply(PartnerSubst, 
                term_Copy(term_SecondArgument(PartnerAtom)));

              Compare = ord_Compare(NewPartnerLeft,NewPartnerRight,
                    Flags, Precedence);

              term_Delete(NewPartnerLeft);
              term_Delete(NewPartnerRight);
            }

            switch (Compare) {
            case ord_SMALLER_THAN:
              SupAtom = inf_AllTermsRightRplac(PartnerAtom,Term,
                        NewRight,PartnerSubst);
              break;
            case ord_GREATER_THAN:
              SupAtom = inf_AllTermsLeftRplac(PartnerAtom,Term,
                       NewRight,PartnerSubst);
              break;
            default:
              SupAtom = inf_AllTermsRplac(PartnerAtom,Term,
                   NewRight,PartnerSubst);
            }            
          }
	      }

	      if (SupAtom != NULL)
		Result = list_Cons(inf_ApplyGenSuperposition(Clause, i, Subst, 
							     PartnerClause, pli,
							     PartnerSubst, 
							     SupAtom, FALSE,
							     OrdPara, MaxPara,
							     Flags, Precedence),
				   Result);
	    }
	    if (NewLeft != Left)
	      term_Delete(NewLeft);
	    term_Delete(NewRight);
	  }
	  subst_Delete(Subst);
	  subst_Delete(PartnerSubst);
	}
      }
    }
  }
  return Result;
}


static LIST inf_GenSPLeftEqTransToGiven(CLAUSE Clause, int i, BOOL Left,
				   SHARED_INDEX ShIndex, BOOL OrdPara,
				   BOOL MaxPara, BOOL Unit, FLAGSTORE Flags,
				   PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An unshared clause, the index of an antecedent 
           literal that is an equality (or transitive) literal,
           a boolean value, a shared index, three boolean flags 
	   controlling inference preconditions, a flag store 
	   and a precedence.
           If Left==TRUE then the left argument of the literal is used
	   otherwise the right argument.
	   OrdPara and MaxPara control inference conditions.
	   If <Unit>==TRUE the clause with the maximal, positive
	   equality must be a unit clause.
  RETURNS: A list of clauses derivable from generalized 
           superposition Left on the
           GivenCopy  wrt. the Index. See GenSuperpositionLeft
	   for effects of OrdPara and MaxPara
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  LIST    Result, TermList, ParentList;
  int     Bottom;
  LITERAL Lit;
  TERM    Atom, Term, PartnerTerm, PartnerEq;

  Result = list_Nil();
  Lit    = clause_GetLiteral(Clause,i); /* Is an antecedent Literal ! */
  Atom   = clause_LiteralAtom(Lit);

#ifdef CHECK
  {
    if (clause_GetFlag(Clause, NOPARAINTO) ||
        (!clause_LiteralIsEquality(Lit) &&
            (!symbol_HasProperty(term_TopSymbol(Atom), TRANSITIVE))) ||
        !clause_LiteralIsFromAntecedent(Lit) ||
        (MaxPara && clause_LiteralIsOrientedEquality(Lit) && !Left) ||
        (!clause_LiteralGetFlag(clause_GetLiteral(Clause,i),LITSELECT) &&
        (clause_GetFlag(Clause, CLAUSESELECT) ||
    (MaxPara && !clause_LiteralIsMaximal(clause_GetLiteral(Clause,i)))))) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In inf_GenSPLeftEqTransToGiven: Illegal input.");
      misc_FinishErrorReport();
    }
  }
#endif

  Bottom = stack_Bottom();
  if (Left) 
    sharing_PushOnStack(term_FirstArgument(Atom));
  else
    sharing_PushOnStack(term_SecondArgument(Atom));

  while (!stack_Empty(Bottom)) {
    Term = (TERM)stack_PopResult();
    if (!term_IsVariable(Term)) {
      /* Superposition into variables is not necessary */
      TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
			       cont_RightContext(), Term);
      for ( ;!list_Empty(TermList); TermList = list_Pop(TermList)) {
	PartnerTerm = (TERM)list_Car(TermList);
	for (ParentList = term_SupertermList(PartnerTerm);
	     !list_Empty(ParentList); ParentList = list_Cdr(ParentList)) {
	  PartnerEq = (TERM)list_Car(ParentList);
	  if (fol_IsEquality(PartnerEq)) {
	    CLAUSE  PartnerClause;
	    LITERAL PartnerLit;
	    LIST    Scl;
	    int     j;
	    for (Scl = sharing_NAtomDataList(PartnerEq);
		 !list_Empty(Scl); Scl = list_Cdr(Scl)) {
	      PartnerLit    = (LITERAL)list_Car(Scl);
	      j             = clause_LiteralGetIndex(PartnerLit);
	      PartnerClause = clause_LiteralOwningClause(PartnerLit);

	      if (!clause_GetFlag(PartnerClause,CLAUSESELECT) &&
		  (!MaxPara ||
		   clause_LiteralGetFlag(PartnerLit,STRICTMAXIMAL)) &&
		  (!OrdPara ||
		   PartnerTerm == term_FirstArgument(PartnerEq) ||
		   !clause_LiteralIsOrientedEquality(PartnerLit)) &&
		  clause_LiteralIsPositive(PartnerLit) &&
		  clause_Number(PartnerClause) != clause_Number(Clause) &&
		  (!Unit || clause_Length(PartnerClause) == 1) &&
		  clause_HasSolvedConstraint(PartnerClause)) {
		SYMBOL MaxVar;
		SUBST  Subst, PartnerSubst;

		MaxVar      = clause_MaxVar(PartnerClause);
		clause_RenameVarsBiggerThan(Clause,MaxVar);
		cont_Check();
		unify_UnifyNoOC(cont_LeftContext(), Term,
				cont_RightContext(),PartnerTerm);
		subst_ExtractUnifier(cont_LeftContext(), &Subst,
				     cont_RightContext(),&PartnerSubst);
		cont_Reset();
		if (!MaxPara ||
		    inf_LiteralsMax(Clause, i, Subst, PartnerClause, j,
				    PartnerSubst, Flags, Precedence)) {
		  TERM PartnerLeft,PartnerRight;
		  BOOL Check, PartnerCheck;
		  PartnerLeft = PartnerRight = NULL;
		  PartnerCheck = Check = TRUE;
		  if (OrdPara &&
		      !clause_LiteralIsOrientedEquality(PartnerLit)) {
		    /* Check post condition for partner literal */
		    if (PartnerTerm == term_FirstArgument(PartnerEq))
		      PartnerRight = term_SecondArgument(PartnerEq);
		    else
		      PartnerRight = term_FirstArgument(PartnerEq);
		    PartnerLeft  = subst_Apply(PartnerSubst,
					       term_Copy(PartnerTerm));
		    PartnerRight = subst_Apply(PartnerSubst,
					       term_Copy(PartnerRight));
		    PartnerCheck = (ord_Compare(PartnerLeft,PartnerRight,
						Flags, Precedence)
				    != ord_SmallerThan());
		  }
		  if (PartnerCheck &&
		      MaxPara && !clause_LiteralIsOrientedEquality(Lit)) {
		    /* Check post condition for literal in given clause */
		    TERM NewLeft, NewRight;
		    if (Left) {
		      NewLeft  = term_FirstArgument(Atom);
		      NewRight = term_SecondArgument(Atom);
		    } else {
		      NewLeft  = term_SecondArgument(Atom);
		      NewRight = term_FirstArgument(Atom);
		    }
		    NewLeft  = subst_Apply(Subst, term_Copy(NewLeft));
		    NewRight = subst_Apply(Subst, term_Copy(NewRight));
		    Check = (ord_Compare(NewLeft, NewRight, Flags, Precedence)
			     != ord_SmallerThan());
		    term_Delete(NewLeft);
		    term_Delete(NewRight);
		  }
		  if (Check && PartnerCheck) {
		    /* Make inference only if both tests were successful */
		    TERM SupAtom;
		    SupAtom = NULL;
		    if (PartnerRight == NULL) {
		      if (PartnerTerm==term_FirstArgument(PartnerEq))
			PartnerRight = term_SecondArgument(PartnerEq);
		      else
			PartnerRight = term_FirstArgument(PartnerEq);
		      PartnerRight = subst_Apply(PartnerSubst,
						 term_Copy(PartnerRight));
		    }
		    if (Left)
		      SupAtom = inf_AllTermsLeftRplac(Atom, Term,
						      PartnerRight, Subst);
		    else 
		      SupAtom = inf_AllTermsRightRplac(Atom, Term,
						       PartnerRight, Subst);
#ifdef CHECK
		    if (SupAtom == NULL) {
		      misc_StartErrorReport();
		      misc_ErrorReport("\n In inf_GenSPLeftEqTransToGiven:");
		      misc_ErrorReport(" replacement wasn't possible.");
		      misc_FinishErrorReport();
		    }
#endif
		    Result =
		      list_Cons(inf_ApplyGenSuperposition(PartnerClause, j,
							  PartnerSubst,Clause,
							  i,Subst,SupAtom,
							  FALSE,OrdPara,
							  MaxPara,Flags,
							  Precedence), 
				Result);
		  }
		  if (PartnerLeft != term_Null())
		    term_Delete(PartnerLeft);
		  if (PartnerRight != term_Null())
		    term_Delete(PartnerRight);
		}
		subst_Delete(Subst);
		subst_Delete(PartnerSubst);
	      }
	    }
	  }
	}
      }
    }
  }
  return Result;
}


static LIST inf_GenSPLeftLitToGiven(CLAUSE Clause, int i, TERM Atom,
				    SHARED_INDEX ShIndex, BOOL OrdPara,
				    BOOL MaxPara, BOOL Unit, FLAGSTORE Flags,
				    PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An unshared clause, the index of an antecedent 
           literal that is not an equality literal and its 
	   atom, a shared index, three boolean flags 
	   controlling inference preconditions (see also 
	   inf_GenSuperpositionLeft), a flag store and a
	   precedence.
  RETURNS: A list of clauses derivable from superposition left on the
           GivenCopy  wrt. the Index.
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  LIST    Result, TermList, ParentList;
  int     Bottom;
  LITERAL Lit;
  TERM    Term, PartnerTerm, PartnerEq;

  Result = list_Nil();
  Lit    = clause_GetLiteral(Clause,i);

#ifdef CHECK
  if (clause_LiteralIsEquality(Lit) || !clause_LiteralIsFromAntecedent(Lit)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GenSPLeftLitToGiven: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  Bottom = stack_Bottom();
  sharing_PushListOnStack(term_ArgumentList(Atom));

  while (!stack_Empty(Bottom)) {
    Term = (TERM)stack_PopResult();
    if (!term_IsVariable(Term)) {
      /* Superposition into variables is not necessary */
      TermList = st_GetUnifier(cont_LeftContext(),
			       sharing_Index(ShIndex),
			       cont_RightContext(),
			       Term);
      for ( ; !list_Empty(TermList); TermList=list_Pop(TermList)) {
	PartnerTerm = (TERM)list_Car(TermList);
	for (ParentList = term_SupertermList(PartnerTerm);
	     !list_Empty(ParentList); ParentList = list_Cdr(ParentList)) {
	  PartnerEq = (TERM)list_Car(ParentList);
	  if (fol_IsEquality(PartnerEq)) {
	    CLAUSE  PartnerClause;
	    LITERAL PartnerLit;
	    TERM    PartnerAtom;
	    LIST    Scl;
	    int     j;
	    for (Scl = sharing_NAtomDataList(PartnerEq);
		 !list_Empty(Scl); Scl = list_Cdr(Scl)) {
	      PartnerLit    = (LITERAL)list_Car(Scl);
	      j             = clause_LiteralGetIndex(PartnerLit);
	      PartnerClause = clause_LiteralOwningClause(PartnerLit);
	      PartnerAtom   = clause_LiteralAtom(PartnerLit);
	      if (!clause_GetFlag(PartnerClause,CLAUSESELECT) &&
		  (!MaxPara ||
		   clause_LiteralGetFlag(PartnerLit,STRICTMAXIMAL)) &&
		  (!OrdPara ||
		   PartnerTerm == term_FirstArgument(PartnerAtom) ||
		   !clause_LiteralIsOrientedEquality(PartnerLit)) &&
		  clause_LiteralIsPositive(PartnerLit) &&
		  clause_Number(PartnerClause) != clause_Number(Clause) &&
		  (!Unit || clause_Length(PartnerClause) == 1) &&
		  clause_HasSolvedConstraint(PartnerClause)) {
		SYMBOL MaxVar;
		TERM   PartnerLeft,PartnerRight;
		SUBST  Subst, PartnerSubst;
		TERM   SupAtom;

		SupAtom = (TERM)NULL;
		MaxVar      = clause_MaxVar(PartnerClause);
		clause_RenameVarsBiggerThan(Clause,MaxVar);
		cont_Check();
		unify_UnifyNoOC(cont_LeftContext(),Term,cont_RightContext(),PartnerTerm);
		subst_ExtractUnifier(cont_LeftContext(),&Subst,cont_RightContext(),&PartnerSubst);
		cont_Reset();
		if (!MaxPara ||
		    inf_LiteralsMax(Clause, i, Subst, PartnerClause, j,
				     PartnerSubst, Flags, Precedence)) {
		  PartnerLeft  = subst_Apply(PartnerSubst,
				 term_Copy(PartnerTerm));
		  if (PartnerTerm == term_FirstArgument(PartnerAtom))
		    PartnerRight = subst_Apply(PartnerSubst,
				 term_Copy(term_SecondArgument(PartnerAtom)));
		  else
		    PartnerRight = subst_Apply(PartnerSubst,
				 term_Copy(term_FirstArgument(PartnerAtom)));

		  if (!OrdPara ||
		      clause_LiteralIsOrientedEquality(PartnerLit) ||
		      ord_Compare(PartnerLeft,PartnerRight,Flags, Precedence)
		      != ord_SmallerThan()) {
		    SupAtom = inf_AllTermsRplac(Atom,Term,PartnerRight,Subst);
#ifdef CHECK
		    if (SupAtom == NULL) {
		      misc_StartErrorReport();
		      misc_ErrorReport("\n In inf_GenSPRightLitToGiven:");
		      misc_ErrorReport(" replacement wasn't possible.");
		      misc_FinishErrorReport();
		    }
#endif
		    Result =
		      list_Cons(inf_ApplyGenSuperposition(PartnerClause, j,
							  PartnerSubst, Clause,
							  i, Subst, SupAtom,
							  FALSE, OrdPara,
							  MaxPara, Flags,
							  Precedence), 
				Result);

		  }
		  term_Delete(PartnerLeft);
		  term_Delete(PartnerRight);
		}
		subst_Delete(Subst);
		subst_Delete(PartnerSubst);
	      }
	    }
	  }
	}
      }
    }
  }
  return Result;
}


static LIST inf_GenSPLeftToGiven(CLAUSE Clause, int i, SHARED_INDEX ShIndex, 
				 BOOL OrdPara, BOOL MaxPara, BOOL Unit,
				 FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   An unshared clause, the index of an antecedent
           literal, an index of shared clauses, three boolean
	   flags for controlling inference preconditions (see
	   inf_GenSuperpositionLeft), a flag store and a
	   precedence.
  RETURNS: A list of clauses derivable from Superposition Left
           on the GivenCopy  wrt. the Index.
  MEMORY:  A list of clauses is produced, where memory for the
           list and the clauses is allocated.
***************************************************************/
{
  TERM Atom;
  LIST Result;

#ifdef CHECK
  if (clause_GetFlag(Clause, NOPARAINTO) ||
      (!clause_LiteralGetFlag(clause_GetLiteral(Clause,i),LITSELECT) &&
       (clause_GetFlag(Clause, CLAUSESELECT) ||
	(MaxPara && !clause_LiteralIsMaximal(clause_GetLiteral(Clause,i)))))) {
      misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GenSPLeftToGiven: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  Result  = list_Nil();
  Atom    = clause_LiteralAtom(clause_GetLiteral(Clause,i));

  if (fol_IsEquality(Atom)) {
    Result = list_Nconc(inf_GenSPLeftEqTransToGiven(Clause,i, TRUE,ShIndex, OrdPara,
					       MaxPara, Unit, Flags,Precedence), 
			Result);
    if (!MaxPara ||
	!clause_LiteralIsOrientedEquality(clause_GetLiteral(Clause,i)))
      /* For SPm and OPm always try other direction, for SpR try it */
      /* only if the literal is not oriented.                       */
      Result = list_Nconc(inf_GenSPLeftEqTransToGiven(Clause,i,FALSE,ShIndex,OrdPara,
						 MaxPara,Unit,Flags,Precedence),
			  Result);        
  } else if (symbol_HasProperty(term_TopSymbol(Atom) , TRANSITIVE)) {
    ord_RESULT Compare;
    Compare = clause_LiteralGetOrderStatus(clause_GetLiteral(Clause,i));
    switch(Compare) {
      case ord_GREATER_THAN:
        Result = list_Nconc(inf_GenSPLeftEqTransToGiven(Clause,i,TRUE,ShIndex, OrdPara,
                     MaxPara, Unit, Flags,Precedence), 
          Result);      
        break;
      case ord_SMALLER_THAN:
      case ord_EQUAL:
        Result = list_Nconc(inf_GenSPLeftEqTransToGiven(Clause,i,FALSE,ShIndex,OrdPara,
               MaxPara,Unit,Flags,Precedence),
          Result);      
        break;
      case ord_UNCOMPARABLE:
        Result = list_Nconc(inf_GenSPLeftEqTransToGiven(Clause,i,TRUE,ShIndex, OrdPara,
                     MaxPara, Unit, Flags,Precedence), 
          Result);
        Result = list_Nconc(inf_GenSPLeftEqTransToGiven(Clause,i,FALSE,ShIndex,OrdPara,
               MaxPara,Unit,Flags,Precedence),
          Result);
        break;      
    }  
  } else
    Result = list_Nconc(inf_GenSPLeftLitToGiven(Clause,i,Atom,ShIndex,OrdPara,
						MaxPara,Unit,Flags,Precedence),
			Result);

  return Result;
}


LIST inf_GenSuperpositionLeft(CLAUSE GivenClause, SHARED_INDEX ShIndex, 
			      BOOL OrdPara, BOOL MaxPara, BOOL Unit,
			      FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause and an Index, usually the WorkedOffIndex,
           two boolean flags for controlling inference
           preconditions, a flag store and a precedence.
  RETURNS: A list of clauses derivable from the Givenclause by 
           one of the following inference rules wrt. the Index:

	   OrdPara=TRUE, MaxPara=TRUE 
	   -> Superposition Left

	   OrdPara=TRUE, MaxPara=FALSE
	   -> ordered Paramodulation

	   OrdPara=FALSE, MaxPara=FALSE
	   -> simple Paramodulation

	   OrdPara=FALSE, MaxPara=TRUE
	   -> not defined

	   If <Unit>==TRUE the clause with the maximal equality 
	   additionally must be a unit clause.
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  LIST    Result;
  TERM    Atom;
  CLAUSE  Copy;
  int     i, n;
  LITERAL ActLit;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GenSuperpositionLeft: Illegal input.");
    misc_FinishErrorReport();
  }
  if (!OrdPara && MaxPara) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GenSuperpositionLeft: Illegal inference rule selection,");
    misc_ErrorReport("\n OrdPara=FALSE & MaxPara=TRUE.");
    misc_FinishErrorReport();
  }
#endif

  Result = list_Nil();

  if (!clause_HasSolvedConstraint(GivenClause))
    return Result;
  
  Copy = clause_Copy(GivenClause);
  n    = clause_LastSuccedentLitIndex(Copy);
  
  if (!clause_GetFlag(Copy, CLAUSESELECT) &&
      (!Unit || clause_Length(Copy) == 1)) {
    for (i = clause_FirstSuccedentLitIndex(Copy); i <= n; i++) {
      ActLit = clause_GetLiteral(Copy, i);
      Atom   = clause_LiteralSignedAtom(ActLit);
      
      if (fol_IsEquality(Atom) &&
	  (!MaxPara ||
	   clause_LiteralGetFlag(ActLit,STRICTMAXIMAL))) {
	
	Result =
	  list_Nconc(inf_GenLitSPLeft(Copy, term_FirstArgument(Atom), 
				      term_SecondArgument(Atom), i, ShIndex,
				      OrdPara, MaxPara, Flags, Precedence), 
		     Result);
	if (!OrdPara || !clause_LiteralIsOrientedEquality(ActLit))
	  /* For SPm always try the other direction, for OPm and SpL */
	  /* only try it if the literal is not oriented.             */
	  Result =
	    list_Nconc(inf_GenLitSPLeft(Copy, term_SecondArgument(Atom), 
					term_FirstArgument(Atom), i, ShIndex,
					OrdPara, MaxPara, Flags, Precedence), 
		       Result);
      }
    }
  }

  n = clause_LastAntecedentLitIndex(Copy);  
  if (!clause_GetFlag(Copy,NOPARAINTO)) {
    for (i = clause_FirstAntecedentLitIndex(Copy); i <= n; i++) {
      ActLit = clause_GetLiteral(Copy, i);
      
      if (clause_LiteralGetFlag(ActLit, LITSELECT) ||
	  (!clause_GetFlag(Copy, CLAUSESELECT) &&
	   (!MaxPara || clause_LiteralIsMaximal(ActLit))))
	Result = list_Nconc(inf_GenSPLeftToGiven(Copy, i, ShIndex, OrdPara,
						 MaxPara,Unit,Flags,Precedence),
			    Result);
    }
  }
  clause_Delete(Copy);

  return(Result);
}



LIST inf_ApplyDefinition(PROOFSEARCH Search, CLAUSE Clause,
			 FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A proof search object, a clause, a flag store and a
           precedence.
  RETURNS: A list of clauses derivable from the given clause by 
           applying the (potential) definitions in <Search>.
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  LIST Result, Defs;
  DEF  Def;
  
  Result = list_Nil();
  for (Defs=prfs_Definitions(Search); !list_Empty(Defs); Defs=list_Cdr(Defs)) {
    Def = (DEF)list_Car(Defs);
    Result = list_Nconc(def_ApplyDefToClauseOnce(Def, Clause, Flags, Precedence),
			Result);
  }
  return Result;
}


/**************************************************************
  block with hyperresolution code starts here
***************************************************************/

typedef struct {
  LITERAL NucleusLit;
  LITERAL ElectronLit;
  SUBST   ElectronSubst;
} INF_MAPNODE, *INF_MAPITEM;


static void inf_CopyHyperElectron(CLAUSE Clause, SUBST Subst2, SUBST Subst1,
				  int PLitInd, LIST* Constraint,
				  LIST* Succedent)
/**************************************************************
  INPUT:   An electron clause, a substitution, an index of a
           succedent literal in this clause (the matched one),
	   and two lists by reference.
  RETURNS: Nothing.
  EFFECTS: The constraint and succedent literals are copied into
           the corresponding lists except for the literal with
	   the given index. The composition <Subst2> ° <Subst1>
	   is applied to all copied literals.
	   The antecedent of the electron clause is empty, so
	   there's no need for a third list by reference.
***************************************************************/
{
  TERM Atom;
  int  n, lc, j;

#ifdef CHECK
  if (clause_NumOfAnteLits(Clause) != 0) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_CopyHyperElectron: Electron contains antecedent literals.");
    misc_FinishErrorReport();
  }
#endif

  n  = clause_LastSuccedentLitIndex(Clause);
  lc = clause_LastConstraintLitIndex(Clause);

  for (j = clause_FirstConstraintLitIndex(Clause); j <= n; j++) {
    if (j != PLitInd) {
      Atom = subst_Apply(Subst1, term_Copy(clause_GetLiteralAtom(Clause, j)));
      Atom = subst_Apply(Subst2, Atom);
      if (j <= lc) 
	*Constraint = list_Cons(Atom, *Constraint);
      else /* Literal must be from succedent */
	*Succedent  = list_Cons(Atom, *Succedent);
    }
  }
}


static CLAUSE inf_BuildHyperResolvent(CLAUSE Nucleus, SUBST Subst,
				      LIST FoundMap, BOOL StrictlyMaximal,
				      FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause <Nucleus> with solved sort constraint,
	   the substitution <Subst> for <Nucleus>, a mapping
	   <FoundMap> of literals of already found partner
	   clauses, a boolean flag indicating whether this is
	   a ordered or a standard hyper resolution inference
	   a flag store and a precedence.
  RETURNS: The newly created hyper resolvent.
***************************************************************/
{
  CLAUSE  NewClause;
  LIST    Constraint, Succedent, Parents, ParentNum, ParentLits, Scan;
  intptr_t     i;
  int  bound, Depth;
  LITERAL Lit;
  SUBST   ESubst;
  INF_MAPITEM MapItem;

  Parents    = list_List(Nucleus);      /* parent clauses  */
  ParentNum  = list_Nil();              /* parent clause numbers */
  ParentLits = list_Nil();              /* literal indices */
  Constraint = Succedent = list_Nil();  /* literals of the new clause */
  
  /* Get constraint literals from nucleus */
  bound = clause_LastConstraintLitIndex(Nucleus);
  for (i = clause_FirstConstraintLitIndex(Nucleus); i <= bound; i++)
    Constraint =
      list_Cons(subst_Apply(Subst,term_Copy(clause_GetLiteralAtom(Nucleus,i))),
		Constraint);
  /* Get succedent literals from nucleus */
  bound = clause_LastSuccedentLitIndex(Nucleus);
  for (i = clause_FirstSuccedentLitIndex(Nucleus); i <= bound; i++)
    Succedent =
      list_Cons(subst_Apply(Subst,term_Copy(clause_GetLiteralAtom(Nucleus,i))),
		Succedent);

  /* Now get the remaining data for the resolvent */
  Depth = clause_Depth(Nucleus);
  bound = clause_LastAntecedentLitIndex(Nucleus);
  for (i = clause_FirstAntecedentLitIndex(Nucleus); i <= bound; i++) {
    /* Search <FoundMap> for the nucleus literal with index <i> */
    Lit = clause_GetLiteral(Nucleus, i);
    for (Scan = FoundMap, MapItem = NULL; !list_Empty(Scan);
	 Scan = list_Cdr(Scan)) {
      MapItem = list_Car(Scan);
      if (MapItem->NucleusLit == Lit)
	break;
    }

    if (MapItem == NULL || MapItem->NucleusLit != Lit) {
      misc_StartErrorReport();
      misc_ErrorReport("\n In inf_BuildHyperResolvent: Map entry not found.");
      misc_FinishErrorReport();
    }

    Lit       = MapItem->ElectronLit;
    NewClause = clause_LiteralOwningClause(Lit);
    ESubst    = MapItem->ElectronSubst;

    Depth      = misc_Max(Depth, clause_Depth(NewClause));
    Parents    = list_Cons(NewClause, Parents);
    ParentNum  = list_Cons((POINTER) clause_Number(Nucleus), ParentNum);
    ParentLits = list_Cons((POINTER) i, ParentLits);
    ParentNum  = list_Cons((POINTER) clause_Number(NewClause), ParentNum);
    ParentLits = list_Cons((POINTER) clause_LiteralGetIndex(Lit), ParentLits);

    /* Get the remaining constraint and succedent literals from electron */
    inf_CopyHyperElectron(NewClause,Subst,ESubst,clause_LiteralGetIndex(Lit),
			  &Constraint, &Succedent);
  }

  /* create new clause and set clause data */
  NewClause = clause_Create(Constraint, list_Nil(), Succedent, Flags,Precedence);

  if (StrictlyMaximal)
    clause_SetFromOrderedHyperResolution(NewClause);
  else
    clause_SetFromSimpleHyperResolution(NewClause);

  clause_SetDepth(NewClause, Depth + 1);

  clause_SetSplitDataFromList(NewClause, Parents);

  clause_SetParentClauses(NewClause, list_NReverse(ParentNum));
  clause_SetParentLiterals(NewClause, list_NReverse(ParentLits));

  /* clean up */
  list_Delete(Parents);
  list_Delete(Constraint);
  list_Delete(Succedent);

  return NewClause;
}


static LIST inf_GetHyperResolutionPartnerLits(TERM Atom, SHARED_INDEX Index,
					      BOOL StrictlyMaximal)
/**************************************************************
  INPUT:   An atom, a clause index, and a boolean flag.
  RETURNS: A list of literals from purely positive clauses
           from the index where either <StrictlyMaximal> is
	   false or the literals are strictly maximal in their
	   respective clauses.
***************************************************************/
{
  LIST    Result, TermList, LitScan;
  LITERAL NextLit;
  CLAUSE  Clause;

#ifdef CHECK  
  if (!term_IsAtom(Atom)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_GetHyperResolutionPartnerLits: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  Result   = list_Nil();
  TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(Index),
			   cont_RightContext(), Atom);
  
  for (; !list_Empty(TermList); TermList = list_Pop(TermList)) {
    if (!term_IsVariable(list_Car(TermList))) {
      for (LitScan = sharing_NAtomDataList(list_Car(TermList)); 
	   !list_Empty(LitScan); 
	   LitScan = list_Cdr(LitScan)) {
	NextLit = list_Car(LitScan);
	Clause  = clause_LiteralOwningClause(NextLit);
	if (clause_LiteralIsFromSuccedent(NextLit) &&
	    (!StrictlyMaximal || clause_LiteralGetFlag(NextLit, STRICTMAXIMAL)) &&
	    clause_HasSolvedConstraint(Clause) &&
	    clause_HasEmptyAntecedent(Clause)) 
	  Result = list_Cons(NextLit, Result);
      }
    }
  }
  return Result;
}

static LIST inf_HyperResolvents(CLAUSE Clause, SUBST Subst, LIST Restlits,
				int GlobalMaxVar, LIST FoundMap, 
				BOOL StrictlyMaximal, SHARED_INDEX Index,
				FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A nucleus <Clause> where the sort constraint is
           solved,
           a substitution, that has to be applied to the
	   nucleus,
	   a list <Restlits> of antecedent literals for which
	   a partner clause is searched,
	   a list <FoundMap> of map items (n,e,s), where
	   n is an antecedent literal from the nucleus,
	   e is a positive literal from an electron clause,
	   that is unifiable with n and s is a substitution
	   that has to be applied to the electron clause,
	   a flag store and
	   a precedence.
	   A main invariant of our algorithm is that all involved
	   clauses are pairwise variable disjoint. For that reason
	   we need, when building the resolvent, only apply the electron
	   specific substitution and the composed substitution <Subst>
	   to the electron clauses, and only <Subst> to the nucleus clause.
  RETURNS: The list of hyper-resolvents.
***************************************************************/
{
  LITERAL Lit, PLit;

  if (list_Empty(Restlits)) {
    /* This case stops the recursion */
    LIST        Scan;
    INF_MAPITEM MapItem;

    /* A posteriori test for the electron literals */
    if (StrictlyMaximal) { /* only for ordered hyper resolution */
      for (Scan = FoundMap; !list_Empty(Scan); Scan = list_Cdr(Scan)) {
	MapItem = list_Car(Scan);
	Lit = MapItem->ElectronLit;
	if (!inf_LitMaxWith2Subst(clause_LiteralOwningClause(Lit),
				  clause_LiteralGetIndex(Lit), -1, Subst,
				  MapItem->ElectronSubst,TRUE,Flags,Precedence))
	  return list_Nil();
      }
    }
    /* Build the resolvent */
    return list_List(inf_BuildHyperResolvent(Clause, Subst, FoundMap,
					     StrictlyMaximal,Flags,Precedence));
  }
  else {
    CLAUSE      PartnerCopy;
    LIST        Result, NextLits;
    TERM        AtomCopy;
    SUBST       NewSubst, RightSubst, HelpSubst;
    SYMBOL      NewMaxVar;
    int         PLitInd;
    BOOL        Swapped;
    INF_MAPNODE MapNode;

    Result   = list_Nil();
    Restlits = clause_MoveBestLiteralToFront(list_Copy(Restlits), Subst,
					     GlobalMaxVar,
					     clause_HyperLiteralIsBetter);
    Lit      = list_Car(Restlits);
    Restlits = list_Pop(Restlits);
    AtomCopy = subst_Apply(Subst, term_Copy(clause_LiteralAtom(Lit)));

    Swapped = FALSE;
    
    /* The 'endless' loop may run twice for equations, once for other atoms */
    while (TRUE) {
      NextLits = inf_GetHyperResolutionPartnerLits(AtomCopy,Index,
						   StrictlyMaximal);
      
      for ( ; !list_Empty(NextLits); NextLits = list_Pop(NextLits)) {
	
	PLit        = list_Car(NextLits);
	PLitInd     = clause_LiteralGetIndex(PLit);
	PartnerCopy = clause_Copy(clause_LiteralOwningClause(PLit));
	
	clause_RenameVarsBiggerThan(PartnerCopy, GlobalMaxVar);
	PLit        = clause_GetLiteral(PartnerCopy, PLitInd);
	
	NewMaxVar   = term_MaxVar(clause_LiteralAtom(PLit));
	NewMaxVar   = symbol_GreaterVariable(GlobalMaxVar, NewMaxVar) ?
	  GlobalMaxVar : NewMaxVar;
	
	cont_Check();
	if (!unify_UnifyNoOC(cont_LeftContext(), AtomCopy, cont_RightContext(),
			     clause_LiteralAtom(PLit))) {
	  misc_StartErrorReport();
	  misc_ErrorReport("\n In inf_HyperResolvents: Unification failed.");
	  misc_FinishErrorReport();
	}
	subst_ExtractUnifier(cont_LeftContext(), &NewSubst,
			     cont_RightContext(), &RightSubst);
	cont_Reset();
	
	HelpSubst = NewSubst;
	NewSubst  = subst_Compose(NewSubst, subst_Copy(Subst));
	subst_Delete(HelpSubst);
	
	MapNode.NucleusLit    = Lit;
	MapNode.ElectronLit   = PLit;
	MapNode.ElectronSubst = RightSubst;
	FoundMap = list_Cons(&MapNode, FoundMap);
	
	Result = list_Nconc(inf_HyperResolvents(Clause, NewSubst, Restlits,
						NewMaxVar, FoundMap,
						StrictlyMaximal, Index, Flags,
						Precedence),
			    Result);
	
	subst_Delete(NewSubst);
	subst_Delete(RightSubst);
	clause_Delete(PartnerCopy);
	FoundMap = list_Pop(FoundMap);
      }
      if (!Swapped && fol_IsEquality(AtomCopy)) {
	term_EqualitySwap(AtomCopy);
	Swapped = TRUE;
      } else
	break;
    } /* end of 'endless' loop */

    list_Delete(Restlits);
    term_Delete(AtomCopy);

    return Result;
  }
}


static LIST inf_GetAntecedentLiterals(CLAUSE Clause)
/**************************************************************
  INPUT:   A clause 
  RETURNS: The list of all antecedent literals of the clause.
***************************************************************/
{ 
  int  lc, i;
  LIST Result;
  
  Result = list_Nil();
  lc     = clause_LastAntecedentLitIndex(Clause);
  for (i = clause_FirstAntecedentLitIndex(Clause); i <= lc ; i++) {
    Result = list_Cons(clause_GetLiteral(Clause, i), Result);
  }
  return Result;
}


static LIST inf_ForwardHyperResolution(CLAUSE GivenClause, SHARED_INDEX Index,
				       BOOL StrictlyMaximal, FLAGSTORE Flags,
				       PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause with an solved sort constraint, an 'Index'
           of clauses, a boolean flag, a flag store and a
	   precedence.
  RETURNS: A list of clauses inferred from  <GivenClause> by
           hyper resolution wrt. the index. 
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  LIST Result, RestLits;
;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_ForwardHyperResolution: Illegal input.");
    misc_FinishErrorReport();
  }
#endif
  
  if (clause_HasEmptyAntecedent(GivenClause))
    return list_Nil();

  Result = list_Nil();
  
  /* Build up list of all antecedent literals. */
  RestLits = inf_GetAntecedentLiterals(GivenClause);
  
  Result =  list_Nconc(inf_HyperResolvents(GivenClause, subst_Nil(),
					   RestLits,clause_MaxVar(GivenClause),
					   list_Nil(),StrictlyMaximal,Index,
					   Flags, Precedence), 
		       Result);
  list_Delete(RestLits);

  return Result;
}


static LIST inf_BackwardHyperResolution(CLAUSE Electron, SHARED_INDEX Index,
					BOOL StrictlyMaximal, FLAGSTORE Flags,
					PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause with an solved sort constraint,
           an 'Index' of clauses, a boolean flag, a flag store,
	   and a precedence.
  RETURNS: A list of clauses inferred by hyper resolution 
           wrt. the index with <Electron> as an electron.
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  CLAUSE ElectronCopy;
  LIST   Result;
  int    i, ls;

  if (!clause_HasEmptyAntecedent(Electron) ||
      clause_HasEmptySuccedent(Electron))
    return list_Nil();

  Result = list_Nil();

  ElectronCopy = clause_Copy(Electron);

  /* Search succedent literal in <Electron> */
  ls = clause_LastSuccedentLitIndex(ElectronCopy);
  for (i = clause_FirstSuccedentLitIndex(Electron); i <= ls; i++) {
    LITERAL ElecLit;
    TERM    ElecAtom;

    ElecLit  = clause_GetLiteral(ElectronCopy, i);
    ElecAtom = clause_LiteralAtom(ElecLit);

    if (!StrictlyMaximal || clause_LiteralGetFlag(ElecLit, STRICTMAXIMAL)) {
      LIST CandAtoms;
      BOOL Swapped;

      Swapped = FALSE;

      /* The 'endless' loop may run twice for equations, once for other atoms */
      while (TRUE) {
	/* Get unifiable antecedent literals in nucleus */
	CandAtoms = st_GetUnifier(cont_LeftContext(), sharing_Index(Index),
				  cont_RightContext(), ElecAtom);
	
	for ( ; !list_Empty(CandAtoms); CandAtoms = list_Pop(CandAtoms)) {
	  if (!term_IsVariable(list_Car(CandAtoms))) {
	    LIST CandLits;
	    
	    CandLits = sharing_NAtomDataList(list_Car(CandAtoms));
	    
	    for (; !list_Empty(CandLits); CandLits = list_Cdr(CandLits)) {
	      LITERAL   NucLit;
	      TERM      NucAtom;
	      CLAUSE    Nucleus;
	      
	      NucLit    = list_Car(CandLits);
	      NucAtom   = clause_LiteralAtom(NucLit);
	      Nucleus   = clause_LiteralOwningClause(NucLit);
	      
	      if (clause_LiteralIsFromAntecedent(NucLit) &&
		  clause_HasSolvedConstraint(Nucleus)) {
		LIST    FoundMap, RestLits;
		SUBST   LeftSubst, RightSubst;
		SYMBOL  GlobalMaxVar, MaxVar;
		INF_MAPNODE MapNode;
		
		GlobalMaxVar = clause_MaxVar(Nucleus);
		clause_RenameVarsBiggerThan(ElectronCopy, GlobalMaxVar);
		MaxVar       = clause_SearchMaxVar(ElectronCopy);
		GlobalMaxVar = symbol_GreaterVariable(GlobalMaxVar, MaxVar) ?
		  GlobalMaxVar : MaxVar;
		/* Now ElecLit is renamed, too */

		RestLits     = inf_GetAntecedentLiterals(Nucleus);
		RestLits     = list_PointerDeleteElement(RestLits, NucLit);

		/* Get unifier */
		cont_Check();
		if (!unify_UnifyNoOC(cont_LeftContext(), NucAtom,
				     cont_RightContext(), ElecAtom)) {
		  misc_StartErrorReport();
		  misc_ErrorReport("\n In inf_BackwardHyperResolution: Unification failed.");
		  misc_FinishErrorReport();
		}
		subst_ExtractUnifier(cont_LeftContext(), &LeftSubst,
				     cont_RightContext(), &RightSubst);
		cont_Reset();
		
		MapNode.NucleusLit    = NucLit;
		MapNode.ElectronLit   = ElecLit;
		MapNode.ElectronSubst = RightSubst;
		FoundMap = list_List(&MapNode);
		
		Result = list_Nconc(inf_HyperResolvents(Nucleus, LeftSubst,
							RestLits, GlobalMaxVar,
							FoundMap,StrictlyMaximal,
							Index, Flags,Precedence),
				    Result);
		
		/* clean up */
		subst_Delete(LeftSubst);
		subst_Delete(RightSubst);
		list_Delete(RestLits);
		list_Free(FoundMap);	    
	      } /* if a nucleus has been found */
	    } /* for all nucleus candidate literals */
	  } /* if term is atom */
	} /* for all nucleus candidate atoms */
      	if (!Swapped && fol_IsEquality(ElecAtom)) {
	  term_EqualitySwap(ElecAtom);    /* Atom is from copied clause */
	  Swapped = TRUE;
	} else
	  break;
      } /* end of 'endless' loop */
    } /* for all lits usable in electron for hyper resolution */
  } /* for all lits in succedent */
  clause_Delete(ElectronCopy);
    
  return Result;
}


LIST inf_GeneralHyperResolution(CLAUSE GivenClause, SHARED_INDEX Index,
				BOOL Ordered, FLAGSTORE Flags,
				PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, an 'Index' of clauses, a boolean flag,
           a flag store and a precedence.
  RETURNS: A list of clauses inferred by
           (ordered) hyper resolution wrt. the index.
	   If <Ordered>=TRUE then ordered hyper resolution
	   inferences are made, else standard hyper resolution
	   inferences.
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{ 
  LIST Result;

  Result = list_Nil();
  if (clause_HasSolvedConstraint(GivenClause)) {
    Result = inf_ForwardHyperResolution(GivenClause, Index, Ordered,
					Flags, Precedence); 
    Result = list_Nconc(inf_BackwardHyperResolution(GivenClause, Index, Ordered,
						    Flags, Precedence),
			Result); 
  }
  return Result;
} 

/* TODO: All the chaining rules are quite similar and so are the functions of their implementation.
         Maybe some common parts could be yet extracted
         to support better readability and maintainability of the code
         without serious efficiency losses.
*/

static int inf_GetAxClNo(PROOFSEARCH Search, SYMBOL s) {
  CLAUSE cl = ana_LookupTransAxClause(prfs_GetAnalyze(Search),s);
  return clause_Number(cl);
}

static BOOL inf_OrdChLitMax(CLAUSE Clause, int i, SUBST Subst,
                            BOOL RightArg, FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, the index of a strictly maximal literal,
           a substitution, two boolean flags, a flag store and a precedence.
  RETURNS: The function returns TRUE iff the
           literal at index <i> is still strictly maximal in the 
	         instantiated clause and the right (of left) argument
           of the literal is (strictly) maximal in the literal.
  CAUTION: DON'T call this function with a clause with selected
           literals!
***************************************************************/
{
  TERM       Max, LitTerm;
  LITERAL    Lit, Other;
  ord_RESULT Compare;
  int        k;
  int        LastLitIndex;

  Max = (TERM) NULL;
  LitTerm = (TERM) NULL;  
  
  LastLitIndex = clause_LastSuccedentLitIndex(Clause);

#ifdef CHECK
  if (! clause_LiteralGetFlag(clause_GetLiteral(Clause, i),STRICTMAXIMAL)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_OrdChLitMax: Literal %d isn't strictly maximal.", i);
    misc_FinishErrorReport();
  }
  if (i < clause_FirstSuccedentLitIndex(Clause) ||
      i > LastLitIndex) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_OrdChLitMax: Literal index %d is out of range.", i);
    misc_FinishErrorReport();
  }
  /* If literal <i> is selected, there's no need to check for maximality, */
  /* if <i> isn't selected, but there're other literals selected,         */
  /* inferences with literal <i> are forbidden.                           */ 
  if (clause_GetFlag(Clause, CLAUSESELECT)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_OrdChLitMax: There're selected literals.");
    misc_FinishErrorReport();
  }
#endif

  Lit = clause_GetLiteral(Clause, i);

  /* Check necessary condition */
  if (! clause_LiteralGetFlag(Lit,STRICTMAXIMAL)) {
    return FALSE;
  }

  Max = subst_Apply(Subst,term_Copy(clause_GetLiteralTerm(Clause,i)));
  Compare = ord_Compare(term_FirstArgument(Max),
      term_SecondArgument(Max),
      Flags,Precedence);

  if ( (Compare == ord_Equal()) ||
       (RightArg ? (Compare == ord_GreaterThan())
                 : (Compare == ord_SmallerThan())) ) {
    term_Delete(Max);
    return FALSE;
  }

  /* Only antecedent and succedent literals until LastLitIndex are      */
  /* compared, so if there's only one such literal, it's maximal.       */
  /* If the substitution is empty, the necessary condition tested above */
  /* is sufficient, too.                                                */
  if (clause_FirstAntecedentLitIndex(Clause) == LastLitIndex || subst_Empty(Subst)) {
    term_Delete(Max);
    return TRUE;
  }

  for (k = clause_FirstAntecedentLitIndex(Clause); k <= LastLitIndex; k++) {
    Other = clause_GetLiteral(Clause, k);
    if (k != i && clause_LiteralGetFlag(Other,STRICTMAXIMAL)) {
      /* Only compare with strictly maximal literals, since for every other */
      /* there's at least one maximal literal, that is bigger or equal.*/
      LitTerm = subst_Apply(Subst,term_Copy(clause_GetLiteralTerm(Clause,k)));
      Compare = ord_LiteralCompare(Max,				   
           clause_LiteralGetOrderStatus(Lit),
				   LitTerm,				   
           clause_LiteralGetOrderStatus(Other),
				   TRUE, Flags, Precedence);
         
      if (Compare == ord_SmallerThan() || Compare == ord_Equal()) {
	      term_Delete(Max);
	      term_Delete(LitTerm);
	      return FALSE;
      }
                 
      term_Delete(LitTerm);
    }
  }
  
  term_Delete(Max);
  return TRUE;
}

static BOOL inf_OrdChLiteralsMax(CLAUSE Clause, int i, SUBST Subst,
                                 CLAUSE PartnerClause, int j, SUBST PartnerSubst,
                                 BOOL RightArg, FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT: The parents of a chaining inference, the respective
         literal indices and substitutions, a flag store and 
	       a precedence.
  RETURNS: TRUE iff the literals are "maximal" in the instantiated
           clauses.
***************************************************************/
{
#ifdef CHECK
  if (clause_GetFlag(Clause, CLAUSESELECT)
      || clause_GetFlag(PartnerClause, CLAUSESELECT)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_OrdChLiteralsMax: Some literal is selected.");
    misc_FinishErrorReport();
  }
#endif

  if (! inf_OrdChLitMax(Clause, i, Subst, RightArg, Flags, Precedence)) {
    return FALSE;
  }
  if (! inf_OrdChLitMax(PartnerClause, j, PartnerSubst, !RightArg, Flags, Precedence)) {
    return FALSE;
  }
  return TRUE;
}

static CLAUSE inf_ApplyOrdCh(LITERAL PartnerLit, LITERAL GivenLit,
                             SUBST PartnerSubst, SUBST GivenSubst,
                             TERM QueryAtom, CLAUSE GivenClause, 
                             BOOL RightArg, SYMBOL NewTopSymbol,
                             PROOFSEARCH Search, FLAGSTORE Flags, PRECEDENCE Precedence)
{
  CLAUSE NewClause, GivenCopyClause, PartnerClause;
  TERM ChTerm1, ChTerm2;
  int    i,j,lc,la,ls,pi,pls,pla,plc,help; /* p=Partner,l=last */
  int    axClNo;

  PartnerClause   = clause_LiteralOwningClause(PartnerLit);
  GivenCopyClause = clause_LiteralOwningClause(GivenLit);

  pls = clause_LastSuccedentLitIndex(PartnerClause);
  pla = clause_LastAntecedentLitIndex(PartnerClause);
  plc = clause_LastConstraintLitIndex(PartnerClause);

  pi  = clause_LiteralGetIndex(PartnerLit);

  ls  = clause_LastSuccedentLitIndex(GivenCopyClause);
  la  = clause_LastAntecedentLitIndex(GivenCopyClause);
  lc  = clause_LastConstraintLitIndex(GivenCopyClause);

  i   = clause_LiteralGetIndex(GivenLit);

  NewClause = clause_CreateBody(clause_Length(GivenCopyClause) + clause_Length(PartnerClause) - 1);

  clause_SetNumOfConsLits(NewClause, clause_NumOfConsLits(GivenCopyClause) + clause_NumOfConsLits(PartnerClause));
  
  clause_SetNumOfAnteLits(NewClause, clause_NumOfAnteLits(GivenCopyClause) + clause_NumOfAnteLits(PartnerClause));
  
  clause_SetNumOfSuccLits(NewClause, clause_NumOfSuccLits(GivenCopyClause) + clause_NumOfSuccLits(PartnerClause) - 1);

  /* First set the literals from the GivenCopyClause : */
  for (j = clause_FirstLitIndex(); j <= lc; j++) {
    clause_SetLiteral(NewClause, j, 
           clause_LiteralCreate(subst_Apply(GivenSubst, term_Copy(clause_GetLiteralTerm(GivenCopyClause, j))),NewClause));
  }

  /* help = number of literals to leave empty */
  help = clause_NumOfConsLits(PartnerClause);
    
  for ( ; j <= la; j++) {
    clause_SetLiteral(NewClause, (j + help),
      clause_LiteralCreate(subst_Apply(GivenSubst, term_Copy(clause_GetLiteralTerm(GivenCopyClause, j))),NewClause));
  }

  /* help = number of literals to leave empty */
  help += clause_NumOfAnteLits(PartnerClause);

  for ( ; j <= ls; j++) {
    if (j != i) {
      /* The ActLit isn't copied! */
      clause_SetLiteral(NewClause, (j + help), 
	      clause_LiteralCreate(subst_Apply(GivenSubst, term_Copy(clause_GetLiteralTerm(GivenCopyClause, j))),NewClause));
    } else {
      /*the index has to be decreased to avoid an empty literal! */
      help--;
    }
  }

  /* Now we consider the PartnerClause : */

  /* help = number of already set constraint (GivenCopyClause-) literals */
  help = clause_NumOfConsLits(GivenCopyClause);

  for (j = clause_FirstLitIndex(); j <= plc; j++) {
    clause_SetLiteral(NewClause, (j + help),
      clause_LiteralCreate(subst_Apply(PartnerSubst, term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));
  }

  /* help = number of already set constraint and antecedent Given-literals */
  help += clause_NumOfAnteLits(GivenCopyClause);

  for ( ; j <= pla; j++) {
    clause_SetLiteral(NewClause, (j + help),
      clause_LiteralCreate(subst_Apply(PartnerSubst, term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));
  }

  /* help = number of already set (GivenCopyClause-) literals */
  help += clause_NumOfSuccLits(GivenCopyClause) - 1;

  for ( ; j <= pls; j++) {
    if (j != pi) {
      clause_SetLiteral(NewClause, (j + help), 
        clause_LiteralCreate(subst_Apply(PartnerSubst,term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));
    } else {
      /* The index has to be shifted as above.  */
      help--;
    }
  } /* end of NewClause creation (last for loop). */

  j += help;

  if (RightArg) {
    ChTerm1 = term_Copy(term_FirstArgument(clause_GetLiteralTerm(GivenCopyClause, i)));
    ChTerm2 = term_Copy(term_SecondArgument(QueryAtom));
  } else {
    ChTerm1 = term_Copy(term_FirstArgument(QueryAtom));
    ChTerm2 = term_Copy(term_SecondArgument(clause_GetLiteralTerm(GivenCopyClause, i)));
  }

  clause_SetLiteral(NewClause, j,
    clause_LiteralCreate(
      subst_Apply(GivenSubst,term_Create(
        NewTopSymbol,list_Cons(ChTerm1,list_List(ChTerm2)))),
    NewClause));

  clause_SetDataFromParents(NewClause,PartnerClause,pi,GivenClause,i,
          Flags, Precedence);
  
  axClNo = inf_GetAxClNo(Search,term_TopSymbol(clause_LiteralAtom(GivenLit))); /* The transitivity axiom */
  clause_AddParentClause(NewClause, axClNo); 
  clause_AddParentLiteral(NewClause,0); 
  clause_AddParentClause(NewClause, axClNo); 
  clause_AddParentLiteral(NewClause,1); 

  clause_SetFromOrderedChaining(NewClause);

  return(NewClause);
}
  
LIST inf_OrderedChaining(CLAUSE GivenClause, PROOFSEARCH Search, SHARED_INDEX ShIndex,
			   FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a proofsearch object, an Index, usually the WorkedOffIndex,
           a flag store and a precedence.
  RETURNS: A list of clauses derivable from the GivenClause by 
           OrderedChaining wrt. the Index.
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  CLAUSE GivenCopy;
  LIST Result;
  LITERAL ActLit;
  LITERAL ActLitGiven;
  TERM Atom;
  TERM QueryAtom;
  SYMBOL SpecSymbol;
  int i,n;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_OrderedChaining: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  if (!clause_HasSolvedConstraint(GivenClause)
      || clause_GetFlag(GivenClause,CLAUSESELECT))
    return list_Nil();
  
  Result = list_Nil();
  GivenCopy = (CLAUSE) NULL;
  QueryAtom = (TERM) NULL;
  
  n = clause_LastSuccedentLitIndex(GivenClause);
  
  for (i = clause_FirstSuccedentLitIndex(GivenClause); i <= n; i++) {
    
    LIST TermList;    
    NAT RightArg;
    ord_RESULT Compare;

    ActLitGiven = clause_GetLiteral(GivenClause, i);
    SpecSymbol = term_TopSymbol(clause_LiteralAtom(ActLitGiven));
        
    if (clause_LiteralGetFlag(ActLitGiven,STRICTMAXIMAL)
        && symbol_HasProperty(SpecSymbol, TRANSITIVE)) {

      if (GivenCopy == NULL) {
        SYMBOL NewVar;
        
        GivenCopy = clause_Copy(GivenClause);
        symbol_SetStandardVarCounter(clause_MaxVar(GivenCopy));
        NewVar = symbol_CreateStandardVariable();
        /* 
          QueryAtom will be used to fetch partner literals from the index.
          The equality at the top will always be replaced by a concrete special predicate,
          exactly one of left/right term of the predicate will be 
          assigned some term from GivenCopy,
          and the other term will be a "fresh" variable used
          as "wildcard" (Prolog "_") in unification.
        */
        QueryAtom = term_Create(SpecSymbol,
                        list_Cons(term_Create(NewVar, list_Nil()),
                          list_Cons(term_Create(NewVar, list_Nil()),
                          list_Nil())));
      }

      ActLit    = clause_GetLiteral(GivenCopy, i);
      Atom      = clause_LiteralAtom(ActLit);
      Compare   = clause_LiteralGetOrderStatus(ActLit);

      term_RplacTop(QueryAtom, SpecSymbol); /* replacing is necessary if more different specials are involved */

      for (RightArg = 0; RightArg <= 1; RightArg++) {
        /* RightArg means that the right argument of the special predicate from Atom
           is going to be unified with left argument from elsewhere (WorkedOff). */
        /* This also means that RightArg <=> term_TopSymbol(Atom) == S_from_the_paper. */
        if (RightArg) { 

          /* given: Atom == S(s,t); QueryAtom == X(x,u)
             if t <(=) s:
               replace QueryAtom by X(x,x)
               and start new iteration
             otherwise:
               replace QueryAtom by X(t,x) */

          term_Delete(term_SecondArgument(QueryAtom));
          if (Compare == ord_GreaterThan() || (Compare == ord_Equal()) ) {
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_Copy(term_FirstArgument(QueryAtom)));
            continue;
          } else {
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_FirstArgument(QueryAtom));
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_Copy(term_SecondArgument(Atom)));
          }
        } else {
          /* given: Atom == S(s,t); QueryAtom == X(u,x)
             if s <= t:
               replace QueryAtom by X(x,x)
               and start new iteration
             otherwise:
               replace QueryAtom by X(x,s) */

          term_Delete(term_FirstArgument(QueryAtom));
          if ( Compare == ord_SmallerThan() || Compare == ord_Equal()) {
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_Copy(term_SecondArgument(QueryAtom)));
            continue;
          } else {
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_SecondArgument(QueryAtom));
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_Copy(term_FirstArgument(Atom)));
          }
        }
                  
        TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
                                 cont_RightContext(), QueryAtom);
  
        for ( ; !list_Empty(TermList); TermList = list_Pop(TermList)) {
          LIST       LitList;
          TERM       PartnerAtom;
          LITERAL    PartnerLit;
          CLAUSE     PartnerClause;
          int        j;
          ord_RESULT ComparePartner;
    
          PartnerAtom = list_First(TermList);            
          
          if (term_IsVariable(PartnerAtom)) 
            continue;

          for (LitList = sharing_NAtomDataList(PartnerAtom); !list_Empty(LitList); LitList = list_Cdr(LitList)) {
            PartnerLit    = list_Car(LitList);
            j             = clause_LiteralGetIndex(PartnerLit);
            PartnerClause = clause_LiteralOwningClause(PartnerLit);
            
            ComparePartner = clause_LiteralGetOrderStatus(PartnerLit);
            
            if (RightArg ? (ComparePartner == ord_SmallerThan() || ComparePartner == ord_Equal())
                         : (ComparePartner == ord_GreaterThan() || (ComparePartner == ord_Equal())))
              break; /* The first clause was as good as any to tell us (via PartnerLit's order status) 
                        that the two terms (left, right) are incopatible with our inference */
            
            if (clause_LiteralIsPositive(PartnerLit) &&
                clause_HasSolvedConstraint(PartnerClause) &&
                !clause_GetFlag(PartnerClause,CLAUSESELECT) &&                  
                clause_LiteralGetFlag(PartnerLit,STRICTMAXIMAL)) {
              SUBST  Subst, PartnerSubst;
              SYMBOL MaxVar;
              
              MaxVar = clause_MaxVar(PartnerClause);
              if (MaxVar != symbol_GetInitialStandardVarCounter()) { /* renaming the query atom and given copy clause variables */
                int k,len;
                
                len = clause_Length(GivenCopy);
                
                term_StartMaxRenaming(MaxVar);
                term_Rename(QueryAtom);
                
                for (k = clause_FirstLitIndex(); k < len; k++)
                  term_Rename(clause_GetLiteralTerm(GivenCopy, k));
              }

              cont_Check(); 
              if (!unify_UnifyNoOC(cont_LeftContext(), QueryAtom, cont_RightContext(), PartnerAtom)) {                  
                misc_StartErrorReport();
                misc_ErrorReport("\n In inf_OrderedChaining: Unification failed.");
                misc_FinishErrorReport();
              }
              subst_ExtractUnifier(cont_LeftContext(), &Subst, cont_RightContext(), &PartnerSubst);
              cont_Reset();

              if (inf_OrdChLiteralsMax(GivenCopy, i, Subst,
                                       PartnerClause, j, PartnerSubst,
                                       RightArg, Flags, Precedence)) {
                Result = list_Cons(inf_ApplyOrdCh(PartnerLit, ActLit, PartnerSubst, Subst, 
                                     QueryAtom, GivenClause,
                                     RightArg, SpecSymbol, Search, Flags, Precedence),
                           Result);
              }
              subst_Delete(Subst);
              subst_Delete(PartnerSubst);
            }
          } /* end of for (LitList = sharing_NAtomDataList ...). */
        } /* end of for (TermList = st_GetUnifier...). */
      } /* end of for (RightArg = 0/1). */
    } /* end of if (clause_LiteralGetFlag(ActLitGiven,STRICTMAXIMAL) && ...). */
  } /* end of for 'all succedent literals'. */
  
  if (GivenCopy != NULL) {
    clause_Delete(GivenCopy);
    term_Delete(QueryAtom);
  }

/*
  if (list_Exist(Result)) {
    LIST results = Result;    
    printf("\n We have had a chaining inference(s):\n");
    while (list_Exist(results)) {
      clause_Print(list_Car(results));
      printf("\n");
      results = list_Cdr(results);
    }
  }
*/
  return Result;
}

static BOOL inf_NegChLitMax(CLAUSE Clause, int i, SUBST Subst,
                            BOOL RightArg, BOOL Strict,
                            FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, the index of a strictly maximal literal,
           a substitution, two boolean flags, a flag store and a precedence.
  RETURNS: The function returns TRUE iff the
           literal at index <i> is still strictly maximal in the 
	         instantiated clause and the right (of left) argument
           of the literal is (strictly) maximal in the literal.
  CAUTION: DON'T call this function with a clause with selected
           literals!
***************************************************************/
{
  TERM       Max, MaxAtom, LitTerm;
  LITERAL    Lit, Other;
  ord_RESULT Compare;
  int        k;
  BOOL MildTest;
  MAXFLAG MaxFlag;
  int LastLitIndex;
     
  LastLitIndex = clause_LastSuccedentLitIndex(Clause);
     
  MildTest = RightArg && !Strict;
  MaxFlag = Strict ? STRICTMAXIMAL : MAXIMAL;
  
  Max = (TERM) NULL;
  LitTerm = (TERM) NULL;
  Lit = clause_GetLiteral(Clause, i);

#ifdef CHECK
  if (clause_GetFlag(Clause, CLAUSESELECT) && !clause_LiteralGetFlag(Lit,LITSELECT)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_NegChLitMax: Selected literal not used.");
    misc_FinishErrorReport();
  }

  if (!clause_GetFlag(Clause, CLAUSESELECT) && !clause_LiteralGetFlag(Lit, MaxFlag)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_NegChLitMax: Maximality criterium for literal %d not satisfied.", i);
    misc_FinishErrorReport();
  }

  if (i < clause_FirstAntecedentLitIndex(Clause) ||
      i > LastLitIndex) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_NegChLitMax: Literal index %d is out of range.", i);
    misc_FinishErrorReport();
  }
#endif

  Max = subst_Apply(Subst,term_Copy(clause_GetLiteralTerm(Clause,i)));

  if (clause_LiteralIsNegative(Lit))
    MaxAtom = term_FirstArgument(Max);
  else
    MaxAtom = Max;
  
  Compare = ord_Compare(term_FirstArgument(MaxAtom),
      term_SecondArgument(MaxAtom),
      Flags,Precedence);

  if ( (Compare == ord_Equal() && !MildTest) ||
       (RightArg ? (Compare == ord_GreaterThan())
                 : (Compare == ord_SmallerThan())) ) {
    term_Delete(Max);
    return FALSE;
  }

  /* Only antecedent and succedent literals until LastLitIndex are      */
  /* compared, so if there's only one such literal, it's maximal.       */
  if (clause_FirstAntecedentLitIndex(Clause) == LastLitIndex ||
      clause_GetFlag(Clause, CLAUSESELECT) || /* this implies the literal is selected and so no maximality check will be done*/
      subst_Empty(Subst) ) {                  /* precomputed maximality will not get changed */      
    term_Delete(Max);
    return TRUE;
  }

  for (k = clause_FirstAntecedentLitIndex(Clause); k <= LastLitIndex; k++) {
    Other = clause_GetLiteral(Clause, k);
    if (k != i && clause_LiteralGetFlag(Other,MaxFlag)) {
      /* Only compare with (strictly) maximal literals, since for every other */
      /* there's at least one maximal literal, that is bigger (or equal).*/
      LitTerm = subst_Apply(Subst,term_Copy(clause_GetLiteralTerm(Clause,k)));
      Compare = ord_LiteralCompare(Max,				   
           clause_LiteralGetOrderStatus(Lit),
				   LitTerm,				   
           clause_LiteralGetOrderStatus(Other),
				   TRUE, Flags, Precedence);
         
      if (Compare == ord_SmallerThan() || (Compare == ord_Equal() && Strict)) {
	      term_Delete(Max);
	      term_Delete(LitTerm);
	      return FALSE;
      }
                 
      term_Delete(LitTerm);
    }
  }
  
  term_Delete(Max);
  return TRUE;
}

static BOOL inf_NegChLiteralsMax(CLAUSE Clause, int i, SUBST Subst,
                                 CLAUSE PartnerClause, int j, SUBST PartnerSubst,
                                 BOOL RightArg, BOOL ActLitPositive,
                                 FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT: The parents of a chaining inference, the respective
         literal indices and substitutions, a flag store and 
	       a precedence.
  RETURNS: TRUE iff the literals are "maximal" in the instantiated
           clauses.
***************************************************************/
{
  if (! inf_NegChLitMax(Clause,i,Subst,
                        RightArg, ActLitPositive, Flags, Precedence)) {
    return FALSE;
  }
  if (! inf_NegChLitMax(PartnerClause,j,PartnerSubst,
                        RightArg, !ActLitPositive, Flags, Precedence)) {
    return FALSE;
  }
  return TRUE;
}

static CLAUSE inf_ApplyNegCh(LITERAL PartnerLit, LITERAL GivenLit,
                             SUBST PartnerSubst, SUBST GivenSubst,
                             TERM QueryAtom, CLAUSE GivenClause,
                             BOOL RightArg, BOOL ActLitPositive, SYMBOL NewTopSymbol,
                             PROOFSEARCH Search, FLAGSTORE Flags, PRECEDENCE Precedence)
{ 
  CLAUSE NewClause, GivenCopyClause, PartnerClause;
  TERM   ChTerm1, ChTerm2;
  int    i,j,lc,la,ls,pi,pls,pla,plc,idx,target; /* p=Partner,l=last */
  int    axClNo;

  PartnerClause   = clause_LiteralOwningClause(PartnerLit);
  GivenCopyClause = clause_LiteralOwningClause(GivenLit);

  pls = clause_LastSuccedentLitIndex(PartnerClause);
  pla = clause_LastAntecedentLitIndex(PartnerClause);
  plc = clause_LastConstraintLitIndex(PartnerClause);

  pi  = clause_LiteralGetIndex(PartnerLit);

  ls  = clause_LastSuccedentLitIndex(GivenCopyClause);
  la  = clause_LastAntecedentLitIndex(GivenCopyClause);
  lc  = clause_LastConstraintLitIndex(GivenCopyClause);

  i   = clause_LiteralGetIndex(GivenLit);
  
  NewClause = clause_CreateBody(clause_Length(GivenCopyClause) + clause_Length(PartnerClause) - 1);

  clause_SetNumOfConsLits(NewClause, clause_NumOfConsLits(GivenCopyClause) + clause_NumOfConsLits(PartnerClause));
  
  clause_SetNumOfAnteLits(NewClause, clause_NumOfAnteLits(GivenCopyClause) + clause_NumOfAnteLits(PartnerClause));
  
  clause_SetNumOfSuccLits(NewClause, clause_NumOfSuccLits(GivenCopyClause) + clause_NumOfSuccLits(PartnerClause) - 1);

  /* idx = index to place the next literal in the new clause */
  idx = clause_FirstLitIndex();
  
  /* target = index where the brand new literal will be placed*/
  target = -1;
  
  /* Constr literals from the GivenCopyClause : */
  for (j = clause_FirstConstraintLitIndex(GivenCopyClause); j <= lc; j++) {
    clause_SetLiteral(NewClause, idx++, 
           clause_LiteralCreate(subst_Apply(GivenSubst, term_Copy(clause_GetLiteralTerm(GivenCopyClause, j))),NewClause));
  }
  
  /* Constr literals from the PartnerClause : */
  for (j = clause_FirstConstraintLitIndex(PartnerClause); j <= plc; j++) {
    clause_SetLiteral(NewClause, idx++,
      clause_LiteralCreate(subst_Apply(PartnerSubst, term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));
  }
  
  /* Ante literals from the GivenCopyClause : */
  for (j = clause_FirstAntecedentLitIndex(GivenCopyClause); j <= la; j++) {
    if (j != i) {      
      clause_SetLiteral(NewClause, idx++, 
	      clause_LiteralCreate(subst_Apply(GivenSubst, term_Copy(clause_GetLiteralTerm(GivenCopyClause, j))),NewClause));
    } else {
      target = idx++; /*leaving a hole for target here*/
    }
  }
  
  /* Ante literals from the PartnerClause : */
    for (j = clause_FirstAntecedentLitIndex(PartnerClause); j <= pla; j++) {
    if (j != pi) {
      clause_SetLiteral(NewClause, idx++,
        clause_LiteralCreate(subst_Apply(PartnerSubst, term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));
    } else {
      target = idx++; /*leaving a hole for target here*/
    }
  }

  /* Succ literals from the GivenCopyClause : */
  for ( j = clause_FirstSuccedentLitIndex(GivenCopyClause); j <= ls; j++) {
    if (j != i) {
      clause_SetLiteral(NewClause, idx++, 
	      clause_LiteralCreate(subst_Apply(GivenSubst, term_Copy(clause_GetLiteralTerm(GivenCopyClause, j))),NewClause));
    }
  }

  /* Succ literals from the PartnerClause : */
  for (j = clause_FirstSuccedentLitIndex(PartnerClause); j <= pls; j++) {
    if (j != pi) {
      clause_SetLiteral(NewClause, idx++,
        clause_LiteralCreate(subst_Apply(PartnerSubst,term_Copy(clause_GetLiteralTerm(PartnerClause, j))),NewClause));
    }
  }
     
  if (RightArg) {  
    if (ActLitPositive) {
      ChTerm1 = term_Copy(term_FirstArgument(QueryAtom));
      ChTerm2 = term_Copy(term_FirstArgument(clause_GetLiteralAtom(GivenCopyClause, i)));      
    } else {
      ChTerm1 = term_Copy(term_FirstArgument(clause_GetLiteralAtom(GivenCopyClause, i)));
      ChTerm2 = term_Copy(term_FirstArgument(QueryAtom));
    }
  } else {
    if (ActLitPositive) {
      ChTerm1 = term_Copy(term_SecondArgument(clause_GetLiteralAtom(GivenCopyClause, i)));
      ChTerm2 = term_Copy(term_SecondArgument(QueryAtom));      
    } else {
      ChTerm1 = term_Copy(term_SecondArgument(QueryAtom));
      ChTerm2 = term_Copy(term_SecondArgument(clause_GetLiteralAtom(GivenCopyClause, i)));
    }    
  }
  
  clause_SetLiteral(NewClause, target,
    clause_LiteralCreate(
      term_Create(fol_Not(), list_List(
        subst_Apply(GivenSubst,term_Create(
          NewTopSymbol,list_Cons(ChTerm1,list_List(ChTerm2)))))),
    NewClause));

  clause_SetDataFromParents(NewClause,PartnerClause,pi,GivenClause,i,
          Flags, Precedence);
          
  axClNo = inf_GetAxClNo(Search,term_TopSymbol(clause_LiteralAtom(GivenLit))); /* The transitivity axiom */
  clause_AddParentClause(NewClause, axClNo);
  clause_AddParentLiteral(NewClause, RightArg);
  clause_AddParentClause(NewClause, axClNo);
  clause_AddParentLiteral(NewClause, 2);
      
  clause_SetFromNegativeChaining(NewClause);  

  return(NewClause);
}

LIST inf_NegativeChaining(CLAUSE GivenClause, PROOFSEARCH Search, SHARED_INDEX ShIndex,
			   FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a proofsearch object, an Index, usually the WorkedOffIndex,
           a flag store and a precedence.
  RETURNS: A list of clauses derivable from the GivenClause by 
           NegativeChaining wrt. the Index.
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  CLAUSE GivenCopy;
  LIST Result;
  LITERAL ActLit;
  LITERAL ActLitGiven;
  TERM Atom;
  TERM QueryAtom;
  SYMBOL SpecSymbol;
  int i,n,m;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_NegativeChaining: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  if (!clause_HasSolvedConstraint(GivenClause))
    return list_Nil();
  
  Result = list_Nil();
  GivenCopy = (CLAUSE) NULL;
  QueryAtom = (TERM) NULL;  
  
  n = clause_LastSuccedentLitIndex(GivenClause);

  if (clause_GetFlag(GivenClause,CLAUSESELECT))
    m = clause_LastAntecedentLitIndex(GivenClause);
  else
    m = clause_LastSuccedentLitIndex(GivenClause);

  for (i = clause_FirstAntecedentLitIndex(GivenClause); i <= m; i++) {
    /* In general both positive and negative literals !!! */
    
    LIST TermList;    
    NAT RightArg;
    ord_RESULT Compare;
    BOOL ActLitPositive;

    ActLitGiven = clause_GetLiteral(GivenClause, i);
    SpecSymbol = term_TopSymbol(clause_LiteralAtom(ActLitGiven));
    ActLitPositive = clause_LiteralIsPositive(ActLitGiven);
        
    if ( symbol_HasProperty(SpecSymbol, TRANSITIVE) &&
         ( clause_LiteralGetFlag(ActLitGiven,LITSELECT) ||
           (!clause_GetFlag(GivenClause,CLAUSESELECT) &&
                  ( ActLitPositive ? clause_LiteralGetFlag(ActLitGiven,STRICTMAXIMAL) :
                                     clause_LiteralIsMaximal(ActLitGiven))))) {

      if (GivenCopy == NULL) {
        SYMBOL NewVar;
        
        GivenCopy = clause_Copy(GivenClause);
        symbol_SetStandardVarCounter(clause_MaxVar(GivenCopy));
        NewVar = symbol_CreateStandardVariable();
        /* 
          QueryAtom will be used to fetch partner literals from the index.
          The equality at the top will always be replaced by a concrete special predicate,
          exactly one of left/right term of the predicate will be 
          assigned some term from GivenCopy,
          and the other term will be a "fresh" variable used
          as "wildcard" (Prolog "_") in unification.
        */
        QueryAtom = term_Create(SpecSymbol,
                        list_Cons(term_Create(NewVar, list_Nil()),
                          list_Cons(term_Create(NewVar, list_Nil()),
                          list_Nil())));
      }
            
      ActLit    = clause_GetLiteral(GivenCopy, i);
      Atom      = clause_LiteralAtom(ActLit);
      Compare   = clause_LiteralGetOrderStatus(ActLit);

      term_RplacTop(QueryAtom, SpecSymbol); /* replacing is necessary if more different specials are involved */

      for (RightArg = 0; RightArg <= 1; RightArg++) {

        /* RightArg means that the right argument of the special predicate from Atom
          is going to be unified with right argument from elsewhere (WorkedOff). */
        if (RightArg) {

          /* given: Atom == S/T(s,t); QueryAtom == T/S(u,x)
             if t <= s:
               replace QueryAtom by T/S(x,x)
               and start new iteration
             otherwise:
               replace QueryAtom by T/S(x,t) */

          term_Delete(term_FirstArgument(QueryAtom));
          if (Compare == ord_GreaterThan() || (Compare == ord_Equal() && ActLitPositive) ) {
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_Copy(term_SecondArgument(QueryAtom)));
            continue;
          } else {
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_SecondArgument(QueryAtom));
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_Copy(term_SecondArgument(Atom)));              
          }
        } else {
          /* given: Atom == S/T(s,t); QueryAtom == T/S(x,u)
             if s <= t:
               replace QueryAtom by T/S(x,x)
               and start new iteration
             otherwise:
               replace QueryAtom by T/S(s,x) */

          term_Delete(term_SecondArgument(QueryAtom));
          if ( Compare == ord_SmallerThan() || Compare == ord_Equal()) {
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_Copy(term_FirstArgument(QueryAtom)));              
            continue;
          } else {
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_FirstArgument(QueryAtom));
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_Copy(term_FirstArgument(Atom)));
          }
        }

        TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
                                 cont_RightContext(), QueryAtom);
  
        for ( ; !list_Empty(TermList); TermList = list_Pop(TermList)) {
          LIST       LitList;
          TERM       PartnerAtom;
          LITERAL    PartnerLit;
          CLAUSE     PartnerClause;
          int        j;
          ord_RESULT ComparePartner;
    
          PartnerAtom = list_First(TermList);
                                          
          if (term_IsVariable(PartnerAtom))
            continue;
          
          for (LitList = sharing_NAtomDataList(PartnerAtom); !list_Empty(LitList); LitList = list_Cdr(LitList)) {
            PartnerLit    = list_Car(LitList);
            j             = clause_LiteralGetIndex(PartnerLit);
            PartnerClause = clause_LiteralOwningClause(PartnerLit);
            
            ComparePartner = clause_LiteralGetOrderStatus(PartnerLit);
            
            if (RightArg ? (ComparePartner == ord_GreaterThan() || (ComparePartner == ord_Equal() && !ActLitPositive))
                         : (ComparePartner == ord_SmallerThan() || ComparePartner == ord_Equal()))
              break; /* The first clause was as good as any to tell us (via PartnerLit's order status) 
                        that the two terms (left, right) are incopatible with our inference */
            
            if ( (clause_LiteralIsPositive(PartnerLit) ^ ActLitPositive) && /* they have different signs */
                clause_HasSolvedConstraint(PartnerClause) &&
                ( clause_LiteralGetFlag(PartnerLit,LITSELECT) ||
                 (!clause_GetFlag(PartnerClause,CLAUSESELECT) &&
                  (ActLitPositive ? clause_LiteralIsMaximal(PartnerLit) :
                                    clause_LiteralGetFlag(PartnerLit,STRICTMAXIMAL))))) {
              SUBST  Subst, PartnerSubst;
              SYMBOL MaxVar;
              
              MaxVar = clause_MaxVar(PartnerClause);
              if (MaxVar != symbol_GetInitialStandardVarCounter()) { /* renaming the query atom and given copy clause variables */
                int k,len;
                
                len = clause_Length(GivenCopy);
                
                term_StartMaxRenaming(MaxVar);
                term_Rename(QueryAtom);
                
                for (k = clause_FirstLitIndex(); k < len; k++)
                  term_Rename(clause_GetLiteralTerm(GivenCopy, k));
              }
              
              cont_Check(); 
              if (!unify_UnifyNoOC(cont_LeftContext(), QueryAtom, cont_RightContext(), PartnerAtom)) {                  
                misc_StartErrorReport();
                misc_ErrorReport("\n In inf_NegativeChaining: Unification failed.");
                misc_FinishErrorReport();
              }
              subst_ExtractUnifier(cont_LeftContext(), &Subst, cont_RightContext(), &PartnerSubst);
              cont_Reset();               
              
              if (inf_NegChLiteralsMax(GivenCopy, i, Subst,
                                       PartnerClause, j, PartnerSubst,
                                       RightArg, ActLitPositive,                                         
                                       Flags, Precedence)) {
                
                Result = list_Cons(inf_ApplyNegCh(PartnerLit, ActLit, PartnerSubst, Subst, QueryAtom, GivenClause,
                                     RightArg, ActLitPositive, SpecSymbol, Search, Flags, Precedence),
                           Result);
                           
                /*This is the general negative chaining version of the rule for the case where the conclusion contains the equality predicate*/
                if (ana_GetFeature(prfs_GetAnalyze(Search),ana_EQUATIONS)) {
                  Result = list_Cons(inf_ApplyNegCh(PartnerLit, ActLit, PartnerSubst, Subst, QueryAtom, GivenClause,
                                     RightArg, ActLitPositive, fol_Equality(), Search, Flags, Precedence),
                            Result);
                }

              }
              subst_Delete(Subst);
              subst_Delete(PartnerSubst);
            }
          } /* end of for (LitList = sharing_NAtomDataList ...). */
        } /* end of for (TermList = st_GetUnifier...). */
      } /* end of for (RightArg = 0/1). */
    } /* end of if ( ... clause_LiteralGetFlag(ActLitGiven,STRICTMAXIMAL) ... ). */
  } /* end of for 'all literals'. */
  
  if (GivenCopy != NULL) {
    clause_Delete(GivenCopy);
    term_Delete(QueryAtom);
  }

/*
  if (list_Exist(Result)) {
    LIST results = Result;    
    printf("\n We have had a negative chaining inference(s)\n");
    while (list_Exist(results)) {
      clause_Print(list_Car(results));
      printf("\n");
      results = list_Cdr(results);
    }
  }
*/
  return Result;
}

static BOOL inf_CompResLiteralsMax(CLAUSE UnorderedClause, int i, SUBST USubst,
                                   CLAUSE OrderedClause, int j, SUBST OSubst,
                                   BOOL RightArg,
                                   FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT: The parents of a composition resolution inference, the respective
         literal indices, and substitutions,
         a flag store and a precedence.
  RETURNS: TRUE iff the all the required ordering constraints 
           are satisfied when the substitutions are applied.
***************************************************************/
{
  TERM       UMax;
  TERM       OMax;
  TERM       LitTerm;
  LITERAL    ULit;
  LITERAL    OLit;
  LITERAL    Other;
  ord_RESULT Compare;
  int        k;
  int        ULast, OLast;
  
  ULast = clause_LastSuccedentLitIndex(UnorderedClause);
  OLast = clause_LastSuccedentLitIndex(OrderedClause);
  
#ifdef CHECK
  if (! clause_LiteralGetFlag(clause_GetLiteral(UnorderedClause, i), STRICTMAXIMAL) || 
      ! clause_LiteralGetFlag(clause_GetLiteral(OrderedClause, j), STRICTMAXIMAL)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_CompResLiteralsMax: Maximality criterium for a literal not satisfied.");
    misc_FinishErrorReport();
  }
  if (i < clause_FirstSuccedentLitIndex(UnorderedClause) || i > ULast || 
      j < clause_FirstSuccedentLitIndex(OrderedClause) || j > OLast) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_CompResLiteralsMax: Literal index is out of range.");
    misc_FinishErrorReport();
  }
  if (clause_GetFlag(UnorderedClause, CLAUSESELECT)
     || clause_GetFlag(OrderedClause, CLAUSESELECT)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_CompResLiteralsMax: There're selected literals.");
    misc_FinishErrorReport();
  }    
#endif
  
  OMax = subst_Apply(OSubst,term_Copy(clause_GetLiteralAtom(OrderedClause,j)));

  Compare = ord_Compare(term_FirstArgument(OMax),
                        term_SecondArgument(OMax),
                        Flags,Precedence);

  if ( Compare == ord_Equal() ||
       (RightArg ? (Compare == ord_GreaterThan())
                 : (Compare == ord_SmallerThan())) ) {
    term_Delete(OMax);
    return FALSE;
  }

  UMax = subst_Apply(USubst,term_Copy(clause_GetLiteralAtom(UnorderedClause,i)));
  
  OLit = clause_GetLiteral(OrderedClause,j);
  ULit = clause_GetLiteral(UnorderedClause,i);
  
  Compare = ord_LiteralCompare(UMax,				    
            clause_LiteralGetOrderStatus(ULit),
				    OMax,				    
            clause_LiteralGetOrderStatus(OLit),
				    TRUE, Flags, Precedence);
  
  if (Compare == ord_GreaterThan()) {
    term_Delete(OMax);
    term_Delete(UMax);
    return FALSE;
  }
      
  /* Only antecedent and succedent literals until LastIndex are      
     compared, so if there's only one such literal, it's maximal.    
     If the substitution is empty
     the necessary condition already tested is sufficient, too.   */
  if (clause_FirstAntecedentLitIndex(UnorderedClause) != ULast && !subst_Empty(USubst)) {
  
    /*we have to do the proper test for UnorderedClause*/
    
    for (k = clause_FirstAntecedentLitIndex(UnorderedClause); k <= ULast; k++) {
      Other = clause_GetLiteral(UnorderedClause, k); 
      
      if (k != i && clause_LiteralGetFlag(Other,STRICTMAXIMAL)) {
        /* Only compare with strictly maximal literals, since for every other */
        /* there's at least one maximal literal, that is bigger or equal.*/
        LitTerm = subst_Apply(USubst,term_Copy(clause_GetLiteralTerm(UnorderedClause,k)));
        Compare = ord_LiteralCompare(UMax,             
             clause_LiteralGetOrderStatus(ULit),
             LitTerm,             
             clause_LiteralGetOrderStatus(Other),
             TRUE, Flags, Precedence);
           
        if (Compare == ord_SmallerThan() || Compare == ord_Equal()) {
          term_Delete(OMax);
          term_Delete(UMax);
          term_Delete(LitTerm);
          return FALSE;
        }
                   
        term_Delete(LitTerm);
      }
    }          
  }
  
  /* Only antecedent and succedent literals until LastIndex are      
     compared, so if there's only one such literal, it's maximal.    
     If the substitution is empty
     the necessary condition already tested is sufficient, too.   */
  if (clause_FirstAntecedentLitIndex(OrderedClause) != OLast && !subst_Empty(OSubst)) {
  
    /*we have to do the proper test for OrderedClause*/
    
    for (k = clause_FirstAntecedentLitIndex(OrderedClause); k <= OLast; k++) {
      Other = clause_GetLiteral(OrderedClause, k);
      if (k != j && clause_LiteralGetFlag(Other,STRICTMAXIMAL)) {
        /* Only compare with strictly maximal literals, since for every other */
        /* there's at least one maximal literal, that is bigger or equal.*/
        LitTerm = subst_Apply(OSubst,term_Copy(clause_GetLiteralTerm(OrderedClause,k)));
        Compare = ord_LiteralCompare(OMax,             
             clause_LiteralGetOrderStatus(OLit),
             LitTerm,            
             clause_LiteralGetOrderStatus(Other),
             TRUE, Flags, Precedence);
           
        if (Compare == ord_SmallerThan() || Compare == ord_Equal()) {
          term_Delete(OMax);
          term_Delete(UMax);
          term_Delete(LitTerm);
          return FALSE;
        }
                   
        term_Delete(LitTerm);
      }
    }          
  }

  term_Delete(UMax);
  term_Delete(OMax);
  return TRUE;
}

static CLAUSE inf_ApplyCompRes(CLAUSE UnorderedClause, int i, SUBST USubst,
                               CLAUSE OrderedClause, int j, SUBST OSubst,
                               BOOL RightArg, SYMBOL NewTopSymbol,
                               PROOFSEARCH Search, FLAGSTORE Flags, PRECEDENCE Precedence)
{
  CLAUSE NewClause;
  int length, temp, help, idx, k, saved;
  TERM HelpTerm, NegLitTerm, NegLitAtom, PosLitTerm;
  TERM Small;
    
  length = clause_Length(OrderedClause) + 1 /* for one additional negative literal */;
  temp = length;
  NewClause = clause_CreateBody(length);

  help = clause_NumOfConsLits(OrderedClause);
  temp -= help;
  clause_SetNumOfConsLits(NewClause, help);
  
  help = clause_NumOfAnteLits(OrderedClause) + 1;
  temp -= help;
  clause_SetNumOfAnteLits(NewClause, help);
  
  clause_SetNumOfSuccLits(NewClause, temp);

  idx = clause_FirstLitIndex();
  
  /* First set the literals from the OrderedClause (except the j-th): */
  for (k = clause_FirstLitIndex(); k <= clause_LastAntecedentLitIndex(OrderedClause); k++) {
    clause_SetLiteral(NewClause, idx++,
           clause_LiteralCreate(subst_Apply(OSubst, term_Copy(clause_GetLiteralTerm(OrderedClause, k))),NewClause));
  }

  saved = idx++; /* for the new negative literal */
  
  for (k = clause_FirstSuccedentLitIndex(OrderedClause); k <= clause_LastSuccedentLitIndex(OrderedClause); k++) {
    if (k != j)
      clause_SetLiteral(NewClause, idx++,
                        clause_LiteralCreate(
                          subst_Apply(OSubst, term_Copy(clause_GetLiteralTerm(OrderedClause, k))),NewClause));
  }
  
  HelpTerm = subst_Apply(OSubst, term_Copy(clause_GetLiteralTerm(OrderedClause, j))); /* s\sigma S t\sigma */
  
  if (RightArg) {
    Small = subst_Apply(USubst,term_Copy(term_FirstArgument(clause_GetLiteralTerm(UnorderedClause, i)))); /* u\sigma */
    
    NegLitAtom = term_Create(NewTopSymbol,
                             list_Cons(term_Copy(Small),list_List(term_FirstArgument(HelpTerm)))); /* u\sigma S s\sigma */
    
    PosLitTerm = HelpTerm;
    term_RplacFirstArgument(PosLitTerm, Small); /*  u\sigma S t\sigma */
  } else {
    Small = subst_Apply(USubst,term_Copy(term_SecondArgument(clause_GetLiteralTerm(UnorderedClause, i)))); /* v\sigma */
    
    NegLitAtom = term_Create(NewTopSymbol,
                             list_Cons(term_SecondArgument(HelpTerm),list_List(term_Copy(Small)))); /* t\sigma S v\sigma */
    
    PosLitTerm = HelpTerm;
    term_RplacSecondArgument(PosLitTerm, Small); /*  s\sigma S v\sigma */  
  }
   
  /* add the true negation */
  NegLitTerm = term_Create(fol_Not(), list_List(NegLitAtom));
  
  /* the last is the positive one */
  clause_SetLiteral(NewClause, idx /*idx not needed anymore */,
                        clause_LiteralCreate(PosLitTerm,NewClause));

  clause_SetLiteral(NewClause, saved,
                        clause_LiteralCreate(NegLitTerm,NewClause));
  
  clause_SetDataFromFather(NewClause,OrderedClause, j, Flags, Precedence);

  clause_AddParentClause(NewClause,
      inf_GetAxClNo(Search, term_TopSymbol(HelpTerm)));  /* The transitivity axiom */
  clause_AddParentLiteral(NewClause,RightArg);

  clause_SetFromCompositionResolution(NewClause);
  
  return NewClause;
}

LIST inf_CompositionResolution(CLAUSE GivenClause, PROOFSEARCH Search, SHARED_INDEX ShIndex,
			   FLAGSTORE Flags, PRECEDENCE Precedence)
/**************************************************************
  INPUT:   A clause, a proofsearch object, an Index, usually the WorkedOffIndex,
           a flag store and a precedence.
  RETURNS: A list of clauses derivable from the GivenClause by 
           CompositionResolution wrt. the Index.
  MEMORY:  A list of clauses is produced, where memory for the list
           and the clauses is allocated.
***************************************************************/
{
  CLAUSE GivenCopy;
  LIST Result;
  LITERAL ActLit;
  LITERAL ActLitGiven;
  TERM Atom;
  TERM QueryAtom;
  TERM Other;
  LITERAL OtherLit;
  SYMBOL SpecSymbol;
  int i,n;

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_CompositionResolution: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  if (!clause_HasSolvedConstraint(GivenClause)
      || clause_GetFlag(GivenClause,CLAUSESELECT))
    return list_Nil();
  
  Result = list_Nil();
  GivenCopy = (CLAUSE) NULL;
  QueryAtom = (TERM) NULL;  
  
  n = clause_LastSuccedentLitIndex(GivenClause);

  for (i = clause_FirstSuccedentLitIndex(GivenClause); i <= n; i++) {
    
    LIST TermList;
    NAT RightArg;
    ord_RESULT Compare;

    ActLitGiven = clause_GetLiteral(GivenClause, i);
    SpecSymbol = term_TopSymbol(clause_LiteralAtom(ActLitGiven));
        
    if (clause_LiteralGetFlag(ActLitGiven,STRICTMAXIMAL)
        && symbol_HasProperty(SpecSymbol, TRANSITIVE)) {
        
      int k;
      BOOL ExistsOther, ExistsOtherCompatible;
      
      if (GivenCopy == NULL) {
        SYMBOL NewVar;
        
        GivenCopy = clause_Copy(GivenClause);
        symbol_SetStandardVarCounter(clause_MaxVar(GivenCopy));
        NewVar = symbol_CreateStandardVariable();
        /* 
          QueryAtom will be used to fetch partner literals from the index.
          The equality at the top will always be replaced by a concrete special predicate,
          exactly one of left/right term of the predicate will be 
          assigned some term from GivenCopy,
          and the other term will be a "fresh" variable used
          as "wildcard" (Prolog "_") in unification.
        */
        QueryAtom = term_Create(SpecSymbol,
                        list_Cons(term_Create(NewVar, list_Nil()),
                          list_Cons(term_Create(NewVar, list_Nil()),
                          list_Nil())));
      }
            
      ActLit    = clause_GetLiteral(GivenCopy, i);
      Atom      = clause_LiteralAtom(ActLit);
      /*To be only used in the second half: */
      Compare   = clause_LiteralGetOrderStatus(ActLit);

      term_RplacTop(QueryAtom, SpecSymbol); /* replacing is necessary if more different specials are involved */

      /* First half - ActLit is considered to be u'Sv' in the left clause: */

      /* Necessary condition - ( expendible ) */
      ExistsOther = FALSE;
      for (k = clause_FirstSuccedentLitIndex(GivenCopy); k <= n; k++)
        if (i != k ) {
          OtherLit = clause_GetLiteral(GivenCopy,k);
          Other = clause_LiteralAtom(OtherLit);
          if (symbol_Equal(term_TopSymbol(Other),SpecSymbol)) {
            ExistsOther = TRUE;
            break;
          }
        }
     
      if (ExistsOther) {
        for (RightArg = 0; RightArg <= 1; RightArg++) {      
          /* RightArg means that we are currently considering
             the second version of the rule in which $s\sigma \not \succeq t\sigma$ */        
          /* It also means that it will be the right arguments of the special predicate terms
             that will be unified with each other */

          if (RightArg) {
            /* given: Atom == S(u',v'); QueryAtom == S(u,x)               
               replace QueryAtom by S(x,v')                 */

            term_Delete(term_FirstArgument(QueryAtom));          
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_SecondArgument(QueryAtom));
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_Copy(term_SecondArgument(Atom)));
          } else {
            /* given: Atom == S(u',v'); QueryAtom == S(x,u)             
               replace QueryAtom by S(u',x)                 */

            term_Delete(term_SecondArgument(QueryAtom));          
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_FirstArgument(QueryAtom));
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_Copy(term_FirstArgument(Atom)));          
          }

          /* TODO - Consider:
            There seems to be another option how to implement this side of the rule.
            We could first look for the other literal in the given clause (uSv),
            look for an unifier (of the left/right sides) of the two
            and only then query the sharing for matching clauses.
                   
            The basic trade-off is greater sets of retrieved terms from workedoff (current solution)
            vs. possibly more retrievals but possibly more constrained ones (the potential option).
            
            But the other option also poses a technical problem:
             the two substitutions would probably have to be composed explicitly 
             ( as st_GetUnifier resets the context module, 
             and, moreover, there seems to be no way of renaming a clause
             - necessary step of "standardization apart" -
             that could also be reflected in the implicited substitution hidden in the context )
          */

          /* Neccessary condition - ( expendible ) */
          ExistsOtherCompatible = FALSE;
          for (k = clause_FirstSuccedentLitIndex(GivenCopy); k <= n; k++)
            if (i != k ) {
              OtherLit = clause_GetLiteral(GivenCopy,k);
              Other = clause_LiteralAtom(OtherLit);
              if (symbol_Equal(term_TopSymbol(Other),SpecSymbol)) {
                cont_Check();
          
                /* exists other with unifiable respectives */
                if (RightArg ? unify_UnifyCom(cont_LeftContext(), term_SecondArgument(Atom), term_SecondArgument(Other)) 
                             : unify_UnifyCom(cont_LeftContext(), term_FirstArgument(Atom), term_FirstArgument(Other))) {

                  /* TODO: we could also test whether the ordering condition
                   (namely the strict maximality of ActLitGiven)
                  is not violated after the unifier is applied */

                  ExistsOtherCompatible = TRUE;
                  cont_Reset();
                  break;
                }
                cont_Reset();
              }
            }

          if (!ExistsOtherCompatible) 
            continue;

          TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
                                   cont_RightContext(), QueryAtom);
            
          for ( ; !list_Empty(TermList); TermList = list_Pop(TermList)) {
            LIST       LitList;
            TERM       PartnerAtom;
            LITERAL    PartnerLit;
            CLAUSE     PartnerClause;
            int        j;
            ord_RESULT ComparePartner;
      
            PartnerAtom = list_First(TermList);            
            
            if (term_IsVariable(PartnerAtom))
              continue;

            for (LitList = sharing_NAtomDataList(PartnerAtom); !list_Empty(LitList); LitList = list_Cdr(LitList)) {
              PartnerLit    = list_Car(LitList);
              j             = clause_LiteralGetIndex(PartnerLit);
              PartnerClause = clause_LiteralOwningClause(PartnerLit);
              
              ComparePartner = clause_LiteralGetOrderStatus(PartnerLit);
              
              if (RightArg ? (ComparePartner == ord_GreaterThan() || ComparePartner == ord_Equal()) 
                           : (ComparePartner == ord_SmallerThan() || ComparePartner == ord_Equal()) )
                break; /* The first clause was as good as any to tell us (via PartnerLit's order status) 
                          that the two terms (left, right) are incopatible with our inference */

              if (ord_LiteralCompare(Atom, clause_LiteralGetOrderStatus(ActLit),
                                   PartnerAtom, clause_LiteralGetOrderStatus(PartnerLit),
                                   TRUE, Flags, Precedence) == ord_GreaterThan())
                break;

              if (clause_LiteralIsPositive(PartnerLit) &&
                  clause_HasSolvedConstraint(PartnerClause) &&
                  !clause_GetFlag(PartnerClause,CLAUSESELECT) &&                  
                  clause_LiteralGetFlag(PartnerLit,STRICTMAXIMAL)) {

                SUBST  Subst, PartnerSubst;
                SYMBOL MaxVar;

                MaxVar = clause_MaxVar(PartnerClause);                
                if (MaxVar != symbol_GetInitialStandardVarCounter()) { /* renaming the query atom and given copy clause variables */
                  int l,len;
                  
                  len = clause_Length(GivenCopy);
                  
                  term_StartMaxRenaming(MaxVar);
                  term_Rename(QueryAtom);
                  
                  for (l = clause_FirstLitIndex(); l < len; l++)
                    term_Rename(clause_GetLiteralTerm(GivenCopy, l));
                }

                cont_Check();
                if (!unify_UnifyNoOC(cont_LeftContext(), QueryAtom, cont_RightContext(), PartnerAtom)) {                  
                  misc_StartErrorReport();
                  misc_ErrorReport("\n In inf_CompositionResolution: Unification failed.");
                  misc_FinishErrorReport();
                }

                cont_StartBinding(); /*saving the current number of bindings, so that we'll be able to undo up to this point */
                
                for (k = clause_FirstSuccedentLitIndex(GivenCopy); k <= n; k++ ) 
                  if ( j != k ) {
                    OtherLit = clause_GetLiteral(GivenCopy,k);
                    Other = clause_LiteralAtom(OtherLit);
                    if (symbol_Equal(term_TopSymbol(Other),SpecSymbol) &&
                       (RightArg ? unify_ContinueToUnify(cont_LeftContext(), term_SecondArgument(QueryAtom),
                                                         cont_LeftContext(), term_SecondArgument(Other)) 
                                 : unify_ContinueToUnify(cont_LeftContext(), term_FirstArgument(QueryAtom),
                                                         cont_LeftContext(), term_FirstArgument(Other)) )) {

                      subst_ExtractUnifier(cont_LeftContext(), &Subst, cont_RightContext(), &PartnerSubst);

                      if (inf_CompResLiteralsMax(GivenCopy, i, Subst,
                                                 PartnerClause, j, PartnerSubst,
                                                 RightArg, Flags, Precedence)) {
                                                
                        Result = list_Cons(inf_ApplyCompRes(GivenCopy, k, Subst,
                                                            PartnerClause, j, PartnerSubst,
                                                            RightArg,SpecSymbol,
                                                            Search,Flags,Precedence),Result);

                        /*This is the general composition law version of the rule for the case where the conclusion contains the equality predicate*/
                        if (ana_GetFeature(prfs_GetAnalyze(Search),ana_EQUATIONS)) {
                          Result = list_Cons(inf_ApplyCompRes(GivenCopy, k, Subst,
                                                              PartnerClause, j, PartnerSubst,
                                                              RightArg,fol_Equality(),
                                                              Search,Flags,Precedence),Result);
                        }

                      }
                      subst_Delete(Subst);
                      subst_Delete(PartnerSubst);
                    }
                    cont_BackTrackAndStart();
                  }

                cont_StopAndBackTrack();
                cont_Reset();
              }
            } /* end of for (LitList = sharing_NAtomDataList ...). */
          } /* end of for (TermList = st_GetUnifier...). */                                     

        } /* end of for (RightArg = 0/1). */
      } /* end of if (ExistsOther) */
            
      /* Second half - ActLit is considered to be (sSt) in the right clause: */
      
      for (RightArg = 0; RightArg <= 1; RightArg++) {      
        /* RightArg means that we are currently considering
           the second version of the rule in which $s\sigma \not \succeq t\sigma$ */
        /* It also means that it will be the right arguments of the special predicate terms
           that will be unified with each other */
        
        if (RightArg) {
          /* given: Atom == S(s,t); QueryAtom == S(u,x)
               if s >= t:
                 replace QueryAtom by S(x,x)
                 and start new iteration
               otherwise:
                 replace QueryAtom by S(x,t) */
               
          term_Delete(term_FirstArgument(QueryAtom));          
          if (Compare == ord_GreaterThan() || Compare == ord_Equal()) {
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_Copy(term_SecondArgument(QueryAtom)));
            continue;
          } else {
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_SecondArgument(QueryAtom));
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_Copy(term_SecondArgument(Atom)));
          }
        } else {
          /* given: Atom == S(s,t); QueryAtom == S(x,u)
             if s <= t:
               replace QueryAtom by S(x,x)
               and start new iteration
             otherwise:
               replace QueryAtom by S(s,x) */

          term_Delete(term_SecondArgument(QueryAtom));
          if (Compare == ord_SmallerThan() || Compare == ord_Equal()) {
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_Copy(term_FirstArgument(QueryAtom)));
            continue;
          } else {
            list_Rplaca(list_Cdr(term_ArgumentList(QueryAtom)),
                        term_FirstArgument(QueryAtom));
            list_Rplaca(term_ArgumentList(QueryAtom),
                        term_Copy(term_FirstArgument(Atom)));
          }
        }

        TermList = st_GetUnifier(cont_LeftContext(), sharing_Index(ShIndex),
                                 cont_RightContext(), QueryAtom);
  
        for ( ; !list_Empty(TermList); TermList = list_Pop(TermList)) {
          LIST       LitList;
          TERM       PartnerAtom;
          LITERAL    PartnerLit;
          CLAUSE     PartnerClause;
          int        j;          
    
          PartnerAtom = list_First(TermList);
                    
          if (term_IsVariable(PartnerAtom)) 
            continue;

          for (LitList = sharing_NAtomDataList(PartnerAtom); !list_Empty(LitList); LitList = list_Cdr(LitList)) {
            PartnerLit    = list_Car(LitList);
            j             = clause_LiteralGetIndex(PartnerLit);
            PartnerClause = clause_LiteralOwningClause(PartnerLit);
            
            /* avoid second inference of a clause with itself */
            if (clause_Number(PartnerClause) == clause_Number(GivenClause))
              break;
            
            if (ord_LiteralCompare(Atom,clause_LiteralGetOrderStatus(ActLit),
                                   PartnerAtom,clause_LiteralGetOrderStatus(PartnerLit),
                                   TRUE, Flags, Precedence) == ord_SmallerThan())
              break;
            
            if (clause_LiteralIsPositive(PartnerLit) &&
                clause_HasSolvedConstraint(PartnerClause) &&
                !clause_GetFlag(PartnerClause,CLAUSESELECT) &&                
                clause_LiteralGetFlag(PartnerLit,STRICTMAXIMAL)) {

              SUBST  Subst, PartnerSubst;
              int pl;
              SYMBOL MaxVar;

              MaxVar = clause_MaxVar(PartnerClause);                
              if (MaxVar != symbol_GetInitialStandardVarCounter()) { /* renaming the query atom and given copy clause variables */
                int l,len;
                
                len = clause_Length(GivenCopy);
                
                term_StartMaxRenaming(MaxVar);
                term_Rename(QueryAtom);
                
                for (l = clause_FirstLitIndex(); l < len; l++)
                  term_Rename(clause_GetLiteralTerm(GivenCopy, l));
              }
              
              cont_Check();
              if (!unify_UnifyNoOC(cont_LeftContext(), QueryAtom, cont_RightContext(), PartnerAtom)) {                
                misc_StartErrorReport();
                misc_ErrorReport("\n In inf_CompositionResolution: Unification failed.");
                misc_FinishErrorReport();
              }

              cont_StartBinding(); /*saving the current number of bindings, so that we'll be able to undo up to this point */

              pl = clause_LastSuccedentLitIndex(PartnerClause);
              for (k = clause_FirstSuccedentLitIndex(PartnerClause); k <= pl; k++ ) 
                if ( j != k ) {
                  OtherLit = clause_GetLiteral(PartnerClause,k);
                  Other = clause_LiteralAtom(OtherLit);
                  if (symbol_Equal(term_TopSymbol(Other),SpecSymbol) &&
                     (RightArg ? unify_ContinueToUnify(cont_RightContext(), term_SecondArgument(PartnerAtom),
                                                       cont_RightContext(), term_SecondArgument(Other)) 
                               : unify_ContinueToUnify(cont_RightContext(), term_FirstArgument(PartnerAtom),
                                                       cont_RightContext(), term_FirstArgument(Other)) )) {

                    subst_ExtractUnifier(cont_LeftContext(), &Subst, cont_RightContext(), &PartnerSubst);

                    if (inf_CompResLiteralsMax(PartnerClause, j, PartnerSubst,
                                               GivenCopy, i, Subst,
                                               RightArg, Flags, Precedence)) {
                                                                                              
                      Result = list_Cons(inf_ApplyCompRes(PartnerClause, k, PartnerSubst,
                                                          GivenCopy, i, Subst,
                                                          RightArg,
                                                          SpecSymbol, 
                                                          Search,Flags, Precedence), Result);
                                                          
                      /*This is the general composition law version of the rule for the case where the conclusion contains the equality predicate*/
                      if (ana_GetFeature(prfs_GetAnalyze(Search),ana_EQUATIONS)) {
                        Result = list_Cons(inf_ApplyCompRes(PartnerClause, k, PartnerSubst,
                                                            GivenCopy, i, Subst,
                                                            RightArg,
                                                            fol_Equality(), 
                                                            Search,Flags, Precedence), Result);
                      }
                    }
                    subst_Delete(Subst);
                    subst_Delete(PartnerSubst);
                  }
                  cont_BackTrackAndStart();
                }

              cont_StopAndBackTrack();
              cont_Reset(); 
            }
          } /* end of for (LitList = sharing_NAtomDataList ...). */
        } /* end of for (TermList = st_GetUnifier...). */                                     
      } /* end of for (RightArg = 0/1). */
      
    } /* end of if (clause_LiteralGetFlag(ActLitGiven,STRICTMAXIMAL) && ...). */
  } /* end of for 'all succedent literals'. */
  
  if (GivenCopy != NULL) {
    clause_Delete(GivenCopy);
    term_Delete(QueryAtom);
  }
  
/*
  if (list_Exist(Result)) {
    LIST results = Result;    
    printf("\n We have had a composition resolution inference(s):\n");
    while (list_Exist(results)) {
      clause_Print(list_Car(results));
      printf("\n");
      results = list_Cdr(results);
    }
  }
*/
  return Result;
}

LIST inf_DerivableClauses(PROOFSEARCH Search, CLAUSE GivenClause)
/**************************************************************
  INPUT:   A clause and an Index, usually the WorkedOffIndex.
  RETURNS: A list of clauses derivable from 'GivenClause' wrt index.
  EFFECT:  Allocates memory for the clauselistnodes and new clauses.
***************************************************************/
{
  LIST         ListOfDerivedClauses;
  SHARED_INDEX ShIndex;
  SORTTHEORY   Dynamic;
  FLAGSTORE    Flags;
  PRECEDENCE   Precedence;

  Flags        = prfs_Store(Search);
  Precedence   = prfs_Precedence(Search);

#ifdef CHECK
  if (!clause_IsClause(GivenClause, Flags, Precedence)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In inf_DerivableClauses: Illegal input.");
    misc_FinishErrorReport();
  }
#endif

  ListOfDerivedClauses = list_Nil();
  ShIndex              = prfs_WorkedOffSharingIndex(Search);
  Dynamic              = prfs_DynamicSortTheory(Search);

  if (Dynamic && !clause_HasSolvedConstraint(GivenClause)) {

    if (clause_HasTermSortConstraintLits(GivenClause)) {
      if (flag_GetFlagIntValue(Flags, flag_ISOR))
	ListOfDerivedClauses =
	  list_Nconc(inf_ForwardSortResolution(GivenClause,
					       sharing_Index(ShIndex),	
					       Dynamic, FALSE, Flags,Precedence),
		     ListOfDerivedClauses);
    }
    else
      if (flag_GetFlagIntValue(Flags, flag_IEMS))	
	ListOfDerivedClauses =
	  list_Nconc(inf_ForwardEmptySort(GivenClause,
					  sharing_Index(ShIndex), Dynamic,
					  FALSE, Flags, Precedence), 
		     ListOfDerivedClauses); 
  } else { /* Given with solved Constraint! */

    if (Dynamic && flag_GetFlagIntValue(Flags, flag_IEMS))
      ListOfDerivedClauses =
	list_Nconc(inf_BackwardEmptySort(GivenClause, sharing_Index(ShIndex),
					 Dynamic, FALSE, Flags, Precedence),
		   ListOfDerivedClauses); 

    if (Dynamic && flag_GetFlagIntValue(Flags, flag_ISOR))
      ListOfDerivedClauses =
	list_Nconc(inf_BackwardSortResolution(GivenClause,
					      sharing_Index(ShIndex), Dynamic,
					      FALSE, Flags, Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_IEQR))
      ListOfDerivedClauses =
	list_Nconc(inf_EqualityResolution(GivenClause, TRUE, Flags, Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_IERR))
      ListOfDerivedClauses =
	list_Nconc(inf_EqualityResolution(GivenClause, FALSE, Flags, Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_IMPM))
      ListOfDerivedClauses =
	list_Nconc(inf_MergingParamodulation(GivenClause, ShIndex, Flags,
					     Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_IEQF))
      ListOfDerivedClauses =
	list_Nconc(inf_EqualityFactoring(GivenClause,Flags, Precedence),
		   ListOfDerivedClauses);

    switch (flag_GetFlagIntValue(Flags, flag_IOFC)) {
    case flag_FACTORINGOFF:
      break; /* Do nothing */
    case flag_FACTORINGONLYRIGHT:
      ListOfDerivedClauses =
	list_Nconc(inf_GeneralFactoring(GivenClause, TRUE, FALSE,TRUE, Flags,
					Precedence),
		   ListOfDerivedClauses);
      break;
    case flag_FACTORINGRIGHTANDLEFT:
      ListOfDerivedClauses =
	list_Nconc(inf_GeneralFactoring(GivenClause, TRUE, TRUE, TRUE, Flags,
					Precedence),
		   ListOfDerivedClauses);
      break;
    default:
      misc_StartUserErrorReport();
      misc_UserErrorReport("\n Error: Flag \"IOFC\" has invalid value.\n");
      misc_FinishUserErrorReport();
    }

    if (flag_GetFlagIntValue(Flags, flag_ISFC))
      ListOfDerivedClauses =
	list_Nconc(inf_GeneralFactoring(GivenClause, FALSE, TRUE, TRUE, Flags,
					Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_ISPR))
      ListOfDerivedClauses =
	list_Nconc(inf_SuperpositionRight(GivenClause,ShIndex,Flags,Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_ISPM))
      ListOfDerivedClauses =
	list_Nconc(inf_Paramodulation(GivenClause, ShIndex, Flags, Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_IOPM))
      ListOfDerivedClauses =
	list_Nconc(inf_OrderedParamodulation(GivenClause, ShIndex, Flags,
					     Precedence),
		   ListOfDerivedClauses);  

    if (flag_GetFlagIntValue(Flags, flag_ISPL)) 
      ListOfDerivedClauses =
	list_Nconc(inf_SuperpositionLeft(GivenClause, ShIndex, Flags,Precedence),
		   ListOfDerivedClauses);  

    switch (flag_GetFlagIntValue(Flags, flag_IORE)) {
    case flag_ORDEREDRESOLUTIONOFF:
      break;   /* Do nothing */
    case flag_ORDEREDRESOLUTIONNOEQUATIONS:  /* no equations */
      ListOfDerivedClauses =
	list_Nconc(inf_GeneralResolution(GivenClause,ShIndex,TRUE,FALSE,Flags,
					 Precedence),
		   ListOfDerivedClauses);
      break;

    case flag_ORDEREDRESOLUTIONWITHEQUATIONS:  /* allow equations */
      ListOfDerivedClauses =
	list_Nconc(inf_GeneralResolution(GivenClause,ShIndex,TRUE,TRUE, Flags,
					 Precedence),
		   ListOfDerivedClauses);
      break;
    default:
      misc_StartUserErrorReport();
      misc_UserErrorReport("\n Error: Flag \"IORE\" has invalid value.\n");
      misc_FinishUserErrorReport();
    }
      

    switch (flag_GetFlagIntValue(Flags, flag_ISRE)) {
    case flag_STANDARDRESOLUTIONOFF:
      break;   /* Do nothing */
    case flag_STANDARDRESOLUTIONNOEQUATIONS:  /* no equations */
      ListOfDerivedClauses =
	list_Nconc(inf_GeneralResolution(GivenClause,ShIndex,FALSE,FALSE,Flags,
 Precedence),
		   ListOfDerivedClauses);
      break;
    case flag_STANDARDRESOLUTIONWITHEQUATIONS:  /* allow equations */
      ListOfDerivedClauses =
	list_Nconc(inf_GeneralResolution(GivenClause,ShIndex,FALSE,TRUE,Flags,
					 Precedence),
		   ListOfDerivedClauses);
      break;
    default:
      misc_StartUserErrorReport();
      misc_UserErrorReport("\n Error: Flag \"ISRE\" has invalid value.\n");
      misc_FinishUserErrorReport();
    }

    if (flag_GetFlagIntValue(Flags, flag_IUNR))
      ListOfDerivedClauses =
	list_Nconc(inf_UnitResolution(GivenClause, ShIndex, FALSE,
				      Flags, Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_IBUR))
      ListOfDerivedClauses =
	list_Nconc(inf_BoundedDepthUnitResolution(GivenClause,ShIndex,FALSE,
						  Flags, Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_ISHY))
      ListOfDerivedClauses =
      	list_Nconc(inf_GeneralHyperResolution(GivenClause,ShIndex,FALSE,Flags,
					      Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_IOHY))
      ListOfDerivedClauses =
      	list_Nconc(inf_GeneralHyperResolution(GivenClause,ShIndex,TRUE,Flags,
					      Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_IURR))
      ListOfDerivedClauses =
	list_Nconc(inf_URResolution(GivenClause, ShIndex, Flags, Precedence),
		   ListOfDerivedClauses);

    if (flag_GetFlagIntValue(Flags, flag_IDEF))
      ListOfDerivedClauses =
	list_Nconc(inf_ApplyDefinition(Search, GivenClause, Flags, Precedence),
		   ListOfDerivedClauses);
     
    if (flag_GetFlagIntValue(Flags, flag_IChain)) {
      ListOfDerivedClauses =
        list_Nconc(inf_OrderedChaining(GivenClause, Search, ShIndex, Flags, Precedence),
                   ListOfDerivedClauses);

      ListOfDerivedClauses =
        list_Nconc(inf_NegativeChaining(GivenClause, Search, ShIndex, Flags, Precedence),
                   ListOfDerivedClauses);
    }     
    
    if (flag_GetFlagIntValue(Flags, flag_ICompRes)) {
      ListOfDerivedClauses =
        list_Nconc(inf_CompositionResolution(GivenClause, Search, ShIndex, Flags, Precedence),
                   ListOfDerivedClauses);    
    }
  }

#ifdef CHECK
  if (list_Exist(ListOfDerivedClauses)) {
    LIST results = ListOfDerivedClauses;        
    while (list_Exist(results)) {
      if (!clause_IsClause((CLAUSE)list_Car(results), Flags, Precedence)) {
        misc_StartErrorReport();
        misc_ErrorReport("\n In inf_DerivableClauses: Unsound output.");
        misc_FinishErrorReport();
      }
      results = list_Cdr(results);
    }
  } 
#endif
  
/*
  printf("\nAll the inferences for given{");  
  if (list_Exist(ListOfDerivedClauses)) {
    LIST results = ListOfDerivedClauses;        
    while (list_Exist(results)) {
      printf("\n    ");    
      clause_Print(list_Car(results));
      results = list_Cdr(results);
    }
  }
  printf("\n}");
*/
  
  return ListOfDerivedClauses;
}
