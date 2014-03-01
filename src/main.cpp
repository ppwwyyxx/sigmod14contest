//File: main.cpp
//Date: Sat Mar 01 13:28:54 2014 +0800
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
					// TODO
					break;
				}
			case 4:
				{
					int k;
					fscanf(fin, "%d, %s", &k, buf);
					string tag_name(buf, strlen(buf) - 1);
					// TODO
					break;
				}
			default:
				m_assert(false);
		}
		fgets(buf, 1024, fin);
	}
}

int main(int argc, char* argv[]) {
	Timer timer;
	read_data(string(argv[1]));
	read_query(string(argv[2]));


	Data::free();
}
