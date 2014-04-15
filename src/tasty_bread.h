/*
 * $File: tasty_bread.h
 * $Date: Tue Apr 15 22:13:56 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#pragma once
#include <vector>
#include <string>

class tasty_bread {
	public:
		void init(const std::string &dir);
		void destroy();

		typedef size_t int_t;

		static const int_t NOT_FOUND = ~0llu;

		// return tasty_bread::NOT_FOUND if no such number
		int_t get_second(int_t first);

#ifdef DEBUG_BREAD
		size_t last_nr_iter;
#endif

	protected:
		size_t size;
		size_t real_size;
		char *buf_begin;
		char *ptr;
		char *buf_end;

		int_t min_v, max_v;

		size_t guess_position(
				size_t left, size_t right,
				int_t left_v, int_t right_v, int_t v);

		int_t get_int(char *&p, char delim); // modify p to point to where delim is or buf_end
		void move_to_line_start(char *&p);
};

/**
 * vim: syntax=cpp11 foldmethod=marker
 */
