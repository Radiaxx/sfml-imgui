#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

#include "cellTooltip.hpp"
#include "geoData.hpp"
#include "gridOverlay.hpp"
#include "heatmap.hpp"

class UIManager
{
public:
    UIManager(sf::View view) : m_view(view) {};

    void draw(Heatmap& heatmap, GeoData& geoData, CellTooltip& cellTooltip, GridOverlay& gridOverlay);

    void setView(sf::View view);

private:
    sf::View m_view;
};

#endif