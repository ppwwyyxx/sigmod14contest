// File: Timer.cpp
// Author: Yuxin Wu <ppwwyyxxc@gmail.com>


#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <mutex>
#include <string>

#include "Timer.h"
#include "common.h"
using namespace std;

Timer::Timer() {
#ifdef WIN32
    QueryPerformanceFrequency(&freq);
    startCount.QuadPart = 0;
    endCount.QuadPart = 0;
#else
    startCount.tv_sec = startCount.tv_usec = 0;
    endCount.tv_sec = endCount.tv_usec = 0;
#endif
	reset();
}

void Timer::reset() {
    stopped = 0;
#ifdef WIN32
    QueryPerformanceCounter(&startCount);
#else
    gettimeofday(&startCount, NULL);
#endif
}

void Timer::stop() {
    stopped = 1;

#ifdef WIN32
    QueryPerformanceCounter(&endCount);
#else
    gettimeofday(&endCount, NULL);
#endif
}

double Timer::get_time_microsec() {
#ifdef WIN32
    if(!stopped) QueryPerformanceCounter(&endCount);
    double start = startCount.QuadPart * ((double)1e6 / freq.QuadPart);
    double end = endCount.QuadPart * ((double)1e6 / freq.QuadPart);
#else
    if(!stopped) gettimeofday(&endCount, NULL);
    double start = ((int)startCount.tv_sec * 1e6) + (int)startCount.tv_usec;
    double end = ((int)endCount.tv_sec * 1e6) + (int)endCount.tv_usec;
#endif

    return end - start;
}

GuardedTimer::GuardedTimer(const char *fmt, ...) {
	const int BUF_SIZE = 256;
	const int MAX_SIZE = 1 << 20;
	char buf[BUF_SIZE];

	va_list arg;
	int byte_to_print;

	// first give @buf a try
	va_start(arg, fmt);
	byte_to_print = vsnprintf(buf, BUF_SIZE, fmt, arg);
	va_end(arg);

	if (byte_to_print < BUF_SIZE) {
		msg = buf;
		return ;
	}

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
			msg = buf;
			return;
		}

		if (size * 2 >= MAX_SIZE)
			ret = ptr;

		size *= 2;
		delete [] ptr;
	}
}

GuardedTimer::~GuardedTimer() {
	print_debug("%s %f secs\n", msg.c_str(), timer.get_time_sec());
}

TotalTimer::TotalTimer(const string & _msg):msg(_msg) {
	//if (rst.find(msg) == rst.end()) rst[msg] = 0;
	timer.reset();
}

TotalTimer::~TotalTimer() {
	static mutex mt;
	lock_guard<mutex> lg(mt);
	rst[msg] += timer.get_time();
}

void TotalTimer::print() {
	FOR_ITR(itr, rst) {
		print_debug("%s spent %lf secs in all\n", itr->first.c_str(), itr->second);
	}
}

ManualTotalTimer::ManualTotalTimer(const std::string &msg) : msg(msg) {
	reset();
}

ManualTotalTimer::~ManualTotalTimer() {
}

void ManualTotalTimer::reset() {
	timer.reset();
}

void ManualTotalTimer::record() {
	{
		static mutex mt;
		lock_guard<mutex> lg(mt);
		rst[msg] += timer.get_time();
	}
	reset();
}

void ManualTotalTimer::print() {
	FOR_ITR(itr, rst) {
		print_debug("%s spent %lf secs in all\n", itr->first.c_str(), itr->second);
	}
}


std::map<std::string, double> TotalTimer::rst;

std::map<std::string, double> ManualTotalTimer::rst;
