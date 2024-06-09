
Transform OWL EL ontologies into FOL formulae in spass format.

In order to build the project, you need to have Maven installed, available here:

https://maven.apache.org/

Apart from that, everything should be handled by the dependency management, so nothing else has to be done.


To build, execute:

$ mvn package

This should create the file target/owl2spass-0.1-SNAPSHOT-jar-with-dependencies.jar, which can then be 
executed to translate an OWL file as follows:

$ java -jar target/owl2spass-0.1-SNAPSHOT-jar-with-dependencies.jar INPUT_OWL_FILE OUTPUT_SPASS_FILE

where INPUT_OWL_FILE is the path of the owl file to be translated, and OUTPUT_SPASS_FILE is the path to the spass file 
to be written. Axioms that are not in EL are ignored, with according messages printed to the standard output.


