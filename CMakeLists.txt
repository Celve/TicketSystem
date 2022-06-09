cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=address -ftrapv")
# 删除编译选项

#MATH(EXPR stack_size "512*1024*1024")
#set(CMAKE_EXE_LINKER_FLAGS "-Wl,-stack,${stack_size}")

add_subdirectory(database)

add_executable(code main.cpp
        back_end/src/Command.cpp
        back_end/src/Account.cpp
        back_end/src/TrainSystem.cpp
        back_end/src/Management.cpp

        back_end/libs/FileStorage.h
        back_end/libs/map.hpp
#        back_end/libs/vector.hpp
        back_end/libs/Library.h
        back_end/libs/Library.cpp
        back_end/libs/ull.h
        )

target_link_libraries(code database)

include_directories(back_end/src back_end/libs)
