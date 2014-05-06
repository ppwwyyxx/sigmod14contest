/*
 * $File: query4.cpp
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 * $Date: Tue May 06 22:41:16 2014 +0000
 */

#include "query4.h"
#include "data.h"
#include "lib/common.h"
#include "SumEstimator.h"
//#include "search_depth_estimator.h"
#include "lib/hash_lib.h"
#include <omp.h>
#include <queue>
#include <algorithm>
#include <set>
#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>

#include "query4.h"
#include "data.h"
#include "lib/common.h"
#include "SumEstimator.h"
#include "HybridEstimator.h"
#include "lib/hash_lib.h"

using namespace std;

namespace {
	struct HeapEle {
		int vtx;
		double centrality;

		HeapEle() {}
		HeapEle(int _vtx, double _centrality) :
			vtx(_vtx), centrality(_centrality)
		{ }

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
	TotalTimer ttt("q4calculator");
	Timer timer;

	const bool use_estimate = (np > 10000 && k < 20);

	vector<bool> noneed(np, false);
	vector<int> approx_result;
	vector<int> s_calculated;
	//int thres = (int)((double)np * 0.31);		// 0.51 is ratio to keep
	int thres = np;
	vector<PII> approx_result_with_person; approx_result_with_person.reserve(np);
	int sum_bound = 1e9;
	std::vector<int> wrong_result;
	{
		TotalTimer tttt("estimate random");
		if (use_estimate) {
			//	RandomChoiceEstimator estimator1(friends, degree, pow(log(np), 0.333) / (20.2 * pow(np, 0.333)));
			float perc = 0.002f;
			if (np > 100000) perc = 0.0015f;
			//if (np > 200000) perc = 0.001;
			RandomChoiceEstimator estimator1(friends, degree, perc);
			/*
			 *if (np > 100000)
			 *    estimator1.error();
			 */

			approx_result = move(estimator1.result);
			FOR_ITR(index, estimator1.samples) {
				noneed[*index] = true;
				exact_s[*index] = approx_result[*index];			// they are not wrong result
				s_calculated.push_back(*index);
			}

			REP(i, np)
				approx_result_with_person.emplace_back(approx_result[i], i);
			sort(approx_result_with_person.begin(), approx_result_with_person.end());

			vector<PDI> some_real_cent;
			int nr_sample = 2 * k;// can be x * k
			FOR_ITR(itr, approx_result_with_person) {
				int pid = itr->second;
				int s = get_exact_s(pid);

				s_calculated.push_back(pid);
				noneed[pid] = true;
				if (s < 10)
					continue;
				some_real_cent.emplace_back(
						get_centrality_by_vtx_and_s(pid, s), pid);
				if ((int)some_real_cent.size() == nr_sample)
					break;
			}

			// find kth largest
			nth_element(some_real_cent.begin(),
					some_real_cent.begin() + nr_sample - k,
					some_real_cent.end());

			sum_bound = exact_s[some_real_cent[nr_sample - k].second];
			double cut_bound = (double)sum_bound / 0.9 * 1.1;
			auto itr = lower_bound(approx_result_with_person.begin(),
					approx_result_with_person.end(),
					PII(cut_bound, 0));
			thres = std::distance(approx_result_with_person.begin(), itr);
			thres = max(thres, (int)(np * 0.3));
			thres = min(thres, (int)(np * 0.5));
			REPL(i, thres, (int)np)
			    noneed[approx_result_with_person[i].second] = true;
		}
	}

	HybridEstimator estimator(friends, degree,
			noneed, sum_bound, approx_result);
	estimator.init();

	estimated_s = move(estimator.result);

	if (use_estimate) {
		REPL(i, thres, (int)np)
			estimated_s[approx_result_with_person[i].second] = 1e9;
		FOR_ITR(itr, s_calculated)
			estimated_s[*itr] = exact_s[*itr];
	} else {
		m_assert(approx_result.size() == 0);		// Hybridestimator assumes this.
	}

	vector<HeapEle> heap_ele_buf; heap_ele_buf.reserve(np);
	for (int i = 0; i < (int)np; i ++) {
		double centrality = get_centrality_by_vtx_and_s(i, estimated_s[i]);
		heap_ele_buf.emplace_back(i, centrality);
	}

	priority_queue<HeapEle> q(heap_ele_buf.begin(), heap_ele_buf.end());

	// iterate
	vector<int> ans;
	int cnt = 0;
	{
		DEBUG_DECL(TotalTimer, ttt("iterate q4 heap"));		// about 6% of total q4 time
		DEBUG_DECL(GuardedTimer, tttt(string_format("np: %d iterate q4 heap", np).c_str()));
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
				q.emplace(vtx, new_centrality);
			}

			last_centrality = centrality;
			last_vtx = vtx;
		}
	}

	if (np > 1e4) {
		static int print = 0;
#ifndef DEBUG
		if (print < 3) {

#endif
			fprintf(stderr, "%lu/%d/%dd%d:%.4lf\n", np, cnt, k,
					estimator.depth, timer.get_time());
			fflush(stderr);
#ifndef DEBUG
		}
#endif
		if (estimator.cutcnt > 1000)
			fprintf(stderr, "cut%d~%d~%d/%d\n", exact_s[ans.front()], exact_s[ans.back()],
					sum_bound, estimator.cutcnt);
		print ++;
	}
	return move(ans);
}


void Query4Handler::add_query(int k, const string& s, int index) {
	TotalTimer timer("Q4");
	// build graph
	vector<bool> persons = get_tag_persons_hash(s);

	size_t np = 0;
	vector<vector<int>> friends;
	vector<int> old_pid;

	{
		TotalTimer tt("build graph q4");
		vector<int> new_pid(Data::nperson);
		REP(i, Data::nperson) {
			if (persons[i]) {
				new_pid[i] = (int)np;
				old_pid.push_back(i);
				np ++;
			}
		}
		friends.resize(np);
//#pragma omp parallel for schedule(dynamic) num_threads(2)
		REP(i, Data::nperson) {
			if (not persons[i]) continue;
			auto &fs = Data::friends[i];
			FOR_ITR(itr, fs) {
				int pid = itr->pid;
				if (persons[pid])
					friends[new_pid[i]].push_back(new_pid[pid]);
			}
		}
	}
	fprintf(stderr, "np%d\n", np);fflush(stderr);
	// finish building graph

	Query4Calculator worker(friends, k);
	auto now_ans = worker.work();
	FOR_ITR(itr, now_ans)
		*itr = old_pid[*itr];
	ans[index] = move(now_ans);
	fprintf(stderr, "fnp%d\n", np);fflush(stderr);

	if (Data::nperson > 1e4)
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
