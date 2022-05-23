#include "storage/index/b_plus_tree_index.h"

#include <cstring>
#include <cassert>

namespace thomas {

#define BPLUSTREEINDEX_TYPE BPlusTreeIndex<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREEINDEX_TYPE::BPlusTreeIndex(const std::string &index_name) {
  assert(index_name.size() < 32);
  strcpy(index_name_, index_name.c_str());
  disk_manager_ = new DiskManager(index_name + ".db");
  bpm_ = new BufferPoolManager(50, disk_manager_);

  /* some restore */
  HeaderPage *header_page;
  try {
    header_page = static_cast<HeaderPage *>(bpm_->FetchPage(HEADER_PAGE_ID));
    page_id_t next_page_id;
    if (!header_page->SearchRecord("page_amount", &next_page_id)) {
      throw metadata_error();
    }
    disk_manager_->SetNextPageId(next_page_id);
  } catch (read_less_then_a_page error) {
    page_id_t header_page_id;
    header_page = static_cast<HeaderPage *>(bpm_->NewPage(&header_page_id));
    header_page->InsertRecord("index", -1);
    header_page->InsertRecord("page_amount", 1);
  }

  key_comparator_ = new KeyComparator();
  tree_ = new BPLUSTREE_TYPE("index", bpm_, *key_comparator_);
}

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREEINDEX_TYPE::~BPlusTreeIndex() {
  bpm_->UnpinPage(HEADER_PAGE_ID, true);
  bpm_->FlushAllPages();
  delete disk_manager_;
  delete bpm_;
  delete key_comparator_;
  delete tree_;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEX_TYPE::InsertEntry(const KeyType &key, const ValueType &value) {
  tree_->Insert(key, value);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEX_TYPE::DeleteEntry(const KeyType &key) {
  tree_->Remove(key);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREEINDEX_TYPE::ScanKey(const KeyType &key, vector<ValueType> *result, bool (*comp)(const KeyType &lhs, const KeyType &rhs)) {
  tree_->GetValue(key, result, comp);
}

template class BPlusTreeIndex<FixedString<48>, size_t, FixedStringComparator<48>>;
template class BPlusTreeIndex<MixedStringInt<64>, int, MixedStringIntForMixedComparator<64>>;

}
