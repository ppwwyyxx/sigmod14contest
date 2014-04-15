//class : binary read

#pragma once
#include <string>
#include <vector>


class bread
{
public:
	std::vector<std::vector<int> > people;
	std::vector<int> owner;
	char* ptr, *buf_end;
	size_t size;

	int n_vst;

	void init(const std::string &dir);
	bool check(int a, int b, int threshold);
	bool check_oneside(int a, int b, int threshold);
};

