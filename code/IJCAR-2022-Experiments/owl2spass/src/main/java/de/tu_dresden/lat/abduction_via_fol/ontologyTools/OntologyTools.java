package de.tu_dresden.lat.abduction_via_fol.ontologyTools;

import org.semanticweb.owlapi.model.OWLEntity;
import org.semanticweb.owlapi.model.OWLOntology;
import org.semanticweb.owlapi.model.OWLOntologyCreationException;
import org.semanticweb.owlapi.modularity.locality.LocalityClass;
import org.semanticweb.owlapi.modularity.locality.SyntacticLocalityModuleExtractor;

import java.util.stream.Stream;

public class OntologyTools {

    public static OWLOntology extractStarModule(OWLOntology ontology, Stream<OWLEntity> signature) throws OWLOntologyCreationException {
        OWLOntology module = ontology.getOWLOntologyManager().createOntology();

        assert(signature!=null);

        new SyntacticLocalityModuleExtractor(LocalityClass.STAR, ontology.axioms()).extract(signature).forEach(
                module::addAxiom
        );
        return module;
    }

}
