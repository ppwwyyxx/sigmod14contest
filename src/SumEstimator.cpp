//File: SumEstimator.cpp
//Date: Fri Mar 28 11:40:45 2014 +0000
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "SumEstimator.h"
#include "lib/common.h"
#include <queue>
#include <limits>
#include <cmath>
using namespace std;
using namespace boost;


__thread std::vector<boost::dynamic_bitset<>> *UnionSetDepthEstimator::s;
__thread std::vector<boost::dynamic_bitset<>> *UnionSetDepthEstimator::s_prev;

int SumEstimator::get_exact_s(int source) {
	std::vector<bool> hash(np);
	std::queue<int> q;
	hash[source] = true;
	q.push(source);
	int s = 0;
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		s += depth * qsize;
		for (int i = 0; i < qsize; i ++) {
			int v0 = q.front(); q.pop();
			FOR_ITR(itr, graph[v0]) {
				if (hash[*itr]) continue;
				hash[*itr] = true;
				q.push(*itr);
			}
		}
	}
	return s;
}

void SumEstimator::error() {
	double ret = 0;
	int pos_cnt = 0;
	REP(i, np) {
		auto est = estimate(i);
		auto truth = get_exact_s(i);
		if (truth != 0 && est != numeric_limits<int>::max()) {
			double err = (double)(truth - est) / truth;
			if (err > 0)
				pos_cnt ++;
			if (fabs(err) > 0.03) {
				print_debug("Error: %lf, truth: %d, est: %d\n", err, truth, est);
			}
			ret += fabs(err);
		}
	}
	print_debug("Error: %lf, smaller: %lf\n", ret / np, (double)pos_cnt / np);
}

void RandomChoiceEstimator::work() {
	vector<int> vst_cnt(np, 0);
	auto n = samples.size();
	vector<int> true_result(n);
#pragma omp parallel for schedule(static) num_threads(4)
	REP(i, n)
		true_result[i] = bfs_all(samples[i], vst_cnt);
	REP(i, np) {
		if (vst_cnt[i] == 0)
			result[i] = get_exact_s(i);
		else {
			result[i] = result[i] * degree[i] / vst_cnt[i];

		}
	}
	REP(i, n)
		result[samples[i]] = true_result[i];
}

int RandomChoiceEstimator::bfs_all(int source, vector<int>& vst_cnt) {
	int sum = 0;
	queue<int> q;
	vector<bool> vst(np);
	q.push(source);
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize = (int)q.size();
		sum += depth * qsize;
		REP(i, qsize) {
			int top = q.front(); q.pop();
			FOR_ITR(f, graph[top]) {
				if (vst[*f]) continue;
				vst[*f] = true;
				result[*f] += depth + 1;
				vst_cnt[*f] ++;
				q.push(*f);
			}
		}
	}
	return sum;
}

UnionSetDepthEstimator::UnionSetDepthEstimator(
		const std::vector<std::vector<int>>& _graph,
		int* degree, int _depth_max)
		: SumEstimator(_graph), depth_max(_depth_max) {
	DEBUG_DECL(Timer, init);
	size_t alloc = np;
	/*
	 *if (!s){
	 *    s = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
	 *    s_prev = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
	 *} else if ((int)s->size() < np) {
	 *    delete s; delete s_prev;
	 *    s = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
	 *    s_prev = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
	 *} else {
	 *    REP(i, s->size()) {
	 *        (*s)[i].reset();
	 *        (*s_prev)[i].reset();
	 *    }
	 *}
	 */
	s = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));
	s_prev = new std::vector<boost::dynamic_bitset<>>(alloc, boost::dynamic_bitset<>(alloc));


	print_debug("allocTime: %lf\n", init.get_time());
	result.resize((size_t)np, 0);
	nr_remain.resize(np);

	auto& out_scope_ptr = *s_prev;

//#pragma omp parallel for schedule(static) num_threads(4)
	REP(i, np) {
		out_scope_ptr[i][i] = 1;
		FOR_ITR(fr, graph[i]) {
			out_scope_ptr[i][*fr] = 1;
		}
		result[i] += (int)graph[i].size();
		nr_remain[i] = degree[i] - 1 - (int)graph[i].size();
	}
	work();
	delete s; delete s_prev;
}

void UnionSetDepthEstimator::work() {
	DEBUG_DECL(TotalTimer, gg("union work"));
	auto& out_s_prev = *s_prev;
	auto& out_s = *s;

	for (int k = 2; k <= depth_max; k ++) {
		// CANNOT USE OMP HERE!
//#pragma omp parallel for schedule(dynamic) num_threads(4)
		REP(i, np) {
			out_s[i].reset();
			FOR_ITR(fr, graph[i])
				out_s[i] |= out_s_prev[*fr];
			out_s[i] &= (~out_s_prev[i]);

			int c = (int)out_s[i].count();
			result[i] += c * k;
			nr_remain[i] -= c;

			out_s[i] |= out_s_prev[i];
		}
		s->swap(out_s_prev);
	}
	//s_prev is ans

	REP(i, np)
		result[i] += nr_remain[i] * (depth_max + 1);
}

SSEUnionSetEstimator::SSEUnionSetEstimator(
		const vector<vector<int>>& _graph,
		int* degree, int _depth_max) :SumEstimator(_graph), depth_max(_depth_max) {
	//DEBUG_DECL(Timer, init);
	Timer init;
	int len = get_len_from_bit(np);

	s.reserve(np); s_prev.reserve(np);
	REP(i, np) {
		s.emplace_back(len);
		s_prev.emplace_back(len);
	}

	static int print = 0;
	if (print < 3) {
		fprintf(stderr, "alloc%d,%.4lf\n", np, init.get_time());
		print ++;
	}

//	print_debug("allocTime: %lf\n", init.get_time());
	result.resize((size_t)np, 0);
	nr_remain.resize(np);

#pragma omp parallel for schedule(static) num_threads(4)
	REP(i, np) {
		s_prev[i].set(i);
		FOR_ITR(fr, graph[i])
			s_prev[i].set(*fr);
		result[i] += (int)graph[i].size();
		nr_remain[i] = degree[i] - 1 - (int)graph[i].size();
	}
	work();
}

void SSEUnionSetEstimator::work() {
	DEBUG_DECL(TotalTimer, uniont("sse"));
	int len = get_len_from_bit(np);
	for (int k = 2; k <= depth_max; k ++) {
#pragma omp parallel for schedule(static) num_threads(4)
		REP(i, np) {
			s[i].reset(len);
			FOR_ITR(fr, graph[i])
				s[i].or_arr(s_prev[*fr], len);
			s[i].and_not_arr(s_prev[i], len);

			int c = s[i].count(len);
			result[i] += c * k;
			nr_remain[i] -= c;
			s[i].or_arr(s_prev[i], len);
		}
		s.swap(s_prev);
	}
	REP(i, np)
		result[i] += nr_remain[i] * (depth_max + 1);
}


HybridEstimator::HybridEstimator(const std::vector<std::vector<int>>& _graph, int* degree, int _depth_max):
	SumEstimator(_graph), depth_max(_depth_max)
{
	m_assert(depth_max == 3);
	Timer init;
	int len = get_len_from_bit(np);

	std::vector<Bitset> s_prev;
	s_prev.reserve(np);
	REP(i, np) {
		s_prev.emplace_back(len);
	}

	result.resize((size_t)np, 0);
	nr_remain.resize(np);
	REP(i, np) nr_remain[i] = degree[i];

	print_debug("Alloc: %lf\n", init.get_time());
	init.reset();

	// bfs 2 depth
	PP(np);
#pragma omp parallel for schedule(static) num_threads(4)
	REP(i, np) {
		vector<bool> hash(np, false);

		// depth 0
		hash[i] = true;
		s_prev[i].set(i);
		nr_remain[i] -= 1;

		// depth 1
		FOR_ITR(fr, graph[i]) {
			hash[*fr] = true;
			s_prev[i].set(*fr);
		}
		nr_remain[i] -= (int)graph[i].size();
		result[i] += graph[i].size();

		// depth 2
		FOR_ITR(fr, graph[i]) {
			int j = *fr;
			FOR_ITR(fr2, graph[j]) {
				if (hash[*fr2])
					continue;
				hash[*fr2] = true;
				s_prev[i].set(*fr2);
				nr_remain[i] --;
				result[i] += 2;
			}
		}
		/*
		 *int c = s_prev[i].count(len);
		 *if (c > 10000)
		 *    print_debug("np: %d count d2: %d\n", np, c);
		 */
	}
	print_debug("Depth2: %lf\n", init.get_time());
	init.reset();

	// union depth 3
#pragma omp parallel for schedule(static) num_threads(4)
	REP(i, np) {
		Bitset s(len);
		FOR_ITR(fr, graph[i])
			s.or_arr(s_prev[*fr], len);
		s.and_not_arr(s_prev[i], len);

		int c = s.count(len);
		result[i] += c * 3;
		nr_remain[i] -= c;
	}

	print_debug("Depth3: %lf\n", init.get_time());

	REP(i, np)
		result[i] += nr_remain[i] * 4;
}
