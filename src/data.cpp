//File: data.cpp
//Date: Thu Feb 27 16:47:14 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "data.h"
#include "lib/debugutils.h"
#include "lib/utils.h"
using namespace std;

int Data::nperson= 0;
bool ** Data::pp_map = NULL;
int * Data::birthday = NULL;
vector<vector<ConnectedPerson> > Data::friends;

void Data::allocate(int max_pid) {
	m_assert(nperson == 0);
	Data::nperson = max_pid + 1;

	birthday = new int[nperson];
	pp_map = new bool*[nperson];
	for (int i = 0; i < nperson; i ++) {
		pp_map[i] = new bool[nperson]();
		friends.resize(nperson);
	}
}

void Data::free() {
	free_2d<bool>(pp_map, nperson);
}
