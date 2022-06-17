#pragma once

#include <mutex>
#include <vector>

#include "buffer/buffer_pool_manager.h"
#include "common/config.h"
#include "concurrency/transaction.h"
#include "storage/disk/disk_manager.h"
#include "storage/index/b_plus_tree_nts.h"
#include "storage/page/header_page.h"

namespace thomas {

INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeIndexNTS {
 public:
  explicit BPlusTreeIndexNTS(const std::string &index_name, const KeyComparator &key_comparator,
                             int buffer_pool_size = BUFFER_POOL_SIZE);
  ~BPlusTreeIndexNTS();

  bool IsEmpty();

  void InsertEntry(const KeyType &key, const ValueType &value);

  void DeleteEntry(const KeyType &key);

  void ScanKey(const KeyType &key, std::vector<ValueType> *result, const KeyComparator &standby_comparator);

  void SearchKey(const KeyType &key, std::vector<ValueType> *result);

  int Size();

  int TimeStamp();

  void Clear();

  void Debug();

 private:
  char index_name_[32];
  DiskManager *disk_manager_;
  BufferPoolManager *bpm_;
  HeaderPage *header_page_;
  int buffer_pool_size_;
  int size_;
  int time_stamp_;

  KeyComparator key_comparator_;

  BPLUSTREENTS_TYPE *tree_;
};

}  // namespace thomas
