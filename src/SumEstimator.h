//File: SumEstimator.h
//Date: Wed Mar 26 19:07:28 2014 +0000
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include "lib/common.h"
#include "lib/Timer.h"
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
				samples.resize((size_t)((float)np * p));
				work();
		}

		void work();
		int bfs_all(int source, std::vector<int>&);

		int estimate(int i) { return result[i]; }

};

#include <boost/dynamic_bitset.hpp>
class UnionSetDepthEstimator: public SumEstimator {
	// estimate a lower bound
	public:
		int depth_max;

		static __thread std::vector<boost::dynamic_bitset<>> *s_prev;
		static __thread std::vector<boost::dynamic_bitset<>> *s;

		std::vector<int> result;
		std::vector<int> nr_remain;

		UnionSetDepthEstimator(const std::vector<std::vector<int>>& _graph, int* degree, int _depth_max)
			: SumEstimator(_graph), depth_max(_depth_max)
		{
			Timer init;
			size_t alloc = np;
			if (!s){
				s = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
				s_prev = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
			} else if ((int)s->size() < np) {
				delete s; delete s_prev;
				s = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
				s_prev = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
			} else {
				REP(i, s->size()) {
					(*s)[i].reset();
					(*s_prev)[i].reset();
				}
			}
				/*
				 *s = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
				 *s_prev = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
				 */


//			print_debug("Time: %lf\n", init.get_time());
			result.resize((size_t)np, 0);
			nr_remain.resize(np);

			auto& out_scope_ptr = *s_prev;

#pragma omp parallel for schedule(static) num_threads(4)
			REP(i, np) {
				out_scope_ptr[i][i] = 1;
				FOR_ITR(fr, graph[i]) {
					out_scope_ptr[i][*fr] = 1;
				}
				result[i] += (int)graph[i].size();
				nr_remain[i] = degree[i] - 1 - (int)graph[i].size();
			}
			work();
//			delete s; delete s_prev;
		}

		void work();

		int estimate(int i) { return result[i]; }
};
