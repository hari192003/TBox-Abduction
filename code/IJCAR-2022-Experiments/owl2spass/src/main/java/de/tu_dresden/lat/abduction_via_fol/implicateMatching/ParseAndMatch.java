package de.tu_dresden.lat.abduction_via_fol.implicateMatching;

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.Comparator;

public class ParseAndMatch {

    public static void main(String[] args) throws IOException {
        String filename = args[0];

        PrimeImplicateParser parser = new PrimeImplicateParser();
        parser.parse(new File(filename));
        PositiveClauseCollection posClauses = parser.getPosClauses();
        System.out.println("Relevant positive ground clauses:");
        System.out.println("-----");
        System.out.println(posClauses);
        System.out.println("number of positive ground clauses: "+posClauses.size());
        System.out.println();
        System.out.println();

        Collection<NegativeGroundClause> negClauses = parser.getNegClauses();
        System.out.println("Relevant negative ground clauses:");
        System.out.println("-----");
        for(NegativeGroundClause cl:negClauses) {
            System.out.println(cl);
        }
        System.out.println("number of negative ground clauses: "+negClauses.size());
        System.out.println();
        System.out.println();


        SolutionGenerator solutionGenerator = new SolutionGenerator(posClauses);

        Collection<Solution> solutions = solutionGenerator.generateSolutions(
                negClauses);

        System.out.println("Solutions: ");
        System.out.println("===========");
        System.out.println();
        for(Solution solution:solutions){
            System.out.println(solution);
            System.out.println("------------");
        }
        System.out.println("number of solutions: "+solutions.size());
        System.out.println("size of largest solution: "
                +solutions.stream().map(s -> s.size()).max(Integer::compare));
        System.out.println("size of largest axiom: "
                +solutions.stream()
                .flatMap(s -> s.subsumptions().stream())
                .map(s -> s.size())
                .max(Integer::compare));
    }
}
