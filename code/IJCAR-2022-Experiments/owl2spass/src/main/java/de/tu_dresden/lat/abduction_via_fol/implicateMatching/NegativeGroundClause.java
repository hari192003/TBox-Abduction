package de.tu_dresden.lat.abduction_via_fol.implicateMatching;

import com.google.common.collect.*;

import java.util.Collection;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Represents a negative ground clause in the way it is relevant for the hypothesis generation.
 */
public class NegativeGroundClause {
    private final Multimap<Term, ClassName> term2predicates = HashMultimap.create();

    public void addLiteral(ClassName className, Term term) {
        className = new ClassName(className.toString().replaceAll("__COPY__",""));

        if(!className.toString().equals("A_TOP"))
            term2predicates.put(term, className);
    }

    public Set<Term> getTerms(){
        return term2predicates.keySet();
    }

    public Collection<ClassName> classesFor(Term term){
        return term2predicates.get(term);
    }

    @Override
    public String toString() {
        return term2predicates.entries()
                .stream()
                .map(e -> "¬"+e.getValue()+"("+e.getKey()+")")
                .collect(Collectors.joining(" v "));
    }

}
