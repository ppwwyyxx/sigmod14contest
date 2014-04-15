//File: SumEstimator.h
//Date: Tue Apr 15 15:44:00 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include <boost/dynamic_bitset.hpp>
#include "lib/common.h"
#include "lib/bitset.h"
#include "lib/Timer.h"
#include "lib/debugutils.h"
#include "lib/bitset.h"

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
		RandomChoiceEstimator(const std::vector<std::vector<int>>& _graph, int* _degree, double p)
			: SumEstimator(_graph),
			result(np, 0), degree(_degree)
		{
				m_assert(0 < p && p < 1);
				REP(i, np)
					if (degree[i] > 100)
						samples.push_back(i);
//				std::iota(samples.begin(), samples.end(), 0);
				std::random_shuffle(samples.begin(), samples.end());
				samples.resize((size_t)((double)np * p));		// TODO can be better than this
				work();
		}

		void work();
		int bfs_all(int source, std::vector<int>*);

		int estimate(int i) { return result[i]; }

};

class SSEUnionSetEstimator: public SumEstimator {
	// estimate a lower bound
	public:
		int depth_max;

		std::vector<Bitset> s, s_prev;

		std::vector<int> result;
		std::vector<int> nr_remain;

		SSEUnionSetEstimator(const std::vector<std::vector<int>>& _graph, int* degree, int _depth_max);
		void work();

		int estimate(int i) { return result[i]; }
};

class LimitDepthEstimator: public SumEstimator {
	public:
		int* degree;
		int depth_max;

		LimitDepthEstimator(const std::vector<std::vector<int>>& _graph, int* _degree, int depth_max);

		int estimate(int i);
};
