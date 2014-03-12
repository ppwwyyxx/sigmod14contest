//File: read.cpp
//Date: Wed Mar 12 20:02:32 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <stdlib.h>
#include <algorithm>
#include <mutex>
#include <thread>
#include <stdio.h>

#include "lib/debugutils.h"
#include "lib/utils.h"
#include "lib/Timer.h"
#include "lib/common.h"
#include "read.h"
#include "data.h"
using namespace std;

namespace {
	const int BUFFER_LEN = 1024 * 1024 * 5;
}

#define safe_open(fname) \
	FILE* fin = fopen((fname).c_str(), "r"); \
	m_assert(fin != NULL);

#define PTR_NEXT() \
{ \
	ptr ++; \
	if (ptr == buf_end) \
	{ \
		ptr = buffer; \
		buf_end = buffer + fread(buffer, 1, BUFFER_LEN, fin); \
	} \
}

// only read non-negative int
#define READ_INT(_x_) \
{ \
	while (*ptr < '0' || *ptr > '9') \
		PTR_NEXT(); \
	int _n_ = 0; \
	while (*ptr >= '0' && *ptr <= '9') \
	{ \
		_n_ = _n_ * 10 + *ptr - '0'; \
		PTR_NEXT(); \
	} \
	(_x_) = (_n_); \
}

#define READ_STR(_s_) \
{ \
	char *_p_ = (_s_); \
	while (*ptr != '|') \
	{ \
		*(_p_ ++) = *ptr; \
		PTR_NEXT(); \
	} \
	*_p_ = 0; \
}

#define READ_TILL_EOL() \
	while (*ptr != '\n') PTR_NEXT();

void read_person_file(const string& dir) {
	char buffer[BUFFER_LEN];
	char *ptr, *buf_end;
	char tmpBuf[1024];

	safe_open(dir + "/person.csv");
	ptr = buffer, buf_end = ptr + 1;

	READ_TILL_EOL();
	int pid, maxid = 0;
	while (true) {
		READ_INT(pid);
		if (buf_end == buffer) break;
		update_max(maxid, pid);
		READ_TILL_EOL();
	}
	Data::allocate(maxid);

	// read birthday
	rewind(fin);
	ptr = buffer, buf_end = ptr + 1;

	READ_TILL_EOL();
	int year, month, day;
	while (true) {
		READ_INT(pid);
		if (buffer == buf_end) break;
		PTR_NEXT();
		READ_STR(tmpBuf); PTR_NEXT();
		READ_STR(tmpBuf); PTR_NEXT();
		READ_STR(tmpBuf);
		READ_INT(year);PTR_NEXT();
		READ_INT(month);PTR_NEXT();
		READ_INT(day);
		READ_TILL_EOL();
		Data::birthday[pid] = year * 10000 + month * 100 + day;
	}
	fclose(fin);
}

void read_person_knows_person(const string& dir) {
	char buffer[BUFFER_LEN];
	char *ptr, *buf_end;

	safe_open(dir + "/person_knows_person.csv");
	ptr = buffer, buf_end = ptr + 1;

	READ_TILL_EOL();
	int p1, p2;
	while (true) {
		READ_INT(p1);
		if (buffer == buf_end) break;
		READ_INT(p2);
		PTR_NEXT();
		//Data::pp_map[p1][p2] = Data::pp_map[p2][p1] = true;

		Data::friends[p1].push_back(ConnectedPerson(p2, 0));
		Data::friends[p2].push_back(ConnectedPerson(p1, 0));
	}
	fclose(fin);
}

namespace { char buffer[BUFFER_LEN];	}	// WARNING: GLOBAL VARIABLE
void read_comments(string dir) {
	char *ptr, *buf_end;

	vector<int> owner;
	Timer timer;
	{
		safe_open(dir + "/comment_hasCreator_person.csv");
		ptr = buffer, buf_end = ptr + 1;

		READ_TILL_EOL();
		unsigned cid, pid;
		owner.reserve(4000000);
		while (true) {
			READ_INT(cid);
			if (buffer == buf_end) break;
			READ_INT(pid);
			m_assert(cid % 10 == 0);
			m_assert(cid / 10 == owner.size());
			owner.push_back(pid);
		}
		fclose(fin);
	}

	// read comment->comment
	int ** comment_map = new int*[Data::nperson];
	for (int i = 0; i < Data::nperson; i ++)
		comment_map[i] = new int[Data::nperson]();
	{
		safe_open(dir + "/comment_replyOf_comment.csv");
		ptr = buffer, buf_end = ptr + 1;

		READ_TILL_EOL();
		int cid1, cid2;
		while (true) {
			READ_INT(cid1);
			if (buffer == buf_end) break;
			READ_INT(cid2);

			int p1 = owner[cid1 / 10], p2 = owner[cid2 / 10];
			//if (not Data::pp_map[p1][p2]) continue;
			comment_map[p1][p2] += 1;		// p1 reply to p2
		}
		fclose(fin);
	}

	for (int i = 0; i < Data::nperson; i ++) {
		auto& fs = Data::friends[i];
		FOR_ITR(itr, fs) {
			itr->ncmts = min(comment_map[i][itr->pid], comment_map[itr->pid][i]);
		}
		/*
		 *for (int j = 0; j < Data::nperson; j ++) {
		 *    if (not Data::pp_map[i][j]) continue;
		 *    int ncmt = std::min(comment_map[i][j], comment_map[j][i]);
		 *    Data::friends[i].push_back(ConnectedPerson(j, ncmt));		// i reply to j
		 *    Data::friends[j].push_back(ConnectedPerson(i, ncmt));		// j reply to i
		 *}
		 *sort(Data::friends[i].begin(), Data::friends[i].end());
		 */
	}
	free_2d<int>(comment_map, Data::nperson);
	print_debug("Read comment spent %lf secs\n", timer.get_time());

	Data::comment_read = true;
	Data::comment_read_cv.notify_all();
}

namespace { char buffer2[BUFFER_LEN]; }
void read_tags_forums_places(string dir) {
	char* buffer = buffer2;
	char *ptr, *buf_end;

	Timer timer;

	unordered_map<int, int> id_map; // map from real id to continuous id
#ifdef GOOGLE_HASH
	id_map.set_empty_key(-1);
#endif
	int tid, pid, fid;
	{		// read tag and tag names
		safe_open(dir + "/tag.csv");
		fgets(buffer, BUFFER_LEN, fin);
		while (fscanf(fin, "%d|", &tid) == 1) {
			int k = 0; char c;
			while ((c = (char)fgetc(fin)) != '|')
				buffer[k++] = c;
			string tag_name(buffer, k);
			id_map[tid] = (int)Data::tag_name.size();
#ifdef DEBUG
			Data::real_tag_id.push_back(tid);
#endif
			Data::tag_name.push_back(tag_name);
			m_assert(Data::tagid.count(tag_name) == 0);		//  no repeated tag name
			Data::tagid[tag_name] = (int)Data::tag_name.size() - 1;
			fgets(buffer, BUFFER_LEN, fin);
		}
		Data::ntag = (int)Data::tag_name.size();
		fclose(fin);
	}
	Data::person_in_tags.resize(Data::ntag);

	{		// read person->tags
		safe_open(dir + "/person_hasInterest_tag.csv");
		fgets(buffer, BUFFER_LEN, fin);
		while (fscanf(fin, "%d|%d", &pid, &tid) == 2) {
			int c_id = id_map[tid];
			Data::tags[pid].insert(c_id);
			Data::person_in_tags[c_id].push_back(pid);
		}
		fclose(fin);
	}

	//read places, need tag data to sort
	read_places(dir);

	Data::tag_read = true;
	Data::tag_read_cv.notify_all();

	print_debug("Read tag and places spent %lf secs\n", timer.get_time());
	timer.reset();

	unordered_map<int, Forum*> forum_idmap;
#ifdef GOOGLE_HASH
	forum_idmap.set_empty_key(-1);
#endif
	Data::tag_forums.resize(Data::ntag);
	{
		safe_open(dir + "/forum_hasMember_person.csv");
		ptr = buffer, buf_end = ptr + 1;
		READ_TILL_EOL();
		while (true) {
			READ_INT(fid);
			if (buffer == buf_end) break;
			READ_INT(pid);
			READ_TILL_EOL();

			unordered_map<int, Forum*>::const_iterator itr = forum_idmap.find(fid);
			if (itr != forum_idmap.end()) {
				itr->second->persons.insert(PersonInForum(pid));
			} else {
				Forum* forum = new Forum();		// XXX these memory will never be freed until program exited.
#ifdef DEBUG
				forum->id = fid;
#endif
				forum->persons.insert(PersonInForum(pid));
				forum_idmap[fid] = forum;
			}
		}
		fclose(fin);
	}
	{
		safe_open(dir + "/forum_hasTag_tag.csv");
		ptr = buffer, buf_end = ptr + 1;
		READ_TILL_EOL();
		while (true) {
			READ_INT(fid);
			if (buffer == buf_end) break;
			READ_INT(tid);
			m_assert(id_map.find(tid) != id_map.end());
			if (forum_idmap.find(fid) == forum_idmap.end()) continue;		// forums with no person in
			int c_tid = id_map[tid];
			Data::tag_forums[c_tid].push_back(forum_idmap[fid]);
		}
		fclose(fin);
	}
	print_debug("Read forum spent %lf secs\n", timer.get_time());

	Data::forum_read = true;
	Data::forum_read_cv.notify_all();
}

void read_org_places(const string& fname, const vector<int>& org_places) {
	char buffer[1024];
	safe_open(fname);
	fgets(buffer, BUFFER_LEN, fin);
	int oid, pid;
	while (fscanf(fin, "%d|%d", &pid, &oid) == 2) {
		fgets(buffer, BUFFER_LEN, fin);
		m_assert(oid % 10 == 0);

		Data::places[org_places[oid / 10]].persons.push_back(
				PersonInPlace(pid));
	}
	fclose(fin);
}

void build_places_tree(const string& dir) {
	char buffer[1024];
	int pid, max_pid = 0;
	{
		safe_open(dir + "/place.csv");
		fgets(buffer, BUFFER_LEN, fin);
		while (fscanf(fin, "%d|", &pid) == 1) {
			int k = 0; char c;
			while ((c = (char)fgetc(fin)) != '|')
				buffer[k++] = c;
			string place_name(buffer, k);
			Data::placeid[place_name].push_back(pid);
			update_max(max_pid, pid);
			fgets(buffer, BUFFER_LEN, fin);
		}
		Data::places.resize(max_pid + 1);
		fclose(fin);
	}

	{
		safe_open(dir + "/place_isPartOf_place.csv");
		fgets(buffer, BUFFER_LEN, fin);
		int p1, p2;
		while (fscanf(fin, "%d|%d", &p1, &p2) == 2) {
			Data::places[p2].sub_places.push_back(&Data::places[p1]);
			Data::places[p1].parent = &Data::places[p2];
		}
		fclose(fin);
	}
}

void read_places(const string& dir) {
	char buffer[1024];
	build_places_tree(dir);

	{
		safe_open(dir + "/person_isLocatedIn_place.csv");
		fgets(buffer, BUFFER_LEN, fin);
		int person, place;
		while (fscanf(fin, "%d|%d", &person, &place) == 2) {
			Data::places[place].persons.push_back(
					PersonInPlace(person));
		}
		fclose(fin);
	}

	vector<int> org_places;
	{
		safe_open(dir + "/organisation_isLocatedIn_place.csv");
		fgets(buffer, BUFFER_LEN, fin);
		int pid, oid;
		while (fscanf(fin, "%d|%d", &oid, &pid) == 2) {
			m_assert(oid % 10 == 0);
			m_assert(oid / 10 == (int)org_places.size());
			org_places.push_back(pid);
		}
		fclose(fin);
	}

	read_org_places(dir + "/person_studyAt_organisation.csv", org_places);
	read_org_places(dir + "/person_workAt_organisation.csv", org_places);

	// sort and unique
	for (vector<PlaceNode>::iterator it = Data::places.begin();
			it != Data::places.end(); it ++) {
		sort(it->persons.begin(), it->persons.end());
		vector<PersonInPlace>::iterator last = unique(it->persons.begin(), it->persons.end());
		it->persons.resize(distance(it->persons.begin(), last));
	}
}

void read_data(const string& dir) {		// may need to be implemented synchronously
	Timer timer;
	read_person_file(dir);
	read_person_knows_person(dir);
	print_debug("Read person spent %lf secs\n", timer.get_time());
	timer.reset();
#ifdef USE_THREAD
	thread(read_comments, dir).detach();		// peak memory will be large
	thread(read_tags_forums_places, dir).detach();
#else
	read_comments(dir);
	read_tags_forums_places(dir);
#endif
}
