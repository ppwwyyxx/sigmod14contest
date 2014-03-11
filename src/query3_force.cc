//File: query3.cpp
//Date: Wed Mar 12 13:33:50 2014 +0800
//Author: Junbang Liang <williamm2006@126.com>
//Method:	Online. For each query, find the subset of persons included.
//			And do bfs for each person, adding required pairs. Finally sort and output.
//			Complexity: O(N^2 * ntags * logN)

#include "query3.h"
#include <algorithm>
#include <queue>
#include <vector>
using namespace std;

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

void Query3Handler::bfs(int pid, int h)
{
	vector<bool> vst(Data::nperson, false);
	deque<int> q;
	q.push_back(pid);
	vst[pid] = true;
	int depth = 0;
	while (depth < h)
	{
		int now_size = q.size();
		while (now_size--)
		{
			int cur_id = q.front();
			q.pop_front();
			vector<ConnectedPerson>& friends = Data::friends[cur_id];
			for (vector<ConnectedPerson>::iterator it = friends.begin(); it != friends.end(); ++it)
			{
				int person = it->pid;
				if (!vst[person])
				{
					q.push_back(person);
					vst[person] = true;
					if (pinplace.find(person) != pinplace.end() && person > pid)
						answers.push_back(Answer3(get_common_tag(pid, person), pid, person));
				}
			}
		}
		depth++;
	}
}

void Query3Handler::add_query(int k, int h, const string& p) {
	//printf("\t\t\t%s\n", p.c_str());
	//query input
	//init
	pset.clear();
	answers.clear();
	pinplace.clear();
	//union sets
	for (vector<int>::iterator it = Data::placeid[p].begin(); it != Data::placeid[p].end(); ++it)
	{
		PersonSet sub = Data::places[*it].get_all_persons();
		PersonSet tmp; tmp.swap(pset);
		pset.resize(tmp.size() + sub.size());
		PersonSet::iterator pset_end = set_union(
				sub.begin(), sub.end(),
				tmp.begin(), tmp.end(), pset.begin());
		pset.resize(std::distance(pset.begin(), pset_end));
	}
	//pickup people
	for (PersonSet::iterator it = pset.begin(); it != pset.end(); ++it)
	{
		pinplace.insert(it->pid);
		//printf("%d ", it->pid);
	}
	//printf("\n");
	//bfs
	for (PersonSet::iterator it = pset.begin(); it != pset.end(); ++it)
		bfs(it->pid, h);
	//sort and output
	sort(answers.begin(), answers.begin() + answers.size());
	vector<Answer3> tmp;
	for (vector<Answer3>::iterator it = answers.begin(); it != answers.end() && (k--); ++it)
		tmp.push_back(*it);
	global_answer.push_back(tmp);
}

void Query3Handler::work() {

}

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
