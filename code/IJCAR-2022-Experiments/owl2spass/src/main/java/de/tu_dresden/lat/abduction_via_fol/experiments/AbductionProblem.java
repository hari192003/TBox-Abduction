package de.tu_dresden.lat.abduction_via_fol.experiments;

import org.semanticweb.owlapi.model.OWLOntology;
import org.semanticweb.owlapi.model.OWLSubClassOfAxiom;

public class AbductionProblem {
    private OWLOntology ontology;
    private OWLSubClassOfAxiom observation;

    public AbductionProblem(OWLOntology ontology, OWLSubClassOfAxiom observation) {
        this.ontology = ontology;
        this.observation = observation;
    }

    public OWLOntology getOntology() {
        return ontology;
    }

    public OWLSubClassOfAxiom getObservation() {
        return observation;
    }

}
