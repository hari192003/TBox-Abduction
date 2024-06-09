package de.tu_dresden.lat.abduction_via_fol.ontologyTools;

import org.semanticweb.owlapi.model.*;

import java.util.HashSet;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class ELTBoxNormaliser implements OWLAxiomVisitor {

    private final static boolean TOP_AS_CONCEPT = true; // as part of the translation,
                                                        // treat owl:thing as special concept name
    private final static IRI TOP_CONCEPT = IRI.create("A_TOP");

    private Set<OWLAxiom> transformedAxioms;

    private final OWLDataFactory factory;

    private final NameProvider nameProvider;

    private final boolean filterNonEL; // ignore axioms not in pure EL or throw an exception?

    private int ignoredAxioms; // counts the non-supported axioms that have been ignored

    private Set<OWLClass> collectedClasses = new HashSet<>();
    private Set<OWLObjectProperty> collectedProperties = new HashSet<>();

    /**
     *
     * @param factory
     * @param nameProvider
     * @param filterNonEL if set to true, then axioms not in EL are removed by the transformation. Otherwise, non-EL
     *                    axioms will throw an exception.
     */
    public ELTBoxNormaliser(OWLDataFactory factory, NameProvider nameProvider, boolean filterNonEL) {
        this.factory=factory;
        this.nameProvider=nameProvider;
        this.filterNonEL=filterNonEL;
        init();
    }

    public void init(){
        this.transformedAxioms =new HashSet<>();
        this.ignoredAxioms=0;
    }

    public Set<OWLAxiom> getTransformedAxioms() {
        if(TOP_AS_CONCEPT){
            collectedClasses.forEach(cl -> {
                transformedAxioms.add(factory.getOWLSubClassOfAxiom(cl,factory.getOWLClass(TOP_CONCEPT)));
            });
            collectedProperties.forEach(prp -> {
                transformedAxioms.add(factory.getOWLSubClassOfAxiom(
                        factory.getOWLObjectSomeValuesFrom(prp, factory.getOWLClass(TOP_CONCEPT)),
                        factory.getOWLClass(TOP_CONCEPT)
                ));
            });
        }
        return transformedAxioms;
    }

    public int getNumberOfIgnoredAxioms(){
        return ignoredAxioms;
    }

    @Override
    public void doDefault(Object object) {
        if(object instanceof OWLSubClassOfAxiomShortCut){
            ((OWLSubClassOfAxiomShortCut) object).asOWLSubClassOfAxiom().accept(this);
        } else if(object instanceof OWLSubClassOfAxiomSetShortCut){
            ((OWLSubClassOfAxiomSetShortCut) object).asOWLSubClassOfAxioms().forEach(a -> a.accept(this));
        } else
            notSupported(object);
    }

    private void notSupported(Object object){
        if(!filterNonEL)
            throw new NotInELException("Not supported: "+object);
        else {
            //System.out.println("Not supported: "+object);
            ignoredAxioms++;
        }
    }

    @Override
    public void visit(OWLSubClassOfAxiom ax) {
        try {
            OWLClassExpression lhs = ax.getSubClass().accept(lhsNormaliser);
            ax.getSuperClass().accept(rhsNormaliser).forEach(rhs -> {
                transformedAxioms.add(factory.getOWLSubClassOfAxiom(lhs, rhs));
            });
        } catch (NotInELException exp){
            notSupported(ax);
        }
    }

    @Override
    public void visit(OWLEquivalentClassesAxiom ax) {
        try {
            for (OWLAxiom axiom : ax.asOWLSubClassOfAxioms())
                axiom.accept(this);
        } catch (NotInELException exp) {
            notSupported(ax);
        }
    }

    public void visit(OWLObjectPropertyDomainAxiom ax) {
        ax.asOWLSubClassOfAxiom().accept(this);
    }

    private OWLClassExpressionVisitorEx<OWLClassExpression> lhsNormaliser = new OWLClassExpressionVisitorEx<>() {

        private Set<OWLClassExpression> lhsKnown = new HashSet<>();

        private OWLClass flatten(OWLClassExpression exp) {
            if(exp.isOWLThing())
                return factory.getOWLClass(TOP_CONCEPT);
            else if(exp instanceof OWLClass)
                return (OWLClass) exp;
            else if(lhsKnown.contains(exp))
                return nameProvider.nameFor(exp);
            else {
                OWLClass name = nameProvider.nameFor(exp);
                lhsKnown.add(exp);
                collectedClasses.add(name);
                OWLAxiom definition = factory.getOWLSubClassOfAxiom(
                        exp.accept(this),
                        name
                );
                transformedAxioms.add(definition);
                return name;
            }
        }

        @Override
        public OWLObjectIntersectionOf visit(OWLObjectIntersectionOf ce) {
            return factory.getOWLObjectIntersectionOf(
                    ce.operands()
                            .map(this::flatten)
                            .collect(Collectors.toList()));
        }

        @Override
        public OWLObjectSomeValuesFrom visit(OWLObjectSomeValuesFrom ce) {
            collectedProperties.add((OWLObjectProperty) ce.getProperty());
            return factory.getOWLObjectSomeValuesFrom(ce.getProperty(), flatten(ce.getFiller()));
        }

        @Override
        public OWLClass visit(OWLClass ce) {
            if(TOP_AS_CONCEPT && ce.isOWLThing())
                return factory.getOWLClass(TOP_CONCEPT);
            collectedClasses.add(ce);

            return ce;
        }

        @Override
        public <T> OWLClass doDefault(T object) {
            throw new NotInELException("Not supported: "+object);
        }
    };


    private OWLClassExpressionVisitorEx<Stream<OWLClassExpression>> rhsNormaliser = new OWLClassExpressionVisitorEx<>() {

        Set<OWLClassExpression> rhsKnown = new HashSet<>();

        private OWLClass flatten(OWLClassExpression exp) {
            if(exp.isOWLThing())
                return factory.getOWLClass(TOP_CONCEPT);
            else if(exp instanceof OWLClass)
                return (OWLClass) exp;
            else if(rhsKnown.contains(exp))
                return nameProvider.nameFor(exp);
            else {
                OWLClass name = nameProvider.nameFor(exp);
                collectedClasses.add(name);
                rhsKnown.add(exp);
                exp.accept(this).forEach(
                        rhs -> {
                            OWLAxiom definition = factory.getOWLSubClassOfAxiom(name, rhs);
                            transformedAxioms.add(definition);
                        }
                );
                return name;
            }
        }

        private Stream<OWLClassExpression> singleton(OWLClassExpression exp) {
            return Stream.<OWLClassExpression>builder()
                    .add(exp)
                    .build();
        }

        @Override
        public Stream<OWLClassExpression> visit(OWLObjectIntersectionOf ce) {
            return ce.operands()
                    .flatMap(c -> c.accept(this));
        }

        @Override
        public Stream<OWLClassExpression> visit(OWLObjectSomeValuesFrom ce) {
            collectedProperties.add((OWLObjectProperty) ce.getProperty());
            OWLClassExpression flattened =
                    factory.getOWLObjectSomeValuesFrom(ce.getProperty(), flatten(ce.getFiller()));
            return singleton(flattened);
        }

        @Override
        public Stream<OWLClassExpression> visit(OWLClass ce) {
            if(TOP_AS_CONCEPT && ce.isOWLThing())
                return singleton(factory.getOWLClass(TOP_CONCEPT));
            collectedClasses.add(ce);
            return singleton(ce);
        }

        @Override
        public <T> Stream<OWLClassExpression> doDefault(T object) {
            throw new NotInELException("Not supported: " + object);
        }
    };

    private static final class NotInELException extends IllegalArgumentException {
        public NotInELException(String message){
            super(message);
        }
    }
}
