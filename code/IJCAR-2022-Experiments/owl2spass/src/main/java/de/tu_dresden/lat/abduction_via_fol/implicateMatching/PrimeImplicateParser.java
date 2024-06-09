package de.tu_dresden.lat.abduction_via_fol.implicateMatching;

import java.io.*;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class PrimeImplicateParser {


    private Set<NegativeGroundClause> negClauses = new HashSet<>();
    private PositiveClauseCollection posClauses = new PositiveClauseCollection();

    private final static String skolemTermPattern =
            "sk[skfc0-9\\)\\(]+";

    private final static Pattern posClausePattern = Pattern.compile(
            "clause\\( *\\|\\| *-> *([A-Za-z0-9_]+)\\(("+ skolemTermPattern +")\\),[0-9]+\\).");

    private final static Pattern negClausePattern = Pattern.compile(
            "clause\\( *\\|\\| *(([A-Za-z0-9_]+\\("+skolemTermPattern+"\\) *)+) -> ,[0-9]+\\).");

    private final static Pattern litMatcher = Pattern.compile(
            "([A-Za-z0-9_]+)\\(("+ skolemTermPattern +")\\)");


    public void parse(File file) throws IOException {
        negClauses = new HashSet<>();
        posClauses = new PositiveClauseCollection();

        BufferedReader reader = new BufferedReader(new FileReader(file));

        String line;
        while( ( line=reader.readLine() )!=null){
            line = line.trim();
            parse(line);
        }
        reader.close();
    }

    public Set<NegativeGroundClause> getNegClauses(){
        return negClauses;
    }

    public PositiveClauseCollection getPosClauses(){
        return posClauses;
    }

    private void parse(String line){
        //System.out.println("checking "+line);
        line = line.trim();

        matchPosClause(line);
        matchNegClause(line);
    }

    private void matchPosClause(String line){
        Matcher posM = posClausePattern.matcher(line);
        if(posM.matches()){{
            //System.out.println("matched positive clause pattern: "+line);
            posClauses.addClause(new ClassName(posM.group(1)), new Term(posM.group(2)));}
        }
    }

    private void matchNegClause(String line) {
        Matcher posM = negClausePattern.matcher(line);
        if(posM.matches()){
            //System.out.println("matched negative clause pattern:" +line);
            NegativeGroundClause clause = new NegativeGroundClause();
            String litList = posM.group(1);
            for(String lit: litList.split(" ")){
                //System.out.println(lit);
                Matcher litM = litMatcher.matcher(lit);
                if(!litM.matches()){
                    throw new AssertionError("Unexpected literal: "+lit);
                }
                clause.addLiteral(new ClassName(litM.group(1)), new Term(litM.group(2)));
            }
            negClauses.add(clause);
        }
    }


    public static void main(String[] args){
        Matcher matcher = posClausePattern.matcher("clause( ||  -> A(skc1),12).");
        assert(matcher.matches());
        System.out.println(matcher.matches());
        System.out.println(matcher.group(0));
        System.out.println(matcher.group(1));
        System.out.println(matcher.group(2));
        System.out.println();
        matcher = posClausePattern.matcher("clause( || C(u) r2(skc1,u) -> ,14).");
        assert(!matcher.matches());
        matcher = posClausePattern.matcher("clause( ||  -> A(skf3(u)),4).)");
        System.out.println(matcher.matches());
        assert(!matcher.matches());
        matcher = posClausePattern.matcher("clause( ||  -> A(skf3(u)),4).");
        System.out.println(matcher.matches());
        assert(!matcher.matches());

        matcher = negClausePattern.matcher("clause( || E(skf2(skc1)) B(sk5)   -> ,20).");
        assert(matcher.matches());
        System.out.println(matcher.matches());
        System.out.println(matcher.group(0));
        System.out.println(matcher.group(1));
        System.out.println(matcher.group(2));

        String litList = matcher.group(1);
        for(String lit: litList.split(" ")){
            System.out.println(lit);
            Matcher litM = litMatcher.matcher(lit);
            System.out.println(litM.matches());
            System.out.println(litM.group(1));
            System.out.println(litM.group(2));
        }


        matcher = negClausePattern.matcher("clause( || E(u) C(skc1) r1(skc1,u) -> ,13).");
    }

}
