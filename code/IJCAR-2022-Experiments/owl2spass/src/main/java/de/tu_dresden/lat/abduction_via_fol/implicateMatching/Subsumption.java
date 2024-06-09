package de.tu_dresden.lat.abduction_via_fol.implicateMatching;

import java.util.Collection;
import java.util.Objects;
import java.util.stream.Collectors;

public class Subsumption {
    private Collection<ClassName> lhs;
    private Collection<ClassName> rhs;

    public Subsumption(Collection<ClassName> lhs, Collection<ClassName> rhs){
        assert lhs!=null && rhs!=null;
        this.lhs=lhs;
        this.rhs=rhs;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Subsumption that = (Subsumption) o;
        return lhs.equals(that.lhs) && rhs.equals(that.rhs);
    }

    public int size() {
        return lhs.size() + rhs.size();
    }

    @Override
    public int hashCode() {
        return Objects.hash(lhs, rhs);
    }

    public String toString(){
        String lhsString = lhs.stream()
                .map(c -> c.toString())
                .collect(Collectors.joining(" AND "));
        if(lhsString.equals(""))
            lhsString = "TOP";
        return lhsString
                + " SUBCLASS_OF "
                + rhs.stream()
                .map(c -> c.toString())
                .collect(Collectors.joining(" AND "));
    }
}
