package nl.vu.kai.dl_lib.datatypes

trait Annotation
case class LabelAnnotation(name: Name, label: String, language: String) extends Annotation

case class SeeAlsoAnnotation(name: Name, ref: String) extends Annotation

case class TaxonAnnotation(name: Name, ref: String) extends Annotation