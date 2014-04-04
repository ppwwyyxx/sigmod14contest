//File: globals.h
//Date: Fri Apr 04 10:32:43 2014 +0000
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <condition_variable>
#include <mutex>
#include "lib/hash_lib.h"
#include "lib/ThreadPool.hh"
#include "lib/Timer.h"
// global variables!!

#define WAIT_FOR(s) \
	unique_lock<mutex> lk(s ## _mt); \
	while (!s) (s ## _cv).wait(lk); \
	lk.unlock();

extern Timer globaltimer;
extern std::condition_variable tag_read_cv;
extern std::mutex tag_read_mt;
extern bool tag_read;

extern bool friends_hash_built;
extern std::mutex friends_hash_built_mt;
extern std::condition_variable friends_hash_built_cv;

extern ThreadPool* threadpool;

extern std::vector<double> tot_time;

extern unordered_set<std::string, StringHashFunc> q4_tag_set;
// end of global variables!!
