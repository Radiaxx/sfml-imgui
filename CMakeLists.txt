cmake_minimum_required(VERSION 3.28)
project(sfml-imgui LANGUAGES CXX)

add_subdirectory(dependencies)
add_subdirectory(src/utils)
add_subdirectory(src)

set(DATA_PATH "${CMAKE_SOURCE_DIR}/data")
target_compile_definitions(sfml-imgui PRIVATE DATA_PATH="${DATA_PATH}")
