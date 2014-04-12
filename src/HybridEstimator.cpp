//File: HybridEstimator.cpp
//Date: Sat Apr 12 10:28:31 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "HybridEstimator.h"
#include "globals.h"
using namespace std;

HybridEstimator::HybridEstimator(const std::vector<std::vector<int>>& _graph, int* _degree,
		vector<bool>& _noneed, int _sum_bound,
		const vector<int>& _approx_result):
	SumEstimator(_graph), degree(_degree),
	noneed(_noneed), sum_bound(_sum_bound),
	approx_result(_approx_result)
{
//    bfs_2_dp_1();
//    init();
}

void HybridEstimator::init() {
//    fprintf(stderr, "@@@@@@@@@@\n");
	bfs_2_dp_1();
}

void HybridEstimator::bfs_2_dp_1() {
	depth = 3;
	Timer init;
	int len = get_len_from_bit(np);

	//std::vector<Bitset> s_prev;
	BitBoard s_prev{np};
	{
		TotalTimer tt("hybrid alloc");			// about 3% of total q4 time

		result.resize((size_t)np, 0);
		nr_remain.resize(np);
		REP(i, np)
			nr_remain[i] = degree[i];
	}

	// bfs 2 depth
	cutcnt = 0;
	{
		TotalTimer ttt("depth 2");
		int tag = 0;
		REP(i, np) {
			tag ++;

			// depth 0
			s_prev[i].set(i);
			nr_remain[i] -= 1;

			size_t sum_dv1 = 0;
			// depth 1
			FOR_ITR(fr, graph[i]) {
				sum_dv1 += graph[*fr].size();
				s_prev[i].set(*fr);
			}
			nr_remain[i] -= (int)graph[i].size();
			result[i] += (int)graph[i].size();

			size_t sum_dv2 = 0;
			// depth 2
			FOR_ITR(fr, graph[i]) {
				int j = *fr;
				FOR_ITR(fr2, graph[j]) {
					if (s_prev[i].get_and_set(*fr2))
						continue;
					sum_dv2 += graph[*fr2].size();
					nr_remain[i] --;
					result[i] += 2;
				}
			}

			if (not noneed[i]) {
				// XXX this is wrong
				int n3_upper = (int)sum_dv2 - (int)sum_dv1 + (int)graph[i].size() + 1;
				m_assert(n3_upper >= 0);
				int est_s_lowerbound = result[i] + n3_upper * 3 + (nr_remain[i] - n3_upper) * 4;
				if (est_s_lowerbound > sum_bound) {		// cut
					noneed[i] = true;
					cutcnt ++;
					result[i] = 1e9;
				}
			}
		}
	}

	// union depth 3
	{
		TotalTimer ttt("depth 3");
		int nr_idle = threadpool->get_nr_idle_thread();
		if (nr_idle) {
			print_debug("Idle thread: %d\n", nr_idle);
			/*
			 *if (nr_idle == 1)
			 *    nr_idle = 2;
			 */
#pragma omp parallel for schedule(dynamic) num_threads(4)
			REP(i, np) {
				if (noneed[i]) continue;
				if (result[i] == 0) continue;
				if (nr_remain[i] == 0) continue;
				Bitset s(len);
				FOR_ITR(fr, graph[i])
					s.or_arr(s_prev[*fr], len);
				s.and_not_arr(s_prev[i], len);

				int c = s.count(len);
				//			print_debug("S1: %lu, S2: %d S3: %d\n", graph[i].size(), s_prev[i].count(len) - graph[i].size(), c);
				result[i] += c * 3;
				nr_remain[i] -= c;
				result[i] += nr_remain[i] * 4;
				s.free();
			}
		} else {
			Bitset s(len);
			REP(i, np) {
				if (noneed[i]) continue;
				if (result[i] == 0) continue;
				if (nr_remain[i] == 0) continue;
				s.reset(len);
				FOR_ITR(fr, graph[i]) s.or_arr(s_prev[*fr], len);
				s.and_not_arr(s_prev[i], len);

				int c = s.count(len);
				//			print_debug("S1: %lu, S2: %d S3: %d\n", graph[i].size(), s_prev[i].count(len) - graph[i].size(), c);
				result[i] += c * 3;
				nr_remain[i] -= c;
				result[i] += nr_remain[i] * 4;
			}
			s.free();
		}
	}
}

void HybridEstimator::bfs_2_dp_more() {
	Timer init;
	int len = get_len_from_bit(np);

	//std::vector<Bitset> s_prev;
	BitBoard s_prev(np);
	{
		TotalTimer tt("hybrid alloc");			// about 3% of total q4 time
		/*
		 *s_prev.reserve(np);
		 *REP(i, np)
		 *    s_prev.emplace_back(len);
		 */


		result.resize((size_t)np, 0);
		nr_remain.resize(np);
		REP(i, np)
			nr_remain[i] = degree[i];
	}

	// bfs 2 depth
	cutcnt = 0;
	{
		TotalTimer ttt("depth 2");
		int tag = 0;
		REP(i, np) {
			tag ++;

			// depth 0
			s_prev[i].set(i);
			nr_remain[i] -= 1;

			size_t sum_dv1 = 0;
			// depth 1
			FOR_ITR(fr, graph[i]) {
				sum_dv1 += graph[*fr].size();
				s_prev[i].set(*fr);
			}
			nr_remain[i] -= (int)graph[i].size();
			result[i] += (int)graph[i].size();

			size_t sum_dv2 = 0;
			// depth 2
			FOR_ITR(fr, graph[i]) {
				int j = *fr;
				FOR_ITR(fr2, graph[j]) {
					if (s_prev[i].get_and_set(*fr2))
						continue;
					sum_dv2 += graph[*fr2].size();
					nr_remain[i] --;
					result[i] += 2;
				}
			}

			if (not noneed[i]) {
				// XXX this is wrong
				int n3_upper = (int)sum_dv2 - (int)sum_dv1 + (int)graph[i].size() + 1;
				m_assert(n3_upper >= 0);
				int est_s_lowerbound = result[i] + n3_upper * 3 + (nr_remain[i] - n3_upper) * 4;
				if (est_s_lowerbound > sum_bound) {		// cut
					noneed[i] = true;
					cutcnt ++;
					result[i] = 1e9;
				}
			}
		}
	}

	vector<int> tmp_result(np);
	BitBoard s(np);
	/*
	 *vector<Bitset> s; s.reserve(np);		// XXX MEMORY!!
	 *{
	 *    TotalTimer tt("hybrid alloc");
	 *    REP(i, np)
	 *        s.emplace_back(len);
	 *}
	 */
	depth = 3;
	double err = 0;
	while (true) {
		// TODO separate d3 and d4 to save memory at d4
		TotalTimer ttt("Depth 3+");
		// calculate s from s_prev
		REP(i, np) {
			if (depth == 4) {	// cannot prune in depth=3, because depth=4 need every s[i]_3
				if (noneed[i]) continue;
				if (result[i] == 0) continue;
				if (nr_remain[i] == 0) continue;
			}

			s[i].reset(len);
			FOR_ITR(fr, graph[i])
				s[i].or_arr(s_prev[*fr], len);
			s[i].and_not_arr(s_prev[i], len);
			int c = s[i].count(len);
			result[i] += c * depth;
			nr_remain[i] -= c;
			tmp_result[i] = result[i] + nr_remain[i] * (depth + 1);
		}

		if (depth == 4)		// XXX at most 4? might be enough!  // TODO: 4th level can be implemented with half memory
			break;

		// judge whether tmp_result is accurate enough
		if (good_err(tmp_result))
			break;
		print_debug("Err: %.4lf with np=%d\n", err, np);
		depth ++;
		s.swap(s_prev);
	}

/*
 *    long long all = 0;
 *    for (int i = 0; i < np; i ++)
 *        all += s_prev[i].count(len);
 *    double ave = all / (double)np;
 *
 *    fprintf(stderr, "ave popcount: d %d %f/%d %f\n", depth, ave, np, ave / np);
 */

	result = move(tmp_result);
}


VectorMergeHybridEstimator::VectorMergeHybridEstimator(
		const std::vector<std::vector<int>>& _graph, int* _degree,
		vector<bool>& _noneed, int _sum_bound,
		const vector<int>& _approx_result):
	HybridEstimator(_graph, _degree, _noneed, _sum_bound, _approx_result)
{
//    vector_dp();
}


void VectorMergeHybridEstimator::init() {
	vector_dp();
}

void VectorMergeHybridEstimator::vector_dp() {

	vector<vector<int>> s_now(np), s_prev(np); // f[i]_k: {points | dist(points, i) <= k}

	vector<vector<size_t>> count(np);  // of size np * depth
	result.resize((size_t)np, 0);
	cutcnt = 0;

	{
		for (int i = 0; i < np; i ++) {
			s_now[i] = graph[i];
			s_now[i].emplace_back(i);
			sort(s_now[i].begin(), s_now[i].end());

			count[i].resize(2);
			count[i][0] = 1;
			count[i][1] = 1 + graph[i].size();
//            fprintf(stderr, "%lu %lu %lu\n", count[i][1], s_now[i].size(), graph[i].size());
			assert(count[i][1] == s_now[i].size());
			result[i] = graph[i].size();
		}
	}

	long long cost = 0;
	long long estimated_cost = 0;;
	vector<int> buf[2];
	buf[0].resize(np);
	buf[1].resize(np);
	vector<int> tmp_result(result.size());
	int depth_max = 3;
	for (depth = 2; ;) {
		s_now.swap(s_prev);
		for (int i = 0; i < np; i ++) {
			int cnt = 0;
			buf[0].resize(0);
			buf[1].resize(0);


#if 1
			// merge all neighbours
			for (auto &fr: graph[i]) {
				auto &prev = buf[cnt & 1];
				auto &cur = buf[(cnt + 1) & 1];
				auto &g = s_prev[fr];

				cost += unique_merge(prev, g, cur);
//                cur.resize(prev.size() + g.size());
//                merge(
//                    prev.begin(), prev.end(),
//                    g.begin(), g.end(),
//                    cur.begin());
//                cur.resize(unique(cur.begin(), cur.end()) - cur.begin());
				cnt ++;
			}

			cost += unique_merge(buf[cnt & 1], s_prev[i], s_now[i]);
#else

			std::vector<std::vector<int> *> items;
			for (auto &fr: graph[i])
				items.emplace_back(&s_prev[fr]);
			items.emplace_back(&s_prev[i]);
			cost += quick_unique_merge(items, s_now[i]);
#endif

#if 1
			// TEST: estimate cost
			vector<int> sizes;
			for (auto &fr: graph[i])
				sizes.emplace_back(s_prev[fr].size());
			sizes.emplace_back(s_prev[i].size());
			estimated_cost += estimate_merge_cost(sizes);
#endif


			count[i].resize(depth + 1);
			count[i][depth] = s_now[i].size();

			result[i] += (count[i][depth] - count[i][depth - 1]) * depth;

			if (noneed[i])
				continue;
			int nr_next_depth_upper = 0;
			for (auto &fr: s_now[i]) {
				nr_next_depth_upper += graph[fr].size();
			}

			int est_s_lowerbound =
				result[i] +
				(degree[i] - count[i][depth]) * (depth + 1);
//
//                nr_next_depth_upper * (depth + 1) +
//                std::max(0, (int)(degree[i] - count[i][depth] - nr_next_depth_upper)) * (depth + 2);
			tmp_result[i] = est_s_lowerbound;


			if (0) {
				int s = get_exact_s(i);
				if (est_s_lowerbound > s)
					fprintf(stderr, "%d %d\n", est_s_lowerbound, s);
				m_assert(est_s_lowerbound <= s);
			}

			if (not noneed[i]) {
//                fprintf(stderr, "%d %d\n", est_s_lowerbound, sum_bound);
				if (est_s_lowerbound > sum_bound) {		// cut
					noneed[i] = true;
					cutcnt ++;
					result[i] = 1e9;
				}
			}
		}

		if (depth == depth_max)
			break;
		if (good_err(tmp_result))
			break;

//        fprintf(stderr, "depth: %d\n", depth);
		depth ++;
	}

    fprintf(stderr, "merge cost: %lld %lld %f\n", cost, estimated_cost, (double)estimated_cost / cost);

	result = move(tmp_result);

#if 0
	for (int i = 0; i < np; i ++) {
		auto &r = result[i] = 0;
		for (int d = 1; d <= depth; d ++)
			r += (count[i][d] - count[i][d - 1]) * d;
		r += (degree[i] - count[i][depth]) * (depth + 1);

#if 0
		int p = estimate_s_limit_depth(i, depth);
		if (r != p) {
			fprintf(stderr, "%d %d\n", r, p);
		}
		m_assert(r == p);
#endif

//        fprintf(stderr, "%d\n", r);
	}
#endif
}

int VectorMergeHybridEstimator::estimate_s_limit_depth(int source, int depth_max) {
	std::vector<bool> hash(np);
	std::queue<int> q;
	hash[source] = true;
	q.push(source);
	long long s = 0;
	int nr_remain = degree[source];
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		s += depth * qsize;
		nr_remain -= qsize;
		if (depth == depth_max)
			break;
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
			for (auto &v1: graph[v0]) {
				if (hash[v1])
					continue;
				hash[v1] = true;
				q.push(v1);
			}
		}
	}
	s += nr_remain * (depth_max + 1);
	return s;
}


int VectorMergeHybridEstimator::unique_merge(const std::vector<int> &a, const std::vector<int> &b, std::vector<int> &c) {
	TotalTimer ttt("unique_merge");

#if 1
	c.resize(a.size() + b.size());
	std::merge(a.begin(), a.end(),
			b.begin(), b.end(),
			c.begin());
	c.resize(std::unique(c.begin(), c.end()) - c.begin());

#else
	c.reserve(a.size() + b.size());
	c.resize(0);

	size_t p = 0, q = 0;
	while (p < a.size() && q < b.size()) {
		if (c.size()) {
			if (a[p] == c.back()) {
				p ++;
				continue;
			}
			else if (b[q] == c.back()) {
				q ++;
				continue;
			}
		}
		if (a[p] < b[q])
			c.emplace_back(a[p ++]);
		else c.emplace_back(b[q ++]);
	}
	while (p < a.size()) { // guaranteed to be unique
		c.emplace_back(a[p ++]);
	}
	while (q < b.size()) { // guaranteed to be unique
		c.emplace_back(b[q ++]);
	}
#endif
	return a.size() + b.size();
}

int VectorMergeHybridEstimator::estimate_merge_cost(const std::vector<int> &sizes) {
	if (sizes.size() <= 1)
		return 0;
	std::priority_queue<int, vector<int>, greater<int>> heap(sizes.begin(), sizes.end());
	int cost = 0;
	while (heap.size() >= 2) {
		int v0 = heap.top(); heap.pop();
		int v1 = heap.top(); heap.pop();
		int v = v0 + v1;
		cost += v;
		heap.push(v);
	}
	return cost;
}


struct VectorPtrComparator {
	public:
		bool operator () (const vector<int> *a, const vector<int> *b) {
			return a->size() > b->size();
		}
};


__thread std::vector<vector<int>> *buf_ptr = NULL;
long long VectorMergeHybridEstimator::quick_unique_merge(const vector<vector<int> *> &items, std::vector<int> &out) {
	long long cost = 0;
	if (buf_ptr == NULL) {
		buf_ptr = new vector<vector<int>>();
	}
	vector<vector<int>> buf = *buf_ptr;
	buf.resize(items.size());
	for (auto &p: buf)
		p.resize(0);

	std::priority_queue<vector<int> *, vector<vector<int>*>, VectorPtrComparator> heap(items.begin(), items.end());
	int cnt = 0;
	while (heap.size() >= 2) {
		auto a = heap.top(); heap.pop();
		auto b = heap.top(); heap.pop();
		cost += unique_merge(*a, *b, buf[cnt]);
		heap.push(&buf[cnt]);
		cnt ++;
	}
	out = *heap.top();
	return cost;
}
