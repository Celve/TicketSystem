#include "storage/index/b_plus_tree_index_pool.h"

#include <cassert>
#include <chrono>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <thread>

#include "common/config.h"
#include "common/exceptions.hpp"
#include "common/macros.h"
#include "concurrency/transaction.h"
#include "storage/page/b_plus_tree_page.h"
#include "thread/thread_pool.h"

namespace thomas {

#define BPLUSTREEINDEXPOOL_TYPE BPlusTreeIndexPool<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREEINDEXPOOL_TYPE::BPlusTreeIndexPool(const std::string &index_name, const KeyComparator &key_comparator,
                                            int buffer_pool_size, int thread_number)
    : key_comparator_(key_comparator), buffer_pool_size_(buffer_pool_size), thread_number_(thread_number) {
  assert(index_name.size() < 32);
  strcpy(index_name_, index_name.c_str());
  disk_manager_ = new DiskManager(index_name + ".db");
  bpm_ = new BufferPoolManager(buffer_pool_size, disk_manager_, THREAD_SAFE_TYPE::THREAD_SAFE);
  pool_ = new ThreadPool(thread_number);

  /* some restore */
  try {
    header_page_ = static_cast<HeaderPage *>(bpm_->FetchPage(HEADER_PAGE_ID));
    page_id_t next_page_id;
    if (!header_page_->SearchRecord("page_amount", &next_page_id)) {
      /* the metadata cannot be broken */
      throw metadata_error();
    }
    disk_manager_->SetNextPageId(next_page_id);
    if (!header_page_->SearchRecord("time_stamp", &time_stamp_)) {
      throw metadata_error();
    }
  } catch (read_less_then_a_page &error) {
    /* complicated here, because the page is not fetched successfully */
    bpm_->UnpinPage(HEADER_PAGE_ID, false);
    bpm_->DeletePage(HEADER_PAGE_ID);
    page_id_t header_page_id;
    header_page_ = static_cast<HeaderPage *>(bpm_->NewPage(&header_page_id));
    header_page_->InsertRecord("index", -1);
    header_page_->InsertRecord("page_amount", 1);
    header_page_->InsertRecord("time_stamp", 0);
  }
  tree_ = new BPLUSTREETS_TYPE("index", bpm_, key_comparator_);
}

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREEINDEXPOOL_TYPE::~BPlusTreeIndexPool() {
  delete pool_;
  header_page_->UpdateRecord("page_amount", disk_manager_->GetNextPageId());
  header_page_->UpdateRecord("time_stamp", time_stamp_);
  page_id_t root_page_id;
  header_page_->SearchRecord("index", &root_page_id);
  if (root_page_id != -1) {
    bpm_->UnpinPage(root_page_id, false);
  }
  bpm_->UnpinPage(HEADER_PAGE_ID, true);
  bpm_->FlushAllPages();
  disk_manager_->ShutDown();
  delete disk_manager_;
  delete bpm_;
  delete tree_;
}

/**
 * @brief
 * this function is not thread safe, please do not use it
 * @return INDEX_TEMPLATE_ARGUMENTS
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXPOOL_TYPE::Debug() { tree_->Print(bpm_); }

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXPOOL_TYPE::AcquireRLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].RLock();
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXPOOL_TYPE::ReleaseRLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].RUnlock();
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXPOOL_TYPE::AcquireWLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].WLock();
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXPOOL_TYPE::ReleaseWLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].WUnlock();
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXPOOL_TYPE::InsertEntry(KeyType *key, ValueType *value) {
  ++time_stamp_;
  pool_->Join([&, key, value]() {
    AcquireWLatch(*key);
    Transaction *transaction = new Transaction(pool_->GetLock());
    tree_->OptimisticInsert(*key, *value, transaction);
    ReleaseWLatch(*key);
    delete transaction;
    delete key;
    delete value;
  });
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXPOOL_TYPE::DeleteEntry(KeyType *key) {
  pool_->Join([&, key]() {
    AcquireWLatch(*key);
    Transaction *transaction = new Transaction(pool_->GetLock());
    tree_->OptimisticRemove(*key, transaction);
    ReleaseWLatch(*key);
    delete key;
    delete transaction;
  });
}

INDEX_TEMPLATE_ARGUMENTS
std::future<std::vector<ValueType>> BPLUSTREEINDEXPOOL_TYPE::ScanKey(KeyType *key,
                                                                     const KeyComparator &standby_comparator) {
  return pool_->Join([&, key]() {
    std::vector<ValueType> result;
    AcquireRLatch(*key);
    Transaction *transaction = new Transaction(pool_->GetLock());
    tree_->GetValue(*key, &result, standby_comparator, transaction);
    ReleaseRLatch(*key);
    delete key;
    delete transaction;
    return result;
  });
}

INDEX_TEMPLATE_ARGUMENTS
std::future<std::vector<ValueType>> BPLUSTREEINDEXPOOL_TYPE::SearchKey(KeyType *key) {
  return pool_->Join([&, key]() {
    std::vector<ValueType> result;
    AcquireRLatch(*key);
    Transaction *transaction = new Transaction(pool_->GetLock());
    tree_->GetValue(*key, &result, transaction);
    ReleaseRLatch(*key);
    delete key;
    delete transaction;
    return result;
  });
}

/**
 * @brief
 * make sure that the operation should be done before using the thread pool
 * this function is not thread safe
 * @param pool
 * @return INDEX_TEMPLATE_ARGUMENTS
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXPOOL_TYPE::ResetPool(ThreadPool *pool) { pool_ = pool; }

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXPOOL_TYPE::Clear() {
  delete pool_;
  delete tree_;
  bpm_->Initialize();
  disk_manager_->Clear();
  page_id_t header_page_id;
  header_page_ = static_cast<HeaderPage *>(bpm_->NewPage(&header_page_id));
  header_page_->InsertRecord("index", -1);
  header_page_->InsertRecord("page_amount", 1);
  header_page_->InsertRecord("time_stamp", 0);
  pool_ = new ThreadPool(THREAD_NUMBER);
  tree_ = new BPLUSTREETS_TYPE("index", bpm_, key_comparator_);
}

INDEX_TEMPLATE_ARGUMENTS
int BPLUSTREEINDEXPOOL_TYPE::TimeStamp() { return time_stamp_; }

DECLARE(BPlusTreeIndexPool)

}  // namespace thomas
