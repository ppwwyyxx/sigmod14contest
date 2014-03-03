//File: main.cpp
//Date: Sun Mar 02 23:53:28 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "lib/Timer.h"
#include "lib/debugutils.h"
#include "data.h"
#include "read.h"
#include <cstdio>
#include <string.h>
#include <string>

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
int query_cnt[5];

void read_query(const string& fname) {
	FILE* fin = fopen(fname.c_str(), "r");
	m_assert(fin != NULL);
	int type;
	char buf[1024];
	while (fscanf(fin, "query%d(", &type) == 1) {
		Timer timer;
		switch (type) {
			case 1:
				{
					int p1, p2, x;
					fscanf(fin, "%d, %d, %d)", &p1, &p2, &x);
					q1.add_query(p1, p2, x);
					break;
				}
			case 2:
				{
					int k, y, m, d;
					fscanf(fin, "%d, %d-%d-%d)", &k, &y, &m, &d);
					d = 10000 * y + 100 * m + d;
					q2.add_query(k, d);
					break;
				}
			case 3:
				{
					int k, h;
					fscanf(fin, "%d, %d, %s", &k, &h, buf);
					string place(buf, strlen(buf) - 1);
					q3.add_query(k, h, place);
					break;
				}
			case 4:
				{
					int k;
					fscanf(fin, "%d, %s", &k, buf);
					string tag_name(buf, strlen(buf) - 1);
					q4.add_query(k, tag_name);
					break;
				}
			default:
				m_assert(false);
		}
		tot_time[type] += timer.get_time();
		query_cnt[type] ++;
		fgets(buf, 1024, fin);
	}
}

int main(int argc, char* argv[]) {
	memset(tot_time, 0, 5 * sizeof(double));
	memset(query_cnt, 0, 5 * sizeof(int));
	Timer timer;
	read_data(string(argv[1]));
	print_debug("Read all spent %lf secs\n", timer.get_time());
	print_debug("nperson: %d, ntags: %d\n", Data::nperson, Data::ntag);
	read_query(string(argv[2]));

	timer.reset();
	q1.work();
	tot_time[1] += timer.get_time();
	timer.reset();
	q2.work();
	tot_time[2] += timer.get_time();
	timer.reset();
	q3.work();
	tot_time[3] += timer.get_time();
	timer.reset();
	q4.work();
	tot_time[4] += timer.get_time();

	timer.reset();
//	q1.print_result();
	tot_time[1] += timer.get_time();
	timer.reset();
//	q2.print_result();
	tot_time[2] += timer.get_time();
	timer.reset();
	q3.print_result();
	tot_time[3] += timer.get_time();
	timer.reset();
	q4.print_result();
	tot_time[4] += timer.get_time();

	for (int i = 1; i <= 4; i ++)
		print_debug("%d queries of type %d spent %lf secs in total\n", query_cnt[i], i, tot_time[i]);

	Data::free();
}
