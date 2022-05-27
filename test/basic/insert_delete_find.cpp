#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <future>
#include <map>
#include <random>
#include <thread>
#include <vector>

#include "container/vector.hpp"
#include "storage/index/b_plus_tree_index.h"
#include "thread/thread_pool.h"
#include "type/fixed_string.h"
#include "type/mixed_string_int.h"

int main() {
  BPlusTreeIndex<MixedStringInt<68>, int, MixedStringIntComparator<68>> *index_tree;
  MixedStringIntComparator<68> comparator(1);
  MixedStringIntComparator<68> standby_comparator(2);
  index_tree = new BPlusTreeIndex<MixedStringInt<68>, int, MixedStringIntComparator<68>>("index", comparator);
  int n;
  char opt[16];
  char index[68];
  int value;
  MixedStringInt<68> key;
  scanf("%d", &n);
  while (n-- != 0) {
    scanf("%s", opt);
    if (strcmp(opt, "insert") == 0) {
      scanf("%s %d", index, &value);
      key.SetValue(index, value);
      index_tree->InsertEntry(key, value);
    } else if (strcmp(opt, "delete") == 0) {
      scanf("%s %d", index, &value);
      key.SetValue(index, value);
      index_tree->DeleteEntry(key);
    } else {
      scanf("%s", index);
      key.SetValue(index, -2147483648);
      vector<int> result;
      index_tree->ScanKey(key, &result, standby_comparator);
      if (result.empty()) {
        puts("null");
      } else {
        for (auto &item : result) {
          printf("%d ", item);
        }
        puts("");
      }
    }
    // index_tree->Debug();
  }
  delete index_tree;
}