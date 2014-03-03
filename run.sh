#!/bin/bash -e
# File: run.sh
# Date: Mon Mar 03 16:38:16 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

[[ -z "$1" ]] && (echo "Usage: $0 /path/to/data/directory/ /path/to/query/file" && exit 1)

#make -C src
echo $2
./main "$1" "$2"
