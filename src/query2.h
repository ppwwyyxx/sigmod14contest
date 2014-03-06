//File: query2.h

#pragma once
#include <vector>

struct Query2 {
	int k, d, qid;
	Query2(int _k, int _d, int _qid):
		k(_k), d(_d), qid(_qid){}
    bool operator <(const Query2 &b) const {
        return d > b.d;
    }
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
