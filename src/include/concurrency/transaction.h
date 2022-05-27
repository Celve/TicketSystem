#pragma once

#include <mutex>

#include "container/vector.hpp"
#include "storage/page/page.h"

namespace thomas {

class Transaction {
 public:
  explicit Transaction(std::mutex *latch) : latch_(latch) {}

  void Unlock() { latch_->unlock(); }

  void AddIntoPageSet(Page *page) { page_set_.push_back(page); }
  void AddIntoDeletedPageSet(page_id_t page_id) { deleted_page_set_.push_back(page_id); }

  vector<Page *> *GetPageSet() { return &page_set_; }

  vector<page_id_t> *GetDeletedPageSet() { return &deleted_page_set_; }

 private:
  vector<Page *> page_set_;
  vector<page_id_t> deleted_page_set_;
  std::mutex *latch_;
};

}  // namespace thomas