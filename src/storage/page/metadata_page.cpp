#include <cassert>
#include <iostream>

#include "storage/page/Metadata_page.h"

namespace thomas {

/**
 * Record related
 */
bool MetadataPage::InsertRecord(const std::string &name, const page_id_t root_id) {
  assert(name.length() < 32);
  assert(root_id > INVALID_PAGE_ID);

  int record_num = GetRecordCount();
  int offset = 4 + record_num * 36;
  // check for duplicate name
  if (FindRecord(name) != -1) {
    return false;
  }
  // copy record content
  memcpy(data_ + offset, name.c_str(), (name.length() + 1));
  memcpy((data_ + offset + 32), &root_id, 4);

  SetRecordCount(record_num + 1);
  return true;
}

bool MetadataPage::DeleteRecord(const std::string &name) {
  int record_num = GetRecordCount();
  assert(record_num > 0);

  int index = FindRecord(name);
  // record does not exsit
  if (index == -1) {
    return false;
  }
  int offset = index * 36 + 4;
  memmove(data_ + offset, data_ + offset + 36, (record_num - index - 1) * 36);

  SetRecordCount(record_num - 1);
  return true;
}

bool MetadataPage::UpdateRecord(const std::string &name, const page_id_t root_id) {
  assert(name.length() < 32);

  int index = FindRecord(name);
  // record does not exsit
  if (index == -1) {
    return false;
  }
  int offset = index * 36 + 4;
  // update record content, only root_id
  memcpy((data_ + offset + 32), &root_id, 4);

  return true;
}

bool MetadataPage::GetRootId(const std::string &name, page_id_t *root_id) {
  assert(name.length() < 32);

  int index = FindRecord(name);
  // record does not exsit
  if (index == -1) {
    return false;
  }
  int offset = (index + 1) * 36;
  *root_id = *reinterpret_cast<page_id_t *>(data_ + offset);

  return true;
}

/**
 * helper functions
 */
// record count
int MetadataPage::GetRecordCount() { return *reinterpret_cast<int *>(data_); }

void MetadataPage::SetRecordCount(int record_count) { memcpy(data_, &record_count, 4); }

int MetadataPage::FindRecord(const std::string &name) {
  int record_num = GetRecordCount();

  for (int i = 0; i < record_num; i++) {
    char *raw_name = reinterpret_cast<char *>(data_ + (4 + i * 36));
    if (strcmp(raw_name, name.c_str()) == 0) {
      return i;
    }
  }
  return -1;
}
}  // namespace thomas
