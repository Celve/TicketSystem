#pragma once

#include <mutex>  // NOLINT
#include <ratio>

#include "buffer/replacer.h"
#include "common/config.h"
#include "container/linked_hashmap.hpp"
#include "thread/thread_safe.h"

namespace thomas {

/**
 * LRUReplacer implements the lru replacement policy, which approximates the Least Recently Used policy.
 */
class InnodbReplacer : public Replacer {
  const double FACTOR = 1.0 / 32;

 public:
  /**
   * Create a new LRUReplacer.
   * @param num_pages the maximum number of pages the LRUReplacer will be required to store
   */
  explicit InnodbReplacer(size_t num_pages);

  /**
   * Destroys the LRUReplacer.
   */
  ~InnodbReplacer() override;

  bool Victim(frame_id_t *frame_id) override;

  void Pin(frame_id_t frame_id) override;

  void Unpin(frame_id_t frame_id) override;

  size_t Size() override;

 private:
  void Adjust();

  linked_hashmap<frame_id_t, frame_id_t> left_half, right_half;
  std::mutex latch_;
  size_t num_pages_;
};

}  // namespace thomas
