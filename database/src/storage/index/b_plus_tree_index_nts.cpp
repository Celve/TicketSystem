#include "storage/index/b_plus_tree_index_nts.h"

#include <cassert>
#include <chrono>
#include <cstring>
#include <mutex>
#include <stdexcept>

#include "common/config.h"
#include "common/exceptions.hpp"
#include "common/macros.h"
#include "concurrency/transaction.h"
#include "storage/page/b_plus_tree_page.h"

namespace thomas {

#define BPLUSTREEINDEXNTS_TYPE BPlusTreeIndexNTS<KeyType, ValueType, KeyComparator>

/**
 * @brief
 * a non-thread-safe b+ tree constructor
 * @param index_name the name of b+ tree
 * @param key_comparator the comparator used to compare keys
 * @param buffer_pool_size the size of the buffer pool
 * @return INDEX_TEMPLATE_ARGUMENTS
 */
INDEX_TEMPLATE_ARGUMENTS
BPLUSTREEINDEXNTS_TYPE::BPlusTreeIndexNTS(const std::string &index_name, const KeyComparator &key_comparator,
                                          int buffer_pool_size)
    : key_comparator_(key_comparator), buffer_pool_size_(buffer_pool_size) {
  assert(index_name.size() < 32);
  strcpy(index_name_, index_name.c_str());
  disk_manager_ = new DiskManager(index_name + ".db");
  bpm_ = new BufferPoolManager(buffer_pool_size, disk_manager_, THREAD_SAFE_TYPE::NON_THREAD_SAFE);

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
  tree_ = new BPLUSTREENTS_TYPE("index", bpm_, key_comparator_);
}

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREEINDEXNTS_TYPE::~BPlusTreeIndexNTS() {
  header_page_->UpdateRecord("page_amount", disk_manager_->GetNextPageId());
  header_page_->UpdateRecord("size", size_);
  bpm_->UnpinPage(HEADER_PAGE_ID, true);
  bpm_->FlushAllPages();
  disk_manager_->ShutDown();
  delete disk_manager_;
  delete bpm_;
  delete tree_;
}

INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREEINDEXNTS_TYPE::IsEmpty() { return tree_->IsEmpty(); }

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXNTS_TYPE::Debug() { tree_->Print(bpm_); }

/**
 * @brief
 * insert a key value pair into the b+ tree, and the duplicate key is not allowed, repeated key is allowed, but
 * considered as modification
 * @param key the key to be inserted
 * @param value the value to be inserted
 * @return INDEX_TEMPLATE_ARGUMENTS
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXNTS_TYPE::InsertEntry(const KeyType &key, const ValueType &value) {
  size_ += tree_->Insert(key, value);
}

/**
 * @brief
 * delete a key value pair into the b+ tree, nothing happens if no such entry
 * @param key
 * @return INDEX_TEMPLATE_ARGUMENTS
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXNTS_TYPE::DeleteEntry(const KeyType &key) { size_ -= tree_->Remove(key); }

/**
 * @brief
 * scan to find out all values with the same key
 * due to the property of b+ tree, duplicate key is not allowed
 * so a new comparator
 * @param key
 * @param result
 * @param standby_comparator
 * @return INDEX_TEMPLATE_ARGUMENTS
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXNTS_TYPE::ScanKey(const KeyType &key, vector<ValueType> *result,
                                     const KeyComparator &standby_comparator) {
  tree_->GetValue(key, result, standby_comparator);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXNTS_TYPE::SearchKey(const KeyType &key, vector<ValueType> *result) { tree_->GetValue(key, result); }

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEXNTS_TYPE::Clear() {
  delete tree_;
  bpm_->Initialize();
  disk_manager_->Clear();
  page_id_t header_page_id;
  header_page_ = static_cast<HeaderPage *>(bpm_->NewPage(&header_page_id));
  header_page_->InsertRecord("index", -1);
  header_page_->InsertRecord("page_amount", 1);
  header_page_->InsertRecord("size", 0);
  tree_ = new BPLUSTREENTS_TYPE("index", bpm_, key_comparator_);
}

INDEX_TEMPLATE_ARGUMENTS
int BPLUSTREEINDEXNTS_TYPE::Size() { return size_; }

DECLARE(BPlusTreeIndexNTS)

}  // namespace thomas
