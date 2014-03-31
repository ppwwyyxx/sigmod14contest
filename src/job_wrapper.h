//File: job_wrapper.h
//Date: Sat Mar 29 01:04:19 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <string>
#include <mutex>
#include "read.h"
#include "lib/Timer.h"
#include "lib/common.h"
#include "lib/finish_time_continuation.h"
#include "globals.h"
#include "query1.h"
#include "query2.h"
#include "query3.h"
#include "query4.h"

extern Query1Handler q1;
extern Query2Handler q2;
extern Query3Handler q3;
extern Query4Handler q4;

extern std::vector<Query1> q1_set;
extern std::vector<Query2> q2_set;
extern std::vector<Query3> q3_set;
extern std::vector<Query4> q4_set;

void add_all_query(int);


inline int do_read_comments(const std::string dir) {
	Timer timer;
	read_comments(dir);
//	if (Data::nperson > 11000) fprintf(stderr, "r cmt: %.4lf\n", timer.get_time());
	return 0;
}
inline int do_read_tags_forums_places(const std::string dir) {
	Timer timer;
	read_tags_forums_places(dir);
//	if (Data::nperson > 11000) fprintf(stderr, "npl:%lu, forut:%.4lf\n", Data::placeid.size(), timer.get_time());
	return 0;
}


#define WAIT_FOR(s) \
	unique_lock<mutex> lk(s ## _mt); \
	while (!s) (s ## _cv).wait(lk); \
	lk.unlock();

inline void start_1(int) {
	PP("start1");
	Timer timer;
//	q1.pre_work();		// sort Data::friends
	add_all_query(1);
	tot_time[1] += timer.get_time();
}

inline void start_2() {
	Timer timer;
	timer.reset();
	q2.work();
	tot_time[2] += timer.get_time();
}

inline void start_3() {
	Timer timer;
	size_t s = q3_set.size();
	REP(i, s) {
		//q3.add_query(q3_set[i].k, q3_set[i].hop, q3_set[i].place, i);
		//print_debug("finish q3 %lu at %lf\n", i, timer.get_time());
		threadpool->enqueue(bind(&Query3Handler::add_query, &q3, q3_set[i].k, q3_set[i].hop, q3_set[i].place, i));
	}
}

inline void start_4(int) {
	PP("start4");
	Timer timer;
	size_t s = q4_set.size();
	q4.continuation = std::make_shared<FinishTimeContinuation>(s, "q4 finish time");
	REP(i, s) {
//		q4.add_query(q4_set[i].k, q4_set[i].tag, i);
		threadpool->enqueue(bind(&Query4Handler::add_query, &q4, q4_set[i].k, q4_set[i].tag, i), 10);
	}
}


void add_all_query(int type);
