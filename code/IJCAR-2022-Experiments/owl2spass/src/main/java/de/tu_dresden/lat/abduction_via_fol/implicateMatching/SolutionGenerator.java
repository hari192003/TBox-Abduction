package de.tu_dresden.lat.abduction_via_fol.implicateMatching;

import org.semanticweb.owlapi.model.OWLClass;

import java.util.*;
import java.util.stream.Collectors;

public class SolutionGenerator {


    private final PositiveClauseCollection posClauses;

    public SolutionGenerator(PositiveClauseCollection posClauses) {
        this.posClauses=posClauses;
    }

    public Set<Solution> generateSolutions(Collection<NegativeGroundClause> negClauses) {
        Set<Solution> result = new HashSet<>();
        System.out.println("Generating solutions");
        for(NegativeGroundClause clause:negClauses)
            generateSolution(clause)
                    .stream()
                    .forEach(result::add);
        System.out.println("Selecting subset-minimal ones");
        Set<Solution> subsetMinimal =
                result.stream().filter(x ->
                        result.stream().anyMatch(y ->
                                x.subsumptions().size()>y.subsumptions().size()
                                        && x.subsumptions().containsAll(y.subsumptions())

                        )).collect(Collectors.toSet());
        return result;
    }

    public Optional<Solution> generateSolution(NegativeGroundClause negClause) {
        Solution solution = new Solution();

        long maxNestingDepth =0;

        for(Term term: negClause.getTerms()) {
            maxNestingDepth = Math.max(maxNestingDepth, nestingDepth(term));
            Collection<ClassName> lhs = posClauses.classesFor(term);
            if(lhs.isEmpty())
                return Optional.empty(); // one term wasn't matched, so there is no solution
            Collection<ClassName> rhs = negClause.classesFor(term);

            rhs.removeAll(lhs);
            if(!rhs.isEmpty())
                solution.addSubsumption(new Subsumption(posClauses.classesFor(term), negClause.classesFor(term)));
        }

        if(maxNestingDepth>0){
            System.out.println("Skolem nesting in solution >0: "+maxNestingDepth);
        }

        return Optional.of(solution);
    }

    private static long nestingDepth(Term term) {
        return term
                .toString()
                .chars()
                .filter(c -> c=='(')
                .count();
    }
}
