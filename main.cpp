#include <algorithm>
#include <cstdio>
#include <random>
#include <ctime>

#include "storage/index/b_plus_tree_index.h"

using namespace thomas;

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
  while(n--) {
    scanf("%s", opt);
    if (!strcmp(opt, "insert")) {
      scanf("%s %d", index, &value);
      key.SetValue(index, value);
      index_tree->InsertEntry(key, value);
    } else if (!strcmp(opt, "delete")) {
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
  return 0;
}