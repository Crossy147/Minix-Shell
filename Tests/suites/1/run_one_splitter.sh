#!/bin/sh

. ./common

if [ $# != 3 ]; then 
	echo Syntax: $0 shell_path test_directory test_number;
	exit 1;
fi

base_dir=$PWD
shell=$1
test_directory=$2
tn=$3

cd $test_directory

if [ $(find /$base_dir/$INPUT_DIR/$tn.in -type f -size +300c 2>/dev/null) ]; then
	delay=0;
else
	delay=1;
fi

echo Delay $delay

echo -n "Splitter test " $tn ": " 
head -1 $base_dir/$INPUT_DIR/$tn.in
echo -n "Command:"
echo $base_dir/$BIN/splitter $delay "<" $base_dir/$INPUT_DIR/$tn.in  "|" $shell ">" $base_dir/$OUTPUT_DIR/$tn.sout "2>"$base_dir/$OUTPUT_DIR/$tn.serr
$base_dir/$BIN/splitter $delay < $base_dir/$INPUT_DIR/$tn.in  | $shell > $base_dir/$OUTPUT_DIR/$tn.sout 2>$base_dir/$OUTPUT_DIR/$tn.serr
diff $base_dir/$OUTPUT_DIR/$tn.sout $base_dir/$EXPECTED_DIR/$tn.out > /dev/null
result1=$?

result2=0
if [ -f $base_dir/$EXPECTED_DIR/$tn.err ]; then
	diff $base_dir/$OUTPUT_DIR/$tn.serr $base_dir/$EXPECTED_DIR/$tn.err > /dev/null
	result2=$?
fi

if [ $result1 -eq 0 ] && [ $result2 -eq 0 ]; 
	then echo OK; 
	else echo FAIL ;
fi	
