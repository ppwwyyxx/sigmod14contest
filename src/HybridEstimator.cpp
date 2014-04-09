//File: HybridEstimator.cpp
//Date: Wed Apr 09 08:47:35 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "HybridEstimator.h"
#include "globals.h"
using namespace std;

HybridEstimator::HybridEstimator(const std::vector<std::vector<int>>& _graph, int* _degree,
		vector<bool>& _noneed, int _sum_bound,
		const vector<int>& _approx_result):
	SumEstimator(_graph), degree(_degree),
	noneed(_noneed), sum_bound(_sum_bound),
	approx_result(_approx_result)
{
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

			/*
			 *if (not noneed[i]) {
			 *    // XXX this is wrong
			 *    int n3_upper = (int)sum_dv2 - (int)sum_dv1 + (int)graph[i].size() + 1;
			 *    m_assert(n3_upper >= 0);
			 *    int est_s_lowerbound = result[i] + n3_upper * 3 + (nr_remain[i] - n3_upper) * 4;
			 *    if (est_s_lowerbound > sum_bound) {		// cut
			 *        noneed[i] = true;
			 *        cutcnt ++;
			 *        result[i] = 1e9;
			 *    }
			 *}
			 */
		}
	}

	// union depth 3
	{
		TotalTimer ttt("depth 3");
		int nr_empty = threadpool->get_nr_empty_thread();
		if (nr_empty) {
#pragma omp parallel for schedule(dynamic) num_threads(nr_empty)
			REP(i, np) {
				if (noneed[i]) continue;
				if (result[i] == 0) continue;
				if (nr_remain[i] == 0) continue;
				Bitset s(len);
				FOR_ITR(fr, graph[i])
					s.or_arr(s_prev[*fr], len);
				s.and_not_arr(s_prev[i], len);

				int c = s.count(len);
				//			print_debug("S1: %lu, S2: %d S3: %d\n", graph[i].size(), s_prev[i].count(len) - graph[i].size(), c);
				result[i] += c * 3;
				nr_remain[i] -= c;
				result[i] += nr_remain[i] * 4;
			}
		} else {
			Bitset s(len);
			REP(i, np) {
				if (noneed[i]) continue;
				if (result[i] == 0) continue;
				if (nr_remain[i] == 0) continue;
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
	print_debug("Q4 with np=%d have errorrate=%.4lf\n", np, average_err(result));
}

void HybridEstimator::bfs_2_dp_more() {
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

	vector<Bitset> s; s.reserve(np);		// XXX MEMORY!!
	vector<int> tmp_result(np);
	REP(i, np)
		s.emplace_back(len);
	int depth = 3;
	while (true) {
		TotalTimer ttt("Depth 3+");
		// calculate s from s_prev
		REP(i, np) {
			/*		// cannot prune in depth=3, because depth=4 need every s[i]_3
			 *if (noneed[i]) continue;
			 *if (result[i] == 0) continue;
			 *if (nr_remain[i] == 0) continue;
			 */
			s[i].reset(len);
			FOR_ITR(fr, graph[i])
				s[i].or_arr(s_prev[*fr], len);
			s[i].and_not_arr(s_prev[i], len);
			int c = s[i].count(len);
			result[i] += c * depth;
			nr_remain[i] -= c;
			tmp_result[i] = result[i] + nr_remain[i] * (depth + 1);
		}
		// judge whether tmp_result is accurate enough
		double err = average_err(tmp_result);
		if (err < 0.05) {		// TODO Need better criteria
			break;
		}
		print_debug("Err: %.4lf with np=%d\n", err, np);

		if (depth == 4)		// XXX at most 4? might be enough!  // TODO: 4th level can be implemented with half memory
			break;
		depth ++;
		s.swap(s_prev);
	}
	result = move(tmp_result);
	print_debug("Q4 with np=%d, depth=%d, have errorrate=%.4lf\n", np, depth, average_err(result));
}
