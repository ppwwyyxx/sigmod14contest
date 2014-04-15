//File: query4.h
//Date: Tue Apr 15 16:59:16 2014 +0000
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include "lib/utils.h"
#include "lib/Timer.h"
#include "lib/hash_lib.h"
#include "lib/finish_time_continuation.h"
#include "data.h"

struct Query4 {
	int k;
	std::string tag;
	Query4(int _k, const std::string& s):
		k(_k), tag(s){}
};

class Query4Handler {
	public:
		void add_query(int k, const std::string& s, int index);

		void work();

		void print_result();		// TODO

		std::vector<std::vector<int>> ans;	// must be resized correctly

		std::shared_ptr<FinishTimeContinuation> continuation;
};


class Query4Calculator {
	public:
		size_t np;
		const std::vector<std::vector<int>>& friends;
		int k;

		int contract_dist;
		int contract_nr_vtx;

		Query4Calculator(const std::vector<std::vector<int>>& _friends,
				int _k):
			np(_friends.size()), friends(_friends), k(_k) {

				degree = new int[np];
				estimated_s.resize(np);
				exact_s.resize(np, -1);

				compute_degree();
			}

		std::vector<int> work();


		~Query4Calculator() {
			delete[] degree;
		}


	protected:
		int* degree;
		std::vector<int> estimated_s;
		std::vector<int> exact_s;

		void compute_degree() {
			std::vector<size_t> que(np);
			REP(i, np)
				degree[i] = -1;

			for (size_t i = 0; i < np; i ++) {
				if (degree[i] != -1)
					continue;
				degree[i] = 1;
				int qh = 0, qt = 1;
				que[qh] = i;
				while (qh != qt) {
					size_t v0 = que[qh ++];
					FOR_ITR(itr, friends[v0]) {
						auto& v1 = *itr;
						if (degree[v1] != -1)
							continue;

						degree[v1] = 1;
						que[qt ++] = v1;
					}
				}
				REP(j, qt)
					degree[que[j]] = qt;
			}
		}

		int get_exact_s(int source) {
			if (exact_s[source] != -1)
				return exact_s[source];

			std::vector<bool> hash(np);
			std::queue<int> q;
			hash[source] = true;
			q.push(source);
			int s = 0;
			for (int depth = 0; !q.empty(); depth ++) {
				int qsize = (int)q.size();
				for (int i = 0; i < qsize; i ++) {
					int v0 = q.front(); q.pop();
					s += depth;
					FOR_ITR(v1, friends[v0]) {
						if (hash[*v1])
							continue;
						hash[*v1] = true;
						q.push(*v1);
					}
				}
			}
			exact_s[source] = s;
			return s;
		}

		double get_centrality_by_vtx_and_s(int v, int s) {
			if (s == 0)
				return 0;
			double ret = ::sqr(degree[v] - 1.0) / (double)s / ((int)np - 1);
			return ret;
		}

		int estimate_s_limit_depth(int source, int depth_max);
		int estimate_s_limit_depth_cut_upper(int source, int depth_max, double upper);
		void bfs_diameter(const std::vector<std::vector<int>> &g, int source, int &farthest_vtx,
				int &dist_max, std::vector<bool> &hash);


		void estimate_all_s_using_delta_bfs(int est_dist_max);



		//! change_vtx is a vector of pair (vtx, dist)
		//! return number of vertex traversed
		int bfs(const std::vector<std::vector<int>> &graph,
				int source, int base_dist, int est_dist_max, std::vector<int> &dist,
				std::vector<int> &dist_count,
				std::vector<std::pair<int, int>> *changed_vtx = NULL);

		int get_s_by_dist_count(int vtx, const std::vector<int> &dist_count, int begin, int finish);


		void estimate_all_s_using_delta_bfs_and_schedule(int est_dist_max);

		struct ScheduleNode {
			int vtx;
			ScheduleNode(int vtx) : vtx(vtx) {}
			std::vector<std::shared_ptr<ScheduleNode>> children;
		};


		//! A scheduler returns a ScheduleNode, which is the
		typedef std::function<std::shared_ptr<ScheduleNode>(const std::vector<std::vector<int>> &)> scheduler_t;

		static std::shared_ptr<ScheduleNode> scheduler_bfs(const std::vector<std::vector<int>> &graph);

		typedef std::priority_queue<std::pair<double, int>> TopKList;

		void process_est(std::shared_ptr<ScheduleNode> &node,
				int base_dist, std::vector<int> &dist, std::vector<int> &dist_count,
				int est_dist_max);

		void process_est_rollback(std::vector<int> &dist,
				std::vector<int> &dist_count, const std::vector<std::pair<int, int>> &changed_vtx);
};


inline std::string get_graph_name(int np, int cnt, int k, int est_dist)
{ return string_format("%d-%d-%d-%d", np, cnt, k, est_dist); }
