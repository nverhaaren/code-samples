#! /bin/bash

YEAR=`date +%Y`
MONTH=`date +%m`
MAIN=/home/nverhaaren/houghton

if ! [ -d "$MAIN/archive/$YEAR" ]; then
    mkdir $MAIN/archive/$YEAR
fi
if ! [ -d "$MAIN/archive/$YEAR/$MONTH" ]; then
    mkdir $MAIN/archive/$YEAR/$MONTH
fi

for FILE in `find $MAIN -regex "$MAIN/[^/]*[.]html$" -print`; do
    FILE_LEN=${#FILE}
    FILE_YEAR=${FILE:$FILE_LEN-14:4}
    FILE_MONTH=${FILE:$FILE_LEN-17:2}
    mv $FILE $MAIN/archive/$FILE_YEAR/$FILE_MONTH
done

if [ "`cat $MAIN/errors`" ]; then
    date >> $MAIN/archive/$YEAR/$MONTH/errors
    cat $MAIN/errors >> $MAIN/archive/$YEAR/$MONTH/errors
    echo "Found errors in error file:" >&2
    date >&2
    cat $MAIN/errors >&2
    rm $MAIN/errors
fi
