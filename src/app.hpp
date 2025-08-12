#ifndef APP_HPP
#define APP_HPP

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

#include "UIManager.hpp"
#include "heatmap.hpp"
#include "utils/AscParser.hpp"

class App
{
public:
    struct Config
    {
        std::string  title;
        sf::Vector2u initialWindowSize;
        unsigned int frameRateLimit;
    };

    App(const Config& config);
    ~App();

    void run();

private:
    sf::RenderWindow m_window;
    sf::Clock        m_clock;

    Heatmap   m_heatmap;
    UIManager m_uiManager;

    void handleEvents();
    void update(float deltaTime);
    void render();

    void handleWindowClose();
    void handleWindowResize(const sf::Event::Resized& newSize);
};

#endif