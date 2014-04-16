//File: HybridEstimator.h
//Date: Wed Apr 16 02:43:41 2014 +0000
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <cmath>
#include "SumEstimator.h"

class HybridEstimator: public SumEstimator {
	public:
		int cutcnt;
		std::vector<int> result;
		std::vector<int> nr_remain;
		int depth;

		int* degree;
		std::vector<bool>& noneed;
		int sum_bound;
		const std::vector<int>& approx_result;


		HybridEstimator(const std::vector<std::vector<int>>& _graph, int* _degree,
				std::vector<bool>& noneed, int sum_bound,
				const std::vector<int>& _approx_result);


		virtual void init();


		// bfs 2level, dp 1level
		void bfs_2_dp_1();
		void bfs_3_dp_1();
		void bfs_2_dp_more(bool use_4 = false);
		void bfs_depth(int d);

		int d3_estimate(int source, int d);

		int estimate(int i) { return result[i]; }

		// error between result[] ans approx_result[]
		bool good_err(const std::vector<int>& now_result) {
			if (approx_result.size() == 0) {		// small data not using estimation!
				return 0.0;
			}

			int cnt1 = 0, cnt2 = 0;
			int np = (int)now_result.size();
			double sum = 0;
			int n_err = 0;
			REP(i, np) {
				if (approx_result[i] > 100 && not noneed[i]) {
					double err = (double)(now_result[i] - approx_result[i]) / approx_result[i];
					if (err < -0.05) {
						cnt1 ++;
						if (err < -0.1)
							cnt2 ++;
						if (err < -0.5) {
							print_debug("Now bound: %d, approx: %d\n", now_result[i], approx_result[i]);
						}
					}
					sum += fabs(err);
					n_err ++;
				}
			}
			sum /= n_err;
			print_debug("Graph np=%d, cnt: %d/%d, err: %.4lf\n", np, cnt1, cnt2, sum);
			if (sum > 0.25 or cnt2 > 3000) {
//                fprintf(stderr, "sum: %f cnt2: %d\n", sum, cnt2);
				return false;
			}
			return true;
		}
};

/*
 *
 *class VectorMergeHybridEstimator: public HybridEstimator{
 *    public:
 *        VectorMergeHybridEstimator(const std::vector<std::vector<int>>& _graph, int* _degree,
 *                std::vector<bool>& noneed, int sum_bound,
 *                const std::vector<int>& _approx_result);
 *
 *        virtual void init();
 *        void vector_dp();
 *
 *        void get_nr_le(int v0, int depth);
 *        int estimate_s_limit_depth(int source, int depth_max);
 *
 *        int unique_merge(const std::vector<int> &a, const std::vector<int> &b, std::vector<int> &c);
 *
 *        int estimate_merge_cost(const std::vector<int> &sizes);
 *        long long quick_unique_merge(const std::vector<std::vector<int> *> &items, std::vector<int> &out);
 *};
 */
