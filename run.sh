#!/bin/bash -e
# File: run.sh
# Date: Mon Apr 14 16:19:34 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#[[ -z "$1" ]] && (echo "Usage: $0 /path/to/data/directory/ /path/to/query/file" && exit 1)
#make -C src
#export LD_PRELOAD=src/third-party/libtcmalloc.so
free | tail -n 1 1>&2
./main "$1" "$2" #2>/dev/null
