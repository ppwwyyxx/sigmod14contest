//File: globals.cpp
//Date: Wed Apr 16 08:26:48 2014 +0800
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

int q1_cmt_vst = 0;

unordered_set<string, StringHashFunc> q4_tag_set;
unordered_map<string, vector<bool>> q4_persons;
vector<thread> q4_jobs;
Q4Scheduler* q4_sched;

bread mybread;
// global variables
