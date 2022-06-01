#include "buffer/innodb_replacer.h"

#include "container/linked_hashmap.hpp"

namespace thomas {

InnodbReplacer::InnodbReplacer(size_t num_pages) : num_pages_(num_pages) {}

InnodbReplacer::~InnodbReplacer() = default;

bool InnodbReplacer::Victim(frame_id_t *frame_id) {
  if (left_half.empty() && right_half.empty()) {
    return false;
  }

  /* there must be a victim */
  *frame_id = left_half.begin()->second;
  left_half.erase(left_half.begin());
  Adjust();
  return true;
}

void InnodbReplacer::Pin(frame_id_t frame_id) {
  /* it might can't be found */
  auto left_half_iter = left_half.find(frame_id);
  if (left_half_iter != left_half.end()) {
    left_half.erase(left_half_iter);
    Adjust();
  } else {
    auto right_half_iter = right_half.find(frame_id);
    if (right_half_iter != right_half.end()) {
      right_half.erase(right_half_iter);
    }
  }
}

void InnodbReplacer::Unpin(frame_id_t frame_id) {
  /* avoid being too large */
  if (left_half.size() + right_half.size() >= num_pages_) {
    return;
  }

  /* maybe it's existed, maybe it's not */
  if (left_half.count(frame_id) == 0 && right_half.count(frame_id) == 0) {
    right_half.front_insert(linked_hashmap<frame_id_t, frame_id_t>::value_type(frame_id, frame_id));
    Adjust();
  }
}

void InnodbReplacer::Adjust() {
  while (left_half.size() * FACTOR < right_half.size()) {  // NOLINT
    frame_id_t transfer = right_half.begin()->first;
    left_half[transfer] = transfer;
    right_half.erase(right_half.begin());
  }
}

size_t InnodbReplacer::Size() { return left_half.size() + right_half.size(); }

}  // namespace thomas
