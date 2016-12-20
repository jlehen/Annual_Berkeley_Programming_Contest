#!/bin/sh

set -e

make 3
echo

f=`mktemp`
trap "rm $f" EXIT
cat > $f <<EOF
a&&cc&ddd
aaa&b&&dddd
EOF
cat $f
echo

./3 $f
