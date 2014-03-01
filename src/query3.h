//File: query3.h
//Date: Sat Mar 01 19:25:44 2014 +0800

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
		void add_query(int k, int h, const std::string& p);

		void work();

		void print_result();		// TODO

	protected:
		std::vector<Query3> queries;
};
