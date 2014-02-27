//File: read.cpp
//Date: Thu Feb 27 16:26:51 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "lib/debugutils.h"
#include "lib/utils.h"
#include "data.h"
#include <stdlib.h>
#include <algorithm>
#include <stdio.h>
using namespace std;

inline FILE* safe_open(const string& fname) {
	FILE* fin = fopen(fname.c_str(), "r");
	m_assert(fin != NULL);
	return fin;
}

void read_data(const string& dir) {
#define MAX_LINE_LEN 65536
	char buf[MAX_LINE_LEN];

	// read person file
	FILE* fin = safe_open(dir + "/person.csv");
	fgets(buf, MAX_LINE_LEN, fin);
	int pid, maxid = 0;
	while (fscanf(fin, "%d|", &pid) == 1) {
		fgets(buf, MAX_LINE_LEN, fin);
		update_max(maxid, pid);
	}
	fclose(fin);
	P("Max id of Nodes: "); P(maxid);P("\n");
	Data::allocate(maxid);


	// read person knows person
	fin = safe_open(dir + "/person_knows_person.csv");
	fgets(buf, MAX_LINE_LEN, fin);
	int p1, p2;
	while (fscanf(fin, "%d|%d", &p1, &p2) == 2) {
		// map
		Data::pp_map[p1][p2] = Data::pp_map[p2][p1] = true;
	}
	fclose(fin);

	// read comment->person
	fin = safe_open(dir + "/comment_hasCreator_person.csv");
	fgets(buf, MAX_LINE_LEN, fin);
	int cid, cid2;
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

	while (fscanf(fin, "%d|%d", &cid, &cid2) == 2) {
		p1 = owner[cid / 10], p2 = owner[cid2 / 10];
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
			int nc = comment_map[i][j];
			Data::friends[i].push_back(ConnectedPerson(j, nc));
			Data::friends[j].push_back(ConnectedPerson(i, nc));
		}
		sort(Data::friends[i].begin(), Data::friends[i].end());
	}
	free_2d<int>(comment_map, Data::nperson);
}
