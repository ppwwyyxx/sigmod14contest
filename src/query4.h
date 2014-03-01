//File: query4.h
//Date: Sat Mar 01 13:28:42 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <vector>
#include <string>

struct Query4 {
	int k;
	std::string tag;
	Query4(int _k, const std::string& s):
		k(_k), tag(s){}
};

class Query4Handler {
	public:
		void add_query(int k, const std::string& s) {
			queries.push_back(Query4(k, s));
			// TODO
		}

		void print_result();		// TODO

	protected:
		std::vector<Query4> queries;
};
