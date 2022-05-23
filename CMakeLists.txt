cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

project(Database)

# set(CMAKE_CXX_FLAGS "-v")

add_executable(code main.cpp
    src/storage/disk/disk_manager.cpp

    src/buffer/lru_replacer.cpp
    src/buffer/buffer_pool_manager.cpp

    src/type/mixed_string_int.cpp
    src/storage/page/metadata_page.cpp
    src/storage/page/b_plus_tree_page.cpp
    src/storage/page/b_plus_tree_internal_page.cpp
    src/storage/page/b_plus_tree_leaf_page.cpp
    src/storage/index/index_iterator.cpp
    src/storage/index/b_plus_tree.cpp
    src/storage/index/b_plus_tree_index.cpp

    src/storage/page/header_page.cpp
)

include_directories(src/include)

