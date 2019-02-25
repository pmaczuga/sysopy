#!/bin/bash

results="results.txt"
filter="filter.txt"
output="out.pgm"

printf "Results of time measurment: \n\n" > $results

for input in "1.pgm" "2.pgm" "3.pgm" "4.pgm" "5.pgm"; do
	for filter_size in 4 8 16 32 64; do
		./filterCreator $filter_size $filter
		for threads in 1 2 4 8; do
			./main $threads $input $filter $output >> $results
		done
	done
done
rm $filter
rm $output

