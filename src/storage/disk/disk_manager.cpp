#include <sys/stat.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>  // NOLINT

#include "storage/disk/disk_manager.h"

namespace thomas {

static char *buffer_used;

/**
 * Constructor: open/create a single database file & log file
 * @input db_file: database file name
 */
DiskManager::DiskManager(const std::string &db_file)
    : file_name_(db_file), next_page_id_(0) {
  std::string::size_type n = file_name_.rfind('.');
  if (n == std::string::npos) {
    throw std::runtime_error("wrong file format");
  }

  db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
  // directory or file does not exist
  if (!db_io_.is_open()) {
    db_io_.clear();
    // create a new file
    db_io_.open(db_file, std::ios::binary | std::ios::trunc | std::ios::out);
    db_io_.close();
    // reopen with original mode
    db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
    if (!db_io_.is_open()) {
      throw std::runtime_error("can't open db file");
    }
  }
  buffer_used = nullptr;
}

/**
 * Close all file streams
 */
void DiskManager::ShutDown() {
  db_io_.close();
}

/**
 * Write the contents of the specified page into disk file
 */
void DiskManager::WritePage(page_id_t page_id, const char *page_data) {
  size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
  // set write cursor to offset
  db_io_.seekp(offset);
  db_io_.write(page_data, PAGE_SIZE);
  // check for I/O error
  if (db_io_.bad()) {
    throw std::runtime_error("I/O error while writing");
  }
  // needs to flush to keep disk file in sync
  db_io_.flush();
}

/**
 * Read the contents of the specified page into the given memory area
 */
void DiskManager::ReadPage(page_id_t page_id, char *page_data) {
  int offset = page_id * PAGE_SIZE;
  // check if read beyond file length
  if (offset > GetFileSize(file_name_)) {
    throw std::runtime_error("I/O error reading past end of file");
    // std::cerr << "I/O error while reading" << std::endl;
  } else {
    // set read cursor to offset
    db_io_.seekp(offset);
    db_io_.read(page_data, PAGE_SIZE);
    if (db_io_.bad()) {
      throw std::runtime_error("I/O error while reading");
    }
    // if file ends before reading PAGE_SIZE
    int read_count = db_io_.gcount();
    if (read_count < PAGE_SIZE) {
      throw std::runtime_error("Read less than a page");
      db_io_.clear();
      // std::cerr << "Read less than a page" << std::endl;
      memset(page_data + read_count, 0, PAGE_SIZE - read_count);
    }
  }
}

/**
 * Allocate new page (operations like create index/table)
 * For now just keep an increasing counter
 */
page_id_t DiskManager::AllocatePage() { return next_page_id_++; }

/**
 * Deallocate page (operations like drop index/table)
 * Need bitmap in header page for tracking pages
 * This does not actually need to do anything for now.
 */
void DiskManager::DeallocatePage(__attribute__((unused)) page_id_t page_id) {}

/**
 * Private helper function to get disk file size
 */
int DiskManager::GetFileSize(const std::string &file_name) {
  struct stat stat_buf;
  int rc = stat(file_name.c_str(), &stat_buf);
  return rc == 0 ? static_cast<int>(stat_buf.st_size) : -1;
}

}  // namespace thomas
