package de.tu_dresden.lat.abduction_via_fol.owl2spass;

import java.util.List;
import java.util.stream.Collectors;

import org.semanticweb.owlapi.model.OWLAxiomVisitorEx;
import org.semanticweb.owlapi.model.OWLClassExpression;
import org.semanticweb.owlapi.model.OWLEntity;
import org.semanticweb.owlapi.model.OWLEquivalentClassesAxiom;
import org.semanticweb.owlapi.model.OWLSubClassOfAxiom;
import org.semanticweb.owlapi.model.OWLSubClassOfAxiomSetShortCut;
import org.semanticweb.owlapi.model.OWLSubClassOfAxiomShortCut;

/**
 * 
 * @author Patrick Koopmann
 *
 */
public class Axiom2SpassVisitor implements OWLAxiomVisitorEx<String> {

	private ClassExpression2SpassVisitor classVisitor = new ClassExpression2SpassVisitor();

	public String getName(OWLEntity entity) {
		return classVisitor.getName(entity);
	}

	public String visit(OWLSubClassOfAxiom ax) {
		classVisitor.init();

		return "formula(forall([X1], implies(" + ax.getSubClass().accept(classVisitor) + ", "
				+ ax.getSuperClass().accept(classVisitor) + "))).";
	}

	public String visit(OWLEquivalentClassesAxiom ax) {
		List<OWLClassExpression> operands = ax.getClassExpressionsAsList();
		if (operands.size() != 2)
			return ax.asPairwiseAxioms().stream().map((ax2) -> ax2.accept(this)).collect(Collectors.joining("\n"));
		else {
			classVisitor.init();
			return "formula(forall([X1], equiv(" + operands.get(0).accept(classVisitor) + ", "
					+ operands.get(1).accept(classVisitor) + "))).";
		}
	}

	@Override
	public <T> String doDefault(T object) {
		if (object instanceof OWLSubClassOfAxiomShortCut) {
			return ((OWLSubClassOfAxiomShortCut) object).asOWLSubClassOfAxiom().accept(this);
		} else if (object instanceof OWLSubClassOfAxiomSetShortCut) {
			return ((OWLSubClassOfAxiomSetShortCut) object).asOWLSubClassOfAxioms().stream()
					.map((ax) -> ax.accept(this)).collect(Collectors.joining("\n"));
		} else
			throw new IllegalArgumentException("Not supported: " + object);
	}
}
