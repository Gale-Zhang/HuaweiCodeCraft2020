#!/bin/bash
for var in {1,2,3,4,5,6,7,8,9,10}
do
	echo $var
	rm -rf 1.txt
	time ./baseline data/$var/test_data.txt 1.txt
	rm -rf 2.txt
	time ./test2 data/$var/test_data.txt 2.txt
	java CheckSame 1.txt 2.txt
	echo OK
	echo
done
