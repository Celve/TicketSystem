#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>

#include "type/string.h"

namespace thomas {

/**
 * @brief
 * A class with two string
 * @tparam FirstStringSize the size of the string used as the first key
 * @tparam SecondStringSize the size of the string used as the second key
 */
template <size_t FirstStringSize, size_t SecondStringSize>
class DualString {
 public:
  DualString() = default;
  explicit DualString(const std::string &str1, const std::string &str2) : first_str_(str1), second_str_(str2) {}

  /**
   * @brief Set the Value object
   * @param str1 please no overflow
   * @param str2 please no overflow
   */
  void SetValue(const std::string &str1, const std::string &str2) {
    first_str_.SetValue(str1);
    second_str_.SetValue(str2);
  }

  /**
   * @brief Set the Value object
   * @param str1 please no overflow
   * @param str2 please no overflow
   */
  void SetValue(char *str1, char *str2) {
    first_str_.SetValue(str1);
    second_str_.SetValue(str2);
  }

  int CompareFirstWith(const DualString &rhs) const { return first_str_.CompareWith(rhs.first_str_); }

  int CompareSecondWith(const DualString &rhs) const { return second_str_.CompareWith(rhs.second_str_); }

  friend std::ostream &operator<<(std::ostream &os, const DualString &src) {
    os << "(" << src.first_str_ << "," << src.second_str_ << ")";
    return os;
  }

 private:
  String<FirstStringSize> first_str_;
  String<SecondStringSize> second_str_;
};

/**
 * @brief
 * A comparator for the dual string class.
 * @tparam FirstStringSize the size of the string used as the first key
 * @tparam SecondStringSize the size of the string used as the second key
 */
template <size_t FirstStringSize, size_t SecondStringSize>
class DualStringComparator {
 public:
  /**
   * @brief Construct a new String Int Comparator object
   * @param type 1 for the comparator used to compare both; 2 for the comparator
   * used to compare the first key; other values are invalid
   */
  explicit DualStringComparator(int type) : type_(type) {}

  int operator()(const DualString<FirstStringSize, SecondStringSize> &lhs,
                 const DualString<FirstStringSize, SecondStringSize> &rhs) const {
    auto first_result = lhs.CompareFirstWith(rhs);
    switch (type_) {
      case 1:
        return first_result;
      case 2:
        if (first_result != 0) {
          return first_result;
        }
        return lhs.CompareSecondWith(rhs);
      default:
        return 0;
    }
  }

 private:
  int type_;
};

}  // namespace thomas