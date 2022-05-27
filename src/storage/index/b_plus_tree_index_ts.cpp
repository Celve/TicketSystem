#include "storage/index/b_plus_tree_index_ts.h"

#include <cassert>
#include <chrono>
#include <cstring>
#include <mutex>
#include <stdexcept>

#include "common/config.h"
#include "concurrency/transaction.h"

namespace thomas {

#define BPLUSTREEINDEXTS_TYPE BPlusTreeIndexTS<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREEINDEXTS_TYPE::BPlusTreeIndexTS(const std::string &index_name, const KeyComparator &key_comparator,
                                        ThreadPool *pool, int buffer_pool_size)
    : key_comparator_(key_comparator), pool_(pool) {
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
  } catch (read_less_then_a_page &error) {
    /* complicated here, because the page is not fetched successfully */
    bpm_->UnpinPage(HEADER_PAGE_ID, false);
    bpm_->DeletePage(HEADER_PAGE_ID);
    page_id_t header_page_id;
    header_page_ = static_cast<HeaderPage *>(bpm_->NewPage(&header_page_id));
    header_page_->InsertRecord("index", -1);
    header_page_->InsertRecord("page_amount", 1);
  }
  tree_ = new BPLUSTREETS_TYPE("index", bpm_, key_comparator_);
}

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREEINDEXTS_TYPE::~BPlusTreeIndexTS() {
  header_page_->UpdateRecord("page_amount", disk_manager_->GetNextPageId());
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
void BPLUSTREEINDEXTS_TYPE::InsertEntry(const KeyType &key, const ValueType &value) {
  Transaction *transaction = new Transaction(pool_->GetLock());
  tree_->Insert(key, value, transaction);
  delete transaction;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::DeleteEntry(const KeyType &key) {
  Transaction *transaction = new Transaction(pool_->GetLock());
  tree_->Remove(key, transaction);
  delete transaction;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::ScanKey(const KeyType &key, vector<ValueType> *result,
                                    const KeyComparator &standby_comparator) {
  Transaction *transaction = new Transaction(pool_->GetLock());
  tree_->GetValue(key, result, standby_comparator, transaction);
  delete transaction;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXTS_TYPE::SearchKey(const KeyType &key, vector<ValueType> *result) {
  Transaction *transaction = new Transaction(pool_->GetLock());
  tree_->GetValue(key, result, transaction);
  delete transaction;
}

template class BPlusTreeIndexTS<FixedString<48>, size_t, FixedStringComparator<48>>;
template class BPlusTreeIndexTS<MixedStringInt<68>, int, MixedStringIntComparator<68>>;

}  // namespace thomas
