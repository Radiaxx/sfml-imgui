#ifndef GRID_OVERLAY_HPP
#define GRID_OVERLAY_HPP

#include <SFML/Graphics.hpp>

#include "heatmap.hpp"

class GridOverlay
{
public:
    void update(Heatmap& heatmap, sf::RenderWindow& window);
};

#endif