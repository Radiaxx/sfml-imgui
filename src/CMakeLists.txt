add_executable(sfml-imgui main.cpp game.cpp)
target_compile_features(sfml-imgui PRIVATE cxx_std_17)

target_link_libraries(sfml-imgui PUBLIC ImGui-SFML::ImGui-SFML SFML::Graphics)
