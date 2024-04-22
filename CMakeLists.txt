cmake_minimum_required(VERSION 3.16)
project(ds_visualizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_program(CLANG_TIDY_EXECUTABLE "clang-tidy")
if (CLANG_TIDY_EXECUTABLE)
  message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXECUTABLE}")
  set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
else()
  message(STATUS "clang-tidy not found")
endif()


if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  if (CMAKE_BUILD_TYPE MATCHES "DEBUG")
    add_compile_options(-g -O0)
  endif()
  add_compile_options(-Wall -Wextra -Werror)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_compile_options(/W4)
endif()

add_executable(ds_visualizer
    main.cpp 
    src/application.cpp
    src/controller.cpp
    src/two_three_tree.cpp
    src/view.cpp
    src/window.cpp
)

target_link_libraries(ds_visualizer Qt6::Widgets Qt6::Gui)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (TESTS)
  add_subdirectory(lib/gtest)
  enable_testing()
  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
  add_executable(test_two_three_tree
      src/two_three_tree.cpp
      tests/two_three_tree_ut.cpp)
  target_link_libraries(test_two_three_tree gtest gtest_main)
endif()
