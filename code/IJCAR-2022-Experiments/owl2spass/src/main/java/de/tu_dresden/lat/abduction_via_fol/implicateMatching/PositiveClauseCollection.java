package de.tu_dresden.lat.abduction_via_fol.implicateMatching;

import com.google.common.collect.HashMultimap;
import com.google.common.collect.Multimap;

import java.util.Collection;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Representation of a set of positive ground clauses, as relevant for the term matching
 */
public class PositiveClauseCollection {
    private final Multimap<Term, ClassName> term2predicates = HashMultimap.create();

    public void addClause(ClassName className, Term term) {
        //if(className.toString().equals("A_TOP"))
        term2predicates.put(term, className);
    }

    public Set<Term> getTerms(){
        return term2predicates.keySet();
    }

    public Collection<ClassName> classesFor(Term term){
        return term2predicates.get(term);
    }

    public String toString(){
        return term2predicates.entries()
                .stream()
                .map(e -> e.getValue()+"("+e.getKey()+")")
                .collect(Collectors.joining("\n"));
    }

    public int size() {
        return term2predicates.entries().size(); // counts the entries of the multimap, not the underlying map
    }
}
