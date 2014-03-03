#!/bin/bash -e
# File: pack.sh
# Date: Mon Mar 03 16:36:53 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

make -C src
make clean -C src
tar czvf all.tar.gz \
	run.sh README src main
