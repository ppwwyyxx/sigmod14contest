//File: query4_force.cc
//Date: Mon Mar 03 18:08:38 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "query4.h"
#include "data.h"
#include "lib/common.h"
#include "lib/utils.h"
#ifdef __linux__
#include <tr1/unordered_set>
#else
#include <unordered_set>
#endif
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
}

vector<PersonInForum> get_tag_persons(const string& s) {
	int tagid = Data::tagid[s];
	auto& forums = Data::tag_forums[tagid];
	vector<PersonInForum> persons;

	for (auto itr = forums.begin(); itr != forums.end(); itr ++) {
		set<PersonInForum>& persons_in_forum = (*itr)->persons;
		vector<PersonInForum> tmp; tmp.swap(persons);
		persons.resize(tmp.size() + persons_in_forum.size());
		vector<PersonInForum>::iterator ret_end = set_union(
				persons_in_forum.begin(), persons_in_forum.end(),
				tmp.begin(), tmp.end(), persons.begin());
		persons.resize(std::distance(persons.begin(), ret_end));
	}
	return persons;
}

void calculate_SP(const vector<PersonInForum>& ps) {		// TOO SLOW
	unordered_map<int, int> persons;  // <pid, index>
//	persons.reserve(np);
	REP(i, np) persons[ps[i]] = (int)i;

#pragma omp parallel for schedule(dynamic)
	REP(i, np) {
		int nowpid = ps[i];
		deque<int> q; q.push_back(nowpid);
		int depth = 0;
		while (size_t qsize = q.size()) {
			depth ++;
			REP(_k, qsize) {
				int top = q.front(); q.pop_front();
				auto& friends = Data::friends[top];
				for (auto itr = friends.begin(); itr != friends.end(); itr ++) {
					int pid = itr->pid;
					auto rst = persons.find(pid);
					if (rst != persons.end()) {
						// found a friend in the forum
						size_t index = rst->second;
						if (SP_matrix[i][index] != -1 or i == index) continue;
						degree[i] ++;
						SP_matrix[i][index] = depth;
						q.push_back(itr->pid);
					}
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
	vector<PersonInForum> persons = get_tag_persons(s);
	np = persons.size();
	print_debug("Nperson under this tag: %lu\n", np);
	degree = new int[np];
	REP(i, np) degree[i] = 1;
	SP_matrix = new int*[np];
	REP(i, np) {
		SP_matrix[i] = new int[np];
		REP(j, np) SP_matrix[i][j] = -1;
	}

	calculate_SP(persons);

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
