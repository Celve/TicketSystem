#pragma once

#include "common/config.h"
#include "common/rwlatch.h"

namespace thomas {

#define LOCK_ARGUMENT_TEMPLATE template <typename KeyType>

class LockPool {
 public:
  LOCK_ARGUMENT_TEMPLATE
  void AcquireRLatch(const KeyType &key) {
    auto hash_key = key.Hash();
    map_latch_[hash_key % MUTEX_NUMBER].RLock();
  }

  LOCK_ARGUMENT_TEMPLATE
  void ReleaseRLatch(const KeyType &key) {
    auto hash_key = key.Hash();
    map_latch_[hash_key % MUTEX_NUMBER].RUnlock();
  }

  LOCK_ARGUMENT_TEMPLATE
  void AcquireWLatch(const KeyType &key) {
    auto hash_key = key.Hash();
    map_latch_[hash_key % MUTEX_NUMBER].WLock();
  }

  LOCK_ARGUMENT_TEMPLATE
  void ReleaseWLatch(const KeyType &key) {
    auto hash_key = key.Hash();
    map_latch_[hash_key % MUTEX_NUMBER].WUnlock();
  }

 private:
  ReaderWriterLatch map_latch_[MUTEX_NUMBER];
};

}  // namespace thomas