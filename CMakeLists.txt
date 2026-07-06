cmake_minimum_required(VERSION 3.16)
project(lock_free_order_book CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(include)
enable_testing()

# ---------- Phase 1: Stage 1 ----------
add_library(lob_book_stl src/order_book_stl.cpp)

add_executable(stage1_demo apps/stage1_book_demo.cpp)
target_link_libraries(stage1_demo PRIVATE lob_book_stl)
