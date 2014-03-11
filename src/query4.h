//File: query4.h
//Date: Wed Mar 12 11:16:58 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <vector>
#include <mutex>
#include <string>

struct Query4 {
	int k;
	std::string tag;
	Query4(int _k, const std::string& s):
		k(_k), tag(s){}
};

class Query4Handler {
	public:
		std::mutex mt_work_done;

		void add_query(int k, const std::string& s);

		void work();

		void print_result();		// TODO

	protected:
		std::vector<std::vector<int> > ans;
};
