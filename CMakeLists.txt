cmake_minimum_required(VERSION 3.28)
project(sfml-imgui LANGUAGES CXX)

add_subdirectory(dependencies)
add_subdirectory(src/utils)
add_subdirectory(src)

set(DATA_PATH "${CMAKE_SOURCE_DIR}/data")
set(SHADERS_PATH "${CMAKE_SOURCE_DIR}/shaders") 
target_compile_definitions(sfml-imgui PRIVATE DATA_PATH="${DATA_PATH}" SHADERS_PATH="${SHADERS_PATH}")
    
target_include_directories(sfml-imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)