//File: SumEstimator.h
//Date: Tue Mar 25 14:20:45 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include "lib/debugutils.h"

class SumEstimator {
	public:
		const std::vector<std::vector<int>>& graph;
		int np;

		SumEstimator(const std::vector<std::vector<int>>& _graph)
			: graph(_graph), np((int)graph.size()) { }

		virtual long long estimate(int i) = 0;


		// print error
		virtual void error();

		long long get_exact_s(int i);
};

class RandomChoiceEstimator: public SumEstimator {
	public:
		std::vector<long long> result;
		std::vector<int> samples;
		int* degree;

		// p: percent of point to randomly choose. 0 < p < 1
		RandomChoiceEstimator(const std::vector<std::vector<int>>& _graph, int* _degree, float p)
			: SumEstimator(_graph),
			result(np, 0LL), samples(np), degree(_degree)
		{
			PP(p);
				m_assert(0 < p && p < 1);
				std::iota(samples.begin(), samples.end(), 0);
				std::random_shuffle(samples.begin(), samples.end());
				samples.resize((size_t)np * p);
				work();
		}

		void work();
		long long bfs_all(int source, std::vector<int>&);

		long long estimate(int i) override;

};
