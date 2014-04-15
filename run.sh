#!/bin/bash -e
# File: run.sh
# Date: Wed Apr 16 04:47:27 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#[[ -z "$1" ]] && (echo "Usage: $0 /path/to/data/directory/ /path/to/query/file" && exit 1)
#make -C src
#export LD_PRELOAD=src/third-party/libtcmalloc.so
./main "$1" "$2" #2>/dev/null
