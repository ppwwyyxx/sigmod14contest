#!/bin/bash -e
# File: memcheck.sh
# Date: Wed Mar 12 15:52:04 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

[[ -z "$1" ]] && (echo "Usage: $0 /path/to/data/directory/" && exit 1)
DATA_DIRNAME=`basename $1`
ALL_DIRNAME=`dirname $1`
QUERY=$ALL_DIRNAME/$DATA_DIRNAME-queries.txt

if [[ ! -d "$1" || ! -f $QUERY ]] ; then
	echo "No data/query file found!"
	exit 1
fi

TIME=`date "+%m%d-%H:%M:%S"`

mkdir -p log
VOUTPUT=log/valg-"$TIME".valgrind
HOUTPUT=log/heapcheck-"$TIME"

sed -i 's/^OPTFLAGS += -O3/#OPTFLAGS += -O3/g; s/^#OPTFLAGS += -g/OPTFLAGS += -g/g' src/Makefile
#make -B -C src

export PPROF_PATH=/usr/bin/pprof
export LD_PRELOAD=src/third-party/libtcmalloc.so.4.1.2
export HEAPCHECK=normal
./src/main "$1" "$QUERY" >/dev/null 2>$HOUTPUT

#echo "=====WARNING: valgrind memcheck might take extreamly long time to run...====="
#export HEAPCHECK=
#valgrind --leak-check=full --track-origins=yes --show-possibly-lost=yes ./main "$1" "$QUERY" > /dev/null

#rm /tmp/main.*.heap
