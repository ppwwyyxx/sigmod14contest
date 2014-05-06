//File: job_wrapper.h
//Date: Tue May 06 11:24:37 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#ifndef __HEADER_JOBWRAPPER
#define __HEADER_JOBWRAPPER



#include <string>
#include <mutex>
#include "read.h"
#include "lib/Timer.h"
#include "lib/common.h"
#include "lib/finish_time_continuation.h"
#include "q4Scheduler.h"
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



inline int do_read_comments(const std::string dir) {
	Timer timer;
	read_comments_tim(dir);
	if (Data::nperson > 11000)
		fprintf(stderr, "r cmt: %.4lf\n", timer.get_time());
	fflush(stderr);
	return 0;
}
inline int do_read_tags_forums_places(const std::string dir) {
	Timer timer;
	read_tags_forums_places(dir);
//	if (Data::nperson > 11000)
//		fprintf(stderr, "npl:%lu, forut:%.4lf\n", Data::placeid.size(), timer.get_time());
	return 0;
}


inline void start_1(int) {
	PP("start1");
	Timer timer;
//	q1.pre_work();		// sort Data::frien
	q1.continuation = std::make_shared<FinishTimeContinuation>(q1_set.size(), "q1 finish time");
	REP(i, q1_set.size())
		q1.add_query(q1_set[i], (int)i);
	tot_time[1] += timer.get_time();
	PP(q1_cmt_vst);
}

inline void start_2() {
	Timer timer;
	timer.reset();
	q2.continuation = std::make_shared<FinishTimeContinuation>(1, "q2 finish time");
	q2.work();
	tot_time[2] += timer.get_time();
}

inline void start_3() {
	Timer timer;
	size_t s = q3_set.size();
	q3.continuation = std::make_shared<FinishTimeContinuation>(s, "q3 finish time");
	REP(i, s) {
		//q3.add_query(q3_set[i].k, q3_set[i].hop, q3_set[i].place, i);
		//print_debug("finish q3 %lu at %lf\n", i, timer.get_time());
		threadpool->enqueue(bind(&Query3Handler::add_query, &q3, q3_set[i].k, q3_set[i].hop, q3_set[i].place, i));
	}
	q3_set = std::vector<Query3>();
}

inline void start_4(int) {
	fprintf(stderr, "start4\n");
	//std::this_thread::sleep_for(std::chrono::seconds(7));
	Timer timer;
	size_t s = q4_set.size();
	q4.continuation = std::make_shared<FinishTimeContinuation>(s, "q4 finish time");
	/*
	 *if (Data::nperson > 300000) {
	 *        q4_sched = new Q4Scheduler(4);
	 *        q4_sched->work();
	 *} else {
	 */
			REP(i, s) {
					threadpool->enqueue(bind(&Query4Handler::add_query,
											&q4, q4_set[i].k, q4_set[i].tag, i), 10);
			}
	/*
	 *}
	 */
}

// call after read forum
void destroy_tag_name() {
		WAIT_FOR(q2_finished);
		FreeAll(Data::tag_name);
}

// call after all q3 finished
void destroy_q3_data() {
		Data::placeid.clear();
		Data::placeid = unordered_map<std::string, std::vector<int>, StringHashFunc>();
		FreeAll(Data::places);
		WAIT_FOR(q2_finished);
		FreeAll(Data::tags);
}

#else
#error "This header should only be included once"
#endif
