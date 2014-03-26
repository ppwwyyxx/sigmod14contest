/*
 * $File: query4_v2.cc
 * $Date: Wed Mar 26 12:26:55 2014 +0800
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#include "query4.h"
#include "data.h"
#include "lib/common.h"
#include "lib/Timer.h"
#include "lib/utils.h"
#include "lib/hash_lib.h"
#include <omp.h>
#include <queue>
#include <algorithm>
#include <set>
#include <cassert>

using namespace std;

namespace {
	struct HeapEle {
		int pid;
		double centrality;

		HeapEle() {}
		HeapEle(int _pid, double _centrality) :
			pid(_pid), centrality(_centrality) {
			}
		bool operator < (const HeapEle& r) const
		{
			if (centrality == r.centrality)
				return pid > r.pid;
			return centrality < r.centrality;
		}
	};

}

/*
 *int check_compute_degree(int v) {
 *    deque<size_t> q;
 *    q.push_back(v);
 *    vector<bool> hash(np);
 *    hash[v] = true;
 *    int deg = 1;
 *    while (!q.empty()) {
 *        size_t size = q.size();
 *        for (size_t j = 0; j < size; j ++) {
 *            size_t u = q.front();
 *            q.pop_front();
 *            auto &fr = friends[u];
 *            for (auto it = fr.begin(); it != fr.end(); it ++) {
 *                auto &w = *it;
 *                if (hash[w])
 *                    continue;
 *                hash[w] = true;
 *                deg ++;
 *                q.push_back(w);
 *            }
 *        }
 *    }
 *    fprintf(stderr, "deg %d = %d\n", v, deg);
 *    return deg;
 *}
 *
 */
void Query4Calculator::compute_degree() {
	for (size_t i = 0; i < np; i ++)
		degree[i] = -1;

	for (size_t i = 0; i < np; i ++) {
		if (degree[i] != -1)
			continue;
		degree[i] = 1;
		int qh = 0, qt = 1;
		que[qh] = i;
		while (qh != qt) {
			size_t v0 = que[qh ++];
			FOR_ITR(itr, friends[v0]) {
				auto& v1 = *itr;
				if (degree[v1] != -1)
					continue;

				degree[v1] = 1;
				que[qt ++] = v1;
			}
		}
		for (int j = 0; j < qt; j ++)
			degree[que[j]] = qt;
	}
//    for (size_t i = 0; i < np; i ++) {
//        assert(degree[i] == check_compute_degree(i));
//        assert(degree[i] != -1 && degree[i] != 0);
//    }
}

/*
 *long long check_compute_s(int v) {
 *    deque<size_t> q;
 *    q.push_back(v);
 *    vector<bool> hash(np);
 *    hash[v] = true;
 *    long long ret = 0;
 *    int depth = 0;
 *    while (!q.empty()) {
 *        size_t size = q.size();
 *        depth ++;
 *        for (size_t j = 0; j < size; j ++) {
 *            size_t u = q.front();
 *            q.pop_front();
 *            FOR_ITR(itr, friends[u]) {
 *                auto&w = *itr;
 *                if (hash[w])
 *                    continue;
 *                hash[w] = true;
 *                ret += depth;
 *                q.push_back(w);
 *            }
 *        }
 *    }
 *    fprintf(stderr, "s %d = %lld\n", v, ret);
 *    return ret;
 *}
 *
 */

class CentralityEstimator {
	public:
		size_t np;
		const std::vector<std::vector<int>>& friends;
		int* degree;

		CentralityEstimator(const vector<vector<int>>& fr, int* deg) :
			np(fr.size()), friends(fr), degree(deg)
		{ }

		double estimate(int v0, int dist_max) {
			vector<bool> hash(np, false);
			deque<size_t> q; q.push_back(v0);

			long long s = 0;
			size_t nr_remain = degree[v0];
			int depth = 0;
			hash[v0] = true;
			while (size_t qsize = q.size()) {
				s += qsize * depth;
//                assert(nr_remain >= qsize);
				nr_remain -= qsize;
				depth ++;
				if (depth > dist_max)
					break;
				REP(_k, qsize) {
					auto top = q.front(); q.pop_front();
					auto& fs = friends[top];
					FOR_ITR(itr, fs) {
						// found a friend in the forum
						size_t index = *itr;
						if (hash[index]) continue;
						hash[index] = 1;
						q.push_back(index);
					}
				}
			}
			s += nr_remain * (dist_max + 1);
			if (np == 14301)
				print_debug("degree[v0]-nr_remain:%d\n", degree[v0] - nr_remain);
			if (s == 0)
				return 0;
			else
				return ::sqr(degree[v0] - 1.0)  / (double)s / (double)(np - 1);
		}
};

vector<int> Query4Calculator::work() {
	// estimate centrality
	Timer timer;
	CentralityEstimator estimator(friends, degree);
	int est_dist_max = 2;  // TODO: this parameter needs tune
	if (np > 2400 and Data::nperson > 11000) est_dist_max = 3;

	vector<HeapEle> heap_ele_buf(np);
	{
		GuardedTimer timer("omp2");
#pragma omp parallel for schedule(static) num_threads(4)
		for (int i = 0; i < (int)np; i ++) {
			double centrality = estimator.estimate(i, est_dist_max);
			heap_ele_buf[i] = HeapEle(i, centrality);
			//        q.push(HeapEle(i, centrality));
			//        fprintf(stderr, "cent %lu = %f\n", i, centrality);
		}
	}
	priority_queue<HeapEle> q(heap_ele_buf.begin(), heap_ele_buf.end());


	// calculate answer
	vector<int> ans;
	double last_centrality = 1e100;
	int last_pid = -1;
	int cnt = 0;
	{
	GuardedTimer timer("haha")	;
	while (!q.empty()) {
		auto he = q.top(); q.pop();
		auto pid = he.pid;
		auto centrality = he.centrality;
		assert(centrality <= last_centrality);
		if (centrality == last_centrality && pid == last_pid) {
			//fprintf(stderr, "%f\n", centrality);
			ans.emplace_back(pid);
			if ((int)ans.size() == k)
				break;
		} else {
			cnt ++;
			q.push(HeapEle(pid, estimator.estimate(pid, (int)np)));
		}
		last_centrality = centrality;
		last_pid = pid;
	}
	}

	if (timer.get_time() > 0.1) {
		print_debug("----------------------------->%lf\n", timer.get_time());
	}
	print_debug("cnt: %d, k: %d, np: %d\n", cnt, k, np);
	return move(ans);
}


void Query4Handler::add_query(int k, const string& s, int index) {
	TotalTimer timer("Q4");
	// build graph
	vector<PersonInForum> persons = get_tag_persons(s);
	size_t np = persons.size();
	vector<vector<int>> friends(np);

	{
		GuardedTimer timer("omp1");
#pragma omp parallel for schedule(static) num_threads(4)
		REP(i, np) {
			auto& fs = Data::friends[persons[i]];
			FOR_ITR(itr, fs) {
				auto lb_itr = lower_bound(persons.begin(), persons.end(), itr->pid);
				if (lb_itr != persons.end() and *lb_itr == itr->pid) {
					friends[i].push_back((int)distance(persons.begin(), lb_itr));
				}
			}
		}
	}
	// finish building graph

	Query4Calculator worker(friends, k);
	auto now_ans = worker.work();
	FOR_ITR(itr, now_ans) {
		*itr = persons[*itr];
	}
	ans[index] = move(now_ans);
}


void Query4Handler::work() { }
void Query4Handler::print_result() {
	FOR_ITR(itr, ans) {
		vector<int>& line = *itr;
		for (size_t k = 0; k < line.size(); k ++) {
			if (k) printf(" ");
			printf("%d", line[k]);
		}
		printf("\n");
	}
}

/*
 * vim: syntax=cpp11.doxygen foldmethod=marker
 */
