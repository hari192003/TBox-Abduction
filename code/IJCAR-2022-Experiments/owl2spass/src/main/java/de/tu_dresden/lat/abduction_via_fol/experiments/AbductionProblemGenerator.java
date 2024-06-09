package de.tu_dresden.lat.abduction_via_fol.experiments;

import com.clarkparsia.owlapi.explanation.DefaultExplanationGenerator;
import com.clarkparsia.owlapi.explanation.util.SilentExplanationProgressMonitor;
import com.google.common.collect.HashMultimap;
import com.google.common.collect.Multimap;
import de.tu_dresden.lat.abduction_via_fol.ontologyTools.ELFilter;
import de.tu_dresden.lat.abduction_via_fol.ontologyTools.OntologyTools;
import de.tu_dresden.lat.abduction_via_fol.owl2spass.OWL2SpassConverter;
import org.liveontologies.puli.DynamicProof;
import org.liveontologies.puli.Inference;
import org.liveontologies.puli.InferenceJustifier;
import org.liveontologies.puli.InferenceJustifiers;
import org.liveontologies.puli.pinpointing.InterruptMonitor;
import org.liveontologies.puli.pinpointing.MinimalSubsetCollector;
import org.liveontologies.puli.pinpointing.MinimalSubsetEnumerators;
import org.liveontologies.puli.pinpointing.MinimalSubsetsFromProofs;
import org.semanticweb.HermiT.ReasonerFactory;
import org.semanticweb.elk.owlapi.ElkReasoner;
import org.semanticweb.elk.owlapi.ElkReasonerFactory;
import org.semanticweb.elk.owlapi.proofs.ElkOwlInference;
import org.semanticweb.elk.owlapi.proofs.ElkOwlProof;
import org.semanticweb.owlapi.apibinding.OWLManager;
import org.semanticweb.owlapi.model.*;
import org.semanticweb.owlapi.model.parameters.AxiomAnnotations;
import org.semanticweb.owlapi.model.parameters.Imports;
import org.semanticweb.owlapi.model.parameters.Navigation;
import org.semanticweb.owlapi.reasoner.OWLReasoner;
import org.semanticweb.owlapi.reasoner.OWLReasonerFactory;
import org.semanticweb.owlapi.util.OWLAxiomSearchFilter;

import javax.annotation.Nullable;
import java.io.*;
import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Create abduction problems for a given ontology.
 */
public class AbductionProblemGenerator {

    private enum AbductionProblemVariant {
        NO_OBSERVATION,
        RANDOM_OBSERVATION_NON_ENTAILED,
        RANDOM_OBSERVATION_ENTAILED,
        JUSTIFICATION_BASED,
        MODULE_BASED,
        RANDOM_OBSERVATION_GENUINE_MODULE,
    }

    public static void main(String[] args) throws OWLOntologyCreationException, IOException, OWL2SpassConverter.TranslationException, ProblemGenerationException, OWLOntologyStorageException {
        if (args.length < 2) {
            System.out.println("Usage:");
            System.out.print(
                    "java " + AbductionProblemGenerator.class.getCanonicalName());
            System.out.println(" INPUT_OWL_FILE OUTPUT_SPASS_FILE [random_non_entailed|random_entailed|just_based|module_based] [sat-tbox]");
            System.out.println();
            System.out.print("The arguments [random_non_entailed|random_entailed|just-based] are optional, . ");
            System.out.print("If provided, the generated spass file will contain ");
            System.out.print("a list of formulae named \"conjectures\" ");
            System.out.print("corresponding to a concept inclusion. ");
            System.out.print("The parameter determines how this conjecture is chosen\n");
            System.out.println("  random_non_entailed: pick random atomic CI that is not entailed" );
            System.out.println("  random_entailed: pick random atomic CI that is entailed, and remove sufficient axioms to break that entailment using a repair" );
            System.out.println("  just_based: pick random atomic CI that is entailed, take a justification for it, and remove a random axiom from it (should generate small examples)" );
            System.out.println("  module_based: as before, but using a star module instead of a justification (should generate slightly larger examples)" );
            System.out.println();
            System.out.print("If the optional parameter 'sat-tbox' is given, the TBox to be used is saturated before it is translated.");


            System.exit(0);
        }

        AbductionProblemVariant useObservation = AbductionProblemVariant.NO_OBSERVATION;

        boolean saturateTBox = false;

        if (args.length >2 ) {
            if (args[2].equals("sat_tbox"))
                saturateTBox = true;
            else {
                String obsArg = args[2];
                System.out.println("Use " + obsArg);
                switch (obsArg) {
                    case "random_non_entailed":
                        useObservation = AbductionProblemVariant.RANDOM_OBSERVATION_NON_ENTAILED;
                        break;
                    case "random_entailed":
                        useObservation = AbductionProblemVariant.RANDOM_OBSERVATION_ENTAILED;
                        break;
                    case "just_based":
                        useObservation = AbductionProblemVariant.JUSTIFICATION_BASED;
                        break;
                    case "module_based":
                        useObservation = AbductionProblemVariant.MODULE_BASED;
                        break;
                    default:
                        System.out.println("Unknown observation type");
                        System.out.println("Use one of : random_non_entailed, random_entailed, just_based");
                        System.out.println("run without arguments for help.");
                        System.exit(1);
                }
            }
        }
        if(args.length>3 && args[3].equals("sat_tbox"))
            saturateTBox=true;

        System.out.println("Loading ontology..");
        OWLOntologyManager manager = OWLManager.createOWLOntologyManager();
        OWLOntology ontology = manager
                .loadOntologyFromOntologyDocument(new File(args[0]));
        System.out.println("done loading ontology.");
        ELFilter.deleteNonELAxioms(ontology);

        OWLSubClassOfAxiom observation;

        AbductionProblemGenerator problemGenerator = new AbductionProblemGenerator(ontology);

        // in some cases we get an exception - retry maximally 5 times in this case
        int triesLeft = 5;
        boolean success = false;

        boolean useModules = true;

        while(!success && triesLeft>0) {

            success=true; // start optimistic

            switch (useObservation) {
                case NO_OBSERVATION:
                    observation = null;
                    break;
                case RANDOM_OBSERVATION_NON_ENTAILED:
                    observation = problemGenerator.problemWithRandomObservation();
                    break;
                case RANDOM_OBSERVATION_ENTAILED:
                    System.out.println("Building observation from entailed CIs");
                    observation = problemGenerator.problemFromEntailedCI();
                    break;
                case JUSTIFICATION_BASED:
                    System.out.println("Building observation and TBox based on justification.");
                    AbductionProblem ap = problemGenerator.justificationBasedProblem(ontology);
                    observation=ap.getObservation();
                    ontology=ap.getOntology();
                    useModules=false; // not needed here
                    break;
                case MODULE_BASED:
                    System.out.println("Building observation and TBox based on justification.");
                    AbductionProblem ap2 = problemGenerator.moduleBasedProblem();
                    observation=ap2.getObservation();
                    ontology=ap2.getOntology();
                    useModules=false; // not needed here
                    break;
                case RANDOM_OBSERVATION_GENUINE_MODULE:
                    observation = problemGenerator.extractRandomObservationFromModule();
                    break;
                default:
                    observation = null;
            }

            System.out.println("Observation: " + observation);
            System.out.println("Ontology size: "+ontology.getAxiomCount(Imports.INCLUDED));

            manager.saveOntology(ontology, new FileOutputStream(new File("ontology.owl")));
            OWLOntology obsOntology = manager.createOntology();
            obsOntology.addAxiom(observation);
            manager.saveOntology(obsOntology, new FileOutputStream(new File("observation.owl")));

            System.out.println("Converting...");
            OutputStream output = new FileOutputStream(new File(args[1]));
            PrintWriter writer = new PrintWriter(output);
            OWL2SpassConverter converter = new OWL2SpassConverter(useModules);


            try {
                if (useObservation.equals(AbductionProblemVariant.NO_OBSERVATION))
                    converter.convertOntology(ontology, writer, args[0], saturateTBox);
                else {
                    String boundFileName = args[1]+".bound";
                    converter.convertAbductionProblem(
                            new AbductionProblem(ontology, observation), writer, args[0], saturateTBox, boundFileName);
                }
            } catch(NullPointerException exp) {
                System.out.println("That null pointer exception again!");
                exp.printStackTrace();
                System.out.println("I try again ("+triesLeft+" tries left)");
                triesLeft--;
                success=false;
            } finally {
                output.close();
            }

            output.close();
        }
        System.out.println("Done.");
    }


    private OWLOntology ontology;

    private final OWLReasonerFactory reasonerFactory;
    private OWLReasoner reasoner;

    public AbductionProblemGenerator(OWLOntology ontology){
        this.ontology=ontology;
        this.reasonerFactory = new ElkReasonerFactory(); //ReasonerFactory();
        this.reasoner = reasonerFactory.createReasoner(ontology);
    }

    public void setOntology(OWLOntology ontology) {
        this.ontology=ontology;
        reasoner = reasonerFactory.createReasoner(ontology);
    }

    private OWLSubClassOfAxiom problemWithRandomObservation() {

        OWLDataFactory factory = ontology.getOWLOntologyManager().getOWLDataFactory();

        OWLSubClassOfAxiom result = null;
        List<OWLClass> classes = ontology.classesInSignature()
                .collect(Collectors.toList());

        Multimap<OWLClass, OWLClass> processedPairs = HashMultimap.create();

        while(result==null || entailed(result)){
            if(classes.isEmpty()){
                throw new IllegalArgumentException("No non-entailed observations can be found.");
            }
            Random random = new Random();
            OWLClass cl1 = classes.get(random.nextInt(classes.size()));

            Set<OWLClass> candidates = new HashSet<>(processedPairs.get(cl1));
            candidates.retainAll(classes);// necessary because we add it again, and don't want to add something previously removed

            classes.removeAll(candidates);

            if(classes.isEmpty()){
                classes.addAll(candidates);
                classes.remove(cl1);
            } else {
                OWLClass cl2 = classes.get(random.nextInt(classes.size()));
                processedPairs.put(cl1,cl2);
                result = factory.getOWLSubClassOfAxiom(cl1, cl2);
                classes.addAll(candidates);
            }
        }
        return result;
    }

    private OWLSubClassOfAxiom problemFromEntailedCI() throws ProblemGenerationException {
        OWLSubClassOfAxiom observation = getEntailedCI();

        repairEntailment(ontology,observation);

        return observation;
    }

    /**
     * Pick a random entailed CI. Replace the ontology by a justification for this CI. Now just remove a random axiom.
     * @return
     * @throws ProblemGenerationException
     */
    private AbductionProblem justificationBasedProblem(OWLOntology ontology) throws ProblemGenerationException, OWLOntologyCreationException {

        Set<? extends OWLAxiom> justification = new HashSet<>();

        Set<OWLSubClassOfAxiom> processed = new HashSet<>();
        OWLSubClassOfAxiom observation=null;

        while(justification.size()<2) {
            observation = getEntailedCI(processed);
            processed.add(observation);

            justification = getJustification(ontology, observation);
        }

        OWLOntology newOntology = ontology.getOWLOntologyManager().createOntology();
        newOntology.addAxioms(justification);
        newOntology.remove(justification.iterator().next());

        return new AbductionProblem(newOntology, observation);
    }

    /**
     * Pick a random entailed CI. Replace the ontology by a star module for this CI. Then repair the entailment.
     * @return
     * @throws ProblemGenerationException
     */
    private AbductionProblem moduleBasedProblem() throws ProblemGenerationException, OWLOntologyCreationException {
        OWLSubClassOfAxiom observation = getEntailedCI();


        try {
            OWLOntology module  =
                    OntologyTools.extractStarModule(ontology, observation.signature());

            setOntology(module);

            repairEntailment(module, observation);

            return new AbductionProblem(module, observation);


        } catch (OWLOntologyCreationException e) {
            throw new ProblemGenerationException(e);
        }

    }
    /**
     * Removes some minimal set of axioms to make the given axiom non-entailed
     */
    private void repairEntailment(OWLOntology ontology, OWLSubClassOfAxiom observation) {
        while (entailed(observation)) {
            Set<? extends OWLAxiom> justification = getJustification(ontology, observation);
            OWLAxiom remove = justification.iterator().next();
            System.out.println("Removing "+remove);
            ontology.remove(remove);
            reasoner.flush();
        }
    }

    private static class InterruptStatus {
        public boolean status = false;
    }

    private Set<? extends OWLAxiom> getJustification(OWLOntology ontology, OWLLogicalAxiom entailment){
        DynamicProof<ElkOwlInference> proof = ElkOwlProof.create((ElkReasoner) reasoner, entailment);
        final InferenceJustifier<Inference<OWLAxiom>, ? extends Set<? extends OWLAxiom>> justifier =
                InferenceJustifiers
                .justifyAssertedInferences();

        // TODO: any way to avoid computing all?

        InterruptStatus interrupt = new InterruptStatus();

        InterruptMonitor monitor = new InterruptMonitor() {
            @Override
            public boolean isInterrupted() {
                return interrupt.status;
            }
        };

        final Set<Set<? extends OWLAxiom>> just = new HashSet<>();

        MinimalSubsetCollector<OWLAxiom> collector = new MinimalSubsetCollector<OWLAxiom>(just) {
            @Override
            public void newMinimalSubset(Set<OWLAxiom> set) {
                super.newMinimalSubset(set);
                interrupt.status=true;
            }
        };

        MinimalSubsetEnumerators.enumerateJustifications(entailment, proof, justifier, monitor,
                collector);

        return just.iterator().next();

        /*DefaultExplanationGenerator explainer = new DefaultExplanationGenerator(
                ontology.getOWLOntologyManager(), reasonerFactory, ontology,
                new SilentExplanationProgressMonitor());
        return explainer.getExplanation(entailment);*/
    }

    /**
     * Return a random atomic CI that is "non-trivially" entailed, which means:
     * 1. it does not occur syntactically in the ontology
     * 2. the right-hand side is not TOP
     * 3. the left- and right-hand side are not identical
     */
    private OWLSubClassOfAxiom getEntailedCI() throws ProblemGenerationException {
        return getEntailedCI(new HashSet<>());
    }

    private OWLSubClassOfAxiom getEntailedCI(Set<OWLSubClassOfAxiom> processed) throws ProblemGenerationException {
        OWLDataFactory factory = ontology.getOWLOntologyManager().getOWLDataFactory();
        Random random = new Random();

        List<OWLClass> unprocessed = ontology.classesInSignature().collect(Collectors.toList());
        while(!unprocessed.isEmpty()){
            int index = random.nextInt(unprocessed.size());
            OWLClass lhs = unprocessed.get(index);
            unprocessed.remove(index);
            List<OWLClass> unprocessedRHS = new ArrayList(reasoner.getSuperClasses(lhs).getFlattened());
            unprocessedRHS.addAll(reasoner.getEquivalentClasses(lhs).getEntities());
            while(!unprocessedRHS.isEmpty()){
                int index2 = random.nextInt(unprocessedRHS.size());
                OWLClass rhs = unprocessedRHS.get(index2);
                unprocessedRHS.remove(index2);
                OWLSubClassOfAxiom candidate = factory.getOWLSubClassOfAxiom(lhs,rhs);
                if(!processed.contains(candidate) && !rhs.isOWLThing() && !rhs.equals(lhs) && !ontology.containsAxiom(candidate))
                    return candidate;
            }
        }
        throw new ProblemGenerationException("Could not find any non-trivially entailed CI!");
    }

    private OWLSubClassOfAxiom extractRandomObservationFromModule() {
        assert false : "NOT IMPLEMENTED YET!";
        return null;
    }

    private boolean entailed(OWLLogicalAxiom axiom) {
        return reasoner.isEntailed(axiom);
    }

    public static class ProblemGenerationException extends Exception {
        private ProblemGenerationException(String message) {
            super(message);
        }
        private ProblemGenerationException(Throwable cause) {
            super(cause);
        }
    }
}
