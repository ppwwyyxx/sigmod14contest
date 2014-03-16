//File: data.cpp
//Date: Sun Mar 16 23:23:49 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "data.h"
#include "lib/debugutils.h"
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

// global variables
mutex comment_read_mt;
mutex tag_read_mt;
mutex forum_read_mt;
bool comment_read = false;
bool tag_read = false;
bool forum_read = false;
condition_variable comment_read_cv;
condition_variable tag_read_cv;
condition_variable forum_read_cv;

mutex friends_change_lock;
unordered_set<string, StringHashFunc> q4_tag_set;
// global variables

#ifdef DEBUG
vector<int> Data::real_tag_id;
#endif

void Data::allocate() {
#ifdef GOOGLE_HASH
	tagid.set_empty_key("");
	placeid.set_empty_key("");
#endif
	m_assert(nperson != 0);

	/*
	 *pp_map = new bool*[nperson];
	 *for (int i = 0; i < nperson; i ++)
	 *    pp_map[i] = new bool[nperson]();
	 */

	birthday = new int[nperson];
	friends.resize(nperson);
	tags.resize(nperson);
}

void Data::free() {
	/*
	 *free_2d<bool>(pp_map, nperson);
	 */
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
