//File: globals.h
//Date: Wed Mar 26 12:25:35 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <condition_variable>
#include <mutex>
#include "lib/hash_lib.h"
#include "lib/ThreadPool.hh"
// global variables!!
/*
 *extern std::mutex comment_read_mt;
 *extern std::condition_variable comment_read_cv;
 *extern bool comment_read;
 */

extern std::condition_variable tag_read_cv;
extern std::mutex tag_read_mt;
extern bool tag_read;

extern ThreadPool* threadpool;

extern std::vector<double> tot_time;

extern unordered_set<std::string, StringHashFunc> q4_tag_set;
// end of global variables!!
