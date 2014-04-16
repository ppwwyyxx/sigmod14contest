/*
 * $File: mem_limit_scheduler.cpp
 * $Date: Fri Apr 11 23:41:54 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#include "mem_limit_scheduler.hh"

#include <mutex>
#include <thread>
#include <functional>

using namespace std;

static size_t get_unique_id() {
	static size_t cnt = 0;
	static mutex cnt_mutex;

	size_t ret;
	{
		std::lock_guard<mutex> lock(cnt_mutex);
		ret = cnt ++;
	}
	return ret;
}


void MemAcquirer::aquire(size_t mem_size) {
	MemLimitScheduler::acquire(acquire_mutex, id, mem_size);
}

void MemAcquirer::release(size_t mem_size) {
	MemLimitScheduler::release(id, mem_size);
}


MemAcquirer::MemAcquirer() : id(get_unique_id()) {
//    id = (size_t)this;
}



size_t MemLimitScheduler::nr_threads;
size_t MemLimitScheduler::nr_free_slots;
size_t MemLimitScheduler::mem_size_max;
size_t MemLimitScheduler::free_mem;
size_t MemLimitScheduler::nr_active;

std::set<MemLimitScheduler::Acquisition, MemLimitScheduler::AcquisitionBiggerSizeFirst> MemLimitScheduler::acquisitions_bigger_size_first;
std::map<size_t, size_t> MemLimitScheduler::acquired_mem_by_id;

std::mutex MemLimitScheduler::acquisitions_bigger_size_first_mutex,
	MemLimitScheduler::free_mem_mutex,
	MemLimitScheduler::nr_free_slots_mutex,
	MemLimitScheduler::acquired_mem_by_id_mutex,
	MemLimitScheduler::nr_active_mutex;

/**
 * vim: syntax=cpp11 foldmethod=marker
 */

