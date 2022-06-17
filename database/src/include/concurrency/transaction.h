#pragma once

#include <mutex>
#include <vector>

#include "storage/page/page.h"

namespace thomas {

class Transaction {
 public:
  explicit Transaction() = default;

  void AddIntoPageSet(Page *page) { page_set_.push_back(page); }
  void AddIntoDeletedPageSet(page_id_t page_id) { deleted_page_set_.push_back(page_id); }

  std::vector<Page *> *GetPageSet() { return &page_set_; }

  std::vector<page_id_t> *GetDeletedPageSet() { return &deleted_page_set_; }

 private:
  std::vector<Page *> page_set_;
  std::vector<page_id_t> deleted_page_set_;
  std::unique_lock<std::mutex> *latch_;
};

}  // namespace thomas