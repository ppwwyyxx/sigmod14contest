//File: query4.h
//Date: Wed Mar 26 16:49:20 2014 +0000
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include "lib/hash_lib.h"
#include "data.h"

std::vector<PersonInForum> get_tag_persons(const std::string& s);

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
};


class Query4Calculator {
	public:
		size_t np;
		const std::vector<std::vector<int>>& friends;
		int k;

		int contract_dist;
		int contract_nr_vtx;

		Query4Calculator(const std::vector<std::vector<int>>& _friends, int _k):
			np(_friends.size()), friends(_friends), k(_k) {

			degree = new int[np];
			que = new size_t[np];
			compute_degree();

			contract_dist = (int)np;
			contract_nr_vtx = 3;
		}

		std::vector<int> work();



		~Query4Calculator() {
			delete[] degree;
			delete[] que;
		}


	protected:
		void compute_degree();
		void contract_graph();

		//! estimate s using contracted graph
		int estimate_s_using_cgraph(int source);

		int get_extact_s(int source);


		std::vector<std::vector<int>> cgraph;
		std::vector<int> cgraph_vtx_weight;

		std::vector<int> vtx_old2new;
		std::vector<std::vector<int>> vtx_new2old;

		int* degree;
		size_t* que;

		static int dummy;

		std::vector<int> cgraph_estimated_s_inner;
		std::vector<int> cgraph_estimated_s_outter;
		std::vector<int> cgraph_estimated_s;

		std::vector<int> estimated_s;
		std::vector<int> pre_estimated_s;
		double get_centrality_by_vtx_and_s(int v, int s);
	 	int estimate_s_limit_depth(int source, int depth_max);
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

/*
 * vim: syntax=cpp11.doxygen foldmethod=marker
 */
