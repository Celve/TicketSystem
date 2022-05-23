#pragma once

#include "storage/disk/disk_manager.h"
#include "buffer/buffer_pool_manager.h"
#include "storage/index/b_plus_tree.h"
#include "storage/page/header_page.h"
#include "container/vector.hpp"

namespace thomas {

INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeIndex {
 public:
  explicit BPlusTreeIndex(const std::string &index_name, const KeyComparator &key_comparator, int buffer_pool_size = 1000);
  ~BPlusTreeIndex();

  void InsertEntry(const KeyType &key, const ValueType &value);

  void DeleteEntry(const KeyType &key);

  void ScanKey(const KeyType &key, vector<ValueType> *result, const KeyComparator &standby_comparator);

  void Debug();

 private:
  char index_name_[32];
  DiskManager *disk_manager_;
  BufferPoolManager *bpm_;
  HeaderPage *header_page_;

  KeyComparator key_comparator_;

  BPLUSTREE_TYPE *tree_;
};

}
