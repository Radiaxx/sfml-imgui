cmake_minimum_required(VERSION 3.28)
project(sfml-imgui LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(dependencies)
add_subdirectory(src/utils)
add_subdirectory(src)

set(ASC_DATA_PATH "${CMAKE_SOURCE_DIR}/data/asc")
set(GEO_DATA_PATH "${CMAKE_SOURCE_DIR}/data/geo")
set(SHADERS_PATH "${CMAKE_SOURCE_DIR}/shaders") 
target_compile_definitions(sfml-imgui PRIVATE ASC_DATA_PATH="${ASC_DATA_PATH}" GEO_DATA_PATH="${GEO_DATA_PATH}" SHADERS_PATH="${SHADERS_PATH}")

target_include_directories(sfml-imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)