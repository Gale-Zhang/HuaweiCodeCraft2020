#!/bin/bash
rm -rf answer_baseline.txt
g++ baseline.cpp -O3 -lpthread -o baseline
time ./baseline data/10/test_data.txt answer_baseline.txt
