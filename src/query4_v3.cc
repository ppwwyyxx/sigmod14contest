/*
 * $File: query4_v3.cc
 * $Date: Thu Apr 03 20:21:28 2014 +0000
 * $Date: Thu Apr 03 20:21:28 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#include "query4.h"
#include "data.h"
#include "lib/common.h"
#include "lib/Timer.h"
#include "lib/utils.h"
#include "SumEstimator.h"
#include "lib/hash_lib.h"
#include <omp.h>
#include <queue>
#include <algorithm>
#include <set>
#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

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

void Query4Calculator::contract_graph() {
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
		int s_inner = 0, s_outter = 0;
		for (int depth = 0; depth < contract_dist && qt != contract_nr_vtx; depth ++) {
			int qsize = qt - qh;
			s_inner += qsize * depth;
			nr_remain -= qsize;
			int d1 = depth + 1;
			if (qsize == 0)
				break;
			for (int i = 0; i < qsize && qt != contract_nr_vtx; i ++) {
				int v0 = q[qh ++];
				for (auto &v1: friends[v0]) {
					if (vtx_old2new[v1] != -1)
						continue;
					vtx_old2new[v1] = cur_vtx;
					dist[v1] = d1;
					qt ++; q.push_back(v1);
					if (qt == contract_nr_vtx)
						break;
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




int Query4Calculator::estimate_s_using_cgraph(int source) {
//    int nr_vtx = (int)cgraph.size();
	std::vector<bool> hash(cgraph.size());

	std::queue<int> q;
	q.push(source);
	hash[source] = 1;
	int s = 0;
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

void Query4Calculator::bfs_diameter(const std::vector<vector<int>> &g, int source, int &farthest_vtx,
		int &dist_max, vector<bool> &hash) {
	queue<int> q;
	q.push(source);
	hash[source] = true;
	farthest_vtx = source;
	dist_max = 0;
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
			for (auto &v1: g[v0]) {
				if (hash[v1])
					continue;
				hash[v1] = true;
				farthest_vtx = v1;
				dist_max = depth + 1;
				q.push(v1);
			}
		}
	}
}

vector<int> Query4Calculator::work() {

	Timer timer;

	int test_nvtx = -1;

	if ((int)np == test_nvtx) {
		{
			GuardedTimer timer("contract graph");
			contract_graph();
		}


		// estimate s using contracted graph
		int nr_vtx = (int)cgraph.size();
		fprintf(stderr, "contract_graph: nr_vtx: %d/%d\n", nr_vtx, (int)np);
		cgraph_estimated_s.resize(nr_vtx);

		{
			GuardedTimer timer("estimate s using contract graph");
			for (int i = 0; i < nr_vtx; i ++) {
				cgraph_estimated_s[i] = estimate_s_using_cgraph(i);
			}
		}
	}

	int est_dist_max = 2;  // TODO: this parameter needs tune
//    if (np > 2400 and Data::nperson > 11000) est_dist_max = 3;
//    if (np == 2567 && k == 4) est_dist_max = 4;
	int diameter = -1;
	std::vector<bool> diameter_hash(np), h2(np);
	for (int i = 0; i < (int)np; i ++) {
		if (diameter_hash[i])
			continue;
		int fv, d;
		bfs_diameter(friends, i, fv, d, h2);
		bfs_diameter(friends, fv, fv, d, diameter_hash);
		if (d > diameter)
			diameter = d;
	}
//    assert(diameter != -1);

	est_dist_max = max(2, (int)floor(log((double)diameter) / log(2.0) + 0.5));
//    if (np == 8544 && k == 3)
//        est_dist_max = 3;


	vector<HeapEle> heap_ele_buf(np);
	estimated_s.resize(np);

	// ESTIMATE LOWER_BOUND OF S {
//    estimate_all_s_using_delta_bfs(est_dist_max);

	//estimate_all_s_using_delta_bfs_and_schedule(est_dist_max);

	LimitDepthEstimator estimator(friends, degree, est_dist_max);
#pragma omp parallel for schedule(static) num_threads(4)
    for (int i = 0; i < (int)np; i ++) estimated_s[i] = estimator.estimate(i);

	//  }

	for (int i = 0; i < (int)np; i ++) {
		double centrality = get_centrality_by_vtx_and_s(i, estimated_s[i]);
		heap_ele_buf[i] = HeapEle(i, centrality);
	}

	double time_phase1 = timer.get_time();
	priority_queue<HeapEle> q(heap_ele_buf.begin(), heap_ele_buf.end());

	// iterate
	vector<int> ans;

	int cnt = 0;
	{
		GuardedTimer timer("iterate");
		double last_centrality = 1e100;
		int last_vtx = -1;
		while (!q.empty()) {
			auto he = q.top(); q.pop();
			int vtx = he.vtx;
			double centrality = he.centrality;
			assert(centrality <= last_centrality);
			if (centrality == last_centrality && vtx == last_vtx) {
				ans.emplace_back(vtx);
				//                if (np == 3453 && k == 1) {
				//                    cerr << "vtx: " << he.centrality << endl;
				//                }
				if ((int)ans.size() == k)
					break;
			} else {
				cnt ++;
				int s = get_exact_s(vtx);
				// int es = estimated_s[vtx];
				double new_centrality = get_centrality_by_vtx_and_s(vtx, s);
				//                if (np == 3453 && k == 1) {
				//                    cerr << "failed vtx: " << vtx << endl;
				//                    cerr << "deg: " << degree[vtx] << endl;
				//                    cerr << "s: " << s << " " << es << endl;
				//                    cerr << "cent: " << centrality << " " << he.centrality << endl;
				//                }
				q.push(HeapEle(vtx, new_centrality));
			}

				last_centrality = centrality;
				last_vtx = vtx;
		}

#if 0

		if ((np == 1420 && cnt == 1279 && k == 4) ||
				(np == 13097 && cnt == 4 && k == 2) ||
				(np == 708 && cnt == 20 && k == 6) ||
				(np == 2567 && cnt == 2431 && k == 4)) {
			auto name = get_graph_name(np, cnt, k, est_dist_max);
			output_dot_graph(name + ".dot", friends);
			output_tgf_graph(name + ".tgf", friends);
		}
#endif
	}

	auto time = timer.get_time();
	print_debug("total: %f secs\n", time);
	if (time > 0.1)
		fprintf(stderr, "cnt: %f-%f %lu/%d/%d/%d/%d\n", time_phase1, time, np, cnt, k, (int)diameter, (int)est_dist_max);
	return move(ans);
}


int Query4Calculator::get_s_by_dist_count(int vtx, const std::vector<int> &dist_count, int begin, int finish) {
	int s = 0;
	int nr_remain = degree[vtx];
	for (int i = begin; i <= finish; i ++) {
		s += (i - begin) * dist_count[i];
		nr_remain -= dist_count[i];
	}

	s += (finish - begin + 1) * (int)nr_remain;
	return s;
}


std::shared_ptr<Query4Calculator::ScheduleNode> Query4Calculator::scheduler_bfs(
		const std::vector<std::vector<int>> &graph) {


	// Step one: choose a source
	int source = 0;


	// Step two: build a shortest path tree
	std::vector<std::shared_ptr<ScheduleNode>> tree(graph.size());
	for (int i = 0; i < (int)graph.size(); i ++)
		tree[i] = std::move(make_shared<ScheduleNode>(i));

	std::vector<bool> hash(graph.size());
	std::queue<int> q;
	hash[source] = true;
	q.push(source);
//    int s = 0;
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
//            s += depth;
			for (auto &v1: graph[v0]) {
				if (hash[v1])
					continue;
				hash[v1] = true;
				tree[v0]->children.push_back(tree[v1]);
				q.push(v1);
			}
		}
	}

	auto ret = make_shared<ScheduleNode>(-1);
	ret->children.push_back(tree[source]);
	return ret;
}


void Query4Calculator::estimate_all_s_using_delta_bfs_and_schedule(int est_dist_max) {

	//! NOTE: you should implement your own scheduler.
	//! Scheduler is defined as a std::function to ease further investigation
	//! on different Schedulers.
	scheduler_t scheduler = scheduler_bfs;

	auto schedule = scheduler(friends);

	for (auto &p: schedule->children) {
		std::vector<int> dist(np, (int)np + est_dist_max);
		std::vector<int> dist_count(np + est_dist_max + 1);
		dist_count[np + est_dist_max] = (int)np;

		int base_dist = (int)np - 1;
		process_est(p, base_dist - 1, dist, dist_count, est_dist_max);
	}
}

void Query4Calculator::process_est(std::shared_ptr<ScheduleNode> &node,
	int base_dist, std::vector<int> &dist, std::vector<int> &dist_count,
	int est_dist_max) {

	std::vector<std::pair<int, int>> changed_vtx;

	bfs(friends, node->vtx, base_dist, est_dist_max, dist, dist_count, &changed_vtx);

	int s = get_s_by_dist_count(node->vtx, dist_count,
			base_dist, base_dist + est_dist_max);

	estimated_s[node->vtx] = s;

	for (auto &child: node->children) {
		process_est(child, base_dist - 1, dist, dist_count, est_dist_max);
	}

	process_est_rollback(dist, dist_count, changed_vtx);
}

void Query4Calculator::process_est_rollback(std::vector<int> &dist,
		std::vector<int> &dist_count, const std::vector<std::pair<int, int>> &changed_vtx) {
	for (auto &p: changed_vtx) {
		int v = p.first, d = p.second;
		dist_count[dist[v]] --;
		dist_count[d] ++;
		dist[v] = d;
	}
}

void Query4Calculator::estimate_all_s_using_delta_bfs(int est_dist_max) {
	vector<bool> is_done(np);

	int nr_expect_to_search = 0;
	int nr_searched = 0;
	int nr_search_restart = 0;


	for (int root = 0; root < (int)np; root ++) {
		if (is_done[root])
			continue;

		nr_search_restart ++;

		int base_dist = (int)np - 1;

		vector<int> dist(np, (int)np + est_dist_max);
		vector<int> dist_count(np + est_dist_max + 1);

		dist_count[np + est_dist_max] = (int)np;

		int cur_vtx = root;
		for (; ;) {
			nr_searched += bfs(friends, cur_vtx, base_dist, est_dist_max, dist,
					dist_count);

			for (int i = 0; i <= est_dist_max; i ++)
				nr_expect_to_search += dist_count[base_dist + i];

			estimated_s[cur_vtx] = get_s_by_dist_count(cur_vtx, dist_count,
					base_dist, base_dist + est_dist_max);

			is_done[cur_vtx] = true;

			// XXX: CHECK
//            int exact_s = estimate_s_limit_depth(cur_vtx, est_dist_max);
//            m_assert(estimated_s[cur_vtx] == exact_s);

			// next round
			int next_vtx = -1;
			for (auto &v: friends[cur_vtx])
				if (!is_done[v]) {
					next_vtx = v;
					break;
				}
			if (next_vtx == -1)
				break;
			cur_vtx = next_vtx;
			base_dist --;
		}
	}

	fprintf(stderr, "search_cutoff: %f %d/%d restart: %d\n",
			(double)nr_searched / nr_expect_to_search, nr_expect_to_search,
			nr_searched, nr_search_restart);
}

int Query4Calculator::bfs(const std::vector<std::vector<int>> &graph,
		int source, int base_dist, int est_dist_max,
		std::vector<int> &dist, std::vector<int> &dist_count,
		std::vector<std::pair<int, int>> *changed_vtx) {
	std::queue<int> q;
	if (changed_vtx)
		changed_vtx->emplace_back(source, dist[source]);
	dist_count[dist[source]] --;
	dist[source] = base_dist;
	dist_count[base_dist] ++;
	q.push(source);
	int nr_vtx_traversed = 0;
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		nr_vtx_traversed += qsize;
		if (depth == est_dist_max)
			break;
		int d1 = base_dist + depth + 1;
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
			for (auto &v1: graph[v0]) {
				if (dist[v1] <= d1)
					continue;
				if (changed_vtx)
					changed_vtx->emplace_back(v1, dist[v1]);
				dist_count[dist[v1]] --;
				dist_count[d1] ++;
				dist[v1] = d1;
				q.push(v1);
			}
		}
	}
	return nr_vtx_traversed;
}

void Query4Handler::add_query(int k, const string& s, int index) {
	TotalTimer timer("Q4");
	// build graph
	vector<PersonInForum> persons = get_tag_persons(s);
	size_t np = persons.size();
	vector<vector<int>> friends(np);

	{
		GuardedTimer timer("omp1");
#pragma omp parallel for schedule(static) num_threads(4)
		REP(i, np) {
			auto& fs = Data::friends[persons[i]];
			FOR_ITR(itr, fs) {
				auto lb_itr = lower_bound(persons.begin(), persons.end(), itr->pid);
				if (lb_itr != persons.end() and *lb_itr == itr->pid) {
					int v = (int)distance(persons.begin(), lb_itr);
					friends[i].push_back(v);
				}
			}

			//            set<int> s(friends[i].begin(), friends[i].end());
			//            friends[i].resize(s.size());
			//            std::copy(s.begin(), s.end(), friends[i].begin());
		}
	}
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
