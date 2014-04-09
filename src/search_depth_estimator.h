/*
 * $File: search_depth_estimator.h
 * $Date: Sun Apr 06 00:26:29 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#pragma once

#include "lib/Timer.h"

#include <cstdlib>
#include <queue>
#include <vector>
#include <algorithm>
#include <cassert>

class SearchDepthEstimator {
	public:
		virtual ~SearchDepthEstimator() {}
		virtual int get_search_depth() = 0;
};

//! ID = Iterative Deepening
class IDDepthEstimator : public SearchDepthEstimator {
	public:
		IDDepthEstimator(
				const std::vector<std::vector<int>> &graph,
				const std::vector<int> &estimated_s,
				const int *degree,  // use int * for historical reasons
				size_t nr_vtx) :  // nr_vtx is number of vertices used for estimating stopping criteria
			graph(graph), estimated_s(estimated_s), degree(degree), np(graph.size()), nr_vtx(nr_vtx) {
				threshold = 0.10;
		}

		virtual int get_search_depth() {
			std::vector<Vertex> vtx(np);
			for (size_t i = 0; i < np; i ++)
				vtx[i].id = i;
			std::random_shuffle(vtx.begin(), vtx.end());
			vtx.resize(nr_vtx);
			{
				GuardedTimer ttt("get_search_depth: init");
				for (auto &v: vtx) {
					v.init(np);
					v.nr_remain = degree[v.id] - 1;
					v.nr_searched = 1;
					v.depth = 0;
					v.q.push(v.id);
					v.hash[v.id] = true;
				}
			}

			long long es_sum = 0,
				 es_sqr_sum = 0;
			for (auto &v: vtx) {
				es_sum += estimated_s[v.id];
				es_sqr_sum += (double)estimated_s[v.id] * estimated_s[v.id];
			}

//            long long nr_search_sum = 0;
//            for (auto &v: vtx)
//                nr_search_sum += degree[v.id];

			int depth;
			{
				GuardedTimer ttt("get_search_depth: iterate");
				for (depth = 1; ; depth ++) {
//                    long long ns = 0;
//                    for (auto &v: vtx) {
//                        advance(v, depth);
//                        ns += v.nr_searched;
//                    }
//                    if (ns / (double)nr_search_sum >= 0.6)
//                        break;


//                    long long error = 0;
//                    for (auto &v: vtx) {
//                        long long diff = estimated_s[v.id] - advance(v, depth);
//                        error += diff * diff;
//                    }
//                    if (error / es_sqr_sum < threshold)
//                        break;

					double ave_err = 0;
					double max_err = 0;
					for (auto &v: vtx) {
						double es = estimated_s[v.id];
						double diff = es - advance(v, depth);
						ave_err += diff / es;
						max_err = std::max(max_err, diff / es);
					}
					ave_err /= vtx.size();
					if (ave_err < 0.1)
						break;
					if (max_err < 0.1)
						break;
				}
			}

			return depth;
		};

	private:
		const std::vector<std::vector<int>> &graph;
		const std::vector<int> &estimated_s;
		const int *degree;
		size_t np;
		size_t nr_vtx;

		double threshold;


		struct Vertex {
			Vertex() :id(0), nr_remain(0), nr_searched(0), depth(0), prev_s(0) {
			}
			void init(size_t np) {
				hash.resize(np);
			}
			int id;
			std::queue<int> q;  // search queue
			int nr_remain;
			int nr_searched;
			int depth;
			int prev_s;

			std::vector<bool> hash;

			int estimate_s() {
				return prev_s + (depth + 1) * nr_remain;
			}
		};

		int advance(Vertex &v, int depth) {
			assert(v.depth + 1 == depth);
			v.depth = depth;
			auto &q = v.q;
			size_t qsize = q.size();
			for (size_t i = 0; i < qsize; i ++) {
				int v0 = q.front(); q.pop();
				for (auto &v1: graph[v0]) {
					if (v.hash[v1])
						continue;
					v.hash[v1] = true;
					q.push(v1);
					v.nr_searched ++;
				}
			}

			v.nr_remain -= q.size();
			v.prev_s += q.size() * depth;
			return v.estimate_s();
		};
};

/**
 * vim: syntax=cpp11 foldmethod=marker
 */
