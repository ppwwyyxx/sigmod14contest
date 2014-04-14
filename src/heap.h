#include <vector>
#include "lib/debugutils.h"
using namespace std;

struct HeapEle {
	int range;      // defined in the problem
	int tag_id;
	HeapEle(int _range = -1, int _tag_id = -1)
		:range(_range), tag_id(_tag_id) {}

    bool operator <(const HeapEle &b) const {
		return (range > b.range) || ((range == b.range) && (Data::tag_name[tag_id] < Data::tag_name[b.tag_id]));
	}
};

class MyHeap {
public:
    MyHeap() :heap(1) {}
    void insert(int id, int key) {
        place[id] = (int) heap.size();
        heap.push_back(HeapEle(key, id));
        move_up((int) heap.size() -1);
    }
    HeapEle get() {
        HeapEle ans = heap[1];
        heap[1] = heap[heap.size() -1];
        place[heap[1].tag_id] = 1;
        heap.pop_back();
        move_down(1);
        return ans;
    }
    void change(int id, int key) {
        if (heap[place[id]].range >= key) return ;
        heap[place[id]].range = key;
        move_down(place[id]);
        move_up(place[id]);
    }
    HeapEle top() {
        if ((int) heap.size() == 1) return HeapEle(-1, -1);
        return heap[1];
    }
    void resize(int _n) {
        place.resize(_n+1);
    }

	void free() {
		place = vector<int>();
		heap = vector<HeapEle>();
	}

private:
    vector<int> place;
    vector<HeapEle> heap;

    void swap(int i, int j) {
        HeapEle temp = heap[i]; heap[i] = heap[j]; heap[j] = temp;
        place[heap[i].tag_id] = i;
        place[heap[j].tag_id] = j;
    }
    void move_down(int p) {
        while (true) {
            int p2 = p;
            if ((p << 1) < (int) heap.size() && heap[p << 1] < heap[p2]) p2 = p << 1;
            if ((p << 1)+1 < (int) heap.size() && heap[(p << 1)+1] < heap[p2]) p2 = (p << 1) + 1;
            if (p == p2) break;
            swap(p, p2);
            p = p2;
        }
    }
    void move_up(int p) {
        while (p > 1 && heap[p] < heap[p>>1])  {
            swap(p, p>>1);
            p = p >> 1;
        }
    }
    bool check() {
        for (int i=2; i<(int) heap.size(); i++) {
            if (heap[i] < heap[i >> 1]) return false;
            if (place[heap[i].tag_id] != i) {print_debug("Oh, no!\n");m_assert(0);return false;}
        }
        return true;
    }
};
