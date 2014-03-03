//File: data.h
//Date: Mon Mar 03 18:09:53 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include "lib/debugutils.h"
#include <vector>
#include <iostream>
#include <string>
#ifdef __linux__
#include <tr1/unordered_map>
using std::tr1::unordered_map;
#else
#include <unordered_map>
using std::unordered_map;
#endif

#include <set>
#include <bitset>

struct ConnectedPerson {
// Data structure to store a connected person for a specific person
// store person_id, and number of reciprocal comments(for query1)

	int pid;
	int ncmts;		// number of comments
	ConnectedPerson(int _pid, int _ncmts):
		pid(_pid), ncmts(_ncmts){}

	bool operator < (const ConnectedPerson& r) const
	{ return ncmts > r.ncmts; }

	friend std::ostream& operator << (std::ostream& os, const ConnectedPerson& cp)
	{ os << cp.pid << " " << cp.ncmts; return os; }
};

typedef std::set<int> TagSet;
// Data structure to store all the interest tag of a person
// could be implemented as vector<bool> later

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
	PlaceNode* parent;
	std::vector<PlaceNode*> sub_places;
	PersonSet persons;		// sorted

	PersonSet get_all_persons();		// sorted
	// TODO the result of this func may need to be cached

	PlaceNode(): parent(NULL){}
};

typedef int PersonInForum;
// Data structure to store a person in a forum
// now it is only implemented as person id

struct Forum {
#ifdef DEBUG
	int id;
#endif
	std::set<PersonInForum> persons;
};

class Data {
public:
	static int nperson, ntag;

	static bool ** pp_map;		// person_knows_person
	static std::vector<std::vector<ConnectedPerson> > friends;
	// friends[i] is a vector(sorted by 'ncomment') of friends of the person with id=i

	static int *birthday;	// birthday[i] for the person with id=i

	static std::vector<TagSet> tags;
	// tags[i] is a TagSet containing interest tags(continuous id) for the person with id=i

	static std::vector<std::vector<int> > person_in_tags;

	static std::vector<std::string> tag_name;		// name of each tags, indexed by continuous id
	static unordered_map<std::string, int> tagid;			// tag name -> tag id
	static std::vector<std::vector<Forum*> > tag_forums;			// related forums for each tag

	static unordered_map<std::string, std::vector<int> > placeid;
	// id of each place. note that for a specific name, there might be several places
	static std::vector<PlaceNode> places;			// each place indexed by id

#ifdef DEBUG
	static std::vector<int> real_tag_id;		// continuous id -> real id
#endif

	static void allocate(int max_person_id);
	static void free();
private:
	Data(){};

	Data(Data const &);
	void operator=(Data const &);
};

