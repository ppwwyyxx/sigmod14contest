//File: globals.h
//Date: Tue Apr 08 19:38:48 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <condition_variable>
#include <mutex>
#include "lib/hash_lib.h"
#include "lib/ThreadPool.hh"
#include "lib/Timer.h"
// global variables!!

#define WAIT_FOR(s) \
	std::unique_lock<std::mutex> lk(s ## _mt); \
	while (!s) (s ## _cv).wait(lk); \
	lk.unlock();

extern Timer globaltimer;

#define DECLARE_SIGNAL(s) \
	extern std::condition_variable s ## _cv; \
	extern std::mutex s ## _mt; \
	extern bool s;

DECLARE_SIGNAL(tag_read)
DECLARE_SIGNAL(friends_hash_built)
DECLARE_SIGNAL(q2_finished)

#undef DECLARE_SIGNAL

extern ThreadPool* threadpool;

extern std::vector<double> tot_time;

extern unordered_set<std::string, StringHashFunc> q4_tag_set;
// end of global variables!!
