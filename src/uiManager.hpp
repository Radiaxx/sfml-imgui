#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

#include "heatmap.hpp"

class UIManager
{
public:
    UIManager(sf::View view) : m_view(view) {};

    void update(Heatmap& heatmap);

    void setView(sf::View view);

private:
    sf::View m_view;
};

#endif