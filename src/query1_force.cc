//File: query1_example.cc
//Date: Sun Mar 02 14:35:14 2014 +0800

#include "query1.h"
#include "lib/common.h"
#include "data.h"
#include <queue>
#include <vector>
using namespace std;

int force(int p1, int p2, int x) {
	vector<bool> vst(Data::nperson, false);
	deque<int> q;
	q.push_back(p1); vst[p1] = true;
	int depth = 0;
	while (int now_size = q.size()) {
		depth ++;
		REP(k, now_size) {
			int now_ele = q.front();
			q.pop_front();
			vector<ConnectedPerson>& friends = Data::friends[now_ele];
			for (vector<ConnectedPerson>::iterator it = friends.begin();
					it != friends.end(); it ++) {
				int person = it->pid;
				if (it->ncmts <= x) continue;
				if (not vst[person]) {
					if (person == p2) return depth;
					q.push_back(person);
					vst[person] = true;
				}
			}
		}
	}
	return -1;
}

void Query1Handler::add_query(int p1, int p2, int x) {
	m_assert(p1 != p2);
	int ans = force(p1, p2, x);
	this->ans.push_back(ans);
//	queries.push_back(Query1(p1, p2, x));
}

void Query1Handler::work() {

}

void Query1Handler::print_result() {
	REP(k, ans.size()) {
		printf("%d\n", ans[k]);
	}
}
