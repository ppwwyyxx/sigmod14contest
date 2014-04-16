/*
 * $File: allocator.cpp
 * $Date: Mon Apr 14 23:14:43 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#include "allocator.hh"
#include <vector>


#define safe_get(___container, key) \
	({ \
	 std::lock_guard<std::mutex> ___guard(___container##_mutex); \
	 auto ___it = ___container.find(key); \
	 if (___it == ___container.end()) { \
		std::cerr << "ERROR fecthing " << #___container << "[" << #key << "]" << std::endl; \
		 std::cerr << "   where " << #key << " is " << key << std::endl; \
	 } \
	 assert(___it != ___container.end()); \
	 ___it->second; \
	 })

SIGMODAllocator *SIGMODAllocator::_allocator;

void SIGMODAllocator::enter(Allocator *allocator, size_t id) {
	do_syscall(allocator, id, 50, true);
}


void SIGMODAllocator::do_syscall(Allocator *allocator, size_t id, size_t size, bool is_enter) throw (AllocFailed) {
	auto &lock = allocator->lock;
	lock.lock();

	std::thread th(std::bind(&SIGMODAllocator::wait_mem, this, allocator, id, size, is_enter));
	th.detach();

	lock.lock();  // capture a thread
	if (allocator->last_result == Allocator::FAILED_TO_ALLOC) {
		throw AllocFailed();
	}

	lock.unlock();
}

void *SIGMODAllocator::alloc(Allocator *allocator, size_t id, size_t size) throw (AllocFailed){
	while (true) {
		// XXX: debug
		size_t fake_free_mem = get_fake_free_mem();
		if (size + reserve_mem <= fake_free_mem) {
			std::lock_guard<std::mutex> lock(alloc_mutex);
			fprintf(stderr, "alloc size: %luM free_mem_size_max: %luM\n", size / 1024 / 1024, fake_free_mem / 1024LU / 1024LU);
			char *data = (char *)_mm_malloc(size, 16);
			if (data) { // succeed
				{
					std::lock_guard<std::mutex> lock(alloc_info_mutex);
					alloc_info[data] = AllocInfo(allocator, id, size);
				}
				{
					std::lock_guard<std::mutex> lock(alloced_size_mutex);
					alloced_size[allocator] += size;
				}

				memset(data, 0, size);
				return data;
			} else {
				PP("unknown error in malloc!");
			}
		}

		fprintf(stderr, "failed to alloc size: %luM free_mem_size_max: %luM\n", size / 1024 / 1024, fake_free_mem / 1024 / 1024);

		nr_free_slots ++;
		do_syscall(allocator, id, size, false);
	}
};

void SIGMODAllocator::dealloc(Allocator *allocator, void *data) {
	_mm_free(data);

	AllocInfo ai;
	{
		std::lock_guard<std::mutex> lock(alloc_info_mutex);
		auto it = alloc_info.find(data);
		assert(it != alloc_info.end());  // XXX
		ai = it->second;
		alloc_info.erase(it);
	}
	{
		std::lock_guard<std::mutex> lock(alloced_size_mutex);
		assert(allocator == ai.allocator);  // XXX

		auto it = alloced_size.find(ai.allocator);
		assert(it != alloced_size.end());
		it->second -= ai.mem;
	}
}

void SIGMODAllocator::exit(Allocator *allocator) {
	{
		std::lock_guard<std::mutex> guard(nr_free_slots_mutex);
		nr_free_slots ++;
	}
	{
		std::lock_guard<std::mutex> guard(alloced_size_mutex);

		auto it = alloced_size.find(allocator);
		assert(it != alloced_size.end());  // XXX
		assert(it->second == 0);
		alloced_size.erase(it);
	}

	std::thread th(std::bind(&SIGMODAllocator::schedule, this));
	th.detach();
}

void SIGMODAllocator::schedule() {
	std::lock_guard<std::mutex> lock(acquisitions_bigger_size_first_mutex);
	std::lock_guard<std::mutex> lock_alloc(alloc_mutex);

	bool new_in_failing = this->in_failing;
	//            size_t free_mem = get_free_mem();
	size_t free_mem = get_fake_free_mem(); // XXX: debug
	std::cerr << "schedule free mem: " << free_mem / 1024 / 1024 << "M" << std::endl;
	for (auto it = acquisitions_bigger_size_first.begin(); it != acquisitions_bigger_size_first.end(); ) {
		{
			std::lock_guard<std::mutex> nr_free_slots_lock(nr_free_slots_mutex);
			if (nr_free_slots == 0)
				break;
		}
		auto it_next = it;
		it_next ++;
		{
			std::lock_guard<std::mutex> guard(in_failing_mutex);
			if (this->in_failing && it->allocator->last_result == Allocator::FAILED_TO_ALLOC)
				continue;
		}

		{
			bool unlock = false;
			if ((it->is_enter && free_mem > 50ul * 1024 * 1024) || (!it->is_enter && free_mem >= it->mem_size + reserve_mem)) {
				unlock = true;
			}
			if (unlock) {
				fprintf(stderr, "unlock: free_mem: %lu reserve_mem: %lu required_mem: %lu\n",
						free_mem, reserve_mem, it->mem_size);
				if (!it->is_enter)
					free_mem -= it->mem_size;

				fprintf(stderr, "start running %lu ...\n", it->id);
				it->allocator->release();  // the thread starts runing
//                {
//                    std::lock_guard<std::mutex> lock(alloced_size_mutex);
//                    alloced_size[it->allocator] += it->mem_size;
//                }

				new_in_failing = false;

				{
					std::lock_guard<std::mutex> nr_free_slots_lock(nr_free_slots_mutex);
					nr_free_slots --;
				}
				acquisitions_bigger_size_first.erase(it);
			}
		}
		it = it_next;
	}

	{
		std::lock_guard<std::mutex> guard(in_failing_mutex);
		this->in_failing = new_in_failing;
	}
	fprintf(stderr, "free_mem: %lu\n", free_mem);
	fprintf(stderr, "nr_free_slots: %lu\n", nr_free_slots);
	fprintf(stderr, "waiting acquisitions: %lu\n", acquisitions_bigger_size_first.size());
	if (!acquisitions_bigger_size_first.empty() && nr_free_slots == nr_threads) {
		fprintf(stderr, "ERROR: unable to schedule\n");
		fail_to_schedule();
	}
}

void SIGMODAllocator::fail_to_schedule() {
	// this is already inside a acquisitions_bigger_size_first_mutex lock

	{
		std::lock_guard<std::mutex> guard(in_failing_mutex);
		this->in_failing = true;
	}

	assert(acquisitions_bigger_size_first.size() >= 2);

	auto &biggest_acq = *acquisitions_bigger_size_first.begin();

	size_t required_size = biggest_acq.mem_size;

	size_t accum_size = get_fake_free_mem();

	std::vector<std::pair<Allocator *, size_t> > acqs;
	// find some small memory consuming thread and fail them in order to
	// leave space for the largest one
	for (auto it = acquisitions_bigger_size_first.rbegin(); it != acquisitions_bigger_size_first.rend(); it ++) {
		if (&(*it) == &biggest_acq)
			break;


		size_t size;
		{
			auto p = alloced_size.find(it->allocator);
			assert(p != alloced_size.end());
			size = p->second;
			accum_size += size;
		}

		acqs.emplace_back(it->allocator, size);
		if (accum_size >= required_size) {  // XXX: >= or >
			break;
		}
	}


	// if it is possible to gather enough memory for the largest
	// acquisition, then fail all these threads
	if (accum_size >= required_size) {
		for (size_t i = 0; i < acqs.size(); i ++)
			acqs[i].first->fail();
		return;
	}


	// when approach here: required_size >= all_alloced_size / 2
	// we should fail the biggest one instead
	biggest_acq.allocator->fail();
}

/**
 * vim: syntax=cpp11 foldmethod=marker
 */

