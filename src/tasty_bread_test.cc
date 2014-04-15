/*
 * $File: tasty_bread_test.cc
 * $Date: Tue Apr 15 21:58:07 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#define DEBUG_BREAD

#include "tasty_bread.h"

#include <iostream>

int main() {
	tasty_bread bread;
	bread.init("data/1000k");
	size_t first;

	size_t nr_iter_total = 0;

	std::cout << "start running" << std::endl;
	while (std::cin >> first) {
		std::cout << bread.get_second(first) << std::endl;
		std::cout << bread.last_nr_iter << std::endl;
		nr_iter_total += bread.last_nr_iter;
	}

	std::cout << "nr_iter_total: " << nr_iter_total << std::endl;
	return 0;
}

/**
 * vim: syntax=cpp11 foldmethod=marker
 */

