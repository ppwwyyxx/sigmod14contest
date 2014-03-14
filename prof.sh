#!/bin/bash -e
# File: prof.sh
# Date: Wed Mar 12 15:51:56 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

[[ -z "$1" ]] && (echo "Usage: $0 /path/to/data/directory/" && exit 1)
DATA_DIRNAME=`basename $1`
ALL_DIRNAME=`dirname $1`
QUERY=$ALL_DIRNAME/$DATA_DIRNAME-queries.txt

if [[ ! -d "$1" || ! -f $QUERY ]] ; then
	echo "No data/query file found!"
	exit 1
fi

for PPROF in pprof google-pprof; do
	which $PPROF > /dev/null && break
done

TIME=`date "+%m%d-%H:%M:%S"`

mkdir -p log
export CPUPROFILE=log/prof-$TIME
#export PROFILEFREQUENCY=10000

# see http://google-perftools.googlecode.com/svn/trunk/doc/cpuprofile.html for more options

make -C src
./src/main "$1" "$QUERY" > /dev/null

OUTPUT="$CPUPROFILE".png
$PPROF --dot ./src/main $CPUPROFILE | dot -Tpng -o$OUTPUT
[[ -x `which feh` ]] && feh $OUTPUT
