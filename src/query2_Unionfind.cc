#include "query2.h"
#include "data.h"
#include "lib/debugutils.h"
#include "heap.h"
#include "lib/utils.h"
#include <vector>
#include <cstdio>
#include <algorithm>
#include "lib/hash_lib.h"

extern vector<Query2> q2_set;
#define queries q2_set

using namespace std;

namespace {
	// union find set && sum, firends sorted
	vector<vector<int>> f, sum, myfriends;
	// ans heap
	MyHeap heap;
	// map   myhash[tag][person] = ith
	vector<unordered_map<int, int>> myhash;
}

bool cmp_d(int a, int b) {
	return (Data::birthday[a] > Data::birthday[b] ||
			(Data::birthday[a] == Data::birthday[b] && a < b));
}

void Query2Handler::add_query(const Query2 &) {}

vector<int> find_ans(int k) {
	vector<int> ans_now;
	vector<HeapEle> tmp;
	for (int j = 0; j < k; j++) {
		tmp.push_back(heap.get());
		ans_now.push_back(tmp.back().tag_id);
		// print_debug("%d ", tmp.back().range);
	}
	for (int j = 0; j < k; j++) heap.insert(tmp[j].tag_id, tmp[j].range);
	return ans_now;
}

int getf(int a, vector<int> &f) {
	int b = a;
	while (f[a] != a) a = f[a];
	int c;
	while (f[b] != a) {
		c = f[b];
		f[b] = a;
		b = c;
	}
	return a;
}

void Query2Handler::work() {
	vector<vector<int>> ans;
	f.resize(Data::ntag);
	sum.resize(Data::ntag);
	myfriends.resize(Data::nperson);
	heap.resize(Data::ntag);
	myhash.resize(Data::ntag);
#ifdef GOOGLE_HASH
	for (auto it = myhash.begin(); it != myhash.end(); it++)
		it->set_empty_key(-1);
#endif

	// sort quries by d
	sort(queries.begin(), queries.end());
	// sort persons by d
	vector<int> person;
	for (int i = 0; i < Data::nperson; i++)
		person.push_back(i);
	sort(person.begin(), person.end(), cmp_d);
	// sort person in tags by d
	for (int i = 0; i < (int)Data::person_in_tags.size(); i++) {
		sort(Data::person_in_tags[i].begin(), Data::person_in_tags[i].end(),
				cmp_d);
		for (int j = 0; j < (int)Data::person_in_tags[i].size(); j++)
			myhash[i][Data::person_in_tags[i][j]] = j;
	}
	// sort friends by d
	for (int i = 0; i < Data::nperson; i++) {
		for (int j = 0; j < (int)Data::friends[i].size(); j++)
			myfriends[i].push_back(Data::friends[i][j].pid);
		sort(myfriends[i].begin(), myfriends[i].end(), cmp_d);
	}

	// ans heap
	for (int i = 0; i < Data::ntag; i++) heap.insert(i, 0);

	int queryP = 0;
	for (int i = 0; i < Data::nperson && queryP < (int)queries.size(); i++) {
		int person_now = person[i];
		// answer queries
		for (; queryP < (int)queries.size() &&
				queries[queryP].d > Data::birthday[person_now];
				queryP++)
			ans.push_back(find_ans(queries[queryP].k));
		if (queryP == (int)queries.size()) break;

		// new person in tag
		for (auto j = Data::tags[person_now].begin();
				j != Data::tags[person_now].end(); j++) {
			int tag_now = *j;
			f[tag_now].push_back((int)f[tag_now].size());
			sum[tag_now].push_back(1);
			heap.change(tag_now, 1);
		}
		for (int j = 0; j < (int)myfriends[person_now].size(); j++) {
			int friend_now = myfriends[person_now][j];
			if (cmp_d(person_now, friend_now)) break;
			for (auto k = Data::tags[friend_now].begin();
					k != Data::tags[friend_now].end(); k++) {
				int tag_now = *k;
				if (Data::tags[person_now].find(tag_now) ==
						Data::tags[person_now].end())
					continue;
				int p = myhash[tag_now][friend_now];
				int set_now = getf(p, f[tag_now]);
				if (set_now == f[tag_now][f[tag_now].size() - 1U]) continue;
				sum[tag_now][(int)f[tag_now].size() - 1] +=
					sum[tag_now][set_now];
				f[tag_now][set_now] = (int)f[tag_now].size() - 1;
				heap.change(tag_now, sum[tag_now][f[tag_now].size() - 1U]);
			}
		}
	}
	for (; queryP < (int)queries.size(); queryP++)
		ans.push_back(find_ans(queries[queryP].k));

	int nquery = (int) queries.size();
	vector<vector<int>> final_ans(nquery);
	m_assert((int)ans.size() == nquery);
	for (int i = 0; i < nquery; i++)
		final_ans[queries[i].qid] = ans[i];

	all_ans.resize(nquery);
	REP(i, nquery) {
		auto l = final_ans[i].size();
		all_ans[i].reserve(l);
		REP(j, l)
			all_ans[i].emplace_back(Data::tag_name[final_ans[i][j]]);
	}

	if (Data::nperson > 1e4)
		continuation->cont();

	// clean q2 data
	delete[] Data::birthday;
	FreeAll(Data::person_in_tags);
	FreeAll(f);
	FreeAll(sum);
	FreeAll(myhash);
	FreeAll(myfriends);
	heap.free();

	q2_finished = true;
	q2_finished_cv.notify_all();
}

void Query2Handler::print_result() {
	for (int i = 0; i < (int)queries.size(); i++) {
		for (int j = 0; j < (int)all_ans[i].size(); j++) {
			if (j > 0) printf(" ");
			printf("%s", all_ans[i][j].c_str());
			}
		printf("\n");
	}
}
