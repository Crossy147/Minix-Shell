#!/bin/sh

. ./setup
. ./common

if [ $# != 2 ]; then 
		echo Syntax: $0 shell_path suite_number;
			exit 1;
		fi

TESTED_SHELL=$(readlink -f $1)
SUITE_NUMBER=$2
SUITE_DIR=$SUITES_DIR/$SUITE_NUMBER

if [ -z  $TESTED_SHELL ]; then TESTED_SHELL=$(which $1); fi 
if [ -z  $TESTED_SHELL ]; then 
	echo shell file \" $1 \" not found
	exit 1
fi 

echo "--- Suite" $SUITE_NUMBER "---"

for t in $(ls $SUITE_DIR/$INPUT_DIR/*.in)
do
	filename=$(basename $t)
	./run_one.sh $TESTED_SHELL $SUITE_NUMBER ${filename%.in}
done

