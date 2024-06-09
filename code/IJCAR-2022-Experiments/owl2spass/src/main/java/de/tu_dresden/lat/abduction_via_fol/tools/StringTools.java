package de.tu_dresden.lat.abduction_via_fol.tools;

import java.util.regex.Pattern;

public class StringTools {
    private StringTools() {
        // utilities class
    }

    /**
     * Transform string by replacing alphanumeric characters
     */
    public static String toAlphaNumeric(String string) {
        return string.replaceAll("[^a-zA-Z0-9_]","_");
    }
}
