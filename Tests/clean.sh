#!/bin/sh

. ./setup
. ./common

make clean

echo removing test directory $TEST_DIR
remove_test_dir

for s in $(ls $SUITES_DIR/)
do
	SUITE_DIR=$s
	./clean_suite.sh $SUITE_DIR
done

