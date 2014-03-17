//File: gen_query.cc
//Date: Sun Mar 16 12:02:50 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "data.h"
#include "read.h"
#include <cstdio>
#include <ctime>
#include <cstdlib>
#define random(l, r) (rand() % ((r) - (l) + 1) + (l))
using namespace std;

const int q1_max_comment = 4;
const int q2_max_k = 8, q2_min_year = 1980, q2_max_year = 2000;
const int q3_max_k = 8, q3_max_h = 7;
const int q4_max_k = 7;
const int days[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int num_q[4];

void make_q1()
{
	for (int i = 0; i < num_q[0]; ++i)
		printf("query1(%d, %d, %d)\n", 
				random(0, Data::nperson - 1), 
				random(0, Data::nperson - 1), 
				random(-1, q1_max_comment));
}

void make_q2()
{
	for (int i = 0; i < num_q[1]; ++i)
	{
		printf("query2(%d, ", random(1, q2_max_k));
		int year = random(q2_min_year, q2_max_year);
		int month = random(1, 12);
		int day = random(1, days[month - 1]);
		printf("%d-", year);
		if (month < 10)
			printf("0");
		printf("%d-", month);
		if (day < 10)
			printf("0");
		printf("%d)\n", day);
	}
}

void make_q3()
{
	for (int i = 0; i < num_q[2]; ++i)
	{
		printf("query3(%d, %d, ",
				random(1, q3_max_k),
				random(1, q3_max_h));
		int p = random(0, Data::placeid.size() - 1);
		unordered_map<std::string, std::vector<int>, StringHashFunc>::iterator it = Data::placeid.begin();
		while (p--)
			++it;
		cout << it->first << ")" << endl;
	}
}

void make_q4()
{
	for (int i = 0; i < num_q[3]; ++i)
	{
		printf("query4(%d, ", random(1, q4_max_k));
		cout << Data::tag_name[random(0, Data::tag_name.size() - 1)] << ")" << endl;
	}
}

int main(int argc, char* argv[]) {
	srand((unsigned)time(NULL));
	read_data(string(argv[1]));
	freopen(argv[2], "w", stdout);
	for (int i = 3; i < 7; ++i)
		num_q[i - 3] = atoi(argv[i]);
	make_q1();
	make_q2();
	make_q3();
	make_q4();
	return 0;
}
