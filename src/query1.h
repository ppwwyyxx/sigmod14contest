//File: query1.h
//Date: Sun Mar 02 21:31:59 2014 +0800

#pragma once
#include <vector>

/*
 *struct Query1 {
 *    int p1, p2, x;
 *    Query1(int _p1, int _p2, int _x):
 *        p1(_p1), p2(_p2), x(_x){}
 *};
 */

class Query1Handler {
	public:
		void add_query(int p1, int p2, int x);

		void work();

		void print_result();		// TODO

	protected:
		std::vector<int> ans;
};
