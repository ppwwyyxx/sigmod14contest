#include "job_wrapper.h"

using namespace std;

Query1Handler q1;
Query2Handler q2;
Query3Handler q3;
Query4Handler q4;

vector<Query1> q1_set;
vector<Query2> q2_set;
vector<Query3> q3_set;
vector<Query4> q4_set;

void add_all_query(int type) {
	Timer timer;
	switch (type) {
		case 1:
			q1.continuation = std::make_shared<FinishTimeContinuation>(q1_set.size(), "q1 finish time");
			for (size_t i = 0; i < q1_set.size(); i ++)
				q1.add_query(q1_set[i], i);
			break;
		case 2:
			FOR_ITR(itr, q2_set) {
				q2.add_query(*itr);
			}
			break;
		case 3:
			m_assert(false);
			break;
		case 4:
			m_assert(false);
			break;
	}
}
