#ifndef CELL_TOOLTIP_HPP
#define CELL_TOOLTIP_HPP

#include <SFML/Graphics.hpp>

#include "heatmap.hpp"

class CellTooltip
{
public:
    struct TooltipData
    {
        int    col      = 0;
        int    row      = 0;
        double value    = 0.0;
        bool   hasValue = false;
    };

    void update(Heatmap& heatmap, sf::RenderWindow& window);

    void handleMouseWheelScrolled(const sf::Event::MouseWheelScrolled& event);
    void handleMouseButtonPressed(const sf::Event::MouseButtonPressed& event, Heatmap& heatmap, sf::RenderWindow& window);

    void show(Heatmap& heatmap, sf::RenderWindow& window, int screenX, int screenY);
    void hide();

private:
    bool        m_isVisible = false;
    TooltipData m_data;
};

#endif