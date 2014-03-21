/*
 * $File: query4_v3.cc
 * $Date: Fri Mar 21 20:43:10 2014 +0800
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

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
#include <cassert>

using namespace std;

vector<PersonInForum> get_tag_persons(const string& s) {
	int tagid = Data::tagid[s];
	auto& forums = Data::tag_forums[tagid];
	vector<PersonInForum> persons;

	FOR_ITR(itr, forums) {
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

namespace {
	struct HeapEle {
		int vtx;
		double centrality;

		HeapEle() {}
		HeapEle(int _vtx, double _centrality) :
			vtx(_vtx), centrality(_centrality) {
			}
		bool operator < (const HeapEle& r) const
		{
			if (centrality == r.centrality)
				return vtx > r.vtx;
			return centrality < r.centrality;
		}
	};
}

void Query4Calculator::compute_degree() {
	for (size_t i = 0; i < np; i ++)
		degree[i] = -1;

	for (size_t i = 0; i < np; i ++) {
		if (degree[i] != -1)
			continue;
		degree[i] = 1;
		int qh = 0, qt = 1;
		que[qh] = i;
		while (qh != qt) {
			size_t v0 = que[qh ++];
			FOR_ITR(itr, friends[v0]) {
				auto& v1 = *itr;
				if (degree[v1] != -1)
					continue;

				degree[v1] = 1;
				que[qt ++] = v1;
			}
		}
		for (int j = 0; j < qt; j ++)
			degree[que[j]] = qt;
	}
}

void Query4Calculator::contract_graph() {
	int contract_dist = 2;
	std::vector<int> dist(np);
	vtx_old2new.resize(np, -1);
	std::vector<std::vector<int>> &nodes = vtx_new2old;

	std::vector<int> qbuf;
	int cur_vtx = 0;
	for (int source = 0; source < (int)np; source ++) {
		if (vtx_old2new[source] != -1)
			continue;

		// bfs
		int qh = 0, qt = 1;
		nodes.push_back(std::vector<int>());
		std::vector<int> &q = nodes.back();
		q.resize(1);
		q[qh] = source;
		dist[source] = 0;
		vtx_old2new[source] = cur_vtx;

		int nr_remain = degree[source];
		long long s_inner = 0, s_outter = 0;
		for (int depth = 0; depth < contract_dist; depth ++) {
			int qsize = qt - qh;
			s_inner += qsize * depth;
			nr_remain -= qsize;
			int d1 = depth + 1;
			if (qsize == 0)
				break;
			for (int i = 0; i < qsize; i ++) {
				int v0 = q[qh ++];
				for (auto &v1: friends[v0]) {
					if (vtx_old2new[v1] != -1)
						continue;
					vtx_old2new[v1] = cur_vtx;
					dist[v1] = d1;
					qt ++; q.push_back(v1);
				}
			}
		}

		s_inner += (qt - qh) * contract_dist;
		nr_remain -= (qt - qh);
		s_outter = nr_remain * (contract_dist + 1);

		cgraph_estimated_s_inner.push_back(s_inner);
		cgraph_estimated_s_outter.push_back(s_outter);

		cur_vtx ++;
	}

	int nr_vtx = cur_vtx;
	cgraph.resize(nr_vtx);
	std::vector<int> hash(np, -1);
	for (int i = 0; i < nr_vtx; i ++) {
		auto &vtx = nodes[i];
		for (auto &v0: vtx) {
			for (auto &v1: friends[v0]) {
				int u = vtx_old2new[v1];
				if (hash[u] == i)
					continue;
				hash[u] = i;
				cgraph[i].push_back(u);
			}
		}
	}
}

long long Query4Calculator::get_extact_s(int source) {
	std::vector<bool> hash(np);
	std::queue<int> q;
	hash[source] = true;
	q.push(source);
	long long s = 0;
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
			s += depth;
			for (auto &v1: friends[v0]) {
				if (hash[v1])
					continue;
				hash[v1] = true;
				q.push(v1);
			}
		}
	}
	return s;
}


long long Query4Calculator::estimate_s_using_cgraph(int source) {
//    int nr_vtx = (int)cgraph.size();
	std::vector<bool> hash(cgraph.size());

	std::queue<int> q;
	q.push(source);
	hash[source] = 1;
	long long s = 0;
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
			s += depth * vtx_new2old[v0].size();
			for (auto &v1: cgraph[v0]) {  // TODO: traversal of last layer is not needed
				if (hash[v1])
					continue;
				hash[v1] = true;
				q.push(v1);
			}
		}
	}
	return s;
}

vector<int> Query4Calculator::work() {
	contract_graph();

	// estimate s using contracted graph
	int nr_vtx = (int)cgraph.size();
	cgraph_estimated_s.resize(nr_vtx);

	for (int i = 0; i < nr_vtx; i ++) {
		cgraph_estimated_s[i] = estimate_s_using_cgraph(i);
	}

	// build heap
	vector<HeapEle> heap_ele_buf(np);
	for (int i = 0; i < (int)np; i ++) {
		double centrality = get_centrality_by_vtx_and_s(i, cgraph_estimated_s[vtx_old2new[i]]);
		heap_ele_buf[i] = HeapEle(i, centrality);
	}

	priority_queue<HeapEle> q(heap_ele_buf.begin(), heap_ele_buf.end());

	// iterate
	vector<int> ans;

	double last_centrality = 1e100;
	int last_vtx = -1;
	int cnt = 0;
	while (!q.empty()) {
		auto he = q.top(); q.pop();
		int vtx = he.vtx;
		double centrality = he.centrality;
		if (centrality == last_centrality && vtx == last_vtx) {
			ans.emplace_back(vtx);
			if ((int)ans.size() == k)
				break;
		} else {
			cnt ++;
			q.push(HeapEle(vtx, get_centrality_by_vtx_and_s(vtx, get_extact_s(vtx))));
		}

		last_centrality = centrality;
		last_vtx = vtx;
	}

	return move(ans);

}

double Query4Calculator::get_centrality_by_vtx_and_s(int v, long long s) {
	return ::sqr(degree[v] - 1.0) / (double)s / ((int)np - 1);
}


void Query4Handler::add_query(int k, const string& s, int index) {
	TotalTimer timer("Q4");
	// build graph
	vector<PersonInForum> persons = get_tag_persons(s);
	size_t np = persons.size();
	vector<vector<int>> friends(np);

	{
		lock_guard<mutex> lg(mt_friends_data_changing);
		friends_data_reader ++;
	}
	{
		GuardedTimer timer("omp1");
#pragma omp parallel for schedule(static) num_threads(4)
		REP(i, np) {
			auto& fs = Data::friends[persons[i]];
			FOR_ITR(itr, fs) {
				auto lb_itr = lower_bound(persons.begin(), persons.end(), itr->pid);
				if (lb_itr != persons.end() and *lb_itr == itr->pid) {
					friends[i].push_back((int)distance(persons.begin(), lb_itr));
				}
			}
		}
	}
	friends_data_reader --;
	cv_friends_data_changing.notify_one();
	// finish building graph

	Query4Calculator worker(friends, k);
	auto now_ans = worker.work();
	FOR_ITR(itr, now_ans) {
		*itr = persons[*itr];
	}
	ans[index] = move(now_ans);
}


void Query4Handler::work() { }
void Query4Handler::print_result() {
	FOR_ITR(itr, ans) {
		vector<int>& line = *itr;
		for (size_t k = 0; k < line.size(); k ++) {
			if (k) printf(" ");
			printf("%d", line[k]);
		}
		printf("\n");
	}
}

/*
 * vim: syntax=cpp11.doxygen foldmethod=marker
 */
