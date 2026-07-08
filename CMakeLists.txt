cmake_minimum_required(VERSION 3.16)
project(lock_free_order_book CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(include)
enable_testing()

# ---------- Phase 1: Stage 1 ----------
add_library(lob_book_stl src/order_book_stl.cpp)

add_executable(test_order_book_stl tests/test_order_book.cpp)
target_link_libraries(test_order_book_stl PRIVATE lob_book_stl)
add_test(NAME order_book_stl COMMAND test_order_book_stl)

# ---------- Phase 1.5: Stage 1.5 ----------
add_library(lob_book_intrusive src/order_book_intrusive.cpp)

add_executable(test_intrusive_list tests/test_intrusive_list.cpp)
add_test(NAME intrusive_list COMMAND test_intrusive_list)

add_executable(test_intrusive_bst tests/test_intrusive_bst.cpp)
add_test(NAME intrusive_bst COMMAND test_intrusive_bst)

# (Optional) If you also want to test order_book_intrusive.cpp
# add_executable(test_order_book_intrusive tests/test_order_book.cpp)
# target_link_libraries(test_order_book_intrusive PRIVATE lob_book_intrusive)
# add_test(NAME order_book_intrusive COMMAND test_order_book_intrusive)
