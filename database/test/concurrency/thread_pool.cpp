void Test2() {
  ThreadPool pool(4);
  std::vector<std::future<int>> results;
  results.reserve(8);
  for (int i = 0; i < 8; ++i) {
    results.emplace_back(pool.Join([i] {
      Wow(i);
      return i;
    }));
  }

  for (auto &&result : results) {
    std::cout << result.get() << std::endl;
  }
  std::cout << std::endl;
}