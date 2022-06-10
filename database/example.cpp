/**
 * @file example.cpp
 * @author Linyu Wu
 * @brief this is the cpp used to guides users how to use the b+ tree
 * @version 1.0.0
 * @date 2022-06-09
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <random>

#include "storage/index/b_plus_tree_index_nts.h"
#include "type/string.h"
#include "type/string_any.h"

using namespace thomas;  // NOLINT

int main() {
  /**
   * @brief
   * both thread-safe version and non-thread-safe version is provided
   * Here use the non-thread-safe version
   */
  BPlusTreeIndexNTS<StringAny<68, int>, int, StringAnyComparator<68, int>> *index_tree;
  StringAnyComparator<68, int> comparator(3);
  StringAnyComparator<68, int> standby_comparator(1);
  index_tree = new BPlusTreeIndexNTS<StringAny<int, 68>, int, StringAnyComparator<int, 68>>("index", comparator);
  int n;
  char opt[16];
  char index[68];
  int value;
  StringAny<int, 68> key;
  scanf("%d", &n);
  while ((n--) != 0) {
    scanf("%s", opt);
    if (strcmp(opt, "insert") == 0) {
      scanf("%s %d", index, &value);
      key.SetValue(index, value);
      index_tree->InsertEntry(key, value);
    } else if (strcmp(opt, "delete") == 0) {
      scanf("%s %d", index, &value);
      key.SetValue(index, value);
      index_tree->DeleteEntry(key);
    } else if (strcmp(opt, "find") == 0) {
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
    } else {
      index_tree->Clear();
    }

    /* a debug function is provided */
    // index_tree->Debug();
  }
  delete index_tree;
  return 0;
}