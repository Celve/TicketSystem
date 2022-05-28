#include "thread/thread_pool.h"

#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>

#include "common/exceptions.hpp"

namespace thomas {

std::unique_lock<std::mutex> *ThreadPool::GetLock() { return lock_; }

void ThreadPool::WorkerFunction() {
  while (true) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(latch_);
      this->cv_.wait(lock, [this] { return this->isTerminated || !this->tasks_.empty(); });
      std::cout << "a";
      if (this->isTerminated && this->tasks_.empty()) {
        return;
      }
      task = std::move(this->tasks_.front());
      this->tasks_.pop();
      lock_ = &lock;
      task();
    }
  }
}

ThreadPool::ThreadPool(size_t number_of_threads) {
  if (number_of_threads == 0) {
    number_of_threads = std::thread::hardware_concurrency();
  }
  for (size_t i = 0; i < number_of_threads; ++i) {
    workers_.emplace_back(&ThreadPool::WorkerFunction, this);
  }
  isTerminated = false;
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(latch_);
    isTerminated = true;
  }
  cv_.notify_all();
  for (std::thread &worker : workers_) {
    worker.join();
  }
}

}  // namespace thomas
