#include <stdlib.h>
#include <algorithm>
#include <mutex>
#include <cstdio>
#include <fstream>
#include <thread>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "bread.h"

#include "lib/debugutils.h"
#include "lib/utils.h"
#include "lib/Timer.h"
#include "lib/common.h"
#include "read.h"
#include "cache.h"
#include "data.h"
#include "lib/fast_read.h"
using namespace std;

const int CACHE_FIRST_LEN = 20;

void bread::init(const std::string &dir)
{
	static char buffer[BUFFER_LEN];
	people.resize(1000010);
	safe_open(dir + "/comment_hasCreator_person.csv");
	ptr = buffer, buf_end = ptr + 1;
	n_vst = 0;

	READ_TILL_EOL();
	unsigned long long cid;
	int pid;
	if (Data::nperson > 9000)
		owner.reserve(20100000);
	while (true) {
		READ_ULL(cid);
		if (buffer == buf_end) break;
		READ_INT(pid);
		m_assert(cid % 10 == 0);
		m_assert(cid / 10 == owner.size());
		people[pid].emplace_back(cid);
		owner.emplace_back(pid);
	}
	fclose(fin);
	tasty.init(dir);

	unsigned long long maxv = tasty.max_v;
	m_assert(maxv);
	/*
	 *int n_digit = 64 - __builtin_clzll(maxv);
	 *PP(n_digit);PP(maxv);
	 *n_offset = n_digit < CACHE_FIRST_LEN ? 0 : n_digit - CACHE_FIRST_LEN;
	 */
	cache.resize(maxv, 0);
	c2.resize(Data::nperson);
#ifdef GOOGLE_HASH
	FOR_ITR(itr, c2)
		itr->set_empty_key(-1);
#endif
	/*
	 *FOR_ITR(itr, cache)
	 *    itr->set_empty_key((1 << (n_offset + 1)) - 1);
	 */
}

#include <unordered_set>
bool bread::check(int a, int b, int threashold) {
	/*
	 *if (a < b) swap(a, b);
	 *static std::unordered_set<pair<int, int>> c;
	 *c.emplace(a, b);
	 *PP(c.size());
	 */

	n_vst ++;
	if (n_vst % 10000 == 0)
		PP(n_vst);
    return check_oneside(a, b, threashold) && check_oneside(b, a, threashold);
}

bool bread::check_oneside(int a, int b, int threshold)
{
	auto& m = c2[a][b];
	if (m > threshold)
		return true;
	int tot = 0;
	auto& pp = people[a];
	for (int i = 0; i < (int) pp.size(); i ++)
	{
		unsigned long long cid = pp[i];
		int owner2 = get_second(cid);
		if (owner2 == b) {
			tot ++;
			if (tot > threshold) {
				m = tot;
				return true;
			}
		}
	}
	if (tot > m)
		m = tot;
	return false;
}


int bread::get_second(unsigned long long cid) {
	/*
	 *int idx = cid >> n_offset,
	 *    offset = cid & ((1 << n_offset) - 1);
	 */

	int* pp = cache.data() + cid;
	if (*pp == 0) {
		ULL t = tasty.get_second(cid);
		if (t == tasty_bread::NOT_FOUND)
			*pp = 1e9;
		else
			*pp = owner[t / 10];
	}
	return *pp;
}

