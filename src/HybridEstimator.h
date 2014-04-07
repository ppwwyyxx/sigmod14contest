//File: HybridEstimator.h
//Date: Sun Apr 06 23:56:39 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include "SumEstimator.h"

class HybridEstimator: public SumEstimator {
	public:
		int depth_max;
		int cutcnt;
		std::vector<int> result;
		std::vector<int> nr_remain;

		int* degree;
		std::vector<bool>& noneed;
		int sum_bound;
		const std::vector<int>& approx_result;


		HybridEstimator(const std::vector<std::vector<int>>& _graph, int* _degree,
				int _depth_max,
				std::vector<bool>& noneed, int sum_bound,
				const std::vector<int>& _approx_result);


		void bfs_2_dp_1();

		int estimate(int i) { return result[i]; }
};

