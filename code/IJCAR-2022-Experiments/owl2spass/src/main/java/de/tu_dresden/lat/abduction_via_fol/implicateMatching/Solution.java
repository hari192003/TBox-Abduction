package de.tu_dresden.lat.abduction_via_fol.implicateMatching;

import java.util.HashSet;
import java.util.Set;
import java.util.stream.Collectors;

public class Solution {
    private final Set<Subsumption> subsumptions = new HashSet<>();

    public void addSubsumption(Subsumption subsumption){
        this.subsumptions.add(subsumption);
    }

    @Override
    public String toString(){
        return subsumptions.stream()
                .map(c -> c.toString())
                .collect(Collectors.joining("\n"));
    }

    public int size() {
        return subsumptions.size();
    }

    public Set<Subsumption> subsumptions() {
        return subsumptions;
    }
}
