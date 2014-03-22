//File: read.cpp
//Date: Sat Mar 22 12:53:03 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <stdlib.h>
#include <algorithm>
#include <mutex>
#include <cstdio>
#include <fstream>
#include <list>
#include <thread>
#include <stdio.h>

#include "lib/debugutils.h"
#include "lib/utils.h"
#include "lib/Timer.h"
#include "lib/common.h"
#include "read.h"
#include "cache.h"
#include "data.h"
#include "lib/fast_read.h"
using namespace std;

void read_person_file(const string& dir) {
	static char buffer[BUFFER_LEN];
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
	Data::nperson = maxid + 1;
	Data::allocate();
	CachedNeighbor::init(Data::nperson);

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
	static char buffer[BUFFER_LEN];
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

		Data::friends[p1].emplace_back(p2, 0);
		//Data::friends[p2].emplace_back(p1, 0);
	}
	fclose(fin);
}

void read_comments(const string &dir) {
	static char buffer[BUFFER_LEN];
	char *ptr, *buf_end;

	vector<int> owner;
	Timer timer;
	{
		GuardedTimer guarded_timer("read comment_hasCreator_person.csv");
		safe_open(dir + "/comment_hasCreator_person.csv");
		ptr = buffer, buf_end = ptr + 1;

		READ_TILL_EOL();
		unsigned cid, pid;
		if (Data::nperson > 9000) owner.reserve(20100000);
		while (true) {
			READ_INT(cid);
			if (buffer == buf_end) break;
			READ_INT(pid);
			m_assert(cid % 10 == 0);
			m_assert(cid / 10 == owner.size());
			owner.emplace_back(pid);
		}
		fclose(fin);
	}		// 0.6s
	if (Data::nperson > 11000) {
		fprintf(stderr, "ncomments: %lu\n", owner.size());
		fflush(stderr);
	}


	int max_diff = 0;
	// read comment->comment
	vector<unordered_map<int, int>> comment_map(Data::nperson);
	FOR_ITR(itr, comment_map) itr->set_empty_key(-1);
	{
		GuardedTimer guarded_timer("read comment_replyOf_comment.csv");
		safe_open(dir + "/comment_replyOf_comment.csv");
		ptr = buffer, buf_end = ptr + 1;

		READ_TILL_EOL();
		int cid1, cid2;
		while (true) {
			READ_INT(cid1);
			if (buffer == buf_end) break;
			READ_INT(cid2);		// max difference cid1 - cdi2 is 180
			update_max(max_diff, cid1 - cid2);

			int p1 = owner[cid1 / 10], p2 = owner[cid2 / 10];
			comment_map[p1][p2] += 1;		// p1 reply to p2
		}
		fclose(fin);
	}
	PP(max_diff);

	REP(i, Data::nperson) {
		auto& fs = Data::friends[i];
		auto& m = comment_map[i];
		FOR_ITR(itr, fs) {
			itr->ncmts = min(m[itr->pid], comment_map[itr->pid][i]);
		}
	}
	print_debug("Read comment spent %lf secs\n", timer.get_time());
}

void read_comments_2file(const string& dir) {
	// This function assumes that when comment a reply to b, a - b < COMMENT_CACHE_LEN
	const int COMMENT_CACHE_LEN = 512;		// 10k: max(cid1 - cdi2) = 180

	Timer timer;
	static char buffer_p[BUFFER_LEN];
	static char buffer_c[BUFFER_LEN];
	FILE* fp_p = fopen((dir + "/comment_hasCreator_person.csv").c_str(), "r");
	FILE* fp_c = fopen((dir + "/comment_replyOf_comment.csv").c_str(), "r");
	char *ptr_p = buffer_p, *buf_end_p = ptr_p + 1,
		 *ptr_c = buffer_c, *buf_end_c = ptr_c + 1;
	READ_TILL_EOL_s(_p); READ_TILL_EOL_s(_c);

	int comment_owner[COMMENT_CACHE_LEN];
	int cid, pid, cid1, cid2;

	vector<unordered_map<int, int>> comment_map(Data::nperson);
	FOR_ITR(itr, comment_map) itr->set_empty_key(-1);
	//vector<vector<int>> comment_map(Data::nperson, vector<int>(Data::nperson));

	while (true) {
		READ_INT_s(_c, cid1);
		if (buffer_c == buf_end_c) break;
		READ_INT_s(_c, cid2);
		m_assert(cid1 - cid2 < COMMENT_CACHE_LEN);
		while (true) {
			READ_INT_s(_p, cid);
			READ_INT_s(_p, pid);
			comment_owner[cid % COMMENT_CACHE_LEN] = pid;
			if (cid == cid1) {
				int p2 = comment_owner[cid2 % COMMENT_CACHE_LEN];
				comment_map[pid][p2] ++;
				break;
			}
		}
	}
	fclose(fp_p);fclose(fp_c);

	REP(i, Data::nperson) {
		auto& fs = Data::friends[i];
		auto& m = comment_map[i];
		FOR_ITR(itr, fs) {
			itr->ncmts = min(m[itr->pid], comment_map[itr->pid][i]);
		}
	}

	print_debug("Read comment spent %lf secs\n", timer.get_time());
}

void read_forum(const string& dir, unordered_map<int, int>& id_map, const unordered_set<int>& q4_tag_ids) {
	static char buffer[BUFFER_LEN];
	Timer timer;
	char* ptr, *buf_end;
	Data::tag_forums.resize(Data::ntag);
	int fid, tid, pid;
	unordered_map<int, vector<int>> forum_to_tags;		// fid -> continuous tids
#ifdef GOOGLE_HASH
	forum_to_tags.set_empty_key(-1);
#endif
	{
		safe_open(dir + "/forum_hasTag_tag.csv");
		ptr = buffer, buf_end = ptr + 1;
		READ_TILL_EOL();
		while (true) {
			READ_INT(fid);
			if (buffer == buf_end) break;
			READ_INT(tid);
			if (not q4_tag_ids.count(tid))
				continue;
			m_assert(id_map.find(tid) != id_map.end());
			int c_tid = id_map[tid];
			forum_to_tags[fid].emplace_back(c_tid);
		}
		fclose(fin);
	}

	{
		safe_open(dir + "/forum_hasMember_person.csv");
		ptr = buffer, buf_end = ptr + 1;
		READ_TILL_EOL();
		list<int> person_in_now_forum;
		int old_fid = -1;
		while (true) {
			READ_INT(fid);
			if (buffer == buf_end) break;

			if (fid != old_fid && old_fid != -1) {
				auto itr = forum_to_tags.find(old_fid);
				if (itr != forum_to_tags.end()) {
					Forum* forum = new Forum();		// XXX these memory will never be freed until program exited.
					FOR_ITR(p, person_in_now_forum)
						forum->persons.insert(PersonInForum(*p));
					FOR_ITR(titr, itr->second) {
						Data::tag_forums[*titr].emplace_back(forum);
					}
				}
				person_in_now_forum.clear();
			}
			old_fid = fid;
			READ_INT(pid);
			READ_TILL_EOL();
			person_in_now_forum.emplace_back(pid);
		}
		fclose(fin);
	}

	print_debug("Read forum spent %lf secs\n", timer.get_time());
}


void read_tags_forums_places(const string& dir) {
	char buffer[1024];
	Timer timer;

	unordered_map<int, int> id_map; // map from real id to continuous id
	unordered_set<int> q4_tag_ids;	// tag id (real id) used in q4
#ifdef GOOGLE_HASH
	q4_tag_ids.set_empty_key(-1);
	id_map.set_empty_key(-1);
#endif
	int tid, pid;
	{		// read tag and tag names
		safe_open(dir + "/tag.csv");
		fgets(buffer, BUFFER_LEN, fin);
		while (fscanf(fin, "%d|", &tid) == 1) {
			int k = 0; char c;
			while ((c = (char)fgetc(fin)) != '|')
				buffer[k++] = c;
			string tag_name(buffer, k);
			id_map[tid] = (int)Data::tag_name.size();

			if (q4_tag_set.count(tag_name))		// cache all q4 tid (real tid)
				q4_tag_ids.insert(tid);
#ifdef DEBUG
			Data::real_tag_id.emplace_back(tid);
#endif
			Data::tagid[tag_name] = (int)Data::tag_name.size();
			Data::tag_name.emplace_back(move(tag_name));
			m_assert(Data::tagid.count(tag_name) == 0);		//  no repeated tag name
			fgets(buffer, BUFFER_LEN, fin);
		}
		Data::ntag = (int)Data::tag_name.size();
		fclose(fin);
	}
	Data::person_in_tags.resize(Data::ntag);
	q4_tag_set.clear();

	{		// read person->tags
		safe_open(dir + "/person_hasInterest_tag.csv");
		fgets(buffer, BUFFER_LEN, fin);
		while (fscanf(fin, "%d|%d", &pid, &tid) == 2) {
			int c_id = id_map[tid];
			Data::tags[pid].insert(c_id);
			Data::person_in_tags[c_id].emplace_back(pid);
		}
		fclose(fin);
	}

	//read places, need tag data to sort
	read_places(dir);

	tag_read = true;
	tag_read_cv.notify_all();

	print_debug("Read tag and places spent %lf secs\n", timer.get_time());
	read_forum(dir, id_map, q4_tag_ids);
}

void read_org_places(const string& fname, const vector<int>& org_places) {
	char buffer[1024];
	safe_open(fname);
	fgets(buffer, BUFFER_LEN, fin);
	int oid, pid;
	while (fscanf(fin, "%d|%d", &pid, &oid) == 2) {
		fgets(buffer, BUFFER_LEN, fin);
		m_assert(oid % 10 == 0);

		Data::places[org_places[oid / 10]].persons.emplace_back(pid);
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
			Data::placeid[place_name].emplace_back(pid);
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
			Data::places[p2].sub_places.emplace_back(&Data::places[p1]);
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
			Data::places[place].persons.emplace_back(person);
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
			org_places.emplace_back(pid);
		}
		fclose(fin);
	}

	read_org_places(dir + "/person_studyAt_organisation.csv", org_places);
	read_org_places(dir + "/person_workAt_organisation.csv", org_places);

	// sort and unique
	FOR_ITR(it, Data::places) {
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
}
