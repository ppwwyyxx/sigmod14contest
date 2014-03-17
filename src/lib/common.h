//File: common.h
//Date: Mon Mar 17 21:18:25 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#define REP(x, y) for (auto x = decltype(y){0}; x < (y); x ++)
#define REPL(x, y, z) for (auto x = decltype(z){y}; x < (z); x ++)
#define REPD(x, y, z) for (auto x = decltype(z){y}; x >= (z); x --)
#define FOR_ITR(x, y) for (auto x = (y).begin(); x != (y).end(); x ++)

#define NUM_THREADS 8
