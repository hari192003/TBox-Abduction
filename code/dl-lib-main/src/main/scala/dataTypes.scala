package nl.vu.kai.dl_lib.datatypes

//import com.dongxiguo.fastring.Fastring.Implicits._


import com.typesafe.scalalogging.Logger
import nl.vu.kai.dl_lib.tools.MultiSet
import org.semanticweb.owlapi.model.OWLAxiom

import scala.collection.JavaConverters._
import java.util


abstract class Expression { 
  def signature: Set[String]
  def conceptNames: Set[String]
  def roleNames: Set[String]
  def roles: Set[Role] = Set()
  def size: Int
  def subConcepts: MultiSet[Concept]

  def foreachNested(function: Expression => Unit): Unit = {
    function(this)
  }
}

abstract class Concept extends Expression

object TopConcept extends Concept {
  override def toString = "TOP"
  override def signature = Set() //Set("TOP")
  override def conceptNames = Set()
  override def roleNames = Set()
  override def size = 1
  override def subConcepts = MultiSet(TopConcept)

}

object BottomConcept extends Concept {
  override def toString = "BOTTOM"
  override def signature = Set() //Set("BOTTOM")
  override def conceptNames = Set()
  override def roleNames = Set()
  override def size = 1
  override def subConcepts = MultiSet(BottomConcept)
}

trait Name {
  def nameAsString(): String
}

case class ConceptName(name: String) extends Concept with Name {
  override def toString = name
  override def nameAsString = name
  override def signature = Set(name)
  override def conceptNames = Set(name)
  override def roleNames = Set()
  override def size = 1
  override def subConcepts = MultiSet(this)
}

case class ConceptComplement(concept: Concept) extends Concept {
  override def toString = "-"+concept.toString
  override def signature = concept.signature
  override def conceptNames = concept.conceptNames
  override def roleNames = concept.roleNames
  override def size = concept.size+1
  override def subConcepts = concept.subConcepts+this

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    concept.foreachNested(function)
  }

  override def roles = concept.roles
}

case class ConceptConjunction(var conjuncts: Seq[Concept]) extends Concept {
  override def toString = conjuncts.mkString("(", " n ", ")")
  override def signature = conjuncts.toSet[Concept].flatMap(_.signature)
  override def conceptNames = conjuncts.toSet[Concept].flatMap(_.conceptNames)
  override def roleNames = conjuncts.toSet[Concept].flatMap(_.roleNames)
  override def size = conjuncts.map(_.size).foldLeft(0)((a,b)=> a+b) + conjuncts.size - 1
  override def subConcepts = conjuncts.toSet[Concept].map(_.subConcepts).foldLeft(MultiSet[Concept])(_++_) + this

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    conjuncts.foreach(_.foreachNested(function))
  }

  override def roles = conjuncts.toSet[Concept].flatMap(_.roles)

  def getConjuncts() =
    conjuncts.asJava
}

case class ConceptDisjunction(var disjuncts: Seq[Concept]) extends Concept {
  override def toString = disjuncts.mkString("(", " u ", ")")
  override def signature = disjuncts.toSet[Concept].flatMap(_.signature)
  override def conceptNames = disjuncts.toSet[Concept].flatMap(_.conceptNames)
  override def roleNames = disjuncts.toSet[Concept].flatMap(_.roleNames)
  override def size = disjuncts.map(_.size).foldLeft(0)((a,b)=> a+b) + disjuncts.size - 1
  override def subConcepts = disjuncts.toSet[Concept].map(_.subConcepts).foldLeft(MultiSet[Concept])(_++_) + this

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    disjuncts.foreach(_.foreachNested(function))
  }

  override def roles = disjuncts.toSet[Concept].flatMap(_.roles)

  def getDisjuncts() = disjuncts.asJava
}

case class NominalSet(nominals: Seq[Individual]) extends Concept {
  override def toString = nominals.mkString("{", ", ", "}")
  override def signature = Set()
  override def conceptNames = Set()
  override def roleNames = Set()
  override def size = nominals.size*2-1
  override def subConcepts = MultiSet(this)
}

abstract class Role extends Expression {
  override def roleNames = signature
  override def conceptNames = Set()
  override def subConcepts = MultiSet()
  override def roles = Set(this)
}

case class RoleName(name: String) extends Role with Name {
  override def toString = name
  override def nameAsString = name
  override def signature = Set(name)
  override def size = 1
}

object TopRole extends RoleName("TOP")

case class InverseRole(role: Role) extends Role {
  override def toString = "("+role.toString+")^-1"
  override def signature = role.signature
  override def size = role.size+1

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
  }
}

case class RoleConjunction(cs: Iterable[Role]) extends Role {
  override def toString = cs.mkString("(", " n ", ")")
  override def signature = cs.flatMap(_.signature).toSet[String]
  override def size = cs.map(_.size).reduce((a,b)=> a+b) + cs.size - 1

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    cs.foreach(_.foreachNested(function))
  }
}

case class ExistentialRoleRestriction(role: Role, filler: Concept) extends Concept {
  override def toString = "E" + role + "." + filler
  override def signature = role.signature ++ filler.signature
  override def conceptNames = filler.conceptNames
  override def roleNames = role.signature ++ filler.roleNames
  override def size = 1 + role.size + filler.size
  override def subConcepts = filler.subConcepts + this

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
    filler.foreachNested(function)
  }

  override def roles = filler.roles + role
}

case class UniversalRoleRestriction(role: Role, filler: Concept) extends Concept {
  override def toString = "A" + role + "." + filler
  override def signature = role.signature ++ filler.signature
  override def conceptNames = filler.conceptNames
  override def roleNames = role.signature ++ filler.roleNames
  override def size = 1 + role.size + filler.size
  override def subConcepts = filler.subConcepts + this

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
    filler.foreachNested(function)
  }

  override def roles = filler.roles + role
}

case class MinNumberRestriction(number: Int, role: Role, filler: Concept) extends Concept {
  //  assert(number>=1)
  assert(number>=0)
  override def toString = ">=" + number.toString + role + "." + filler
  override def signature = role.signature ++ filler.signature
  override def conceptNames = filler.conceptNames
  override def roleNames = role.signature ++ filler.roleNames
  override def size = 1 + role.size + filler.size // + number
  override def subConcepts = filler.subConcepts + this

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
    filler.foreachNested(function)
  }

  override def roles = filler.roles + role
}

case class MaxNumberRestriction(number: Int, role: Role, filler: Concept) extends Concept {
  assert(number>=0)
  override def toString = "=<" + number.toString + role + "." + filler
  override def signature = role.signature ++ filler.signature
  override def conceptNames = filler.conceptNames
  override def roleNames = role.signature ++ filler.roleNames
  override def size = 1 + role.size + filler.size // + number // unary encoding
  override def subConcepts = filler.subConcepts + this

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
    filler.foreachNested(function)
  }

  override def roles = filler.roles + role
}

case class EqualNumberRestriction(number: Int, role: Role, filler: Concept) extends Concept {
  assert(number>=0)
  override def toString = "=" + number.toString + role + "." + filler
  override def signature = role.signature ++ filler.signature
  override def conceptNames = filler.conceptNames
  override def roleNames = role.signature ++ filler.roleNames
  override def size = 1 + role.size + filler.size // + number // unary encoding
  override def subConcepts = filler.subConcepts + this

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
    filler.foreachNested(function)
  }

  override def roles = filler.roles + role
}



abstract class DLStatement extends Expression

abstract class Axiom extends DLStatement

case class GeneralConceptInclusion(lhs: Concept, rhs: Concept) extends Axiom {
  override def toString = lhs + " <= " + rhs
  override def signature = lhs.signature ++ rhs.signature
  override def conceptNames = lhs.conceptNames ++ rhs.conceptNames
  override def roleNames = lhs.roleNames ++ rhs.roleNames
  override def size = lhs.size + rhs.size + 1
  override def subConcepts = lhs.subConcepts ++ rhs.subConcepts

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    lhs.foreachNested(function)
    rhs.foreachNested(function)
  }

  override def roles = lhs.roles ++ rhs.roles
}

case class EquivalenceAxiom(concepts: Seq[Concept]) extends Axiom {
  override def toString = concepts.mkString(" = ")
  override def signature = concepts.toSet[Concept].flatMap(_.signature)
  override def conceptNames = concepts.toSet[Concept].flatMap(_.conceptNames)
  override def roleNames = concepts.toSet[Concept].flatMap(_.roleNames)
  override def size = concepts.map(_.size).sum
  override def subConcepts =
    concepts.toSet[Concept].map(_.subConcepts).foldLeft(MultiSet[Concept])(_++_)

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    concepts.foreach(_.foreachNested(function))
  }

  override def roles = concepts.toSet[Concept].flatMap(_.roles)

  def getConcepts() = concepts.asJava
}

case class DisjointnessAxiom(concepts: Seq[Concept]) extends Axiom {
  override def toString = "disjoint("+concepts.mkString(", ")+")"
  override def signature = concepts.toSet[Concept].flatMap(_.signature)
  override def conceptNames = concepts.toSet[Concept].flatMap(_.conceptNames)
  override def roleNames = concepts.toSet[Concept].flatMap(_.roleNames)
  override def size = concepts.map(_.size).sum
  override def subConcepts =
    concepts.toSet[Concept].map(_.subConcepts).foldLeft(MultiSet[Concept])(_++_)

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    concepts.foreach(_.foreachNested(function))
  }

  override def roles = concepts.toSet[Concept].flatMap(_.roles)

  def getConcepts = concepts.asJava
}

case class DomainAxiom(role: Role, concept: Concept) extends Axiom {
  override def toString = "domain("+role+") = "+concept
  override def signature = role.signature ++ concept.signature
  override def conceptNames = concept.conceptNames
  override def roleNames = role.roleNames
  override def size = role.size + concept.size + 1
  override def subConcepts = concept.subConcepts

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
    concept.foreachNested(function)
  }


  def toSubsumption =
    GeneralConceptInclusion(
      ExistentialRoleRestriction(role, TopConcept),
      concept)
}

case class RangeAxiom(role: Role, concept: Concept) extends Axiom {
  override def toString = "range("+role+") = "+concept
  override def signature = role.signature ++ concept.signature
  override def conceptNames = concept.conceptNames
  override def roleNames = role.roleNames
  override def size = role.size + concept.size + 1
  override def subConcepts = concept.subConcepts

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
    concept.foreachNested(function)
  }

  def toSubsumption =
    GeneralConceptInclusion(TopConcept,
	  UniversalRoleRestriction(role, concept))
}


case class TBox(var axioms: Set[Axiom]) extends DLStatement {
  def add(axiom: Axiom) = axioms = axioms + axiom

  override def toString = axioms.mkString("\n")
  override def signature = axioms.flatMap(_.signature).toSet[String]
  override def conceptNames = axioms.flatMap(_.conceptNames).toSet[String]
  override def roleNames = axioms.flatMap(_.roleNames).toSet[String]

  def isEmpty = axioms.isEmpty

  override def size =
    axioms.toSeq.map(_.size).foldLeft(0)((a,b) => a+b)

  override def subConcepts = axioms.map(_.subConcepts).foldLeft(MultiSet[Concept])(_++_)

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    axioms.foreach(_.foreachNested(function))
  }

  override def roles = axioms.flatMap(_.roles)

  def getAxioms() = axioms.asJava
}

abstract class RoleAxiom extends DLStatement {
  override def conceptNames = Set()
  override def subConcepts = MultiSet()
}

case class RoleInclusion(lhs: Role, rhs: Role) extends RoleAxiom {
  override def toString = lhs.toString+ " <r "+rhs.toString
  override def signature = lhs.signature ++ rhs.signature
  override def roleNames = lhs.signature ++ rhs.signature
  override def size = lhs.size + rhs.size

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    lhs.foreachNested(function)
    rhs.foreachNested(function)
  }

  override def roles = lhs.roles ++ rhs.roles
}

case class TransitiveRoleAxiom(role: Role) extends RoleAxiom {
  override def toString = "trans("+role+")"
  override def signature = role.signature
  override def roleNames = role.signature
  override def size = role.size+1

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
  }

  override def roles = role.roles
}

case class SymmetricRoleAxiom(role: Role) extends RoleAxiom {
  override def toString = "symmetric("+role+")"
  override def signature = role.signature
  override def roleNames = role.signature
  override def size = role.size+1

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
  }

  override def roles = role.roles
}


case class FunctionalRoleAxiom(role: Role) extends RoleAxiom {
  override def toString = "func("+role+")"
  override def signature = role.signature
  override def roleNames = role.signature
  override def size = role.size+1

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
  }

  override def roles = role.roles
}

case class RoleChainAxiom(lhs: List[Role], rhs: Role) extends RoleAxiom {
  override def toString = lhs.mkString(" o ")+" <r "+rhs
  override def signature = lhs.toSet[Role].flatMap(_.signature) ++ rhs.signature
  override def roleNames = lhs.toSet[Role].flatMap(_.signature) ++ rhs.signature
  override def size = lhs.map(_.size).foldLeft(0)((a, b) => a+b) + lhs.size + rhs.size

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    lhs.foreach(_.foreachNested(function))
    rhs.foreachNested(function)
  }

  override def roles = lhs.toSet[Role].flatMap(_.roles) ++ rhs.roles

  def getLHS(): util.List[Role] =
    lhs.asJava
}


case class RBox(var axioms: Set[RoleAxiom]) extends DLStatement {
  def add(axiom: RoleAxiom) = axioms = axioms + axiom

  override def toString = axioms.mkString("\n")
  override def signature = axioms.flatMap(_.signature).toSet[String]
  override def roleNames = axioms.flatMap(_.roleNames).toSet[String]
  override def conceptNames = Set()
  override def subConcepts = MultiSet()

  def isEmpty = axioms.isEmpty

  override def size =
    axioms.toSeq.map(_.size).foldLeft(0)((a,b) => a+b)

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    axioms.foreach(_.foreachNested(function))
  }

  override def roles = axioms.flatMap(_.roles)

  def getAxioms = axioms.asJava
}

case class Individual(name: String) extends Expression {
  override def toString = name

  override def signature = Set()
  override def conceptNames = Set()
  override def roleNames = Set()
  override def size = 1
  override def subConcepts = MultiSet()
}


abstract class Assertion extends DLStatement

case class ConceptAssertion(concept: Concept, individual: Individual) extends Assertion {
  override def toString = concept.toString+"("+individual.toString+")"
  override def signature = concept.signature ++ individual.signature
  override def conceptNames = concept.conceptNames ++ individual.conceptNames
  override def roleNames = concept.roleNames ++ individual.roleNames
  override def size = concept.size + individual.size
  override def subConcepts = concept.subConcepts ++ individual.subConcepts

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    concept.foreachNested(function)
    individual.foreachNested(function)
  }

  override def roles = concept.roles
}


case class RoleAssertion(role: Role, individual1: Individual, individual2: Individual) extends Assertion {
  override def toString = role.toString+"("+individual1+", "+individual2+")"
  override def signature = role.signature
  override def conceptNames = Set()
  override def subConcepts = MultiSet()
  override def roleNames = role.signature
  override def size = role.size + 2

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    role.foreachNested(function)
  }

  override def roles = role.roles
}

case class ABox(var assertions: Set[Assertion]) extends DLStatement {
  override def toString = assertions.mkString("\n")
  override def signature = assertions.flatMap(_.signature)
  override def conceptNames = assertions.flatMap(_.conceptNames)
  override def roleNames = assertions.flatMap(_.roleNames)
  override def subConcepts = assertions.map(_.subConcepts).foldLeft(MultiSet[Concept])(_++_)

  def isEmpty = assertions.isEmpty

  override def size =
    assertions.toSeq.map(_.size).foldLeft(0)((a,b) => a+b)

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    assertions.foreach(_.foreachNested(function))
  }

  override def roles = assertions.flatMap(_.roles)

  def getAssertions = assertions.asJava
}

object Ontology {
  def buildFrom(statements: Iterable[DLStatement]): Ontology = {
    val result = new Ontology()
    statements.foreach(result.addStatement)
    result
  }
}

class Ontology(var tbox:TBox = new TBox(Set()),
	       var abox: ABox = new ABox(Set()),
	       var rbox: RBox = new RBox(Set())) extends DLStatement {

  var annotations: Set[Annotation] = Set()
  var unsupportedOWLAxioms: Set[OWLAxiom] = Set()

  def this(tbox: TBox, abox: ABox) = this(tbox, abox, new RBox(Set()))

  def this() = this(new TBox(Set()), new ABox(Set()))

  def isEmpty = tbox.isEmpty && abox.isEmpty && rbox.isEmpty

  def getSubConcepts() = subConcepts.keySet.asJava
  def getConceptNames() = conceptNames.map(ConceptName).asJava
  def getRoleNames() = roleNames.map(RoleName).asJava

  def addStatement(statement: DLStatement) = statement match {
    case a: Assertion => abox.assertions = abox.assertions + a
    case a: Axiom => tbox.axioms = tbox.axioms + a
    case a: RoleAxiom => rbox.axioms = rbox.axioms + a
  }

  def addStatements(statements: Iterable[DLStatement]) =
    statements.foreach(addStatement)

  def addAnnotation(annotation: Annotation) =
    annotations += annotation

  def addUnsupportedOWLAxiom(axiom: OWLAxiom) =
    unsupportedOWLAxioms+=axiom

  def statements: Iterable[DLStatement] = tbox.axioms ++ rbox.axioms ++ abox.assertions

  override def toString = "TBox:\n" + tbox.toString + "\n\nRBox:\n" + rbox.toString+"\n\n"+ "\n\nABox:\n" + abox.toString+"\n\n"
  override def signature = tbox.signature ++ rbox.signature ++ abox.signature
  override def conceptNames = tbox.conceptNames ++ abox.conceptNames
  override def roleNames = tbox.roleNames ++ rbox.roleNames ++ abox.roleNames
  override def subConcepts = tbox.subConcepts ++ rbox.subConcepts ++ abox.subConcepts

  override def foreachNested(function: Expression => Unit) = {
    super.foreachNested(function)
    tbox.foreachNested(function)
    abox.foreachNested(function)
    rbox.foreachNested(function)
  }

  override def roles = tbox.roles ++ rbox.roles ++ abox.roles

  override def equals(other: Any) = other match {
    case other: Ontology =>    tbox==other.tbox && abox==other.abox && rbox==other.rbox
    case _ => false
  }

  override def hashCode =
    13*tbox.hashCode+abox.hashCode+rbox.hashCode

  def size: Int = tbox.size+abox.size+rbox.size


  def remove(dlStatement: DLStatement) = dlStatement match {
    case a: Axiom => tbox.axioms -= a
    case a: Assertion => abox.assertions -= a
    case ra: RoleAxiom => rbox.axioms -= ra
  }
}

object DLHelpers {

  //implicit val (logger, formatter, appender) = ZeroLoggerFactory.newLogger(this)
  //import formatter._
  val logger = Logger(DLHelpers.getClass)

  def convert2binaryConjunctions(ontology: Ontology): Unit = {
    ontology.tbox.axioms = ontology.tbox.axioms.map(convert2binaryConjunctions(_).asInstanceOf[Axiom])
    ontology.rbox.axioms = ontology.rbox.axioms.map(convert2binaryConjunctions(_).asInstanceOf[RoleAxiom])
    ontology.abox.assertions = ontology.abox.assertions.map(convert2binaryConjunctions(_).asInstanceOf[Assertion])
  }

  def convert2binaryConjunctions(dlStatement: DLStatement): DLStatement = dlStatement match {
    case GeneralConceptInclusion(lhs,rhs) =>
      GeneralConceptInclusion(convert2binaryConjunctions(lhs),convert2binaryConjunctions(rhs))
    case EquivalenceAxiom(cs) => EquivalenceAxiom(cs.map(convert2binaryConjunctions))
    case DisjointnessAxiom(cs) => DisjointnessAxiom(cs.map(convert2binaryConjunctions))
    case DomainAxiom(r,c) => DomainAxiom(r,convert2binaryConjunctions(c))
    case RangeAxiom(r,c) => RangeAxiom(r, convert2binaryConjunctions(c))
    case ConceptAssertion(c,a) => ConceptAssertion(convert2binaryConjunctions(c),a)
    case other => other // assuming here that all other dl statements do not contain concepts
  }
  def convert2binaryConjunctions(concept: Concept): Concept = concept match {
    case ConceptConjunction(cs) if cs.isEmpty => TopConcept
    case ConceptConjunction(cs) if cs.size == 1 => convert2binaryConjunctions(cs.head)
    case ConceptConjunction(cs) =>
      val head = cs.head
      val rest = ConceptConjunction(cs.tail)
      ConceptConjunction(Seq(convert2binaryConjunctions(head), convert2binaryConjunctions(rest)))
    case TopConcept => concept
    case BottomConcept => concept
    case _: ConceptName => concept
    case ExistentialRoleRestriction(r,c) => ExistentialRoleRestriction(r,convert2binaryConjunctions(c))
    case UniversalRoleRestriction(r,c) => UniversalRoleRestriction(r,convert2binaryConjunctions(c))
    case ConceptDisjunction(ds) => ConceptDisjunction(ds.map(convert2binaryConjunctions))
    case ConceptComplement(c) => ConceptComplement(convert2binaryConjunctions(c))
    case _: NominalSet => concept
    case MinNumberRestriction(n,r,c) => MinNumberRestriction(n,r,convert2binaryConjunctions(c))
    case MaxNumberRestriction(n,r,c) => MaxNumberRestriction(n,r,convert2binaryConjunctions(c))
    case EqualNumberRestriction(n,r,c) => EqualNumberRestriction(n,r,convert2binaryConjunctions(c))
  }

  def nnf(ontology: Ontology): Ontology =
    new Ontology(tbox = nnf(ontology.tbox),
		 abox = nnf(ontology.abox),
		 rbox = ontology.rbox)

  def nnf(tbox: TBox): TBox = new TBox(tbox.axioms.flatMap(nnf))

  def nnf(axiom: Axiom): Set[GeneralConceptInclusion] = axiom match {
    case EquivalenceAxiom(concepts) =>
      concepts.toSet[Concept].flatMap(c1 => (concepts.toSet-c1).flatMap(c2 => nnf(GeneralConceptInclusion(c1,c2))))
  }

  def nnf(abox: ABox): ABox = new ABox(abox.assertions.flatMap(nnf))

  def nnf(assertion: Assertion): Set[Assertion] = assertion match {
    case _: RoleAssertion => Set(assertion)
    case ConceptAssertion(c, a) => Set(ConceptAssertion(nnf(c), a))
  }


  def nnf(concept: Concept): Concept = concept match {
    case ConceptComplement(TopConcept) => BottomConcept
    case ConceptComplement(BottomConcept) => TopConcept
    case ConceptComplement(ExistentialRoleRestriction(r, f)) =>
      UniversalRoleRestriction(r, nnf(ConceptComplement(f)))
    case ConceptComplement(UniversalRoleRestriction(r, f)) =>
      ExistentialRoleRestriction(r, nnf(ConceptComplement(f)))
    case ConceptComplement(MinNumberRestriction(n, r, f)) =>
      MaxNumberRestriction(n-1, r, ConceptComplement(nnf(ConceptComplement(f))))
    case ConceptComplement(MaxNumberRestriction(n, r, f)) =>
      MinNumberRestriction(n+1, r, nnf(f))
    case ConceptComplement(ConceptConjunction(cs)) =>
      ConceptDisjunction(cs.map(f => nnf(ConceptComplement(f))))
    case ConceptComplement(ConceptDisjunction(ds)) =>
      ConceptConjunction(ds.map(f => nnf(ConceptComplement(f))))
    case ConceptComplement(ConceptComplement(f)) => nnf(f)
    case ConceptComplement(f) => ConceptComplement(nnf(f))
    case ExistentialRoleRestriction(r, f) => ExistentialRoleRestriction(r, nnf(f))
    case UniversalRoleRestriction(r, f) => UniversalRoleRestriction(r, nnf(f))
    case MinNumberRestriction(n, r, f) => MinNumberRestriction(n, r, nnf(f))
    case MaxNumberRestriction(n, r, f) =>
      MaxNumberRestriction(n, r, ConceptComplement(nnf(ConceptComplement(f))))
    case ConceptConjunction(cs) => ConceptConjunction((cs.toSet[Concept].map(nnf)-TopConcept).toSeq)
    case ConceptDisjunction(ds) => ConceptDisjunction((ds.toSet[Concept].map(nnf)-BottomConcept).toSeq)
    case b: ConceptName => b
    case TopConcept => TopConcept
    case BottomConcept => BottomConcept
    case _: NominalSet => concept
  }

  def disjoint(c1: Concept, c2: Concept) =
    new GeneralConceptInclusion(ConceptConjunction(Seq(c1, c2)), BottomConcept)
//    new Subsumption(c1, new ConceptComplement(c2))



  def neg(concept: Concept): Concept = concept match {
    case TopConcept => BottomConcept
    case BottomConcept => TopConcept
    case c: ConceptName => ConceptComplement(c)
    case ConceptComplement(f) => f
    case ConceptDisjunction(fs) => ConceptConjunction(fs.map(neg))
    case ConceptConjunction(fs) => ConceptDisjunction(fs.map(neg))
    case UniversalRoleRestriction(r, f) => ExistentialRoleRestriction(r, neg(f))
    case ExistentialRoleRestriction(r, f) => UniversalRoleRestriction(r, neg(f))
    case MinNumberRestriction(n, r, f) => MaxNumberRestriction(n-1, r, f)
    case MaxNumberRestriction(n, r, f) => MinNumberRestriction(n+1, r, f)
  }

  def conjunction(concepts: Iterable[Concept]): Concept = {

    if(concepts.exists(c1 => concepts.exists(c2 => c1==neg(c2))))
      return BottomConcept
    if(concepts.exists(_==BottomConcept))
      return BottomConcept

    var set = concepts.toSet[Concept] - TopConcept
    if(set.isEmpty)
      TopConcept
    else if(set.size==1)
      set.head
    else {
      // flatten
      while(set.exists(_.isInstanceOf[ConceptConjunction])) {
	val nested = (set.collectFirst{ case c:ConceptConjunction => c }).get.asInstanceOf[ConceptConjunction]
	set -= nested
	set ++= nested.conjuncts
      }

      ConceptConjunction(set.toSeq)
    }
  }

  def conjunction(roles: Iterable[Role]): Role = {
    if(roles.size==1)
      roles.head
    else if(roles.size>1)
      new RoleConjunction(roles)
    else {
      assert(false, "Not implemented yet!")
      null
    }
  }


  def disjunction(c1: Concept, c2: Concept): Concept =
    disjunction(Set(c1, c2))

  def disjunction(concepts: Iterable[Concept]): Concept = {
    if(concepts.toSet(TopConcept))
      return TopConcept
    if(concepts.exists(c1 => concepts.exists(c2 => c1==neg(c2))))
      return TopConcept

    var set = concepts.toSet[Concept] - BottomConcept
    if(set.isEmpty)
      BottomConcept
    else if(set.size==1)
      set.head
    else {
      //flatten
      while(set.exists(_.isInstanceOf[ConceptDisjunction])) {
	val nested = (set.collectFirst{ case c:ConceptDisjunction => c }).get.asInstanceOf[ConceptDisjunction]
	set -= nested
	set ++= nested.disjuncts
      }

      ConceptDisjunction(set.toSeq)
    }
  }  // <---- looks correct


  def inverse(role: Role): Role = role match {
    case r: RoleName => InverseRole(r)
    case InverseRole(r: RoleName) => r
    case InverseRole(InverseRole(r)) => inverse(r)
    case _ => assert(false, "complex roles not supported"); null
  }

  def inverseOf(role1: Role, role2: Role): Boolean =
    inverse(role1)==inverse(inverse(role2))

  // General simplifications
  def simplify(ont: Ontology): Ontology = {
    logger.info(s"Simplifying ontology of ${ont.statements.size} statements.")
    new Ontology(simplify(ont.tbox), simplify(ont.abox), ont.rbox)
  }
  def simplify(tbox: TBox): TBox = {
    var axioms = tbox.axioms.map(simplify)
    axioms = axioms.filterNot(_ match {
      case GeneralConceptInclusion(BottomConcept, _) => true
      case GeneralConceptInclusion(_, TopConcept) => true
      case _ => false
    })
    new TBox(axioms)
}
  def simplify(abox: ABox): ABox = new ABox(abox.assertions.map(simplify))
  def simplify(axiom: Axiom): Axiom = {
    logger.trace(s"Simplifying ${axiom}")
    axiom match {
      case GeneralConceptInclusion(c1, c2) => GeneralConceptInclusion(simplify(c1), simplify(c2))
      case EquivalenceAxiom(cs) => EquivalenceAxiom(cs.map(simplify))
    }
  }

  def simplify(assertion: Assertion): Assertion = assertion match {
    case ConceptAssertion(c, i) => ConceptAssertion(simplify(c), i)
    case r: RoleAssertion => r
  }

  def simplify(concept: Concept): Concept = concept match {
    case ConceptComplement(c) => neg(simplify(c))
    case ConceptConjunction(cs) => {
      if(cs.size==1)
	simplify(cs.head)
      else if(cs.isEmpty)
	BottomConcept
      else {
	val csX = cs.filter{ _ match {
	  case ConceptDisjunction(ds) => !ds.toSet.exists(cs.toSet) // redundant then
	  case _ => true
	}}
	if(csX.forall(d => d.isInstanceOf[ConceptDisjunction])){
	  // (A u B u C) n (A u B u D) => (A u B u (C n D)), if reasonable

	  val diss = csX.map(_.asInstanceOf[ConceptDisjunction])
	  var overlap = diss.toSeq(0).disjuncts.toSet
	  diss.foreach { dis =>
	    overlap = overlap.filter(dis.disjuncts.contains)
		      }
	  if(overlap.isEmpty)
	    conjunction(csX.map(simplify))
	  else disjunction(overlap.map(simplify) + conjunction(
	    diss.map(dis => simplify(ConceptDisjunction((dis.disjuncts.toSet--overlap).toSeq)))))
	}
	  else
	    conjunction(csX.map(simplify))
      }
    }
    case ConceptDisjunction(ds) => {
      if(ds.size==1)
	simplify(ds.head)
      else
	disjunction(ds.map(simplify))
    }
    case UniversalRoleRestriction(r1, ExistentialRoleRestriction(r2, c)) if inverseOf(r1, r2) =>
      simplify(c)

    case ExistentialRoleRestriction(r, c) =>
      val filler = simplify(c)
      if(filler==BottomConcept)
	BottomConcept
      else
	ExistentialRoleRestriction(r, filler)
    case UniversalRoleRestriction(r, c) =>
      val filler = simplify(c)
      if(filler==TopConcept)
	TopConcept
      else
	UniversalRoleRestriction(r, filler)
    case MinNumberRestriction(1, r, c) => simplify(ExistentialRoleRestriction(r,c))
    case MaxNumberRestriction(0, r, ConceptComplement(c)) => simplify(UniversalRoleRestriction(r,c))
    case MinNumberRestriction(n, r, c) => {
      val filler = simplify(c)
      if(filler==BottomConcept)
	BottomConcept
      else
	MinNumberRestriction(n, r, filler)
    }
    case MaxNumberRestriction(n, r, c) => {
      val filler = simplify(c)
      if(filler==BottomConcept)
	TopConcept
      else
	MaxNumberRestriction(n, r, simplify(c))
    }
    case c => c
  }

  /**
   * Splits subsumptions of the kind C <= (C1 n C2) to subsumptions C <= C1, C <= C2
   */
  def splitConjunctions(ontology: Ontology) = {
    val result = new Ontology(abox=ontology.abox, rbox=ontology.rbox)
    val axioms = ontology.tbox.axioms.flatMap{_ match {
      case GeneralConceptInclusion(c, ConceptConjunction(cs)) => cs.map(GeneralConceptInclusion(c,_)).toSet[Axiom]
      case other => Set(other)
    }}
    result.tbox = TBox(axioms)
    result
  }
}
