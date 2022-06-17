#include "storage/index/b_plus_tree_index_ts.h"

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

#define BPLUSTREEINDEXTS_TYPE BPlusTreeIndexTS<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREEINDEXTS_TYPE::BPlusTreeIndexTS(const std::string &index_name, const KeyComparator &key_comparator,
                                        ThreadPool *pool, int buffer_pool_size)
    : key_comparator_(key_comparator), pool_(pool), buffer_pool_size_(buffer_pool_size) {
  assert(index_name.size() < 32);
  strcpy(index_name_, index_name.c_str());
  disk_manager_ = new DiskManager(index_name + ".db");
  bpm_ = new BufferPoolManager(buffer_pool_size, disk_manager_, THREAD_SAFE_TYPE::THREAD_SAFE);

  /* some restore */
  try {
    header_page_ = static_cast<HeaderPage *>(bpm_->FetchPage(HEADER_PAGE_ID));
    page_id_t next_page_id;
    if (!header_page_->SearchRecord("page_amount", &next_page_id)) {
      /* the metadata cannot be broken */
      throw metadata_error();
    }
    disk_manager_->SetNextPageId(next_page_id);
    if (!header_page_->SearchRecord("size", &size_)) {
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
    header_page_->InsertRecord("size", 0);
  }
  tree_ = new BPLUSTREETS_TYPE("index", bpm_, key_comparator_);
}

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREEINDEXTS_TYPE::~BPlusTreeIndexTS() {
  header_page_->UpdateRecord("page_amount", disk_manager_->GetNextPageId());
  header_page_->UpdateRecord("size", size_);
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

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::Debug() { tree_->Print(bpm_); }

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::AcquireRLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].RLock();
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::ReleaseRLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].RUnlock();
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::AcquireWLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].WLock();
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::ReleaseWLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].WUnlock();
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::InsertEntry(const KeyType &key, const ValueType &value) {
  // AcquireWLatch(key);
  Transaction *transaction = new Transaction(pool_->GetLock());
  size_ += tree_->OptimisticInsert(key, value, transaction);
  // size_ += tree_->Insert(key, value, transaction));
  // ReleaseWLatch(key);
  delete transaction;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::DeleteEntry(const KeyType &key) {
  // AcquireWLatch(key);
  Transaction *transaction = new Transaction(pool_->GetLock());
  size_ -= tree_->OptimisticRemove(key, transaction);
  // size_ -= tree_->Remove(key, transaction);
  // ReleaseWLatch(key);
  delete transaction;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::ScanKey(const KeyType &key, std::vector<ValueType> *result,
                                    const KeyComparator &standby_comparator) {
  // AcquireRLatch(key);
  Transaction *transaction = new Transaction(pool_->GetLock());
  tree_->GetValue(key, result, standby_comparator, transaction);
  // ReleaseRLatch(key);
  delete transaction;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::SearchKey(const KeyType &key, std::vector<ValueType> *result) {
  // AcquireRLatch(key);
  Transaction *transaction = new Transaction(pool_->GetLock());
  tree_->GetValue(key, result, transaction);
  // ReleaseRLatch(key);
  delete transaction;
}

/**
 * @brief
 * make sure that the operation should be done before using the thread pool
 * @param pool
 * @return INDEX_TEMPLATE_ARGUMENTS
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::ResetPool(ThreadPool *pool) { pool_ = pool; }

DECLARE(BPlusTreeIndexTS)

}  // namespace thomas
