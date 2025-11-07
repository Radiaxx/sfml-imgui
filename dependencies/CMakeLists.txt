include(FetchContent)

# Allow ImGui to use more than 64k vertices (needed when showing text for each grid cell)
add_compile_definitions("ImDrawIdx=unsigned int") 

FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML
  GIT_TAG 3.0.1
)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.91.9
)

FetchContent_Declare(   
  imgui-sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_TAG v3.0
)

option(SFML_BUILD_AUDIO "Build audio" OFF)
option(SFML_BUILD_NETWORK "Build network" OFF)
FetchContent_MakeAvailable(SFML)

FetchContent_MakeAvailable(imgui)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO ON)
FetchContent_MakeAvailable(imgui-sfml)

FetchContent_Declare(
  boost_headers
  URL      https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-b2-nodocs.tar.gz
  URL_HASH SHA256=aa25e7b9c227c21abb8a681efd4fe6e54823815ffc12394c9339de998eb503fb
)

FetchContent_MakeAvailable(boost_headers)

add_library(BoostHeaders INTERFACE)
target_include_directories(BoostHeaders INTERFACE
  "${boost_headers_SOURCE_DIR}")
