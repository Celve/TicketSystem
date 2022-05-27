#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

#include "common/exceptions.hpp"

namespace thomas {

class ThreadPool {
#define THREAD_ARGS_TEMPLATE template <class F, class... Args>
#define THREAD_RETURN_TYPE std::future<decltype(std::declval<F>()(std::declval<Args>...))>
  // #define THREAD_RETURN_TYPE std::future<typename std::result_of<F(Args...)>::type>

 public:
  explicit ThreadPool(size_t number_of_threads = 0);
  ~ThreadPool();

  THREAD_ARGS_TEMPLATE
  auto Join(F &&f, Args &&...args) -> THREAD_RETURN_TYPE;

  std::mutex *GetLock();

 private:
  void WorkerFunction();

  // the worker threads
  std::vector<std::thread> workers_;
  // the task queue, latched by mutex
  std::queue<std::function<void()>> tasks_;

  // synchronization
  std::mutex lock_;
  std::mutex latch_;
  std::condition_variable cv_;

  // terminated signal
  bool isTerminated;
};

THREAD_ARGS_TEMPLATE
auto ThreadPool::Join(F &&f, Args &&...args) -> THREAD_RETURN_TYPE {
  // using return_type = typename std::result_of<F(Args...)>::type;
  using return_type = decltype(std::declval<F>()(std::declval<Args>...));
  auto task =
      std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(latch_);
    if (isTerminated) {
      throw terminated_queue();
    }
    tasks_.emplace([task]() { (*task)(); });
  }
  cv_.notify_one();
  return res;
}

}  // namespace thomas