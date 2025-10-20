#include <fstream>
#include <unordered_map>

#include "geoCsvParser.hpp"
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

// Remove surrounding quotes
std::string unquote(const std::string& in)
{
    if (in.size() >= 2 && in.front() == '"' && in.back() == '"')
    {
        std::string out;
        out.reserve(in.size() - 2);

        for (size_t i = 1; i + 1 < in.size(); ++i)
        {
            if (in[i] == '"' && i + 1 < in.size() - 1 && in[i + 1] == '"')
            {
                out.push_back('"');
                ++i; // consume escaped quote
            }
            else
            {
                out.push_back(in[i]);
            }
        }

        return out;
    }

    return in;
}

std::vector<std::string> splitCSVLine(const std::string& line)
{
    std::vector<std::string> fields;
    std::string              curr;
    bool                     inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];

        if (inQuotes)
        {
            if (c == '"')
            {
                if (i + 1 < line.size() && line[i + 1] == '"')
                {
                    curr.push_back('"');
                    ++i;
                } // escaped quote
                else
                {
                    inQuotes = false;
                }
            }
            else
            {
                curr.push_back(c);
            }
        }
        else
        {
            if (c == '"')
                inQuotes = true;
            else if (c == ';')
            {
                fields.emplace_back(std::move(curr));
                curr.clear();
            }
            else
            {
                curr.push_back(c);
            }
        }
    }

    fields.emplace_back(std::move(curr));

    return fields;
}

GeoCsvParser::EntityType parseEntityType(std::string v)
{
    v = toLower(v);

    if (v == "maximum")
    {
        return GeoCsvParser::EntityType::Maximum;
    }

    if (v == "minimum")
    {
        return GeoCsvParser::EntityType::Minimum;
    }

    if (v == "saddle")
    {
        return GeoCsvParser::EntityType::Saddle;
    }

    if (v == "line-ascending")
    {
        return GeoCsvParser::EntityType::LineAscending;
    }

    if (v == "line-descending")
    {
        return GeoCsvParser::EntityType::LineDescending;
    }

    if (v == "area")
    {
        return GeoCsvParser::EntityType::Area;
    }

    return GeoCsvParser::EntityType::Unknown;
}

GeoCsvParser::GeoCsvParser(const std::string& filepath)
{
    loadFile(filepath);
    findLifeMinMax();
}

void GeoCsvParser::loadFile(const std::string& filepath)
{
    std::ifstream in(filepath);
    if (!in.is_open())
    {
        throw std::runtime_error("GeoCsvParser: cannot open file: " + filepath);
    }

    std::string headerLine;
    if (!std::getline(in, headerLine))
    {
        throw std::runtime_error("GeoCsvParser: empty file: " + filepath);
    }

    auto header = splitCSVLine(headerLine);
    for (auto& f : header)
    {
        trim(f);
        f = toLower(f);
    }

    std::unordered_map<std::string, size_t> headerMap;
    for (size_t i = 0; i < header.size(); ++i)
    {
        trim(header[i]);
        header[i]            = toLower(header[i]);
        headerMap[header[i]] = i;
    }

    const char* requiredColumns[] = {"id", "name", "type", "life", "misc", "geom"};
    for (auto column : requiredColumns)
    {
        if (!headerMap.count(column))
        {
            throw std::runtime_error(std::string("GeoCsvParser: missing header '") + column + "'");
        }
    }

    std::string line;
    uint64_t    currentLineIndex = 1; // already consumed header
    m_entities.clear();

    while (std::getline(in, line))
    {
        ++currentLineIndex;
        if (line.empty())
        {
            continue;
        }

        auto cols = splitCSVLine(line);
        auto get  = [&](const char* k) -> std::string
        {
            auto it = headerMap.find(k);

            if (it == headerMap.end() || it->second >= cols.size())
            {
                throw std::runtime_error("GeoCsvParser: missing col '" + std::string(k) + "'");
            }

            return cols[it->second];
        };

        uint32_t    id      = 0;
        std::string idStr   = get("id");
        std::string name    = unquote(get("name"));
        std::string typeStr = get("type");
        std::string lifeStr = get("life");
        std::string misc    = unquote(get("misc"));
        std::string wkt     = unquote(get("geom"));

        try
        {
            size_t        index = 0;
            unsigned long val   = std::stoul(idStr, &index);

            if (index != idStr.size())
            {
                throw std::runtime_error("trailing chars");
            }

            id = static_cast<uint32_t>(val);
        } catch (...)
        {
            throw std::runtime_error(
                "GeoCsvParser: invalid id at line " + std::to_string(currentLineIndex) + ": '" + idStr + "'");
        }

        auto   entityType = parseEntityType(typeStr);
        double life       = 0.0;

        try
        {
            size_t index = 0;
            life         = std::stod(lifeStr, &index);

            if (index != lifeStr.size())
            {
                throw std::runtime_error("trailing chars");
            }
        } catch (...)
        {
            throw std::runtime_error(
                "GeoCsvParser: invalid life at line " + std::to_string(currentLineIndex) + ": '" + lifeStr + "'");
        }

        Geometry geom = WktParser(wkt).parse();
        m_entities.push_back(Entity{id, std::move(name), entityType, life, std::move(misc), std::move(geom)});
    }
}

void GeoCsvParser::findLifeMinMax()
{
    if (m_entities.empty())
    {
        m_minLife = m_maxLife = 0.0;

        return;
    }

    m_minLife = std::numeric_limits<double>::infinity();
    m_maxLife = -std::numeric_limits<double>::infinity();

    for (const auto& entity : m_entities)
    {
        m_minLife = std::min(m_minLife, entity.life);
        m_maxLife = std::max(m_maxLife, entity.life);
    }
}

const std::vector<GeoCsvParser::Entity>& GeoCsvParser::getEntities() const
{
    return m_entities;
}

double GeoCsvParser::getMinLife() const
{
    return m_minLife;
}

double GeoCsvParser::getMaxLife() const
{
    return m_maxLife;
}
