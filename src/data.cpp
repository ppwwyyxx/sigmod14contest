//File: data.cpp
//Date: Tue Apr 15 14:31:12 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "data.h"
#include "lib/debugutils.h"
#include "lib/common.h"
#include "lib/utils.h"
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <iterator>
using namespace std;

int Data::nperson= 0;
int Data::ntag = 0;
int * Data::birthday = NULL;
vector<vector<ConnectedPerson> > Data::friends;
vector<TagSet> Data::tags;
vector<vector<int> > Data::person_in_tags;
vector<string> Data::tag_name;
unordered_map<string, vector<int>, StringHashFunc> Data::placeid;
vector<PlaceNode> Data::places;
vector<unordered_set<int>> Data::friends_hash;

#ifdef DEBUG
vector<int> Data::real_tag_id;
#endif

void Data::allocate() {
	m_assert(nperson != 0);
	friends_hash.resize(nperson);
#ifdef GOOGLE_HASH
	FOR_ITR(itr, friends_hash) itr->set_empty_key(-1);
#endif

	birthday = new int[nperson];
	friends.resize(nperson);
	tags.resize(nperson);
}

void Data::free() {
}

PersonSet PlaceNode::get_all_persons() {
	// TODO faster?
	PersonSet ret = persons;
	for (vector<PlaceNode*>::iterator it = sub_places.begin();
			it != sub_places.end(); it++) {
		PersonSet sub = (*it)->get_all_persons();
		PersonSet tmp; tmp.swap(ret);
		ret.resize(tmp.size() + sub.size());
		PersonSet::iterator ret_end = set_union(
				sub.begin(), sub.end(),
				tmp.begin(), tmp.end(), ret.begin());
		ret.resize(std::distance(ret.begin(), ret_end));
	}
	return ret;
}

PersonInPlace::PersonInPlace(int _pid):
	pid(_pid), ntags(int(Data::tags[_pid].size())) {}

vector<bool> get_tag_persons_hash(const string& s) {
	DEBUG_DECL(TotalTimer, tt("get_tag_persons_hash"));
	return q4_persons[s];
}

int cnt_tag_persons_hash(const string& s) {
	int np = 0;
	auto& hash = q4_persons[s];
	REP(j, Data::nperson)
		if (hash[j])
			np ++;
	return np;
}

vector<int> get_tag_persons(const string& s) {
	TotalTimer tt("get_tag_persons");
	vector<int> ret;
	auto hash = get_tag_persons_hash(s);
	REP(i, Data::nperson)
		if (hash[i])
			ret.emplace_back(i);
	return ret;
}

void output_tgf_graph(string fname, const vector<vector<int>> &friends) {
	ofstream fout(fname);
	for (size_t i = 0; i < friends.size(); i ++)
		fout << i + 1<< ' ' << i + 1 << endl;
	fout << "#" << endl;
	for (size_t i = 0; i < friends.size(); i ++) {
		FOR_ITR(j, friends[i])
			fout << i + 1 << ' ' << *j + 1 << endl;
	}
}

void output_dot_graph(string fname, const vector<vector<int>> &friends) {
	ofstream fout(fname);
	fout << "graph {\n";
	for (size_t i = 0; i < friends.size(); i ++) {
		FOR_ITR(j, friends[i])
			fout << "    " << i << " -- " << *j << ";\n";
	}
	fout << "}\n";
}

