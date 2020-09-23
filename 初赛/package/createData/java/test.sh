#!/bin/bash
set -e
for var in {1..5000}
do
	echo $var
	java GenerateTestData
	time ./four test_data.txt
	time ./test2 test_data.txt answer_cpp_new2.txt
	java CheckSame answer_cpp.txt answer_cpp_new2.txt
	echo OK
	rm -rf answer_cpp.txt
	rm -rf answer_cpp_new2.txt
	echo
done
