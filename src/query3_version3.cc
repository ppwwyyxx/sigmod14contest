//File: query3.cpp
//Author: Wenbo Tao.
//Method:	Inverted List.

#include "query3.h"
#include "lib/common.h"
#include "lib/Timer.h"
#include <algorithm>
#include <queue>
#include <vector>
#include <map>
using namespace std;

int sumbfs = 0;

int bfs3(int p1, int p2, int x, int h) {
	sumbfs ++;
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

        if (depth1+depth2>=h) break;

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

        if (depth1+depth2>=h) break;
	}
	return 2e9;
}

void destroy_q3_data();

void Query3Handler::add_query(int k, int h, const string& p, int index) {
	TotalTimer timer("Q3");

	Query3Calculator calc;
	calc.work(k, h, p, global_answer[index]);

	if (Data::nperson > 1e4)
		continuation->cont();

	int task_count = continuation->get_count();
	if (!task_count) {
		thread th(destroy_q3_data);		// clear useless data
		th.detach();
	}
	return;
}

void Query3Handler::work() { }

void Query3Handler::print_result() {
	FOR_ITR(it, global_answer) {
		for (auto it1 = it->begin(); it1 != it->end(); ++it1) {
			if (it1 != it->begin())	 printf(" ");
			printf("%d|%d", it1->p1, it1->p2);
		}
		printf("\n");
	}
}


void Query3Calculator::init(const string &p)
{
	//init
	pset.clear();
	answers.clear();

	pinplace.clear();
	//union sets
	FOR_ITR(it, Data::placeid[p]) {
		PersonSet sub = Data::places[*it].get_all_persons();
		PersonSet tmp; tmp.swap(pset);
		pset.resize(tmp.size() + sub.size());
		PersonSet::iterator pset_end = set_union(
				sub.begin(), sub.end(),
				tmp.begin(), tmp.end(), pset.begin());
		pset.resize(std::distance(pset.begin(), pset_end));
	}

	people.clear();
	FOR_ITR(it1, pset)
		people.push_back(it1->pid);
	sort(people.begin(), people.end());		// TODO simplify this if pset is not needed

	invList.resize(people.size());
	candidate.resize(people.size());
	i_itr.resize(people.size());
	map_itr.resize(people.size());
	zero_itr.resize(people.size());
	pool.resize(people.size());
	curRound.resize(people.size());
	first.resize(people.size());
//	oneHeap.resize(people.size());
	forsake.resize(people.size());
#ifdef GOOGLE_HASH
	for (int i = 0; i < (int) candidate.size(); i ++)
		candidate[i].set_empty_key(-1);
	for (int i = 0; i < (int) forsake.size(); i ++)
		forsake[i].set_empty_key(-1);
#endif
}

void Query3Calculator::calcInvertedList()
{
	invPeople.clear();
	for (int g = 0; g < (int) people.size(); g ++)
		invPeople[people[g]] = g;
	//
	int maxTag = 0;
	FOR_ITR(it1, pset)
	{
		int curPerson = it1->pid;
		for (TagSet::iterator i = Data::tags[curPerson].begin(); i != Data::tags[curPerson].end(); i ++)
			maxTag = max(maxTag, (*i));
	}

	vector<vector<int> > invertedList;
	invertedList.resize(maxTag + 10);

	for (int g = (int) people.size() - 1; g >= 0; g --)
	{
		TagSet curTagSet = Data::tags[people[g]];

		int curPerson = people[g];

		vector<pair<int, int> > curTags; curTags.clear();

		//modify global inverted list
		for (TagSet::iterator i = curTagSet.begin(); i != curTagSet.end(); i ++)
			invertedList[*i].push_back(curPerson), curTags.push_back(make_pair(invertedList[*i].size(), (*i)));

		//sort by size
		sort(curTags.begin(), curTags.end());
		invList[g].clear();

		//generate local inverted list
		for (int i = 0; i < (int) curTags.size(); i ++)
		{
			vector<int> r = invertedList[curTags[i].second];
			sort(r.begin(), r.end());
			invList[g].push_back(r);
		}
	}
}

void Query3Calculator::moveOneStep(int g, int h, int f)
{
	vector<vector<int> > &r = invList[g];
	int curPerson = people[g];

	for (; i_itr[g] < (int) r.size(); i_itr[g] ++)
	{
		sum ++;
		int i = i_itr[g], curLen = (int) r.size() - i;
		//prune
		if (heapSize == qk)
		{
			set<Answer3>::iterator it = answerHeap.end();
			it --;
			if (it->com_interest > curLen) return ;
		}

		if (i > curRound[g])
		{
			FOR_ITR(j, r[i])
				candidate[g][*j] ++;
			curRound[g] = i;
			map_itr[g] = r[i].begin();
		}

		//
		while (! pool[g][curLen].empty())
		{
			int cur = pool[g][curLen].top();
			pool[g][curLen].pop();
			if (bfs3(curPerson, cur, -1, h) <= h)
				insHeap(Answer3(curLen, curPerson, cur), g, f);
		}

//		if (i == (int) r.size() - 1) break;
		FOR_ITR(jj, r[i])
		{
			int j = *jj;
			if (! forsake[g].count(j))
			{
				forsake[g].insert(j);
				int tot = candidate[g][j];
				for (int p = i + 1; p < (int) r.size(); p ++)
				{
					int flag = 0;
					for (int lo = 0, hi = (int) r[p].size() - 1; lo <= hi; )
					{
						int mid = (lo + hi) / 2;
						if (r[p][mid] == j)
						{
							flag = 1; break;
						}
						if (r[p][mid] < j) lo = mid + 1; else hi = mid - 1;
					}
					if (flag) tot ++;
				}

				if (tot != (int) r.size() - i)
				{
					pool[g][tot].push(j);
					continue;
				}
				if (bfs3(curPerson, j, -1, h) > h)
					continue;
				insHeap(Answer3(curLen, curPerson, j), g, f);
				map_itr[g] ++;
				return ;
			}
		}
	}
/*
	while (! oneHeap[g].empty())
	{
		set<pair<int, int> >::iterator i = oneHeap[g].begin();
		int cur =  i->first, bar = i->second;
		oneHeap[g].erase(i);
		r[bar].erase(r[bar].begin());
		if (! r[bar].empty())
			oneHeap[g].insert(make_pair(*(r[bar].begin()), bar));
		if (! forsake[g].count(cur) && bfs3(curPerson, cur, -1, h) <= h)
			forsake[g].insert(cur), insHeap(Answer3(1, curPerson, cur), g, f);
	}
*/
}

void Query3Calculator::insHeap(Answer3 cur, int g, int f)
{
//	answerHeap.insert(cur); return ;
	if (! f)
	{
		if (heapSize < qk)
			answerHeap.insert(cur), heapSize ++;
		else
		{
			set<Answer3>::iterator it = answerHeap.end();
			it --;
			if ((*it) < cur) return ;
			answerHeap.erase(it);
			answerHeap.insert(cur);
		}
		return ;
	}
	first[g] = cur;
	return ;
}

void Query3Calculator::work(int k, int h, const string &p, std::vector<Answer3> &ans)
{
	qk = k;
	init(p);
	calcInvertedList();
	//Arsenal is the champion!!
	answerHeap.clear();

//#pragma omp parallel for schedule(dynamic)
	for (int g = 0; g < (int) people.size(); g ++)
	{
		i_itr[g] = 0;
		zero_itr[g] = g + 1;
		curRound[g] = -1;

		candidate[g].clear();
		forsake[g].clear();
		pool[g].resize(invList[g].size() + 2);
		forsake[g].insert(people[g]);
/*
		//init oneHeap
		for (int i = 0; i < (int) invList[g].size(); i ++)
			oneHeap[g].insert(make_pair(*(invList[g][i].begin()), i));*/
		moveOneStep(g, h, 0);
	}
//	return ;

//	cout << people.size() << " " << sum << " " << sumbfs << endl;
//	return ;
//	for (int i = 0; i < (int) people.size(); i ++)
//		if (first[i].p1 != first[i].p2)
//			answerHeap.insert(first[i]);
//
	set<pair<int, int> > dup;
	dup.clear();
	vector<Answer3> tmp; tmp.clear();
	for (int lp = 1; lp <= k; lp ++)
	{
		if (answerHeap.empty()) break;
		set<Answer3>::iterator i = answerHeap.begin();
		tmp.push_back(*i);
		dup.insert(make_pair(i->p1, i->p2));
		moveOneStep(invPeople[i->p1], h, 0);
		answerHeap.erase(i);
	}

	for (int i = 0; i < (int) people.size() && (int) tmp.size() < k; i ++)
		for (int j = i + 1; j < (int) people.size() && (int) tmp.size() < k; j ++)
			if (! dup.count(make_pair(people[i], people[j])) && bfs3(people[i], people[j], -1, h) <= h)
				dup.insert(make_pair(people[i], people[j])), tmp.push_back(Answer3(0, people[i], people[j]));
	// TODO count, insert

	ans = move(tmp);
}

