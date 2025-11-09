#include "geoUtils.hpp"

namespace GeoUtils
{
sf::Vector2f wktToLocal(const GeoCsvParser::Point& point, const AscParser::Header& header)
{
    const double cellSize = header.cellsize;
    const double xLocal   = (point.x - header.xllcorner) / cellSize;
    const double yTop     = header.yllcorner + static_cast<double>(header.nrows) * cellSize;
    const double yLocal   = (yTop - point.y) / cellSize; // must flip Y to match sprite top left origin

    return {static_cast<float>(xLocal), static_cast<float>(yLocal)};
}

bool computeViewSpriteIntersection(const sf::View& view, const sf::Sprite& sprite, sf::FloatRect& outIntersection)
{
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

    // Check for no overlap
    if (intersectionMax.x <= intersectionMin.x || intersectionMax.y <= intersectionMin.y)
    {
        return false;
    }

    outIntersection = sf::FloatRect(intersectionMin, intersectionMax - intersectionMin);

    return true;
}

VisibleArea getVisibleAreaInLocalCoords(const sf::View& view, const sf::Sprite& sprite)
{
    VisibleArea result{};

    sf::FloatRect intersectionRect;

    if (!computeViewSpriteIntersection(view, sprite, intersectionRect))
    {
        result.isValid = false;
        return result;
    }

    // Get intersected (visible) sprite local coords (texture space)
    const sf::Transform inv              = sprite.getInverseTransform();
    const sf::Vector2f  topLeftLocal     = inv.transformPoint(intersectionRect.position);
    const sf::Vector2f  bottomRightLocal = inv.transformPoint(intersectionRect.position + intersectionRect.size);

    // TODO: Local coords might not be ordered if there is a negative scale. Assume that there will never be a negative scale
    result.left   = topLeftLocal.x;
    result.right  = bottomRightLocal.x;
    result.top    = topLeftLocal.y;
    result.bottom = bottomRightLocal.y;

    // Validate the bounds
    result.isValid = (result.right > result.left && result.bottom > result.top);

    return result;
}
} // namespace GeoUtils
