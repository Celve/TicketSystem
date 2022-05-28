cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
# set(CMAKE_CXX_COMPILER /usr/bin/clang++)
# set(CMAKE_C_COMPILER /usr/bin/clang)

project(Database)

set(CMAKE_CXX_FLAGS "-pthread -g")

set(SOURCE_CPPS
    src/storage/disk/disk_manager.cpp

    src/buffer/lru_replacer.cpp
    src/buffer/buffer_pool_manager.cpp

    src/storage/page/b_plus_tree_page.cpp
    src/storage/page/b_plus_tree_internal_page.cpp
    src/storage/page/b_plus_tree_leaf_page.cpp
    src/storage/index/index_iterator.cpp
    src/storage/index/b_plus_tree_nts.cpp
    src/storage/index/b_plus_tree_ts.cpp
    src/storage/index/b_plus_tree_index_ts.cpp
    src/storage/index/b_plus_tree_index_nts.cpp
    src/storage/page/header_page.cpp

    src/thread/thread_pool.cpp
)

add_executable(code main.cpp ${SOURCE_CPPS})

include_directories(src/include)

