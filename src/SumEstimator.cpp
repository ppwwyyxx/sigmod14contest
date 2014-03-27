//File: SumEstimator.cpp
//Date: Thu Mar 27 16:33:31 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "SumEstimator.h"
#include "lib/common.h"
#include <queue>
#include <limits>
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

void UnionSetDepthEstimator::work() {
	GuardedTimer gg("work");
	auto& out_s_prev = *s_prev;
	auto& out_s = *s;

	for (int k = 2; k <= depth_max; k ++) {
		// CANNOT USE OMP HERE!
#pragma omp parallel for schedule(dynamic) num_threads(4)
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
