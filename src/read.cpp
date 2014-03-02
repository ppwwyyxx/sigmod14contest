//File: read.cpp
//Date: Sun Mar 02 13:15:16 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "lib/debugutils.h"
#include "lib/utils.h"
#include "lib/Timer.h"
#include "data.h"
#include <stdlib.h>
#include <algorithm>
#include <stdio.h>
using namespace std;
using namespace std::tr1;

namespace {
	const int BUFFER_LEN = 1024 * 1024 * 5;
	char buffer[BUFFER_LEN];
	char tmpBuf[1024];
	char *ptr, *buf_end;
}

inline FILE* safe_open(const string& fname) {
	FILE* fin = fopen(fname.c_str(), "r");
	m_assert(fin != NULL);
	ptr = buffer, buf_end = ptr + 1;
	return fin;
}

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
	int register _n_ = 0; \
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
	FILE* fin = safe_open(dir + "/person.csv");
	READ_TILL_EOL();
	int pid, maxid = 0;
	while (true) {
		READ_INT(pid);
		if (buf_end == buffer) break;
		update_max(maxid, pid);
		READ_TILL_EOL();
	}
	print_debug("Number of Person: %d\n", maxid);
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
		if (pid == 996) {
			PP(tmpBuf);
			PP(year);
			PP(month);
			PP(day);
		}
		Data::birthday[pid] = year * 10000 + month * 100 + day;
	}
	fclose(fin);
}

void read_person_knows_person(const string& dir) {
	FILE* fin = safe_open(dir + "/person_knows_person.csv");
	READ_TILL_EOL();
	int p1, p2;
	while (true) {
		READ_INT(p1);
		if (buffer == buf_end) break;
		READ_INT(p2);
		PTR_NEXT();
		Data::pp_map[p1][p2] = Data::pp_map[p2][p1] = true;
	}
	fclose(fin);
}

void read_comments(const string& dir) {
	Timer timer;
	FILE* fin = safe_open(dir + "/comment_hasCreator_person.csv");
	READ_TILL_EOL();
	unsigned cid, pid;
	vector<int> owner;
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

	// read comment->comment
	fin = safe_open(dir + "/comment_replyOf_comment.csv");
	READ_TILL_EOL();

	int ** comment_map = new int*[Data::nperson];
	for (int i = 0; i < Data::nperson; i ++)
		comment_map[i] = new int[Data::nperson]();

	int cid1, cid2;
	while (true) {
		READ_INT(cid1);
		if (buffer == buf_end) break;
		READ_INT(cid2);

		int p1 = owner[cid1 / 10], p2 = owner[cid2 / 10];
		if (not Data::pp_map[p1][p2]) continue;
		comment_map[p1][p2] += 1;		// p1 reply to p2
	}
	fclose(fin);

	for (int i = 0; i < Data::nperson; i ++) {
		for (int j = 0; j < Data::nperson; j ++) {
			if (not Data::pp_map[i][j]) continue;
			Data::friends[i].push_back(ConnectedPerson(j, comment_map[i][j]));		// i reply to j
			Data::friends[j].push_back(ConnectedPerson(i, comment_map[j][i]));		// j reply to i
		}
		sort(Data::friends[i].begin(), Data::friends[i].end());
	}
	free_2d<int>(comment_map, Data::nperson);
}

void read_tags_forums(const string & dir) {
	unordered_map<int, int> id_map; // map from real id to continuous id
	int tid, pid, fid;
	{		// read tag and tag names
		FILE* fin = safe_open(dir + "/tag.csv");
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
			Data::tagid[tag_name] = (int)Data::tag_name.size() - 1;
			fgets(buffer, BUFFER_LEN, fin);
		}
		Data::ntag = (int)Data::tag_name.size();
		fclose(fin);
	}
	Data::person_in_tags.resize(Data::ntag);
	print_debug("Number of tags: %d\n", Data::ntag);

	{		// read person->tags
		FILE* fin = safe_open(dir + "/person_hasInterest_tag.csv");
		fgets(buffer, BUFFER_LEN, fin);
		while (fscanf(fin, "%d|%d", &pid, &tid) == 2) {
			int c_id = id_map[tid];
			Data::tags[pid].insert(c_id);
			Data::person_in_tags[c_id].push_back(pid);
		}
		fclose(fin);
	}

	unordered_map<int, Forum*> forum_idmap;
	Data::tag_forums.resize(Data::ntag);
	{
		FILE* fin = safe_open(dir + "/forum_hasMember_person.csv");
		READ_TILL_EOL();
		while (true) {
			READ_INT(fid);
			if (buffer == buf_end) break;
			READ_INT(pid);
			READ_TILL_EOL();

			unordered_map<int, Forum*>::const_iterator itr = forum_idmap.find(fid);
			if (itr != forum_idmap.end()) {
				itr->second->persons.push_back(PersonInForum(pid));
			} else {
				Forum* forum = new Forum();		// XXX these memory will never be free until program exited.
#ifdef DEBUG
				forum->id = fid;
#endif
				forum->persons.push_back(PersonInForum(pid));
				forum_idmap[fid] = forum;
			}
		}
		fclose(fin);
	}
	{
		FILE* fin = safe_open(dir + "/forum_hasTag_tag.csv");
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
}

void read_org_places(const string& fname, const vector<int>& org_places) {
	FILE* fin = safe_open(fname);
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
	int pid, max_pid = 0;
	{
		FILE* fin = safe_open(dir + "/place.csv");
		fgets(buffer, BUFFER_LEN, fin);
		while (fscanf(fin, "%d|", &pid) == 1) {
			int k = 0; char c;
			while ((c = (char)fgetc(fin)) != '|')
				buffer[k++] = c;
			string place_name(buffer, k);
			Data::placeid[place_name] = pid;
			update_max(max_pid, pid);
			fgets(buffer, BUFFER_LEN, fin);
		}
		Data::places.resize(max_pid + 1);
		fclose(fin);
	}

	{
		FILE* fin = safe_open(dir + "/place_isPartOf_place.csv");
		fgets(buffer, BUFFER_LEN, fin);
		int p1, p2;
		while (fscanf(fin, "%d|%d", &p1, &p2) == 2) {
			Data::places[p2].sub_places.push_back(&Data::places[p1]);
		}
		fclose(fin);
	}
}

void read_places(const string& dir) {
	build_places_tree(dir);

	FILE* fin = safe_open(dir + "/person_isLocatedIn_place.csv");
	fgets(buffer, BUFFER_LEN, fin);
	int person, place;
	while (fscanf(fin, "%d|%d", &person, &place) == 2) {
		Data::places[place].persons.push_back(
				PersonInPlace(person));
	}
	fclose(fin);

	vector<int> org_places;
	fin = safe_open(dir + "/organisation_isLocatedIn_place.csv");
	fgets(buffer, BUFFER_LEN, fin);
	int pid, oid;
	while (fscanf(fin, "%d|%d", &oid, &pid) == 2) {
		m_assert(oid % 10 == 0);
		m_assert(oid / 10 == (int)org_places.size());
		org_places.push_back(pid);
	}
	fclose(fin);

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
	read_comments(dir);
	read_tags_forums(dir);
	read_places(dir);
	print_debug("Read spent %lf secs\n", timer.get_time());
}
