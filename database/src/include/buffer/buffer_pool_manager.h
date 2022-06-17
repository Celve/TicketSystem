#pragma once

#include <list>
#include <mutex>  // NOLINT
#include <vector>

#include "buffer/lru_replacer.h"
#include "storage/disk/disk_manager.h"
#include "storage/page/page.h"
#include "thread/thread_safe.h"

namespace thomas {

/**
 * BufferPoolManager reads disk pages to and from its internal buffer pool.
 */
class BufferPoolManager {
 public:
  enum class CallbackType { BEFORE, AFTER };
  using bufferpool_callback_fn = void (*)(enum CallbackType, const page_id_t page_id);

  /**
   * Creates a new BufferPoolManager.
   * @param pool_size the size of the buffer pool
   * @param disk_manager the disk manager
   * @param log_manager the log manager (for testing only: nullptr = disable logging)
   */
  BufferPoolManager(size_t pool_size, DiskManager *disk_manager,
                    THREAD_SAFE_TYPE ts_type = THREAD_SAFE_TYPE::NON_THREAD_SAFE);

  /**
   * Destroys an existing BufferPoolManager.
   */
  ~BufferPoolManager();

  size_t Size() { return free_list_.size() + replacer_->Size(); }

  /** @return pointer to all the pages in the buffer pool */
  Page *GetPages() { return pages_; }

  /** @return size of the buffer pool */
  size_t GetPoolSize() { return pool_size_; }

  /**
   * Fetch the requested page from the buffer pool.
   * @param page_id id of page to be fetched
   * @return the requested page
   */
  Page *FetchPage(page_id_t page_id);

  /**
   * Unpin the target page from the buffer pool.
   * @param page_id id of page to be unpinned
   * @param is_dirty true if the page should be marked as dirty, false otherwise
   * @return false if the page pin count is <= 0 before this call, true otherwise
   */
  bool UnpinPage(page_id_t page_id, bool is_dirty);

  /**
   * Flushes the target page to disk.
   * @param page_id id of page to be flushed, cannot be INVALID_PAGE_ID
   * @return false if the page could not be found in the page table, true otherwise
   */
  bool FlushPage(page_id_t page_id);

  /**
   * Creates a new page in the buffer pool.
   * @param[out] page_id id of created page
   * @return nullptr if no new pages could be created, otherwise pointer to new page
   */
  Page *NewPage(page_id_t *page_id);

  /**
   * Deletes a page from the buffer pool.
   * @param page_id id of page to be deleted
   * @return false if the page exists but could not be deleted, true if the page didn't exist or deletion succeeded
   */
  bool DeletePage(page_id_t page_id);

  /**
   * Flushes all the pages in the buffer pool to disk.
   */
  void FlushAllPages();

  /**
   * @brief
   * Initialize the buffer pool
   */
  void Initialize();

  /**
   * @brief
   * Whether the algorithm is thread-safe.
   */
  bool IsThreadSafe() { return ts_type_ == THREAD_SAFE_TYPE::THREAD_SAFE; }

 protected:
  /**
   * @brief
   * Try to find an empty frame for later use.
   * @return frame_id_t for the particular frame.
   * @return -1 is none is available.
   */
  frame_id_t FindFrame();

  /** Number of pages in the buffer pool. */
  size_t pool_size_;
  /** Array of buffer pool pages. */
  Page *pages_;
  /** Pointer to the disk manager. */
  DiskManager *disk_manager_;
  /** Page table for keeping track of buffer pool pages. */
  linked_hashmap<page_id_t, frame_id_t> page_table_;
  /** Replacer to find unpinned pages for replacement. */
  Replacer *replacer_;
  /** List of free pages. */
  std::list<frame_id_t> free_list_;
  /** This latch protects shared data structures. We recommend updating this comment to describe what it protects. */
  std::mutex latch_;
  /** Whether it needs to be thread safe*/
  THREAD_SAFE_TYPE ts_type_;
};
}  // namespace thomas
