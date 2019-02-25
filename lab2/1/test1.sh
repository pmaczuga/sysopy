#!/bin/bash

for bytes in 81920000 327680000
do
	let lines1=$bytes/8192
	./zad1 generate file.txt $lines1 8192
	
	for size in 4 512 4096 8192
	do
		let lines=$bytes/$size
		./zad1 copy file.txt copy.txt $lines $size sys
		rm copy.txt
		./zad1 copy file.txt copy.txt $lines $size lib
		rm copy.txt
	done
	
	rm file.txt
done
