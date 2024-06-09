import nl.vu.kai.dl_lib.datatypes.{RoleChainAxiom, RoleInclusion, TransitiveRoleAxiom}
import nl.vu.kai.dl_lib.filtering.ELTBoxFilter
import nl.vu.kai.dl_lib.owlapi.{OWLApiConverter, OWLParser}
import org.semanticweb.owlapi.apibinding.OWLManager

import java.io.File

object CheckForRoleAxioms {
  def main(args: Array[String]) = {
    println("Loading ontology file "+args(0))
    val owlOntology = OWLManager.createOWLOntologyManager().loadOntologyFromOntologyDocument(new File(args(0)))
    val ontology = new OWLApiConverter().convert(owlOntology)

    println("Number of TBox axioms: "+ontology.tbox.axioms.size)
    println("Number of pure EL TBox axioms: "+owlOntology.axioms().filter(ELTBoxFilter.supported).count())
    println("Number of RBox axioms: "+ontology.rbox.axioms.size)
    println("Number of role inclusion axioms: "+ontology.rbox.axioms.filter(x => x.isInstanceOf[RoleInclusion]).size)
    println("Number of role chain axioms: "+ontology.rbox.axioms.filter(x => x.isInstanceOf[RoleChainAxiom]).size)
    println("Number of transitive role axioms: " + ontology.rbox.axioms.filter(x => x.isInstanceOf[TransitiveRoleAxiom]).size)
  }
}
