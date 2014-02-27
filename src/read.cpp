//File: read.cpp
//Date: Thu Feb 27 15:40:45 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "lib/debugutils.h"
#include "lib/utils.h"
#include "data.h"
#include <stdlib.h>
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
		// map & list
		Data::pp_map[p1][p2] = Data::pp_map[p2][p1] = true;
		Data::friends[p1].push_back(p2);
		Data::friends[p2].push_back(p1);
	}
	fclose(fin);
}
