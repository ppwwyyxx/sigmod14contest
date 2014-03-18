//File: job_wrapper.h
//Date: Tue Mar 18 14:32:09 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <string>
#include <mutex>
#include "read.h"
#include "lib/Timer.h"
#include "lib/common.h"
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

inline int do_read_comments(const std::string dir) { read_comments(dir); return 0; }
inline int do_read_tags_forums_places(const std::string dir) { read_tags_forums_places(dir); return 0; }

#define WAIT_FOR(s) \
	unique_lock<mutex> lk(s ## _mt); \
	while (!s) (s ## _cv).wait(lk); \
	lk.unlock();

inline void start_1(int) {
	Timer timer;
	{
		std::lock_guard<std::mutex> lg(q2.mt_work_done);
		std::unique_lock<std::mutex> lgg(mt_friends_data_changing);
		while (friends_data_reader) {
			cv_friends_data_changing.wait(lgg);
		}

		timer.reset();
		q1.pre_work();		// sort Data::friends
	}
	add_all_query(1);
	tot_time[1] += timer.get_time();
}

inline void start_2() {
	//add_all_query(2);
	Timer timer;
	timer.reset();
	q2.work();
	tot_time[2] += timer.get_time();
}

inline void start_3() {
	size_t s = q3_set.size();
	REP(i, s) {
		q3.add_query(q3_set[i].k, q3_set[i].hop, q3_set[i].place, i);
		//threadpool->enqueue(bind(&Query3Handler::add_query, &q3, q3_set[i].k, q3_set[i].hop, q3_set[i].place, i));
	}
}

inline void start_4(int) {
	size_t s = q4_set.size();
	REP(i, s) {
	//	q4.add_query(q4_set[i].k, q4_set[i].tag, i);
		threadpool->enqueue(bind(&Query4Handler::add_query, &q4, q4_set[i].k, q4_set[i].tag, i));
	}
}


void add_all_query(int type) {
	Timer timer;
	switch (type) {
		case 1:
			for (size_t i = 0; i < q1_set.size(); i ++)
				q1.add_query(q1_set[i], i);
			break;
		case 2:
			FOR_ITR(itr, q2_set) {
				q2.add_query(*itr);
			}
			break;
		case 3:
			m_assert(false);
			break;
		case 4:
			m_assert(false);
			break;
	}
}
