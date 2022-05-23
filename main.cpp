#include <algorithm>
#include <cstdio>
#include <random>
#include <ctime>

#include "storage/index/b_plus_tree_index.h"

using namespace thomas;

int main() {
  BPlusTreeIndex<MixedStringInt<68>, int, MixedStringIntForMixedComparator<68>> *index_tree;
  index_tree = new BPlusTreeIndex<MixedStringInt<68>, int, MixedStringIntForMixedComparator<68>>("index");
  int n;
  char opt[16];
  char index[68];
  int value;
  MixedStringInt<68> key;
  srand(time(nullptr));
  for (int i = 1; i <= 10000; ++i) {
    std::string test = "";
    for (int j = 1; j <= 64; ++j) {
      char alpha = rand() % 26 + 'a';
      test += alpha;
    }
    int temp = rand();
//    std::cout << test << " " << temp << std::endl;
    key.SetValue(test, temp);
    index_tree->InsertEntry(key, temp);
  }
  for (int i = 1; i <= 10000; ++i) {
    std::string test = "";
    for (int j = 1; j <= 64; ++j) {
      char alpha = rand() % 26 + 'a';
      test += alpha;
    }
    int temp = rand();
//    std::cout << test << " " << temp << std::endl;
    key.SetValue(test, temp);
    index_tree->DeleteEntry(key);
  }
  for (int i = 1; i <= 10000; ++i) {
    std::string test = "";
    for (int j = 1; j <= 64; ++j) {
      char alpha = rand() % 26 + 'a';
      test += alpha;
    }
//    std::cout << test << std::endl;
    key.SetValue(test, -2147483647);
    vector<int> result;
    index_tree->ScanKey(key, &result, MixedStringIntForStringComparator);
  }
  delete index_tree;
  return 0;
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
      index_tree->ScanKey(key, &result, MixedStringIntForStringComparator);
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