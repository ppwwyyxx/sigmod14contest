#!/bin/bash -e
# File: test.sh
# Date: Tue Apr 08 18:57:08 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

[[ -z "$1" ]] && (echo "Usage: $0 /path/to/data/directory/" && exit 1)

PROG_NAME=`readlink -f "$0"`
PROG_DIR=`dirname "$PROG_NAME"`
cd "$PROG_DIR"

DATA_DIRNAME=`basename $1`
ALL_DIRNAME=`dirname $1`
QUERY=$ALL_DIRNAME/$DATA_DIRNAME-queries.txt
ANS=$ALL_DIRNAME/$DATA_DIRNAME-answers-nocomment.txt

if [[ ! -d "$1" || ! -f $QUERY || ! -f $ANS ]] ; then
	echo "No data/query/ans file found!"
	exit 1
fi

TIME=`date "+%m%d-%H:%M:%S"`
mkdir -p log
OUTPUT=log/ans-"$TIME".txt

make -C src
make mem_monitor -C src
#export LD_PRELOAD=src/third-party/libtcmalloc.so
time ./memusg ./src/main "$1" $QUERY > $OUTPUT
kill -SIGHUP $(pgrep mem_monitor)
diff $OUTPUT $ANS && echo "Accepted" || echo "Wrong Answer"
