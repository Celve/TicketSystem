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
#include "storage/index/b_plus_tree_index_ts.h"
#include "thread/thread_pool.h"
#include "type/fixed_string.h"

using namespace thomas;  // NOLINT

void Test4() {  // NOLINT
  srand(time(nullptr));
  ThreadPool *pool = new ThreadPool(8);
  clock_t start = clock();
  BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>> *index_tree;
  FixedStringComparator<48> comparator;
  index_tree =
      new BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>>("index", comparator, pool, 10000);
  int NUMBER = 100000;
  std::map<std::string, size_t> remap;
  std::vector<std::future<size_t>> results;

  // first step: insert
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int i = 0; i < 10; ++i) {
      key_string += char(rand() % 26 + 'a');
    }
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    if (remap.count(key_string) == 0) {
      pool->Join([&, key, i]() {
        index_tree->InsertEntry(*key, i);
        delete key;
      });
      // remap[key_string] = i;
    }
  }
  clock_t half = clock();
  std::cout << double(half - start) / CLOCKS_PER_SEC << std::endl;
  puts("first finish");
  delete pool;
  std::cout << double(clock() - half) / CLOCKS_PER_SEC << std::endl;
  delete index_tree;
  return;

  // second step: find
  results.reserve(NUMBER);
  for (auto &iter : remap) {
    std::string key_string = iter.first;
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    results.emplace_back(pool->Join([&, key]() {
      vector<size_t> res;
      index_tree->SearchKey(*key, &res);
      size_t temp = res.empty() ? -1 : res[0];
      delete key;
      return temp;
    }));
  }
  int i = 0;
  for (auto &iter : remap) {
    auto answer = results[i].get();  // the answer can be caught only once
    assert(answer == iter.second);
    ++i;
  }

  // third step: clean all
  for (auto &iter : remap) {
    std::string key_string = iter.first;
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    pool->Join([&, key]() {
      index_tree->DeleteEntry(*key);
      delete key;
    });
  }
  remap.clear();
  puts("third finish");

  // fourth step: insert
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int i = 0; i < 10; ++i) {
      key_string += char(rand() % 26 + 'a');
    }
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    if (remap.count(key_string) == 0) {
      pool->Join([&, i, key]() {
        index_tree->InsertEntry(*key, i);
        delete key;
      });
      remap[key_string] = i;
    }
  }
  puts("fourth finish");

  // fifth step: find
  results.clear();
  results.reserve(NUMBER);
  for (auto &iter : remap) {
    std::string key_string = iter.first;
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    results.emplace_back(pool->Join([&, key]() {
      vector<size_t> res;
      index_tree->SearchKey(*key, &res);
      size_t temp = size_t(res[0]);
      delete key;
      return temp;
    }));
  }
  puts("fifth finish");

  // sixth step: random delete
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int i = 0; i < 15; ++i) {
      key_string += char(rand() % 26 + 'a');
    }
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    pool->Join([&, key]() {
      index_tree->DeleteEntry(*key);
      delete key;
    });
  }
  puts("sixth finish");

  // seventh step: find
  results.clear();
  results.reserve(NUMBER);
  for (auto &iter : remap) {
    std::string key_string = iter.first;
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    results.emplace_back(pool->Join([&, key]() {
      vector<size_t> res;
      index_tree->SearchKey(*key, &res);
      size_t temp = size_t(res[0]);
      delete key;
      return temp;
    }));
  }
  puts("seventh finish");

  delete pool;
  delete index_tree;
}

int main() { Test4(); }