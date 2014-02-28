//File: read.cpp
//Date: Fri Feb 28 12:02:50 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "lib/debugutils.h"
#include "lib/utils.h"
#include "lib/Timer.h"
#include "data.h"
#include <stdlib.h>
#include <map>
#include <algorithm>
#include <stdio.h>
using namespace std;
using namespace std::tr1;

inline FILE* safe_open(const string& fname) {
	FILE* fin = fopen(fname.c_str(), "r");
	m_assert(fin != NULL);
	return fin;
}

#define MAX_LINE_LEN 65536
namespace {
	char buf[MAX_LINE_LEN];
}

void read_person_file(const string& dir) {
	FILE* fin = safe_open(dir + "/person.csv");
	fgets(buf, MAX_LINE_LEN, fin);
	int pid, maxid = 0;
	while (fscanf(fin, "%d|", &pid) == 1) {
		update_max(maxid, pid);
		fgets(buf, MAX_LINE_LEN, fin);
	}
	P("Max id of Nodes: "); P(maxid);P("\n");
	Data::allocate(maxid);

	// read birthday
	rewind(fin);
	fgets(buf, MAX_LINE_LEN, fin);
	int year, month, day;
	while (fscanf(fin, "%d|", &pid) == 1) {
		for (int i = 0; i < 3; i ++) while (fgetc(fin) != '|') {}
		fscanf(fin, "%d-%d-%d", &year, &month, &day);
		fgets(buf, MAX_LINE_LEN, fin);
		Data::birthday[pid] = year * 10000 + month * 100 + day;
	}
	fclose(fin);
}

void read_person_knows_person(const string& dir) {
	FILE* fin = safe_open(dir + "/person_knows_person.csv");
	fgets(buf, MAX_LINE_LEN, fin);
	int p1, p2;
	while (fscanf(fin, "%d|%d", &p1, &p2) == 2) {
		Data::pp_map[p1][p2] = Data::pp_map[p2][p1] = true;
	}
	fclose(fin);
}

void read_comments(const string& dir) {
	Timer timer;
	print_debug("%lf\n", timer.get_time());
	FILE* fin = safe_open(dir + "/comment_hasCreator_person.csv");
	fgets(buf, MAX_LINE_LEN, fin);
	unsigned cid, pid;
	vector<int> owner;
	owner.reserve(4000000);
	while (fscanf(fin, "%d|%d", &cid, &pid) == 2) {
		m_assert(cid % 10 == 0);
		m_assert(cid / 10 == owner.size());
		owner.push_back(pid);
	}
	fclose(fin);

	print_debug("After Read comment creator: %lf seconds\n", timer.get_time());
	// read comment->comment
	fin = safe_open(dir + "/comment_replyOf_comment.csv");
	fgets(buf, MAX_LINE_LEN, fin);

	int ** comment_map = new int*[Data::nperson];
	for (int i = 0; i < Data::nperson; i ++) comment_map[i] = new int[Data::nperson]();

	int cid1, cid2;
	while (fscanf(fin, "%d|%d", &cid1, &cid2) == 2) {
		int p1 = owner[cid1 / 10], p2 = owner[cid2 / 10];
		if (not Data::pp_map[p1][p2]) continue;
		if (p1 < p2)
			comment_map[p1][p2] += 1;
		else
			comment_map[p2][p1] += 1;
	}
	fclose(fin);
	print_debug("After Read comment replyOf: %lf seconds\n", timer.get_time());

	for (int i = 0; i < Data::nperson; i ++) {
		for (int j = i + 1; j < Data::nperson; j ++) {
			if (not Data::pp_map[i][j]) continue;
			int nc = comment_map[i][j];		// i < j
			Data::friends[i].push_back(ConnectedPerson(j, nc));
			Data::friends[j].push_back(ConnectedPerson(i, nc));
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
		fgets(buf, MAX_LINE_LEN, fin);
		while (fscanf(fin, "%d|", &tid) == 1) {
			int k = 0; char c;
			while ((c = (char)fgetc(fin)) != '|')
				buf[k++] = c;
			string tag_name(buf, k);
			id_map[tid] = (int)Data::tag_name.size();
			Data::tag_name.push_back(tag_name);
			Data::tagid[tag_name] = (int)Data::tag_name.size() - 1;
			fgets(buf, MAX_LINE_LEN, fin);
		}
		Data::ntag = (int)Data::tag_name.size();
		fclose(fin);
	}

	{		// read person->tags
		FILE* fin = safe_open(dir + "/person_hasInterest_tag.csv");
		fgets(buf, MAX_LINE_LEN, fin);
		while (fscanf(fin, "%d|%d", &pid, &tid) == 2) {
			Data::tags[pid].insert(id_map[tid]);
		}
		fclose(fin);
	}

	unordered_map<int, Forum*> forum_idmap;
	Data::tag_forums.resize(Data::ntag);
	{
		FILE* fin = safe_open(dir + "/forum_hasMember_person.csv");
		fgets(buf, MAX_LINE_LEN, fin);
		while (fscanf(fin, "%d|%d", &fid, &pid) == 2) {
			unordered_map<int, Forum*>::const_iterator itr = forum_idmap.find(fid);
			if (itr != forum_idmap.end()) {
				itr->second->persons.push_back(PersonInForum(pid));
			} else {
				Forum* forum = new Forum();
#ifdef DEBUG
				forum->id = fid;
#endif
				forum->persons.push_back(PersonInForum(pid));
				forum_idmap[fid] = forum;
			}
			fgets(buf, MAX_LINE_LEN, fin);
		}
		fclose(fin);
	}
	{
		FILE* fin = safe_open(dir + "/forum_hasTag_tag.csv");
		fgets(buf, MAX_LINE_LEN, fin);
		while (fscanf(fin, "%d|%d", &fid, &tid) == 2) {
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
	fgets(buf, MAX_LINE_LEN, fin);
	int oid, pid;
	while (fscanf(fin, "%d|%d", &pid, &oid) == 2) {
		fgets(buf, MAX_LINE_LEN, fin);
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
		fgets(buf, MAX_LINE_LEN, fin);
		while (fscanf(fin, "%d|", &pid) == 1) {
			int k = 0; char c;
			while ((c = (char)fgetc(fin)) != '|')
				buf[k++] = c;
			string place_name(buf, k);
			Data::placeid[place_name] = pid;
			update_max(max_pid, pid);
			fgets(buf, MAX_LINE_LEN, fin);
		}
		Data::places.resize(max_pid + 1);
		fclose(fin);
	}

	{
		FILE* fin = safe_open(dir + "/place_isPartOf_place.csv");
		fgets(buf, MAX_LINE_LEN, fin);
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
	fgets(buf, MAX_LINE_LEN, fin);
	int person, place;
	while (fscanf(fin, "%d|%d", &person, &place) == 2) {
		Data::places[place].persons.push_back(
				PersonInPlace(person));
	}
	fclose(fin);

	vector<int> org_places;
	fin = safe_open(dir + "/organisation_isLocatedIn_place.csv");
	fgets(buf, MAX_LINE_LEN, fin);
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
	read_person_file(dir);
	read_person_knows_person(dir);
	read_comments(dir);
	read_tags_forums(dir);
	read_places(dir);
}
