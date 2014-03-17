//File: globals.cpp
//Date: Mon Mar 17 23:55:29 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "globals.h"
#include "lib/common.h"

using namespace std;

// global variables
mutex comment_read_mt;
mutex tag_read_mt;
mutex forum_read_mt;
bool comment_read = false;
bool tag_read = false;
bool forum_read = false;
condition_variable comment_read_cv;
condition_variable tag_read_cv;
condition_variable forum_read_cv;

int friends_data_reader = 0;
std::mutex mt_friends_data_changing;
std::condition_variable cv_friends_data_changing;

ThreadPool* threadpool;

vector<double> tot_time(5, 0);

unordered_set<string, StringHashFunc> q4_tag_set;
// global variables
