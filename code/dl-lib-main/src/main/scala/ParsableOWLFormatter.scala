package nl.vu.kai.dl_lib.formatting

import nl.vu.kai.dl_lib.owlapi.OWLApiConverter
import org.semanticweb.owlapi.model.OWLAxiom

class ParsableOWLFormatter extends Formatter[OWLAxiom] {

  val converter = new OWLApiConverter(simplifiedNames = false)

  override def format(axiom: OWLAxiom): String = {
    converter.convert(axiom).map(_.toString).mkString("\n");
  }
}
