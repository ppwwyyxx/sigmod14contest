//File: query4.h
//Date: Tue Mar 18 18:07:02 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <vector>
#include <mutex>
#include <string>
#include "lib/hash_lib.h"

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

			contract_dist = np;
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
		long long estimate_s_using_cgraph(int source);

		long long get_extact_s(int source);


		std::vector<std::vector<int>> cgraph;
		std::vector<int> cgraph_vtx_weight;

		std::vector<int> vtx_old2new;
		std::vector<std::vector<int>> vtx_new2old;

		int* degree;
		size_t* que;

		static int dummy;

		std::vector<long long> cgraph_estimated_s_inner;
		std::vector<long long> cgraph_estimated_s_outter;
		std::vector<long long> cgraph_estimated_s;

		std::vector<long long> estimated_s;
		double get_centrality_by_vtx_and_s(int v, long long s);
		long long estimate_s_limit_depth(int source, int depth_max);
};
