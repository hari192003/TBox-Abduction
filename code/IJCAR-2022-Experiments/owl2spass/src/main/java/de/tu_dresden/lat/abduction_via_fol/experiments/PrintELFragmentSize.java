package de.tu_dresden.lat.abduction_via_fol.experiments;

import de.tu_dresden.lat.abduction_via_fol.ontologyTools.ELFilter;
import org.semanticweb.owlapi.apibinding.OWLManager;
import org.semanticweb.owlapi.model.AxiomType;
import org.semanticweb.owlapi.model.OWLOntology;
import org.semanticweb.owlapi.model.OWLOntologyCreationException;
import org.semanticweb.owlapi.model.parameters.Imports;

import java.io.File;

public final class PrintELFragmentSize {
    public static void main(String[] args) throws OWLOntologyCreationException {
        OWLOntology ontology = OWLManager.createOWLOntologyManager()
                .loadOntologyFromOntologyDocument(new File(args[0]));

        ELFilter.deleteNonELAxioms(ontology);

        System.out.println(args[0]+"\t "+ontology.getAxiomCount(Imports.INCLUDED));
    }
}
