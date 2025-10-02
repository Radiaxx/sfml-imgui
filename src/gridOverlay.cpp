#include <imgui.h>

#include "gridOverlay.hpp"

const float GRID_MIN_PX = 30.f;

// Draw an overlay on top of the heatmap when cell size is >= 30x30 pixels
void GridOverlay::update(Heatmap& heatmap, sf::RenderWindow& window)
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

    if (startCol >= endCol || startRow >= endRow)
    {
        return;
    }

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    const ImU32 color    = IM_COL32(0, 0, 0, 255);

    // Vertical lines on grid x. The column edges
    for (int col = startCol; col <= endCol; ++col)
    {
        const sf::Vector2f worldTop     = sprite.getTransform().transformPoint({static_cast<float>(col), top});
        const sf::Vector2f worldBottom  = sprite.getTransform().transformPoint({static_cast<float>(col), bottom});
        const sf::Vector2i screenTop    = window.mapCoordsToPixel(worldTop, view);
        const sf::Vector2i screenBottom = window.mapCoordsToPixel(worldBottom, view);

        const ImVec2 start = ImVec2(static_cast<float>(screenTop.x), static_cast<float>(screenTop.y));
        const ImVec2 end   = ImVec2(static_cast<float>(screenBottom.x), static_cast<float>(screenBottom.y));
        drawList->AddLine(start, end, color);
    }

    // Horizontal lines on grid y. The row edges
    for (int row = startRow; row <= endRow; ++row)
    {
        const sf::Vector2f worldLeft   = sprite.getTransform().transformPoint({left, static_cast<float>(row)});
        const sf::Vector2f worldRight  = sprite.getTransform().transformPoint({right, static_cast<float>(row)});
        const sf::Vector2i screenLeft  = window.mapCoordsToPixel(worldLeft, view);
        const sf::Vector2i screenRight = window.mapCoordsToPixel(worldRight, view);

        const ImVec2 start = ImVec2(static_cast<float>(screenLeft.x), static_cast<float>(screenLeft.y));
        const ImVec2 end   = ImVec2(static_cast<float>(screenRight.x), static_cast<float>(screenRight.y));
        drawList->AddLine(start, end, color);
    }
}