#! /bin/bash

YEAR=`date +%Y`
MONTH=`date +%b`
MONTH_NUM=`date +%m`

if [ ${MONTH_NUM:0:1} == "0" ]; then
    MONTH_NUM=${MONTH_NUM:1:1}
fi

PMONTH_NUM=$(( $MONTH_NUM - 1 ))
if [ $PMONTH_NUM -eq 0 ]; then
    PMONTH_NUM=12
    PYEAR=$[ $YEAR - 1 ]
elif [ $PMONTH_NUM -lt 10 ]; then
    PMONTH_NUM=0$PMONTH_NUM
    PYEAR=$YEAR
else
    PYEAR=$YEAR
fi

MAIN=/home/nverhaaren/houghton
ARCHIVE=$MAIN/archive/$PYEAR/$PMONTH_NUM

sed -n "1,/$MONTH  2.*$YEAR/p" $MAIN/result.txt | head -n -2 > $ARCHIVE/result.txt
if [ -f $ARCHIVE/result.txt ]; then
    TOTAL=$(( `cat $ARCHIVE/result.txt | wc -l` + `sed -n "/$MONTH  2.*$YEAR/,$ p" $MAIN/result.txt | wc -l` ))
    TOTAL=$(( $TOTAL + 1 ))
    if [ $TOTAL -eq `cat $MAIN/result.txt | wc -l` ]; then
	sed -n -i "/$MONTH  2.*$YEAR/,$ p" $MAIN/result.txt
    else
	echo "archive_results.sh: Not all results archived successfully" >&2
    fi
else
    echo "archive_results.sh: Archive creation failed" >&2
fi


