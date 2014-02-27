//File: data.h
//Date: Thu Feb 27 15:39:58 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <vector>

class Data {
public:
	static int nperson;

	static bool ** pp_map;
	static std::vector<std::vector<int> > friends;

	static void allocate(int max_person_id);

	static void free();

private:
	Data(){};

	Data(Data const &);
	void operator=(Data const &);
};

