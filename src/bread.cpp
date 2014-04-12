#include <stdlib.h>
#include <algorithm>
#include <mutex>
#include <cstdio>
#include <fstream>
#include <thread>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "bread.h"

#include "lib/debugutils.h"
#include "lib/utils.h"
#include "lib/Timer.h"
#include "lib/common.h"
#include "read.h"
#include "cache.h"
#include "data.h"
#include "lib/fast_read.h";
using namespace std;


void bread::init(const std::string &dir)
{
	static char buffer[BUFFER_LEN];
	people.resize(1000010);
	safe_open(dir + "/comment_hasCreator_person.csv");
	ptr = buffer, buf_end = ptr + 1;

	READ_TILL_EOL();
	unsigned long long cid;
	int pid;
	if (Data::nperson > 9000)
		owner.reserve(20100000);
	while (true) {
		READ_ULL(cid);
		if (buffer == buf_end) break;
		READ_INT(pid);
		m_assert(cid % 10 == 0);
		m_assert(cid / 10 == owner.size());
		people[pid].emplace_back(cid);
		owner.emplace_back(pid);
	}
	fclose(fin);


	int fd = open((dir + "/comment_replyOf_comment.csv").c_str(), O_RDONLY);
	struct stat s; fstat(fd, &s);
	size = s.st_size;
	void* mapped = mmap(0, size, PROT_READ, MAP_FILE|MAP_PRIVATE|MAP_POPULATE, fd, 0);
	madvise(mapped, size, MADV_WILLNEED);
	ptr = (char*)mapped;
	buf_end = (char*)mapped + size;
	
	MMAP_READ_TILL_EOL();
	
}

int bread::check(int a, int b, int threshold)
{
	int tot = 0;
	for (int i = 0; i < (int) people[a].size(); i ++)
	{
		int cid = people[a][i];
		
		for (int lo = 0, hi = (int) size - 1; lo <= hi; )
		{
			int mid = (lo + hi) >> 1;
			
			char* lp = ptr + mid;
			
			while ((*lp) != '\n') lp --;
			lp ++;
			
			unsigned long long cid1 = 0;
			do {
				cid1 = cid1 * 10 + *lp - '0';
				lp ++;
			} while (*lp != '|');
			lp ++;
			unsigned long long cid2 = 0;
			do {
				cid2 = cid2 * 10 + *lp - '0';
				lp ++;
			} while (*lp != '\n');
			
			if (cid1 == cid)
			{
				if (owner[cid2 / 10] == b)
					tot ++;
				break;
			}
			if (cid1 < cid)
				lo = mid + 1;
			else hi = mid - 1;
		}
		
		if (tot > threshold) return 0;
	}
	return 1;
}			
	
	
