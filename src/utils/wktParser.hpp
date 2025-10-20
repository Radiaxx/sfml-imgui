#ifndef WKT_PARSER_HPP
#define WKT_PARSER_HPP

#include <string>

#include "geoCsvParser.hpp"

class WktParser
{
public:
    WktParser(std::string src);
    GeoCsvParser::Geometry parse();

private:
    std::string wktString;
    size_t      pos = 0;

    void        skipWhiteSpace();
    std::string readWord();
    void        skipOptionalDimensionToken();
    bool        consume(char c);
    void        expect(char c);
    double      readNumber();

    GeoCsvParser::Point              readPoint();
    std::vector<GeoCsvParser::Point> readPointList();
    GeoCsvParser::LineString         readLineString();
    GeoCsvParser::Polygon            readPolygon();
    GeoCsvParser::GeometryType       baseGeomType(const std::string& lowerType);
};

#endif
