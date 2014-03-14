// File: debugutils.cpp

// Author: Yuxin Wu <ppwwyyxxc@gmail.com>


#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <cstdarg>
#include <map>
using namespace std;

#include "utils.h"
#include "debugutils.h"

void __m_assert_check__(bool val, const char *expr, const char *file, const char *func, int line) {
	if (val)
		return;
	c_fprintf(COLOR_RED, stderr, "assertion \"%s\" failed, in %s, (%s:%d)\n",
			expr, func, file, line);
	abort();
}


void __print_debug__(const char *file, const char *func, int line, const char *fmt, ...) {
	static map<int, string> colormap;
	static int color = 0;
	if (! colormap[line].length()) {
		colormap[line] = TERM_COLOR(color);
		color = (color + 1) % 5;
	}

	char *fbase = basename(strdupa(file));
	c_fprintf(colormap[line].c_str(), stderr, "[%s@%s:%d] ", func, fbase, line);

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}


void error_exit(const char *msg) {
	c_fprintf(COLOR_RED, stderr, "error: %s\n", msg);
	abort();
}

std::string ssprintf(const char *fmt, ...) {
	const int BUF_SIZE = 256;
	const int MAX_SIZE = 1 << 20;
	thread_local char buf[BUF_SIZE];

	va_list arg;
	int byte_to_print;

	// first give @buf a try
	va_start(arg, fmt);
	byte_to_print = vsnprintf(buf, BUF_SIZE, fmt, arg);
	va_end(arg);

	if (byte_to_print < BUF_SIZE)
		return buf;

	std::string ret;

	// buf is not enough, iteratively increasing size
	int size = byte_to_print + 1;
	char *ptr;
	for (; size < MAX_SIZE; size *= 2) {
		ptr = new char[size];
		va_start(arg, fmt);
		byte_to_print = vsnprintf(buf, size, fmt, arg);
		va_end(arg);

		if (byte_to_print < size) {
			ret = ptr;
			delete [] ptr;
			return ret;
		}

		if (size * 2 >= MAX_SIZE)
			ret = ptr;

		size *= 2;
		delete [] ptr;
	}
	return ret;
}
