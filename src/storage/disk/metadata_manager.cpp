#include "storage/disk/metadata_manager.h"
#include "config.h"

namespace thomas {

MetadataManager::MetadataManager() : file_name_("metadata.db") {
  db_io_.open(file_name_, std::ios::binary | std::ios::in | std::ios::out);
  if (!db_io_.is_open()) {
    db_io_.clear();
    // create a new file
    db_io_.open(file_name_, std::ios::binary | std::ios::trunc | std::ios::out);
    db_io_.close();
    // reopen with original mode
    db_io_.open(file_name_, std::ios::binary | std::ios::in | std::ios::out);
    // do some initialization
    db_io_.seekp(0);
    int size = 0;
    db_io_.write(reinterpret_cast<char *>(&size), sizeof(int));
  }
}

void MetadataManager::ShutDown() {
  db_io_.close();
}

void MetadataManager::Write(const char *page_data) {
  db_io_.seekp(0);
  db_io_.write(page_data, PAGE_SIZE);
  db_io_.flush();
}

void MetadataManager::Read(char *page_data) {
  db_io_.seekp(0);
  db_io_.read(page_data, PAGE_SIZE);
}

}