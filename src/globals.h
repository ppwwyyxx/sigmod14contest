//File: globals.h
//Date: Mon Mar 17 23:55:14 2014 +0800
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

extern int friends_data_reader;
extern std::mutex mt_friends_data_changing;
extern std::condition_variable cv_friends_data_changing;

extern ThreadPool* threadpool;

extern std::vector<double> tot_time;

extern unordered_set<std::string, StringHashFunc> q4_tag_set;
// end of global variables!!
