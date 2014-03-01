#!/bin/bash -e
# File: run.sh
# Date: Sun Mar 02 01:36:10 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

[[ -z "$1" ]] && (echo "Usage: $0 /path/to/data/directory/ /path/to/query/file" && exit 1)

make -C src
./src/main "$1" "$2"
