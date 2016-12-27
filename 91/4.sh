#!/bin/sh

E=4

set -e

make $E
echo

ulimit -d 275
set -x
time ./$E 1
time ./$E 10
time ./$E 100
time ./$E 1000
time ./$E 10000
time ./$E 100000
time ./$E 250000
