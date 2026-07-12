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

add_executable(test_order_book_stl tests/test_order_book.cpp)
target_link_libraries(test_order_book_stl PRIVATE lob_book_stl)
add_test(NAME order_book_stl COMMAND test_order_book_stl)

# ---------- Phase 1.5: Stage 1.5 ----------
add_library(lob_book_intrusive src/order_book_intrusive.cpp)

add_executable(test_order_book_intrusive tests/test_order_book.cpp)
target_compile_definitions(test_order_book_intrusive PRIVATE USE_INTRUSIVE)
target_link_libraries(test_order_book_intrusive PRIVATE lob_book_intrusive)
add_test(NAME order_book_intrusive COMMAND test_order_book_intrusive)

add_executable(test_intrusive_list tests/test_intrusive_list.cpp)
add_test(NAME intrusive_list COMMAND test_intrusive_list)

add_executable(test_intrusive_bst tests/test_intrusive_bst.cpp)
add_test(NAME intrusive_bst COMMAND test_intrusive_bst)

add_executable(bench_book_backend bench/bench_book_backend.cpp)
target_link_libraries(bench_book_backend PRIVATE lob_book_stl lob_book_intrusive)

# ---------- Phase 2: Stage 2 ----------
add_library(lob_itch_parser src/itch_parser.cpp)
target_link_libraries(lob_itch_parser PUBLIC lob_book_stl)

add_executable(stage2_demo apps/stage2_itch_replay.cpp)
target_link_libraries(stage2_demo PRIVATE lob_itch_parser)

add_executable(test_itch_parser tests/test_itch_parser.cpp)
target_link_libraries(test_itch_parser PRIVATE lob_itch_parser)
add_test(NAME itch_parser COMMAND test_itch_parser)

# ---------- Phase 3: Stage 3 ----------
add_executable(test_mutex_queue tests/test_mutex_queue.cpp)
add_test(NAME mutex_queue COMMAND test_mutex_queue)

add_executable(stage3_demo apps/stage3_threaded_replay.cpp)
target_link_libraries(stage3_demo PRIVATE lob_itch_parser)