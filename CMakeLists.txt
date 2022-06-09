cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=address -ftrapv -Ofast")

add_subdirectory(database)

add_executable(code main.cpp   
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

target_link_libraries(code database)

include_directories(back_end/src back_end/database)
