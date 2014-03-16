//File: cache.h
//Date: Sun Mar 16 16:13:32 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once
#include <vector>
#include <memory>
#include "lib/hash_lib.h"

typedef unordered_set<int> NeighborT;

class CachedNeighbor {
	public:
		/*
		 *CachedNeighbor(std::vector<int>&& n1, std::vector<int>&& n2):
		 *    neighbor_1(std::move(n1)), neighbor_2(std::move(n2)){}
		 */

		// must be sorted
		NeighborT neighbor_1;
		NeighborT neighbor_2;

		static const NeighborT& get_neighbor_1(int person);
		static const NeighborT& get_neighbor_2(int person);


		static void init(int nperson) {
			cached1.resize(nperson);
			cached2.resize(nperson);
			data.resize(nperson);
		}
	private:
		static std::vector<CachedNeighbor> data;
		static std::vector<bool> cached1, cached2;
};

int intersect_cnt(const NeighborT& n, const unordered_set<int>& large);
