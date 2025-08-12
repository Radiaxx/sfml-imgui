add_executable(sfml-imgui main.cpp app.cpp heatmap.cpp uiManager.cpp)
target_compile_features(sfml-imgui PRIVATE cxx_std_17)

target_link_libraries(sfml-imgui PUBLIC AscParser ImGui-SFML::ImGui-SFML SFML::Graphics)
