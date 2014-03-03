//File: query4.h
//Date: Mon Mar 03 10:40:04 2014 +0800
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
		void add_query(int k, const std::string& s);

		void work();

		void print_result();		// TODO

	protected:
		std::vector<Query4> queries;
		std::vector<std::vector<int> > ans;
};
