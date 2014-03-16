//File: cache.cpp
//Date: Sun Mar 16 16:20:57 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "data.h"
#include "lib/common.h"
#include "lib/Timer.h"
#include "cache.h"
#include <mutex>
using namespace std;

vector<CachedNeighbor> CachedNeighbor::data;
std::vector<bool> CachedNeighbor::cached1, CachedNeighbor::cached2;

const NeighborT& CachedNeighbor::get_neighbor_1(int person) {
	// No lock, ok?

	if (cached1[person]) return data[person].neighbor_1;

	NeighborT ret;

#ifdef GOOGLE_HASH
	ret.set_empty_key(-1);
#endif
	auto& friends = Data::friends[person];
	FOR_ITR(itr, friends) {
		ret.insert(itr->pid);
	}

	auto& c = data[person];
	c.neighbor_1 = move(ret);
	cached1[person] = true;
	return c.neighbor_1;
}

const NeighborT& CachedNeighbor::get_neighbor_2(int person) {
	// No lock, ok?

	if (cached2[person]) return data[person].neighbor_2;

	const auto& n1 = get_neighbor_1(person);

	NeighborT ret;
#ifdef GOOGLE_HASH
	ret.set_empty_key(-1);
#endif
	FOR_ITR(itr, n1) {
		auto& f = Data::friends[*itr];
		FOR_ITR(p, f) {
			if (p->pid != person && n1.count(p->pid) == 0)
				ret.insert(p->pid);
		}
	}

	auto& c = data[person];
	c.neighbor_2 = move(ret);
	cached2[person] = true;
	return c.neighbor_2;
}

int intersect_cnt(const NeighborT& n, const unordered_set<int>& large) {
	TotalTimer timer("Intersect");
	int ret = 0;
	if (n.size() < large.size()) {
		FOR_ITR(itr, n)
			if (large.count(*itr))
				ret ++;
	} else {
		FOR_ITR(itr, large)
			if (n.count(*itr))
				ret ++;
	}
	return ret;
}
