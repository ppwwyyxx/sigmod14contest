//File: read.cpp
//Date: Thu Feb 27 22:55:56 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "lib/debugutils.h"
#include "lib/utils.h"
#include "data.h"
#include <stdlib.h>
#include <map>
#include <algorithm>
#include <stdio.h>
using namespace std;

inline FILE* safe_open(const string& fname) {
	FILE* fin = fopen(fname.c_str(), "r");
	m_assert(fin != NULL);
	return fin;
}

void read_data(const string& dir) {		// may need to be implemented synchronously
#define MAX_LINE_LEN 65536
	char buf[MAX_LINE_LEN];

	// read person file
	FILE* fin = safe_open(dir + "/person.csv");
	fgets(buf, MAX_LINE_LEN, fin);
	int pid, maxid = 0;
	while (fscanf(fin, "%d|", &pid) == 1) {
		update_max(maxid, pid);
		fgets(buf, MAX_LINE_LEN, fin);
	}
	P("Max id of Nodes: "); P(maxid);P("\n");
	Data::allocate(maxid);

	{		// read birthday
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

	{		// read person knows person
		fin = safe_open(dir + "/person_knows_person.csv");
		fgets(buf, MAX_LINE_LEN, fin);
		int p1, p2;
		while (fscanf(fin, "%d|%d", &p1, &p2) == 2) {
			Data::pp_map[p1][p2] = Data::pp_map[p2][p1] = true;
		}
		fclose(fin);
	}

	{		// read comment->person
		fin = safe_open(dir + "/comment_hasCreator_person.csv");
		fgets(buf, MAX_LINE_LEN, fin);
		unsigned cid;
		vector<int> owner;
		owner.reserve(400000);
		while (fscanf(fin, "%d|%d", &cid, &pid) == 2) {
			m_assert(cid % 10 == 0);
			m_assert(cid / 10 == owner.size());
			owner.push_back(pid);
		}
		fclose(fin);

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

	{		// tags
		map<int, int> id_map; // map from real id to continuous id
		int tid;
		{		// read tag and tag names
			fin = safe_open(dir + "/tag.csv");
			fgets(buf, MAX_LINE_LEN, fin);
			while (fscanf(fin, "%d|", &tid) == 1) {
				int k = 0;
				char c;
				while ((c = (char)fgetc(fin)) != '|') buf[k++] = c;
				string tag_name(buf, k);
				id_map[tid] = (int)Data::tagname.size();
				Data::tagname.push_back(tag_name);
				fgets(buf, MAX_LINE_LEN, fin);
			}
			Data::ntag = (int)Data::tagname.size();
			fclose(fin);
		}

		{		// read person->tags
			fin = safe_open(dir + "/person_hasInterest_tag.csv");
			fgets(buf, MAX_LINE_LEN, fin);
			while (fscanf(fin, "%d|%d", &pid, &tid) == 2) {
				Data::tags[pid].insert(id_map[tid]);
			}
			fclose(fin);
		}
	}
}
