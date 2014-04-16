//File: q4Scheduler.h
//Date: Wed Apr 16 02:00:49 2014 +0000
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <set>
#include <string>
#include <vector>
#include <mutex>
#include <thread>

#include "globals.h"
#include "query4.h"
#include "lib/common.h"
#include "data.h"
#include "lib/utils.h"

extern std::vector<Query4> q4_set;
extern Query4Handler q4;

class Q4Scheduler {
	public:
		struct Q4Job {
			int k, idx;
			std::string& tag;
			int mem;
			Q4Job(int k, int idx, std::string& tag, int mem):
				k(k), idx(idx), tag(tag), mem(mem) {}

			bool operator < (const Q4Job& r) const {
				return (mem > r.mem) || (mem == r.mem && tag > r.tag);
			}
		};

		int n_thread;
		int n_free;
		int mem_free;
		int mem_using;
		std::set<Q4Job> jobs;

		std::mutex work_mt;


		Q4Scheduler(int n_thread) :
			n_thread(n_thread), n_free(n_thread), mem_using(0)
		{
			size_t nq4 = q4_set.size();
			REP(i, nq4) {
				std::string & s = q4_set[i].tag;
				size_t np = cnt_tag_persons_hash(s);
				/*
				 *int size = np * np / 8 / 1024 / 1024;
				 *if (np < 100000 && Data::nperson > 3000000)
				 *    size *= 2;
				 *else size /= 10;
				 */
				int size = np / 100;
				jobs.insert(Q4Job(q4_set[i].k, i, s, size));
			}
			mem_free = Q4Scheduler::get_free_mem();
		}

		void work() {
			std::lock_guard<std::mutex> lg(work_mt);
			if (jobs.size() == 0)
				return;

			if (n_free) {
				for (auto itr = jobs.begin(); itr != jobs.end();) {
					if (itr->mem < mem_free) {
						print_debug("About to start job with np=%d, mem=%d, nfree=%d, mem_using=%d, mem_free=%d\n",
								cnt_tag_persons_hash(itr->tag), itr->mem, n_free,
								mem_using, mem_free);
						__sync_fetch_and_add(&n_free, -1);
						__sync_fetch_and_add(&mem_free, -itr->mem);
						__sync_fetch_and_add(&mem_using, itr->mem);
						std::thread th(&Q4Scheduler::do_job, this, *itr);
						th.detach();
						jobs.erase(itr++);
						if (not n_free)
							break;
						continue;
					}
					++itr;
				}
			}
		}

		void do_job(Q4Job job) {
			print_debug("Start job with np=%d, mem=%d, nfree=%d, mem_using=%d, mem_free=%d\n",
					cnt_tag_persons_hash(job.tag), job.mem, n_free,
					mem_using, mem_free);
			q4.add_query(job.k, job.tag, job.idx);
			__sync_fetch_and_add(&n_free, 1);
			__sync_fetch_and_add(&mem_free, job.mem);
			__sync_fetch_and_add(&mem_using, -job.mem);
			refresh_mem();
			work();
		}

		void refresh_mem() {
			int free = Q4Scheduler::get_free_mem();
			mem_free = free - mem_using;
		}

		static int get_mem_total() { // in MB
			std::ifstream fin("/proc/meminfo");
			std::string str;
			while (not fin.eof()) {
				fin >> str;
				if (str == "MemTotal:") {
					fin >> str;
					return (int)(stoul(str) / 1024lu);
				}
			}
			return 0;
		}

		static int get_free_mem() {
			// might be fake
			int m = ::get_free_mem();
			int tot = Q4Scheduler::get_mem_total();
			int used = tot - m;
			int ret = 13u * 1024 - used;
			if (ret < 0) return 0;
			return ret;
		}

};
