package nl.vu.kai.dl_lib.filtering

import com.typesafe.scalalogging.Logger
import nl.vu.kai.dl_lib.formatting.SimpleOWLFormatter
import org.semanticweb.owlapi.model.parameters.Imports
import org.semanticweb.owlapi.model.{AxiomType, OWLAxiom, OWLClass, OWLClassExpression, OWLDisjointClassesAxiom, OWLEquivalentClassesAxiom, OWLObjectAllValuesFrom, OWLObjectComplementOf, OWLObjectIntersectionOf, OWLObjectOneOf, OWLObjectProperty, OWLObjectPropertyDomainAxiom, OWLObjectPropertyRangeAxiom, OWLObjectSomeValuesFrom, OWLObjectUnionOf, OWLOntology, OWLOntologyManager, OWLSubClassOfAxiom, OWLSubClassOfAxiomSetShortCut, OWLSubClassOfAxiomShortCut, OWLSubPropertyAxiom}

import scala.collection.JavaConverters.{seqAsJavaListConverter, setAsJavaSetConverter}


abstract class OWLFilter {

  private val logger = Logger[OWLFilter]

  def axiomTypeSupported(axiom: OWLAxiom): Boolean
  def supported(expression: OWLClassExpression): Boolean

  def supported(axiom: OWLAxiom): Boolean = {
    axiomTypeSupported(axiom) && axiom.nestedClassExpressions().allMatch(supported)
  }

  def removeUnsupportedAxioms(ontology: OWLOntology): Unit = {
    var toRemove = List[OWLAxiom]()
    ontology.getAxioms(Imports.INCLUDED).forEach(axiom => {
      if(!supported(axiom))
        toRemove = axiom::toRemove
    })
    logger.debug(s"Unsupported axioms: ${toRemove.map(SimpleOWLFormatter.format)}")
    ontology.getOWLOntologyManager.removeAxioms(ontology, toRemove.toSet.asJava)
  }

  def filteredCopy(ontology: OWLOntology, manager: OWLOntologyManager)
  : OWLOntology = {
    val copy = manager.createOntology()
    ontology.getAxioms(Imports.INCLUDED).forEach(axiom => {
      if(supported(axiom))
        manager.addAxiom(copy, axiom)
    })
    copy
  }
}

object ELTBoxFilter extends OWLFilter {

  val logger = Logger(ELTBoxFilter.getClass)

  override def axiomTypeSupported(axiom: OWLAxiom) = {
    val result = axiom.isOfType(AxiomType.TBoxAndRBoxAxiomTypes) && (axiom match {
      case _ =>
        axiom.isInstanceOf[OWLSubClassOfAxiom] ||
          axiom.isInstanceOf[OWLEquivalentClassesAxiom]
    })
    if (!result)
      logger.debug(s"axiom type not supported: ${axiom}")
    result
  }

  override def supported(expression: OWLClassExpression) = {
    val result = expression.getObjectPropertiesInSignature.stream().allMatch(_.isInstanceOf[OWLObjectProperty]) &&
      (expression.isInstanceOf[OWLClass] ||
        expression.isInstanceOf[OWLObjectIntersectionOf] || expression.isInstanceOf[OWLObjectSomeValuesFrom])

    if (!result)
      logger.debug(s"class expression not supported: ${expression}")
    result
  }
}

object ALCHTBoxFilter extends OWLFilter {

  val logger = Logger(ALCHTBoxFilter.getClass)

  override def axiomTypeSupported(axiom: OWLAxiom) = {
    val result = axiom.isOfType(AxiomType.TBoxAndRBoxAxiomTypes) && (axiom match {
      case sp: OWLSubPropertyAxiom[_] =>
        sp.getSubProperty.isInstanceOf[OWLObjectProperty] && sp.getSuperProperty.isInstanceOf[OWLObjectProperty]
      case _ =>
        axiom.isInstanceOf[OWLSubClassOfAxiom] ||
          axiom.isInstanceOf[OWLSubClassOfAxiomShortCut] || axiom.isInstanceOf[OWLSubClassOfAxiomSetShortCut] ||
          axiom.isInstanceOf[OWLDisjointClassesAxiom] ||
        axiom.isInstanceOf[OWLObjectPropertyDomainAxiom] || axiom.isInstanceOf[OWLObjectPropertyRangeAxiom]
    })
    if(!result)
      logger.debug(s"axiom type not supported: ${axiom}")
    result
  }

  override def supported(expression: OWLClassExpression) = {
    val result = expression.getObjectPropertiesInSignature.stream().allMatch(_.isInstanceOf[OWLObjectProperty]) &&
      (expression.isInstanceOf[OWLClass] || expression.isInstanceOf[OWLObjectUnionOf] ||
      expression.isInstanceOf[OWLObjectIntersectionOf] || expression.isInstanceOf[OWLObjectSomeValuesFrom] ||
      expression.isInstanceOf[OWLObjectAllValuesFrom] || expression.isInstanceOf[OWLObjectComplementOf])

    if(!result)
      logger.debug(s"class expression not supported: ${expression}")
    result
  }
}


object ALCTBoxFilter extends OWLFilter {

  val logger = Logger(ALCHTBoxFilter.getClass)

  override def axiomTypeSupported(axiom: OWLAxiom) = {
    val result = axiom.isOfType(AxiomType.TBoxAndRBoxAxiomTypes) && (axiom match {
      case _ =>
        axiom.isInstanceOf[OWLSubClassOfAxiom] ||
          axiom.isInstanceOf[OWLSubClassOfAxiomShortCut] || axiom.isInstanceOf[OWLSubClassOfAxiomSetShortCut] ||
          axiom.isInstanceOf[OWLDisjointClassesAxiom] ||
          axiom.isInstanceOf[OWLObjectPropertyDomainAxiom] || axiom.isInstanceOf[OWLObjectPropertyRangeAxiom]
    })
    if(!result)
      logger.debug(s"axiom type not supported: ${axiom}")
    result
  }

  override def supported(expression: OWLClassExpression) = {
    val result = expression.getObjectPropertiesInSignature.stream().allMatch(_.isInstanceOf[OWLObjectProperty]) &&
      (expression.isInstanceOf[OWLClass] || expression.isInstanceOf[OWLObjectUnionOf] ||
        expression.isInstanceOf[OWLObjectIntersectionOf] || expression.isInstanceOf[OWLObjectSomeValuesFrom] ||
        expression.isInstanceOf[OWLObjectAllValuesFrom] || expression.isInstanceOf[OWLObjectComplementOf])

    if(!result)
      logger.debug(s"class expression not supported: ${expression}")
    result
  }
}

object ALCOITBoxFilter extends OWLFilter {
  override def axiomTypeSupported(axiom: OWLAxiom) = {
    axiom.isOfType(AxiomType.TBoxAndRBoxAxiomTypes) && (axiom match {
      case _ =>
        axiom.isInstanceOf[OWLSubClassOfAxiom] ||
          axiom.isInstanceOf[OWLSubClassOfAxiomShortCut] || axiom.isInstanceOf[OWLSubClassOfAxiomSetShortCut] ||
          axiom.isInstanceOf[OWLObjectPropertyDomainAxiom] || axiom.isInstanceOf[OWLObjectPropertyRangeAxiom]
    })

  }

  override def supported(expression: OWLClassExpression) = {
    expression.isInstanceOf[OWLClass] || expression.isInstanceOf[OWLObjectUnionOf] ||
      expression.isInstanceOf[OWLObjectIntersectionOf] || expression.isInstanceOf[OWLObjectSomeValuesFrom] ||
      expression.isInstanceOf[OWLObjectAllValuesFrom] || expression.isInstanceOf[OWLObjectOneOf] ||
      expression.isInstanceOf[OWLObjectComplementOf]
  }
}