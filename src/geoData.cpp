#include <algorithm>
#include <filesystem>
#include <iostream>

#include "geoData.hpp"

GeoData::GeoData()
{
    scanDataDirectory();
}

void GeoData::draw(Heatmap& heatmap, sf::RenderWindow& window)
{
    if (!m_geoData || !heatmap.getAscData())
    {
        return;
    }

    const sf::Vector2f  viewCenter = window.getView().getCenter();
    const sf::Vector2f  viewSize   = window.getView().getSize();
    const sf::FloatRect viewRect(viewCenter - viewSize * 0.5f, viewSize);
    const sf::Sprite&   heatmapSprite = heatmap.getHeatmapSprite();
    const sf::FloatRect spriteRect    = heatmap.getHeatmapSprite().getGlobalBounds();

    // AABB Intersection between viewRect and spriteRect (world coords)
    const sf::Vector2f aMin = spriteRect.position;
    const sf::Vector2f aMax = spriteRect.position + spriteRect.size;
    const sf::Vector2f bMin = viewRect.position;
    const sf::Vector2f bMax = viewRect.position + viewRect.size;

    sf::Vector2f intersectionMin{std::max(aMin.x, bMin.x), std::max(aMin.y, bMin.y)};
    sf::Vector2f intersectionMax{std::min(aMax.x, bMax.x), std::min(aMax.y, bMax.y)};

    sf::FloatRect intersectionRect(intersectionMin, intersectionMax - intersectionMin);

    // Get intersected (visible) sprite local coords (texture space)
    const sf::Transform inv              = heatmap.getHeatmapSprite().getInverseTransform();
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

    const auto& ascHeader = heatmap.getAscData()->getHeader();

    auto wktToLocal = [&](const GeoCsvParser::Point& point) -> sf::Vector2f
    {
        const double cell   = ascHeader.cellsize;
        const double xLocal = (point.x - ascHeader.xllcorner) / cell;
        const double yTop   = ascHeader.yllcorner + static_cast<double>(ascHeader.nrows) * cell;
        const double yLocal = (yTop - point.y) / cell; // must flip Y to match sprite top left origin

        return {static_cast<float>(xLocal), static_cast<float>(yLocal)};
    };

    auto getLifeScaledPointSize = [&](double life) -> float
    {
        if (m_pointSizeScaleByLife)
        {
            const float normalizedLife = lifeToUnit(life);
            return m_pointSizeMin + normalizedLife * (m_pointSizeMax - m_pointSizeMin);
        }

        return m_pointSizeBase;
    };

    auto applyBrightness = [&](sf::Color base, double lifeValue) -> sf::Color
    {
        if (!m_lineColorScaleByLife)
        {
            return base;
        }

        const float value = lifeToUnit(lifeValue); // 0..1
        const auto  scale = [&](std::uint8_t color) -> std::uint8_t
        { return static_cast<std::uint8_t>(std::roundf(color * value)); };

        return sf::Color(scale(base.r), scale(base.g), scale(base.b), base.a);
    };

    // This actually is used for both triangles and crosses (crosses are made by two quads which each is made of two triangles).
    // The point of having a single vertex array is to have a single draw call
    std::vector<sf::Vertex> triangles;
    const int               triangleVertices = (maximumInRange().size() + minimumInRange().size()) * 3;
    const int               saddleVertices   = saddlesInRange().size() * 6 * 2;
    const int               lineVertices     = (linesAscendingInRange().size() + linesDescendingInRange().size()) * 64;

    triangles.reserve(triangleVertices + saddleVertices + lineVertices);

    auto triangleUp = [&](sf::Vector2f center, float sideLength, sf::Color color)
    {
        // Equilateral triangle with pivot at the center position
        const float        sqrt32 = 0.866025403784f; // sqrt(3) / 2
        const float        height = sqrt32 * sideLength;
        const sf::Vector2f p1(center.x, center.y - (2.f / 3.f) * height);                     // apex up
        const sf::Vector2f p2(center.x - sideLength * 0.5f, center.y + (1.f / 3.f) * height); // base left
        const sf::Vector2f p3(center.x + sideLength * 0.5f, center.y + (1.f / 3.f) * height); // base right

        triangles.push_back(sf::Vertex{p1, color});
        triangles.push_back(sf::Vertex{p2, color});
        triangles.push_back(sf::Vertex{p3, color});
    };

    auto triangleDown = [&](sf::Vector2f center, float sideLength, sf::Color color)
    {
        // Equilateral triangle with pivot at the center position
        const float        sqrt32 = 0.866025403784f; // sqrt(3) / 2
        const float        height = sqrt32 * sideLength;
        const sf::Vector2f p1(center.x, center.y + (2.f / 3.f) * height);                     // apex down
        const sf::Vector2f p2(center.x - sideLength * 0.5f, center.y - (1.f / 3.f) * height); // base left
        const sf::Vector2f p3(center.x + sideLength * 0.5f, center.y - (1.f / 3.f) * height); // base right

        triangles.push_back(sf::Vertex{p1, color});
        triangles.push_back(sf::Vertex{p2, color});
        triangles.push_back(sf::Vertex{p3, color});
    };

    auto appendRectangle = [&](sf::Vector2f start, sf::Vector2f end, float thickness, sf::Color color)
    {
        sf::Vector2f direction = end - start;
        float        length    = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (length <= 0.0001f)
        {
            return;
        }

        direction.x /= length;
        direction.y /= length;
        sf::Vector2f normal(-direction.y, direction.x);
        normal *= (thickness * 0.5f);

        const sf::Vector2f v0 = start - normal;
        const sf::Vector2f v1 = start + normal;
        const sf::Vector2f v2 = end + normal;
        const sf::Vector2f v3 = end - normal;

        // two triangles: (v0, v1, v2) and (v0, v2, v3) (two quads, so 4 triangles)
        triangles.push_back(sf::Vertex{v0, color});
        triangles.push_back(sf::Vertex{v1, color});
        triangles.push_back(sf::Vertex{v2, color});

        triangles.push_back(sf::Vertex{v0, color});
        triangles.push_back(sf::Vertex{v2, color});
        triangles.push_back(sf::Vertex{v3, color});
    };

    auto appendCircle = [&](sf::Vector2f center, float radius, sf::Color color)
    {
        if (radius <= 0.0f)
        {
            return;
        }

        const int   segments = 16;
        const float pi       = 3.14159265359f;
        const float step     = 2.0f * pi / segments;

        sf::Vector2f prev(center.x + radius, center.y);

        for (int i = 1; i <= segments; ++i)
        {
            const float        angle = i * step;
            const sf::Vector2f current(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius);

            triangles.push_back(sf::Vertex{center, color});
            triangles.push_back(sf::Vertex{prev, color});
            triangles.push_back(sf::Vertex{current, color});

            prev = current;
        }
    };

    auto cross = [&](sf::Vector2f center, float side, sf::Color color)
    {
        const float length    = side * 0.6f;
        const float thickness = std::max(2.0f, side * 0.18f);

        appendRectangle({center.x - length, center.y - length}, {center.x + length, center.y + length}, thickness, color);
        appendRectangle({center.x - length, center.y + length}, {center.x + length, center.y - length}, thickness, color);
    };

    auto isPointWithinVisibleArea = [&](const sf::Vector2f& point) -> bool
    { return !(point.x < left || point.x > right || point.y < top || point.y > bottom); };


    auto emit = [&](const std::vector<const GeoCsvParser::Entity*>& group, sf::Color color, Shape shape)
    {
        for (const auto* entity : group)
        {
            if (!entity || entity->geom.type != GeoCsvParser::GeometryType::Point)
            {
                continue;
            }

            const sf::Vector2f localPoint = wktToLocal(entity->geom.point);

            // Cull only the ones in visible sprite area
            if (!isPointWithinVisibleArea(localPoint))
            {
                continue;
            }

            const sf::Vector2f transformedPoint = heatmap.getHeatmapSprite().getTransform().transformPoint(localPoint);
            const float        sideLength       = getLifeScaledPointSize(entity->life);

            switch (shape)
            {
                case Shape::TriangleUp:
                    triangleUp(transformedPoint, sideLength, color);
                    break;
                case Shape::TriangleDown:
                    triangleDown(transformedPoint, sideLength, color);
                    break;
                case Shape::Cross:
                    cross(transformedPoint, sideLength, color);
                    break;
            }
        }
    };

    if (m_displayMode == DisplayMode::Lines)
    {
        const float thickness = std::max(1.0f, m_lineThicknessBase);

        auto emitLineString = [&](const std::vector<GeoCsvParser::Point>& path, sf::Color color, double life)
        {
            if (path.size() < 2)
            {
                return;
            }

            const sf::Color lineColor   = applyBrightness(color, life);
            const float     jointRadius = thickness * 0.5f;

            // Draw segments as rectangles
            for (std::size_t i = 0; i + 1 < path.size(); ++i)
            {
                const sf::Vector2f aLocal = wktToLocal(path[i]);
                const sf::Vector2f bLocal = wktToLocal(path[i + 1]);

                // Draw if at least one endpoint is visible in the current view
                if (!isPointWithinVisibleArea(aLocal) && !isPointWithinVisibleArea(bLocal))
                {
                    continue;
                }

                const sf::Vector2f aWorld = heatmapSprite.getTransform().transformPoint(aLocal);
                const sf::Vector2f bWorld = heatmapSprite.getTransform().transformPoint(bLocal);

                appendRectangle(aWorld, bWorld, thickness, lineColor);
            }

            // Draw joint circles at endpoints
            for (const auto& endpoint : path)
            {
                const sf::Vector2f localPosition = wktToLocal(endpoint);

                if (!isPointWithinVisibleArea(localPosition))
                {
                    continue;
                }

                const sf::Vector2f worldPosition = heatmapSprite.getTransform().transformPoint(localPosition);
                appendCircle(worldPosition, jointRadius, lineColor);
            }
        };

        // For each linestring in the geometry emit a line. Note: lines.size() is 1 for LINESTRING and >=1 for MULTILINESTRING but they are basically the same behavior
        auto emitLines = [&](const GeoCsvParser::Geometry& geometry, sf::Color color, double life)
        {
            for (const auto& lineString : geometry.lines)
            {
                if (lineString.points.size() >= 2)
                {
                    emitLineString(lineString.points, color, life);
                }
            }
        };

        if (getShowLinesAscending())
        {
            const sf::Color color = getLineAscColor();
            for (const auto* entity : linesAscendingInRange())
            {
                emitLines(entity->geom, color, entity->life);
            }
        }

        if (getShowLinesDescending())
        {
            const sf::Color color = getLineDescColor();
            for (const auto* entity : linesDescendingInRange())
            {
                emitLines(entity->geom, color, entity->life);
            }
        }
    }


    // Draw markers after lines / areas so they are always on top
    if (getShowMaximum())
    {
        emit(maximumInRange(), getMaximumColor(), Shape::TriangleUp);
    }

    if (getShowMinimum())
    {
        emit(minimumInRange(), getMinimumColor(), Shape::TriangleDown);
    }

    if (getShowSaddles())
    {
        emit(saddlesInRange(), getSaddlesColor(), Shape::Cross);
    }

    if (!triangles.empty())
    {
        window.draw(triangles.data(), triangles.size(), sf::PrimitiveType::Triangles);
    }
}

void GeoData::scanDataDirectory()
{
    const std::string geoFolderPath    = GEO_DATA_PATH;
    const std::string geoFileExtension = ".csv";
    m_geoFiles.clear();

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(geoFolderPath))
        {
            if (!entry.is_regular_file())
                continue;
            if (entry.path().extension() == geoFileExtension)
                m_geoFiles.push_back(entry.path().filename().string());
        }
        std::sort(m_geoFiles.begin(), m_geoFiles.end());
    } catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "GeoData - error scanning directory: " << e.what() << std::endl;
        throw;
    }
}

void GeoData::loadData(int fileIndex)
{
    if (fileIndex < 0 || fileIndex >= static_cast<int>(m_geoFiles.size()))
    {
        return;
    }

    m_selectedFileIndex         = fileIndex;
    const std::string& filename = m_geoFiles[m_selectedFileIndex];

    try
    {
        const std::string geoFolderPath = GEO_DATA_PATH;
        const std::string filepath      = geoFolderPath + "/" + filename;

        m_geoData = std::make_unique<GeoCsvParser>(filepath);

        m_lifeMin = m_geoData->getMinLife();
        m_lifeMax = m_geoData->getMaxLife();

        m_lifeFilterMin = m_lifeMin;
        m_lifeFilterMax = m_lifeMax;

        groupEntities();
        updateLifeFilteredGroups();

        std::cout << "GeoData loaded '" << filename << "' with " << m_geoData->getEntities().size()
                  << " entities. Life: [" << m_lifeMin << ", " << m_lifeMax << "]\n";
    } catch (const std::exception& e)
    {
        std::cerr << "Failed to load geo data: " << e.what() << std::endl;
        m_geoData.reset();
        m_groups.clear();
        m_groupsInRange.clear();
        m_lifeMin = m_lifeMax = 0.0;
        m_lifeFilterMin = m_lifeFilterMax = 0.0;
    }
}

void GeoData::groupEntities()
{
    m_groups.clear();
    if (!m_geoData)
    {
        return;
    }

    const auto& entities = m_geoData->getEntities();
    for (const auto& entity : entities)
    {
        switch (entity.type)
        {
            case GeoCsvParser::EntityType::Maximum:
                m_groups.maximum.push_back(&entity);
                break;
            case GeoCsvParser::EntityType::Minimum:
                m_groups.minimum.push_back(&entity);
                break;
            case GeoCsvParser::EntityType::Saddle:
                m_groups.saddles.push_back(&entity);
                break;
            case GeoCsvParser::EntityType::LineAscending:
                m_groups.linesAscending.push_back(&entity);
                break;
            case GeoCsvParser::EntityType::LineDescending:
                m_groups.linesDescending.push_back(&entity);
                break;
            case GeoCsvParser::EntityType::Area:
                m_groups.areas.push_back(&entity);
                break;
            default:
                break;
        }
    }
}

void GeoData::updateLifeFilteredGroups()
{
    m_groupsInRange.clear();
    if (!m_geoData)
    {
        return;
    }

    auto isInRange = [&](const GeoCsvParser::Entity* e)
    { return (e->life >= m_lifeFilterMin && e->life <= m_lifeFilterMax); };

    for (auto* entity : m_groups.maximum)
    {
        if (isInRange(entity))
        {
            m_groupsInRange.maximum.push_back(entity);
        }
    }

    for (auto* entity : m_groups.minimum)
    {
        if (isInRange(entity))
        {
            m_groupsInRange.minimum.push_back(entity);
        }
    }

    for (auto* entity : m_groups.saddles)
    {
        if (isInRange(entity))
        {
            m_groupsInRange.saddles.push_back(entity);
        }
    }

    for (auto* entity : m_groups.linesAscending)
    {
        if (isInRange(entity))
        {
            m_groupsInRange.linesAscending.push_back(entity);
        }
    }

    for (auto* entity : m_groups.linesDescending)
    {
        if (isInRange(entity))
        {
            m_groupsInRange.linesDescending.push_back(entity);
        }
    }

    for (auto* entity : m_groups.areas)
    {
        if (isInRange(entity))
        {
            m_groupsInRange.areas.push_back(entity);
        }
    }
}

void GeoData::setLifeFilterRange(double minValue, double maxValue)
{
    if (!m_geoData)
    {
        return;
    }

    if (minValue < m_lifeMin)
    {
        minValue = m_lifeMin;
    }

    if (maxValue > m_lifeMax)
    {
        maxValue = m_lifeMax;
    }

    if (minValue > maxValue)
    {
        std::swap(minValue, maxValue);
    }

    bool changed    = (minValue != m_lifeFilterMin) || (maxValue != m_lifeFilterMax);
    m_lifeFilterMin = minValue;
    m_lifeFilterMax = maxValue;

    if (changed)
    {
        updateLifeFilteredGroups();
    }
}

float GeoData::lifeToUnit(double life) const
{
    if (m_lifeFilterMax <= m_lifeFilterMin)
    {
        return 0.0f;
    }

    double normalizedLifeValue = (life - m_lifeFilterMin) / (m_lifeFilterMax - m_lifeFilterMin);

    if (normalizedLifeValue < 0.0)
    {
        normalizedLifeValue = 0.0;
    }
    if (normalizedLifeValue > 1.0)
    {
        normalizedLifeValue = 1.0;
    }

    return static_cast<float>(normalizedLifeValue);
}

#pragma region Getters and Setters

const std::vector<std::string>& GeoData::getGeoFiles() const
{
    return m_geoFiles;
}

int GeoData::getSelectedFileIndex() const
{
    return m_selectedFileIndex;
}

const GeoCsvParser* GeoData::getGeoData() const
{
    return m_geoData.get();
}

double GeoData::getLifeMin() const
{
    return m_lifeMin;
}

double GeoData::getLifeMax() const
{
    return m_lifeMax;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::maximum() const
{
    return m_groups.maximum;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::minimum() const
{
    return m_groups.minimum;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::saddles() const
{
    return m_groups.saddles;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::linesAscending() const
{
    return m_groups.linesAscending;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::linesDescending() const
{
    return m_groups.linesDescending;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::areas() const
{
    return m_groups.areas;
}

bool GeoData::getShowMaximum() const
{
    return m_toggles.maximum;
}

bool GeoData::getShowMinimum() const
{
    return m_toggles.minimum;
}

bool GeoData::getShowSaddles() const
{
    return m_toggles.saddles;
}

bool GeoData::getShowLinesAscending() const
{
    return m_toggles.linesAscending;
}

bool GeoData::getShowLinesDescending() const
{
    return m_toggles.linesDescending;
}

bool GeoData::getShowAreas() const
{
    return m_toggles.areas;
}

void GeoData::setShowMaximum(bool value)
{
    m_toggles.maximum = value;
}

void GeoData::setShowMinimum(bool value)
{
    m_toggles.minimum = value;
}

void GeoData::setShowSaddles(bool value)
{
    m_toggles.saddles = value;
}

void GeoData::setShowLinesAscending(bool value)
{
    m_toggles.linesAscending = value;
}

void GeoData::setShowLinesDescending(bool value)
{
    m_toggles.linesDescending = value;
}

void GeoData::setShowAreas(bool value)
{
    m_toggles.areas = value;
}

double GeoData::getLifeFilterMin() const
{
    return m_lifeFilterMin;
}

double GeoData::getLifeFilterMax() const
{
    return m_lifeFilterMax;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::maximumInRange() const
{
    return m_groupsInRange.maximum;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::minimumInRange() const
{
    return m_groupsInRange.minimum;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::saddlesInRange() const
{
    return m_groupsInRange.saddles;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::linesAscendingInRange() const
{
    return m_groupsInRange.linesAscending;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::linesDescendingInRange() const
{
    return m_groupsInRange.linesDescending;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::areasInRange() const
{
    return m_groupsInRange.areas;
}

GeoData::DisplayMode GeoData::getDisplayMode() const
{
    return m_displayMode;
}

void GeoData::setDisplayMode(DisplayMode value)
{
    m_displayMode = value;
}

sf::Color GeoData::getMaximumColor() const
{
    return m_colorMaximum;
}

sf::Color GeoData::getMinimumColor() const
{
    return m_colorMinimum;
}

sf::Color GeoData::getSaddlesColor() const
{
    return m_colorSaddles;
}

sf::Color GeoData::getLineAscColor() const
{
    return m_colorLineAsc;
}

sf::Color GeoData::getLineDescColor() const
{
    return m_colorLineDesc;
}

sf::Color GeoData::getAreasColor() const
{
    return m_colorAreas;
}

void GeoData::setMaximumColor(sf::Color value)
{
    m_colorMaximum = value;
}

void GeoData::setMinimumColor(sf::Color value)
{
    m_colorMinimum = value;
}

void GeoData::setSaddlesColor(sf::Color value)
{
    m_colorSaddles = value;
}

void GeoData::setLineAscColor(sf::Color value)
{
    m_colorLineAsc = value;
}

void GeoData::setLineDescColor(sf::Color value)
{
    m_colorLineDesc = value;
}

void GeoData::setAreasColor(sf::Color value)
{
    m_colorAreas = value;
}

bool GeoData::getPointSizeScaleByLife() const
{
    return m_pointSizeScaleByLife;
}

void GeoData::setPointSizeScaleByLife(bool value)
{
    m_pointSizeScaleByLife = value;
}

bool GeoData::getLineColorScaleByLife() const
{
    return m_lineColorScaleByLife;
}

void GeoData::setLineColorScaleByLife(bool value)
{
    m_lineColorScaleByLife = value;
}

float GeoData::getPointSizeBase() const
{
    return m_pointSizeBase;
}

void GeoData::setPointSizeBase(float value)
{
    m_pointSizeBase = value;
}

void GeoData::setPointSizeRange(float minValue, float maxValue)
{
    m_pointSizeMin = minValue;
    m_pointSizeMax = maxValue;
}

float GeoData::getPointSizeMin() const
{
    return m_pointSizeMin;
}

float GeoData::getPointSizeMax() const
{
    return m_pointSizeMax;
}

float GeoData::getLineThicknessBase() const
{
    return m_lineThicknessBase;
}

void GeoData::setLineThicknessBase(float value)
{
    m_lineThicknessBase = value;
}

#pragma endregion