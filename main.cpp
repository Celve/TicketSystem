#include <algorithm>
#include <cstdio>
#include <random>

#include "buffer/buffer_pool_manager.h"
#include "common/rid.h"
#include "storage/index/b_plus_tree.h"

using namespace thomas;

class Comparator {
public:
    int operator()(int a, int b) {
        if (a < b) {
            return -1;
        }
        if (a == b) {
            return 0;
        }
        return 1;
    }

    Comparator() = default;

    Comparator(const Comparator &comp) = default;
};

int main() {
    DiskManager *disk_manager = new DiskManager("test.db");
    BufferPoolManager *bpm = new BufferPoolManager(50, disk_manager);
    FixedStringComparator<48> comparator;
    BPlusTree<FixedString<48>, size_t, FixedStringComparator<48>> tree("foo_pk", bpm, comparator);
    return 0;
}