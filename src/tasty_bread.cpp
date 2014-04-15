/*
 * $File: tasty_bread.cpp
 * $Date: Tue Apr 15 23:20:42 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <cassert>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib/fast_read.h"
#include "tasty_bread.h"

void tasty_bread::init(const std::string &dir) {
	int fd = open((dir + "/comment_replyOf_comment.csv").c_str(), O_RDONLY);
	struct stat s; fstat(fd, &s);
	size = s.st_size;
	void* mapped = mmap(0, size, PROT_READ, MAP_FILE|MAP_PRIVATE|MAP_POPULATE, fd, 0);
	madvise(mapped, size, MADV_WILLNEED);
	ptr = (char*)mapped;
	buf_begin = (char *)mapped;
	buf_end = (char*)mapped + size;


	MMAP_READ_TILL_EOL();

	real_size = buf_end - ptr;
	char *p = ptr;
	min_v = get_int(p, '|');

	p = buf_end - 1;
	move_to_line_start(p);
	max_v = get_int(p, '|');
}

void tasty_bread::destroy() {
	munmap(buf_begin, size);
}

tasty_bread::int_t tasty_bread::get_second(int_t first) {
	size_t left = 0, right = real_size - 1;

	int_t left_v = min_v, right_v = max_v;
#ifdef DEBUG_BREAD
	last_nr_iter = 0;
#endif

	while (left < right) {

#ifdef DEBUG_BREAD
		last_nr_iter ++;
#endif
		size_t mid = guess_position(left, right,
				left_v, right_v, first);

		char *p = ptr + mid;
		move_to_line_start(p);

		int_t a = get_int(p, '|');

		if (a == first) {
			p ++;
			int_t b = get_int(p, '\n');
			return b;
		} else {
			if (left_v == right_v)
				return NOT_FOUND;
			if (a < first) {
				left = mid + 1;
				left_v = a;
			} else {  // a > first
				right = mid - 1;
				right_v = a;
			}
		}
	}

	return NOT_FOUND;
}

void tasty_bread::move_to_line_start(char *&p) {
	if (*p == '\n')
		p --;
	while (p >= ptr && *p != '\n')
		p --;
	p ++;
}

tasty_bread::int_t tasty_bread::get_int(char *&p, char delim) {
	int_t ret = 0;
	while (p < buf_end && *p != delim) {
		ret = ret * 10 + *p - '0';
		p ++;
	}
	return ret;
}

size_t tasty_bread::guess_position(
		size_t left, size_t right,
		int_t left_v, int_t right_v, int_t v) {

//    int nr_bits_left_v = nr_bits_10_based(left_v);
//    return (left + right) >> 1;
	size_t pos = left + (double)(v - left_v) / (right_v - left_v) * (right - left);
//    assert(pos >= 0 && pos < size);
//    assert(pos >= left && pos <= right);
	return pos;
}

/**
 * vim: syntax=cpp11 foldmethod=marker
 */
