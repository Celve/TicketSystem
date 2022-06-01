#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>

namespace thomas {

/**
 * @brief
 * A class with a string as the first key and a int as the second key
 * @tparam StringSize the size of the string
 */
template <size_t StringSize>
class StringInt {
 public:
  void SetValue(const std::string &str, int integer) {
    memset(data_str_, 0, sizeof(data_str_));
    memcpy(data_str_, str.c_str(), str.size());
    data_int_ = integer;
  }

  void SetValue(char *str, int integer) {
    memset(data_str_, 0, sizeof(data_str_));
    memcpy(data_str_, str, sizeof(data_str_));
    data_int_ = integer;
  }

  int CompareMixedWith(const StringInt &rhs) const {
    int result_str = strcmp(data_str_, rhs.data_str_);
    int result_int =
        data_int_ < rhs.data_int_ ? -1 : (data_int_ == rhs.data_int_ ? 0 : 1);
    return result_str == 0 ? result_int : result_str;
  }

  int CompareStringWith(const StringInt &rhs) const {
    return strcmp(data_str_, rhs.data_str_);
  }

  inline int64_t ToString() const {
    return *reinterpret_cast<int64_t *>(const_cast<char *>(data_str_));
  }

  friend std::ostream &operator<<(std::ostream &os, const StringInt &src) {
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

/**
 * @brief
 * A comparator for string int class
 * @tparam StringSize the size of the string.
 */
template <size_t StringSize>
class StringIntComparator {
 public:
  /**
   * @brief Construct a new String Int Comparator object
   * @param type 0 for a comparator comparing both keys; 1 for a comparator
   * comparing only the first key; other values are invalid
   */
  explicit StringIntComparator(int type = 0) : type_(type) {}

  int operator()(const StringInt<StringSize> &lhs,
                 const StringInt<StringSize> &rhs) const {
    switch (type_) {
      case 1:
        return lhs.CompareMixedWith(rhs);
      case 2:
        return lhs.CompareStringWith(rhs);
      default:
        return 0;
    }
  }

 private:
  int type_;
};

}  // namespace thomas