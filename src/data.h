//File: data.h
//Date: Tue Apr 15 14:31:43 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <mutex>
#include <set>

#include "lib/debugutils.h"
#include "globals.h"
#include "lib/hash_lib.h"
#include "lib/common.h"


struct ConnectedPerson {
// Data structure to store a connected person for a specific person
// store person_id, and number of reciprocal comments(for query1)

	int pid;
	int ncmts;		// number of comments
	ConnectedPerson(int _pid, int _ncmts):
		pid(_pid), ncmts(_ncmts){}

	bool operator < (const ConnectedPerson& r) const
	{ return pid < r.pid; }

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
	std::vector<PlaceNode*> sub_places;
	PersonSet persons;		// sorted

	PersonSet get_all_persons();		// sorted
};

typedef int PersonInForum;
// Data structure to store a person in a forum
// now it is only implemented as person id

class Data {
public:
	static int nperson, ntag;

	static std::vector<std::vector<ConnectedPerson> > friends;
	// friends[i] is a vector(sorted by 'id') of friends of the person with id=i
	static std::vector<unordered_set<int>> friends_hash;
	// destroyed after read_comments

	static int *birthday;	// birthday[i] for the person with id=i
	// destroyed after q2 finished

	static std::vector<TagSet> tags;
	// tags[i] is a TagSet containing interest tags(continuous id) for the person with id=i
	// destroyed after q2 and q3 finished

	static std::vector<std::vector<int>> person_in_tags;
	// destroyed after q2 finished

	static std::vector<std::string> tag_name;		// name of each tags, indexed by continuous id
	// destroyed after q2 and read q4 finished!


	// destroyed after q3 finished:
	static unordered_map<std::string, std::vector<int>, StringHashFunc> placeid;
	// id of each place. note that for a specific name, there might be several places
	static std::vector<PlaceNode> places;			// each place indexed by id

#ifdef DEBUG
	static std::vector<int> real_tag_id;		// continuous id -> real id
#endif

	static void allocate();
	static void free();
private:
	Data(){};

	Data(Data const &);
	void operator=(Data const &);
};

std::vector<PersonInForum> get_tag_persons(const std::string& s);
int cnt_tag_persons_hash(const std::string& s);
std::vector<bool> get_tag_persons_hash(const std::string& s);

template <typename T>
inline int edge_count(const std::vector<std::vector<T>>& f) {
	int n_edge = 0;
	FOR_ITR(itr, f) n_edge += (int)itr->size();
	return n_edge;
}
void output_tgf_graph(std::string fname, const std::vector<std::vector<int>> &friends);
void output_dot_graph(std::string fname, const std::vector<std::vector<int>> &friends);
