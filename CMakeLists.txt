cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=address -ftrapv -Ofast")

add_subdirectory(database)

include_directories(backend/src backend/libs log)

add_executable(code main.cpp
  backend/src/Command.cpp
  backend/src/Account.cpp
  backend/src/TrainSystem.cpp
  backend/src/Management.cpp

  backend/libs/FileStorage.h
  backend/libs/map.hpp
  backend/libs/Library.h
  backend/libs/Library.cpp
  backend/libs/ull.h
)

target_link_libraries(code PUBLIC database)

