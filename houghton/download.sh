#! /bin/bash

OUT=/home/nverhaaren/houghton/result.txt
CMD=/home/nverhaaren/houghton/ua-lookup.py

date >> $OUT
echo >> $OUT
echo 5061 >> $OUT
$CMD 5061 >> $OUT
echo >> $OUT
echo 5141 >> $OUT
$CMD 5141 >> $OUT
echo >> $OUT
echo 5001 >> $OUT
$CMD 5001 >> $OUT
echo >> $OUT
echo 5109 >> $OUT
$CMD 5109 >> $OUT
echo >> $OUT
