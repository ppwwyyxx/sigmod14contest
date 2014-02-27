//File: data.h
//Date: Thu Feb 27 23:00:56 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <set>

struct ConnectedPerson {
	// Data structure to store a connected person for a specific person
	// store person_id and number of reciprocal comments(for query1)

	int pid;
	int ncomment;
	ConnectedPerson(int _pid, int _ncomment): pid(_pid), ncomment(_ncomment){}

	bool operator < (const ConnectedPerson& r) const
	{ return ncomment < r.ncomment; }

	friend std::ostream& operator << (std::ostream& os, const ConnectedPerson& cp)
	{ os << cp.pid << " " << cp.ncomment; return os; }
};

typedef std::set<int> TagSet;
// Data structure to store all the interest tag of a person
// could be implemented as bitset later

class Data {
public:
	static int nperson, ntag;

	static bool ** pp_map;		// person_knows_person
	static std::vector<std::vector<ConnectedPerson> > friends;
	// friends[i] is a vector(sorted by 'ncomment') of friends of the person with id=i

	static int *birthday;	// birthday[i] for the person with id=i

	static std::vector<TagSet> tags;
	// tags[i] is a TagSet containing interest tags(continuous id) for the person with id=i

	static std::vector<std::string> tagname;		// name of each tags, indexed by continuous id


	static void allocate(int max_person_id);
	static void free();
private:
	Data(){};

	Data(Data const &);
	void operator=(Data const &);
};

