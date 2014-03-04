#!/bin/bash -e
# File: run.sh
# Date: Mon Mar 03 23:19:46 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#[[ -z "$1" ]] && (echo "Usage: $0 /path/to/data/directory/ /path/to/query/file" && exit 1)
#make -C src
time ./main "$1" "$2"
