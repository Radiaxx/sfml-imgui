#ifndef CELL_TOOLTIP_HPP
#define CELL_TOOLTIP_HPP

#include <optional>
#include <unordered_map>
#include <vector>

#include <SFML/Graphics.hpp>
#include <imgui.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "geoCsvParser.hpp"
#include "geoData.hpp"
#include "heatmap.hpp"

class CellTooltip
{
public:
    void draw(Heatmap& heatmap, sf::RenderWindow& window);
    void handleMouseWheelScrolled(const sf::Event::MouseWheelScrolled& event);
    void handleMouseButtonPressed(const sf::Event::MouseButtonPressed& event,
                                  Heatmap&                             heatmap,
                                  const GeoData&                       geoData,
                                  sf::RenderWindow&                    window);

    void rebuildSpatialIndex(const Heatmap& heatmap, const GeoData& geoData);

    void show(Heatmap& heatmap, sf::RenderWindow& window, const GeoData& geoData, int screenX, int screenY);
    void hide();

    sf::Vector2f wktToLocal(const GeoCsvParser::Point& point, const AscParser::Header& header);

    // Boost.Geometry aliases
    using BPoint   = boost::geometry::model::d2::point_xy<float>;
    using BBox     = boost::geometry::model::box<BPoint>;
    using BSegment = boost::geometry::model::segment<BPoint>;

    using PointValue   = std::pair<BPoint, const GeoCsvParser::Entity*>;
    using SegmentValue = std::pair<BSegment, const GeoCsvParser::Entity*>;

    using PointsRTree   = boost::geometry::index::rtree<PointValue, boost::geometry::index::rstar<16>>;
    using SegmentsRTree = boost::geometry::index::rtree<SegmentValue, boost::geometry::index::rstar<16>>;

private:
    struct CellData
    {
        int    col      = 0;
        int    row      = 0;
        double value    = 0.0;
        bool   hasValue = false;
    };

    bool     m_isVisible = false;
    CellData m_data{};

    const GeoCsvParser::Entity* m_geoPickEntity  = nullptr;
    sf::Vector2i                m_geoPickMousePx = {0, 0};
    float                       m_geoPickDistPx  = 0.f;

    float m_pickRadiusPx = 10.f;

    PointsRTree   m_points;
    SegmentsRTree m_lineSegsAsc;
    SegmentsRTree m_lineSegsDesc;
    SegmentsRTree m_areaSegs;

    BBox getLocalPickBBox(const sf::Vector2i mousePixel, float radiusPixel, const sf::RenderWindow& window, const Heatmap& heatmap);
    float        pixelDistance(sf::Vector2i a, sf::Vector2i b) const;
    sf::Vector2i localToPixel(sf::Vector2f local, const sf::RenderWindow& window, const Heatmap& heatmap) const;
    float        pixelDistanceToSegment(sf::Vector2i mousePx, sf::Vector2i Apx, sf::Vector2i Bpx) const;

    // Spatial index population methods
    void collectPoints(const Heatmap& heatmap, const GeoData& geoData, std::vector<PointValue>& out);
    void collectLineSegmentsAscending(const Heatmap& heatmap, const GeoData& geoData, std::vector<SegmentValue>& out);
    void collectLineSegmentsDescending(const Heatmap& heatmap, const GeoData& geoData, std::vector<SegmentValue>& out);
    void collectAreaSegments(const Heatmap& heatmap, const GeoData& geoData, std::vector<SegmentValue>& out);

    void addSegmentsForOpenPath(const std::vector<GeoCsvParser::Point>& points,
                                const AscParser::Header&                header,
                                const GeoCsvParser::Entity*             owner,
                                std::vector<SegmentValue>&              out);
    void addSegmentsForClosedRing(const std::vector<GeoCsvParser::Point>& ring,
                                  const AscParser::Header&                header,
                                  const GeoCsvParser::Entity*             owner,
                                  std::vector<SegmentValue>&              out);

    template <typename TreeT, typename ValueT>
    void pickFromTree(const TreeT&                 tree,
                      const BBox&                  localBox,
                      const sf::RenderWindow&      window,
                      const Heatmap&               heatmap,
                      const GeoData&               geoData,
                      const sf::Vector2i           mousePixel,
                      const GeoCsvParser::Entity*& bestEntity,
                      float&                       bestDistancePixel) const;

    void performPick(const sf::Vector2i mousePixel, sf::RenderWindow& window, const Heatmap& heatmap, const GeoData& geoData);
};

#endif