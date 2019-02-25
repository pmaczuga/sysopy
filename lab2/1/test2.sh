#!/bin/bash

for lines in 2000 8000
do
	for size in 4 512 4096 8192
	do
		./zad1 generate file.txt $lines $size
		./zad1 copy file.txt 1.txt $lines $size sys
		./zad1 copy file.txt 2.txt $lines $size sys
		
		./zad1 sort 1.txt $lines $size sys
		./zad1 sort 2.txt $lines $size lib
		rm 1.txt
		rm 2.txt
	done
done
