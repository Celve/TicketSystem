#pragma once

#include <mutex>

#include "buffer/buffer_pool_manager.h"
#include "concurrency/transaction.h"
#include "container/vector.hpp"
#include "storage/disk/disk_manager.h"
#include "storage/index/b_plus_tree_nts.h"
#include "storage/index/b_plus_tree_ts.h"
#include "storage/page/header_page.h"
#include "thread/thread_pool.h"

namespace thomas {

INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeIndexTS {
 public:
  explicit BPlusTreeIndexTS(const std::string &index_name, const KeyComparator &key_comparator, ThreadPool *pool,
                            int buffer_pool_size = 10000);
  ~BPlusTreeIndexTS();

  void InsertEntry(const KeyType &key, const ValueType &value);

  void DeleteEntry(const KeyType &key);

  void ScanKey(const KeyType &key, vector<ValueType> *result, const KeyComparator &standby_comparator);

  void SearchKey(const KeyType &key, vector<ValueType> *result);

  void Debug();

  void ResetPool(ThreadPool *pool);

 private:
  char index_name_[32];
  DiskManager *disk_manager_;
  BufferPoolManager *bpm_;
  HeaderPage *header_page_;
  ThreadPool *pool_;

  KeyComparator key_comparator_;

  BPLUSTREETS_TYPE *tree_;
};

}  // namespace thomas
