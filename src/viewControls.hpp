#ifndef CAMERA_CONTROLS_HPP
#define CAMERA_CONTROLS_HPP

#include <SFML/Graphics.hpp>

class ViewControls
{
public:
    ViewControls();

    void handleMouseWheelScrolled(const sf::Event::MouseWheelScrolled& event, sf::RenderWindow& window);
    void handleMouseButtonPressed(const sf::Event::MouseButtonPressed& event);
    void handleMouseButtonReleased(const sf::Event::MouseButtonReleased& event);
    void handleMouseMoved(const sf::Event::MouseMoved& event, sf::RenderWindow& window);

private:
    bool         m_isPanning;
    sf::Vector2i m_lastMousePos;
};

#endif