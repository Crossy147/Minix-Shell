#!/bin/sh

. ./setup
. ./common

if [ -d $TEST_DIR ]; then
	echo ERROR: Test directory already exists. ;
	exit 1;
fi

BASE_DIR=$(pwd)

echo building utils 
make

echo creating test directory $TEST_DIR
create_test_dir

#echo generating outputs
#cd $BASE_DIR
#for g in $(ls $EXPECTED_DIR/*.gen 2> /dev/null)
#do
#	. ./$g
#done 
