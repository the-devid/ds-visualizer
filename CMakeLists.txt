cmake_minimum_required(VERSION 3.16)
project(ds_visualizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
endif()
