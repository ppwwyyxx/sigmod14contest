#include "query2.h"
#include "data.h"
#include "lib/debugutils.h"
#include <vector>
#include <queue>
#include <cstdio>

using namespace std;

struct HeapEle {
    int range;      // defined in the problem
    int tag_id;
    HeapEle(int _range, int _tag_id) 
        :range(_range), tag_id(_tag_id) {}

    bool operator <(const HeapEle &b) const {
        return range > b.range; 
    }
};

void Query2Handler::add_query(int k, int d) {
    priority_queue<HeapEle> heap;
    print_debug("person number = %d\n", Data::person_in_tags.size());
    for (int i=0; i<Data::person_in_tags.size(); i++) {
        //if the person is here~
        if (i == 55) {
            int j = i;
            int k = j;
        }
        vector<bool> person(Data::nperson, false);
        for (int j=0; j<Data::person_in_tags[i].size(); j++)
            if (Data::birthday[Data::person_in_tags[i][j]]>=d) {
                person[Data::person_in_tags[i][j]] = true;
                if (i == 55) 
                    print_debug("%d possible", Data::person_in_tags[i][j]);
            }
        //if the person is arrived
        vector<bool> arrived(Data::nperson, false);

        int max = 0;
        for (int j=0; j<Data::person_in_tags[i].size(); j++) {
            int now = Data::person_in_tags[i][j];
            if (arrived[now] || Data::birthday[now]<d) continue;
            arrived[now] = true;
            //BFS
            int st = 0;
            vector<int> q(1, now);
            for (;st<q.size(); st++) {
                int person_now = q[st];
                for (int k=0; k<Data::friends[person_now].size(); k++) {
                    int person_new = Data::friends[person_now][k].pid;
                    //print_debug("%d %d\n",person_new, Data::birthday[person_new]);
                    if (arrived[person_new] || (!person[person_new])) continue;
                    arrived[person_new] = true;
                    q.push_back(person_new);
                }
            }
            if ((q.size()) > max) max = q.size();
        }
        if (max > 0 && (heap.size()<k || max>heap.top().range)) {
            if (heap.size() == k) heap.pop();
            heap.push(HeapEle(max, i));
        }
    }

    vector<int> ans_now(k);
    print_debug("heap size=%d\n", heap.size());
    for (int i=0; i<k; i++) {
        HeapEle now = heap.top();
        heap.pop();
        ans_now[k-i-1] = now.tag_id;
        print_debug("%d %d ", now.range, now.tag_id);
    }
    print_debug("\n");
    ans.push_back(ans_now);
}

void Query2Handler::work() {}

void Query2Handler::print_result() {
    for (int i=0; i<ans.size(); i++) {
        for (int j=0; j<ans[i].size(); j++) {
            if (j>0) printf(" ");
            printf("%s", Data::tag_name[ans[i][j]].c_str());
        }
        printf("\n");
    }
}
