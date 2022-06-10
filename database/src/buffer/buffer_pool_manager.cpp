#include "buffer/buffer_pool_manager.h"

#include <list>
#include <mutex>
#include <unordered_map>

#include "buffer/innodb_replacer.h"
#include "buffer/lru_replacer.h"

namespace thomas {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, THREAD_SAFE_TYPE ts_type)
    : pool_size_(pool_size), disk_manager_(disk_manager), ts_type_(ts_type) {
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.push_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  delete[] pages_;
  delete replacer_;
}

frame_id_t BufferPoolManager::FindFrame() {
  frame_id_t frame_id;

  /* none is available */
  if (free_list_.empty() && !replacer_->Victim(&frame_id)) {
    return -1;
  }

  if (!free_list_.empty()) {
    /* find it from free list */
    frame_id = free_list_.back();
    free_list_.pop_back();
  } else {
    /* find it from queue */
    Page *page = &pages_[frame_id];

    /* maybe it's dirty */
    if (page->IsDirty()) {
      disk_manager_->WritePage(page->GetPageId(), page->GetData());
    }

    /* delete the mapping */
    page_table_.erase(page_table_.find(page->GetPageId()));
  }

  /* initialization */
  Page *page = &pages_[frame_id];
  page->page_id_ = INVALID_PAGE_ID;
  page->is_dirty_ = false;
  page->pin_count_ = 0;
  page->ResetMemory();

  return frame_id;
}

Page *BufferPoolManager::FetchPage(page_id_t page_id) {
  std::unique_lock<std::mutex> lock =
      IsThreadSafe() ? std::unique_lock<std::mutex>(latch_) : std::unique_lock<std::mutex>();
  //  std::scoped_lock lock{latch_};

  // 1.     Search the page table for the requested page (P).
  auto it = page_table_.find(page_id);
  if (it != page_table_.end()) {
    // 1.1    If P exists, pin it and return it immediately.
    frame_id_t frame_id = it->second;

    /* pin it */
    Page *page = &pages_[frame_id];
    ++page->pin_count_;
    replacer_->Pin(frame_id);

    return page;
  }
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  frame_id_t frame_id = FindFrame();

  /* none is available */
  if (frame_id == -1) {
    return nullptr;
  }

  /* update in page table */
  page_table_[page_id] = frame_id;

  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  Page *page = &pages_[frame_id];
  page->page_id_ = page_id;
  page->pin_count_ = 1;
  replacer_->Pin(frame_id);
  disk_manager_->ReadPage(page_id, page->GetData());
  return page;
}

bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
  std::unique_lock<std::mutex> lock =
      IsThreadSafe() ? std::unique_lock<std::mutex>(latch_) : std::unique_lock<std::mutex>();
  //  std::scoped_lock lock{latch_};

  /* whether it's in the buffer pool */
  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    return false;
  }

  frame_id_t frame_id = it->second;
  Page *page = &pages_[frame_id];

  /* it's not pinned */
  if (page->pin_count_ == 0) {
    return false;
  }

  /* modify the metadata */
  page->is_dirty_ |= is_dirty;
  --page->pin_count_;

  /* maybe it becomes the unpinned page */
  if (page->pin_count_ == 0) {
    replacer_->Unpin(frame_id);
  }

  return true;
}

bool BufferPoolManager::FlushPage(page_id_t page_id) {
  std::unique_lock<std::mutex> lock =
      IsThreadSafe() ? std::unique_lock<std::mutex>(latch_) : std::unique_lock<std::mutex>();
  //  std::scoped_lock lock{latch_};

  /* invalid operation */
  if (page_id == INVALID_PAGE_ID) {
    return false;
  }

  /* no exist in page table, cannot be flushed */
  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    return false;
  }

  // Make sure you call DiskManager::WritePage!
  frame_id_t frame_id = it->second;
  Page *page = &pages_[frame_id];

  /* flush whether it's dirty or not */
  disk_manager_->WritePage(page_id, page->GetData());
  page->is_dirty_ = false;
  return true;
}

Page *BufferPoolManager::NewPage(page_id_t *page_id) {
  std::unique_lock<std::mutex> lock =
      IsThreadSafe() ? std::unique_lock<std::mutex>(latch_) : std::unique_lock<std::mutex>();
  //  std::scoped_lock lock{latch_};

  // 0.   Make sure you call DiskManager::AllocatePage!
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  frame_id_t frame_id = FindFrame();

  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  if (frame_id == -1) {
    return nullptr;
  }

  // 3.   Update P's metadata, zero out memory and add P to the page table.
  Page *page = &pages_[frame_id];
  *page_id = disk_manager_->AllocatePage();

  /* flush it */
  disk_manager_->WritePage(*page_id, page->GetData());

  // 4.   Set the page ID output parameter. Return a pointer to P.
  page->page_id_ = *page_id;
  page->pin_count_ = 1;
  replacer_->Pin(frame_id);

  /* update the page table */
  page_table_[*page_id] = frame_id;
  // printf("add %d with %d\n", frame_id, *page_id);

  return page;
}

bool BufferPoolManager::DeletePage(page_id_t page_id) {
  std::unique_lock<std::mutex> lock =
      IsThreadSafe() ? std::unique_lock<std::mutex>(latch_) : std::unique_lock<std::mutex>();
  //  std::scoped_lock lock{latch_};

  // 0.   Make sure you call DiskManager::DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    // 1.   If P does not exist, return true.
    return true;
  }
  frame_id_t frame_id = it->second;
  Page *page = &pages_[frame_id];

  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  if (page->GetPinCount() != 0) {
    return false;
  }

  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free

  /* update the page table */
  page_table_.erase(it);

  /* deallocate */
  disk_manager_->DeallocatePage(page_id);

  /* update metadata */
  page->is_dirty_ = false;
  page->page_id_ = INVALID_PAGE_ID;
  page->pin_count_ = 0;

  /* add it into the free list */
  free_list_.push_back(static_cast<int>(frame_id));

  /* modify the LRU queue */
  replacer_->Pin(frame_id);

  return true;
}

void BufferPoolManager::FlushAllPages() {
  std::unique_lock<std::mutex> lock =
      IsThreadSafe() ? std::unique_lock<std::mutex>(latch_) : std::unique_lock<std::mutex>();
  //  std::scoped_lock lock{latch_};

  for (auto &item : page_table_) {
    frame_id_t frame_id = item.second;
    Page *page = &pages_[frame_id];
    disk_manager_->WritePage(page->GetPageId(), page->GetData());
    page->is_dirty_ = false;
  }
}

void BufferPoolManager::Initialize() {
  std::unique_lock<std::mutex> lock =
      IsThreadSafe() ? std::unique_lock<std::mutex>(latch_) : std::unique_lock<std::mutex>();
  replacer_->Clear();
  free_list_.clear();

  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.push_back(static_cast<int>(i));
  }
}

}  // namespace thomas
