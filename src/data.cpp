//File: data.cpp
//Date: Mon Mar 24 21:15:13 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "data.h"
#include "lib/debugutils.h"
#include "lib/common.h"
#include "lib/utils.h"
#include <stdlib.h>
#include <algorithm>
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
	int tagid = Data::tagid[s];
	auto& forums = Data::tag_forums[tagid];
	vector<PersonInForum> persons;

	FOR_ITR(itr, forums) {
		set<PersonInForum>& persons_in_forum = (*itr)->persons;
		vector<PersonInForum> tmp; tmp.swap(persons);
		persons.resize(tmp.size() + persons_in_forum.size());
		vector<PersonInForum>::iterator ret_end = set_union(
				persons_in_forum.begin(), persons_in_forum.end(),
				tmp.begin(), tmp.end(), persons.begin());
		persons.resize(std::distance(persons.begin(), ret_end));
	}
	return persons;
}
