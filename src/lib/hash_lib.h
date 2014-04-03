//File: hash_lib.h
//Date: Thu Apr 03 16:07:51 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once

#include <string>
#include <cstring>

#ifdef GOOGLE_HASH

#include <google/dense_hash_map>
#include <google/dense_hash_set>
using google::dense_hash_map;
using google::dense_hash_set;
#define unordered_map dense_hash_map
#define unordered_set dense_hash_set

#else

#include <unordered_map>
#include <unordered_set>
using std::unordered_map;
using std::unordered_set;

#endif


uint64_t MurmurHash64A ( const void * key, int len, unsigned int seed );

// simple hash adapter for types without pointers
template<typename T>
struct MurmurHasher {
	size_t operator()(const T& t) const {
		return MurmurHash64A(&t, sizeof(t), 0);
	}
};

// specialization for strings
template<>
struct MurmurHasher<std::string> {
	size_t operator()(const std::string& t) const {
		return MurmurHash64A(t.c_str(), (int)t.size(), 0);
	}
};

template<>
struct MurmurHasher<const char*> {
	size_t operator()(const char*& t) const {
		return MurmurHash64A(t, (int)strlen(t), 0);
	}
};

typedef MurmurHasher<std::string> StringHashFunc;

/*
 *struct eqstr {
 *    bool operator()(const char* s1, const char* s2) const
 *    { return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0); }
 *};
 */
namespace std {
	template <>
	struct hash<std::pair<int, int>> {
		public:
			size_t operator()(const std::pair<int, int> &p) const {
				return hash<int>()(p.first) ^ hash<int>()(p.second);
			}
	};
}
