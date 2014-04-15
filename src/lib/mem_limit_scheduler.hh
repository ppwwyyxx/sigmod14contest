/*
 * $File: mem_limit_scheduler.hh
 * $Date: Tue Apr 15 18:31:11 2014 +0800
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#pragma once

/*
 * Scheduling when memory size is limited.
 *
 */
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
class MemLimitScheduler {
	public:
		static void init(size_t nr_threads, size_t mem_size_max) {
			MemLimitScheduler::nr_threads = nr_threads;
			MemLimitScheduler::mem_size_max = mem_size_max;

			free_mem = mem_size_max;
			nr_free_slots = nr_threads;
			nr_active = 0;

			fprintf(stderr, "scheduler: nr_threads: %lu mem_size_max: %lu\n", nr_threads, mem_size_max);
		}

		// ``user space'' routine
		static void acquire(std::mutex &lock, size_t id, size_t mem_size) {
			lock.lock();
			std::thread th(std::bind(MemLimitScheduler::add_acquisition, std::ref(lock), id, mem_size));  // ``system call''
			th.detach();
			lock.lock();
			lock.unlock();
		}

		// ``user space'' routine
		static void release(size_t id, size_t mem_size) {
			std::thread th(std::bind(MemLimitScheduler::sys_release, id, mem_size));
			th.detach();
		}

	protected:
		static void sys_release(size_t id, size_t mem_size) {
			{
				std::lock_guard<std::mutex> free_mem_lock(free_mem_mutex);
				free_mem += mem_size;
			}
			bool need_sched = false;
			{
				std::lock_guard<std::mutex> acquired_mem_by_id_lock(acquired_mem_by_id_mutex);
				auto idit = acquired_mem_by_id.find(id);

				assert(idit != acquired_mem_by_id.end()); // XXX
//                idit->second -= mem_size;
				idit->second --;
				if (idit->second == 0) {
					acquired_mem_by_id.erase(idit);
					nr_free_slots ++;
					{
						std::lock_guard<std::mutex> guard(nr_active_mutex);
						nr_active --;
					}
					need_sched = true;
				}
			}
			if (need_sched)
				schedule();
		}

		static void add_acquisition(std::mutex &lock, size_t id, size_t mem_size) {

			{
				std::lock_guard<std::mutex> guard(acquisitions_bigger_size_first_mutex);
				acquisitions_bigger_size_first.insert(Acquisition(lock, id, mem_size));
			}
			{
				std::lock_guard<std::mutex> guard(acquired_mem_by_id_mutex);
				auto it = acquired_mem_by_id.find(id);
				if (it == acquired_mem_by_id.end()) {
					acquired_mem_by_id[id] += mem_size;
				}
				else {  // nested acuquisition
//                    it->second += mem_size;
					it->second ++;
					{
						std::lock_guard<std::mutex> guard(nr_active_mutex);
						nr_active --;
						nr_free_slots ++;
					}
				}
			}
			schedule();
		}

		static size_t nr_threads;
		static size_t nr_free_slots;
		static size_t mem_size_max;
		static size_t free_mem;
		static size_t nr_active;


		struct Acquisition {
			Acquisition(std::mutex &lock, size_t id, size_t mem_size) :
				lock(lock), id(id), mem_size(mem_size) {
			}
			std::mutex &lock;
			size_t id, mem_size;
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

		static std::set<Acquisition, AcquisitionBiggerSizeFirst> acquisitions_bigger_size_first;
		static std::map<size_t, size_t> acquired_mem_by_id;
		static std::mutex acquisitions_bigger_size_first_mutex,
			free_mem_mutex, nr_free_slots_mutex,
			acquired_mem_by_id_mutex,
			nr_active_mutex;

		static void schedule() {
			std::lock_guard<std::mutex> lock(acquisitions_bigger_size_first_mutex);
			for (auto it = acquisitions_bigger_size_first.begin();
					it != acquisitions_bigger_size_first.end(); ) {
				{
					std::lock_guard<std::mutex> nr_free_slots_lock(nr_free_slots_mutex);
					if (nr_free_slots == 0)
						break;
				}
				auto it_next = it;
				it_next ++;
				{
					free_mem_mutex.lock();
//                    std::lock_guard<std::mutex> free_mem_lock(free_mem_mutex);
					if (free_mem >= it->mem_size) {
						free_mem -= it->mem_size;
						free_mem_mutex.unlock();

						fprintf(stderr, "start running %lu ...\n", it->id);
						it->lock.unlock();  // the thread starts runing
						nr_active ++;

						{
							std::lock_guard<std::mutex> nr_free_slots_lock(nr_free_slots_mutex);
							nr_free_slots --;
						}
						acquisitions_bigger_size_first.erase(it);
					} else
						free_mem_mutex.unlock();
				}
				it = it_next;
			}
			fprintf(stderr, "free_mem: %lu\n", free_mem);
			fprintf(stderr, "nr_active: %lu\n", nr_active);
			fprintf(stderr, "nr_free_slots: %lu\n", nr_free_slots);
			if (!acquisitions_bigger_size_first.empty() && nr_active == 0) {
				fprintf(stderr, "ERROR: unable to schedule\n");
				print_mem_map();
			}
		}

		static void print_mem_map() {
			std::lock_guard<std::mutex> lock(acquired_mem_by_id_mutex);
			for (auto it = acquired_mem_by_id.begin(); it != acquired_mem_by_id.end(); it ++) {
				fprintf(stderr, "%lu: %lu\n", it->first, it->second);
			}
			fflush(stderr);
		}
};

class MemAcquirer {
	public:
		MemAcquirer();
		size_t get_id() const { return this->id; }
		void aquire(size_t mem_size);
		void release(size_t mem_size);

	protected:
		size_t id;
		std::mutex acquire_mutex;
};



/**
 * vim: syntax=cpp11 foldmethod=marker
 */

