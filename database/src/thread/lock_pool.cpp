#include "thread/lock_pool.h"

namespace thomas {

LOCK_ARGUMENT_TEMPLATE
void LockPool::AcquireRLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].RLock();
}

LOCK_ARGUMENT_TEMPLATE
void LockPool::ReleaseRLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].RUnlock();
}

LOCK_ARGUMENT_TEMPLATE
void LockPool::AcquireWLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].WLock();
}

LOCK_ARGUMENT_TEMPLATE
void LockPool::ReleaseWLatch(const KeyType &key) {
  auto hash_key = key.Hash();
  map_latch_[hash_key % MUTEX_NUMBER].WUnlock();
}

}  // namespace thomas