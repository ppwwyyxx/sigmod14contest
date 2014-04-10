#!/bin/bash -e
# File: compile.sh
# Date: Thu Apr 10 14:58:15 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

make -C third-party/lockless_allocator/
sed -i 's/LDFLAGS += -static-libstdc++//g' Makefile
export BUILD=submit
make -j4
