#ifndef GRID_OVERLAY_HPP
#define GRID_OVERLAY_HPP

#include <SFML/Graphics.hpp>

#include "heatmap.hpp"

class GridOverlay
{
public:
    void update(Heatmap& heatmap, sf::RenderWindow& window);

private:
    struct VisibleGridRegion
    {
        int   startCol;
        int   endCol;
        int   startRow;
        int   endRow;
        float left;
        float right;
        float top;
        float bottom;
    };

    void drawGridLines(const sf::Sprite& sprite, const sf::View& view, sf::RenderWindow& window, const VisibleGridRegion& region);
    void drawGridValues(const sf::Sprite&                 sprite,
                        const sf::View&                   view,
                        sf::RenderWindow&                 window,
                        const VisibleGridRegion&          region,
                        const int                         cellSize,
                        const std::unique_ptr<AscParser>& data,
                        const AscParser::Header&          header);
};

#endif