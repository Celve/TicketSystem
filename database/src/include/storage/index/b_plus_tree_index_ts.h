#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "buffer/buffer_pool_manager.h"
#include "common/config.h"
#include "storage/disk/disk_manager.h"
#include "storage/index/b_plus_tree_ts.h"
#include "storage/page/header_page.h"
#include "thread/thread_pool.h"

namespace thomas {

INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeIndexTS {
 public:
  explicit BPlusTreeIndexTS(const std::string &index_name, const KeyComparator &key_comparator,
                            int buffer_pool_size = BUFFER_POOL_SIZE);
  ~BPlusTreeIndexTS();

  void InsertEntry(const KeyType &key, const ValueType &value);

  void DeleteEntry(const KeyType &key);

  void ScanKey(const KeyType &key, std::vector<ValueType> *result, const KeyComparator &standby_comparator);

  void SearchKey(const KeyType &key, std::vector<ValueType> *result);

  void Debug();

  int TimeStamp();

 private:
  char index_name_[32];
  DiskManager *disk_manager_;
  BufferPoolManager *bpm_;
  HeaderPage *header_page_;
  int buffer_pool_size_;
  int size_;
  int timestamp_;

  KeyComparator key_comparator_;

  BPLUSTREETS_TYPE *tree_;
  // std::unordered_map<KeyType, bool> map_;
};

}  // namespace thomas
