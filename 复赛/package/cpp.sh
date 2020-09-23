#!/bin/bash
set -e

rm -rf answer_cpp.txt
g++ main.cpp -O3 -lpthread -fpic -o test
time ./test data/19630345/test_data.txt answer_cpp.txt
rm -rf baseline.txt
time ./baseline data/19630345/test_data.txt baseline.txt
java CheckSame baseline.txt answer_cpp.txt
