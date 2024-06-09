#! /bin/bash

folder=$1
ontologies_file=$2



echo Folder: $folder
echo Ontologies file: $ontologies_file

total=0
total2=0
problems=0
hypotheses=0
complete=0

rm $folder/spass-times.txt
rm $folder/translation-times.txt
rm $folder/match-times.txt

for i in `cat $ontologies_file`; do

    total=$((total + `ls $folder/$i*abduce-output|wc -l`)) 
    total2=$((total2 + `ls $folder/$i*translate-output|wc -l`)) 

    problems=$((problems + `ls $folder/$i*dfg|wc -l`)) 
    hypotheses=$((hypotheses + `ls $folder/$i*model|wc -l`)) 

    for j in `seq 5`; do
#	echo $folder.$i.$j
	
	if [ ! -f $folder/$i.$j.dfg ]; then
	    if ! grep -q "Could not find any non-trivially entailed CI" $folder/$i.$j.translate-output; then
		if ! grep -q "No non-entailed observations can be found." $folder/$i.$j.translate-output; then 
		    echo no dfg file: $i.$j
		fi
	    fi
	else
	    grep real $folder/$i.$j.translate-output|grep "[0-9][0-9][0-9]s"|tail -n 1 >> $folder/translation-times.txt ;
	    grep real $folder/$i.$j.abduce-output|grep "[0-9][0-9][0-9]s"|head -n 1 >> $folder/spass-times.txt
	    grep real $folder/$i.$j.abduce-output|grep "[0-9][0-9][0-9]s"|tail -n 1 >> $folder/match-times.txt

	    if  grep -q "number of solutions" $folder/$i.$j.abduce-output ; then
		timeString1=`grep real $folder/$i.$j.abduce-output|grep "[0-9][0-9][0-9]s"|head -n 1`
		timeString2=`grep real $folder/$i.$j.abduce-output|grep "[0-9][0-9][0-9]s"|tail -n 1`

		time1=`echo $timeString1 | awk -Fm '{print $1*60+$2}'`
		time2=`echo $timeString2 | awk -Fm '{print $1*60+$2}'`

		timeSum=`echo "$time1 + $time2"|bc`

		echo $timeSum >> $folder/abduction-times.txt
	    fi
		
#	    if grep real $folder/$i.$j.abduce-output|grep "[0-9][0-9][0-9]s"|head -n 1 | grep -q " 0m\([12][0-9]\|[0-9][^0-9]\)"; then
#		complete=$((complete + 1))
#	    fi

	fi
	
	if [ -f $folder/$i.$j.dfg -a ! -f $folder/$i.$j.dfg.model ]; then
	    if ! grep -q "derived 0 clauses" $folder/$i.$j.abduce-output; then
		echo no model, though not 0 clauses derived: $i.$j
	    else
		grep real $folder/$i.$j.abduce-output|grep "[0-9][0-9][0-9]s"|head -n 1 >> $folder/spass-times.txt
	    fi
	fi
		
    done

done

#complete=`grep "0m\([12][0-9]\|[0-9][^0-9]\)" $folder/spass-times.txt|wc -l`

#incomplete=`grep "[3-9][0-9].[0-9][0-9] on the problem" -m1 $folder/*abduce-output|wc -l`
incomplete=`grep "SPASS beiseite: Ran out of time" -m1 $folder/*abduce-output|wc -l`

echo incomplete: $incomplete

complete=$((problems - incomplete))

grep "number of solutions" $folder/*abduce-output|cut -d: -f3 > $folder/number-of-solutions.txt
grep "size of largest solution" $folder/*abduce-output|cut -d'[' -f2|cut -d']' -f1 > $folder/solutions-sizes.txt
grep "size of largest axiom" $folder/*abduce-output|cut -d'[' -f2|cut -d']' -f1 > $folder/axiom-sizes.txt

#done

# total=`ls $folder/*abduce-output|wc -l`
# total2=`ls $folder/*translate-output|wc -l`

# problems=`ls $folder/*dfg|wc -l`
# hypotheses=`ls $folder/*model|wc -l`

echo scripts started: $total $total2
echo problems generated: $problems/$total  = `echo "scale=1; 100*$problems/$total" | bc`%
echo hypotheses found: $hypotheses/$problems = `echo "scale=1; 100*$hypotheses/$problems" | bc`%
echo all hypotheses found: $complete/$problems = `echo "scale=1; 100*$complete/$problems" | bc`%
#echo all hypotheses found: $complete2/$problems = `echo "scale=1; 100*$complete2/$problems" | bc`%

echo
echo All ontologies:

total=`ls $folder/*abduce-output|wc -l`
total2=`ls $folder/*translate-output|wc -l`

problems=`ls $folder/*dfg|wc -l`
hypotheses=`ls $folder/*model|wc -l`

echo scripts started: $total $total2
echo problems generated: $problems/$total  = `echo "scale=1; 100*$problems/$total" | bc`%
echo hypotheses found: $hypotheses/$problems = `echo "scale=1; 100*$hypotheses/$problems" | bc`%
