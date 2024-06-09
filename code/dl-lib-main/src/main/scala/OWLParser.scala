
package nl.vu.kai.dl_lib.owlapi

import java.io.File
import java.net.URL
import java.util.Date

import com.typesafe.scalalogging.Logger

import scala.collection.JavaConverters._

import org.semanticweb.owlapi.apibinding.OWLManager
import org.semanticweb.owlapi.model._
import org.semanticweb.owlapi.model.parameters.Imports
import org.semanticweb.owlapi.vocab.SKOSVocabulary

import nl.vu.kai.dl_lib.datatypes._



object OntologyPrinter { 

  import nl.vu.kai.dl_lib.formatting.SimpleDLFormatter
    

  def main(args: Array[String]) = { 

//    println("Loading "+args(0))
//    println(new Date())
    val file = new File(args(0))


    val manager = OWLManager.createOWLOntologyManager();
    
    val ontology = manager.loadOntologyFromOntologyDocument(file)

    val factory = manager.getOWLDataFactory()



   //  val prefLabel = factory.getOWLAnnotationProperty(SKOSVocabulary.PREFLABEL.getIRI())

   //  ontology.getClassesInSignature().forEach
   //  { _class => 
   //    println(_class)
   //    ontology.getAnnotationAssertionAxioms(_class.getIRI()).asScala.foreach
   //    { anno => 
	
   // 	if( anno.getProperty().equals(prefLabel)){
   // 	  anno.getValue() match {
   // 	    case lit: OWLLiteral if lit.getLang().equals("en") =>
   //            System.out.println( lit.getLiteral() )
   // 	    case _ => ; // do nothing	  }
   // 	  }
   // 	}
   //   }
   // }

   //  System.exit(0)

//    println(new Date())
//    println("Loaded ontology: " + ontology.toString());

    val converter = new OWLApiConverter(referenceOntology = Some(ontology));

    ontology.getLogicalAxioms(Imports.EXCLUDED).forEach{ axiom => {
      converter.convert(axiom).foreach(ax => println(SimpleDLFormatter.format(ax)))
    }}
//    println(new Date())
    
  }
}

class OWLParser(converter: OWLApiConverter) {
//  implicit val (logger, formatter, appender) = ZeroLoggerFactory.newLogger(this)

  val logger = Logger(this.getClass)

  def this() = this(new OWLApiConverter())

  def parseURL(url: String): Ontology = { 
    val manager = OWLManager.createOWLOntologyManager();
    val iri = IRI.create(url)

    val ontology = manager.loadOntologyFromOntologyDocument(iri);


    logger.info("Loaded ontology: " + ontology.toString());
    
    converter.convert(ontology)
  }

  def parseURL(url: URL): Ontology = { 
    val manager = OWLManager.createOWLOntologyManager();
    val iri = IRI.create(url)

    val ontology = manager.loadOntologyFromOntologyDocument(iri);

    logger.info("Loaded ontology: " + ontology.toString());
    
    converter.convert(ontology)
  }


  def parseFile(file: File): Ontology = { 
    val manager = OWLManager.createOWLOntologyManager();
    
    val ontology = manager.loadOntologyFromOntologyDocument(file)

    logger.info("Loaded ontology: " + ontology.toString());
    
    converter.convert(ontology)
  }

  def parseFile(filename: String): Ontology = {
    parseFile(new File(filename))
  }
}


class OWLApiConverter(simplifiedNames: Boolean = false,
		    var referenceOntology: Option[OWLOntology] = None) {
//  implicit val (logger, formatter, appender) =
//  ZeroLoggerFactory.newLogger(this)


  val logger = Logger(this.getClass)

  val manager = OWLManager.createOWLOntologyManager()
  val factory = manager.getOWLDataFactory()
  val prefLabelAnnotationProperty = 
    factory.getOWLAnnotationProperty(SKOSVocabulary.PREFLABEL.getIRI())

  var unsupportedAxiomsEncountered = Set[OWLAxiom]()

  def convert(owlOntology: OWLOntology): Ontology = {
    val oldReferenceOntology = referenceOntology
    referenceOntology = Some(owlOntology)

    unsupportedAxiomsEncountered = Set()

    val result = new Ontology
    val statements =
      owlOntology.getAxioms(Imports.EXCLUDED).asScala
      /*owlOntology.getTBoxAxioms(Imports.EXCLUDED).asScala.toSet++
      owlOntology.getABoxAxioms(Imports.EXCLUDED).asScala++
      owlOntology.getRBoxAxioms(Imports.EXCLUDED).asScala*/
    val statementsNr = statements.size

    val s1 = statements.map(convert)
    s1.flatten.foreach{ st =>
      logger.debug("Converted statement: " + st.toString)
      result.addStatement(st)
    }
    unsupportedAxiomsEncountered.foreach(result.addUnsupportedOWLAxiom)

    logger.debug("Difference in statements size after conversion: "+(result.statements.size-statementsNr))
    referenceOntology = oldReferenceOntology
    result
  }

  def convert(owlAxioms: Set[OWLAxiom]): Set[DLStatement] = { 
    owlAxioms.map(convert).flatten.map{ st =>
      logger.debug("Converted statement: " + st.toString)
      st
    }
  }

  def convert(owlAxiom: OWLAxiom): Set[DLStatement] = { 
    val result: Set[DLStatement] = 
    try { 
      owlAxiom match { 
	    case axiom: OWLSubClassOfAxiom =>
          Set(GeneralConceptInclusion(convert(axiom.getSubClass()), convert(axiom.getSuperClass())))
	    case axiom: OWLEquivalentClassesAxiom =>
        Set(EquivalenceAxiom(axiom.getClassExpressions().asScala.toSeq.map((c: OWLClassExpression) => convert(c))))
	    case axiom: OWLDisjointClassesAxiom =>
        Set(DisjointnessAxiom(axiom.getClassExpressions().asScala.toSeq.map((c: OWLClassExpression) => convert(c))))

	    case axiom: OWLDisjointUnionAxiom =>
          convert(axiom.getOWLDisjointClassesAxiom()) ++
          convert(axiom.getOWLEquivalentClassesAxiom())
	    case axiom: OWLClassAssertionAxiom =>
          Set(ConceptAssertion(
            convert(axiom.getClassExpression()),
            convert(axiom.getIndividual)))
	    case axiom: OWLObjectPropertyAssertionAxiom =>
	      Set(RoleAssertion(
            convert(axiom.getProperty()),
			convert(axiom.getSubject()),
			convert(axiom.getObject())))
	    case axiom: OWLObjectPropertyDomainAxiom =>
          Set(DomainAxiom(convert(axiom.getProperty), convert(axiom.getDomain)))
	    case axiom: OWLObjectPropertyRangeAxiom =>
          Set(RangeAxiom(convert(axiom.getProperty), convert(axiom.getRange)))
	    case axiom: OWLSubObjectPropertyOfAxiom =>
          Set(RoleInclusion(
            convert(axiom.getSubProperty),
            convert(axiom.getSuperProperty)))
      case axiom: OWLSubPropertyChainOfAxiom =>
          Set(RoleChainAxiom(
            axiom.getPropertyChain.asScala.map(convert).toList,
            convert(axiom.getSuperProperty)))
	    case axiom: OWLInverseObjectPropertiesAxiom =>
	      axiom.asSubObjectPropertyOfAxioms.asScala.toSet[OWLAxiom].flatMap(convert)
	    case axiom: OWLTransitiveObjectPropertyAxiom =>
	      Set(TransitiveRoleAxiom(convert(axiom.getProperty)))
	    case axiom: OWLFunctionalObjectPropertyAxiom =>
	      Set(GeneralConceptInclusion(
            TopConcept,
            MaxNumberRestriction(1, convert(axiom.getProperty), TopConcept)))
        case axiom: OWLSubClassOfAxiomShortCut =>
          convert(axiom.asOWLSubClassOfAxiom)
        case axiom: OWLSymmetricObjectPropertyAxiom =>
          Set(SymmetricRoleAxiom(convert(axiom.getProperty)))
	    case other => {
	      logger.warn("ignored axiom (not supported) " + owlAxiom.toString() )
        unsupportedAxiomsEncountered += other
	      Set()
	    }
      }
    } catch {
      case e: Throwable =>
        e.printStackTrace;
        logger.warn("ignored axiom because of exception: " + owlAxiom.toString())
        unsupportedAxiomsEncountered += owlAxiom
      Set()
    }

    result
  }


  def convert(classExpression: OWLClassExpression): Concept = classExpression match {
    case c if c.isOWLThing => TopConcept
    case c if c.isOWLNothing => BottomConcept 
    case concept: OWLClass => ConceptName(getName(concept))
    case complement: OWLObjectComplementOf => ConceptComplement(convert(complement.getOperand()))
    case intersection: OWLObjectIntersectionOf =>
      ConceptConjunction(intersection.getOperands().asScala.map(convert).toSeq)
    case union: OWLObjectUnionOf => ConceptDisjunction(union.getOperands().asScala.map(convert).toSeq)
    case restriction: OWLObjectSomeValuesFrom => 
      ExistentialRoleRestriction(convert(restriction.getProperty), convert(restriction.getFiller)) 
    case restriction: OWLObjectAllValuesFrom =>
      UniversalRoleRestriction(convert(restriction.getProperty), convert(restriction.getFiller))
//    case restriction: OWLObjectMinCardinality if restriction.getCardinality==0 => TopConcept
    case restriction: OWLObjectMinCardinality => 
      MinNumberRestriction(restriction.getCardinality, convert(restriction.getProperty), convert(restriction.getFiller))
    case restriction: OWLObjectMaxCardinality =>
      MaxNumberRestriction(
        restriction.getCardinality,
        convert(restriction.getProperty),
        convert(restriction.getFiller))
    case restriction: OWLObjectExactCardinality =>
      ConceptConjunction(Seq(
        MaxNumberRestriction(
          restriction.getCardinality,
		  convert(restriction.getProperty),
		  convert(restriction.getFiller)),
		MinNumberRestriction(
          restriction.getCardinality,
		  convert(restriction.getProperty),
		  convert(restriction.getFiller))))
    case restriction: OWLObjectHasValue =>
      ExistentialRoleRestriction(
        convert(restriction.getProperty),
        NominalSet(Seq(convert(restriction.getFiller))))
    case oneOf: OWLObjectOneOf =>
      NominalSet(oneOf.getIndividuals().asScala.toSeq.map{i: OWLIndividual =>
        convert(i)})
    case restriction: OWLDataAllValuesFrom => // WARNING: not really correct
      UniversalRoleRestriction(
        convert(restriction.getProperty),
        ConceptName(restriction.getFiller.toString))
    case restriction: OWLDataSomeValuesFrom => // WARNING: not really correct
      ExistentialRoleRestriction(
        convert(restriction.getProperty),
        ConceptName(restriction.getFiller.toString))
    case restriction: OWLDataHasValue => // WARNING: not really correct
      ExistentialRoleRestriction(
        convert(restriction.getProperty),
        ConceptName("{"+restriction.getFiller.toString+"}"))
    case restriction: OWLDataExactCardinality => // WARNING: not really correct
      ConceptConjunction(Seq(
        MaxNumberRestriction(
          restriction.getCardinality,
		  convert(restriction.getProperty),
		  ConceptName(restriction.getFiller.toString)),
		MinNumberRestriction(
          restriction.getCardinality,
		  convert(restriction.getProperty),
                  ConceptName(restriction.getFiller.toString))))
  }

  def convert(property: OWLPropertyExpression): Role = property match {
    case property if property.isOWLTopObjectProperty => TopRole
    case property: OWLObjectProperty => RoleName(getName(property))
    case property: OWLObjectInverseOf =>
      InverseRole(convert(property.getInverse))
    case property: OWLDataProperty => RoleName(getName(property))
  }

  def convert(individual: OWLIndividual): Individual = individual match { 
    case individual: OWLNamedIndividual => Individual(getName(individual))
    case individual: OWLAnonymousIndividual => Individual(individual.getID.getID)
  }

  def getName(owlObject: OWLEntity): String = 
    {
      if(!simplifiedNames){
        return owlObject.getIRI.toString
      }
 //     getPreferedLabel(owlObject.getIRI) match {
 //	case Some(label) => "\"" + label+ "\""
	//case _ =>
      var opt = getEnglishLabel(owlObject.getIRI)
      if(opt.isEmpty)
        opt = getPreferedLabel(owlObject.getIRI)
      if(opt.isEmpty)
        opt = getLabel(owlObject.getIRI)
      val result = opt match {
	      case Some(label) => "\"" + label + "\""
        case _ =>
	        if(simplifiedNames) {
              val iri = owlObject.getIRI.toString
              if(iri.contains('#'))
                iri.split('#').last
              else
                iri.split('/').last
	        // owlObject.getIRI.getFragment
            }
	        else
	          owlObject.toString
	    }

      if(result.trim().equals(""))
        owlObject.toString
      else
        result
    }

  def getPreferedLabel(iri: IRI) = {
    var result: Option[String] = None
    referenceOntology.foreach(
      _.getAnnotationAssertionAxioms(iri).asScala.foreach 
      {anno =>
	if(anno.getProperty().equals(prefLabelAnnotationProperty))
	  anno.getValue() match {
	    case lit: OWLLiteral => result = Some(lit.getLiteral())
	    case other => ; // do nothing
	  }
     }
    )
    result
  }

  def getEnglishLabel(iri: IRI) = {
    var result: Option[String] = None
    referenceOntology.foreach(
      _.getAnnotationAssertionAxioms(iri).asScala.foreach 
      {anno =>
	if(anno.getProperty().equals(factory.getRDFSLabel()))
	  anno.getValue() match {
	    case lit: OWLLiteral if lit.getLang().equals("en") => 
	      result = Some(lit.getLiteral())
	    case other => ; // do nothing
	  }
     }
    )
    result
  }

  def getLabel(iri: IRI) = {
    var result: Option[String] = None
    referenceOntology.foreach(
      _.getAnnotationAssertionAxioms(iri).asScala.foreach { anno =>
        if (anno.getProperty().equals(factory.getRDFSLabel()))
          anno.getValue() match {
            case lit: OWLLiteral  =>
              result = Some(lit.getLiteral())
            case other => ; // do nothing
          }
      }
    )
    result
  }

}
object OWLApiInterface { 
  
//  implicit val (logger, formatter, appender) = ZeroLoggerFactory.newLogger(this)
//  import formatter._

  val logger = Logger(OWLApiInterface.getClass)

  def getOWLOntology(url:String): OWLOntology = { 

    val manager = OWLManager.createOWLOntologyManager();
    val iri = IRI.create(url)

    val ontology = manager.loadOntologyFromOntologyDocument(iri);

    ontology
  }


  def getOWLOntology(url: URL): OWLOntology = { 
    val manager = OWLManager.createOWLOntologyManager();
    val iri = IRI.create(url)

    val ontology = manager.loadOntologyFromOntologyDocument(iri);

    ontology
    }


  def getOWLOntology(file: File, ignoreMissingImports: Boolean = false): OWLOntology = { 
    val manager = OWLManager.createOWLOntologyManager();

    // if(ignoreMissingImports){
    //   manager.setSilentMissingImportsHandling(true)
      // manager.getOntologyLoaderConfiguration.
      //   setMissingImportHandlingStrategy(MissingImportHandlingStrategy.SILENT)
    //}

    val ontology = manager.loadOntologyFromOntologyDocument(file)

    ontology
  }

  def getSignature(ontology: OWLOntology): Set[OWLEntity] = { 
    ontology.getSignature(Imports.EXCLUDED).asScala.toSet[OWLEntity].filter(s =>
	  s.isInstanceOf[OWLClass]||s.isInstanceOf[OWLObjectProperty])
	
    //(ontology.getClassesInSignature++ontology.getObjectPropertiesInSignature).toSet[OWLEntity]
  }

  def getSignature(axioms: Iterable[_<:OWLAxiom]): Set[OWLEntity] = 
    axioms.toSet[OWLAxiom].flatMap(getSignature)

  def getSignature(axiom: OWLAxiom): Set[OWLEntity] = { 
    axiom.getSignature().asScala.toSet[OWLEntity].filter(s =>
      s.isInstanceOf[OWLClass]||s.isInstanceOf[OWLObjectProperty])
  }

  def getConceptSignature(ontology: OWLOntology): Set[OWLClass] = { 
    ontology.getClassesInSignature(Imports.EXCLUDED).asScala.toSet
  }

  // def isConsistent(ontology: OWLOntology): Boolean = { 
  //   print("checking consistency...")
  //   val manager = ontology.getOWLOntologyManager
  //   val reasoner  = new Reasoner.ReasonerFactory().createReasoner(ontology)

  //   val result = reasoner.isConsistent
  //   if(!result)
  //     println(" not consistent")
  //   else
  //     println(" consistent")

  //   result
  // }

  // def classify(ontology: OWLOntology) = {   
  //   logger.info("classifying...")
    
  //   val manager = OWLManager.createOWLOntologyManager()
  //   val reasoner = new Reasoner.ReasonerFactory().createReasoner(ontology)


  //   reasoner.precomputeInferences(InferenceType.CLASS_HIERARCHY);
  //   val gens = List[InferredAxiomGenerator[_ <:OWLAxiom]]();
  //   gens.add(new InferredSubClassAxiomGenerator());
		    
  //   val iog = new InferredOntologyGenerator(reasoner, gens);
  //   iog.fillOntology(manager, ontology);
  //   logger.info("done classifying.")
  // }

}
