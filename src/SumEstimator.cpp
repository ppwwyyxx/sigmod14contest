//File: SumEstimator.cpp
//Date: Tue Mar 25 13:59:55 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "SumEstimator.h"
#include "lib/common.h"
#include <queue>
#include <limits>
using namespace std;

long long SumEstimator::get_exact_s(int source) {
	std::vector<bool> hash(np);
	std::queue<int> q;
	hash[source] = true;
	q.push(source);
	long long s = 0;
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
	int n = samples.size();
	vector<long long> true_result(n);
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

long long RandomChoiceEstimator::bfs_all(int source, vector<int>& vst_cnt) {
	long long sum = 0;
	queue<int> q;
	vector<bool> vst(np);
	q.push(source);
	for (int depth = 0; !q.empty(); depth ++) {
		int qsize(q.size());
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

long long RandomChoiceEstimator::estimate(int i) { return result[i]; }
