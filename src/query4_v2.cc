/*
 * $File: query4_v2.cc
 * $Date: Sat Mar 15 01:44:40 2014 +0000
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
	int * degree;
	size_t *que;
	size_t np;
	vector<vector<int>> friends;
}

vector<PersonInForum> get_tag_persons(const string& s) {
	int tagid = Data::tagid[s];
	auto& forums = Data::tag_forums[tagid];
	vector<PersonInForum> persons;

	FOR_ITR(itr, forums) {
		set<PersonInForum>& persons_in_forum = (*itr)->persons;
		vector<PersonInForum> tmp; tmp.swap(persons);
		persons.resize(tmp.size() + persons_in_forum.size());
		vector<PersonInForum>::iterator ret_end = set_union(
				persons_in_forum.begin(), persons_in_forum.end(),
				tmp.begin(), tmp.end(), persons.begin());
		persons.resize(std::distance(persons.begin(), ret_end));
	}
	return persons;
}

namespace {
//    static int timestamp_counter;
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

int check_compute_degree(int v) {
	deque<size_t> q;
	q.push_back(v);
	vector<bool> hash(np);
	hash[v] = true;
	int deg = 1;
	while (!q.empty()) {
		size_t size = q.size();
		for (size_t j = 0; j < size; j ++) {
			size_t u = q.front();
			q.pop_front();
			auto &fr = friends[u];
			for (auto it = fr.begin(); it != fr.end(); it ++) {
				auto &w = *it;
				if (hash[w])
					continue;
				hash[w] = true;
				deg ++;
				q.push_back(w);
			}
		}
	}
//    fprintf(stderr, "deg %d = %d\n", v, deg);
	return deg;
}

void compute_degree() {
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

long long check_compute_s(int v) {
	deque<size_t> q;
	q.push_back(v);
	vector<bool> hash(np);
	hash[v] = true;
	long long ret = 0;
	int depth = 0;
	while (!q.empty()) {
		size_t size = q.size();
		depth ++;
		for (size_t j = 0; j < size; j ++) {
			size_t u = q.front();
			q.pop_front();
			FOR_ITR(itr, friends[u]) {
				auto&w = *itr;
				if (hash[w])
					continue;
				hash[w] = true;
				ret += depth;
				q.push_back(w);
			}
		}
	}
//    fprintf(stderr, "s %d = %lld\n", v, ret);
	return ret;
}


class CentralityEstimator {
	public:
		deque<size_t> q;
		vector<int> hash;
		int timestamp_counter;
		CentralityEstimator() :
			hash(np), timestamp_counter(0) {
		}
		double estimate(int v0, int dist_max) {
			timestamp_counter ++;
			q.clear();
			q.push_back(v0);
			size_t qsize;
			long long s = 0;
			size_t nr_remain = degree[v0];
			int depth = 0;
			hash[v0] = timestamp_counter;
			while ((qsize = q.size())) {
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
						if (hash[index] == timestamp_counter)
							continue;
						hash[index] = timestamp_counter;
						q.push_back(index);
					}
				}
			}
//            assert(nr_remain == 0);
			s += nr_remain * (dist_max + 1);
//            assert(s == check_compute_s(v0));

			double ret = 0;
			if (s == 0) {
//                assert(degree[v0] == 1);
				ret = 0;
			}
			else
				ret = ::sqr(degree[v0] - 1.0)  / (double)s / (double)(np - 1);
			return ret;
		}
};

void Query4Handler::add_query(int k, const string& s) {
	// build graph
	vector<PersonInForum> persons = get_tag_persons(s);
	np = persons.size();
	friends.resize(np);
	REP(i, np) {
		friends[i].clear();
		auto& fs = Data::friends[persons[i]];
		FOR_ITR(itr, fs) {
			auto lb_itr = lower_bound(persons.begin(), persons.end(), itr->pid);
			if (*lb_itr == itr->pid) {
				friends[i].push_back((int)distance(persons.begin(), lb_itr));
			}
		}
	}

	// compute degree
	degree = new int[np];
	que = new size_t[np];
	compute_degree();

	// estimate centrality
	int est_dist_max = 2;  // TODO: this parameter needs tune
	CentralityEstimator estimator;
	vector<HeapEle> heap_ele_buf(np);
	for (int i = 0; i < (int)np; i ++) {
		double centrality = estimator.estimate(i, est_dist_max);
		heap_ele_buf[i] = HeapEle(i, centrality);
//        q.push(HeapEle(i, centrality));
//        fprintf(stderr, "cent %lu = %f\n", i, centrality);
	}
	priority_queue<HeapEle> q(heap_ele_buf.begin(), heap_ele_buf.end());

	// calculate answer
	ans.push_back(vector<int>());
	auto &line = ans.back();
	double last_centrality = 1e100;
	int last_pid = -1;
	int cnt = 0;
	while (!q.empty()) {
		auto he = q.top(); q.pop();
		auto pid = he.pid;
		auto centrality = he.centrality;
		assert(centrality <= last_centrality);
		if (centrality == last_centrality && pid == last_pid) {
			//fprintf(stderr, "%f\n", centrality);
			line.push_back(persons[pid]);
			if ((int)line.size() == k)
				break;
		} else {
			cnt ++;
			q.push(HeapEle(pid, estimator.estimate(pid, (int)np)));
		}
		last_centrality = centrality;
		last_pid = pid;
	}
	//fprintf(stderr, "cnt: %d/%d\n", cnt, k);
	delete[] degree;
	delete[] que;
}

void Query4Handler::work() {
}

void Query4Handler::print_result() {
	lock_guard<mutex> lg(mt_work_done);
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
