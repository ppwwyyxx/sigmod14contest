/*
 * $File: tasty_bread_test.cc
 * $Date: Tue Apr 15 23:10:15 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#define DEBUG_BREAD

#include "tasty_bread.h"

#include <cstdio>
#include <iostream>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <data dir>\n", argv[0]);
		return 1;
	}
	tasty_bread bread;
	bread.init(argv[1]);
	size_t first;

	size_t nr_iter_total = 0;

	std::cout << "start running" << std::endl;
	size_t nr_test = 0;
	while (std::cin >> first) {
		nr_test ++;
		tasty_bread::int_t second = bread.get_second(first);
//        std::cout << second << std::endl;
//        std::cout << bread.last_nr_iter << std::endl;
		nr_iter_total += bread.last_nr_iter;
	}

	std::cout << "nr_iter_total: " << nr_iter_total << std::endl;
	std::cout << "ave iter: " << nr_iter_total / (double)nr_test << std::endl;
	return 0;
}

/**
 * vim: syntax=cpp11 foldmethod=marker
 */

