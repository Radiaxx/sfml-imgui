#include <algorithm>
#include <filesystem>
#include <iostream>

#include "geoData.hpp"

GeoData::GeoData()
{
    scanDataDirectory();
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

        // Default filter = full dataset range
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

// float GeoData::getLineThicknessBase() const
// {
//     return m_lineThicknessBase;
// }

// void GeoData::setLineThicknessBase(float value)
// {
//     m_lineThicknessBase = value;
// }

#pragma endregion