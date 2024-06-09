package nl.vu.kai.dl_lib

import nl.vu.kai.dl_lib.datatypes._

object ELFactory {

  def getConceptName(name: String) =
    ConceptName(name)

  def getTop() =
    TopConcept


  def getConjunction(first: Concept, second: Concept) =
    ConceptConjunction(Seq(first,second))

  def getRole(name: String) =
    RoleName(name)

  def getExistentialRoleRestriction(role: Role, filler: Concept) =
    ExistentialRoleRestriction(role,filler)

  def getGCI(lhs: Concept, rhs: Concept) =
    GeneralConceptInclusion(lhs,rhs)

  def getBottom() = BottomConcept

  def getIndividual(name: String) = Individual(name)

  def getConceptAssertion(concept: Concept, individual: Individual) = ConceptAssertion(concept,individual)

  def getRoleAssertion(role: Role, ind1: Individual, ind2: Individual) = RoleAssertion(role,ind1, ind2)
}
