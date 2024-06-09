package de.tu_dresden.lat.abduction_via_fol.ontologyTools;

import org.semanticweb.owlapi.model.*;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

public class NameProvider {
    private OWLDataFactory factory;

    private Set<OWLClass> freshNames;

    private Set<OWLClass> forbiddenNames;

    private final Map<OWLClassExpression, OWLClass> knownNames;

    private int counter = 0;

    public NameProvider(OWLOntology ontology){
        this.factory=ontology.getOWLOntologyManager().getOWLDataFactory();
        knownNames=new HashMap<>();
        freshNames = new HashSet<>();
        forbiddenNames = ontology.classesInSignature().collect(Collectors.toSet());
    }

    public Set<OWLClass> getFreshNames(){
        return freshNames;
    }

    public boolean knows(OWLClassExpression exp){
        return knownNames.containsKey(exp);
    }

    public OWLClass nameFor(OWLClassExpression exp){
        if(knows(exp))
            return knownNames.get(exp);
        else {
            OWLClass freshName = null;
            while(freshName==null || forbiddenNames.contains(freshName)){
                counter ++;
                freshName = factory.getOWLClass(IRI.create("_A"+counter));
            }
            knownNames.put(exp, freshName);
            freshNames.add(freshName);
            return freshName;
        }
    }

}
