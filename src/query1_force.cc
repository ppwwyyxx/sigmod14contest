//File: query1.cpp
//Date: Wed Mar 12 00:38:24 2014 +0800

#include "query1.h"
#include "lib/common.h"
#include "data.h"
#include <cstdio>
#include <queue>
#include <algorithm>
#include <vector>
using namespace std;

/*
 *int force(int p1, int p2, int x) {			// 10k: 0.68 sec / 1500queries
 *    if (p1 == p2) return 0;
 *    vector<bool> vst(Data::nperson, false);
 *    deque<int> q;
 *    q.push_back(p1); vst[p1] = true;
 *    int depth = 0;
 *    while (auto now_size = q.size()) {
 *        depth ++;
 *        REP(k, now_size) {
 *            int now_ele = q.front();
 *            q.pop_front();
 *            vector<ConnectedPerson>& friends = Data::friends[now_ele];
 *            for (vector<ConnectedPerson>::iterator it = friends.begin();
 *                    it != friends.end(); it ++) {
 *                int person = it->pid;
 *                if (it->ncmts <= x) break;		// because friends are sorted by ncmts
 *                if (not vst[person]) {
 *                    if (person == p2) return depth;
 *                    q.push_back(person);
 *                    vst[person] = true;
 *                }
 *            }
 *        }
 *    }
 *    return -1;
 *}
 */

int bfs2(int p1, int p2, int x) {			// 10k: 0.014sec / 1500queries
	if (p1 == p2) return 0;
	vector<bool> vst1(Data::nperson, false);
	vector<bool> vst2(Data::nperson, false);
	deque<int> q1, q2;
	q1.push_back(p1); vst1[p1] = true;
	q2.push_back(p2); vst2[p2] = true;
	int depth1 = 0, depth2 = 0;
	while (true) {
		size_t s1 = q1.size(), s2 = q2.size();
		if (!s1 or !s2) break;

		depth1 ++;
		REP(k, s1) {
			int now_ele = q1.front();
			q1.pop_front();
			auto& friends = Data::friends[now_ele];
			for (auto it = friends.begin(); it != friends.end(); it ++) {
				int person = it -> pid;
				if (it->ncmts <= x) break;
				if (not vst1[person]) {
					if (vst2[person]) return depth1 + depth2;
					q1.push_back(person);
					vst1[person] = true;
				}
			}
		}

		depth2 ++;
		REP(k, s2) {
			int now_ele = q2.front();
			q2.pop_front();
			auto& friends = Data::friends[now_ele];
			for (auto it = friends.begin(); it != friends.end(); it ++) {
				int person = it -> pid;
				if (it->ncmts <= x) break;
				if (not vst2[person]) {
					if (vst1[person]) return depth1 + depth2;
					q2.push_back(person);
					vst2[person] = true;
				}
			}
		}
	}
	return -1;
}

void Query1Handler::add_query(const Query1& q) {
	int ans = bfs2(q.p1, q.p2, q.x);
	this->ans.push_back(ans);
}

void Query1Handler::pre_work() {
	REP(i, Data::nperson) {
		sort(Data::friends[i].begin(), Data::friends[i].end());
	}
}

void Query1Handler::work() {}

void Query1Handler::print_result() {
	REP(k, ans.size()) {
		printf("%d\n", ans[k]);
	}
}
