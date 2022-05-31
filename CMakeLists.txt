cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=address -ftrapv")
# 删除编译选项

#MATH(EXPR stack_size "512*1024*1024")
#set(CMAKE_EXE_LINKER_FLAGS "-Wl,-stack,${stack_size}")

include_directories(back_end/src
        back_end/database)

add_executable(code     # 生成的可执行文件名称
        back_end/main.cpp
        back_end/src/Command.cpp
        back_end/src/Account.cpp
        back_end/src/TrainSystem.cpp

        back_end/database/FileStorage.h
        back_end/database/map.hpp
        back_end/database/vector.hpp
        back_end/database/Library.h
        back_end/database/Library.cpp
        back_end/database/ull.h
        )
