#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>

namespace thomas {

/**
 * @brief
 * A class with a string as the first key and a type as the second key
 * type can be whatever you want, however, it should provide some basic compare operators
 * @tparam StringSize the size of the string
 */
template <class T, size_t StringSize>
class StringAny {
 public:
  explicit StringAny(const std::string &str, const T &value) { SetValue(str, value); }

  void SetValue(const std::string &str, const T &value) {
    memset(data_str_, 0, sizeof(data_str_));
    memcpy(data_str_, str.c_str(), str.size());
    data_t_ = value;
  }

  void SetValue(char *str, const T &value) {
    memset(data_str_, 0, sizeof(data_str_));
    memcpy(data_str_, str, sizeof(data_str_));
    data_t_ = value;
  }

  int CompareTypeWith(const StringAny &rhs) const {
    return data_t_ < rhs.data_t_ ? -1 : (data_t_ == rhs.data_t_ ? 0 : 1);
  }

  int CompareStringWith(const StringAny &rhs) const { return strcmp(data_str_, rhs.data_str_); }

  inline int64_t ToString() const { return *reinterpret_cast<int64_t *>(const_cast<char *>(data_str_)); }

  friend std::ostream &operator<<(std::ostream &os, const StringAny &src) {
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
  T data_t_;
};

/**
 * @brief
 * A comparator for string int class
 * @tparam StringSize the size of the string.
 */
template <class T, size_t StringSize>
class StringAnyComparator {
 public:
  /**
   * @brief Construct a new String Int Comparator object
   * @param type 1 only compares string; 2 only compares type; 3 use string as first key; 4 use type as first key
   * comparing only the first key; other values are invalid
   */
  explicit StringAnyComparator(int category) : category_(category) {}

  int operator()(const StringAny<T, StringSize> &lhs, const StringAny<T, StringSize> &rhs) const {
    int temp_result = 0;
    switch (category_) {
      case 1:
        return lhs.CompareStringWith(rhs);
      case 2:
        return lhs.CompareTypeWith(rhs);
      case 3:
        temp_result = lhs.CompareStringWith(rhs);
        return temp_result == 0 ? lhs.CompareTypeWith(rhs) : temp_result;
      case 4:
        temp_result = lhs.CompareIntWith(rhs);
        return temp_result == 0 ? lhs.CompareStringWith(rhs) : temp_result;
      default:
        return 0;
    }
  }

 private:
  int category_;
};

}  // namespace thomas