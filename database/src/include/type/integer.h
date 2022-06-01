#pragma once

namespace thomas {

class Integer {
public: 
  void SetValue(int value) {
    value_ = value;
  }

  int GetValue() {
    return value_;
  }

  int CompareWith(const Integer &rhs) const {
    if (value_ < rhs.value_) {
      return -1;
    }
    if (value_ > rhs.value_) {
      return 1;
    }
    return 0;
  }

private: 
  int value_;   
};

class IntegerComparator {
public:
  int operator()(const Integer &lhs, const Integer &rhs) {
    return lhs.CompareWith(rhs);
  }
};

};