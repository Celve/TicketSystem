#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "buffer/buffer_pool_manager.h"
#include "common/config.h"
#include "common/rwlatch.h"
#include "storage/disk/disk_manager.h"
#include "storage/index/b_plus_tree_ts.h"
#include "storage/page/header_page.h"
#include "thread/thread_pool.h"

namespace thomas {

INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeIndexPool {
 public:
  explicit BPlusTreeIndexPool(const std::string &index_name, const KeyComparator &key_comparator,
                              int buffer_pool_size = BUFFER_POOL_SIZE, int thread_number = THREAD_NUMBER);
  ~BPlusTreeIndexPool();

  void InsertEntry(KeyType *key, ValueType *value);

  void DeleteEntry(KeyType *key);

  std::future<std::vector<ValueType>> ScanKey(KeyType *key, const KeyComparator &standby_comparator);

  std::future<std::vector<ValueType>> SearchKey(KeyType *key);

  bool IsEmpty();

  void Debug();

  int TimeStamp();

  void Clear();

  void ResetPool(ThreadPool *pool);

 private:
  void AcquireRLatch(const KeyType &key);
  void ReleaseRLatch(const KeyType &key);
  void AcquireWLatch(const KeyType &key);
  void ReleaseWLatch(const KeyType &key);

  char index_name_[32];
  DiskManager *disk_manager_;
  BufferPoolManager *bpm_;
  HeaderPage *header_page_;
  ThreadPool *pool_;
  int buffer_pool_size_;
  int thread_number_;
  int time_stamp_;

  KeyComparator key_comparator_;

  BPLUSTREETS_TYPE *tree_;

  ReaderWriterLatch map_latch_[MUTEX_NUMBER];
  // std::unordered_map<KeyType, bool> map_;
};

}  // namespace thomas
