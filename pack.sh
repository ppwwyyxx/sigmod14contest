#!/bin/bash -e
# File: pack.sh
# Date: Sat Mar 15 01:47:57 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

make clean -C src
make -C src BUILD=submit CC=g++-4.4
mv src/main ./

strings -a main | grep 'GLIBCXX_3.4.15' && exit
strings -a main | grep 'GLIBCXX_3.4.12' && exit
strings -a main | grep 'CXXABI_1.3.5' && exit

make clean -C src
sed -i 's/echo/#echo/g;' src/Makefile

[[ -f all.tar.gz ]] && rm all.tar.gz
tar czvf all.tar.gz \
	run.sh README src main

sed -i 's/#echo/echo/g;' src/Makefile
