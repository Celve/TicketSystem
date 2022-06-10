cmake_minimum_required(VERSION 3.16)

project(LogSystem)

set(CMAKE_CXX_STANDARD 17)

add_library(log src/include/stack/stak_manager.hpp)

target_include_directories(src/include/stack)
