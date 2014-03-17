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


int bfs3(int p1, int p2, int x, int h) {
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

int get_common_tag(int p1, int p2)
{
	TagSet &t1 = Data::tags[p1], &t2 = Data::tags[p2];
	int ret = 0;
	TagSet::iterator it1 = t1.begin(), it2 = t2.begin();
	while (it1 != t1.end() && it2 != t2.end())
	{
		if (*it1 == *it2)
			++ret;
		if (*it1 < *it2)
			++it1;
		else
			++it2;
	}
	//printf("%d and %d : %d\n", p1, p2, ret);
	return ret;
}

void Query3Handler::add_query(int k, int h, const string& p) {
	{
		lock_guard<mutex> lg(mt_friends_data_changing);
		friends_data_reader ++;
	}
	TotalTimer timer("Q3");
	//printf("\t\t\t%s\n", p.c_str());
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

	vector<vector<int> > invertedList;
	vector<int> people;
	invertedList.resize(6000);			//This number needs to be fixed.
	for (int i = 0; i < 5000; i ++)		//This number as well.
		invertedList[i].clear();

	people.clear();
	FOR_ITR(it1, pset)
		people.push_back(it1->pid);
	sort(people.begin(), people.end());

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
		invList[curPerson].clear();

		//generate local inverted list
		for (int i = 0; i < (int) curTags.size(); i ++)
		{
			vector<int> &r = invertedList[curTags[i].second];
			invList[curPerson].push_back(set<int>(r.begin(), r.end()));
		}
	}

	int totPair = 0;
	//Arsenal is the champion!!

	for (int g = 0; g < (int) people.size(); g ++)
	{
		int curPerson = people[g];						//current person id
		vector<set<int> > &r = invList[curPerson];		//current inverted list
		ansList[curPerson].clear();						//This method should be discarded if k is too large.

		//enumerate the number of common tags from 2 to r.size()
		map<int, int> candidate; candidate.clear();
		set<int> forsake; forsake.clear(); forsake.insert(curPerson);
		for (int i = 0; i < (int) r.size() - 1; i ++)
		{
			FOR_ITR(j, r[i])
				candidate[*j] ++;
			FOR_ITR(j, candidate)
				if (! forsake.count(j->first))
				{
					int tot = j->second;
					for (int p = i + 1; p < (int) r.size(); p ++)
						if (r[p].count(j->first)) tot ++;
					if (tot != (int) r.size() - i) continue;
					if (bfs3(curPerson, j->first, -1, h) > h) continue;

					ansList[curPerson].push_back(Answer3((int) r.size() - i, curPerson, j->first));
					forsake.insert(j->first);
				}
		}

		//only 1 common tag

		set<pair<int, int> > ids; ids.clear();
		for (int i = 0; i < (int) r.size(); i ++)
			for ( ; ! r[i].empty(); )
				if (! forsake.count(*(r[i].begin())))
				{
					ids.insert(make_pair(*(r[i].begin()), i));
					r[i].erase(r[i].begin());
					break;
				}
				else r[i].erase(r[i].begin());

//		cout << "person : " << curPerson << endl;
		for ( ; (int) ansList[curPerson].size() < k; )
		{
			if (ids.empty()) break;
//			cout << "size : " << ansList[curPerson].size() << endl;
			set<pair<int, int> >::iterator winner = ids.begin();

			if (! forsake.count(winner->first) && bfs3(curPerson, winner->first, -1, h) <= h)
				ansList[curPerson].push_back(Answer3(1, curPerson, winner->first));
			ids.erase(winner);
			forsake.insert(winner->first);

			for (int i = winner->second; ! r[i].empty(); )
				if (! forsake.count(*(r[i].begin())))
				{
					ids.insert(make_pair(*(r[i].begin()), i));
					r[i].erase(r[i].begin());
					break;
				}
				else r[i].erase(r[i].begin());
		}

		//0 common interest

		for (int i = g + 1; i < (int) people.size() && totPair + (int) ansList[curPerson].size() < k; i ++)
			if (! forsake.count(people[i]) && bfs3(curPerson, people[i], -1, h) <= h)
				ansList[curPerson].push_back(Answer3(0, curPerson, people[i]));

		totPair += (int) ansList[curPerson].size();
	}
	vector<Answer3> tmp1, tmp2;
	tmp1.clear(), tmp2.clear();
	FOR_ITR(it, pset) {
		int curPerson = it->pid;
		for (int i = 0; i < (int) ansList[curPerson].size(); i ++)
			tmp1.push_back(ansList[curPerson][i]);
	}

	sort(tmp1.begin(), tmp1.end());
	for (int i = 0; i < min((int)tmp1.size(), k); i ++)
		tmp2.push_back(tmp1[i]);
	global_answer.push_back(tmp2);

	friends_data_reader --;
	cv_friends_data_changing.notify_one();
	return ;
}

void Query3Handler::work() { }

void Query3Handler::print_result() {
	for (auto it = global_answer.begin(); it != global_answer.end(); ++it)
	{
		for (auto it1 = it->begin(); it1 != it->end(); ++it1) {
			if (it1 != it->begin())	 printf(" ");
			printf("%d|%d", it1->p1, it1->p2);
		}
		printf("\n");
	}
}
