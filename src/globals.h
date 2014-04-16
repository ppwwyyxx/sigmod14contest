//File: globals.h
//Date: Wed Apr 16 08:26:39 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <condition_variable>
#include <thread>
#include <mutex>
#include "lib/hash_lib.h"
#include "lib/ThreadPool.hh"
#include "lib/Timer.h"
#include "bread.h"
// global variables!!

#define WAIT_FOR(s) \
	std::unique_lock<std::mutex> s ## lk(s ## _mt); \
	while (!s) (s ## _cv).wait(s ## lk); \
	s ## lk.unlock();

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

extern int q1_cmt_vst;

extern unordered_set<std::string, StringHashFunc> q4_tag_set;
extern unordered_map<std::string, std::vector<bool>> q4_persons;
class Q4Scheduler;
extern Q4Scheduler* q4_sched;

extern bread mybread;
// end of global variables!!
