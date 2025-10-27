add_library(AscParser STATIC
    ascParser.cpp
    ascParser.hpp
)

target_compile_features(AscParser PRIVATE cxx_std_17)
target_include_directories(AscParser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(GeoCsvParser STATIC
    geoCsvParser.cpp
    geoCsvParser.hpp
    wktParser.cpp
    wktParser.hpp
)

target_compile_features(GeoCsvParser PRIVATE cxx_std_17)
target_include_directories(GeoCsvParser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)