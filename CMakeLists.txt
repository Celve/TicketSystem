cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=address -ftrapv")
# 删除编译选项

#MATH(EXPR stack_size "512*1024*1024")
#set(CMAKE_EXE_LINKER_FLAGS "-Wl,-stack,${stack_size}")

add_executable(code     # 生成的可执行文件名称
        main.cpp
        # --------------------------------------------back_end
        # back_end/main.cpp
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

        # --------------------------------------------database

        database/src/include/container/vector.hpp

        database/src/storage/page/b_plus_tree_page.cpp
        database/src/storage/page/b_plus_tree_internal_page.cpp
        database/src/storage/page/b_plus_tree_leaf_page.cpp
        database/src/storage/index/index_iterator.cpp
        database/src/storage/index/b_plus_tree_nts.cpp
        database/src/storage/index/b_plus_tree_ts.cpp
        database/src/storage/index/b_plus_tree_index_ts.cpp
        database/src/storage/index/b_plus_tree_index_nts.cpp
        database/src/storage/page/header_page.cpp

        database/src/thread/thread_pool.cpp

        database/src/storage/disk/disk_manager.cpp

        database/src/buffer/lru_replacer.cpp
        database/src/buffer/innodb_replacer.cpp
        database/src/buffer/buffer_pool_manager.cpp
        )

include_directories(back_end/src back_end/libs database/src/include)
