#include "geoData.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>

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
            {
                continue;
            }

            if (entry.is_regular_file() && entry.path().extension() == geoFileExtension)
            {
                m_geoFiles.push_back(entry.path().filename().string());
            }
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
        m_geoData                       = std::make_unique<GeoCsvParser>(filepath);

        m_lifeMin = m_geoData->getMinLife();
        m_lifeMax = m_geoData->getMaxLife();

        groupEntities();

        std::cout << "GeoData loaded '" << filename << "' with " << m_geoData->getEntities().size()
                  << " entities. Life: [" << m_lifeMin << ", " << m_lifeMax << "]\n";

    } catch (const std::exception& e)
    {
        std::cerr << "Failed to load geo data: " << e.what() << std::endl;
        m_geoData.reset();
        m_groups.clear();
        m_lifeMin = m_lifeMax = 0.0;
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
    m_groups.maxima.reserve(entities.size());
    m_groups.minima.reserve(entities.size());
    m_groups.saddles.reserve(entities.size());
    m_groups.linesAscending.reserve(entities.size());
    m_groups.linesDescending.reserve(entities.size());
    m_groups.areas.reserve(entities.size());

    for (const auto& entity : entities)
    {
        switch (entity.type)
        {
            case GeoCsvParser::EntityType::Maximum:
                m_groups.maxima.push_back(&entity);
                break;
            case GeoCsvParser::EntityType::Minimum:
                m_groups.minima.push_back(&entity);
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

const std::vector<const GeoCsvParser::Entity*>& GeoData::maxima() const
{
    return m_groups.maxima;
}

const std::vector<const GeoCsvParser::Entity*>& GeoData::minima() const
{
    return m_groups.minima;
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

bool GeoData::getShowMaxima() const
{
    return m_toggles.maxima;
}

bool GeoData::getShowMinima() const
{
    return m_toggles.minima;
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

void GeoData::setShowMaxima(bool value)
{
    m_toggles.maxima = value;
}

void GeoData::setShowMinima(bool value)
{
    m_toggles.minima = value;
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
