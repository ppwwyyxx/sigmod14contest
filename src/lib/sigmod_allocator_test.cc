/*
 * $File: sigmod_allocator_test.cc
 * $Date: Sat Apr 12 20:02:55 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#include "allocator.hh"


#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>

void thread_func() {
	Allocator allocator;
	allocator.enter();

	size_t size = 15lu * 1024 * 1024 ;
	printf("%lu 1: allocating %lu\n", allocator.get_id(), size);

	char *data = (char *)allocator.alloc(size);
	{
		printf("%lu 2: allocating %lu\n", allocator.get_id(), size);
		char *data2 = (char *)allocator.alloc(size);
		printf("%lu 2: deallocating %lu\n", allocator.get_id(), size);
		allocator.dealloc(data2);
	}

	printf("%lu 1: deallocating %lu\n", allocator.get_id(), size);
	allocator.dealloc(data);

	allocator.exit();
}

int main() {
	int nr_threads = 10;
	int nr_threads_avalaible = 4;
	std::vector<std::thread> threads;
	SIGMODAllocator::init(nr_threads, 200000);
	for (int i = 0; i < nr_threads; i ++)
		threads.emplace_back(thread_func);

	printf("joining threads ...\n");
	int cnt = 0;
	for (auto &th: threads) {
		printf("joining threads %d ...\n", cnt ++);
		th.join();
	}
	printf("threads joined ...\n");
	return 0;
}

/**
 * vim: syntax=cpp11 foldmethod=marker
 */

