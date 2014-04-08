#!/bin/bash -e
# File: draw-mem.sh
# Date: Tue Apr 08 19:02:57 2014 +0000
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

cut -f 2 -d ' ' $1 >/tmp/xxx && ./plot-point.py -i /tmp/xxx --show
