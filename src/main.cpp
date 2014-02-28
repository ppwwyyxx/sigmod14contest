//File: main.cpp
//Date: Fri Feb 28 16:10:05 2014 +0800
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


	/*
	 *int id = Data::placeid["South_America"];
	 *PlaceNode& p = Data::places[id];
	 *PP(p.sub_places.size());
	 *PA(p.persons);
	 *PA(p.get_all_persons());
	 */
	/*
	 *int tagid = Data::tagid["Jet_Li"];
	 *PP(tagid);
	 *for (auto &k : Data::tag_forums[tagid]) {
	 *    P(k->id);
	 *    PA(k->persons);
	 *    P("\n");
	 *}
	 */
	PP(timer.get_time());
	Data::free();
}
