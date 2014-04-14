//File: query2.h

#pragma once
#include <mutex>
#include <vector>

#include "lib/finish_time_continuation.h"

struct Query2 {
	int k, d, qid;
	Query2(int _k, int _d, int _qid):
		k(_k), d(_d), qid(_qid) {}
    bool operator <(const Query2 &b) const {
        return d > b.d;
    }
};

class Query2Handler {
	public:
		void add_query(const Query2& q);

		void work();

		void print_result();

		std::shared_ptr<FinishTimeContinuation> continuation;

	protected:
        std::vector<std::vector<std::string>> all_ans;
};
