package de.tu_dresden.lat.abduction_via_fol.implicateMatching;

import java.util.Objects;

public class ClassName {
    private final String name;

    public ClassName(String name){
        this.name=name;
    }

    @Override
    public String toString(){
        return name;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ClassName className = (ClassName) o;
        return Objects.equals(name, className.name);
    }

    @Override
    public int hashCode() {
        return Objects.hash(name);
    }
}
