#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>

namespace thomas {

template <size_t StringSize>
class String {
 public:
  String() = default;
  explicit String(const std::string &str) { SetValue(str); }

  void SetValue(const std::string &str) {
    memset(data_, 0, sizeof(data_));
    memcpy(data_, str.c_str(), str.size());
  }

  void SetValue(char *str) {
    memset(data_, 0, sizeof(data_));
    memcpy(data_, str, sizeof(data_));
  }

  std::string GetValue() { return std::string(data_, data_ + strlen(data_)); }

  int CompareWith(const String &rhs) const { return strcmp(data_, rhs.data_); }

  size_t Hash() const {
    size_t hash_value = 0;
    for (int i = 0; data_[i] != '\0'; ++i) {
      hash_value = hash_value * 31 + data_[i];
    }
    return hash_value;
  }

  bool operator<(const String &rhs) const { return CompareWith(rhs) < 0; }

  inline int64_t ToString() const { return *reinterpret_cast<int64_t *>(const_cast<char *>(data_)); }

  friend std::ostream &operator<<(std::ostream &os, const String &src) {
    size_t size = strlen(src.data_);
    for (size_t i = 0; i < size; ++i) {
      os << src.data_[i];
    }
    return os;
  }

 private:
  char data_[StringSize];
};

template <size_t StringSize>
class StringComparator {
 public:
  int operator()(const String<StringSize> &lhs, const String<StringSize> &rhs) const { return lhs.CompareWith(rhs); }
};

}  // namespace thomas