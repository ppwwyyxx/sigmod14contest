/*
 * $File: allocator.hh
 * $Date: Mon Apr 14 23:07:24 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#pragma once

/*
 * Scheduling when memory size is limited.
 *
 */
#include <emmintrin.h>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <mutex>
#include <thread>
#include <cstdlib>
#include <limits>
#include <condition_variable>
#include <map>
#include <string>
#include <fstream>
#include <memory>
#include <set>
#include <iostream>

#include "debugutils.h"

#define assert_equal(a, b) \
	do { \
		auto __a = (a); \
		auto __b = (b); \
		if (__a != __b) { \
			std::cerr << "assert equal failed." << std::endl; \
			std::cerr << __FILE__ << ":" << __func__ << ":" << __LINE__ << std::endl; \
			std::cerr << #a << ":" << __a << std::endl; \
			std::cerr << #b << ":" << __b << std::endl; \
		} \
	} while (0)

#define assert_not_equal(a, b) \
	do { \
		auto __a = (a); \
		auto __b = (b); \
		if (__a == __b) { \
			std::cerr << "assert not equal failed." << std::endl; \
			std::cerr << __FILE__ << ":" << __func__ << ":" << __LINE__ << std::endl; \
			std::cerr << #a << ":" << __a << std::endl; \
			std::cerr << #b << ":" << __b << std::endl; \
		} \
	} while (0)

class Allocator;

class AllocFailed : public std::exception {
};

class SIGMODAllocator {
	public:
		void enter(Allocator *allocator, size_t id);

		void exit(Allocator *allocator);

		size_t get_fake_free_mem() {
			size_t mem_total = get_mem_total();
			size_t fake_mem_total = 15lu * 1024 * 1024 * 1024;
			assert(mem_total > get_free_mem()); //XXX
			size_t used = mem_total - get_free_mem();
			if (fake_mem_total < used)
				return 0;
			size_t fake_free_mem = fake_mem_total - used;
			return fake_free_mem; //XXX
		}
		void *alloc(Allocator *allocator, size_t id, size_t size) throw (AllocFailed);

		void dealloc(Allocator *allocator, void *data);

		static void init(int nr_threads, size_t reserve_mem) {
			assert(!_allocator); // XXX
			_allocator = new SIGMODAllocator(nr_threads, reserve_mem);
		}

		static SIGMODAllocator *get_instance() {
			return _allocator;
		};

	protected:
		static SIGMODAllocator *_allocator;

		SIGMODAllocator(int nr_threads, size_t reserve_mem) {
			this->nr_threads = nr_threads;
			nr_free_slots = nr_threads;
			this->reserve_mem = reserve_mem;
			this->in_failing = false;
		}

		struct Acquisition {
			Acquisition(Allocator *allocator, size_t id, size_t mem_size, bool is_enter) :
				allocator(allocator), id(id), mem_size(mem_size), is_enter(is_enter) {
				}
			Allocator *allocator;
			size_t id, mem_size;
			bool is_enter;
		};

		struct AcquisitionBiggerSizeFirst {
			bool operator () (const Acquisition &a, const Acquisition &b) const {  // bigger size first
				if (a.mem_size != b.mem_size)
					return a.mem_size > b.mem_size;
				return a.id < b.id;
			}
		};

		struct AcquisitionCmpByID {
			bool operator () (const Acquisition &a, const Acquisition &b) const {  // bigger size first
				return a.id < b.id;
			}
		};

		size_t nr_free_slots;
		size_t nr_threads;
		size_t reserve_mem;

		std::set<Acquisition, AcquisitionBiggerSizeFirst> acquisitions_bigger_size_first;
		std::mutex acquisitions_bigger_size_first_mutex,
			free_mem_mutex, nr_free_slots_mutex;
		std::mutex alloc_mutex;

		struct AllocInfo {
			Allocator *allocator;
			size_t id;
			size_t mem;

			AllocInfo() {}
			AllocInfo(Allocator *allocator, size_t id, size_t mem) :
				allocator(allocator), id(id), mem(mem) {}
		};

		std::map<void *, AllocInfo> alloc_info;  // address -> (id, size)
		std::mutex alloc_info_mutex;
		std::map<Allocator *, size_t> alloced_size;
		std::mutex alloced_size_mutex;

		bool in_failing;
		std::mutex in_failing_mutex;

		void wait_mem(Allocator *allocator, size_t id, size_t size, bool is_enter) {
			{
				std::lock_guard<std::mutex> guard(acquisitions_bigger_size_first_mutex);
				acquisitions_bigger_size_first.emplace(allocator, id, size, is_enter);
			}
			{
				//                if (!is_enter) {
				//                    std::lock_guard<std::mutex> guard(nr_free_slots_mutex);
				//                    nr_free_slots ++;
				//                }
			}
			schedule();
		}

		// user space routine
		void do_syscall(Allocator *allocator, size_t id, size_t size, bool is_enter) throw (AllocFailed);

		void schedule();
		void fail_to_schedule();

		static size_t get_free_mem() {
			// return in byte;
			std::ifstream fin("/proc/meminfo");
			std::string str;

			size_t num_max = std::numeric_limits<size_t>::max();
			size_t nfree = num_max, ncache = num_max;
			while (not fin.eof()) {
				fin >> str;
				if (str == "MemFree:") {
					fin >> str;
					nfree = stoul(str);

					std::cerr << "MemFree: " << str << std::endl;

				} else if (str == "Cached:") {
					fin >> str;
					ncache = stoul(str);

					std::cerr << "Cached: " << str << std::endl;
				}
			}
			fin.close();
			if (nfree == num_max || ncache == num_max)
				return 0;
			return (nfree + ncache) * 1024;
		}

		static size_t get_mem_total() {
			// return in byte;
			std::ifstream fin("/proc/meminfo");
			std::string str;

			while (not fin.eof()) {
				fin >> str;
				if (str == "MemTotal:") {
					fin >> str;
					return stoul(str) * 1024lu;
				}
			}
			return 0;
		}

};

class Allocator {
	public:
		Allocator() : id(get_unique_id()), last_result(LastResult::SUCCEED) {
		}
		size_t get_id() const { return this->id; }
		void *alloc(size_t mem_size) {
			return SIGMODAllocator::get_instance()->alloc(this, id, mem_size);
		}
		void dealloc(void *data) {
			SIGMODAllocator::get_instance()->dealloc(this, data);
		}

		void enter() {
			SIGMODAllocator::get_instance()->enter(this, id);
		}
		void exit() {
			SIGMODAllocator::get_instance()->exit(this);
		}

	protected:
		enum LastResult {
			SUCCEED,
			FAILED_TO_ALLOC,
		};
		size_t id;
		std::mutex lock;
		LastResult last_result;

		static size_t get_unique_id() {
			static size_t cnt = 0;
			static std::mutex cnt_mutex;
			size_t ret;
			{
				std::lock_guard<std::mutex> lock(cnt_mutex);
				ret = cnt ++;
			}
			return ret;
		}
		friend class SIGMODAllocator;

		void release() {
			this->lock.unlock();
			this->last_result = SUCCEED;
		}
		void fail() {
			this->lock.unlock();
			this->last_result = FAILED_TO_ALLOC;
		}
};



/**
 * vim: syntax=cpp11 foldmethod=marker
 */

