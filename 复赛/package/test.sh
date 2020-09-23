#!/bin/bash
for var in {43,k16,k18,3512444,1004812,2755223,2861665,38252,19630345}
do
	echo $var
	rm -rf 1.txt
	time ./baseline data/$var/test_data.txt 1.txt
	rm -rf 2.txt
	time ./test data/$var/test_data.txt 2.txt
	java CheckSame 1.txt 2.txt
	echo OK
	echo
done
