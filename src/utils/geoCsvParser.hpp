#ifndef GEO_CSV_PARSER_HPP
#define GEO_CSV_PARSER_HPP

#include <string>
#include <vector>

class GeoCsvParser
{
public:
    struct Point
    {
        double x = 0.0;
        double y = 0.0;
    };

    enum class GeometryType
    {
        Point,
        LineString,
        MultiLineString,
        Polygon,
        MultiPolygon,
        Unknown
    };

    struct LineString
    {
        std::vector<Point> points;
    };

    struct Polygon
    {
        std::vector<LineString> rings;
    }; // ring[0] -> outer, 1.. -> holes

    struct Geometry
    {
        GeometryType            type = GeometryType::Unknown;
        Point                   point{};  // used if (type == Point)
        std::vector<LineString> lines;    // LINESTRING -> (size = 1); MULTILINESTRING -> (size >= 1)
        std::vector<Polygon>    polygons; // POLYGON -> (size = 1); MULTIPOLYGON -> (size >= 1)
    };

    enum class EntityType
    {
        Maximum,
        Minimum,
        Saddle,
        LineAscending,
        LineDescending,
        Area,
        Unknown
    };

    struct Entity
    {
        uint32_t    id = 0;
        std::string name;
        EntityType  type = EntityType::Unknown;
        double      life = 0.0;
        std::string misc;
        Geometry    geom;
    };

    GeoCsvParser(const std::string& filepath);

    const std::vector<Entity>& getEntities() const;
    double                     getMinLife() const;
    double                     getMaxLife() const;

private:
    std::vector<Entity> m_entities;
    double              m_minLife = 0.0;
    double              m_maxLife = 0.0;

    void loadFile(const std::string& filepath);
    void findLifeMinMax();
};

#endif