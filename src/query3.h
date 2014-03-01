//File: query3.h
//Date: Sat Mar 01 13:28:16 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <vector>
#include <string>

struct Query3 {
	int k, hop;
	std::string place;

	Query3(int _k, int _h, const std::string & p):
		k(_k), hop(_h), place(p){}
};

class Query3Handler {
	public:
		void add_query(int k, int h, const std::string& p) {
			queries.push_back(Query3(k, h, p));
			// TODO
		}

		void print_result();		// TODO

	protected:
		std::vector<Query3> queries;
};
