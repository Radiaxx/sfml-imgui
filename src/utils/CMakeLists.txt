add_library(AscParser STATIC
    AscParser.cpp
    AscParser.hpp
)

target_include_directories(AscParser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)