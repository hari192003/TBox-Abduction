Experimental data for the submission "Connection-minimal Abduction in EL via translation FOL"
=========================================================================================


Content
--------

 - SPASS-DL - folder with SPASS source code
 - owl2spass - folder with Java source code (translation, experiments,
   hypothesis generation) 
 - under-50,000.txt - file names of ontologies from Bioportal that
   were used in the experiment (those with under 50,000 axioms in their
   EL fragment) 
	

Dependencies
-------------

Compiling the Java code requires *Maven*, available from
https://maven.apache.org/. For compiling SPASS, a *C compiler* is
needed. 


Compiling
----------

Compiling the Java code:

    cd owlspass
    mvn package


Compiling SPASS:

    cd SPASS-DL
    make clean
    make opt



Creating and Solving Abduction Problems
----------------------------------------

The script `ABDUCT.sh` assumes the SPASS binary to be *in the current folder*,
as well as the presence of the owl2spass folder. It's parameters can be found
at the beginning of the file.  It can be used for two things:

  1. Creating Abduction problems and their translation to SPASS-input
     files. For this, you use the arguments `-t <owl-file>`. You also need
     to specify a method for creating abduction problems (`-m <method>`,
     where `<method>` is one of "just_based" (JUSTIF in the paper), "random_entailed" (REPAIR),
     "random_non_entailed" (ORIGIN)).
     
  2. Solving the abduction problem. For this, use the argument
     `-e <file>` with the DFG-file with the SPASS input, and
     `-l <number>` to set the time limit of SPASS to `<number>` seconds


Rerunning the Experiments
--------------------------

To rerun the experiments from the paper, download the Bioportal 2017
Snapshot from Zenodo (https://zenodo.org/record/439510#.Yg0ATJvTWJB).

Then use the script `RUN_EXPERIMENTS.sh` with the following parameters
(assuming the SPASS binary is in the current folder):


 `-m <method>` Selects the method for producing abduction problems. 
Choose one of "just_based", "random_entailed" and
"random_non_entailed". Their meaning is explained in the paper.


 `-n <number>` How many abduction problems per ontology (we used 5 for the paper)


 `-d <ontology-folder>` Folder with the ontology files downloaded from bioportal


  `-o <output-folder>` Folder where to place the outputs generated.


 `-t <timeout>` Hard timeout to be used for each of the 3 steps (problem generation,
prime implicate generation, and hypothesis construction) 


 `-l <timelimit>` Soft time limit for SPASS


 `-f <filenames>` File containing ontology file names to be used. We used "under-50,000.txt". 


We analysed the completed experiments with the script `analyse.sh`. It
takes two arguments:  first the output folder used for
RUN_EXPERIMENTS.sh, and then a file with ontology-file names
("under-50,000.txt"). The script analyses the output files and outputs
the number of experiments, successful translations/runs and how often
SPASS stopped due to its time limit. Furthermore, several text files
are created in the experiment folder per abduction problem the numbers of solutions,
the maximal size of a solutions, the maximal size of an axiom, and
the run time (in seconds) of the entire abduction process (reasoning
with SPASS plus construction of hypotheses). 




On how we use SPASS-DL
-----------------------

The key SPASS flags are as follows

* `-SOS` applies the set-of-support strategy that is guaranteed to be complete and enforces R1 
* `-BoundVars=1` constrains inferences such that clauses with more than one variable are not generated, as needed to enforce R2.
* `-BoundStart=n -BoundLoops=1 -BoundMode=2` can be used to limit the depth of Skolem terms to `n` the bound from R3, that can be computed during the preprocessing.
* `-TimeLimit=p` enforces a soft time limit of `p` seconds after which we are guaranteed to recover all prime implicates already found in a generated file by using additionally the flags `-PKept` and `-FPModel`.

The others flags passed to SPASS are essentially technical flags used to ensure that
- only standard inferences are applied following a breadth-first strategy regarding the depth of terms in the implicates generated (`-Sorts=0 -Auto=0 -RFSub -RBSub -ISRe -ISFc -WDRation=1`).
- we silence the output generation (`-PBDC`)
- we also turn off strong Skolemization (`-CNFStrSkolem=0 -CNFOptSkolem=0`) that is not compatible with our translation to generate all ground prime implicates.


Authors
---------


[Fajar Haifani](mailto:fhaifani@mpi-inf.mpg.de)

[Patrick Koopmann](mailto:patrick.koopmann@tu-dresden.de) (primary contact for owl2spass)

[Sophie Tourret](mailto:stourret@mpi-inf.mpg.de)

[Christoph Weidenbach](weidenbach@mpi-inf.mpg.de) (primary contact for SPASS)


License
--------

This software is distributed under the BSD-3-Clause license. See the LICENSE file for more details.
