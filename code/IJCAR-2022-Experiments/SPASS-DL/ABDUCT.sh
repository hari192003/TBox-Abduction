#! /bin/bash

# EL Abduction using FO Prime Implicate
# -m <method> using 'method' for creating the abduction problem (based on selected ontology)
# -f <filename> filename used for output dfg-file
# -t <file> normalizes and translates EL description logic file.owl into normalized first order clauses file.owl.dfg
# -t <folder> translates all file with owl extension.
# -e <file.dfg> generates prime implicates and outputs the abductive explanation.
# -e <folder> generates abductive explanation for all dfg file extension.
# -l <number> set timelimit to <number> seconds

while getopts ":l:m:f:t:e:" opt; do
    case $opt in
    l)
	timelimit=${OPTARG}
	echo Using timelimit of $timelimit seconds
	;;
    m)
	method=${OPTARG}
	echo Using method $method for creating abduction problems
        ;;
    f)
	output=${OPTARG}
	echo Using file $output for output
	;;
    t)
	fd=${OPTARG}
	if [[ -d $fd ]]; then
	    # Comment (Patrick): I am ignoring the following, as I will not be using it
	    echo "translating folder $OPTARG" >&2
	    rm $fd/translationerror.txt
            rm $fd/translationtime.txt
	    if [[ ! -d "$fd/dfg/" ]]; then
		mkdir "$fd/dfg/"
	    fi		
		
	    for file in $fd/*
		do
		    if [[ -f $file && [[$file =~ \.xml$ || $file =~ \.owl$]] ]]; then
			echo "Translation for $file"

			dfg="$fd/dfg/${file##*/}.dfg"
#			if [[ -s $dfg ]]  ;then
#				echo "Skipping: translation already exists"
#				continue
#			fi
			# Note: the previous check does not make sense, as we change the observation each time
			echo "=====Translation for $file =====" >>$fd/translationerror.txt
			echo "=====Translation for $file =====" >>$fd/translationtime.txt

		        { time `(java -jar owl2spass/target/owl2spass-0.1-SNAPSHOT-jar-with-dependencies.jar "$file" "$dfg" $method & 2>>$fd/translationerror.txt 1>>$fd/translationlog.txt)  ` ; } 2>>$fd/translationtime.txt

			if [[ -s $dfg ]]  ;then
			    echo "  dfg file $dfg generated"
			else
			    echo "  unsuccessful, please check log"
			    rm $dfg
			fi
		    fi
		done
	elif [[ -f $fd ]]; then
	    dfg=$output
	    echo "translating file $OPTARG" >&2
	    time java -jar owl2spass/target/owl2spass-0.1-SNAPSHOT-jar-with-dependencies.jar "$fd" "$dfg" $method sat-tbox
	else
	    echo "$fd is not valid"
	    exit 1
	fi;;
    e)
      echo "executing $OPTARG" >&2
	fd=$OPTARG
	if [[ -d $fd ]]; then
	    echo FOLDER
	    for file in $fd/*
		do
		    if [[ -f $file ]] && [[ $file =~ \.dfg$ ]]; then
			echo " Abduction for $file"
			echo "=====PIGen for $file =====" >>$fd/executionerror.txt
			echo "=====PIGen for $file =====" >>$fd/executiontime.txt
			{ time `(./SPASS -SOS -Sorts=0 -Auto=0 -RFSub -RBSub -ISRe -ISFc -FPModel -PKept -BoundVars=1 -DLSigBound -PBDC -CNFStrSkolem=0 -CNFOptSkolem=0 "$file" & 2>>$fd/executionerror.txt >>$fd/executiontime.txt) ` ; } 2>>$fd/PIGenerationtime.txt
			echo "  PIGenTime: $spassTime"
			spassOutFile="$file.model"
		        if [[ -s $spassOutFile ]]; then
				time java -cp owl2spass/target/owl2spass-0.1-SNAPSHOT-jar-with-dependencies.jar de.tu_dresden.lat.abduction_via_fol.implicateMatching.ParseAndMatch "$spassOutFile"
			else
				echo "  unsuccessful, please check log"
			fi

		    fi
		done
	elif [[ -f $fd ]]; then
	    echo SINGLE FILE

	    bound=$(( 2 + `cat $fd.bound` ))

	    if (( $bound > 3000 )); then
		bound=3000 # quick fix to avoid overflows 
	    fi
	    
	    echo Using bound $bound
	    
	    #./SPASS -SOS -Sorts=0 -Auto=0 -RFSub -RBSub -ISRe -ISFc -FPModel -BoundVars=1 -CNFStrSkolem=0 -CNFOptSkolem=0 -PGiven=0 -PProblem=0 -TimeLimit=1 -BoundMode=2 -BoundStart=2 -BoundLoops=1 "../entailed-bioportal-01-28/bao-gpcr.g-protein-coupled-receptor-bioassays-ontology.23.owl.xml.3.dfg"

	    #	    time ./SPASS -SOS -Sorts=0 -Auto=0 -RFSub -RBSub -ISRe -ISFc -FPModel -BoundVars=1 -CNFStrSkolem=0 -CNFOptSkolem=0 -DLSigBound -TimeLimit=$timelimit  -PGiven=0 -PProblem=0 "$fd"
	    time ./SPASS -SOS -Sorts=0 -Auto=0 -RFSub -RBSub -ISRe -ISFc -FPModel -BoundVars=1 -CNFStrSkolem=0 -CNFOptSkolem=0 -TimeLimit=$timelimit -PGiven=0 -PProblem=0 -BoundMode=2 -BoundStart=$bound -BoundLoops=1 -WDRatio=1 "$fd"

	    if [[ -f $fd.clauses ]]; then
		echo time limit or nesting level reached!
		mv $fd.clauses $fd.model
	    fi
	    
	    spassOutFile="$fd.model"
	    if [[ -f $spassOutFile ]]; then
		time java -cp owl2spass/target/owl2spass-0.1-SNAPSHOT-jar-with-dependencies.jar de.tu_dresden.lat.abduction_via_fol.implicateMatching.ParseAndMatch "$spassOutFile"
		fi
	else
	    echo "$fd is not valid"
	    exit 1
	fi
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done
