#include <imgui.h>

#include "ViewControls.hpp"

ViewControls::ViewControls() : m_isPanning(false), m_lastMousePos(0, 0)
{
}

void ViewControls::handleMouseWheelScrolled(const sf::Event::MouseWheelScrolled& event, sf::RenderWindow& window)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    sf::View view = window.getView();

    // Zoom at the cursor position
    sf::Vector2i mousePixelPos      = {event.position.x, event.position.y};
    sf::Vector2f worldPosBeforeZoom = window.mapPixelToCoords(mousePixelPos, view);

    const float zoomFactor = (event.delta > 0) ? 0.9f : 1.1f;
    view.zoom(zoomFactor);

    sf::Vector2f worldPosAfterZoom = window.mapPixelToCoords(mousePixelPos, view);
    view.move(worldPosBeforeZoom - worldPosAfterZoom);
    window.setView(view);
}

void ViewControls::handleMouseButtonPressed(const sf::Event::MouseButtonPressed& event)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (event.button == sf::Mouse::Button::Left)
    {
        m_isPanning    = true;
        m_lastMousePos = {event.position.x, event.position.y};
    }
}

void ViewControls::handleMouseButtonReleased(const sf::Event::MouseButtonReleased& event)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (event.button == sf::Mouse::Button::Left)
    {
        m_isPanning = false;
    }
}

void ViewControls::handleMouseMoved(const sf::Event::MouseMoved& event, sf::RenderWindow& window)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (m_isPanning)
    {
        sf::View view = window.getView();

        const sf::Vector2i newMousePos = {event.position.x, event.position.y};
        const sf::Vector2f delta = window.mapPixelToCoords(m_lastMousePos, view) - window.mapPixelToCoords(newMousePos, view);
        view.move(delta);
        window.setView(view);
        m_lastMousePos = newMousePos;
    }
}
