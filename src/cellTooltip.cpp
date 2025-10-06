#include <imgui.h>

#include "cellTooltip.hpp"

const ImVec2 TOOLTIP_OFFSET(14.f, 18.f);

void CellTooltip::update(Heatmap& heatmap, sf::RenderWindow& window)
{
    if (!m_isVisible || !heatmap.getAscData())
    {
        return;
    }

    if (ImGui::GetIO().WantCaptureMouse || ImGui::IsKeyPressed(ImGuiKey_Escape, false))
    {
        hide();
        return;
    }

    const sf::Sprite&  sprite         = heatmap.getHeatmapSprite();
    const sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
    const sf::Vector2f worldPos       = window.mapPixelToCoords(mouseScreenPos, window.getView());
    const sf::Vector2f localPos       = sprite.getInverseTransform().transformPoint(worldPos);

    // Check if the mouse is still inside the selected cell bounds
    // Cell bounds are in local sprite space but the computation needs to be done in update()
    // This way if the view pans/zooms the tooltip will still be correctly positioned
    const float left   = static_cast<float>(m_data.col);
    const float right  = left + 1.f;
    const float top    = static_cast<float>(m_data.row);
    const float bottom = top + 1.f;

    const bool insideLocal = (localPos.x >= left && localPos.x <= right && localPos.y >= top && localPos.y <= bottom);

    if (!insideLocal)
    {
        hide();
        return;
    }

    // Draw a screen space highlight of the selected cell
    const sf::Transform& spriteTransform  = sprite.getTransform();
    const sf::Vector2f   topLeftWorld     = spriteTransform.transformPoint({left, top});
    const sf::Vector2f   bottomRightWorld = spriteTransform.transformPoint({right, bottom});

    const sf::Vector2i topLeftPixel     = window.mapCoordsToPixel(topLeftWorld, window.getView());
    const sf::Vector2i bottomRightPixel = window.mapCoordsToPixel(bottomRightWorld, window.getView());

    ImGui::GetBackgroundDrawList()->AddRect(ImVec2(topLeftPixel.x, topLeftPixel.y),
                                            ImVec2(bottomRightPixel.x, bottomRightPixel.y),
                                            ImGui::GetColorU32(ImGuiCol_Text));

    // Draw the tooltip near the mouse cursor
    const ImVec2 mouseIm = ImGui::GetMousePos();
    const ImVec2 tooltipPos(mouseIm.x + TOOLTIP_OFFSET.x, mouseIm.y + TOOLTIP_OFFSET.y);
    ImGui::SetNextWindowPos(tooltipPos, ImGuiCond_Always);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs;

    if (ImGui::Begin("Cell Tooltip", nullptr, flags))
    {
        ImGui::Text("Cell: (%d, %d)", m_data.col, m_data.row);

        if (m_data.hasValue)
        {
            ImGui::Text("Value: %.6g", m_data.value);
        }
        else
        {
            ImGui::TextDisabled("Value: {no data}");
        }
    }

    ImGui::End();
}

void CellTooltip::handleMouseWheelScrolled(const sf::Event::MouseWheelScrolled& event)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    hide();
}


void CellTooltip::handleMouseButtonPressed(const sf::Event::MouseButtonPressed& event, Heatmap& heatmap, sf::RenderWindow& window)
{
    if (event.button != sf::Mouse::Button::Left || ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    show(heatmap, window, event.position.x, event.position.y);
}

void CellTooltip::show(Heatmap& heatmap, sf::RenderWindow& window, int screenX, int screenY)
{
    if (!heatmap.getAscData())
    {
        return;
    }

    const sf::Sprite&  sprite = heatmap.getHeatmapSprite();
    const sf::Vector2i pixelPos(screenX, screenY);
    const sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, window.getView());
    const sf::Vector2f localPos = sprite.getInverseTransform().transformPoint(worldPos);

    const int col = static_cast<int>(std::floor(localPos.x));
    const int row = static_cast<int>(std::floor(localPos.y));

    const auto& asc        = heatmap.getAscData();
    const auto& header     = asc->getHeader();
    const int   ncols      = header.ncols;
    const int   nrows      = header.nrows;
    const int   clampedCol = std::clamp(col, 0, ncols - 1);
    const int   clampedRow = std::clamp(row, 0, nrows - 1);

    const std::size_t index    = static_cast<std::size_t>(clampedRow) * ncols + clampedCol;
    const double      value    = asc->getData()[index];
    const bool        hasValue = (value != header.nodata_value);

    m_data.col      = clampedCol;
    m_data.row      = clampedRow;
    m_data.value    = value;
    m_data.hasValue = hasValue;
    m_isVisible     = true;
}

void CellTooltip::hide()
{
    m_isVisible = false;
}
