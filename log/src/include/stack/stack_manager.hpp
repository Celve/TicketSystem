#pragma once

#include <cstdlib>
#include <fstream>
namespace thomas {

#define STACK_TEMPLATE template <typename T>
#define STACK_TYPE StackManager<T>

/**
 * @brief
 * a stack on disk
 */
STACK_TEMPLATE
class StackManager {
 public:
  explicit StackManager(const std::string &stack_name);

  StackManager(const StackManager<T> &rhs);

  ~StackManager();

  void Push(const T &value);

  void Pop(T &value);

  size_t Size();

  bool IsEmpty();

 private:
  std::fstream stack_io_;
  std::string file_name_;
  size_t sum_;
};

STACK_TEMPLATE
STACK_TYPE::StackManager(const std::string &stack_name) : file_name_(stack_name) {
  if (stack_name.rfind('.') == std::string::npos) {
    throw std::runtime_error("wrong file format");
  }
  stack_io_.open(stack_name, std::ios::binary | std::ios::in | std::ios::out);
  /* the file is not exist */
  if (!stack_io_.is_open()) {
    /* create a new file */
    stack_io_.clear();
    stack_io_.open(stack_name, std::ios::binary | std::ios::trunc | std::ios::out);
    stack_io_.close();
    stack_io_.open(stack_name, std::ios::binary | std::ios::in | std::ios::out);
  } else {
    /* read exist data */
    int offset = 0;
    stack_io_.seekp(offset);
    stack_io_.read(reinterpret_cast<char *>(&sum_), sizeof(size_t));
  }
}

STACK_TEMPLATE
STACK_TYPE::StackManager(const StackManager<T> &rhs) {

}

STACK_TEMPLATE
STACK_TYPE::~StackManager() {
  int offset = 0;
  stack_io_.seekp(offset);
  stack_io_.write(reinterpret_cast<char *>(&sum_), sizeof(size_t));
  stack_io_.close();
}

STACK_TEMPLATE
void STACK_TYPE::Push(const T &value) {
  int offset = sizeof(size_t) + sizeof(T) * (sum_++);
  stack_io_.seekp(offset);
  stack_io_.write(reinterpret_cast<const char *>(&value), sizeof(T));
  if (stack_io_.bad()) {
    throw std::runtime_error("I/O error while writing");
  }
}

STACK_TEMPLATE
void STACK_TYPE::Pop(T &value) {
  if (sum_ == 0) {
    throw std::runtime_error("cannot pop, no element inside");
  }
  int offset = sizeof(size_t) + sizeof(T) * (--sum_);
  stack_io_.seekp(offset);
  stack_io_.read(reinterpret_cast<char *>(&value), sizeof(T));
  if (stack_io_.bad()) {
    throw std::runtime_error("I/O error while reading");
  }
}

STACK_TEMPLATE
size_t STACK_TYPE::Size() { return sum_; }

STACK_TEMPLATE
bool STACK_TYPE::IsEmpty() { return sum_ == 0; }

}  // namespace thomas