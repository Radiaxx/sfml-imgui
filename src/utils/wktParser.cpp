#include <stdexcept>

#include "wktParser.hpp"

static std::string toLower(const std::string& str)
{
    std::string lower_str = str;

    for (char& c : lower_str)
    {
        c = std::tolower(c);
    }

    return lower_str;
}

// Trimming reference at https://stackoverflow.com/questions/216823/how-can-i-trim-a-stdstring
static void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); }));
}

static void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), s.end());
}

static void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

static GeoCsvParser::GeometryType baseGeomType(const std::string& lowerType)
{
    if (lowerType.rfind("point", 0) == 0)
    {
        return GeoCsvParser::GeometryType::Point;
    }

    if (lowerType.rfind("linestring", 0) == 0)
    {
        return GeoCsvParser::GeometryType::LineString;
    }

    if (lowerType.rfind("multilinestring", 0) == 0)
    {
        return GeoCsvParser::GeometryType::MultiLineString;
    }

    if (lowerType.rfind("polygon", 0) == 0)
    {
        return GeoCsvParser::GeometryType::Polygon;
    }

    if (lowerType.rfind("multipolygon", 0) == 0)
    {
        return GeoCsvParser::GeometryType::MultiPolygon;
    }

    return GeoCsvParser::GeometryType::Unknown;
}


WktParser::WktParser(std::string src) : wktString(std::move(src))
{
    trim(wktString);
}

GeoCsvParser::Geometry WktParser::parse()
{
    skipWhiteSpace();

    std::string type = readWord();
    auto        base = baseGeomType(type);

    skipOptionalDimensionToken();

    GeoCsvParser::Geometry g;

    switch (base)
    {
        case GeoCsvParser::GeometryType::Point:
        {
            expect('(');
            auto p = readPoint();
            expect(')');
            g.type  = GeoCsvParser::GeometryType::Point;
            g.point = p;

            break;
        }
        case GeoCsvParser::GeometryType::LineString:
        {
            g.type = GeoCsvParser::GeometryType::LineString;
            g.lines.push_back(readLineString());
            break;
        }
        case GeoCsvParser::GeometryType::MultiLineString:
        {
            g.type = GeoCsvParser::GeometryType::MultiLineString;
            expect('(');

            do
            {
                g.lines.push_back(readLineString());
            } while (consume(','));

            expect(')');

            break;
        }
        case GeoCsvParser::GeometryType::Polygon:
        {
            g.type = GeoCsvParser::GeometryType::Polygon;
            g.polygons.push_back(readPolygon());

            break;
        }
        case GeoCsvParser::GeometryType::MultiPolygon:
        {
            g.type = GeoCsvParser::GeometryType::MultiPolygon;
            expect('(');

            do
            {
                g.polygons.push_back(readPolygon());
            } while (consume(','));

            expect(')');

            break;
        }
        default:
        {
            throw std::runtime_error("WKT unsupported geometry type: " + type);
        }
    }

    skipWhiteSpace();

    if (pos != wktString.size())
    {
        std::string rest = std::string(wktString.begin() + static_cast<long>(pos), wktString.end());
        trim(rest);

        if (!rest.empty())
        {
            throw std::runtime_error("WKT unexpected trailing input: " + rest);
        }
    }

    return g;
}

void WktParser::skipWhiteSpace()
{
    while (pos < wktString.size() && std::isspace(static_cast<unsigned char>(wktString[pos])))
    {
        ++pos;
    }
}

std::string WktParser::readWord()
{
    skipWhiteSpace();
    size_t start = pos;

    while (pos < wktString.size() && std::isalpha(static_cast<unsigned char>(wktString[pos])))
    {
        ++pos;
    }

    if (start == pos)
    {
        throw std::runtime_error("WKT expected type keyword");
    }

    std::string w = wktString.substr(start, pos - start);

    return toLower(w);
}

void WktParser::skipOptionalDimensionToken()
{
    skipWhiteSpace();

    if (pos >= wktString.size())
    {
        return;
    }

    if (std::isalpha(static_cast<unsigned char>(wktString[pos])))
    {
        std::string tok = readWord();
        if (tok != "z" && tok != "m" && tok != "zm")
        {
            throw std::runtime_error("WKT unexpected token before coordinates: " + tok);
        }
    }

    skipWhiteSpace();
}

bool WktParser::consume(char c)
{
    skipWhiteSpace();

    if (pos < wktString.size() && wktString[pos] == c)
    {
        ++pos;
        skipWhiteSpace();

        return true;
    }

    return false;
}

void WktParser::expect(char c)
{
    if (!consume(c))
    {
        std::string msg = "WKT expected '";
        msg.push_back(c);
        msg += "'";

        throw std::runtime_error(msg);
    }
}

double WktParser::readNumber()
{
    skipWhiteSpace();
    size_t start = pos;

    if (pos < wktString.size() && (wktString[pos] == '+' || wktString[pos] == '-'))
    {
        ++pos;
    }

    bool hasDigit = false;
    while (pos < wktString.size() && std::isdigit(static_cast<unsigned char>(wktString[pos])))
    {
        hasDigit = true;
        ++pos;
    }

    if (pos < wktString.size() && wktString[pos] == '.')
    {
        ++pos;

        while (pos < wktString.size() && std::isdigit(static_cast<unsigned char>(wktString[pos])))
        {
            hasDigit = true;
            ++pos;
        }
    }

    if (pos < wktString.size() && (wktString[pos] == 'e' || wktString[pos] == 'E'))
    {
        ++pos;

        if (pos < wktString.size() && (wktString[pos] == '+' || wktString[pos] == '-'))
        {
            ++pos;
        }

        bool expDigit = false;
        while (pos < wktString.size() && std::isdigit(static_cast<unsigned char>(wktString[pos])))
        {
            expDigit = true;
            ++pos;
        }

        hasDigit = hasDigit && expDigit;
    }

    if (!hasDigit)
    {
        throw std::runtime_error("WKT expected number");
    }

    return std::stod(wktString.substr(start, pos - start));
}

GeoCsvParser::Point WktParser::readPoint()
{
    double x    = readNumber();
    double y    = readNumber();
    size_t save = pos;

    try
    {
        readNumber();
        save = pos;
        try
        {
            readNumber();
        } catch (...)
        {
            pos = save;
        }
    } catch (...)
    {
        pos = save;
    }

    return {x, y};
}

std::vector<GeoCsvParser::Point> WktParser::readPointList()
{
    std::vector<GeoCsvParser::Point> pts;
    expect('(');
    pts.push_back(readPoint());

    while (consume(','))
    {
        pts.push_back(readPoint());
    }

    expect(')');

    return pts;
}

GeoCsvParser::LineString WktParser::readLineString()
{
    GeoCsvParser::LineString ls;

    auto pts  = readPointList();
    ls.points = std::move(pts);

    return ls;
}

GeoCsvParser::Polygon WktParser::readPolygon()
{
    GeoCsvParser::Polygon poly;
    expect('(');

    do
    {
        auto ringPts = readPointList();
        poly.rings.push_back(GeoCsvParser::LineString{std::move(ringPts)});
    } while (consume(','));

    expect(')');

    return poly;
}

GeoCsvParser::GeometryType WktParser::baseGeomType(const std::string& lowerType)
{
    if (lowerType.rfind("point", 0) == 0)
    {
        return GeoCsvParser::GeometryType::Point;
    }

    if (lowerType.rfind("linestring", 0) == 0)
    {
        return GeoCsvParser::GeometryType::LineString;
    }

    if (lowerType.rfind("multilinestring", 0) == 0)
    {
        return GeoCsvParser::GeometryType::MultiLineString;
    }

    if (lowerType.rfind("polygon", 0) == 0)
    {
        return GeoCsvParser::GeometryType::Polygon;
    }

    if (lowerType.rfind("multipolygon", 0) == 0)
    {
        return GeoCsvParser::GeometryType::MultiPolygon;
    }

    return GeoCsvParser::GeometryType::Unknown;
}
