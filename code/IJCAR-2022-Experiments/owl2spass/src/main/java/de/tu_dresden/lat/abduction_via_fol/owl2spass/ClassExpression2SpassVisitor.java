package de.tu_dresden.lat.abduction_via_fol.owl2spass;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

import de.tu_dresden.lat.abduction_via_fol.tools.StringTools;
import org.semanticweb.owlapi.model.OWLClass;
import org.semanticweb.owlapi.model.OWLClassExpressionVisitorEx;
import org.semanticweb.owlapi.model.OWLEntity;
import org.semanticweb.owlapi.model.OWLObjectIntersectionOf;
import org.semanticweb.owlapi.model.OWLObjectProperty;
import org.semanticweb.owlapi.model.OWLObjectPropertyExpression;
import org.semanticweb.owlapi.model.OWLObjectSomeValuesFrom;

/**
 * 
 * @author Patrick Koopmann
 *
 */
public class ClassExpression2SpassVisitor implements OWLClassExpressionVisitorEx<String> {

	private Map<OWLEntity, String> nameMap = new HashMap<>();
	private Set<String> usedNames = new HashSet<>();
	private int nestingLevel = 1;



	public void init() {
		nestingLevel = 1;
	}

	@Override
	public String visit(OWLObjectIntersectionOf intersection) {
		String inner = intersection.operands()
				.map((c) -> c.accept(this))
				.collect(Collectors.joining(", "));

		return "and(" + inner + ")";
	}

	@Override
	public String visit(OWLObjectSomeValuesFrom ex) {
		String result = "exists([" + nextVariable() + "], and(" + translate(ex.getProperty()) + "(" + variable() + ", "
				+ nextVariable() + "), ";

		nestingLevel++;
		result += ex.getFiller().accept(this) + "))";
		nestingLevel--;
		return result;
	}

	private String translate(OWLObjectPropertyExpression property) {
		if (property instanceof OWLObjectProperty)
			return getName((OWLObjectProperty) property);
		else
			throw new IllegalArgumentException("Not supported: " + property);
	}

	@Override
	public String visit(OWLClass _class) {
		if (_class.isOWLThing())
			return "true";
		else
			return getName(_class) + "(" + variable() + ")";
	}

	@Override
	public <T> String doDefault(T object) {
		throw new IllegalArgumentException("Not supported: " + object);
	}

	public String getName(OWLEntity entity) {
		if (nameMap.containsKey(entity))
			return nameMap.get(entity);
		else {
			String nameStart = entity.getIRI().getShortForm();
			nameStart = StringTools.toAlphaNumeric(nameStart);
			if(forbidden(nameStart))
				nameStart = "_FORBIDDEN_NAME_";
			if(nameStart=="")
				nameStart="A";
			int counter = 0;
			String name = nameStart;
			while (usedNames.contains(name)) {
				name = nameStart + "_"+ counter;
				counter++;
			}
			nameMap.put(entity, name);
			usedNames.add(name);
			return name;
		}
	}

	private static Set<String> forbiddenNames = new HashSet<>();

	private static void fillForbiddenNames() {
		forbiddenNames = Set.of(  "MIN",
				"AED",
				"App",
				"CRW",
				"Con",
				"Def",
				"EmS",
				"EqF",
				"EqR",
				"Fac",
				"Inp",
				"MPm",
				"MRR",
				"OHy",
				"OPm",
				"Obv",
				"Res",
				"Rew",
				"SHy",
				"SPASS",
				"SPm",
				"SSi",
				"SoR",
				"SpL",
				"SpR",
				"Spt",
				"Ter",
				"Top",
				"URR",
				"UnC",
				"all",
				"and",
				"author",
				"axioms",
				"begin_problem",
				"by",
				"box",
				"clause",
				"cnf",
				"comp",
				"concept_formula",
				"conjectures",
				"conv",
				"date",
				"description",
				"dia",
				"div",
				"DL",
				"dnf",
				"domain",
				"domrestr",
				"eml",
				"EML",
				"end_of_list",
				"end_problem",
				"equal",
				"equiv",
				"exists",
				"false",
				"forall",
				"formula",
				"freely",
				"functions",
				"generated",
				"hypothesis",
				"id",
				"implied",
				"implies",
				"include",
				"list_of_clauses",
				"list_of_declarations",
				"list_of_descriptions",
				"list_of_formulae",
				"list_of_general_settings",
				"list_of_includes",
				"list_of_proof",
				"list_of_settings",
				"list_of_special_formulae",
				"list_of_symbols",
				"list_of_terms",
				"logic",
				"name",
				"nand",
				"nor",
				"not",
				"operators",
				"or",
				"predicate",
				"predicates",
				"prop_formula",
				"quantifiers",
				"ranrestr",
				"range",
				"rel_formula",
				"role_formula",
				"satisfiable",
				"set_flag",
				"set_precedence",
				"set_selection",
				"set_ClauseFormulaRelation",
				"set_DomPred",
				"some",
				"sort",
				"sorts",
				"splitlevel",
				"status",
				"step",
				"subsort",
				"sum",
				"test",
				"translpairs",
				"true",
				"unknown",
				"unsatisfiable",
				"version",
				"xor"
		);
	}

	/** checks for predicate names forbidden by SPASS **/
	private static boolean forbidden(String name) {
		if(forbiddenNames.isEmpty())
			fillForbiddenNames();
		return forbiddenNames.contains(name);
	}

	private String variable() {
		return "X" + nestingLevel;
	}

	private String nextVariable() {
		return "X" + (nestingLevel + 1);
	}

}
