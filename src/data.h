//File: data.h
//Date: Fri Feb 28 10:57:02 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include "lib/debugutils.h"
#include <vector>
#include <iostream>
#include <string>
#include <map>
#include <set>

struct ConnectedPerson {
	// Data structure to store a connected person for a specific person
	// store person_id and number of reciprocal comments(for query1)

	int pid;
	int ncmts;		// number of comments
	ConnectedPerson(int _pid, int _ncmts): pid(_pid), ncmts(_ncmts){}

	bool operator < (const ConnectedPerson& r) const
	{ return ncmts < r.ncmts; }

	friend std::ostream& operator << (std::ostream& os, const ConnectedPerson& cp)
	{ os << cp.pid << " " << cp.ncmts; return os; }
};

typedef std::set<int> TagSet;
// Data structure to store all the interest tag of a person
// could be implemented as bitset later

struct PersonInPlace {
// Data structure to store a single person in a place
// with additional information for query3
	int pid;
	int ntags;		// number of tags of this person
	PersonInPlace(){}
	PersonInPlace(int _pid);

	bool operator < (const PersonInPlace& r) const
	{ return ntags > r.ntags or (ntags == r.ntags and pid < r.pid); }

	bool operator == (const PersonInPlace& r) const
	{ return pid == r.pid; }

	friend std::ostream& operator << (std::ostream& os, const PersonInPlace& pp)
	{ os << pp.pid << " " << pp.ntags; return os; }
};

typedef std::vector<PersonInPlace> PersonSet;	// must be sorted

class PlaceNode {
public:
	std::vector<PlaceNode*> sub_places;
	PersonSet persons;		// sorted
	PersonSet get_all_persons();		// sorted
};
// TODO may need to cache place->set of person

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

	static std::map<std::string, int> placeid;		// id of each place
	static std::vector<PlaceNode> places;			// each place indexed by id


	static void allocate(int max_person_id);
	static void free();
private:
	Data(){};

	Data(Data const &);
	void operator=(Data const &);
};

