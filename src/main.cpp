//File: main.cpp
//Date: Sun Mar 16 23:53:13 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <cstdio>
#include <string.h>
#include <string>
#include <thread>

#include "lib/Timer.h"
#include "lib/debugutils.h"
#include "lib/common.h"
#include "data.h"
#include "cache.h"
#include "read.h"

#include "query1.h"
#include "query2.h"
#include "query3.h"
#include "query4.h"
using namespace std;

Query1Handler q1;
Query2Handler q2;
Query3Handler q3;
Query4Handler q4;

double tot_time[5];
vector<Query1> q1_set;
vector<Query2> q2_set;
vector<Query3> q3_set;
vector<Query4> q4_set;

void read_query(const string& fname) {
	FILE* fin = fopen(fname.c_str(), "r");
	m_assert(fin != NULL);
	int type;
	char buf[1024];
	while (fscanf(fin, "query%d(", &type) == 1) {
		switch (type) {
			case 1:
				{
					int p1, p2, x;
					fscanf(fin, "%d, %d, %d)", &p1, &p2, &x);
					q1_set.push_back(Query1(p1, p2, x));
					break;
				}
			case 2:
				{
					int k, y, m, d;
					fscanf(fin, "%d, %d-%d-%d)", &k, &y, &m, &d);
					d = 10000 * y + 100 * m + d;
					q2_set.push_back(Query2(k, d, (int)q2_set.size()));
					break;
				}
			case 3:
				{
					int k, h;
					fscanf(fin, "%d, %d, %s", &k, &h, buf);
					string place(buf, strlen(buf) - 1);
					q3_set.push_back(Query3(k, h, place));
					break;
				}
			case 4:
				{
					int k;
					fscanf(fin, "%d, %s", &k, buf);
					string tag_name(buf, strlen(buf) - 1);
					q4_set.push_back(Query4(k, tag_name));
					q4_tag_set.insert(tag_name);
					break;
				}
			default:
				m_assert(false);
		}
		fgets(buf, 1024, fin);
	}
	fclose(fin);
}

void add_all_query(int type) {
	Timer timer;
	switch (type) {
		case 1:
#pragma omp parallel for num_threads(NUM_THREADS)
			for (size_t i = 0; i < q1_set.size(); i ++)
				q1.add_query(q1_set[i], i);
			break;
		case 2:
			FOR_ITR(itr, q2_set) {
				q2.add_query(*itr);
			}
			break;
		case 3:
			FOR_ITR(itr, q3_set) {
				q3.add_query(itr->k, itr->hop, itr->place);
			}
			break;
		case 4:
			FOR_ITR(itr, q4_set) {
				q4.add_query(itr->k, itr->tag);
			}
			break;
	}
}

#define WAIT_FOR(s) \
	unique_lock<mutex> lk(s ## _mt); \
	while (!s) (s ## _cv).wait(lk); \
	lk.unlock();

inline void start_4() {
	WAIT_FOR(forum_read);

	Timer timer;
	timer.reset();
	add_all_query(4);
	tot_time[4] += timer.get_time();
}

inline void start_1() {
	Timer timer;
	WAIT_FOR(comment_read);
	{
		std::lock_guard<mutex> lgg(q2.mt_work_done);
		std::lock_guard<mutex> lg(q3.mt_work_done);
		timer.reset();
		q1.pre_work();		// sort Data::friends
	}
	add_all_query(1);
	tot_time[1] += timer.get_time();
}

inline void start_2() {
	//add_all_query(2);
	Timer timer;
	WAIT_FOR(tag_read);

	{
		std::lock_guard<mutex> lk(q2.mt_work_done);
		timer.reset();
		q2.work();
		tot_time[2] += timer.get_time();
	}
}

inline void start_3() {
	WAIT_FOR(tag_read);
	{
		std::lock_guard<mutex> lk(q3.mt_work_done);
		Timer timer;
		timer.reset();
		add_all_query(3);
		tot_time[3] += timer.get_time();
	}
}


#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char* argv[]) {
	// initialize global variables...

#ifdef GOOGLE_HASH
	q4_tag_set.set_empty_key("");
#endif
	// end
	read_query(string(argv[2]));		// read query first, so we can read data optionally later

	memset(tot_time, 0, 5 * sizeof(double));
	Timer timer;
	read_data(string(argv[1]));
	print_debug("Read return at %lf secs\n", timer.get_time());

#ifdef USE_THREAD
	thread th_q1(start_1);
	thread th_q4(start_4);
	thread th_q2(start_2);
	thread th_q3(start_3);
	th_q1.join();
	th_q2.join();
	th_q3.join();
	th_q4.join();
#else
	start_1();
	start_2();
	start_3();
	start_4();
#endif

	q1.print_result();
	q2.print_result();
	q3.print_result();
	q4.print_result();

	print_debug("nperson: %d, ntags: %d\n", Data::nperson, Data::ntag);
	fprintf(stderr, "%lu\t%lu\t%lu\t%lu\n", q1_set.size(), q2_set.size(), q3_set.size(), q4_set.size());
	for (int i = 1; i <= 4; i ++)
		fprintf(stderr, "q%d: %.4fs\t", i, tot_time[i]);
	Data::free();
	fprintf(stderr, "\nTime: %.4fs\n", timer.get_time());
	TotalTimer::print();
}
