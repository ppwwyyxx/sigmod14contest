//File: main.cpp
//Date: Thu Feb 27 15:26:13 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "lib/Timer.h"
#include "lib/debugutils.h"
#include "data.h"
#include "read.h"
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
	Timer timer;
	read_data(string(argv[1]));


	PP(timer.get_time());
	Data::free();
}
