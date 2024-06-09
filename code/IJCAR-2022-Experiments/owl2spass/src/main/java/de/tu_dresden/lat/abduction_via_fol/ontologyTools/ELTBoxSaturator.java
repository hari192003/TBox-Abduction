package de.tu_dresden.lat.abduction_via_fol.ontologyTools;

import org.semanticweb.elk.owlapi.ElkReasonerFactory;
import org.semanticweb.owlapi.model.*;
import org.semanticweb.owlapi.reasoner.Node;
import org.semanticweb.owlapi.reasoner.OWLReasoner;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class ELTBoxSaturator {

    private OWLDataFactory factory; // to be used by all

    private Set<Node<OWLClass>> visited; // to be used when walking through hierarchy to avoid redundant steps

    private Map<OWLClass, OWLClassExpression> representers; // to be used for definitions
    private Set<OWLLogicalAxiom> definitionAxioms;

    /**
     * Adds all entailed axioms in one of the following forms to the ontology:
     * A <= B
     * A1 and A2 <= B
     * exists r.A <= B
     * A <= exists r.B
     *
     * @ TODO: currently may also create axioms where both LHS and RHS is conjunction or existential restriction
     */
    public void saturate(OWLOntology ontology){

        this.factory=ontology.getOWLOntologyManager().getOWLDataFactory();
        this.visited = new HashSet<>();

        addDefinitions(ontology);

        OWLReasoner reasoner = new ElkReasonerFactory().createReasoner(ontology);

        reasoner.flush();

        addRecursive(reasoner.getBottomClassNode(),reasoner, ontology);

        removeDefinitions(ontology);

    }

    private void addRecursive(Node<OWLClass> node, OWLReasoner reasoner, OWLOntology ontology) {
        if(visited.contains(node))
            return;

        visited.add(node);

        // first capture th equivalences as a cycle of subsumptions
        OWLClass rep = node.getRepresentativeElement();
        OWLClass last = rep;
        for(OWLClass cl : node.getEntitiesMinus(last)){
            if(last!=null){
                addSubsumption(ontology, last, cl);
            }
            last=cl;
        }
        if(!last.equals(rep) && !rep.isBottomEntity() && !rep.isOWLThing())
            addSubsumption(ontology, last, rep); // now close the cycle to get the equivalence (if more than one)

        // next the intermediate successors
        for(Node<OWLClass> next: reasoner.getSuperClasses(rep, true)){
            if(!next.isTopNode() && !rep.isBottomEntity())
                addSubsumption(ontology, rep, next.getRepresentativeElement());
            addRecursive(next, reasoner, ontology);
        }
    }

    private void addSubsumption(OWLOntology ontology, OWLClass lhs, OWLClass rhs){
        ontology.addAxiom(
                factory.getOWLSubClassOfAxiom(cFor(lhs),cFor(rhs)));
    }

    /**
     * Add definition files for complex concepts occurring.
     * Assumption: ontology is normalised!
     */
    private void addDefinitions(OWLOntology ontology){
        representers = new HashMap<>();
        definitionAxioms = new HashSet<>();

        int counter = 0;

        OWLClass nextName = factory.getOWLClass(IRI.create("__D__"));

        for(OWLClassExpression ce: ontology.getNestedClassExpressions()) {
            if(ce instanceof OWLObjectIntersectionOf || ce instanceof OWLObjectSomeValuesFrom){
                while(ontology.getClassesInSignature().contains(nextName)){
                    counter++;
                    nextName = factory.getOWLClass(IRI.create("__D__"+counter));
                }
                representers.put(nextName, ce);
                OWLLogicalAxiom def = factory.getOWLEquivalentClassesAxiom(nextName, ce);
                definitionAxioms.add(def);
                ontology.add(def);
            }
        }
    }

    private void removeDefinitions(OWLOntology ontology) {
        ontology.removeAxioms(definitionAxioms);
    }

    private OWLClassExpression cFor(OWLClass c){
        if(representers.containsKey(c))
            return representers.get(c);
        else
            return c;
    }
}
