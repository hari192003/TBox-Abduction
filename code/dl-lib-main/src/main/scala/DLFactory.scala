package nl.vu.kai.dl_lib

import nl.vu.kai.dl_lib.datatypes._

object DLFactory {

  def getOntology() =
    new Ontology()

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

  def conceptAssertion(concept: Concept, individual: Individual) = {
    new ConceptAssertion(concept,individual)
  }

  def roleAssertion(role: Role, individual1: Individual, individual2: Individual) =
    new RoleAssertion(role, individual1, individual2)

  def conceptName(name: String) = ConceptName(name)

  def roleName(name: String) = RoleName(name)

  def individual(name: String) = Individual(name)

  def disjointnessAxiom (first: Concept, second: Concept) =
      DisjointnessAxiom(Seq(first, second))

  def getLabelAnnotation(name: Name, label: String, language: String) =
    LabelAnnotation(name,label,language)

  def getSeeAlsoAnnotation(name:Name, ref:String) =
    SeeAlsoAnnotation(name,ref)

  def getTaxonAnnotation(name:Name, ref:String) =
    TaxonAnnotation(name,ref)
}


