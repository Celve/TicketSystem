#pragma once

#include <cstdio>
#include <iostream>

namespace bustub {

template<size_t StringSize>
class FixedString {
public:
  void SetValue(std::string str) {
    memset(data_, 0, sizeof(data_));
    memcpy(data_, str.c_str(), str.size());
  }

  void SetValue(char *str, int size) {
    memset(data_, 0, sizeof(data_));
    memcpy(data_, str, size);
  }

  int CompareWith(const FixedString &rhs) const {
    return strcmp(data_, rhs.data_);
  }

  inline int64_t ToString() const { return *reinterpret_cast<int64_t *>(const_cast<char *>(data_)); }

  friend std::ostream &operator<<(std::ostream &os, const FixedString &src) {
    size_t size = strlen(src.data_);
    for (size_t i = 0; i < size; ++i) {
      os << src.data_[i];
    }
    return os;
  }

private: 
  char data_[StringSize];
};

template<size_t StringSize> 
class FixedStringComparator {
public:
  int operator()(const FixedString<StringSize> &lhs, const FixedString<StringSize> &rhs) const {
    return lhs.CompareWith(rhs);
  }
};

}