#pragma once

#include <cstdio>
#include <iostream>
#include <cstring>

namespace thomas {

template<size_t StringSize>
class MixedStringInt {
public:
  MixedStringInt() = default;

  void SetValue(std::string str, int integer) {
    memset(data_str_, 0, sizeof(data_str_));
    memcpy(data_str_, str.c_str(), str.size());
    data_int_ = integer;
  }

  void SetValue(char *str, int integer) {
    memset(data_str_, 0, sizeof(data_str_));
    memcpy(data_str_, str, sizeof(data_str_));
    data_int_ = integer;
  }

  int CompareMixedWith(const MixedStringInt &rhs) const {
    int result_str = strcmp(data_str_, rhs.data_str_);
    int result_int = data_int_ < rhs.data_int_ ? -1 : (data_int_ == rhs.data_int_ ? 0 : 1);
    return !result_str ? result_int : result_str;
  }

  int CompareStringWith(const MixedStringInt &rhs) const {
    return strcmp(data_str_, rhs.data_str_);
  }

  inline int64_t ToString() const { return *reinterpret_cast<int64_t *>(const_cast<char *>(data_str_)); }

  friend std::ostream &operator<<(std::ostream &os, const MixedStringInt &src) {
    size_t size = strlen(src.data_str_);
    os << "(";
    for (size_t i = 0; i < size; ++i) {
      os << src.data_str_[i];
    }
    os << ", " << src.data_int_ << ")";
    return os;
  }

private: 
  char data_str_[StringSize];
  int data_int_;
};

template<size_t StringSize> 
class MixedStringIntForMixedComparator {
public:
  int operator()(const MixedStringInt<StringSize> &lhs, const MixedStringInt<StringSize> &rhs) const {
    return lhs.CompareMixedWith(rhs);
  }
};

bool MixedStringIntForStringComparator(const MixedStringInt<64> &lhs, const MixedStringInt<64> &rhs);

}