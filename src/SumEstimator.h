//File: SumEstimator.h
//Date: Wed Mar 26 11:18:12 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include "lib/common.h"
#include "lib/debugutils.h"

class SumEstimator {
	public:
		const std::vector<std::vector<int>>& graph;
		int np;

		SumEstimator(const std::vector<std::vector<int>>& _graph)
			: graph(_graph), np((int)graph.size()) { }

		virtual int estimate(int i) = 0;


		// print error
		virtual void error();

		int get_exact_s(int i);
};

class RandomChoiceEstimator: public SumEstimator {
	public:
		std::vector<int> result;
		std::vector<int> samples;
		int* degree;

		// p: percent of point to randomly choose. 0 < p < 1
		RandomChoiceEstimator(const std::vector<std::vector<int>>& _graph, int* _degree, float p)
			: SumEstimator(_graph),
			result(np, 0LL), samples(np), degree(_degree)
		{
				m_assert(0 < p && p < 1);
				std::iota(samples.begin(), samples.end(), 0);
				std::random_shuffle(samples.begin(), samples.end());
				samples.resize((size_t)np * p);
				work();
		}

		void work();
		int bfs_all(int source, std::vector<int>&);

		int estimate(int i) override { return result[i]; }

};

#include <boost/dynamic_bitset.hpp>
class UnionSetDepthEstimator: public SumEstimator {
	// estimate a lower bound
	public:
		int depth_max;
		std::vector<boost::dynamic_bitset<>> s;
		std::vector<boost::dynamic_bitset<>> s_prev;

		std::vector<int> result;
		std::vector<int> nr_remain;

		UnionSetDepthEstimator(const std::vector<std::vector<int>>& _graph, int* degree, int _depth_max)
			: SumEstimator(_graph), depth_max(_depth_max)
		{
			s.resize(np, boost::dynamic_bitset<>(np));
			s_prev.resize(np, boost::dynamic_bitset<>(np));
			result.resize(np, 0);
			nr_remain.resize(np);

			REP(i, np) {
				s_prev[i][i] = 1;
				FOR_ITR(fr, graph[i]) {
					s_prev[i][*fr] = 1;
				}
				result[i] += graph[i].size();
				nr_remain[i] = degree[i] - 1 - graph[i].size();
			}
			work();
		}

		void work();

		int estimate(int i) override { return result[i]; }
};
