package nl.vu.kai.dl_lib.owlapi

import java.io.File
import java.net.URL
import scala.collection.JavaConversions._
import com.typesafe.scalalogging.Logger
import org.semanticweb.owlapi.apibinding.OWLManager
import org.semanticweb.owlapi.io._
import org.semanticweb.owlapi.model._
import nl.vu.kai.dl_lib.datatypes.{Annotation, LabelAnnotation, _}

class OWLExporter(simplifiedNames: Boolean = true) {
//  implicit val (logger, formatter, appender) = ZeroLoggerFactory.newLogger(this)

  val logger = Logger[OWLExporter]

  val manager = OWLManager.createOWLOntologyManager()

  val factory = manager.getOWLDataFactory()

  def exportOntology(ontology: Ontology, filename: String): Unit = {
    exportOntology(ontology, new File(filename))
  }

  def exportOntology(ontology: Ontology, file: File): Unit = {

    val owlOntology = toOwlOntology(ontology)

    val format = new RDFXMLOntologyFormat()

    manager.saveOntology(owlOntology, format, IRI.create(file.toURI()))
  }

  def toOwlOntology(ontology: Ontology): OWLOntology = {
    val owlOntology = manager.createOntology()

    ontology.tbox.axioms.foreach(addAxiom(owlOntology, _))
    ontology.rbox.axioms.foreach(addAxiom(owlOntology, _))
    ontology.abox.assertions.foreach(addAxiom(owlOntology,_))
    ontology.annotations.foreach(addAxiom(owlOntology,_))
    ontology.unsupportedOWLAxioms.foreach(owlOntology.addAxiom)
    owlOntology
  }


  def save(owlOntology: OWLOntology, file: File,
  format:OWLOntologyFormat = new OWLXMLOntologyFormat()) = {
    val manager = OWLManager.createOWLOntologyManager()

    manager.saveOntology(owlOntology, format, IRI.create(file.toURI()))    
  }

  def addAxiom(owlOntology: OWLOntology, axiom: Axiom) = 
    manager.addAxiom(owlOntology, toOwl(owlOntology, axiom))

  def addAxiom(owlOntology: OWLOntology, axiom: RoleAxiom) = 
    manager.addAxiom(owlOntology, toOwl(owlOntology, axiom))

  def addAxiom(owlOntology: OWLOntology, assertion: Assertion) = 
    manager.addAxiom(owlOntology, toOwl(owlOntology, assertion))

  def addAxiom(owlOntology: OWLOntology, annotation: Annotation) =
    manager.addAxiom(owlOntology, toOwl(owlOntology, annotation))

  def toOwl(owlOntology: OWLOntology, statement: DLStatement): OWLLogicalAxiom = statement match {
    case a: Axiom => toOwl(owlOntology, a)
    case ra: RoleAxiom => toOwl(owlOntology, ra)
    case as: Assertion => toOwl(owlOntology, as)
  }

  def toOwl(owlOntology: OWLOntology, axiom: Axiom): OWLLogicalAxiom = axiom match { 
    case GeneralConceptInclusion(subsumer, subsumee) =>
      factory.getOWLSubClassOfAxiom(toOwl(owlOntology, subsumer),
				    toOwl(owlOntology, subsumee))
    case EquivalenceAxiom(cs) =>
      factory.getOWLEquivalentClassesAxiom(cs.toSet[Concept].map(toOwl(owlOntology, _)))
    case DisjointnessAxiom(concepts) =>
      factory.getOWLDisjointClassesAxiom(concepts.toSet[Concept].map(toOwl(owlOntology,_)))
    case DomainAxiom(r,c) =>
      factory.getOWLObjectPropertyDomainAxiom(toOwl(owlOntology,r),
        toOwl(owlOntology, c))
    case RangeAxiom(r,c) =>
      factory.getOWLObjectPropertyRangeAxiom(toOwl(owlOntology,r),
        toOwl(owlOntology, c))
  }

  def toOwl(owlOntology: OWLOntology, axiom: RoleAxiom): OWLLogicalAxiom = axiom match { 
    case RoleInclusion(r1, r2) => factory.getOWLSubObjectPropertyOfAxiom(toOwl(owlOntology, r1),
									   toOwl(owlOntology, r2))
    case RoleChainAxiom(sub, sup) => factory.getOWLSubPropertyChainOfAxiom(sub.map(toOwl(owlOntology, _)),
      toOwl(owlOntology, sup))
    case TransitiveRoleAxiom(r) => factory.getOWLTransitiveObjectPropertyAxiom(toOwl(owlOntology,r))
    case FunctionalRoleAxiom(r) => factory.getOWLFunctionalObjectPropertyAxiom(toOwl(owlOntology, r))
    case SymmetricRoleAxiom(r) => factory.getOWLSymmetricObjectPropertyAxiom(toOwl(owlOntology,r))
  }

  def toOwl(owlOntology: OWLOntology, assertion: Assertion): OWLIndividualAxiom = assertion match { 
    case ConceptAssertion(c, a) => factory.getOWLClassAssertionAxiom(toOwl(owlOntology, c), 
								     toOwl(owlOntology, a))
    case RoleAssertion(r, a, b) => factory.getOWLObjectPropertyAssertionAxiom(toOwl(owlOntology, r), 
									      toOwl(owlOntology, a), 
									      toOwl(owlOntology, b))
  }

  def toOwl(owlOntology: OWLOntology, concept: Concept): OWLClassExpression = concept match { 
    case TopConcept => factory.getOWLThing()
    case BottomConcept => factory.getOWLNothing()
    case ConceptName(name) => factory.getOWLClass(toIRI(owlOntology, name))
    case ConceptComplement(concept) => 
      factory.getOWLObjectComplementOf(toOwl(owlOntology, concept))
    //case ConceptConjunction(conjuncts) if conjuncts.size==0 => toOwl(owlOntology, TopConcept)
    //case ConceptConjunction(conjuncts) if conjuncts.size==1 => toOwl(owlOntology, conjuncts.head)
    case ConceptConjunction(conjuncts) => 
      factory.getOWLObjectIntersectionOf(conjuncts.toSet[Concept].map(toOwl(owlOntology,_)))
    //case ConceptDisjunction(disjuncts) if disjuncts.size==0 => toOwl(owlOntology, BottomConcept)
    //case ConceptDisjunction(disjuncts) if disjuncts.size==1 => toOwl(owlOntology, disjuncts.head)
    case ConceptDisjunction(disjuncts) => 
    //  assert(disjuncts.size>1, "invalid disjunction: only contains: "+disjuncts)
      factory.getOWLObjectUnionOf(disjuncts.toSet[Concept].map(toOwl(owlOntology,_)))
    case ExistentialRoleRestriction(role, concept) =>
      factory.getOWLObjectSomeValuesFrom(toOwl(owlOntology, role), toOwl(owlOntology, concept))
    case UniversalRoleRestriction(role, concept) =>
      factory.getOWLObjectAllValuesFrom(toOwl(owlOntology, role), toOwl(owlOntology, concept))
    case MinNumberRestriction(n, role, concept) =>
      factory.getOWLObjectMinCardinality(n, toOwl(owlOntology, role), toOwl(owlOntology, concept))
    case MaxNumberRestriction(n, role, concept) =>
      factory.getOWLObjectMaxCardinality(n, toOwl(owlOntology, role), toOwl(owlOntology, concept))
    case EqualNumberRestriction(n, role, concept) =>
      factory.getOWLObjectExactCardinality(n, toOwl(owlOntology, role), toOwl(owlOntology, concept))

    case NominalSet(individuals: Seq[Individual]) =>
      factory.getOWLObjectOneOf(individuals.toSet[Individual].map(toOwl(owlOntology, _)))
  }

  def toOwl(owlOntology: OWLOntology, role: Role): OWLObjectPropertyExpression = role match {
    case TopRole => factory.getOWLTopObjectProperty()
    case RoleName(name) => factory.getOWLObjectProperty(toIRI(owlOntology, name))
    case InverseRole(role) => factory.getOWLObjectInverseOf(toOwl(owlOntology, role).asInstanceOf[OWLObjectProperty])
  }

  def toOwl(owlOntology: OWLOntology, individual: Individual): OWLIndividual =
    factory.getOWLNamedIndividual(toIRI(owlOntology, individual.name))

  def toOwl(owlOntology: OWLOntology, annotation: Annotation): OWLAnnotationAssertionAxiom = annotation match {

    case TaxonAnnotation(name: Name, ref:String) =>
      factory.getOWLAnnotationAssertionAxiom(
        getTaxonProperty(),
        toIRI(owlOntology,name.nameAsString()),
        factory.getOWLLiteral(ref)
      )
    case LabelAnnotation(name: Name, label, language) =>
      factory.getOWLAnnotationAssertionAxiom(
        getLabelProperty(),
        toIRI(owlOntology,name.nameAsString()),
        factory.getOWLLiteral(label, language)
      )
    case SeeAlsoAnnotation(name: Name, ref: String) =>
      factory.getOWLAnnotationAssertionAxiom(
        getSeeAlsoProperty(),
        toIRI(owlOntology,name.nameAsString()),
        factory.getOWLLiteral(ref)
      )

    case other => throw new AssertionError("Unsupported annotation type: "+other)
  }

  def getLabelProperty() = factory.getOWLAnnotationProperty("http://www.w3.org/2000/01/rdf-schema#label")

  def getSeeAlsoProperty() = factory.getOWLAnnotationProperty("https://www.w3.org/TR/rdf-schema#seeAlso") //maybe add '/'

  def getTaxonProperty() = factory.getOWLAnnotationProperty("http://rs.tdwg.org/dwc/terms/scientificName")
  def toIRI(owlOntology: OWLOntology, name: String): IRI = 
    if(!simplifiedNames && name.startsWith("<") && name.endsWith(">")) {
      IRI.create(name.substring(1,name.length-1))
    } else if(!simplifiedNames) {
      IRI.create(name)
    } else {
      // val ontIRI = owlOntology.getOntologyID().getOntologyIRI() // <-- needed?
      // IRI.create(ontIRI.getNamespace(), name)
      IRI.create("http://example.com/ns/foo#"+name)
  }
}
