package de.tu_dresden.lat.abduction_via_fol.owl2spass;

import org.semanticweb.owlapi.model.*;

import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class RenamedCopyAxiomVisitor implements OWLAxiomVisitorEx<Stream<OWLAxiom>> {

    private RenamedCopyClassExpressionVisitor classVisitor;
    private OWLDataFactory factory;

    public RenamedCopyAxiomVisitor(OWLDataFactory factory){
        this.factory=factory;
        this.classVisitor = new RenamedCopyClassExpressionVisitor(factory);
    }

    public OWLSubClassOfAxiom renameRHS(OWLSubClassOfAxiom ax) {
        return factory.getOWLSubClassOfAxiom(ax.getSubClass(), ax.getSuperClass().accept(classVisitor));
    }

    public Stream<OWLAxiom>  visit(OWLSubClassOfAxiom ax) {
        return Stream.of(factory.getOWLSubClassOfAxiom(
                ax.getSubClass().accept(classVisitor),
                ax.getSuperClass().accept(classVisitor)
        ));
    }

    public Stream<OWLAxiom> visit(OWLEquivalentClassesAxiom ax) {
        return Stream.of(
                factory.getOWLEquivalentClassesAxiom(ax.operands().map(x -> x.accept(classVisitor))));
    }

    @Override
    public <T> Stream<OWLAxiom> doDefault(T object) {
        if (object instanceof OWLSubClassOfAxiomShortCut) {
            return ((OWLSubClassOfAxiomShortCut) object).asOWLSubClassOfAxiom().accept(this);
        } else if (object instanceof OWLSubClassOfAxiomSetShortCut) {
            return ((OWLSubClassOfAxiomSetShortCut) object).asOWLSubClassOfAxioms().stream()
                    .flatMap((ax) -> ax.accept(this));
        } else
            throw new IllegalArgumentException("Not supported: " + object);
    }
}
