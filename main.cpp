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

#include "common/config.h"
#include "container/vector.hpp"
#include "storage/index/b_plus_tree_index_ts.h"
#include "thread/thread_pool.h"
#include "type/fixed_string.h"

#define THREAD_NUMBER 1
#define BUFFER_POOL_SIZE 10000
#define NUMBER 1000000

using namespace thomas;  // NOLINT

void Test4() {  // NOLINT
  srand(time(nullptr));
  ThreadPool *pool = new ThreadPool(THREAD_NUMBER);
  BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>> *index_tree;
  FixedStringComparator<48> comparator;
  index_tree = new BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>>("index", comparator, pool,
                                                                                        BUFFER_POOL_SIZE);
  std::vector<std::future<size_t>> results;

  auto one = std::chrono::system_clock::now();
  // first step: insert
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int i = 0; i < 15; ++i) {
      key_string += static_cast<char>(rand() % 26 + 'a');
    }
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    pool->Join([&, key, i]() {
      index_tree->InsertEntry(*key, i);
      delete key;
    });
  }
  delete pool;
  pool = new ThreadPool(THREAD_NUMBER);
  index_tree->ResetPool(pool);
  puts("first finish");
  // index_tree->ResetPool(pool);

  auto two = std::chrono::system_clock::now();
  // second step: find
  results.reserve(NUMBER);
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int i = 0; i < 15; ++i) {
      key_string += static_cast<char>(rand() % 26 + 'a');
    }
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

  delete pool;
  pool = new ThreadPool(THREAD_NUMBER);
  index_tree->ResetPool(pool);
  puts("second finish");
  auto three = std::chrono::system_clock::now();

  // third step: clean all
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int i = 0; i < 15; ++i) {
      key_string += static_cast<char>(rand() % 26 + 'a');
    }
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    pool->Join([&, key]() {
      index_tree->DeleteEntry(*key);
      delete key;
    });
  }
  delete pool;
  pool = new ThreadPool(THREAD_NUMBER);
  index_tree->ResetPool(pool);
  puts("third finish");
  auto four = std::chrono::system_clock::now();

  // fourth step: insert
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int i = 0; i < 15; ++i) {
      key_string += static_cast<char>(rand() % 26 + 'a');
    }
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    pool->Join([&, i, key]() {
      index_tree->InsertEntry(*key, i);
      delete key;
    });
  }
  delete pool;
  pool = new ThreadPool(THREAD_NUMBER);
  index_tree->ResetPool(pool);
  puts("fourth finish");
  auto five = std::chrono::system_clock::now();

  // fifth step: find
  results.clear();
  results.reserve(NUMBER);
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int i = 0; i < 15; ++i) {
      key_string += static_cast<char>(rand() % 26 + 'a');
    }
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
  delete pool;
  pool = new ThreadPool(THREAD_NUMBER);
  index_tree->ResetPool(pool);
  puts("fifth finish");
  auto six = std::chrono::system_clock::now();

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
  delete pool;
  pool = new ThreadPool(THREAD_NUMBER);
  index_tree->ResetPool(pool);
  puts("sixth finish");
  auto seven = std::chrono::system_clock::now();

  // seventh step: find
  results.clear();
  results.reserve(NUMBER);
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int i = 0; i < 15; ++i) {
      key_string += char(rand() % 26 + 'a');
    }
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    results.emplace_back(pool->Join([&, key]() {
      vector<size_t> res;
      index_tree->SearchKey(*key, &res);
      size_t temp = res[0];
      delete key;
      return temp;
    }));
  }
  puts("seventh finish");

  delete pool;
  delete index_tree;
  auto eight = std::chrono::system_clock::now();
  std::cout << NUMBER << " " << PAGE_SIZE << " " << BUFFER_POOL_SIZE << " " << THREAD_NUMBER << std::endl;
  std::cout << "insert cost: sec_cost: " << 1.0 * (two - one).count() / 1e9  // NOLINT
            << std::endl;
  std::cout << "find cost: sec_cost: " << 1.0 * (three - one).count() / 1e9  // NOLINT
            << std::endl;
  std::cout << "delete cost: sec_cost: " << 1.0 * (four - one).count() / 1e9  // NOLINT
            << std::endl;
  std::cout << "insert cost: sec_cost: " << 1.0 * (five - one).count() / 1e9  // NOLINT
            << std::endl;
  std::cout << "find cost: sec_cost: " << 1.0 * (six - one).count() / 1e9  // NOLINT
            << std::endl;
  std::cout << "delete random cost: sec_cost: " << 1.0 * (seven - one).count() / 1e9  // NOLINT
            << std::endl;
  std::cout << "find cost: sec_cost: " << 1.0 * (eight - one).count() / 1e9  // NOLINT
            << std::endl;
}

void Test5() {
  // srand(time(nullptr));
  ThreadPool *pool = new ThreadPool(THREAD_NUMBER);
  BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>> *index_tree;
  FixedStringComparator<48> comparator;
  index_tree = new BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>>("index", comparator, pool,
                                                                                        BUFFER_POOL_SIZE);
  std::vector<std::future<size_t>> results;
  auto begin = std::chrono::system_clock::now();
  for (int i = 0; i < NUMBER; ++i) {
    int type = rand() % 14;
    std::string key_string;
    FixedString<48> *key;
    if (type <= 7) {
      for (int j = 0; j < 20; ++j) {
        key_string += static_cast<char>(rand() % 26 + 'a');
      }
      key = new FixedString<48>;
      key->SetValue(key_string);
      pool->Join([&, key, i]() {
        index_tree->InsertEntry(*key, i);
        delete key;
      });
    } else if (type <= 11) {
      for (int j = 0; j < 15; ++j) {
        key_string += static_cast<char>(rand() % 26 + 'a');
      }
      key = new FixedString<48>;
      key->SetValue(key_string);
      results.emplace_back(pool->Join([&, key]() {
        vector<size_t> res;
        index_tree->SearchKey(*key, &res);
        size_t temp = res.empty() ? -1 : res[0];
        delete key;
        return temp;
      }));
    } else if (type <= 13) {
      for (int j = 0; j < 15; ++j) {
        key_string += static_cast<char>(rand() % 26 + 'a');
      }
      key = new FixedString<48>;
      key->SetValue(key_string);
      pool->Join([&, key]() {
        index_tree->DeleteEntry(*key);
        delete key;
      });
    }
  }
  delete pool;
  delete index_tree;
  auto end = std::chrono::system_clock::now();
  std::cout << NUMBER << " " << PAGE_SIZE << " " << BUFFER_POOL_SIZE << " " << THREAD_NUMBER << std::endl;
  std::cout << "final cost:" << 1.0 * (end - begin).count() / 1e9  // NOLINT
            << std::endl;
}

void Test6() {
  srand(time(nullptr));
  ThreadPool *pool = new ThreadPool(THREAD_NUMBER);
  BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>> *index_tree;
  FixedStringComparator<48> comparator;
  index_tree = new BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>>("index", comparator, pool,
                                                                                        BUFFER_POOL_SIZE);
  std::vector<std::future<size_t>> results;
  auto begin = std::chrono::system_clock::now();
  for (int i = 0; i < NUMBER; ++i) {
    std::string key_string;
    for (int j = 0; j < 15; ++j) {
      key_string += static_cast<char>(rand() % 26 + 'a');
    }
    FixedString<48> *key = new FixedString<48>;
    key->SetValue(key_string);
    pool->Join([&, key, i]() {
      index_tree->InsertEntry(*key, i);
      delete key;
    });
  }
  delete pool;
  pool = new ThreadPool(THREAD_NUMBER);
  index_tree->ResetPool(pool);

  auto middle = std::chrono::system_clock::now();
  for (int i = 0; i < NUMBER; ++i) {
    int type = rand() % 12;
    std::string key_string;
    for (int j = 0; j < 15; ++j) {
      key_string += static_cast<char>(rand() % 26 + 'a');
    }
    FixedString<48> *key;
    // if (type <= -1) {
    // key = new FixedString<48>;
    // key->SetValue(key_string);
    // pool->Join([&, key, i]() {
    // index_tree->InsertEntry(*key, i);
    // delete key;
    // });
    // } else if (type <= 11) {
    // for (int j = 0; j < 15; ++j) {
    // key_string += static_cast<char>(rand() % 26 + 'a');
    // }
    key = new FixedString<48>;
    key->SetValue(key_string);
    pool->Join([&, key]() {
      vector<size_t> res;
      index_tree->SearchKey(*key, &res);
      size_t temp = res.empty() ? -1 : res[0];
      delete key;
      return temp;
    });
    // } else if (type <= 15) {
    // for (int j = 0; j < 15; ++j) {
    // key_string += static_cast<char>(rand() % 26 + 'a');
    // }
    // key = new FixedString<48>;
    // key->SetValue(key_string);
    // pool->Join([&, key]() {
    // index_tree->DeleteEntry(*key);
    // delete key;
    // });
    // }
  }
  delete pool;
  delete index_tree;
  auto end = std::chrono::system_clock::now();
  std::cout << NUMBER << " " << PAGE_SIZE << " " << BUFFER_POOL_SIZE << " " << THREAD_NUMBER << std::endl;
  std::cout << "first cost:" << 1.0 * (middle - begin).count() / 1e9  // NOLINT
            << std::endl;
  std::cout << "second cost:" << 1.0 * (end - middle).count() / 1e9  // NOLINT
            << std::endl;
}

int main() { Test6(); }