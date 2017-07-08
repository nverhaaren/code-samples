#! /bin/bash

# Must be run from ~/houghton

for YEAR in `ls archive`; do
    for MONTH in `ls archive/$YEAR`; do
	if [ -f archive/$YEAR/$MONTH/result.txt ]; then
	    cat archive/$YEAR/$MONTH/result.txt
	    echo
	fi
    done
done

cat result.txt

	
