#!/bin/bash -e
# File: draw-mem.sh
# Date: Tue Apr 08 19:58:22 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

if [[ -z "$1" ]] ; then
    echo "Usage: $0 log/mem-xxx.xxx"
    exit
fi
cut -f 2 -d ' ' $1 | ./plot-point.py -i '$stdin$' --show
