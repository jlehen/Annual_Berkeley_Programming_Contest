#!/bin/sh

set -e

make 4
echo

ulimit -d $((275 * 1024))
set -x
time ./4 1
time ./4 10
time ./4 100
time ./4 1000
time ./4 10000
time ./4 100000
time ./4 250000
