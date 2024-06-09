/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *        DFG LEXER TOKENS                                * */
/* *                                                        * */ 
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
/* $Revision: 1.0 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2016/03/23 16:57:37 $                             * */
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


/**
 * Tokens with no or varying text representation.
 */
TOKEN(EOF,        "end of input")
TOKEN(FILENAME,   "filename")
TOKEN(IDENTIFIER, "identifier")
TOKEN(NUMBER,     "number")
TOKEN(TEXT,       "text")

/**
 * Punctuators.
 */
TOKEN(COLON,         ":")
TOKEN(COMMA,         ",")
TOKEN(LBRACKET,      "[")
TOKEN(LPAREN,        "(")
TOKEN(LPARSTAR,      "{*")
TOKEN(LCBRACE,      "{")
TOKEN(MINUS_GREATER, "->")
TOKEN(PERIOD,        ".")
TOKEN(PIPE_PIPE,     "||")
TOKEN(PLUS,         "+")
TOKEN(RBRACKET,      "]")
TOKEN(RPAREN,        ")")
TOKEN(RPARSTAR,      "*}")
TOKEN(RCBRACE,      "}")

/**
 * Keywords.
 *
 * These are identified automatically when an identifier is read.
 */
KEYWORD(MIN)
KEYWORD(AED)
KEYWORD(App)
KEYWORD(CRW)
KEYWORD(Con)
KEYWORD(Def)
KEYWORD(EmS)
KEYWORD(EqF)
KEYWORD(EqR)
KEYWORD(Fac)
KEYWORD(Inp)
KEYWORD(MPm)
KEYWORD(MRR)
KEYWORD(OHy)
KEYWORD(OPm)
KEYWORD(Obv)
KEYWORD(Res)
KEYWORD(Rew)
KEYWORD(SHy)
KEYWORD(SPASS)
KEYWORD(SPm)
KEYWORD(SSi)
KEYWORD(SoR)
KEYWORD(SpL)
KEYWORD(SpR)
KEYWORD(Spt)
KEYWORD(Ter)
KEYWORD(Top)
KEYWORD(URR)
KEYWORD(UnC)
KEYWORD(and)
KEYWORD(author)
KEYWORD(axioms)
KEYWORD(begin_problem)
KEYWORD(by)
KEYWORD(clause)
KEYWORD(cnf)
KEYWORD(concept_formula)
KEYWORD(conjectures)
KEYWORD(date)
KEYWORD(description)
KEYWORD(dnf)
KEYWORD(eml)
KEYWORD(end_of_list)
KEYWORD(end_problem)
KEYWORD(equal)
KEYWORD(equiv)
KEYWORD(exists)
KEYWORD(extrafuns)
KEYWORD(false)
KEYWORD(freely)
KEYWORD(forall)
KEYWORD(formula)
KEYWORD(functions)
KEYWORD(generated)
KEYWORD(hypothesis)
KEYWORD(implied)
KEYWORD(implies)
KEYWORD(include)
KEYWORD(list_of_clauses)
KEYWORD(list_of_declarations)
KEYWORD(list_of_descriptions)
KEYWORD(list_of_formulae)
KEYWORD(list_of_general_settings)
KEYWORD(list_of_includes)
KEYWORD(list_of_proof)
KEYWORD(list_of_settings)
KEYWORD(list_of_special_formulae)
KEYWORD(list_of_symbols)
KEYWORD(logic)
KEYWORD(name)
KEYWORD(nand)
KEYWORD(nor)
KEYWORD(not)
KEYWORD(operators)
KEYWORD(or)
KEYWORD(predicate)
KEYWORD(predicates)
KEYWORD(prop_formula)
KEYWORD(quantifiers)
KEYWORD(rel_formula)
KEYWORD(role_formula)
KEYWORD(satisfiable)
KEYWORD(set_flag)
KEYWORD(set_precedence)
KEYWORD(set_selection)
KEYWORD(set_ClauseFormulaRelation)
KEYWORD(set_DomPred)
KEYWORD(sort)
KEYWORD(sorts)
KEYWORD(splitlevel)
KEYWORD(status)
KEYWORD(step)
KEYWORD(subsort)
KEYWORD(translpairs)
KEYWORD(true)
KEYWORD(unknown)
KEYWORD(unsatisfiable)
KEYWORD(version)
KEYWORD(xor)
