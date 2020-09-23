#!/bin/bash
rm -rf 2.txt
g++ main.cpp -O3 -lpthread -o test2
time ./test2 data/10/test_data.txt 2.txt
