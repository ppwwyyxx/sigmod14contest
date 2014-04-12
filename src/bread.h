//class : binary read

#pragma once
#include <vector>


class bread
{
private :

	std::vector<std::vector<int>> people;
	std::vector<int> owner;
	char* ptr, *buf_end;
	size_t size;
	
public :
	void init(const std::string &dir);
	int check(int a, int b, int threshold);
	
};

