#pragma once

#include "common/config.h"
#include "common/rwlatch.h"

namespace thomas {

#define LOCK_ARGUMENT_TEMPLATE template <typename KeyType>

class LockPool {
 public:
  LOCK_ARGUMENT_TEMPLATE
  void AcquireRLatch(const KeyType &key);

  LOCK_ARGUMENT_TEMPLATE
  void ReleaseRLatch(const KeyType &key);

  LOCK_ARGUMENT_TEMPLATE
  void AcquireWLatch(const KeyType &key);

  LOCK_ARGUMENT_TEMPLATE
  void ReleaseWLatch(const KeyType &key);

 private:
  ReaderWriterLatch map_latch_[MUTEX_NUMBER];
};

}  // namespace thomas