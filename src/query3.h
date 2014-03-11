//File: query3.h
//Date: Wed Mar 12 11:24:16 2014 +0800

#pragma once

#include <vector>
#include <string>
#include <mutex>
#include "data.h"

struct Query3 {
	int k, hop;
	std::string place;

	Query3(int _k, int _h, const std::string & p):
		k(_k), hop(_h), place(p){}
};

struct Answer3 {
	int com_interest, p1, p2;

	Answer3(int _c = 0, int _p1 = 0, int _p2 = 0):
		com_interest(_c), p1(_p1), p2(_p2)
	{
		if (p1 > p2)
		{
			p1 ^= p2;
			p2 ^= p1;
			p1 ^= p2;
		}
	}

	bool operator<(const Answer3& r) const
	{
		if (com_interest != r.com_interest)
			return com_interest > r.com_interest;
		if (p1 != r.p1)
			return p1 < r.p1;
		return p2 < r.p2;
	}
};

class Query3Handler {
	public:
		std::mutex mt_work_done;
		void add_query(int k, int h, const std::string& p);

		void work();

		void print_result();		// TODO

		void bfs(int, int, int);		// for version2
		void bfs(int, int);				// for force

	protected:
		PersonSet pset;
		std::set<int> pinplace;
		std::vector<Answer3> answers;
		std::vector<std::vector<Answer3> > global_answer;
};
