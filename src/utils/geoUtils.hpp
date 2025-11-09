#ifndef GEO_UTILS_HPP
#define GEO_UTILS_HPP

#include <SFML/Graphics.hpp>

#include "ascParser.hpp"
#include "geoCsvParser.hpp"

namespace GeoUtils
{
// Converts a WKT point in world coordinates to local sprite coordinates
sf::Vector2f wktToLocal(const GeoCsvParser::Point& point, const AscParser::Header& header);

// Computes the AABB intersection between a view rectangle and a sprite rectangle in world coordinates.
// outIntersection is the output parameter. The intersection rectangle which is only valid if function returns true.
// Returns true if there is an intersection, false if no overlap
bool computeViewSpriteIntersection(const sf::View& view, const sf::Sprite& sprite, sf::FloatRect& outIntersection);

struct VisibleArea
{
    float left;
    float right;
    float top;
    float bottom;
    bool  isValid;
};

// Computes the visible area of a sprite in local sprite coordinates (texture space).
// Returns visibleArea structure with local coordinates and validity flag
VisibleArea getVisibleAreaInLocalCoords(const sf::View& view, const sf::Sprite& sprite);
} // namespace GeoUtils

#endif
