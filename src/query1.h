//File: query1.h
//Date: Sat Mar 01 19:16:13 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <vector>

struct Query1 {
	int p1, p2, x;
	Query1(int _p1, int _p2, int _x):
		p1(_p1), p2(_p2), x(_x){}
};

class Query1Handler {
	public:
		void add_query(int p1, int p2, int x) {
			queries.push_back(Query1(p1, p2, x));
			// TODO
		}

		void work();

		void print_result();		// TODO

	protected:
		std::vector<Query1> queries;
};
