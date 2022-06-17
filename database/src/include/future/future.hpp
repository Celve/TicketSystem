#include <future>

#pragma once

namespace thomas {

template <typename T>
std::future<T> MakeFuture(T&& t) {
  std::promise<T> p;
  p.set_value(std::forward<T>(t));
  return p.get_future();
}

}  // namespace thomas