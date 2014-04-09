#!/bin/bash -e
# File: pack.sh
# Date: Wed Apr 09 21:48:56 2014 +0000
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

make clean -C src
make -C src BUILD=submit
mv src/main ./

strings -a main | grep 'GLIBCXX_3.4.15' && exit
strings -a main | grep 'GLIBCXX_3.4.14' && exit
strings -a main | grep 'GLIBCXX_3.4.13' && exit
strings -a main | grep 'GLIBCXX_3.4.12' && exit
strings -a main | grep 'CXXABI_1.3.5' && exit
strings -a main | grep 'GLIBC_2.14' && exit

make clean -C src
sed -i 's/@echo/@#echo/g;' src/Makefile

[[ -f all.tar.gz ]] && rm all.tar.gz
tar czvf all.tar.gz \
	run.sh README src main
mv all.tar.gz /tmp

sed -i 's/#echo/echo/g;' src/Makefile
make -C src
