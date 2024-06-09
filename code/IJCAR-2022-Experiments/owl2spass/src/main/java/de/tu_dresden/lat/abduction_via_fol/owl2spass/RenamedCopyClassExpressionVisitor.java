package de.tu_dresden.lat.abduction_via_fol.owl2spass;

import de.tu_dresden.lat.abduction_via_fol.tools.StringTools;
import org.semanticweb.owlapi.model.*;

import java.util.stream.Collectors;

public class RenamedCopyClassExpressionVisitor implements OWLClassExpressionVisitorEx<OWLClassExpression> {

    private OWLDataFactory factory;

    public RenamedCopyClassExpressionVisitor(OWLDataFactory factory) {
        this.factory=factory;
    }

    @Override
    public OWLObjectIntersectionOf visit(OWLObjectIntersectionOf intersection) {
        return factory.getOWLObjectIntersectionOf(
                intersection.operands().map(x -> x.accept(this))
        );
    }

    @Override
    public OWLObjectSomeValuesFrom visit(OWLObjectSomeValuesFrom ex) {
        return factory.getOWLObjectSomeValuesFrom(ex.getProperty(),
                ex.getFiller().accept(this));
    }

    @Override
    public OWLClass visit(OWLClass _class) {
        if (_class.isOWLThing())
            return _class;
        else
            return factory.getOWLClass(IRI.create(_class.getIRI().getIRIString()+"__COPY__"));
    }

    @Override
    public <T> OWLClassExpression doDefault(T object) {
        throw new IllegalArgumentException("Not supported: " + object);
    }

}
