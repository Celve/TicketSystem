#include <bits/types/clock_t.h>

#include <algorithm>
#include <cassert>
#include <chrono>
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

#define THREAD_NUMBER 10

using namespace thomas;  // NOLINT

void Test4() {  // NOLINT
  srand(time(nullptr));
  ThreadPool *pool = new ThreadPool(THREAD_NUMBER);
  BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>> *index_tree;
  FixedStringComparator<48> comparator;
  index_tree =
      new BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>>("index", comparator, pool, 50000);
  int NUMBER = 100;
  std::map<std::string, size_t> remap;
  std::vector<std::future<size_t>> results;

  auto start_time = std::chrono::system_clock::now();
  // first step: insert
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int i = 0; i < 10; ++i) {
      key_string += static_cast<char>(rand() % 26 + 'a');
    }
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    if (remap.count(key_string) == 0) {
      pool->Join([&, key, i]() {
        index_tree->InsertEntry(*key, i);
        std::cout << "b";
        delete key;
      });
      remap[key_string] = i;
    }
  }
  delete pool;
  auto half_time = std::chrono::system_clock::now();
  std::this_thread::sleep_for(std::chrono::milliseconds(70));
  std::cout << "all cost: sec_cost: " << 1.0 * (half_time - start_time).count() / 1e9  // NOLINT
            << std::endl;
  pool = new ThreadPool(THREAD_NUMBER);
  // std::cout << "result of try lock: " << pool->latch_.try_lock() << std::endl;
  // pool->latch_.unlock();

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
      std::cout << "c";
      return temp;
    }));
  }

  int iter_number = 0;
  for (auto &iter : remap) {
    auto answer = results[iter_number].get();  // the answer can be caught only once
    assert(answer == iter.second);
    ++iter_number;
  }

  delete pool;
  delete index_tree;
  auto end_time = std::chrono::system_clock::now();
  std::cout << "all cost: sec_cost: " << 1.0 * (end_time - start_time).count() / 1e9  // NOLINT
            << std::endl;
  return;

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