#pragma once

#include <mutex>  // NOLINT

#include "buffer/replacer.h"
#include "common/config.h"
#include "container/linked_hashmap.hpp"
#include "thread/thread_safe.h"

namespace thomas {

/**
 * LRUReplacer implements the lru replacement policy, which approximates the Least Recently Used policy.
 */
class LRUReplacer : public Replacer {
 public:
  /**
   * Create a new LRUReplacer.
   * @param num_pages the maximum number of pages the LRUReplacer will be required to store
   */
  explicit LRUReplacer(size_t num_pages);

  /**
   * Destroys the LRUReplacer.
   */
  ~LRUReplacer() override;

  bool Victim(frame_id_t *frame_id) override;

  void Pin(frame_id_t frame_id) override;

  void Unpin(frame_id_t frame_id) override;

  size_t Size() override;

  void Clear() override;

 private:
  linked_hashmap<frame_id_t, frame_id_t> queue_;
  std::mutex latch_;
  size_t num_pages_;
};

}  // namespace thomas
