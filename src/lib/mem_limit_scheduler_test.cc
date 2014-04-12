/*
 * $File: mem_limit_scheduler_test.cc
 * $Date: Sat Apr 12 00:12:05 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#include "mem_limit_scheduler.hh"

#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>

void thread_func() {
	MemAcquirer memac;
	size_t id = memac.get_id();
	size_t mem_size = 5000;
	memac.aquire(mem_size);
	{
		printf("%lu: I have %lu bytes of memory\n", id, mem_size);
		char *m0 = new char[mem_size];
		memac.aquire(mem_size);
		{
			printf("%lu: Wow! I have %lu bytes of memory\n", id, mem_size * 2);
			char *m1 = new char[mem_size];
//            std::this_thread::sleep_for(std::chrono::seconds(1));
			delete [] m1;
		}
		memac.release(mem_size);
		delete [] m0;
	}
	memac.release(mem_size);
}

void thread_func_seq() {
	MemAcquirer memac;
	size_t id = memac.get_id();
	size_t mem_size = 10000;
	memac.aquire(mem_size);
	{
		printf("%lu: I have %lu bytes of memory\n", id, mem_size);
		char *m0 = new char[mem_size];
		delete [] m0;
	}
	printf("%lu: exiting ...\n", id);
	memac.release(mem_size);
	printf("%lu: exited.\n", id);
}

#define dprintf(...) \
	do { \
		printf(__VA_ARGS__); \
		fflush(stdout); \
	} while (0)
int main() {

	int nr_threads = 10;
	std::vector<std::thread> threads;
	MemLimitScheduler::init(1, nr_threads * 5000 + 5000);

//    thread_func();
//    return 0;

	dprintf("starting threads ...\n");
	for (int i = 0; i < nr_threads; i ++)
		threads.emplace_back(thread_func);

	dprintf("joining threads ...\n");
	int cnt = 0;
	for (auto &th: threads) {
		dprintf("joining threads %d ...\n", cnt ++);
		th.join();
	}
	dprintf("threads joined ...\n");
	return 0;
}

/**
 * vim: syntax=cpp11 foldmethod=marker
 */

