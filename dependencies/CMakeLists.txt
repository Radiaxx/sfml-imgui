include(FetchContent)

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

set(IMGUI_CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/config/imguiConfig.h")
target_include_directories(ImGui-SFML INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/config>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(ImGui-SFML PUBLIC IMGUI_CONFIG=\"${IMGUI_CONFIG_PATH}\")
