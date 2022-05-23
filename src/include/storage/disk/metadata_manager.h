#pragma once

#include <atomic>
#include <fstream>
#include <string>

#include "common/config.h"

namespace thomas {

class MetadataManager {
 public:
  explicit MetadataManager();

  ~MetadataManager() = default;

  void ShutDown();

  void Write(const char *page_data);

  void Read(char *page_data);
  
 private:
  std::fstream db_io_;
  std::string file_name_;
};

}  // namespace thomas
