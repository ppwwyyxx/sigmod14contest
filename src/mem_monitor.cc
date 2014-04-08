//File: mem_monitor.cc
//Date: Tue Apr 08 19:08:08 2014 +0000
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
#define P(a) std::cout << (a) << std::endl
#define PP(a) std::cout << #a << ": " << (a) << std::endl
#define PA(arr) \
	do { \
		std::cout << #arr << ": "; \
		std::copy(begin(arr), end(arr), std::ostream_iterator<std::remove_reference<decltype(arr)>::type::value_type>(std::cout, " ")); \
		std::cout << std::endl;  \
	} while (0)

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
