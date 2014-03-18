//File: query4.h
//Date: Tue Mar 18 14:10:55 2014 +0800
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

		void add_query(int k, const std::string& s, int index);

		void work();

		void print_result();		// TODO

		std::vector<std::vector<int>> ans;	// must be resized correctly
};


class Query4Calculator {
	public:
		size_t np;
		const std::vector<std::vector<int>>& friends;
		int k;

		Query4Calculator(const std::vector<std::vector<int>>& _friends, int _k):
			np(_friends.size()), friends(_friends), k(_k) {

			degree = new int[np];
			que = new size_t[np];
			compute_degree();
		}

		std::vector<int> work();


		void compute_degree();

		~Query4Calculator() {
			delete[] degree;
			delete[] que;
		}


	protected:
		int* degree;
		size_t* que;
};
