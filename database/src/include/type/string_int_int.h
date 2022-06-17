#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>

#include "type/string.h"

namespace thomas {

/**
 * @brief
 * A class with a string as the first key, a int as the second key, and a int as the third key
 * type can be whatever you want, however, it should provide some basic compare operators
 * @tparam StringSize the size of the string
 */
template <size_t StringSize>
class StringIntInt {
 public:
  StringIntInt() = default;
  explicit StringIntInt(const std::string &str, int first_int, int second_int) { SetValue(str, first_int, second_int); }

  void SetValue(const std::string &str, int first_int, int second_int) {
    str_.SetValue(str);
    first_int_ = first_int;
    second_int_ = second_int;
  }

  void SetValue(char *str, int first_int, int second_int) {
    str_.SetValue(str);
    first_int_ = first_int;
    second_int_ = second_int;
  }

  size_t Hash() const {
    size_t hash_value = str_.Hash() + first_int_;
    return hash_value;
  }

  int CompareStringWith(const StringIntInt &rhs) const { return str_.CompareWith(rhs.str_); }

  int CompareFirstIntWith(const StringIntInt &rhs) const {
    return first_int_ < rhs.first_int_ ? -1 : (first_int_ == rhs.first_int_ ? 0 : 1);
  }

  int CompareSecondIntWith(const StringIntInt &rhs) const {
    return second_int_ < rhs.second_int_ ? -1 : (second_int_ == rhs.second_int_ ? 0 : 1);
  }

  inline int64_t ToString() const { return *reinterpret_cast<int64_t *>(const_cast<char *>(str_)); }

  friend std::ostream &operator<<(std::ostream &os, const StringIntInt &src) {
    os << "(" << src.str_ << "," << src.first_int_ << "," << src.second_int_ << ")";
    return os;
  }

 private:
  String<StringSize> str_;
  int first_int_;
  int second_int_;
};

/**
 * @brief
 * A comparator for string int class
 * @tparam StringSize the size of the string.
 */
template <size_t StringSize>
class StringIntIntComparator {
 public:
  /**
   * @brief Construct a new String Int Comparator object
   * @param type 1 only compares string and first int; 2 compares all
   * key comparing only the first key; other values are invalid
   */
  explicit StringIntIntComparator(int category) : category_(category) {}

  int operator()(const StringIntInt<StringSize> &lhs, const StringIntInt<StringSize> &rhs) const {
    int result = 0;
    switch (category_) {
      case 1:
        result = lhs.CompareStringWith(rhs);
        return result == 0 ? lhs.CompareFirstIntWith(rhs) : result;
      case 2:
        result = lhs.CompareStringWith(rhs);
        if (result == 0) {
          result = lhs.CompareFirstIntWith(rhs);
        }
        if (result == 0) {
          result = lhs.CompareSecondIntWith(rhs);
        }
        return result;
      default:
        return 0;
    }
  }

 private:
  int category_;
};

}  // namespace thomas