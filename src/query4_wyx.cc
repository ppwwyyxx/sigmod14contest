/*
 * $File: query4_wyx.cc
 * $Date: Thu Apr 03 20:17:17 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#include "query4.h"
#include "data.h"
#include "lib/common.h"
#include "lib/Timer.h"
#include "SumEstimator.h"
#include "lib/hash_lib.h"
#include <omp.h>
#include <queue>
#include <algorithm>
#include <set>
#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

namespace {
	struct HeapEle {
		int vtx;
		double centrality;

		HeapEle() {}
		HeapEle(int _vtx, double _centrality) :
			vtx(_vtx), centrality(_centrality) {
			}
		bool operator < (const HeapEle& r) const
		{
			if (centrality == r.centrality)
				return vtx > r.vtx;
			return centrality < r.centrality;
		}
	};
}


void Query4Calculator::bfs_diameter(const std::vector<vector<int>> &g, int source, int &farthest_vtx,
		int &dist_max, vector<bool> &hash) {
	queue<int> q;
	q.push(source);
	hash[source] = true;
	farthest_vtx = source;
	dist_max = 0;
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
			FOR_ITR(v1, g[v0]) {
				if (hash[*v1])
					continue;
				hash[*v1] = true;
				farthest_vtx = *v1;
				dist_max = depth + 1;
				q.push(*v1);
			}
		}
	}
}

vector<int> Query4Calculator::work() {
	Timer timer;

	int est_dist_max = 2;  // TODO: this parameter needs tune
	int diameter = -1;
	std::vector<bool> diameter_hash(np), h2(np);
	for (int i = 0; i < (int)np; i ++) {
		if (diameter_hash[i])
			continue;
		int fv, d;
		bfs_diameter(friends, i, fv, d, h2);
		bfs_diameter(friends, fv, fv, d, diameter_hash);
		if (d > diameter)
			diameter = d;
	}

//	print_debug("Now time: %.4lf\n", timer.get_time());

	est_dist_max = max(2, (int)floor(log((double)diameter) / log(2.0) + 0.5));
	if (np > 10000)
		est_dist_max = 3;


	vector<HeapEle> heap_ele_buf(np);
	estimated_s.resize(np);
	timer.reset();

/*
 *    LimitDepthEstimator l_estimator(friends, degree, est_dist_max);
 *#pragma omp parallel for schedule(static) num_threads(4)
 *    REP(i, np) {
 *        estimated_s[i] = l_estimator.estimate(i);
 *    }
 */

	vector<bool> noneed(np, false);// TODO try adding this to bfs_limit_depth
	if (np > 10000) {
		RandomChoiceEstimator estimator1(friends, degree, pow(log(np), 0.333) / (20.2 * pow(np, 0.333)));
		auto wrong_result = move(estimator1.result);
		vector<PII> wrong_result_with_person;
		REP(i, np)
			wrong_result_with_person.emplace_back(wrong_result[i], i);
		sort(wrong_result_with_person.begin(), wrong_result_with_person.end());
		REPL(i, np * 0.6, np)
			noneed[wrong_result_with_person[i].second] = true;
		print_debug("estimate: %lf\n", timer.get_time());
	}

	HybridEstimator estimator(friends, degree, est_dist_max, noneed);
	estimated_s = move(estimator.result);

	//	estimate_all_s_using_delta_bfs(est_dist_max);
	//	PP(np);


	heap_ele_buf.clear();
	for (int i = 0; i < (int)np; i ++) {
		double centrality = get_centrality_by_vtx_and_s(i, estimated_s[i]);
		heap_ele_buf.emplace_back(i, centrality);
	}

	double time_phase1 = timer.get_time();
	timer.reset();
	priority_queue<HeapEle> q(heap_ele_buf.begin(), heap_ele_buf.end());

	// iterate
	vector<int> ans;

	int cnt = 0;
	{
		double last_centrality = 1e100;
		int last_vtx = -1;
		while (!q.empty()) {
			auto he = q.top(); q.pop();
			int vtx = he.vtx;
			double centrality = he.centrality;
			m_assert(centrality <= last_centrality);
			if (centrality == last_centrality && vtx == last_vtx) {
				ans.emplace_back(vtx);
				if ((int)ans.size() == k) {
					break;
				}
			} else {
				cnt ++;
				int s = get_exact_s(vtx);
				// int es = estimated_s[vtx];
				double new_centrality = get_centrality_by_vtx_and_s(vtx, s);
				q.push(HeapEle(vtx, new_centrality));
			}

			last_centrality = centrality;
			last_vtx = vtx;
		}
	}

	auto time = timer.get_time();
	//	print_debug("total: %f secs\n", time);
	if (np > 11000) {
		static int print = 0;
		if (print < 2)
			fprintf(stderr, "cnt: %f-%f %lu/%d/%d/%d/%d\n", time_phase1, time, np, cnt, k, (int)diameter, (int)est_dist_max);
		print ++;
	}
	return move(ans);
}


void Query4Handler::add_query(int k, const string& s, int index) {
	TotalTimer timer("Q4");
	// build graph
	vector<PersonInForum> persons = get_tag_persons(s);
	size_t np = persons.size();
	vector<vector<int>> friends(np);

	{
#pragma omp parallel for schedule(static) num_threads(4)
		REP(i, np) {
			auto& fs = Data::friends[persons[i]];
			FOR_ITR(itr, fs) {
				auto lb_itr = lower_bound(persons.begin(), persons.end(), itr->pid);
				if (lb_itr != persons.end() and *lb_itr == itr->pid) {
					int v = (int)distance(persons.begin(), lb_itr);
					friends[i].push_back(v);
				}
			}

			//            set<int> s(friends[i].begin(), friends[i].end());
			//            friends[i].resize(s.size());
			//            std::copy(s.begin(), s.end(), friends[i].begin());
		}
	}
	// finish building graph

	Query4Calculator worker(friends, k);
	auto now_ans = worker.work();
	FOR_ITR(itr, now_ans) {
		*itr = persons[*itr];
	}
	ans[index] = move(now_ans);
	continuation->cont();
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
