package nl.vu.kai.dl_lib.reasoning

import nl.vu.kai.dl_lib.datatypes.{Axiom, Concept, ConceptName, DLStatement, Ontology}
import nl.vu.kai.dl_lib.owlapi.{OWLApiConverter, OWLExporter}
import org.semanticweb.HermiT
import org.semanticweb.elk
import org.semanticweb.elk.reasoner.ReasonerFactory
import org.semanticweb.owlapi.model.{OWLClass, OWLClassExpression, OWLOntology}
import org.semanticweb.owlapi.reasoner.{InferenceType, OWLReasoner, OWLReasonerFactory, ReasonerInternalException}

import java.util
import java.util.stream.Collectors


object DLReasoners {
  def getHermiTReasoner() = new DLReasoner(new HermiT.ReasonerFactory())
  def getELKReasoner() = new DLReasoner( new elk.owlapi.ElkReasonerFactory())
}

class DLReasoner(reasonerFactory: OWLReasonerFactory) {
  private var reasoner: OWLReasoner = _
  private val exporter = new OWLExporter()
  private val converter = new OWLApiConverter()
  private var owlOntology: OWLOntology = _
  private var ontologySet = false
  def setOntology(ontology: Ontology) = {
    owlOntology=exporter.toOwlOntology(ontology)
    reasoner = reasonerFactory.createReasoner(owlOntology)
    ontologySet=true
  }

  /**
   * returns all subsumers of the given concept that are either concept names, top or bottom
   * @param concept
   * @return
   */
  def getSubsumers(concept: Concept): java.util.Set[Concept] = {
    checkOntologySet()
    val classExpression = exporter.toOwl(owlOntology, concept)
    getSubsumers(classExpression)
  }

  private def getSubsumers(classExp: OWLClassExpression): java.util.Set[Concept] =
  {
    checkOntologySet()
    val result: java.util.Set[Concept] = new util.HashSet()

    reasoner.getSuperClasses(classExp)
     .nodes()
     .forEach(node =>
       node.entities().forEach(x =>
         result.add(converter.convert(x))
       )
     )
//        .map(c => converter.convert(c.asInstanceOf[OWLClass]))
//        .collect(Collectors.toSet())
    reasoner.getEquivalentClasses(classExp)
      .entities()
      .forEach(x => result.add(converter.convert(x)))

    result
  }

  def classify(): java.util.Map[Concept, java.util.Set[Concept]] = {
    checkOntologySet()
    reasoner.precomputeInferences(InferenceType.CLASS_HIERARCHY)

    val result = new util.HashMap[Concept,java.util.Set[Concept]]();
    owlOntology.classesInSignature().forEach{cl =>
      val concept = converter.convert(cl)
      val subsumers = getSubsumers(cl)
      result.put(concept,subsumers)
    }
    result
  }

  def checkOntologySet() =
    if(!ontologySet) throw new ReasonerException("No ontology set!")

  def isEntailed(axiom: DLStatement) =
    reasoner.isEntailed(exporter.toOwl(owlOntology,axiom))
}

class ReasonerException(message: String) extends Exception(message)
