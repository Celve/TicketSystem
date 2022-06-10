#pragma once

#include <atomic>
#include <fstream>
#include <string>

#include "common/config.h"

namespace thomas {

/**
 * DiskManager takes care of the allocation and deallocation of pages within a database. It performs the reading and
 * writing of pages to and from disk, providing a logical file layer within the context of a database management system.
 */
class DiskManager {
 public:
  /**
   * Creates a new disk manager that writes to the specified database file.
   * @param db_file the file name of the database file to write to
   */
  explicit DiskManager(const std::string &db_file);

  ~DiskManager() = default;

  /**
   * Shut down the disk manager and close all the file resources.
   */
  void ShutDown();

  /**
   * @brief
   * Clear the file.
   */
  void Clear();

  /**
   * Write a page to the database file.
   * @param page_id id of the page
   * @param page_data raw page data
   */
  void WritePage(page_id_t page_id, const char *page_data);

  /**
   * Read a page from the database file.
   * @param page_id id of the page
   * @param[out] page_data output buffer
   */
  void ReadPage(page_id_t page_id, char *page_data);

  /**
   * Allocate a page on disk.
   * @return the id of the allocated page
   */
  page_id_t AllocatePage();

  /**
   * Deallocate a page on disk.
   * @param page_id id of the page to deallocate
   */
  void DeallocatePage(page_id_t page_id);

  void SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

  page_id_t GetNextPageId() { return next_page_id_; }

 private:
  int GetFileSize(const std::string &file_name);
  // stream to write db file
  std::fstream db_io_;
  std::string file_name_;
  std::atomic<page_id_t> next_page_id_;
};

}  // namespace thomas
