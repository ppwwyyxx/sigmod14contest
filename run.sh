#!/bin/bash -e
# File: run.sh
# Date: Thu Mar 27 21:54:14 2014 +0000
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#[[ -z "$1" ]] && (echo "Usage: $0 /path/to/data/directory/ /path/to/query/file" && exit 1)
#make -C src
#export LD_PRELOAD=src/third-party/libtcmalloc.so
./main "$1" "$2" #2>/dev/null
