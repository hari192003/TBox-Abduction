package de.tu_dresden.lat.abduction_via_fol.ontologyTools;

import org.semanticweb.owlapi.model.*;
import uk.ac.manchester.cs.owl.owlapi.OWLObjectSomeValuesFromImpl;

import java.util.HashSet;
import java.util.Set;

/**
 * Filter out non EL axioms from an ontology.
 */
public final class ELFilter {

    private ELFilter() {
        // utilities class
    }

    public static void deleteNonELAxioms(OWLOntology ontology) {
        Set<OWLAxiom> delete = new HashSet<>();

        ontology.axioms().forEach(axiom -> {
            if(!inEL(axiom)){
                //System.out.println("Not in EL: "+axiom);
                delete.add(axiom);
            }
        });
        ontology.removeAxioms(delete);
    }

    public static boolean inEL(OWLAxiom axiom) {
        if(axiom instanceof OWLSubClassOfAxiom){
            OWLSubClassOfAxiom ax = (OWLSubClassOfAxiom) axiom;
            return inEL(ax.getSubClass()) && inEL(ax.getSuperClass());
        } else if(axiom instanceof OWLSubClassOfAxiomShortCut)
            return inEL(((OWLSubClassOfAxiomShortCut) axiom).asOWLSubClassOfAxiom());
        else if(axiom instanceof OWLSubClassOfAxiomSetShortCut){
            OWLSubClassOfAxiomSetShortCut shortcut = (OWLSubClassOfAxiomSetShortCut) axiom;
            return shortcut.asOWLSubClassOfAxioms().stream().allMatch(ELFilter::inEL);
        } else
            return false;
    }

    public static boolean inEL(OWLClassExpression cl){
        if(cl instanceof OWLClass) {
            if(!cl.isOWLNothing())
                return true;
        } else if (cl instanceof OWLObjectIntersectionOf) {
            return cl.conjunctSet().allMatch(ELFilter::inEL);
        } else if(cl instanceof OWLObjectSomeValuesFrom) {
            OWLObjectSomeValuesFrom some = (OWLObjectSomeValuesFrom) cl;
            return (some.getProperty() instanceof OWLProperty) && inEL(some.getFiller());
        }

        return false;
    }
}
