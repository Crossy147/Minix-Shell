#!/bin/sh

. ./common

if [ $# != 1 ]; then 
			echo Syntax: $0 shell_path;
			exit 1;
		fi

TESTED_SHELL=$(readlink -f $1)

# echo ":" $TESTED_SHELL ":"
if [ -z  $TESTED_SHELL ]; then TESTED_SHELL=$(which $1); fi 
if [ -z  $TESTED_SHELL ]; then 
	echo shell file \" $1 \" not found
	exit 1
fi 

for t in $(ls $SUITES_DIR)
do
	suite_number=$(basename $t)
	./run_suite.sh $TESTED_SHELL $suite_number
	echo 
done
