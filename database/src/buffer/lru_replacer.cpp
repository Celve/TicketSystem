#include "buffer/lru_replacer.h"

namespace thomas {

LRUReplacer::LRUReplacer(size_t num_pages) : num_pages_(num_pages) {}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  if (queue_.empty()) {
    return false;
  }

  /* there must be a victim */
  *frame_id = queue_.begin()->second;
  queue_.erase(queue_.begin());
  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  /* it might can't be found */
  auto iter = queue_.find(frame_id);
  if (iter != queue_.end()) {
    queue_.erase(iter);
  }
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  /* avoid being too large */
  if (queue_.size() >= num_pages_) {
    return;
  }

  /* maybe it's existed, maybe it's not */
  auto iter = queue_.find(frame_id);
  if (iter != queue_.end()) {
    queue_.erase(iter);
  }
  queue_[frame_id] = frame_id;
}

size_t LRUReplacer::Size() { return queue_.size(); }

void LRUReplacer::Clear() { queue_.clear(); }

}  // namespace thomas
