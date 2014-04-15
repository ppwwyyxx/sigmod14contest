//class : binary read

#pragma once
#include <string>
#include "tasty_bread.h"
#include "lib/hash_lib.h"
#include "lib/Timer.h"
#include "lib/debugutils.h"
#include <vector>


typedef unsigned long long ULL;
class bread
{
public:
	std::vector<std::vector<int> > people;
	std::vector<int> owner;
	char* ptr, *buf_end;
	size_t size;
	tasty_bread tasty;

	int n_vst;

	int n_offset;

	void init(const std::string &dir);
	bool check(int a, int b, int threshold);
	bool check_oneside(int a, int b, int threshold);

	~bread() {
		tasty.destroy();
	}

//	std::vector<unordered_map<int, unsigned long long>> cache;
	std::vector<int> cache;
	std::vector<unordered_map<int, int>> c2;
	int get_second(unsigned long long cid);
};

