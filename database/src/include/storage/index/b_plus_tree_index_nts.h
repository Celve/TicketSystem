#pragma once

#include <mutex>

#include "buffer/buffer_pool_manager.h"
#include "concurrency/transaction.h"
#include "container/vector.hpp"
#include "storage/disk/disk_manager.h"
#include "storage/index/b_plus_tree_nts.h"
#include "storage/page/header_page.h"

namespace thomas {

INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeIndexNTS {
 public:
  explicit BPlusTreeIndexNTS(const std::string &index_name, const KeyComparator &key_comparator,
                             int buffer_pool_size = 1000);
  ~BPlusTreeIndexNTS();

  bool IsEmpty();

  void InsertEntry(const KeyType &key, const ValueType &value);

  void DeleteEntry(const KeyType &key);

  void ScanKey(const KeyType &key, vector<ValueType> *result, const KeyComparator &standby_comparator);

  void SearchKey(const KeyType &key, vector<ValueType> *result);

  void Debug();

 private:
  char index_name_[32];
  DiskManager *disk_manager_;
  BufferPoolManager *bpm_;
  HeaderPage *header_page_;

  KeyComparator key_comparator_;

  BPLUSTREENTS_TYPE *tree_;
};

}  // namespace thomas
