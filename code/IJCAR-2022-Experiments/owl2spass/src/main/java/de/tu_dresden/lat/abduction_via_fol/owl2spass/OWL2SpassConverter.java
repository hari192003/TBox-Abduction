package de.tu_dresden.lat.abduction_via_fol.owl2spass;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import de.tu_dresden.lat.abduction_via_fol.experiments.AbductionProblem;
import de.tu_dresden.lat.abduction_via_fol.ontologyTools.ELTBoxSaturator;
import org.semanticweb.owlapi.modularity.locality.LocalityClass;
import de.tu_dresden.lat.abduction_via_fol.ontologyTools.ELTBoxNormaliser;
import de.tu_dresden.lat.abduction_via_fol.ontologyTools.NameProvider;
import org.semanticweb.owlapi.model.*;
import org.semanticweb.owlapi.modularity.locality.SyntacticLocalityModuleExtractor;

/**
 *
 * @author Patrick Koopmann
 *
 */
public class OWL2SpassConverter {

	private final static boolean FILTER_NON_EL = false;
	private final boolean useModules;

	private final boolean ADD_RENAMED_COPY=true;

	public OWL2SpassConverter(){
		this(true);
	}
	public OWL2SpassConverter(boolean useModules) {
		this.useModules =useModules;
	}

	public static class TranslationException extends Exception {
		TranslationException(String message, Exception cause) {
			super(message,cause);
		}
	}

	Axiom2SpassVisitor axiomVisitor = new Axiom2SpassVisitor();

	/**
	 *
	 * @param ontology
	 * @param nameProvider
	 * @param filterNonEL If set to true, then all axioms not in pure EL are removed by the transformation.
	 *                    Otherwise, an ontology not in pure EL would throw an exception.
	 * @return
	 * @throws OWLOntologyCreationException
	 */
	private static OWLOntology normalise(OWLOntology ontology,
										 NameProvider nameProvider,
										 boolean filterNonEL) throws OWLOntologyCreationException {

		OWLOntologyManager manager = ontology.getOWLOntologyManager();
		OWLDataFactory factory = manager.getOWLDataFactory();
		ELTBoxNormaliser normaliser = new ELTBoxNormaliser(factory,nameProvider, filterNonEL);
		ontology.logicalAxioms().forEach(ax -> ax.accept(normaliser));
		System.out.println("Ignored axioms (since not in EL): "+normaliser.getNumberOfIgnoredAxioms());
		return manager.createOntology(normaliser.getTransformedAxioms());
	}

	public String convert(OWLLogicalAxiom axiom) {
		try {
			return axiom.accept(axiomVisitor) + "\n";

		} catch (IllegalArgumentException ex) {

			System.out.println(ex.getMessage());
			return "";
		}
	}

	public void convertAbductionProblem(AbductionProblem problem,
										PrintWriter writer, String filename, String boundFilename)
			throws TranslationException {
		convertAbductionProblem(problem,writer,filename,true, boundFilename);
	}

	public void convertAbductionProblem(
			AbductionProblem problem, PrintWriter writer, String filename,
			boolean saturate, String boundFilename)
			throws TranslationException {

		OWLOntology ontology = problem.getOntology();
		OWLSubClassOfAxiom observation = problem.getObservation();

		if(useModules){
			try {
				//ontology = extractModule(ontology, observation.signature());
				ontology = extractModule(ontology, observation);
			} catch(OWLOntologyCreationException ex){
				throw new TranslationException("Error extracting module", ex);
			}
		}

		if(ontology.isEmpty()){
			System.out.println("Ontology for abduction problem empty!");
			System.exit(1);
		}

		// first normalise the ontology
		NameProvider nameProvider = new NameProvider(ontology);
		try {
			ontology = normalise(ontology, nameProvider, FILTER_NON_EL);
		} catch (OWLOntologyCreationException ex) {
			throw new TranslationException("Error normalising ontology", ex);
		}

		long skolemBound = getSkolemBound(ontology);

		try {
			PrintWriter boundWriter = new PrintWriter(new File(boundFilename));
			boundWriter.println(skolemBound);
			boundWriter.close();
		} catch (FileNotFoundException e) {
			throw new TranslationException("error creating bound file", e);
		}

		writeHeader(writer, ontology, filename);
		writeOntologyTranslation(writer, ontology, observation, saturate );
		writeObservation(writer, observation, ontology);
		writeEnd(writer);
	}

	private long getSkolemBound(OWLOntology ontology) {

		// number of class names times number of distinct existential restrictions on the RHS
		return ontology.getClassesInSignature().size()
				* ontology.axioms(AxiomType.SUBCLASS_OF)
				.map(a -> a.getSuperClass())
				.filter(c -> c instanceof  OWLObjectSomeValuesFrom)
				//.collect(Collectors.toSet())
				.count();
	}

	/**
	 * returns the union of the TOP module of the LHS' signature and the
	 * BOTTOM module of the RHS' signature
     *
	 * @throws OWLOntologyCreationException
	 */
	private OWLOntology extractModule(OWLOntology ontology, OWLSubClassOfAxiom observation)
			throws OWLOntologyCreationException {
		OWLOntology module = ontology.getOWLOntologyManager().createOntology();

		Stream<OWLEntity> lhsSignature = observation.getSubClass().signature();
		Stream<OWLEntity> rhsSignature = observation.getSuperClass().signature();

		try {
			new SyntacticLocalityModuleExtractor(LocalityClass.BOTTOM, ontology.axioms())
					.extract(lhsSignature)
					.forEach(module::addAxiom);

			new SyntacticLocalityModuleExtractor(LocalityClass.TOP, ontology.axioms())
					.extract(rhsSignature)
					//.extract(Stream.concat(lhsSignature,rhsSignature))
					.forEach(module::addAxiom);
		} catch (NullPointerException e) {
			e.printStackTrace();
			System.out.println("Null pointer exception while extracting module - taking whole ontology");
			module = ontology;
		}
/*		System.out.println("Module size (old method): "+module.getAxiomCount()+"/"+ontology.getAxiomCount());

		Set<OWLObjectProperty> relevantRoles = module.getObjectPropertiesInSignature();

		module.removeAxioms(module.axioms());


		lhsSignature = observation.getSubClass().signature();
		rhsSignature = observation.getSuperClass().signature();


		Set<OWLEntity> signature = new HashSet();
		lhsSignature.forEach(signature::add);
		rhsSignature.forEach(signature::add);
		relevantRoles.forEach(signature::add);
		new SyntacticLocalityModuleExtractor(LocalityClass.STAR, ontology.axioms())
				.extract(signature.stream())
				.forEach(module::addAxiom);
*/
		System.out.println("Module size: "+module.getAxiomCount()+"/"+ontology.getAxiomCount());

		return module;
	}

	public void convertOntology(OWLOntology ontology,
								PrintWriter writer,
								String filename) throws TranslationException {
		convertOntology(ontology,writer,filename,false);
	}

	public void convertOntology(OWLOntology inputOntology,
						PrintWriter writer,
						String filename, boolean saturate) throws TranslationException {

		// first normalise the ontology
		NameProvider nameProvider = new NameProvider(inputOntology);
		OWLOntology ontology = null;
		try {
			ontology = normalise(inputOntology, nameProvider, FILTER_NON_EL);
		} catch (OWLOntologyCreationException ex) {
			throw new TranslationException("Error normalising ontology", ex);
		}

		writeHeader(writer, inputOntology, filename);
		writeOntologyTranslation(writer, ontology, null, saturate);
		writeEnd(writer);
	}

	private void writeHeader(PrintWriter writer, OWLOntology ontology, String filename) {
		writer.print("begin_problem("
				+ ontology.getOntologyID().getOntologyIRI().orElseGet(() -> IRI.create("noname")).getShortForm()
				+ ").\n");
		writer.print("\n");

		writer.print("list_of_descriptions.\n");
		writer.print("name({* " + ontology.getOntologyID().getOntologyIRI().map((i) -> i.toString()).orElse("noname")
				+ ontology.getOntologyID().getVersionIRI().map((i) -> ", version " + i.getShortForm()).orElse("")
				+ " *}).\n");
		writer.print("author({* left empty as auto-generated from OWL file *}).\n");
		writer.print("status(satisfiable).\n");
		writer.print(
				"description({* obtained from OWL file " + filename + " through its standard FOL translation *}).\n");
		writer.print("end_of_list.\n");
		writer.print("\n");
	}

	private void writeOntologyTranslation(PrintWriter writer, OWLOntology ontology,
										  OWLLogicalAxiom observation, boolean saturate) throws TranslationException {


		System.out.println(ontology);

		if(saturate){
			ELTBoxSaturator saturator = new ELTBoxSaturator();
			saturator.saturate(ontology);
		}

		System.out.println(ontology);

		RenamedCopyAxiomVisitor copier =
				new RenamedCopyAxiomVisitor(ontology.getOWLOntologyManager().getOWLDataFactory());

		if(ADD_RENAMED_COPY) {
			ontology.axioms().flatMap(x -> x.accept(copier)).forEach(ontology::add);
			observation = copier.renameRHS((OWLSubClassOfAxiom) observation);
		}

		writer.print("list_of_symbols.\n");
		// writer.print("functions[].\n");
		writer.print("predicates[\n");
		writer.print(ontology.objectPropertiesInSignature()
				.map((prp) -> "  (" + axiomVisitor.getName(prp) + ", 2)")
				.collect(Collectors.joining(",\n")));
		if (ontology.objectPropertiesInSignature().count() > 0 && ontology.classesInSignature().count() > 0)
			writer.print(",\n");
		writer.print(ontology.classesInSignature()
				.map((cl) -> "  (" + axiomVisitor.getName(cl) + ", 1)")
				.collect(Collectors.joining(",\n")));
		if(observation!=null) {
			writer.print(",\n");
			writer.print(observation.classesInSignature()
					.map((cl) ->"("+axiomVisitor.getName(cl)+", 1)")
					.collect(Collectors.joining(", \n")));
			//writer.print(observation.classesInSignature().map((cl) -> "  (" + axiomVisitor.getName(cl) + ", 1)")
			//		.collect(Collectors.joining(",\n")));
		}
		writer.print("\n].\n");
		writer.print("end_of_list.\n");
		writer.print("\n");

		writer.print("list_of_formulae(axioms).\n");
		writer.print(ontology.logicalAxioms().map((ax) -> convert(ax)).collect(Collectors.joining()));
		writer.print("end_of_list.\n");
		writer.print("\n");
	}

	private void writeObservation(PrintWriter writer, OWLSubClassOfAxiom observation, OWLOntology ontology) {
/*
		if(useObservation!=UseObservation.NO_OBSERVATION) {
			OWLLogicalAxiom observation = null;

			switch(useObservation) {
				case RANDOM_OBSERVATION_ALL:
					observation = extractRandomObservation(ontology, forbiddenNames);
					break;
				case RANDOM_OBSERVATION_GENUINE_MODULE:
					observation = extractRandomObservationFromModule(ontology, forbiddenNames);
					break;
				default:
					assert false: "Unsupported observation type: "+useObservation;
					break;
			}

			System.out.println("Observation: "+observation);
*/

		OWLDataFactory factory = ontology.getOWLOntologyManager().getOWLDataFactory();
		RenamedCopyAxiomVisitor visitor = new RenamedCopyAxiomVisitor(factory);

		writer.print("list_of_formulae(conjectures).\n");
		if(ADD_RENAMED_COPY)
			writer.print(convert(visitor.renameRHS(observation)));
		else
			writer.print(convert(observation));
		writer.print("end_of_list.\n");
		writer.print("\n");

	}

	private void writeEnd(PrintWriter writer) {
		writer.print("end_problem.\n");
		writer.print("\n");

		writer.close();
	}

}
