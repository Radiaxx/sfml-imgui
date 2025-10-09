#include <imgui.h>
#include <sstream>

#include "gridOverlay.hpp"

const float GRID_MIN_PX = 30.f;

// Draw an overlay on top of the heatmap when cell size is >= 30x30 pixels
void GridOverlay::draw(Heatmap& heatmap, sf::RenderWindow& window)
{
    const auto& data = heatmap.getAscData();

    if (!data)
    {
        return;
    }

    const sf::Sprite& sprite = heatmap.getHeatmapSprite();
    const sf::View&   view   = window.getView();

    // Get how big is one cell (took cell at (0,0) for convenience) in pixels

    // Sprite local space
    const sf::Vector2f topLeftCellPos    = sprite.getTransform().transformPoint({0.f, 0.f});
    const sf::Vector2f topRightCellPos   = sprite.getTransform().transformPoint({1.f, 0.f});
    const sf::Vector2f bottomLeftCellPos = sprite.getTransform().transformPoint({0.f, 1.f});

    // Screen space
    const sf::Vector2i topLeftScreenPos    = window.mapCoordsToPixel(topLeftCellPos, view);
    const sf::Vector2i topRightScreenPos   = window.mapCoordsToPixel(topRightCellPos, view);
    const sf::Vector2i bottomLeftScreenPos = window.mapCoordsToPixel(bottomLeftCellPos, view);

    const float cellPixelWidth  = std::abs(static_cast<float>(topRightScreenPos.x - topLeftScreenPos.x));
    const float cellPixelHeight = std::abs(static_cast<float>(bottomLeftScreenPos.y - topLeftScreenPos.y));

    if (cellPixelWidth < GRID_MIN_PX || cellPixelHeight < GRID_MIN_PX)
    {
        return;
    }

    const sf::Vector2f  viewCenter = view.getCenter();
    const sf::Vector2f  viewSize   = view.getSize();
    const sf::FloatRect viewRect(viewCenter - viewSize * 0.5f, viewSize);
    const sf::FloatRect spriteRect = sprite.getGlobalBounds();

    // AABB Intersection between viewRect and spriteRect (world coords)
    const sf::Vector2f aMin = spriteRect.position;
    const sf::Vector2f aMax = spriteRect.position + spriteRect.size;
    const sf::Vector2f bMin = viewRect.position;
    const sf::Vector2f bMax = viewRect.position + viewRect.size;

    sf::Vector2f intersectionMin{std::max(aMin.x, bMin.x), std::max(aMin.y, bMin.y)};
    sf::Vector2f intersectionMax{std::min(aMax.x, bMax.x), std::min(aMax.y, bMax.y)};

    // No overlap
    if (intersectionMax.x <= intersectionMin.x || intersectionMax.y <= intersectionMin.y)
    {
        return;
    }

    sf::FloatRect intersectionRect(intersectionMin, intersectionMax - intersectionMin);

    // Get intersected (visible) sprite local coords (texture space)
    const sf::Transform inv              = sprite.getInverseTransform();
    const sf::Vector2f  topLeftLocal     = inv.transformPoint(intersectionRect.position);
    const sf::Vector2f  bottomRightLocal = inv.transformPoint(intersectionRect.position + intersectionRect.size);

    // TODO: Local coords might not be ordered if there is a negative scale. Assume that there will never be a negative scale
    const float left   = topLeftLocal.x;
    const float right  = bottomRightLocal.x;
    const float top    = topLeftLocal.y;
    const float bottom = bottomRightLocal.y;

    if (right <= left || bottom <= top)
    {
        return;
    }

    const auto& header = data->getHeader();
    const int   ncols  = header.ncols;
    const int   nrows  = header.nrows;

    int startCol = std::max(0, static_cast<int>(std::floor(left)));
    int endCol   = std::min(ncols, static_cast<int>(std::ceil(right)));
    int startRow = std::max(0, static_cast<int>(std::floor(top)));
    int endRow   = std::min(nrows, static_cast<int>(std::ceil(bottom)));

    if (startCol >= endCol || startRow >= endRow)
    {
        return;
    }

    const VisibleGridRegion region{
        startCol,
        endCol,
        startRow,
        endRow,
        left,
        right,
        top,
        bottom,
    };

    drawGridLines(sprite, view, window, region);
    drawGridValues(sprite, view, window, region, cellPixelWidth, data, header);
}

void GridOverlay::setShowValues(bool enabled)
{
    m_isShowingValues = enabled;
}

bool GridOverlay::isShowingValues() const
{
    return m_isShowingValues;
}

void GridOverlay::drawGridLines(const sf::Sprite&        sprite,
                                const sf::View&          view,
                                sf::RenderWindow&        window,
                                const VisibleGridRegion& region)
{
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    const ImU32 color    = IM_COL32(0, 0, 0, 255);

    // Vertical lines on grid x. The column edges
    for (int col = region.startCol; col <= region.endCol; ++col)
    {
        const sf::Vector2f worldTop    = sprite.getTransform().transformPoint({static_cast<float>(col), region.top});
        const sf::Vector2f worldBottom = sprite.getTransform().transformPoint({static_cast<float>(col), region.bottom});
        const sf::Vector2i screenTop   = window.mapCoordsToPixel(worldTop, view);
        const sf::Vector2i screenBottom = window.mapCoordsToPixel(worldBottom, view);

        const ImVec2 start = ImVec2(static_cast<float>(screenTop.x), static_cast<float>(screenTop.y));
        const ImVec2 end   = ImVec2(static_cast<float>(screenBottom.x), static_cast<float>(screenBottom.y));
        drawList->AddLine(start, end, color);
    }

    // Horizontal lines on grid y. The row edges
    for (int row = region.startRow; row <= region.endRow; ++row)
    {
        const sf::Vector2f worldLeft   = sprite.getTransform().transformPoint({region.left, static_cast<float>(row)});
        const sf::Vector2f worldRight  = sprite.getTransform().transformPoint({region.right, static_cast<float>(row)});
        const sf::Vector2i screenLeft  = window.mapCoordsToPixel(worldLeft, view);
        const sf::Vector2i screenRight = window.mapCoordsToPixel(worldRight, view);

        const ImVec2 start = ImVec2(static_cast<float>(screenLeft.x), static_cast<float>(screenLeft.y));
        const ImVec2 end   = ImVec2(static_cast<float>(screenRight.x), static_cast<float>(screenRight.y));
        drawList->AddLine(start, end, color);
    }
}

void GridOverlay::drawGridValues(
    const sf::Sprite&                 sprite,
    const sf::View&                   view,
    sf::RenderWindow&                 window,
    const VisibleGridRegion&          region,
    const int                         cellSize,
    const std::unique_ptr<AscParser>& data,
    const AscParser::Header&          header)
{
    if (!m_isShowingValues)
    {
        return;
    }

    ImDrawList* dl      = ImGui::GetBackgroundDrawList();
    ImFont*     font    = ImGui::GetFont();
    const float padding = 2.f;

    // Values to keep it readable. Currently eye balled
    const int fontSize = 13;
    int       decimals;

    if (cellSize >= 95.f)
    {
        decimals = 3;
    }
    else if (cellSize >= 75.f)
    {
        decimals = 2;
    }
    else if (cellSize >= 55.f)
    {
        decimals = 1;
    }
    else
    {
        decimals = 0;
    }

    std::ostringstream oss;
    oss.setf(std::ios::fixed);

    // For each visible cell draw his value centered in the cell
    for (int row = region.startRow; row < region.endRow; ++row)
    {
        for (int col = region.startCol; col < region.endCol; ++col)
        {
            const float value = data->getData().at(row * header.ncols + col);

            if (value == header.nodata_value)
            {
                continue;
            }

            // Use the same stringstream to avoid per cell allocations
            oss.str({});
            oss.clear();
            oss << std::setprecision(decimals) << value;
            const std::string valueString = oss.str();

            const sf::Vector2f localCenter(static_cast<float>(col) + 0.5f, static_cast<float>(row) + 0.5f);
            const sf::Vector2f worldCenter  = sprite.getTransform().transformPoint(localCenter);
            const sf::Vector2i screenCenter = window.mapCoordsToPixel(worldCenter, view);
            const sf::Vector2f pos(static_cast<float>(screenCenter.x), static_cast<float>(screenCenter.y));

            const ImVec2       textSizeImGui = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, valueString.c_str());
            const sf::Vector2f textSize(textSizeImGui.x, textSizeImGui.y);
            const sf::Vector2f anchor(std::round(pos.x - textSize.x * 0.5f), std::round(pos.y - textSize.y * 0.5f));
            const ImVec2       anchorRounded(std::round(anchor.x), std::round(anchor.y));

            const ImVec2 bottomLeftRect = ImVec2(anchorRounded.x - padding, anchorRounded.y - padding);
            const ImVec2 topRightRect = ImVec2(anchorRounded.x + textSize.x + padding, anchorRounded.y + textSize.y + padding);

            dl->AddRectFilled(bottomLeftRect, topRightRect, ImGui::GetColorU32(ImGuiCol_WindowBg, 0.65f), 2.0f);
            dl->AddText(font, fontSize, anchorRounded, ImGui::GetColorU32(ImGuiCol_Text, 1.f), valueString.c_str());
        }
    }
}