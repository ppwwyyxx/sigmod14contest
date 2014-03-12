//File: query3_version2.cc
//Date: Wed Mar 12 13:43:00 2014 +0800
//Author: Junbang Liang <williamm2006@126.com>, Han Zhao <nikifor383@gmail.com>
//Method:	Online. For each query, find the subset of persons included.
//			If the number of persons is bigger than n/10,
//				do bfs for each person, adding required pairs. Finally sort and output.
//				Complexity: O(NM * ntags * logN)
//			else,
//				for each pair of persons, calculate the common interests and sort.
//				Then do 2-way bfs to check out the answers.
//				Complexity: O(N^2 * ntags * 2-way-bfs)

#include "query3.h"
#include "lib/common.h"
#include <algorithm>
#include <queue>
#include <vector>
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

void Query3Handler::bfs(int pid, int h, int k)
{
	vector<bool> vst(Data::nperson, false);
	deque<int> q;
	q.push_back(pid);
	vst[pid] = true;
	int depth = 0;
	while (depth < h)
	{
		auto now_size = q.size();
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
					if (pinplace.find(person) != pinplace.end() && person > pid){
                        Answer3 ans(get_common_tag(pid, person), pid, person);
                        int p = 0;
                        for (p = 0; p < k; p++){
                            if (ans < answers[p]) break;
                        }
                        if (p==k) continue;
                        for (int i = k-1; i > p; i--){
                            answers[i] = answers[i-1];
                        }
                        answers[p] = ans;
						//answers.push_back(Answer3(get_common_tag(pid, person), pid, person));
                    }
				}
			}
		}
		depth++;
	}
}

void Query3Handler::add_query(int k, int h, const string& p) {
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

    //printf("SIZE : %d\n", pset.size());

    vector<Answer3> tmp;
    if (pset.size() * 10 > Data::nperson){
        //pickup people
        for (int i=0; i<k; i++) answers.push_back(Answer3(0,2e9,2e9));
        for (PersonSet::iterator it = pset.begin(); it != pset.end(); ++it)
        {
            pinplace.insert(it->pid);
            //printf("%d ", it->pid);
        }
        //printf("\n");
        //bfs
        for (PersonSet::iterator it = pset.begin(); it != pset.end(); ++it){
            if (it->ntags < answers[k-1].com_interest) continue;
            bfs(it->pid, h, k);
        }
		while (answers[answers.size() - 1].p1 == 2000000000)
			answers.erase(answers.end() - 1);
        global_answer.push_back(answers);
        //sort and output

//        sort(answers.begin(), answers.begin() + answers.size());
//        for (vector<Answer3>::iterator it = answers.begin(); it != answers.end() && (k--); ++it)
//            tmp.push_back(*it);

    } else {
        for (PersonSet::iterator it1 = pset.begin(); it1 != pset.end(); ++it1) {
            for (PersonSet::iterator it2 = it1+1; it2 != pset.end(); ++it2) {
                int cts = get_common_tag(it1->pid, it2->pid);
                Answer3 ans(cts, it1->pid, it2->pid);
                answers.push_back(ans);
            }
        }
        sort(answers.begin(), answers.begin() + answers.size());
        tmp.clear();
        for (int i = 0; i < answers.size() && tmp.size() < k; i++){
            if (bfs3(answers[i].p1, answers[i].p2, -1, h) > h) continue;
            tmp.push_back(answers[i]);
        }
        global_answer.push_back(tmp);
    }
}

void Query3Handler::work() { }

void Query3Handler::print_result() {
	lock_guard<mutex> lg(mt_work_done);
	for (auto it = global_answer.begin(); it != global_answer.end(); ++it)
	{
		for (auto it1 = it->begin(); it1 != it->end(); ++it1) {
			if (it1 != it->begin())	 printf(" ");
			printf("%d|%d", it1->p1, it1->p2);
		}
		printf("\n");
	}
}
