#pragma once
#include <memory>
#include <string>
#include <vector>

#include "GeoCsvParser.hpp"

class GeoData
{
public:
    GeoData();

    void scanDataDirectory();
    void loadData(int fileIndex);

    const std::vector<std::string>& getGeoFiles() const;
    int                             getSelectedFileIndex() const;

    const GeoCsvParser* getGeoData() const;
    double              getLifeMin() const;
    double              getLifeMax() const;

    // Grouped views (can be used for quick iteration over a specific category)
    const std::vector<const GeoCsvParser::Entity*>& maxima() const;
    const std::vector<const GeoCsvParser::Entity*>& minima() const;
    const std::vector<const GeoCsvParser::Entity*>& saddles() const;
    const std::vector<const GeoCsvParser::Entity*>& linesAscending() const;
    const std::vector<const GeoCsvParser::Entity*>& linesDescending() const;
    const std::vector<const GeoCsvParser::Entity*>& areas() const;

    bool getShowMaxima() const;
    bool getShowMinima() const;
    bool getShowSaddles() const;
    bool getShowLinesAscending() const;
    bool getShowLinesDescending() const;
    bool getShowAreas() const;

    void setShowMaxima(bool value);
    void setShowMinima(bool value);
    void setShowSaddles(bool value);
    void setShowLinesAscending(bool value);
    void setShowLinesDescending(bool value);
    void setShowAreas(bool value);

private:
    std::unique_ptr<GeoCsvParser> m_geoData;
    std::vector<std::string>      m_geoFiles;

    int m_selectedFileIndex = -1;

    double m_lifeMin = 0.0;
    double m_lifeMax = 0.0;

    struct Groups
    {
        std::vector<const GeoCsvParser::Entity*> maxima;
        std::vector<const GeoCsvParser::Entity*> minima;
        std::vector<const GeoCsvParser::Entity*> saddles;
        std::vector<const GeoCsvParser::Entity*> linesAscending;
        std::vector<const GeoCsvParser::Entity*> linesDescending;
        std::vector<const GeoCsvParser::Entity*> areas;

        void clear()
        {
            maxima.clear();
            minima.clear();
            saddles.clear();
            linesAscending.clear();
            linesDescending.clear();
            areas.clear();
        }
    } m_groups;

    struct Toggles
    {
        bool maxima          = true;
        bool minima          = true;
        bool saddles         = true;
        bool linesAscending  = true;
        bool linesDescending = true;
        bool areas           = true;
    } m_toggles;

    void groupEntities();
};
