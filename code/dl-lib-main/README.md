# Overview

dl-lib is a library to facilitate working with description logic ontologies. Different to existing libraries, dl-lib does not assume knowledge of OWL terminology but instead 
uses terminology common in description logic contexts (e.g.: "Class Expressions" are called "Concepts"). This is supposed to make it easier to work with it for implementing
quick prototypes for students or other users that are more used to description logic terminology. However, parsing and writing ontologies in OWL is supported, so that real
ontologies can be accessed and produced with this library.

The library is implemented with a usage from Python and Scala in mind.

# Requirements

You will need maven to compile the project (https://maven.apache.org), and Java 11 (check https://sdkman.io for an easy way to manage multiple java versions on your system). 
To use it from Python, you will need py4j, which can be conveniently installed using pip.

# Installation

Just run the following line in command line:

    mvn install

# Usage from Python

You will need to run a gateway server to be able to communicate with the library from Python. This is done as follows:

    java -jar target/dl-lib-0.1.5-jar-with-dependencies.jar

While a proper documentation of the library is not available yet, you can consult the example file (example.py) to see how the main functionalities are used from python.

To test the example, download the pizza ontology and put it in the folder where you run the gateway server. Then run the example file as follows:

    python example.py
