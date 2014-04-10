//File: mem_monitor.cc
//Date: Thu Apr 10 11:06:08 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <limits>
#include <thread>
#include <signal.h>
#include <unistd.h>
#include "lib/utils.h"
#include "lib/Timer.h"
using namespace std;
#define REP(x, y) for (auto x = decltype(y){0}; x < (y); x ++)
#define REPL(x, y, z) for (auto x = decltype(z){y}; x < (z); x ++)
#define REPD(x, y, z) for (auto x = decltype(z){y}; x >= (z); x --)

sig_atomic_t signaled = 0;

void my_handler(int param) {
	signaled = 1;
}

size_t get_total_mem() {
	long pages = sysconf(_SC_PHYS_PAGES);
	long page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
}

int main(int argc, char* argv[]) {
	auto total = get_total_mem() / 1024 / 1024;
	void (*handler)(int);
	handler = signal(SIGHUP, my_handler);

	Timer timer;
	int interval = atof(argv[1]);
	string ofname(argv[2]);
	ofstream fout(ofname);
	while (true && not signaled) {
		this_thread::sleep_for(chrono::seconds(1));
		int fr = get_free_mem();
		fout << timer.get_time() << " " << total - fr << endl;
	}
	fout.close();
}
