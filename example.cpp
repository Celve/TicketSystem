#include <algorithm>
#include <cstdio>
#include <ctime>
#include <random>

#include "storage/index/b_plus_tree_index_nts.h"
#include "type/string.h"

using namespace thomas;

int main() {
  BPlusTreeIndexNTS<StringInt<68>, int, StringIntComparator<68>> *index_tree;
  StringIntComparator<68> comparator(1);
  StringIntComparator<68> standby_comparator(2);
  index_tree =
      new BPlusTreeIndexNTS<StringInt<68>, int, StringIntComparator<68>>(
          "index", comparator);
  int n;
  char opt[16];
  char index[68];
  int value;
  StringInt<68> key;
  scanf("%d", &n);
  while (n--) {
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