/*
 * $File: finish_time_continuation.h
 * $Date: Mon Mar 31 14:51:01 2014 +0000
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

#pragma once

#include <mutex>
#include <string>
#include "../globals.h"

class FinishTimeContinuation {
	public:
		FinishTimeContinuation(int count, std::string prompt) :
			count(count), prompt(prompt) {
		};

		void cont() {
			{
				std::lock_guard<std::mutex> lock(count_mutex);
				count --;
			}
			if (count == 0) {
				fprintf(stderr, "%s: %f secs\n", prompt.c_str(), globaltimer.get_time());
			}
		}

	private:
		int count;
		std::mutex count_mutex;
		std::string prompt;
};

/**
 * vim: syntax=cpp11 foldmethod=marker
 */
