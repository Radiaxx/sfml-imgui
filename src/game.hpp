#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

class Game
{
public:
    Game(std::string title, sf::Vector2u initialWindowSize, unsigned int frameRateLimit);
    ~Game();

    void run();

private:
    sf::RenderWindow m_window;
    sf::Clock        m_clock;
    sf::CircleShape  m_shape;

    void handleEvents();
    void update(float deltaTime);
    void render();

    void handleWindowClose();
    void handleWindowResize(const sf::Event::Resized& newSize);
};

#endif