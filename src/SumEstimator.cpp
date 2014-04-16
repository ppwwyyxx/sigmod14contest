//File: SumEstimator.cpp
//Date: Wed Apr 16 08:01:56 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <queue>
#include <limits>
#include <cmath>
#include "SumEstimator.h"
#include "lib/common.h"
#include "lib/utils.h"
using namespace std;
using namespace boost;



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

#include <fstream>
void SumEstimator::error() {
	double ret = 0;
	int pos_cnt = 0;
	ofstream fout("/tmp/" + string_format("%d", np) + ".txt");
#pragma omp parallel for schedule(static) num_threads(4)
	REP(i, np) {
		auto est = estimate(i);
		auto truth = get_exact_s(i);
		fout << truth << endl;
		if (truth != 0 && est != numeric_limits<int>::max()) {
			double err = (double)(truth - est) / truth;
			if (err > 0)
				__sync_fetch_and_add(&pos_cnt, 1);
			if (fabs(err) > 0.05)
				print_debug("Error: %lf, truth: %d, est: %d, np: %d\n", err, truth, est, np);
#pragma omp critical
			ret += fabs(err);
		}
	}
	fout.close();
	print_debug("Error: %lf, smaller: %lf\n", ret / np, (double)pos_cnt / np);
}

void RandomChoiceEstimator::work() {
	vector<int> vst_cnt(np, 0);
	auto n = samples.size();
	vector<int> true_result(n);
#pragma omp parallel for schedule(dynamic) num_threads(2)
	REP(i, n)
		true_result[i] = bfs_all(samples[i], &vst_cnt);

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

int RandomChoiceEstimator::bfs_all(int source, vector<int>* vst_cnt) {
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
				__sync_fetch_and_add(result.data() + (*f), depth + 1);
//				result[*f] += depth + 1;

				__sync_fetch_and_add(vst_cnt->data() + (*f), 1);		// TODO
		//		(*vst_cnt)[*f] += 1;
				q.push(*f);
			}
		}
	}
	return sum;
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


LimitDepthEstimator::LimitDepthEstimator(const std::vector<std::vector<int>>& _graph, int* _degree, int _depth_max):
	SumEstimator(_graph), degree(_degree), depth_max(_depth_max)
{ }

int LimitDepthEstimator::estimate(int source) {
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
			FOR_ITR(v1, graph[v0]) {
				if (hash[*v1])
					continue;
				hash[*v1] = true;
				q.push(*v1);
			}
		}
	}
	s += nr_remain * (depth_max + 1);

	/*
	 *int real_s = get_exact_s(source);
	 *print_debug("Err: %d-%d-%.4lf\n", real_s, s, (double)fabs(real_s - s) / real_s);
	 */
	return s;
}
