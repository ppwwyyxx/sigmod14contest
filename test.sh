#!/bin/bash -e
# File: test.sh
# Date: Sun Mar 02 01:40:08 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

[[ -z "$1" ]] && (echo "Usage: $0 <data dir> <1/2/3/4> <cpp file to test>" && exit 1)

ln -sv `readlink -f $3` src/query"$2".cpp
make -C src
./src/main $1 $1-sample-queries"$2".txt
