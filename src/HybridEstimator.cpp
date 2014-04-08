//File: HybridEstimator.cpp
//Date: Tue Apr 08 19:38:01 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "HybridEstimator.h"
using namespace std;

HybridEstimator::HybridEstimator(const std::vector<std::vector<int>>& _graph, int* _degree,
		int _depth_max,
		vector<bool>& _noneed, int _sum_bound,
		const vector<int>& _approx_result):
	SumEstimator(_graph), depth_max(_depth_max), degree(_degree),
	noneed(_noneed), sum_bound(_sum_bound),
	approx_result(_approx_result)
{
	m_assert(depth_max == 3);
	bfs_2_dp_1();
}


void HybridEstimator::bfs_2_dp_1() {
	Timer init;
	int len = get_len_from_bit(np);

	std::vector<Bitset> s_prev;
	{
		TotalTimer tt("hybrid alloc");			// about 3% of total q4 time
		s_prev.reserve(np);
		REP(i, np)
			s_prev.emplace_back(len);

		result.resize((size_t)np, 0);
		nr_remain.resize(np);
		REP(i, np)
			nr_remain[i] = degree[i];
	}

	// bfs 2 depth
	cutcnt = 0;
	{
		TotalTimer ttt("depth 2");
		vector<int> hash(np, 0);
		int tag = 0;
		REP(i, np) {
			tag ++;

			// depth 0
			hash[i] = tag;
			s_prev[i].set(i);
			nr_remain[i] -= 1;

			size_t sum_dv1 = 0;
			// depth 1
			FOR_ITR(fr, graph[i]) {
				sum_dv1 += graph[*fr].size();
				hash[*fr] = tag;
				s_prev[i].set(*fr);
			}
			nr_remain[i] -= (int)graph[i].size();
			result[i] += (int)graph[i].size();

			size_t sum_dv2 = 0;
			// depth 2
			FOR_ITR(fr, graph[i]) {
				int j = *fr;
				FOR_ITR(fr2, graph[j]) {
					if (hash[*fr2] == tag)
						continue;
					sum_dv2 += graph[*fr2].size();
					hash[*fr2] = tag;
					s_prev[i].set(*fr2);
					nr_remain[i] --;
					result[i] += 2;
				}
			}

			if (not noneed[i]) {
				// XXX this is wrong
				int n3_upper = (int)sum_dv2 - (int)sum_dv1 + (int)graph[i].size() + 1;
				m_assert(n3_upper >= 0);
				int est_s_lowerbound = result[i] + n3_upper * 3 + (nr_remain[i] - n3_upper) * 4;
				if (est_s_lowerbound > sum_bound) {		// cut
					noneed[i] = true;
					cutcnt ++;
					result[i] = 1e9;
				}
			}
		}
	}

	// union depth 3
	{
		int cnt = 0;
		TotalTimer ttt("depth 3");
		Bitset s(len);
		REP(i, np) {
			if (noneed[i]) continue;
			if (result[i] == 0) continue;
			if (nr_remain[i] == 0) continue;
			cnt ++;
			s.reset(len);
			FOR_ITR(fr, graph[i]) s.or_arr(s_prev[*fr], len);
			s.and_not_arr(s_prev[i], len);

			int c = s.count(len);
			//			print_debug("S1: %lu, S2: %d S3: %d\n", graph[i].size(), s_prev[i].count(len) - graph[i].size(), c);
			result[i] += c * 3;
			nr_remain[i] -= c;
			result[i] += nr_remain[i] * 4;
		}
	}
}
