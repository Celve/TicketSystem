cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
# set(CMAKE_CXX_COMPILER /usr/bin/clang++)
# set(CMAKE_C_COMPILER /usr/bin/clang)

project(Database)

set(CMAKE_CXX_FLAGS "-pthread -O3")

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

include_directories(../backend/src ../backend/libs)
# include_directories(src/include)

add_executable(example example.cpp ${SOURCE_CPPS})
add_library(database STATIC ${SOURCE_CPPS})

target_include_directories(database PUBLIC src/include)
target_include_directories(example PRIVATE src/include)