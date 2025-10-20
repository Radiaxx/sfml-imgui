add_library(AscParser STATIC
    AscParser.cpp
    AscParser.hpp
)

target_include_directories(AscParser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(GeoCsvParser STATIC
    GeoCsvParser.cpp
    GeoCsvParser.hpp
    WktParser.cpp
    WktParser.hpp
)

target_include_directories(GeoCsvParser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)