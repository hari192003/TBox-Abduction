#! /bin/bash

# run abduction experiments on a folder
# -m <method> method to be used (mention first)
# -n <number> how often to repeat each experiment
# -d <folder> folder with ontology files
# -o <folder> folder where to put output files
# -t <timeout> (overall) timeout to be used on each experiment
# -l <timeout> timeout (in seconds) to be used by spass
# -f <filenames> list of filenames

while getopts ":f:m:n:d:o:t:l:" opt; do

    case $opt in
	f)
	    filenames=$OPTARG
	    echo Using filenames from $filenames
	    ;;
	m)
	    method=$OPTARG
	    echo Use method $method for creating abduction problems
	    ;;
	n)
	    number=$OPTARG
	    echo Repeat each experiment $number times
	    ;;
	d)
	    folder=$OPTARG
	    echo use ontologies in $folder
	    ;;
	o)
	    output_folder=$OPTARG
	    echo write output files to $output_folder
	    ;;
	t)
	    timeout=$OPTARG
	    echo timeout set to $timeout
	    ;;
	l)
	    timelimitSpass=$OPTARG
	    echo spass timeout set to $timelimitSpass
    esac
done

echo Method: $method > $output_folder/parameters.txt
echo Repetitions: $number >> $output_folder/parameters.txt
echo Ontology folder: $folder >> $output_folder/parameters.txt
echo Timeout: $timeout >> $output_folder/parameters.txt
echo Ontologies taken from: $filenames >> $output_folder/parameters.txt
echo SPASS time limit: $timelimitSpass >> $output_folder/parameters.txt

#for ontology in `ls -Sr $folder`; do
for ontology in `cat $filenames`; do

    echo processing $ontology
    echo ====================================================================
    
    for iteration in `seq $number`; do

	echo Iteration $iteration
	echo ===============
	
	output_prefix=$output_folder/$ontology.$iteration
	
	echo $output_prefix

#	if [ -f $output_prefix.dfg -a ! -f $output_prefix.dfg.model ]; then
	if [ ! -f $output_prefix.dfg ]; then

#	    rm $output_prefix.dfg

	    echo recreating $output_prefix.dfg
	    
	    echo timeout $timeout ./ABDUCT.sh -m $method -f $output_prefix.dfg -t $folder/$ontology
	    (timeout $timeout ./ABDUCT.sh -m $method -f $output_prefix.dfg -t $folder/$ontology) &> $output_prefix.translate-output
	fi

	if [ ! -f $output_prefix.dfg.model ]; then
	    echo timeout $timeout ./ABDUCT.sh -l $timelimitSpass -f $output_prefix.model -e $output_prefix.dfg
	    (timeout $timeout ./ABDUCT.sh  -l $timelimitSpass -f $output_prefix.model -e $output_prefix.dfg) &> $output_prefix.abduce-output
	else
	    echo skipping $output_prefix #- DFG file already existed!
	fi
	
    done
done
