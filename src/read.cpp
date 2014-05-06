//File: read.cpp
//Date: Mon May 05 23:15:01 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

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

void build_friends_hash() {
	REP(i, Data::nperson) {
		auto &f = Data::friends[i];
		FOR_ITR(ff, f)
			Data::friends_hash[i].insert(ff->pid);
	}
	friends_hash_built = true;
	friends_hash_built_cv.notify_all();
	print_debug("Friends hash built\n");
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
		Data::friends[p1].emplace_back(p2, 0);
	}
	REP(i, Data::nperson)
		sort(Data::friends[i].begin(), Data::friends[i].end());		// sort by id!
	thread t(build_friends_hash);
	t.detach();
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


		// new read
/*
 *        size_t ncomment = 0;
 *        while (true) {
 *            buf_end = buffer + fread(buffer, 1, BUFFER_LEN, fin);
 *            if (buf_end == buffer) break;
 *            ptr = buffer;
 *            while (ptr != buf_end) {
 *                if (*ptr == '\n') ncomment ++;
 *                ptr ++;
 *            }
 *        }
 *        PP(ncomment);
 *        PP(timer.get_time());
 *        owner.resize(ncomment);
 *        PP(timer.get_time());
 *
 *        if (Data::nperson > 11000)
 *            fprintf(stderr, "s:%.4lf\n", timer.get_time());
 *
 *        rewind(fin);
 *        ptr = buffer, buf_end = ptr + 1;
 */
		// end of new read



		READ_TILL_EOL();
		unsigned long long cid;
		int pid;
		//if (Data::nperson > 90000) owner.reserve(65000000);
		if (Data::nperson > 9000) owner.reserve(20100000);
		while (true) {
			READ_INT(cid);
			if (buffer == buf_end) break;
			READ_INT(pid);
			m_assert(cid % 10 == 0);
		//	owner[cid / 10] = pid;		// new read
			owner.push_back(pid);		// old read
		}
		fclose(fin);
	}
	if (Data::nperson > 11000)
		fprintf(stderr, "1:%.4lf\n", timer.get_time());


//	int max_diff = 0;
	// read comment->comment
	vector<unordered_map<int, int>> comment_map(Data::nperson);
#ifdef GOOGLE_HASH
	FOR_ITR(itr, comment_map) itr->set_empty_key(-1);
#endif
	{
		GuardedTimer guarded_timer("read comment_replyOf_comment.csv");

		// using mmap
		int fd = open((dir + "/comment_replyOf_comment.csv").c_str(), O_RDONLY);
		struct stat s; fstat(fd, &s);
		size_t size = s.st_size;
		PP(size);
		void* mapped = mmap(0, size, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
		madvise(mapped, size, MADV_WILLNEED);

		ptr = (char*)mapped;
		buf_end = (char*)mapped + size;

		do { ptr++; } while (*ptr != '\n');
		ptr ++;
		do {
			unsigned long long cid1 = 0;
			do {
				cid1 = cid1 * 10 + *ptr - '0';
				ptr ++;
			} while (*ptr != '|');
			ptr ++;
			unsigned long long cid2 = 0;
			do {
				cid2 = cid2 * 10 + *ptr - '0';
				ptr ++;
			} while (*ptr != '\n');

			int p1 = owner[cid1 / 10], p2 = owner[cid2 / 10];
			if (p1 != p2) {
				comment_map[p1][p2] += 1;		// p1 reply to p2
			}

			ptr ++;
		} while (ptr != buf_end);
		munmap(mapped, size);
		// end of using mmap
		/*
		 *                safe_open(dir + "/comment_replyOf_comment.csv");
		 *                ptr = buffer, buf_end = ptr + 1;
		 *
		 *                READ_TILL_EOL();
		 *                int cid1, cid2;
		 *                while (true) {
		 *                    READ_INT(cid1);
		 *                    if (buffer == buf_end) break;
		 *                    READ_INT(cid2);		// max difference cid1 - cdi2 is 180
		 *                    int p1 = owner[cid1 / 10], p2 = owner[cid2 / 10];
		 *                    comment_map[p1][p2] += 1;		// p1 reply to p2
		 *                }
		 *                fclose(fin);
		 */
	}
	//	PP(max_diff);
	if (Data::nperson > 11000)
		fprintf(stderr, "2:%.4lf\n", timer.get_time());

	// omp likely to crash?
	//#pragma omp parallel for schedule(static) num_threads(4)
	REP(i, Data::nperson) {
		auto& fs = Data::friends[i];
		auto& m = comment_map[i];
		FOR_ITR(itr, fs) {
			itr->ncmts = min(m[itr->pid], comment_map[itr->pid][i]);
		}
	}
	if (Data::nperson > 11000)
		fprintf(stderr, "e:%.4lf\n", timer.get_time());
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
#ifdef GOOGLE_HASH
	FOR_ITR(itr, comment_map) itr->set_empty_key(-1);
#endif
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

void destroy_tag_name();
void read_forum(const string& dir, unordered_map<int, int>& id_map, const unordered_set<int>& q4_tag_ids) {
	static char buffer[BUFFER_LEN];
	Timer timer;
	char* ptr, *buf_end;
	int fid, tid, pid;
	unordered_map<int, vector<int>> forum_to_tags;		// fid -> continuous tids
#ifdef GOOGLE_HASH
	forum_to_tags.set_empty_key(-1);
#endif
	{
		GuardedTimer timer("read forum_hasTag_tag");
		safe_open(dir + "/forum_hasTag_tag.csv");
		ptr = buffer, buf_end = ptr + 1;
		READ_TILL_EOL();

		int last_fid = -1; vector<int>* last_ptr = NULL;
		while (true) {
			READ_INT(fid);
			if (buffer == buf_end) break;
			READ_INT(tid);

			if (not q4_tag_ids.count(tid))
				continue;
			m_assert(id_map.find(tid) != id_map.end());

			int c_tid = id_map[tid];

			if (fid != last_fid) {
				auto & v = forum_to_tags[fid];
				v.emplace_back(c_tid);
				last_ptr = &v;
			} else {
				last_ptr->emplace_back(c_tid);
			}

			last_fid = fid;
		}
		fclose(fin);
	}
	PP(forum_to_tags.size());

	{
		GuardedTimer timer("read forum_hasMember_person");

		// using mmap
		int fd = open((dir + "/forum_hasMember_person.csv").c_str(), O_RDONLY);
		struct stat s; fstat(fd, &s);
		size_t size = s.st_size;
		void* mapped = mmap(0, size, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
//		madvise(mapped, size, MADV_WILLNEED);
		madvise(mapped, size, MADV_SEQUENTIAL);

		ptr = (char*)mapped;
		buf_end = (char*)mapped + size;

		MMAP_READ_TILL_EOL();

		int last_fid = -1;
		bool last_skip = false;
		vector<vector<bool>*> hashes;
		do {
			fid = 0;
			do {
				fid = fid * 10 + *ptr - '0';
				ptr ++;
			} while (*ptr != '|');
			ptr ++;
			// read fid done

			if (fid != last_fid) {
				last_fid = fid;
				auto itr = forum_to_tags.find(fid);
				if (itr == forum_to_tags.end()) {
					last_skip = true;
					MMAP_READ_TILL_EOL();
					continue;
				}
				last_skip = false;
				hashes.clear();
				FOR_ITR(titr, itr->second)
					hashes.emplace_back(&q4_persons[Data::tag_name[*titr]]);
			} else {
				if (last_skip) {
					MMAP_READ_TILL_EOL();
					continue;
				}
			}

			pid = 0;
			do {
				pid = pid * 10 + *ptr - '0';
				ptr ++;
			} while (*ptr != '|');

			FOR_ITR(hs, hashes)
				(*(*hs))[pid] = true;

			MMAP_READ_TILL_EOL();
		} while (ptr != buf_end);
		munmap(mapped, size);
		close(fd);
	}

	thread th(destroy_tag_name);
	th.detach();

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
		fgets(buffer, 1024, fin);
		string tag_name;
		while (fscanf(fin, "%d|", &tid) == 1) {
			char c;
			tag_name.clear();
			while ((c = (char)fgetc(fin)) != '|')
				tag_name += c;
			id_map[tid] = (int)Data::tag_name.size();

			if (q4_tag_set.count(tag_name))		// cache all q4 tid (real tid)
				q4_tag_ids.insert(tid);
#ifdef DEBUG
			Data::real_tag_id.emplace_back(tid);
#endif
			Data::tag_name.emplace_back(move(tag_name));
			fgets(buffer, 1024, fin);
		}
		Data::ntag = (int)Data::tag_name.size();
		fclose(fin);
	}
	Data::person_in_tags.resize(Data::ntag);

	/*
	 *FOR_ITR(nameitr, q4_tag_set)
	 *    q4_tag_ids.insert(Data::tagid[*nameitr]);
	 */

	FOR_ITR(nameitr, q4_tag_set) {
		q4_persons[*nameitr].resize(Data::nperson, false);
	}
	q4_tag_set = unordered_set<string, StringHashFunc>();

	{		// read person->tags
		safe_open(dir + "/person_hasInterest_tag.csv");
		fgets(buffer, 1024, fin);
		while (fscanf(fin, "%d|%d", &pid, &tid) == 2) {
			int c_id = id_map[tid];
			Data::tags[pid].insert(c_id);
			Data::person_in_tags[c_id].emplace_back(pid);
		}
		fclose(fin);
	}

	//read places, need tag data to sort
	thread th(bind(read_places, dir));
	th.detach();

	print_debug("Read tag and places spent %lf secs\n", timer.get_time());
	read_forum(dir, id_map, q4_tag_ids);
}

void read_org_places(const string& fname, const vector<int>& org_places) {
	char buffer[2048];
	safe_open(fname);
	fgets(buffer, 2048, fin);
	int oid, pid;
	while (fscanf(fin, "%d|%d", &pid, &oid) == 2) {
		fgets(buffer, 2048, fin);
		m_assert(oid % 10 == 0);

		Data::places[org_places[oid / 10]].persons.emplace_back(pid);
	}
	fclose(fin);
}

void build_places_tree(const string& dir) {
	char buffer[2048];
	int pid, max_pid = 0;
	{
		safe_open(dir + "/place.csv");
		fgets(buffer, 2048, fin);
		string place_name;
		while (fscanf(fin, "%d|", &pid) == 1) {
			place_name.clear();
			char c;
			while ((c = (char)fgetc(fin)) != '|')
				place_name += c;
			Data::placeid[place_name].emplace_back(pid);
			update_max(max_pid, pid);
			fgets(buffer, 2048, fin);
		}
		Data::places.resize(max_pid + 1);
		fclose(fin);
	}

	{
		safe_open(dir + "/place_isPartOf_place.csv");
		fgets(buffer, 2048, fin);
		int p1, p2;
		while (fscanf(fin, "%d|%d", &p1, &p2) == 2) {
			Data::places[p2].sub_places.emplace_back(&Data::places[p1]);
		}
		fclose(fin);
	}
}

void read_places(string dir) {
	GuardedTimer tt("read places");
	char buffer[1024];
	build_places_tree(dir);

	{
		safe_open(dir + "/person_isLocatedIn_place.csv");
		fgets(buffer, 1024, fin);
		int person, place;
		while (fscanf(fin, "%d|%d", &person, &place) == 2) {
			Data::places[place].persons.emplace_back(person);
		}
		fclose(fin);
	}

	vector<int> org_places;
	{
		safe_open(dir + "/organisation_isLocatedIn_place.csv");
		fgets(buffer, 1024, fin);
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

	// q3 and q2 will start here
	tag_read = true;
	tag_read_cv.notify_all();
}

void read_data(const string& dir) {		// may need to be implemented synchronously
	DEBUG_DECL(Timer, timer);
	read_person_file(dir);
	read_person_knows_person(dir);
}

/*		// cannot compile
 *void quick_sort(std::vector<PII>& arr) {
 *    auto pivot = arr[rand() % arr.size()];
 *    auto mid = std::partition(arr.begin(), arr.end(), [&](const PII &t){return t < pivot;});
 *
 *    auto pleft = arr[rand() % (mid - arr.begin())];
 *    auto pright = arr[rand() % (arr.end() - mid)];
 *    std::vector<PII>::iterator left, right;
 *
 *    thread t_left([&](){
 *            left = std::partition(arr.begin(), mid, [&](const PII &t){return t < pleft;});
 *            });
 *    thread t_right([&](){
 *            right = std::partition(mid, arr.end(), [&](const PII &t){return t < pright;});
 *            });;
 *
 *    t_left.join();
 *    t_right.join();
 *
 *    vector<thread> threads;
 *    std::vector<PII>::iterator begin = arr.begin();
 *    for (auto &end: {left , mid, right, arr.end()}) {
 *        threads.emplace_back(std::bind(
 *            [](std::vector<PII>::iterator begin,
 *                std::vector<PII>::iterator end){
 *            std::sort(begin, end);},
 *            begin, end));
 *        begin = end;
 *    }
 *
 *    for (auto &t: threads) t.join();
 *}
 */


int find_count(const std::vector<std::pair<std::pair<int, int>, int>> &count, const std::pair<int, int> &pivot) {
	// lower bound
	int left = 0, right = (int)count.size();
	while (left + 1 < right) {
		int mid = (left + right) >> 1;
		if (count[mid].first <= pivot) {
			left = mid;
		} else {
			right = mid;
		}
	}

	if (count[left].first == pivot)
		return count[left].second;
	return 0;
}

void read_comments_tim(const std::string &dir) {
	char *ptr, *buf_end;

	vector<int> owner;
	Timer timer;
	{
		GuardedTimer guarded_timer("read comment_hasCreator_person.csv%d", 1);
		int fd = open((dir + "/comment_hasCreator_person.csv").c_str(), O_RDONLY);
		struct stat s; fstat(fd, &s);
		size_t size = s.st_size;
		void* mapped = mmap(0, size, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
//		madvise(mapped, size, MADV_WILLNEED);

//		ptr = (char*) mapped;
//		buf_end = (char*) mapped + size;

//		MMAP_READ_TILL_EOL();
//		if (Data::nperson > 9000) owner.reserve(20100000);


		ptr = (char*) mapped;
		buf_end = (char*) mapped + size;

		char* seek = buf_end - 1024;
		while (*seek++ != '\n');
		ULL cid = 0;
		while (*seek != '|') cid = cid * 10 + (*(seek++) - '0');
		fprintf(stderr, "ncmt<%llu\n", cid); fflush(stderr);
		madvise(mapped, size, MADV_SEQUENTIAL);
		MMAP_READ_TILL_EOL();
		owner.reserve(cid / 10 + 1000);

		do {
			do { ptr ++; } while (*ptr != '|');
			ptr ++;
			int pid = 0;
			do {
				pid = pid * 10 + *ptr - '0';
				ptr ++;
			} while (*ptr != '\n');
			owner.emplace_back(pid);
			ptr ++;
		} while (ptr != buf_end);
		munmap(mapped, size);
		close(fd);
	}

	WAIT_FOR(friends_hash_built);

	vector<PII> comments;
	{
		GuardedTimer guarded_timer("read comment_replyOf_comment.csv");

		// using mmap
		int fd = open((dir + "/comment_replyOf_comment.csv").c_str(), O_RDONLY);
		struct stat s; fstat(fd, &s);
		size_t size = s.st_size;
		void* mapped = mmap(0, size, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
//		madvise(mapped, size, MADV_WILLNEED);
		madvise(mapped, size, MADV_SEQUENTIAL);

		ptr = (char*)mapped;
		buf_end = (char*)mapped + size;

		MMAP_READ_TILL_EOL();
		do {
			unsigned long long cid1 = 0;
			do {
				cid1 = cid1 * 10 + *ptr - '0';
				ptr ++;
			} while (*ptr != '|');
			ptr ++;
			unsigned long long cid2 = 0;
			do {
				cid2 = cid2 * 10 + *ptr - '0';
				ptr ++;
			} while (*ptr != '\n');

			int p1 = owner[cid1 / 10], p2 = owner[cid2 / 10];
			if (p1 != p2) {
				auto &h = Data::friends_hash[p1];
				if (h.find(p2) != h.end())
					comments.emplace_back(p1, p2);
			}

			ptr ++;
		} while (ptr != buf_end);
		munmap(mapped, size);
		close(fd);
	}
	Data::friends_hash = vector<unordered_set<int>>();

	print_debug("number of valid comment pair: %lu\n", comments.size());
	{
		//	GuardedTimer guarded_timer("sort");		// very fast
		std::sort(comments.begin(), comments.end());
		//	quick_sort(comments);		// lambda cannot compile!
	}

	// aggregate, very fast
	std::vector<pair<PII, int>> count;
	count.emplace_back(comments[0], 1);
	for (size_t i = 1, d = comments.size(); i < d; i ++) {
		auto &cur = comments[i];
		if (cur != comments[i - 1])
			count.emplace_back(cur, 1);
		else
			count.back().second ++;
	}


	{
		GuardedTimer timer("build graph");		// very fast (0.05s/300k)
		int index = 0;
		REP(i, Data::nperson) {
			auto& fs = Data::friends[i];
			FOR_ITR(itr, fs) {
				int j = itr->pid;
				PII now_pair = make_pair(i, j);
				while (count[index].first < now_pair) {
					index ++;
					if (index == (int)count.size()) break;
				}
				if (index == (int)count.size()) break;

				if (count[index].first == now_pair)
					itr->ncmts = min(count[index].second, find_count(count, make_pair(j, i)));
				else
					itr->ncmts = 0;
			}
			if (index == (int)count.size()) break;
		}
	}
	print_debug("Read comment spent %lf secs\n", timer.get_time());
}

