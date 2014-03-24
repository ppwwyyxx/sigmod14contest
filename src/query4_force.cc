//File: query4_force.cc
//Date: Mon Mar 24 23:53:38 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "query4.h"
#include "data.h"
#include "lib/common.h"
#include "lib/Timer.h"
#include "lib/utils.h"
#include "lib/hash_lib.h"
#include <omp.h>
#include <queue>
#include <algorithm>
#include <set>

using namespace std;
using namespace std::tr1;

// TODO:
// cache the Shortest Path caculated before, see if the node in the path
// is in the forum


namespace {
	int ** SP_matrix;
	int * degree;
	size_t np;
	vector<vector<int>> friends;
}

void calculate_SP(const vector<PersonInForum>& ps) {		// TOO SLOW
	unordered_map<int, int> persons;  // <pid, index>
//	persons.reserve(np);
	REP(i, np) persons[ps[i]] = (int)i;

#pragma omp parallel for schedule(dynamic)
	REP(i, np) {
		deque<size_t> q; q.push_back(i);
		int depth = 0;
		while (size_t qsize = q.size()) {
			depth ++;
			REP(_k, qsize) {
				auto top = q.front(); q.pop_front();
				auto& fs = friends[top];
				for (auto itr = fs.begin(); itr != fs.end(); itr ++) {
					// found a friend in the forum
					size_t index = *itr;
					if (SP_matrix[i][index] != -1 or i == index) continue;
					degree[i] ++;
					SP_matrix[i][index] = depth;
					q.push_back(index);
				}
			}
		}
	}
}

namespace {
	struct HeapEle {
		int pid;
		double central;

		HeapEle(int _pid, double _central) :
			pid(_pid), central(_central) {}

		bool operator < (const HeapEle& r) const
		{ return central < r.central or (central == r.central and pid > r.pid); }
	};
}

double cal_central(size_t k) {
	double up = ::sqr(degree[k] - 1);
	int s = 0;
	REP(i, np) {
		if (i == k) continue;
		int t = SP_matrix[k][i];
		if (t > 0) s += t;
	}
	return up / s / (double)(np - 1);
}

void Query4Handler::add_query(int k, const string& s) {
	Timer timer;
	vector<PersonInForum> persons = get_tag_persons(s);
	np = persons.size();
	friends.resize(np);
	REP(i, np) {
		friends[i].clear();
		auto& fs = Data::friends[persons[i]];
		for (auto itr = fs.begin(); itr != fs.end(); itr ++) {
			auto lb_itr = lower_bound(persons.begin(), persons.end(), itr->pid);
			if (*lb_itr == itr->pid) {
				friends[i].push_back((int)distance(persons.begin(), lb_itr));
			}
		}
	}

	degree = new int[np];
	REP(i, np) degree[i] = 1;
	SP_matrix = new int*[np];
	REP(i, np) {
		SP_matrix[i] = new int[np];
		REP(j, np) SP_matrix[i][j] = -1;
	}

	//print_debug("Before cal SP %lf\n", timer.get_time());
	ucalculate_SP(persons);
	//print_debug("After cal SP %lf\n", timer.get_time());

	priority_queue<HeapEle> q;		// need a fixed-size queue later
	REP(i, np) {
		double central = cal_central(i);
		q.push(HeapEle(persons[i], central));
	}

	vector<int> now_ans;
	REP(i, k) {
		now_ans.push_back(q.top().pid);
		q.pop();
	}
	ans.push_back(now_ans);

	free_2d<int>(SP_matrix, (int)np);
	delete[] degree;
}

void Query4Handler::work() {

}

void Query4Handler::print_result() {
	for (auto itr = ans.begin(); itr != ans.end(); itr ++) {
		vector<int>& line = *itr;
		for (size_t k = 0; k < line.size(); k ++) {
			if (k) printf(" ");
			printf("%d", line[k]);
		}
		printf("\n");
	}
}
