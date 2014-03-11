#!/bin/bash -e
# File: pack.sh
# Date: Tue Mar 11 11:50:07 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

make clean -C src
make -C src BUILD=submit
make clean -C src
tar czvf all.tar.gz \
	run.sh README src main
