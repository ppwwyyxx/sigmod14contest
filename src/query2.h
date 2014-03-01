//File: query2.h
//Date: Sat Mar 01 19:16:20 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#pragma once
#include <vector>

struct Query2 {
	int k, d;
	Query2(int _k, int _d):
		k(_k), d(_d){}
};

class Query2Handler {
	public:
		void add_query(int k, int d) {
			queries.push_back(Query2(k, d));
			// TODO
		}

		void work();

		void print_result();		// TODO

	protected:
		std::vector<Query2> queries;
};
