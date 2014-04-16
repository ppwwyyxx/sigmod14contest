//File: test.cc
//Date: Sat Apr 12 16:17:00 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <vector>
#include "bitset.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <limits>
#include "Timer.h"
using namespace std;
#define REP(x, y) for (auto x = decltype(y){0}; x < (y); x ++)
#define REPL(x, y, z) for (auto x = decltype(z){y}; x < (z); x ++)
#define REPD(x, y, z) for (auto x = decltype(z){y}; x >= (z); x --)
#define P(a) std::cout << (a) << std::endl
#define PP(a) std::cout << #a << ": " << (a) << std::endl
#define PA(arr) \
	do { \
		std::cout << #arr << ": "; \
		std::copy(begin(arr), end(arr), std::ostream_iterator<std::remove_reference<decltype(arr)>::type::value_type>(std::cout, " ")); \
		std::cout << std::endl;  \
	} while (0)

int main() {
	Timer tt;
	int n = 200000;
	/*
	 *int mem = n * n / 8;
	 *char* ptr = (char*)calloc(mem, 1);
	 *cout << tt.get_time();
	 *int t;
	 *cin >> t;
	 *if (t != 1) {
	 *    cout << ptr[t];
	 *}
	 */
	/*
	 *std::vector<Bitset> s;
	 *s.reserve(n);
	 *int len = get_len_from_bit(n);
	 *REP(i, n)
	 *    s.emplace_back(len);
	 */
	BitBoard B(n);
	cout << tt.get_time();
	int t;
	cin >> t;
	if (t != 1) {
		B.bitsets[t].set(t);
	}
}
