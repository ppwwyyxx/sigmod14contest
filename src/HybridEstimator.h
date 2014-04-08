//File: HybridEstimator.h
//Date: Tue Apr 08 21:48:32 2014 +0800
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


		// bfs 2level, dp 1level
		void bfs_2_dp_1();

		int estimate(int i) { return result[i]; }

		// error between result[] ans approx_result[]
		double average_err() {
			int cnt0 = 0, cnt1 = 0, cnt2 = 0;
			int np = (int)result.size();
			double sum = 0;
			REP(i, np) {
				if (approx_result[i] > 100 && not noneed[i]) {
					double err = (double)(result[i] - approx_result[i]) / approx_result[i];
					if (err < -0.01) {
						cnt0 ++;
						if (err < -0.05) {
							cnt1 ++;
							if (err < -0.1)
								cnt2 ++;
						}
					}
					sum += fabs(err);
				}
			}
			print_debug("Graph np=%d, cnt: %d/%d/%d\n", np, cnt0, cnt1, cnt2);
			return sum / np;
		}
};

