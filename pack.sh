#!/bin/bash -e
# File: pack.sh
# Date: Wed Mar 12 15:45:22 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

make clean -C src
make -C src BUILD=submit
mv src/main ./
make clean -C src
sed -i 's/echo/#echo/g;' src/Makefile

tar czvf all.tar.gz \
	run.sh README src main

sed -i 's/#echo/echo/g;' src/Makefile
