/*
 * $File: query4_wyx.cc
 * $Date: Thu Mar 27 22:21:47 2014 +0800
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#include "query4.h"
#include "data.h"
#include "lib/common.h"
#include "lib/Timer.h"
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

int Query4Calculator::get_extact_s(int source) {
	std::vector<bool> hash(np);
	std::queue<int> q;
	hash[source] = true;
	q.push(source);
	int s = 0;
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
			s += depth;
			FOR_ITR(v1, friends[v0]) {
				if (hash[*v1])
					continue;
				hash[*v1] = true;
				q.push(*v1);
			}
		}
	}
	return s;
}

int Query4Calculator::estimate_s_limit_depth_cut_upper(int source, int depth_max, double cent_upper) {
	int s_bound = ::sqr(degree[source] - 1.0) / cent_upper / (int(np) - 1);
	std::vector<bool> hash(np);
	std::queue<int> q;
	hash[source] = true;
	q.push(source);
	int s = 0;
	int nr_remain = degree[source];
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		s += depth * qsize;
		nr_remain -= qsize;
		int s_est_cur = s + nr_remain * (depth + 1);
		//		double cent = get_centrality_by_vtx_and_s(source, s_est_cur);
		// TODO
//		if (s_est_cur > s_bound and depth <= depth_max) return 1e9;
		if(depth == depth_max)
			break;
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
			FOR_ITR(v1, friends[v0]) {
				if (hash[*v1])
					continue;
				hash[*v1] = true;
				q.push(*v1);
			}
		}
	}
	s += nr_remain * (depth_max + 1);
	return s;
}

int Query4Calculator::estimate_s_limit_depth(int source, int depth_max) {
	std::vector<bool> hash(np);
	std::queue<int> q;
	hash[source] = true;
	q.push(source);
	int s = 0;
	int nr_remain = degree[source];
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		s += depth * qsize;
		nr_remain -= qsize;
		if (depth == depth_max)
			break;
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
			FOR_ITR(v1, friends[v0]) {
				if (hash[*v1])
					continue;
				hash[*v1] = true;
				q.push(*v1);
			}
		}
	}
	s += nr_remain * (depth_max + 1);
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
			FOR_ITR(v1, g[v0]) {
				if (hash[*v1])
					continue;
				hash[*v1] = true;
				farthest_vtx = *v1;
				dist_max = depth + 1;
				q.push(*v1);
			}
		}
	}
}

vector<int> Query4Calculator::work() {

	Timer timer;

	int est_dist_max = 2;  // TODO: this parameter needs tune
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
    est_dist_max = 3;


	vector<HeapEle> heap_ele_buf(np);
	estimated_s.resize(np);
	timer.reset();

/*
 *    RandomChoiceEstimator estimator(friends, degree, pow(log(np), 0.333) / (10.2 * pow(np, 0.333)));
 *    print_debug("now: %lf\n", timer.get_time());
 *    estimated_s = move(estimator.result);
 *
 *    for (int i = 0; i < (int)np; i ++) {
 *        double centrality = get_centrality_by_vtx_and_s(i, estimated_s[i]);
 *        heap_ele_buf[i] = HeapEle(i, centrality);
 *    }
 *
 *    priority_queue<HeapEle> q2(heap_ele_buf.begin(), heap_ele_buf.end());
 *    int cand_size = 3 * k;
 *    vector<int> cand;
 *    REP(i, cand_size) {
 *        cand.push_back(q2.top().vtx);
 *        q2.pop();
 *    }
 *    print_debug("now: %lf\n", timer.get_time());
 *    vector<double> reals(cand.size());
 *    PP(np);
 *    REP(i, cand_size) {
 *        int s = get_extact_s(cand[i]);
 *        reals[i] = get_centrality_by_vtx_and_s(cand[i], s);
 *    }
 *    sort(reals.begin(), reals.end());
 *    reverse(reals.begin(), reals.end());
 *    double bound = reals[k - 1];
 *    print_debug("bound: %.10lf\n", bound);
 *
 *    print_debug("now: %lf\n", timer.get_time());
 *#pragma omp parallel for schedule(static) num_threads(4)
 *    REP(i, np) {
 *        estimated_s[i] = estimate_s_limit_depth_cut_upper(i, est_dist_max, bound);
 *    }
 */
/*
 *#pragma omp parallel for schedule(static) num_threads(4)
 *    REP(i, np) {
 *        estimated_s[i] = estimate_s_limit_depth(i, est_dist_max);
 *    }
 */

	SSEUnionSetEstimator estimator(friends, degree, est_dist_max);
	estimated_s = move(estimator.result);
	print_debug("now: %lf\n", timer.get_time());

	//	estimate_all_s_using_delta_bfs(est_dist_max);
	//	PP(np);


	heap_ele_buf.clear();
	int cnt_cut = 0;
	for (int i = 0; i < (int)np; i ++) {
		/*
		 *if (estimated_s[i] == 1e9) {
		 *    cnt_cut ++;
		 *    continue;
		 *}
		 */

		double centrality = get_centrality_by_vtx_and_s(i, estimated_s[i]);
		heap_ele_buf.emplace_back(i, centrality);
	}
	//PP(cnt_cut);

	double time_phase1 = timer.get_time();
	timer.reset();
	priority_queue<HeapEle> q(heap_ele_buf.begin(), heap_ele_buf.end());

	// iterate
	vector<int> ans;

	int cnt = 0;
	{
		double last_centrality = 1e100;
		int last_vtx = -1;
		while (!q.empty()) {
			auto he = q.top(); q.pop();
			int vtx = he.vtx;
			double centrality = he.centrality;
			m_assert(centrality <= last_centrality);
			if (centrality == last_centrality && vtx == last_vtx) {
				ans.emplace_back(vtx);
				if ((int)ans.size() == k) {
					print_debug("ans: %.10lf\n", centrality);
					break;
				}
			} else {
				cnt ++;
				int s = get_extact_s(vtx);
				// int es = estimated_s[vtx];
				double new_centrality = get_centrality_by_vtx_and_s(vtx, s);
				q.push(HeapEle(vtx, new_centrality));
			}

			last_centrality = centrality;
			last_vtx = vtx;
		}
	}

	auto time = timer.get_time();
	//	print_debug("total: %f secs\n", time);
	if (np > 11000) {
		static int print = 0;
		if (print < 3)
			fprintf(stderr, "cnt: %f-%f %lu/%d/%d/%d/%d\n", time_phase1, time, np, cnt, k, (int)diameter, (int)est_dist_max);
		print ++;
	}
	return move(ans);
}

double Query4Calculator::get_centrality_by_vtx_and_s(int v, int s) {
	if (s == 0)
		return 0;
	double ret = ::sqr(degree[v] - 1.0) / (double)s / ((int)np - 1);
	//    assert(!isnan((long double)ret));
	return ret;
}


int Query4Calculator::get_s_by_dist_count(int vtx, const std::vector<int> &dist_count,
		int begin, int finish) {
	int s = 0;
	int nr_remain = degree[vtx];
	for (int i = begin; i <= finish; i ++) {
		s += (i - begin) * dist_count[i];
		nr_remain -= dist_count[i];
	}

	s += (finish - begin + 1) * (int)nr_remain;
	return s;
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
		int last_vtx = -1;
		for (; ;) {
			int now_searched = bfs(friends, cur_vtx, base_dist, est_dist_max, dist,
					dist_count);
			//if (last_vtx != -1)
			nr_searched += now_searched;

			int should_searched = 0;
			for (int i = 0; i <= est_dist_max; i ++)
				should_searched += dist_count[base_dist + i];
			//if (last_vtx != -1)
			nr_expect_to_search += should_searched;

			estimated_s[cur_vtx] = get_s_by_dist_count(cur_vtx, dist_count,
					base_dist, base_dist + est_dist_max);

			is_done[cur_vtx] = true;

			// XXX: CHECK
			/*
			 *int exact_s = estimate_s_limit_depth(cur_vtx, est_dist_max);
			 *m_assert(estimated_s[cur_vtx] == exact_s);
			 */

			/*
			 *if (last_vtx != -1)
			 *print_debug("From %d to %d, s: %d to %d, work: %d / %d\n", last_vtx, cur_vtx,
			 *        estimated_s[last_vtx], estimated_s[cur_vtx], now_searched, should_searched);
			 */

			// next round
			int next_vtx = -1;
			int max_friends = -1;
			int max_v = -1;
			FOR_ITR(v, friends[cur_vtx]) {
				if (!is_done[*v]) {
					next_vtx = *v; break;
					/*
					 *if (update_max(max_friends, (int)pre_estimated_s[v]))
					 *    max_v = v;
					 */
				}
				next_vtx = max_v;
			}
			/*
			 *if (pre_estimated_s[next_vtx] < pre_estimated_s[cur_vtx])
			 *    break;
			 */
			if (next_vtx == -1)
				break;
			last_vtx = cur_vtx;
			cur_vtx = next_vtx;
			base_dist --;
		}
	}

	fprintf(stderr, "search_cutoff: %f %d/%d restart: %d np: %lu\n",
			(double)nr_searched / nr_expect_to_search, nr_expect_to_search, nr_searched,
			nr_search_restart, np);
}

int Query4Calculator::bfs(const std::vector<std::vector<int>> &graph,
		int source, int base_dist, int est_dist_max,
		std::vector<int> &dist, std::vector<int> &dist_count, std::vector<std::pair<int, int>>*) {
	std::queue<int> q;
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
			FOR_ITR(v1, graph[v0]) {
				if (dist[*v1] <= d1)
					continue;
				dist_count[dist[*v1]] --;
				dist_count[d1] ++;
				dist[*v1] = d1;
				q.push(*v1);
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
