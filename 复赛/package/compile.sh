#!/bin/bash
set -e

rm -rf answer_cpp.txt
rm -rf 2.txt
g++ main.cpp -O3 -lpthread -fpic -o test
time ./test data/19630345/test_data.txt answer_cpp.txt
