#!/bin/bash -e
# File: run.sh
# Date: Thu Feb 27 09:00:26 2014 +0800
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>

[[ -z "$1" ]] && (echo "Usage: $0 /path/to/data/directory/ /path/to/query/file" && exit 1)
