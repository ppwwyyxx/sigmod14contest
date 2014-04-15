//File: main.cpp
//Date: Wed Apr 16 05:13:03 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <cstdio>
#include <string.h>
#include <string>
#include <omp.h>
#include <thread>

#include "lib/hash_lib.h"
#include "lib/allocator.hh"
#include "lib/Timer.h"
#include "lib/debugutils.h"
#include "lib/common.h"
#include "data.h"
#include "job_wrapper.h"
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
					q1_set.emplace_back(p1, p2, x);
					break;
				}
			case 2:
				{
					int k, y, m, d;
					fscanf(fin, "%d, %d-%d-%d)", &k, &y, &m, &d);
					d = 10000 * y + 100 * m + d;
					q2_set.emplace_back(k, d, (int)q2_set.size());
					break;
				}
			case 3:
				{
					int k, h;
					fscanf(fin, "%d, %d, %s", &k, &h, buf);
					string place(buf, strlen(buf) - 1);
					q3_set.emplace_back(k, h, place);
					break;
				}
			case 4:
				{
					int k;
					fscanf(fin, "%d, %s", &k, buf);
					string tag_name(buf, strlen(buf) - 1);
					q4_set.emplace_back(k, tag_name);
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


#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char* argv[]) {
	globaltimer.reset();
	threadpool = new ThreadPool(NUM_THREADS);
	Timer timer;
	// initialize global variables...
#ifdef GOOGLE_HASH
	q4_tag_set.set_empty_key("");
	q4_persons.set_empty_key("");
	Data::placeid.set_empty_key("");
#endif
	// end
	string dir(argv[1]);

	read_query(string(argv[2]));		// read query first, so we can read data optionally later
	q4.ans.resize(q4_set.size());
	q3.global_answer.resize(q3_set.size());

	threadpool->enqueue(bind(do_read_comments, dir), start_1, 20);
	read_data(dir);
	print_debug("Read return at %lf secs\n", timer.get_time());
	/*
	 *if (Data::nperson > 10000) {
	 *    fprintf(stderr, "th:%dmem:%d\n", thread::hardware_concurrency(), get_free_mem());
	 *}
	 */
	/*
	 *if (Data::nperson > 11000) {
	 *    PP(edge_count(Data::friends));
	 *}
	 */

	threadpool->enqueue(bind(do_read_tags_forums_places, dir), start_4);
	WAIT_FOR(tag_read);
	threadpool->enqueue(start_2);
	start_3();
	/*
	 *WAIT_FOR(comment_read);
	 *threadpool->add_worker(4);
	 */
	PP("deleting...");
	threadpool->condition.notify_all();
	delete threadpool;		// will wait to join all thread

	q1.print_result();
	q2.print_result();
	q3.print_result();
	q4.print_result();

	//fprintf(stderr, "%lu\t%lu\t%lu\t%lu\n", q1_set.size(), q2_set.size(), q3_set.size(), q4_set.size());
	tot_time[3] += TotalTimer::rst["Q3"];
	tot_time[4] += TotalTimer::rst["Q4"];
	if (Data::nperson > 10000) {
		for (int i = 1; i <= 4; i ++)
			fprintf(stderr, "q%d:%.4fs ", i, tot_time[i]);
		fprintf(stderr, "%.4lf ", TotalTimer::rst["depth 2"]);
		fprintf(stderr, "%.4lf ", TotalTimer::rst["depth 3"]);
		fprintf(stderr, "%.4lf ", TotalTimer::rst["estimate random"]);
	}
	//fprintf(stderr, "\nTime: %.4fs\n", timer.get_time());
	Data::free();
	TotalTimer::print();
	ManualTotalTimer::print();
}
