#!/bin/bash -e
# File: compile.sh
# Date: Wed Apr 16 04:40:25 2014 +0800


sed -i 's/LDFLAGS += -static-libstdc++//g' Makefile
export BUILD=submit
make -j4
