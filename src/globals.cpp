//File: globals.cpp
//Date: Wed Apr 09 22:17:53 2014 +0000
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "globals.h"
#include "lib/common.h"

using namespace std;

#define DEFINE_SIGNAL(s) \
	mutex s ## _mt; \
	bool s = false; \
	condition_variable s ## _cv;

// global variables
DEFINE_SIGNAL(tag_read)
DEFINE_SIGNAL(friends_hash_built)
DEFINE_SIGNAL(q2_finished)
#undef DEFINE_SIGNAL

Timer globaltimer;

ThreadPool* threadpool;

vector<double> tot_time(5, 0.0);

unordered_set<string, StringHashFunc> q4_tag_set;
// global variables
