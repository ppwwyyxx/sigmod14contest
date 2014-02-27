//File: data.h
//Date: Thu Feb 27 16:25:20 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <vector>
#include <iostream>

struct ConnectedPerson {
	int pid;
	int ncomment;
	ConnectedPerson(int _pid, int _ncomment):
		pid(_pid), ncomment(_ncomment){}

	bool operator < (const ConnectedPerson& r) const
	{ return ncomment < r.ncomment; }

	friend std::ostream& operator << (std::ostream& os, const ConnectedPerson& cp)
	{
		os << cp.pid << " " << cp.ncomment;
		return os;
	}
};

class Data {
public:
	static int nperson;

	static bool ** pp_map;
	static std::vector<std::vector<ConnectedPerson> > friends;

	static void allocate(int max_person_id);

	static void free();

private:
	Data(){};

	Data(Data const &);
	void operator=(Data const &);
};

