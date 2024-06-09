/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *         INTERFACE FOR ALL ORDERING IMPLEMENTATIONS     * */
/* *                                                        * */
/* *  $Module:   ORDER                                      * */ 
/* *                                                        * */
/* *  Copyright (C) 1997, 1998, 1999, 2000, 2001            * */
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
/* $Revision: 1.7 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-11-27 12:42:39 $                             * */
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


/* $RCSfile: order.c,v $ */

#include "order.h"
#include "kbo.h"
#include "rpos.h"

NAT        ord_VARCOUNT[symbol__MAXSTANDARDVAR][2];
PRECEDENCE ord_PRECEDENCE;

/**************************************************************/
/* Previously Inlined Functions                                           */
/**************************************************************/
ord_RESULT ord_Uncomparable(void)
{
  return ord_UNCOMPARABLE;
}

ord_RESULT ord_Equal(void)
{
  return ord_EQUAL;
}

ord_RESULT ord_GreaterThan(void)
{
  return ord_GREATER_THAN;
}

ord_RESULT ord_SmallerThan(void)
{
  return ord_SMALLER_THAN;
}

BOOL ord_IsGreaterThan(ord_RESULT Res)
{
  return ord_GREATER_THAN == Res;
}

BOOL ord_IsNotGreaterThan(ord_RESULT Res)
{
  return ord_GREATER_THAN != Res;
}

BOOL ord_IsNotGreaterThanOrEqual(ord_RESULT Res)
{
  return !(Res == ord_GREATER_THAN || Res == ord_EQUAL);
}

BOOL ord_IsSmallerThan(ord_RESULT Res)
{
  return ord_SMALLER_THAN == Res;
}

BOOL ord_IsNotSmallerThan(ord_RESULT Res)
{
  return ord_SMALLER_THAN != Res;
}

BOOL ord_IsNotSmallerThanOrEqual(ord_RESULT Res)
{
  return !(Res == ord_SMALLER_THAN || Res == ord_EQUAL);
}

BOOL ord_IsEqual(ord_RESULT Res)
{
  return ord_EQUAL == Res;
}

BOOL ord_IsUncomparable(ord_RESULT Res)
{
  return ord_UNCOMPARABLE == Res;
}


/**************************************************************/
/* Inline Functions ends here                                 */
/**************************************************************/

static ord_RESULT ord_CheckDomPred(TERM T1, TERM T2, PRECEDENCE P)
/**************************************************************
  INPUT:   Two terms and a precedence.
  RETURNS: An ordering result with respect to a possible domination
           of leading predicates in <T1> or <T2>.
***************************************************************/
{
  if ((term_IsAtom(T1) && symbol_HasProperty(term_TopSymbol(T1), DOMPRED)) ||
      (term_IsAtom(T2) && symbol_HasProperty(term_TopSymbol(T2), DOMPRED))) {
    if (term_IsAtom(T1)) {
      if (term_IsAtom(T2)) {
	if (symbol_HasProperty(term_TopSymbol(T1), DOMPRED) && 
	    (!symbol_HasProperty(term_TopSymbol(T2), DOMPRED) ||
	     symbol_PrecedenceGreater(P,term_TopSymbol(T1),term_TopSymbol(T2))))
	  return ord_GREATER_THAN;
	if (!symbol_HasProperty(term_TopSymbol(T1), DOMPRED) ||
	    term_TopSymbol(T1) != term_TopSymbol(T2))
	  return ord_SMALLER_THAN;
	/* If the top symbols are equal, do the normal comparison */
      } else {
	/* T1 is an atom, T2 is not, so T1 is greater */
	return ord_GREATER_THAN;
      }
    } else {
      /* T1 is not an atom, so T2 must be an atom */
      return ord_SMALLER_THAN;
    }
  }
  return ord_UNCOMPARABLE;
}


ord_RESULT ord_CompareAux(TERM T1, TERM T2, FLAGSTORE FlagStore, PRECEDENCE P, BOOL VarIsConst)
/**************************************************************
  INPUT:   
  RETURNS: 
  CAUTION: If <VarIsConst> is set then variables are interpreted as constants
           with lowest precedence. They are ranked to each other using
           their variable index.
***************************************************************/
{
  ord_RESULT Aux;

#ifdef CHECK
  if (fol_IsEquality(T1) || fol_IsEquality(T2)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ord_CompareAux:");
    misc_ErrorReport("\n Illegal input. One input term is an equality.");
    misc_FinishErrorReport();    
  }
#endif
  
  Aux = ord_CheckDomPred(T1, T2, P);
  if (Aux != ord_UNCOMPARABLE)
    return Aux;
  else {
    ord_PRECEDENCE = P;
    switch(flag_GetFlagIntValue(FlagStore, flag_ORD)) {
    case flag_ORDKBO:  return kbo_CompareAux(T1, T2, VarIsConst); break;
    case flag_ORDRPOS: return rpos_CompareAux(T1, T2, VarIsConst); break;
    default:
      misc_StartErrorReport(); 
      misc_ErrorReport("\n In ord_CompareAux:");
      misc_ErrorReport("\n Illegal ordering type.");
      misc_FinishErrorReport();
    }
  }
  return ord_UNCOMPARABLE;
}

ord_RESULT ord_Compare(TERM T1, TERM T2, FLAGSTORE FlagStore, PRECEDENCE P)
/**************************************************************
  INPUT: two terms, a Flagstore and a Precedence  
  RETURNS: compares <T1> and <T2> respecting <P>
***************************************************************/
{
        return ord_CompareAux(T1, T2, FlagStore, P, FALSE);
}
 

ord_RESULT ord_CompareSkolem(TERM T1, TERM T2, FLAGSTORE FlagStore, PRECEDENCE P)
/**************************************************************
  INPUT: two terms, a Flagstore and a Precedence  
  RETURNS: compares <T1> and <T2> respecting <P>, variables 
           in <T1> and <T2> are interpreted as skolem variables
  CAUTION: If <VarIsConst> is set then variables are interpreted as constants
           with lowest precedence. They are ranked to each other using
           their variable index.
***************************************************************/
{
        return ord_CompareAux(T1, T2, FlagStore, P, TRUE);
}


BOOL ord_CompareEqual(TERM T1, TERM T2, FLAGSTORE Flags)
/**************************************************************
  INPUT:   Two terms and a flag store.
  RETURNS: TRUE, iff the two terms are equal with respect to the
           reduction ordering selected by the 'flag_ORD' flag.
***************************************************************/
{
  switch(flag_GetFlagIntValue(Flags, flag_ORD)) {
  case flag_ORDKBO:  return term_Equal(T1, T2); break;
  case flag_ORDRPOS: return rpos_Equal(T1, T2); break;
  default:
    misc_StartErrorReport(); 
    misc_ErrorReport("\n In ord_Compare: Illegal ordering type.");
    misc_FinishErrorReport();
    return FALSE;   /* unreachable code ... */
  }
}

ord_RESULT ord_ContCompare(VARCONT C1, TERM T1, VARCONT C2, TERM T2,
			   FLAGSTORE FlagStore, PRECEDENCE P)
{
  ord_RESULT Aux;

#ifdef CHECK
  if (fol_IsEquality(T1) || fol_IsEquality(T2)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ord_ContCompare:");
    misc_ErrorReport("\n Illegal input. One input term is an equality.");
    misc_FinishErrorReport();
  }
#endif 
  
  Aux = ord_CheckDomPred(T1, T2, P);
  if (Aux != ord_UNCOMPARABLE)
    return Aux;
  else {
    ord_PRECEDENCE = P;
    switch(flag_GetFlagIntValue(FlagStore, flag_ORD)) {
    case flag_ORDKBO:  return kbo_ContCompare(C1, T1, C2, T2); break;
    case flag_ORDRPOS: return rpos_ContCompare(C1, T1, C2, T2); break;
    default:
      misc_StartErrorReport();
      misc_ErrorReport("\n In ord_ContCompare:");
      misc_ErrorReport("\n Illegal ordering type.");
      misc_FinishErrorReport();
    }
  }
  return ord_UNCOMPARABLE;
}


BOOL ord_ContGreater(VARCONT C1, TERM T1, VARCONT C2, TERM T2,
		     FLAGSTORE FlagStore, PRECEDENCE P)
{ 
  ord_RESULT Aux;

#ifdef CHECK
  if (fol_IsEquality(T1) || fol_IsEquality(T2)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ord_ContGreater:");
    misc_ErrorReport("\n Illegal input. One input term is an equality.");
    misc_FinishErrorReport();
  }
#endif 
  
  Aux = ord_CheckDomPred(T1, T2, P);
  if (Aux != ord_UNCOMPARABLE)
    return (Aux == ord_GREATER_THAN);
  else {
    ord_PRECEDENCE = P;
    switch(flag_GetFlagIntValue(FlagStore, flag_ORD)) {
    case flag_ORDKBO:  return kbo_ContGreater(C1, T1, C2, T2); break;
    case flag_ORDRPOS: return rpos_ContGreater(C1, C1, T1, C2, C2, T2); break;
    default:
      misc_StartErrorReport();
      misc_ErrorReport("\n In ord_ContGreater:");
      misc_ErrorReport("\n Illegal ordering type.");
      misc_FinishErrorReport();
    }
  }
  return FALSE;   /* This line is never reached ...  */
}


ord_RESULT ord_Not(ord_RESULT Result)
/**************************************************************
  INPUT:   An ord_RESULT
  RETURNS: The negation with respect to argument switching.
***************************************************************/
{
  if (Result == ord_UNCOMPARABLE || Result == ord_EQUAL)
    return Result;

  if (Result == ord_GREATER_THAN)
    return ord_SMALLER_THAN;

  return ord_GREATER_THAN;
}


void ord_Print(ord_RESULT Result)
/**************************************************************
  INPUT:   An ord_Result.
  RETURNS: None, prints the Result as a string to stdout.
***************************************************************/
{
  switch(Result) { 
  case ord_UNCOMPARABLE:  fputs(" uncomparable ",stdout); break;
  case ord_EQUAL:         fputs(" equal ",stdout); break;
  case ord_GREATER_THAN:  fputs(" greater than ",stdout); break;
  case ord_SMALLER_THAN:  fputs(" smaller than ",stdout); break;
  default:                fputs(" Nonsense! ",stdout);
  }
}

static ord_RESULT ord_CompareBools(BOOL b1, BOOL b2) {
  int i = b1 - b2;

  if (i > 0)
    return ord_GREATER_THAN;
  else if (i<0)
    return ord_SMALLER_THAN;
  else
    return ord_EQUAL;
}

static ord_RESULT ord_CompareSpecials(PRECEDENCE precedence, SYMBOL s1, SYMBOL s2)
/**************************************************************
  INPUT:   A precedence and two special (equality) predicate symbols.
  RETURNS: Comparision status of the two symbols.
  REMARK:  The symbols are compared according to
           precedence with the additional overriding rule that
           equlity is always smallest.
***************************************************************/  
{
#ifdef CHECK 
  if ((!symbol_HasProperty(s1, TRANSITIVE) && !symbol_Equal(s1,fol_EQUALITY)) ||
      (!symbol_HasProperty(s2, TRANSITIVE) && !symbol_Equal(s2,fol_EQUALITY)) ) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ord_CompareSpecials: strange input.");
    misc_FinishErrorReport();
  }
#endif

  /*Equality (if it is special) is forced to be the smallest of all */
  if (symbol_Equal(s1,fol_Equality())) {
    if (symbol_Equal(s2,fol_Equality())) {
      return ord_EQUAL;
    } else {
      return ord_SMALLER_THAN;
    }
  } else {
    if (symbol_Equal(s2,fol_Equality())) {
      return ord_GREATER_THAN;
    } else {
      /* As the induced ordering is discrete in the current, trivial, implementaion,
         we at least try to comply as much as possible with the precedence */
      if (symbol_PrecedenceGreater(precedence,s1,s2))
        return ord_GREATER_THAN;
      else if (symbol_PrecedenceGreater(precedence,s2,s1))
        return ord_SMALLER_THAN;
      else
        return ord_EQUAL;
    }
  }
}

ord_RESULT ord_LiteralCompareAux(TERM Lit1, ord_RESULT OrdStat1,
                                 TERM Lit2, ord_RESULT OrdStat2,
			      BOOL Check, BOOL VarIsConst, FLAGSTORE FlagStore,
			      PRECEDENCE Precedence)
/*********************************************************
  INPUT:   Two term literals and two ordering stutuses. The latter are only
           meaningful for equality and transitive literals and indicate
           how the left-hand side and right-hand side (in this order) terms
           of such a literal relate to each other.

	   Additionally, a check flag that indicates whether
	   the ordering stutuses are sufficient or necessary and sufficient:
	   If Check=TRUE then if the status is ord_UNCOMPARABLE it is interpreted
     that the real ordering status is not known.
     If Check=FALSE, we know that the terms are really uncomparable and that
     additional check wouldn't help.	   
     A flag <VarIsConst>. If <VarIsConst> is set variables
           are treated as skolem constants.
	   A flag store.
	   A precedence.
  RETURNS: The result if these literals are compared with
           respect to the ordering. Dependent on their sign, the literals
	   are compared as multiset of their topsymbol terms,
	   where any literal is considered to be an equation
	   and non equational literals are considered to be
	   equations with the artificial, non-existent, minimal
	   constant tt.
           In the case when transitive predicates are involved
           an different approach to comparing the literals is used
           such that the resulting literal ordering is admissible for chaining.
  EFFECT:  The arguments of the literals are possibly, destructively flipped.
  CAUTION: If <VarIsConst> is set then variables are interpreted as constants
           with lowest precedence. They are ranked to each other using
           their variable index.
**********************************************************/
{
  ord_RESULT result,auxResult,AuxRl1l2,AuxRl1r2,AuxRr1r2,AuxRr1l2,Comp1,Comp2;
  TERM       OLit1,OLit2;
  BOOL       pos1,pos2;
  BOOL       Orient1, Orient2;

  OLit1 = Lit1;
  OLit2 = Lit2;  

  Orient1 = (OrdStat1 == ord_GREATER_THAN);
  Orient2 = (OrdStat2 == ord_GREATER_THAN);
  
  pos1  = pos2  = TRUE;
  Comp1 = Comp2 = result = ord_UNCOMPARABLE;
  if (symbol_Equal(term_TopSymbol(Lit1),fol_Not())) {
    Lit1 = term_FirstArgument(Lit1);
    pos1 = FALSE;
  }
  if (symbol_Equal(term_TopSymbol(Lit2),fol_Not())) {
    Lit2 = term_FirstArgument(Lit2);
    pos2 = FALSE;
  }
  
  if ( flag_GetFlagIntValue(FlagStore,flag_Transitivity) &&
        /*
        if both are equality atoms the admissible ordering coincides 
        with the standard one a we want to use the standard implementation
        */
        (!fol_IsEquality(Lit1) || ! fol_IsEquality(Lit2))) {

    /* The "quintuple" literal ordering from the paper extended to be total on ground literals */

    if ( fol_IsEquality(Lit1) || symbol_HasProperty(term_TopSymbol(Lit1), TRANSITIVE)) {         /* First is special */
      if ( fol_IsEquality(Lit2) || symbol_HasProperty(term_TopSymbol(Lit2), TRANSITIVE)) {       /* Second is special */
        TERM       l1,r1,l2,r2,aux;

        l1 = term_FirstArgument(Lit1);
        r1 = term_SecondArgument(Lit1);
        l2 = term_FirstArgument(Lit2);
        r2 = term_SecondArgument(Lit2);

        if ((Comp1 = OrdStat1) != ord_UNCOMPARABLE ||
            (Check && 
            (Comp1 = ord_CompareAux(l1,r1, FlagStore, Precedence, VarIsConst)) != ord_UNCOMPARABLE)) {
        
          if (Comp1 == ord_SMALLER_THAN) {
            aux = l1; l1 = r1; r1 = aux;
          }

          if ((Comp2 = OrdStat2) != ord_UNCOMPARABLE ||
              (Check &&
              (Comp2 = ord_CompareAux(l2,r2, FlagStore, Precedence, VarIsConst)) != ord_UNCOMPARABLE)) {
              
            /* Both predicates are "oriented" */
            if (Comp2 == ord_SMALLER_THAN) {
              aux = l2; l2 = r2; r2 = aux;
            }

            result = ord_CompareAux(l1,l2, FlagStore, Precedence, VarIsConst);

            if (result != ord_EQUAL) {
              /*case1:  printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2);*/
              return result;
            }

            result = ord_CompareBools((!pos1),(!pos2)); /*negative = 1, positive = 0*/

            if (result != ord_EQUAL) {
              /*case2: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2);*/
              return result;
            }

            result = ord_CompareSpecials(Precedence,term_TopSymbol(Lit1),term_TopSymbol(Lit2));

            if (result != ord_EQUAL) {
               /*case3: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2);*/
              return result;
            }

            result = ord_CompareBools(
                   (ord_IsEqual(Comp1) || (!symbol_Equal(term_TopSymbol(Lit1),fol_Equality()) && ord_IsGreaterThan(Comp1))),
                   (ord_IsEqual(Comp2) || (!symbol_Equal(term_TopSymbol(Lit2),fol_Equality()) && ord_IsGreaterThan(Comp2))));
                       /*Lit = uSv , v>u means 0, otherwise 1
                         for equality 1 is only when u==v, since otherwise we can swap them to make it 0*/

            if (result != ord_EQUAL) {
              /*case4: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            result = ord_CompareBools((ord_IsEqual(Comp1)),
                                    (ord_IsEqual(Comp2))); /*Lit = uSv , u=v means 1, otherwise 0*/

            if (result != ord_EQUAL) {
              /*case5: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            result = ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst);
            /*case6: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return result;
          } else { /* Lit2 is not "oriented" special */

            result = ord_CompareAux(l1,l2, FlagStore, Precedence, VarIsConst);

            if (result == ord_SMALLER_THAN) {
              /*case7: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            if (result == ord_EQUAL) {
#ifdef CHECK
              if (ord_CompareAux(l1,r2, FlagStore, Precedence, VarIsConst) != ord_UNCOMPARABLE) {
                misc_StartErrorReport();
                misc_ErrorReport("\n In ord_LiteralCompareAux: Established relation for uncomparable terms.");
                misc_FinishErrorReport();
              }
#endif
              result = ord_UNCOMPARABLE;
              /*case8: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            AuxRl1r2 = ord_CompareAux(l1,r2, FlagStore, Precedence, VarIsConst);

            if (AuxRl1r2 == ord_SMALLER_THAN || AuxRl1r2 == ord_UNCOMPARABLE) {
              result = AuxRl1r2;
              /*case9: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return AuxRl1r2;
            }

            if (AuxRl1r2 == ord_EQUAL) {
#ifdef CHECK
              if (result != ord_UNCOMPARABLE) {
                misc_StartErrorReport();
                misc_ErrorReport("\n In ord_LiteralCompareAux: Established relation for uncomparable terms.");
                misc_FinishErrorReport();
              }
#endif
              result = ord_UNCOMPARABLE;
              /*case10: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }
            
            /*case11: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return result; /* == ord_GREATER_THAN || ord_UNCOMPARABLE (while AuxRl1r2 == ord_GREATER_THAN)*/
          }
        }
        else /* Lit1 is not "oriented" special */
        if ((Comp2 = OrdStat2) != ord_UNCOMPARABLE ||
            (Check &&
            (Comp2 = ord_CompareAux(l2,r2, FlagStore, Precedence, VarIsConst)) != ord_UNCOMPARABLE)) {
          /* Lit2 is "orientable" */
          if (Comp2 == ord_SMALLER_THAN) {
            aux = l2; l2 = r2; r2 = aux;
          }

          result = ord_CompareAux(l1,l2, FlagStore, Precedence, VarIsConst);         

          if (result == ord_GREATER_THAN) {
            /*case12: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return result;
          }
          
          if (result == ord_EQUAL) {
#ifdef CHECK
            if (ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst) != ord_UNCOMPARABLE) {
              misc_StartErrorReport();
              misc_ErrorReport("\n In ord_LiteralCompareAux: Established relation for uncomparable terms.");
              misc_FinishErrorReport();
            }
#endif
            result = ord_UNCOMPARABLE;
            /*case13: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return result;
          }
          
          AuxRr1l2 = ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst);

          if (AuxRr1l2 == ord_GREATER_THAN || AuxRr1l2 == ord_UNCOMPARABLE) {
            result = AuxRr1l2;
            /*case14: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return AuxRr1l2;
          }

          if (AuxRr1l2 == ord_EQUAL) {
#ifdef CHECK
            if (result != ord_UNCOMPARABLE) {
              misc_StartErrorReport();
              misc_ErrorReport("\n In ord_LiteralCompareAux: Established relation for uncomparable terms.");
              misc_FinishErrorReport();
            }
#endif
            result = ord_UNCOMPARABLE;
            /*case15: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return result;
          }          
          
          /*case16: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
          return result; /* == ord_SMALLER_THAN || ord_UNCOMPARABLE (while AuxRr1l2 == ord_SMALLER_THAN)*/
        }
        else { /* Both literals are "unorientable" */
          ord_RESULT Other;
          ord_RESULT a_LResult;

          /* parallel */
          AuxRl1l2 = ord_CompareAux(l1,l2, FlagStore, Precedence, VarIsConst);
          AuxRr1r2 = ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst);

          if (AuxRl1l2 == ord_UNCOMPARABLE) {
          
            /* cross */
            AuxRr1l2 = ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst);
            AuxRl1r2 = ord_CompareAux(l1,r2, FlagStore, Precedence, VarIsConst);
          
            if (AuxRr1r2 == ord_GREATER_THAN && AuxRr1l2 == ord_GREATER_THAN) {                
              result = ord_GREATER_THAN;
              /*case17: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            if (AuxRr1r2 == ord_SMALLER_THAN && AuxRl1r2 == ord_SMALLER_THAN) {
              result = ord_SMALLER_THAN;
              /*case18: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            if (AuxRr1l2 == AuxRl1r2) {
              auxResult = AuxRr1l2; /* == AuxRl1r2*/ 
              if (auxResult == ord_EQUAL) {
                result = ord_CompareBools((!pos1),(!pos2)); /*negative = 1, positive = 0*/

                if (result != ord_EQUAL) {
                  /*case19: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
                  return result;
                }
             
                result = ord_CompareSpecials(Precedence,term_TopSymbol(Lit1),term_TopSymbol(Lit2));

                if (result != ord_EQUAL) {
                  /*case20: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
                  return result;
                }
              
                result = ord_UNCOMPARABLE;
                /*case21: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
                return ord_UNCOMPARABLE; /* we cannot proceed to a_L, but they may differ in that */
              }
              /* else auxResult == ord_SMALLER_THAN / ord_GREATER_THAN / ord_UNCOMPARABLE */
              result = auxResult;
              /*case22: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            if (AuxRr1l2 == ord_UNCOMPARABLE || AuxRl1r2 == ord_UNCOMPARABLE) {
              result = ord_UNCOMPARABLE;
              /*case23a: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return ord_UNCOMPARABLE;
            }
            
            if (AuxRr1l2 == ord_Not(AuxRl1r2)) {
              result = ord_UNCOMPARABLE;
              /*case23b: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return ord_UNCOMPARABLE;
            }
            
            /* 
              it has to be = one of them and the other </> 
              we can do a speculative test of other criteria
            */
            
            if (AuxRr1l2 == ord_EQUAL) {
              auxResult = AuxRl1r2; /* the speculative result */
              Other = ord_Not(auxResult);
              a_LResult = (auxResult == ord_SMALLER_THAN ? ord_SMALLER_THAN : ord_UNCOMPARABLE);
            } else { /* now it has to be AuxRl1r2 who is == ord_EQUAL*/
              auxResult = AuxRr1l2; /* the speculative result */
              Other = ord_Not(auxResult);
              a_LResult = (auxResult == ord_GREATER_THAN ? ord_GREATER_THAN : ord_UNCOMPARABLE);
            }
            
            result = ord_CompareBools((!pos1),(!pos2)); /*negative = 1, positive = 0*/
                        
            if (result == auxResult) { /* confirmed */
              /*case23c: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            if (result == Other) {
              result = ord_UNCOMPARABLE;
              /*case23d: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return ord_UNCOMPARABLE;
            }

            /*result == ord_EQUAL, i.e. sign's didn't help us decide */
            result = ord_CompareSpecials(Precedence,term_TopSymbol(Lit1),term_TopSymbol(Lit2));

#ifdef CHECK
            if (result == ord_UNCOMPARABLE) {
              misc_StartErrorReport();
              misc_ErrorReport("\n In ord_LiteralCompareAux: ord_CompareSpecials should be total!");
              misc_FinishErrorReport();
            }
#endif

            if (result == auxResult) { /* confirmed */
              /*case23e: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            if (result == Other) {
              result = ord_UNCOMPARABLE;
              /*case23f: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }
                        
            result = a_LResult;
            /*case23g: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return result;
          }

          if (AuxRl1l2 == ord_EQUAL) {
            if (AuxRr1r2 == ord_UNCOMPARABLE) {
              result = ord_UNCOMPARABLE;
              /*case24: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return ord_UNCOMPARABLE;
            }

            if (AuxRr1r2 == ord_EQUAL) {
              result = ord_CompareBools((!pos1),(!pos2)); /*negative = 1, positive = 0*/

              if (result != ord_EQUAL) {
                /*case25: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
                return result;
              }
             
              result = ord_CompareSpecials(Precedence,term_TopSymbol(Lit1),term_TopSymbol(Lit2));

              if (result != ord_EQUAL) {
                /*case26: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
                return result;
              }             
              
              result = ord_EQUAL;
              /*case27: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return ord_EQUAL; /* we cannot proceed to a_L or d_L or min_L, but they will necessarily always be the same*/
            }

            /*
            AuxRr1r2 is < or >, the overall result for comparing max_L is (< or =) or (> or =)
            we continue to confirm the inequality or return |
            */
            Other = ord_Not(AuxRr1r2);

            result = ord_CompareBools((!pos1),(!pos2)); /*negative = 1, positive = 0*/

            if (result == AuxRr1r2) {
              /*case28: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            if (result == Other) {
              result = ord_UNCOMPARABLE;
              /*case29: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return ord_UNCOMPARABLE;
            }

            /* result == ord_EQUAL, i.e. sign's didn't help us decide */
            result = ord_CompareSpecials(Precedence,term_TopSymbol(Lit1),term_TopSymbol(Lit2));
            
#ifdef CHECK
            if (result == ord_UNCOMPARABLE) {
              misc_StartErrorReport();
              misc_ErrorReport("\n In ord_LiteralCompareAux: ord_CompareSpecials should be total!");
              misc_FinishErrorReport();
            }
#endif
            
            if (result == Other) {
              result = ord_UNCOMPARABLE;  
              /*case30: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            result = AuxRr1r2;
            /*case31: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return result; /* Either ord_CompareSpecials == AuxRr1r2 and thus supports, or it is ord_EQUAL
                              in the second case go on to a_L and reason by case split: 
                              1) AuxRr1r2 == ord_GREATER_THAN
                                a) X instantiates to sth < r1 => we already have ord_GREATER_THAN, from comparing max_L
                                b) X instantiates to sth = r1 => a_{Lit1}=1 and a_{Lit2}=1, but d_{Lit1}=1 and d_{Lit2}=0
                                                                      thus we return ord_GREATER_THAN
                                c) X instantiates to sth > r1 => a_{Lit1}=1 and a_{Lit2}=1, d_{Lit1}=0 and d_{Lit2}=0, 
                                      but max_{Lit1}=max_{Lit2}=sth and 
                                          min_{Lit1}=r1, min_{Lit2}=r2 and thus return AuxRr1r2
                              2) AuxRr1r2 == ord_SMALLER_THAN
                                  symmetrical
                           */  
          }

          /* AuxRl1l2 == ord_SMALLER_THAN || AuxRl1l2 == ord_GREATER_THAN */          
          Other = ord_Not(AuxRl1l2);

          if (AuxRr1r2 == AuxRl1l2) {
            result = AuxRr1r2;
            /*case32: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return AuxRr1r2;
          }
          else if (AuxRr1r2 == ord_EQUAL) {
            /* the comparision of max terms is either </> or =.
                (If we could tell which of the two it is,
                we would be able to infer the relation between l1,r2 or l2,r2.)
                So we go on to next tests and if they confirm </> we can return it,
                otherwise the result is | */
            result = ord_CompareBools((!pos1),(!pos2)); /*negative = 1, positive = 0*/
            
            if (result == AuxRl1l2) {
              /*case33: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            if (result == Other) {
              result = ord_UNCOMPARABLE;
              /*case34: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return ord_UNCOMPARABLE;
            }

            /*result == ord_EQUAL, i.e. sign's didn't help us decide */
            result = ord_CompareSpecials(Precedence,term_TopSymbol(Lit1),term_TopSymbol(Lit2));

#ifdef CHECK
            if (result == ord_UNCOMPARABLE) {
              misc_StartErrorReport();
              misc_ErrorReport("\n In ord_LiteralCompareAux: ord_CompareSpecials should be total!");
              misc_FinishErrorReport();
            }
#endif

            if (result == Other) {
              result = ord_UNCOMPARABLE;
              /*case35: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return result;
            }

            result = AuxRl1l2;
            /*case36: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return result; /* Either ord_CompareSpecials == AuxRl1l2 and thus supports, or it is ord_EQUAL
                              in the second case go on to a_L and reason by case split: 
                              1) AuxRl1l2 == ord_GREATER_THAN
                                a) X instantiates to sth < l1 => we already have ord_GREATER_THAN, from comparing max_L
                                b) X instantiates to sth = l1 => a_{Lit1}=1 and a_{Lit2}=0 thus we return ord_GREATER_THAN
                                c) X instantiates to sth > l1 => a_{Lit1}=0 and a_{Lit2}=0, d_{Lit1}=0 and d_{Lit2}=0, 
                                      but max_{Lit1}=max_{Lit2}=sth and 
                                          min_{Lit1}=l1, min_{Lit2}=l2 and thus return AuxRl1l2
                              2) AuxRl1l2 == ord_SMALLER_THAN
                                  symmetrical
                           */                       
          } else if (AuxRr1r2 == Other) {
            result = ord_UNCOMPARABLE;
            /*case37: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return ord_UNCOMPARABLE;
          }
          else { /*AuxRr1r2 == ord_UNCOMPARABLE*/
            if (AuxRl1l2 == ord_SMALLER_THAN &&
                ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst) == ord_SMALLER_THAN) {
              result = ord_SMALLER_THAN;
              /*case38: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return ord_SMALLER_THAN;
            }

            if (AuxRl1l2 == ord_GREATER_THAN &&
                ord_CompareAux(l1,r2, FlagStore, Precedence, VarIsConst) == ord_GREATER_THAN) {
                
              result = ord_GREATER_THAN;
              /*case39: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
              return ord_GREATER_THAN;
            }

            result = ord_UNCOMPARABLE;
            /*case40: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
            return ord_UNCOMPARABLE;
          }
        }
      }
      else {/* Second Atom is not a special */
        /* They can't be equal ! */
        result = ord_CompareAux(term_FirstArgument(Lit1),Lit2,FlagStore,Precedence, VarIsConst);       

        if (result == ord_GREATER_THAN) {
          /*case41: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
          return result;
        }

        auxResult = ord_CompareAux(term_SecondArgument(Lit1),Lit2,FlagStore,Precedence, VarIsConst);

        if (auxResult == ord_GREATER_THAN) {
          result = auxResult;
          /*case42: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
          return auxResult;
        }

        if (result != auxResult)
          result = ord_UNCOMPARABLE;        

        /*case43: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
        return result;
      }
    }
    else /* First Atom is not special */
    /* They can't be equal ! */
    if (fol_IsEquality(Lit2) || symbol_HasProperty(term_TopSymbol(Lit2), TRANSITIVE)) {

      result = ord_CompareAux(Lit1,term_FirstArgument(Lit2),FlagStore,Precedence, VarIsConst);

      if (result == ord_SMALLER_THAN) {
        /*case44: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
        return result;
      }

      auxResult = ord_CompareAux(Lit1,term_SecondArgument(Lit2),FlagStore,Precedence, VarIsConst);

      if (auxResult == ord_SMALLER_THAN) {
        result = auxResult;
        /*case45: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
        return auxResult;
      }

      if (result != auxResult)
        result = ord_UNCOMPARABLE;

      /*case46: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
      return result;

    } else { /* None of the atoms is special */

      result = ord_CompareAux(Lit1,Lit2, FlagStore, Precedence, VarIsConst);

      if (result != ord_EQUAL) {
        /*case47: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
        return result;
      }

      result = ord_CompareBools((!pos1),(!pos2)); /*negative = 1, positive = 0*/
      /*case48: printf("\n ord_LiteralCompareAux@%d ",__LINE__); ord_Print(result); fol_PrintDFG(OLit1); putchar(' '); fol_PrintDFG(OLit2); */
      return result; /* even if result == ord_EQUAL we know that the others (s_L, a_L, ... ) would also be the same for two non specials */
    }

    misc_StartErrorReport();
    misc_ErrorReport("\n In ord_LiteralCompareAux: Unreachable code.");
    misc_FinishErrorReport();
  }
       
  if (fol_IsEquality(Lit1)) {         /* Real equation */
    if (fol_IsEquality(Lit2)) {
      TERM       l1,r1,l2,r2,aux;
      l1 = term_FirstArgument(Lit1);
      r1 = term_SecondArgument(Lit1);
      l2 = term_FirstArgument(Lit2);
      r2 = term_SecondArgument(Lit2);
      if (Orient1 || 
	  (Check && 
	   ((Comp1 = ord_CompareAux(l1,r1,FlagStore,Precedence, VarIsConst)) == ord_GREATER_THAN ||
	    Comp1 == ord_SMALLER_THAN))) {
	if (Comp1 == ord_SMALLER_THAN) {
	  aux = l1; l1 = r1; r1 = aux;
	}
	if (Orient2 ||
	    (Check && 
	     ((Comp2 = ord_CompareAux(l2,r2,FlagStore,Precedence, VarIsConst))==ord_GREATER_THAN ||
	      Comp2 == ord_SMALLER_THAN))) {
	  /* Both equations are oriented */
	  if (Comp2 == ord_SMALLER_THAN) {
	    aux = l2; l2 = r2; r2 = aux;
	  }
	  result = ord_CompareAux(l1,l2, FlagStore, Precedence, VarIsConst);
	  if (result == ord_EQUAL) {
	    if (pos1) {
	      if (pos2)
		return ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst);
	      else
		return ord_SMALLER_THAN;
	    }
	    else
	      if (pos2)
		return ord_GREATER_THAN;
	      else
		return ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst);
	  }
	  return result;
	}
	else { /* Lit2 is not oriented equation */
	  if (term_Equal(l1,l2)) {
	    result = ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst);
	    if (result == ord_EQUAL) {
	      if (pos1 && !pos2)
		return ord_SMALLER_THAN;
	      else
		if (!pos1 && pos2)
		  return ord_GREATER_THAN;
	    }
	    return result;
	  }
	  if (term_Equal(l1,r2)) {
	    result = ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst);
	    if (result == ord_EQUAL) {
	      if (pos1 && !pos2)
		return ord_SMALLER_THAN;
	      else
		if (!pos1 && pos2)
		  return ord_GREATER_THAN;
	    }
	    return result;
	  }
	  result    = ord_CompareAux(l1,l2, FlagStore, Precedence, VarIsConst);
	  AuxRl1r2  = ord_CompareAux(l1,r2, FlagStore, Precedence, VarIsConst);
	  if (result == AuxRl1r2)
	    return result;
	  AuxRr1r2  = ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst);
	  if (result == AuxRr1r2)
	    return result;
	  if (AuxRl1r2 == AuxRr1r2 && AuxRl1r2 == ord_SMALLER_THAN)
	    return ord_SMALLER_THAN;
	  AuxRr1l2  = ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst);
	  if (result == AuxRr1l2 && result == ord_SMALLER_THAN)
	    return  ord_SMALLER_THAN;	  
	  return ord_UNCOMPARABLE;                   
	}
      }
      else /* Lit1 is unorientable equation */
	if (Orient2 ||
	    (Check && 
	     ((Comp2 = ord_CompareAux(term_FirstArgument(Lit2),
				   term_SecondArgument(Lit2),
				   FlagStore, Precedence, VarIsConst) == ord_GREATER_THAN) ||
	      Comp2 == ord_SMALLER_THAN))) {   /* Lit2 is orientable equation */
	  if (Comp2 == ord_SMALLER_THAN) {
	    aux = l2; l2 = r2; r2 = aux;
	  }
	  if (term_Equal(l1,l2)) {
	    result = ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst);
	    if (result == ord_EQUAL) {
	      if (pos1 && !pos2)
		return ord_SMALLER_THAN;
	      else
		if (!pos1 && pos2)
		  return ord_GREATER_THAN;
	    }
	    return result;
	  }
	  if (term_Equal(r1,l2)) {
	    result = ord_CompareAux(l1,r2, FlagStore, Precedence, VarIsConst);
	    if (result == ord_EQUAL) {
	      if (pos1 && !pos2)
		return ord_SMALLER_THAN;
	      else
		if (!pos1 && pos2)
		  return ord_GREATER_THAN;
	    }
	    return result;
	  }
	  
	  result    = ord_CompareAux(l1,l2, FlagStore, Precedence, VarIsConst);
	  AuxRr1l2  = ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst);
	  if (result == AuxRr1l2)
	    return result;
	  AuxRr1r2  = ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst);
	  if (result == AuxRr1r2)
	    return result;
	  if (AuxRr1l2 == AuxRr1r2 && AuxRr1l2 == ord_GREATER_THAN)
	    return ord_GREATER_THAN;
	  AuxRl1r2  = ord_Compare(l1,r2, FlagStore, Precedence);
	  if (result == AuxRl1r2 && result == ord_GREATER_THAN)
	    return  ord_GREATER_THAN;
	  return ord_UNCOMPARABLE;
	}
	else { /* Both literals are unorientable equations */
	  if (term_Equal(l1,l2)) {
	    result = ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst);
	    if (result == ord_EQUAL) {
	      if (pos1 && !pos2)
		return ord_SMALLER_THAN;
	      else if (!pos1 && pos2)
		return ord_GREATER_THAN;
	    }
	    return result;
	  }
	  if (term_Equal(r1,l2)) {
	    result = ord_CompareAux(l1,r2, FlagStore, Precedence, VarIsConst);
	    if (result == ord_EQUAL) {
	      if (pos1 && !pos2)
		return ord_SMALLER_THAN;
	      else if (!pos1 && pos2)
		return ord_GREATER_THAN;
	    }
	    return result;
	  }
	  if (term_Equal(l1,r2)) {
	    result = ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst); 
	    if (result == ord_EQUAL) {
	      if (pos1 && !pos2)
		return ord_SMALLER_THAN;
	      else if (!pos1 && pos2)
		return ord_GREATER_THAN;
	    }
	    return result;
	  }
	  if (term_Equal(r1,r2)) {
	    result = ord_CompareAux(l1,l2, FlagStore, Precedence, VarIsConst);
	    if (result == ord_EQUAL) {
	      if (pos1 && !pos2)
		return ord_SMALLER_THAN;
	      else if (!pos1 && pos2)
		return ord_GREATER_THAN;
	    }
	    return result;
	  }
	  result    = ord_CompareAux(l1,l2, FlagStore, Precedence, VarIsConst);
	  if (result == ord_UNCOMPARABLE) {
	    result = ord_CompareAux(l1,r2, FlagStore, Precedence, VarIsConst);
	    if (result == ord_UNCOMPARABLE) {
	     if (ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst) == ord_GREATER_THAN &&
		 ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst) == ord_GREATER_THAN)
	       return ord_GREATER_THAN;
	     return ord_UNCOMPARABLE;
	    }
	    if (result == ord_GREATER_THAN) {
	      if (ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst) == ord_GREATER_THAN)
		return ord_GREATER_THAN;
	      return ord_UNCOMPARABLE;
	    }
	    if (result == ord_SMALLER_THAN) {
	      if (ord_CompareAux(r1,l2, FlagStore, Precedence, VarIsConst) == ord_SMALLER_THAN ||
		  ord_CompareAux(r1,r2, FlagStore, Precedence, VarIsConst) == ord_SMALLER_THAN)
		return ord_SMALLER_THAN;
	      return ord_UNCOMPARABLE;
	    }
	  }
	  if (result == ord_GREATER_THAN) {
	    if ((result = ord_CompareAux(l1,r2,FlagStore,Precedence, VarIsConst)) == ord_GREATER_THAN ||
		(auxResult = ord_CompareAux(r1,r2,FlagStore,Precedence, VarIsConst)) == ord_GREATER_THAN)
	      return ord_GREATER_THAN;
	    if (result == ord_UNCOMPARABLE || auxResult == ord_UNCOMPARABLE)
		return ord_UNCOMPARABLE;
	    return ord_SMALLER_THAN;
	  }
	  if (result == ord_SMALLER_THAN) {
	    if ((result = ord_CompareAux(r1,l2,FlagStore,Precedence, VarIsConst)) == ord_SMALLER_THAN ||
		(auxResult = ord_CompareAux(r1,r2,FlagStore,Precedence, VarIsConst)) == ord_SMALLER_THAN)
	      return ord_SMALLER_THAN;
	    if (result == ord_UNCOMPARABLE || auxResult == ord_UNCOMPARABLE)
	      return ord_UNCOMPARABLE;
	    return ord_GREATER_THAN;
	  }
	}
    }
    else {/* Second Atom is not an equation */
      /* They can't be equal ! */
      result = ord_CompareAux(term_FirstArgument(Lit1),Lit2,FlagStore,Precedence, VarIsConst);
      if (!Orient1 && result != ord_GREATER_THAN) { 
	auxResult = ord_CompareAux(term_SecondArgument(Lit1),Lit2,FlagStore,Precedence, VarIsConst);
	if (auxResult == ord_GREATER_THAN)
	  result = ord_GREATER_THAN;
	else if (result != auxResult)
	  result = ord_UNCOMPARABLE;
      }
    }
  }
  else /* First Atom is not an equation */
    /* They can't be equal ! */
    if (fol_IsEquality(Lit2)) {
      result = ord_CompareAux(Lit1,term_FirstArgument(Lit2), FlagStore, Precedence, VarIsConst);
      if (!Orient2 && result != ord_SMALLER_THAN) { 
	auxResult = ord_CompareAux(Lit1,term_SecondArgument(Lit2),FlagStore,Precedence, VarIsConst);
	if (auxResult == ord_SMALLER_THAN)
	  result = ord_SMALLER_THAN;
	else if (result != auxResult)
	  result = ord_UNCOMPARABLE;
      }
    } else { /* None of the atoms is an equation */
      result = ord_CompareAux(Lit1,Lit2, FlagStore, Precedence, VarIsConst);
      if (result == ord_EQUAL) {
	if (pos1 && !pos2)
	  result = ord_SMALLER_THAN;
	else if (!pos1 && pos2)
	  result = ord_GREATER_THAN;
      }
    }

  return result;
}

ord_RESULT ord_LiteralCompare(TERM Lit1, ord_RESULT OrdStat1,
                              TERM Lit2, ord_RESULT OrdStat2,
			      BOOL Check, FLAGSTORE FlagStore,
			      PRECEDENCE Precedence)
/*********************************************************
  INPUT:   Two term literals and two ordering structures. The latter are only
           meaningful for equality and transitive literals and indicate
           how the left-hand side and right-hand side (in this order) terms
           of such a literal relate to each other.

	   Additionally, a check flag that indicates whether
	   the ordering stutuses are sufficient or necessary and sufficient:
	   If Check=TRUE then if the status is ord_UNCOMPARABLE it is interpreted
           that the real ordering status is not known.
           If Check=FALSE, we know that the terms are really uncomparable and that
           additional check wouldn't help.
	   A flag store.
	   A precedence.
  RETURNS: The result if these literals are compared with
           respect to the ordering. Dependent on their sign, the literals
	   are compared as multiset of their topsymbol terms,
	   where any literal is considered to be an equation
	   and non equational literals are considered to be
	   equations with the artificial, non-existent, minimal
	   constant tt.
           In the case when transitive predicates are involved
           an different approach to comparing the literals is used
           such that the resulting literal ordering is admissible for chaining.
  EFFECT:  The arguments of the literals are possibly, destructively flipped.
**********************************************************/
{
        return ord_LiteralCompareAux(Lit1, OrdStat1,
                                     Lit2, OrdStat2,
                                     Check, FALSE,
                        FlagStore, Precedence);
}
 


void ord_CompareCountVars(TERM Term, int Index)
/**********************************************************
 * INPUT: a term an integer
 * RETURNS: nothing
 * EFFECT: counts occurence of each variable x in term and
 *         saves it in ord_VARCOUNT[x][<Index>], 
 *         modifies ord_VARCOUNT
 **********************************************************/
{
  if (term_IsStandardVariable(Term)) {
    ord_VARCOUNT[term_TopSymbol(Term)][Index]++;
  }
  else {
    LIST Scan;
    for (Scan=term_ArgumentList(Term); !list_Empty(Scan); Scan=list_Cdr(Scan))
       ord_CompareCountVars(list_Car(Scan), Index);
  }
  return;
}

BOOL ord_CompareCheckVars(TERM Term)
/**********************************************************
 * INPUT:   a term
 * RETURNS: TRUE if <ord_VARCOUNT[i][0]> is greater 0 for
 *          all variables i occuring in <Term>,
 *          FALSE else
 ***********************************************************/
{
  BOOL Result;
  
  Result = TRUE;
  
  if (term_IsStandardVariable(Term) &&
       ord_VARCOUNT[term_TopSymbol(Term)][0] == 0) {
          return FALSE;
  }
  else {
    LIST Scan;
    for (Scan=term_ArgumentList(Term); !list_Empty(Scan) && Result; Scan=list_Cdr(Scan))
       if(!ord_CompareCheckVars(list_Car(Scan)))
                       return FALSE;
  }
  return TRUE;

}


BOOL ord_CompareVarsSubset(TERM Term1, TERM Term2)
/**********************************************************
 * INPUT:  two terms
 * OUTPUT: TRUE, if vars(Term2) is a subset of vars(Term1)
 *         else FALSE
 * EFFECT: modifies ord_VARCOUNT        
 **********************************************************/
{
  SYMBOL MaxVar1,MaxVar2;
  int    i;

  MaxVar1   = term_MaxVar(Term1);
  MaxVar2   = term_MaxVar(Term2);

  if (MaxVar1 < MaxVar2)
    return FALSE;

  for (i = 0; i <= MaxVar1; i++) {
    ord_VARCOUNT[i][0] = 0;
    ord_VARCOUNT[i][1] = 0;
  }

  /* Count variables of <Term1> */
  ord_CompareCountVars(Term1, 0);
  
  return ord_CompareCheckVars(Term2);  

}

BOOL ord_ContGreaterSkolemSubst(VARCONT C1, TERM T1, VARCONT C2, TERM T2,
		     FLAGSTORE FlagStore, PRECEDENCE P)
/**********************************************************
 * INPUT: two terms, two contexts
 * OUTPUT: TRUE, if T1 > T2 in their respective context such
 *         that variables are interpreted as fresh constants
 **********************************************************/
{ 

#ifdef CHECK
  if (fol_IsEquality(T1) || fol_IsEquality(T2)) {
    misc_StartErrorReport();
    misc_ErrorReport("\n In ord_ContGreater:");
    misc_ErrorReport("\n Illegal input. One input term is an equality.");
    misc_FinishErrorReport();
  }
#endif 
  
    ord_PRECEDENCE = P;
    switch(flag_GetFlagIntValue(FlagStore, flag_ORD)) {
    case flag_ORDKBO:  return kbo_ContGreaterSkolemSubst(C1, T1, C2, T2); break;
    case flag_ORDRPOS: return rpos_ContGreaterSkolemSubst(C1, C1, T1, C2, C2, T2); break;
    default:
      misc_StartErrorReport();
      misc_ErrorReport("\n In ord_ContGreaterSkolemSubst:");
      misc_ErrorReport("\n Illegal ordering type.");
      misc_FinishErrorReport();
    }
  return FALSE;   /* This line is never reached ...  */
}
