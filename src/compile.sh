#!/bin/bash -e
# File: compile.sh
# Date: Sat Mar 29 11:30:13 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

sed -i 's/LDFLAGS += -static-libstdc++//g' Makefile
export BUILD=submit
make -j4
