#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <limits>

#include "cellTooltip.hpp"

using namespace boost::geometry;
namespace bgi = boost::geometry::index;

static const ImVec2 TOOLTIP_OFFSET(14.f, 18.f);

void CellTooltip::draw(Heatmap& heatmap, sf::RenderWindow& window)
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

    ImGui::GetBackgroundDrawList()
        ->AddRect(ImVec2(static_cast<float>(topLeftPixel.x), static_cast<float>(topLeftPixel.y)),
                  ImVec2(static_cast<float>(bottomRightPixel.x), static_cast<float>(bottomRightPixel.y)),
                  ImGui::GetColorU32(ImGuiCol_Text));

    // Draw the tooltip near the mouse cursor
    const ImVec2 mouseIm = ImGui::GetMousePos();
    const ImVec2 tooltipPos(mouseIm.x + TOOLTIP_OFFSET.x, mouseIm.y + TOOLTIP_OFFSET.y);
    ImGui::SetNextWindowPos(tooltipPos, ImGuiCond_Always);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs;

    if (ImGui::Begin("Cell Tooltip", nullptr, flags))
    {
        // Cell data
        ImGui::Text("Cell: (%d, %d)", m_data.col, m_data.row);

        if (m_data.hasValue)
        {
            ImGui::Text("Value: %.6g", m_data.value);
        }
        else
        {
            ImGui::TextDisabled("Value: {no data}");
        }

        // Geo pick data
        if (m_geoPickEntity)
        {
            ImGui::Separator();
            ImGui::TextDisabled("Geo:");
            ImGui::Text("id: %u", m_geoPickEntity->id);
            ImGui::Text("name: %s", m_geoPickEntity->name.c_str());

            const char* typeStr = "";
            switch (m_geoPickEntity->type)
            {
                case GeoCsvParser::EntityType::Maximum:
                    typeStr = "MAXIMUM";
                    break;
                case GeoCsvParser::EntityType::Minimum:
                    typeStr = "MINIMUM";
                    break;
                case GeoCsvParser::EntityType::Saddle:
                    typeStr = "SADDLE";
                    break;
                case GeoCsvParser::EntityType::LineAscending:
                    typeStr = "LINE-ASCENDING";
                    break;
                case GeoCsvParser::EntityType::LineDescending:
                    typeStr = "LINE-DESCENDING";
                    break;
                case GeoCsvParser::EntityType::Area:
                    typeStr = "AREA";
                    break;
                default:
                    typeStr = "UNKNOWN";
                    break;
            }

            ImGui::Text("type: %s", typeStr);
            ImGui::Text("life: %.3f", m_geoPickEntity->life);

            if (!m_geoPickEntity->misc.empty())
            {
                ImGui::Text("misc: %s", m_geoPickEntity->misc.c_str());
            }
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

void CellTooltip::handleMouseButtonPressed(const sf::Event::MouseButtonPressed& event,
                                           Heatmap&                             heatmap,
                                           const GeoData&                       geoData,
                                           sf::RenderWindow&                    window)
{
    if (event.button != sf::Mouse::Button::Left || ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    show(heatmap, window, geoData, event.position.x, event.position.y);
}

void CellTooltip::show(Heatmap& heatmap, sf::RenderWindow& window, const GeoData& geoData, int screenX, int screenY)
{
    if (!heatmap.getAscData())
    {
        return;
    }

    const sf::Sprite&  sprite   = heatmap.getHeatmapSprite();
    const sf::Vector2i pixelPos = {screenX, screenY};
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

    performPick(pixelPos, window, heatmap, geoData);
}

void CellTooltip::hide()
{
    m_isVisible     = false;
    m_geoPickEntity = nullptr;
}

void CellTooltip::rebuildSpatialIndex(const Heatmap& heatmap, const GeoData& geoData)
{
    m_points       = PointsRTree{};
    m_lineSegsAsc  = SegmentsRTree{};
    m_lineSegsDesc = SegmentsRTree{};
    m_areaSegs     = SegmentsRTree{};

    if (!geoData.getGeoData() || !heatmap.getAscData())
    {
        return;
    }

    const std::size_t pointCount = geoData.maximum().size() + geoData.minimum().size() + geoData.saddles().size();
    const std::size_t lineAscSegmentCount  = geoData.countLineSegmentsAscending();
    const std::size_t lineDescSegmentCount = geoData.countLineSegmentsDescending();
    const std::size_t areaSegmentCount     = geoData.countAreaSegments();

    std::vector<PointValue> points;
    points.reserve(pointCount);

    std::vector<SegmentValue> lineSegmentsAscending;
    lineSegmentsAscending.reserve(lineAscSegmentCount);

    std::vector<SegmentValue> lineSegmentsDescending;
    lineSegmentsDescending.reserve(lineDescSegmentCount);

    std::vector<SegmentValue> areaSegments;
    areaSegments.reserve(areaSegmentCount);

    collectPoints(heatmap, geoData, points);
    collectLineSegmentsAscending(heatmap, geoData, lineSegmentsAscending);
    collectLineSegmentsDescending(heatmap, geoData, lineSegmentsDescending);
    collectAreaSegments(heatmap, geoData, areaSegments);

    m_points       = PointsRTree(points.begin(), points.end());
    m_lineSegsAsc  = SegmentsRTree(lineSegmentsAscending.begin(), lineSegmentsAscending.end());
    m_lineSegsDesc = SegmentsRTree(lineSegmentsDescending.begin(), lineSegmentsDescending.end());
    m_areaSegs     = SegmentsRTree(areaSegments.begin(), areaSegments.end());
}

sf::Vector2f CellTooltip::wktToLocal(const GeoCsvParser::Point& point, const AscParser::Header& header)
{
    const double cellSize = header.cellsize;
    const double xLocal   = (point.x - header.xllcorner) / cellSize;
    const double yTop     = header.yllcorner + static_cast<double>(header.nrows) * cellSize;
    const double yLocal   = (yTop - point.y) / cellSize;

    return {static_cast<float>(xLocal), static_cast<float>(yLocal)};
}

CellTooltip::BBox CellTooltip::getLocalPickBBox(const sf::Vector2i      mousePixel,
                                                float                   radiusPixel,
                                                const sf::RenderWindow& window,
                                                const Heatmap&          heatmap)
{
    const sf::View view = window.getView();

    const sf::Vector2i aPixel(mousePixel.x - static_cast<int>(radiusPixel), mousePixel.y - static_cast<int>(radiusPixel));
    const sf::Vector2i bPixel(mousePixel.x + static_cast<int>(radiusPixel), mousePixel.y + static_cast<int>(radiusPixel));

    const sf::Vector2f aWorld = window.mapPixelToCoords(aPixel, view);
    const sf::Vector2f bWorld = window.mapPixelToCoords(bPixel, view);

    const sf::Transform inverse = heatmap.getHeatmapSprite().getInverseTransform();
    const sf::Vector2f  aLocal  = inverse.transformPoint(aWorld);
    const sf::Vector2f  bLocal  = inverse.transformPoint(bWorld);

    const float xmin = std::min(aLocal.x, bLocal.x);
    const float xmax = std::max(aLocal.x, bLocal.x);
    const float ymin = std::min(aLocal.y, bLocal.y);
    const float ymax = std::max(aLocal.y, bLocal.y);

    return BBox(BPoint(xmin, ymin), BPoint(xmax, ymax));
}

float CellTooltip::pixelDistance(const sf::Vector2i a, const sf::Vector2i b) const
{
    const int dx = a.x - b.x;
    const int dy = a.y - b.y;

    return std::sqrt(static_cast<float>(dx * dx + dy * dy));
}

sf::Vector2i CellTooltip::localToPixel(const sf::Vector2f local, const sf::RenderWindow& window, const Heatmap& heatmap) const
{
    const sf::Vector2f world = heatmap.getHeatmapSprite().getTransform().transformPoint(local);
    const sf::Vector2i px    = window.mapCoordsToPixel(world);

    return px;
}

// Implementation reference at https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
float CellTooltip::pixelDistanceToSegment(const sf::Vector2i mousePixel,
                                          const sf::Vector2i segmentStart,
                                          const sf::Vector2i segmentEnd) const
{
    const float segmentDeltaX = static_cast<float>(segmentEnd.x - segmentStart.x);
    const float segmentDeltaY = static_cast<float>(segmentEnd.y - segmentStart.y);
    const float mouseToStartX = static_cast<float>(mousePixel.x - segmentStart.x);
    const float mouseToStartY = static_cast<float>(mousePixel.y - segmentStart.y);

    const float segmentLengthSq   = segmentDeltaX * segmentDeltaX + segmentDeltaY * segmentDeltaY;
    const float distanceThreshold = 1e-6f;

    // If the segment is very short then treat it as a point
    if (segmentLengthSq <= distanceThreshold)
    {
        return pixelDistance(mousePixel, segmentStart);
    }

    float projectionParameter = (mouseToStartX * segmentDeltaX + mouseToStartY * segmentDeltaY) / segmentLengthSq;
    projectionParameter       = std::clamp(projectionParameter, 0.0f, 1.0f);

    const float closestPointX = segmentStart.x + projectionParameter * segmentDeltaX;
    const float closestPointY = segmentStart.y + projectionParameter * segmentDeltaY;

    const float deltaX = static_cast<float>(mousePixel.x) - closestPointX;
    const float deltaY = static_cast<float>(mousePixel.y) - closestPointY;

    return std::sqrt(deltaX * deltaX + deltaY * deltaY);
}

void CellTooltip::collectPoints(const Heatmap& heatmap, const GeoData& geoData, std::vector<PointValue>& out)
{
    out.clear();

    if (!geoData.getGeoData() || !heatmap.getAscData())
    {
        return;
    }

    const auto& header   = heatmap.getAscData()->getHeader();
    const auto& entities = geoData.getGeoData()->getEntities();

    for (const auto& entity : entities)
    {
        if (entity.type != GeoCsvParser::EntityType::Maximum && entity.type != GeoCsvParser::EntityType::Minimum &&
            entity.type != GeoCsvParser::EntityType::Saddle)
        {
            continue;
        }

        if (entity.geom.type != GeoCsvParser::GeometryType::Point)
        {
            continue;
        }

        const sf::Vector2f localPosition = wktToLocal(entity.geom.point, header);

        out.emplace_back(BPoint(localPosition.x, localPosition.y), &entity);
    }
}

void CellTooltip::addSegmentsForOpenPath(const std::vector<GeoCsvParser::Point>& points,
                                         const AscParser::Header&                header,
                                         const GeoCsvParser::Entity*             owner,
                                         std::vector<CellTooltip::SegmentValue>& out)
{
    if (points.size() < 2)
    {
        return;
    }

    for (size_t i = 0; i + 1 < points.size(); ++i)
    {
        const sf::Vector2f a = wktToLocal(points[i], header);
        const sf::Vector2f b = wktToLocal(points[i + 1], header);

        out.emplace_back(BSegment(BPoint(a.x, a.y), BPoint(b.x, b.y)), owner);
    }
}

void CellTooltip::addSegmentsForClosedRing(const std::vector<GeoCsvParser::Point>& ring,
                                           const AscParser::Header&                header,
                                           const GeoCsvParser::Entity*             owner,
                                           std::vector<CellTooltip::SegmentValue>& out)
{
    const size_t ringSize = ring.size();

    if (ringSize < 2)
    {
        return;
    }

    for (size_t i = 0; i < ringSize; ++i)
    {
        const size_t       j = (i + 1) % ringSize;
        const sf::Vector2f a = wktToLocal(ring[i], header);
        const sf::Vector2f b = wktToLocal(ring[j], header);

        out.emplace_back(BSegment(BPoint(a.x, a.y), BPoint(b.x, b.y)), owner);
    }
}

void CellTooltip::collectLineSegmentsAscending(const Heatmap& heatmap, const GeoData& geoData, std::vector<SegmentValue>& out)
{
    out.clear();

    if (!geoData.getGeoData() || !heatmap.getAscData())
    {
        return;
    }

    const auto& header   = heatmap.getAscData()->getHeader();
    const auto& entities = geoData.getGeoData()->getEntities();

    for (const auto& entity : entities)
    {
        if (entity.type != GeoCsvParser::EntityType::LineAscending)
        {
            continue;
        }

        for (const auto& lineString : entity.geom.lines)
        {
            addSegmentsForOpenPath(lineString.points, header, &entity, out);
        }
    }
}

void CellTooltip::collectLineSegmentsDescending(const Heatmap& heatmap, const GeoData& geoData, std::vector<SegmentValue>& out)
{
    out.clear();

    if (!geoData.getGeoData() || !heatmap.getAscData())
    {
        return;
    }

    const auto& header   = heatmap.getAscData()->getHeader();
    const auto& entities = geoData.getGeoData()->getEntities();

    for (const auto& entity : entities)
    {
        if (entity.type != GeoCsvParser::EntityType::LineDescending)
        {
            continue;
        }

        for (const auto& lineString : entity.geom.lines)
        {
            addSegmentsForOpenPath(lineString.points, header, &entity, out);
        }
    }
}

void CellTooltip::collectAreaSegments(const Heatmap& heatmap, const GeoData& geoData, std::vector<SegmentValue>& out)
{
    out.clear();

    if (!geoData.getGeoData() || !heatmap.getAscData())
    {
        return;
    }

    const auto& header   = heatmap.getAscData()->getHeader();
    const auto& entities = geoData.getGeoData()->getEntities();

    for (const auto& entity : entities)
    {
        if (entity.type != GeoCsvParser::EntityType::Area)
        {
            continue;
        }

        bool anyPolygon = false;
        for (const auto& polygon : entity.geom.polygons)
        {
            anyPolygon = true;
            for (const auto& ring : polygon.rings)
            {
                addSegmentsForClosedRing(ring.points, header, &entity, out);
            }
        }
        if (!anyPolygon)
        {
            // If multiLineString (?)
            for (const auto& lineString : entity.geom.lines)
            {
                addSegmentsForOpenPath(lineString.points, header, &entity, out);
            }
        }
    }
}

template <typename TreeT, typename ValueT>
void CellTooltip::pickFromTree(
    const TreeT&                 tree,
    const BBox&                  localBox,
    const sf::RenderWindow&      window,
    const Heatmap&               heatmap,
    const GeoData&               geoData,
    const sf::Vector2i           mousePixel,
    const GeoCsvParser::Entity*& bestEntity,
    float&                       bestDistancePixel) const
{
    std::vector<ValueT> candidates;
    tree.query(bgi::intersects(localBox), std::back_inserter(candidates));

    const double lifeMin = geoData.getLifeFilterMin();
    const double lifeMax = geoData.getLifeFilterMax();

    for (const auto& value : candidates)
    {
        const auto* entity = value.second;

        if (!entity)
        {
            continue;
        }

        // Filter by current life range
        if (entity->life < lifeMin || entity->life > lifeMax)
        {
            continue;
        }

        float distanceInPixels = 0.f;

        if constexpr (std::is_same_v<ValueT, PointValue>)
        {
            bool isEntityVisible = false;

            switch (entity->type)
            {
                case GeoCsvParser::EntityType::Maximum:
                    isEntityVisible = geoData.getShowMaximum();
                    break;
                case GeoCsvParser::EntityType::Minimum:
                    isEntityVisible = geoData.getShowMinimum();
                    break;
                case GeoCsvParser::EntityType::Saddle:
                    isEntityVisible = geoData.getShowSaddles();
                    break;
                default:
                    break;
            }

            if (!isEntityVisible)
            {
                continue;
            }

            const BPoint&      point      = value.first;
            const sf::Vector2i pointPixel = localToPixel({point.x(), point.y()}, window, heatmap);

            distanceInPixels = pixelDistance(mousePixel, pointPixel);
        }
        else
        {
            const BSegment&    segment = value.first;
            const sf::Vector2i aPixel  = localToPixel({segment.first.x(), segment.first.y()}, window, heatmap);
            const sf::Vector2i bPixel  = localToPixel({segment.second.x(), segment.second.y()}, window, heatmap);

            distanceInPixels = pixelDistanceToSegment(mousePixel, aPixel, bPixel);
        }

        if (distanceInPixels < bestDistancePixel)
        {
            bestEntity        = entity;
            bestDistancePixel = distanceInPixels;
        }
    }
}

void CellTooltip::performPick(const sf::Vector2i mousePixel, sf::RenderWindow& window, const Heatmap& heatmap, const GeoData& geoData)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        m_geoPickEntity = nullptr;
        return;
    }

    const BBox localBox = getLocalPickBBox(mousePixel, m_pickRadiusPx, window, heatmap);

    const bool wantMax    = geoData.getShowMaximum();
    const bool wantMin    = geoData.getShowMinimum();
    const bool wantSad    = geoData.getShowSaddles();
    const bool wantPoints = (wantMax || wantMin || wantSad);

    const auto mode      = geoData.getDisplayMode();
    const bool linesMode = (mode == GeoData::DisplayMode::Lines);
    const bool areasMode = (mode == GeoData::DisplayMode::Areas);

    const bool wantLinesAsc  = linesMode && geoData.getShowLinesAscending();
    const bool wantLinesDesc = linesMode && geoData.getShowLinesDescending();
    const bool wantAreas     = areasMode && geoData.getShowAreas();

    // Find Points
    if (wantPoints)
    {
        float                       bestDistancePoints = std::numeric_limits<float>::infinity();
        const GeoCsvParser::Entity* bestEntityPoints   = nullptr;

        pickFromTree<PointsRTree, PointValue>(m_points, localBox, window, heatmap, geoData, mousePixel, bestEntityPoints, bestDistancePoints);

        // Early return if we found a point within the pick radius. It has priority over segments of any type
        if (bestEntityPoints && bestDistancePoints <= m_pickRadiusPx)
        {
            m_geoPickEntity  = bestEntityPoints;
            m_geoPickMousePx = mousePixel;
            m_geoPickDistPx  = bestDistancePoints;

            return;
        }
    }

    // Find lines and areas
    // Lines and areas lookup never collides since only one mode (lines or areas) is active at a time
    float                       bestDistance = std::numeric_limits<float>::infinity();
    const GeoCsvParser::Entity* bestEntity   = nullptr;

    if (wantLinesAsc)
    {
        pickFromTree<SegmentsRTree, SegmentValue>(m_lineSegsAsc, localBox, window, heatmap, geoData, mousePixel, bestEntity, bestDistance);
    }

    if (wantLinesDesc)
    {
        pickFromTree<SegmentsRTree, SegmentValue>(m_lineSegsDesc, localBox, window, heatmap, geoData, mousePixel, bestEntity, bestDistance);
    }

    if (wantAreas)
    {
        pickFromTree<SegmentsRTree, SegmentValue>(m_areaSegs, localBox, window, heatmap, geoData, mousePixel, bestEntity, bestDistance);
    }

    if (bestEntity && bestDistance <= m_pickRadiusPx)
    {
        m_geoPickEntity  = bestEntity;
        m_geoPickMousePx = mousePixel;
        m_geoPickDistPx  = bestDistance;
    }
    else
    {
        m_geoPickEntity = nullptr;
    }
}
