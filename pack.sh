#!/bin/bash -e
# File: pack.sh
# Date: Fri Mar 07 10:14:04 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

make clean -C src
sed -i 's/DEBUG/NODEBUG/g' src/Makefile
sed -i 's/^OPTFLAGS += -g/#OPTFLAGS += -g/g; s/^#OPTFLAGS += -O3/OPTFLAGS += -O3/g' src/Makefile
make -C src
make clean -C src
sed -i 's/echo/#echo/g; s/-Wall/#-Wall/g' src/Makefile
tar czvf all.tar.gz \
	run.sh README src main
sed -i 's/#echo/echo/g; s/#-Wall/-Wall/g' src/Makefile
sed -i 's/NODEBUG/DEBUG/g' src/Makefile
