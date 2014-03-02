//File: query2.h

#pragma once
#include <vector>

struct Query2 {
	int k, d;
	Query2(int _k, int _d):
		k(_k), d(_d){}
};

class Query2Handler {
	public:
		void add_query(int k, int d);

		void work();

		void print_result();

	protected:
		std::vector<Query2> queries;
        std::vector< std::vector<int> > ans;
};
