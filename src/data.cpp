//File: data.cpp
//Date: Fri Apr 04 00:23:28 2014 +0000
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
unordered_map<std::string, int, StringHashFunc> Data::tagid;
vector<vector<Forum*> > Data::tag_forums;
unordered_map<string, vector<int>, StringHashFunc> Data::placeid;
vector<PlaceNode> Data::places;

#ifdef DEBUG
vector<int> Data::real_tag_id;
#endif

void Data::allocate() {
#ifdef GOOGLE_HASH
	tagid.set_empty_key("");
	placeid.set_empty_key("");
#endif
	m_assert(nperson != 0);

	birthday = new int[nperson];
	friends.resize(nperson);
	tags.resize(nperson);
}

void Data::free() {
	delete[] birthday;
}

PersonSet PlaceNode::get_all_persons() {
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

vector<PersonInForum> get_tag_persons(const string& s) {
	DEBUG_DECL(TotalTimer, tt("get_tag_persons"));
	int tagid = Data::tagid[s];
	auto& forums = Data::tag_forums[tagid];
	vector<PersonInForum> persons;

	vector<bool> hash(Data::nperson, false);
	FOR_ITR(itr, forums) {
		set<PersonInForum>& persons_in_forum = (*itr)->persons;
		FOR_ITR(p, persons_in_forum)
			hash[*p] = true;
	}
	REP(i, Data::nperson)
		if (hash[i])
			persons.emplace_back(i);
	return persons;
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

