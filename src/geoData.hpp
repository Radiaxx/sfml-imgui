#ifndef GEODATA_HPP
#define GEODATA_HPP

#include <GeoCsvParser.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

#include "heatmap.hpp"

class GeoData
{
public:
    enum class DisplayMode
    {
        Lines = 0,
        Areas = 1
    };

    enum class Shape
    {
        TriangleUp,
        TriangleDown,
        Cross,
        LineString,
        MultiLineString
    };

    GeoData();

    void GeoData::draw(Heatmap& heatmap, sf::RenderWindow& window);

    void loadData(int fileIndex);

    const std::vector<std::string>& getGeoFiles() const;
    int                             getSelectedFileIndex() const;
    const GeoCsvParser*             getGeoData() const;

    const std::vector<const GeoCsvParser::Entity*>& maximum() const;
    const std::vector<const GeoCsvParser::Entity*>& minimum() const;
    const std::vector<const GeoCsvParser::Entity*>& saddles() const;
    const std::vector<const GeoCsvParser::Entity*>& linesAscending() const;
    const std::vector<const GeoCsvParser::Entity*>& linesDescending() const;
    const std::vector<const GeoCsvParser::Entity*>& areas() const;

    double getLifeMin() const;
    double getLifeMax() const;

    double getLifeFilterMin() const;
    double getLifeFilterMax() const;
    void   setLifeFilterRange(double minValue, double maxValue); // clamps and recomputes in range groups

    const std::vector<const GeoCsvParser::Entity*>& maximumInRange() const;
    const std::vector<const GeoCsvParser::Entity*>& minimumInRange() const;
    const std::vector<const GeoCsvParser::Entity*>& saddlesInRange() const;
    const std::vector<const GeoCsvParser::Entity*>& linesAscendingInRange() const;
    const std::vector<const GeoCsvParser::Entity*>& linesDescendingInRange() const;
    const std::vector<const GeoCsvParser::Entity*>& areasInRange() const;

    bool getShowMaximum() const;
    bool getShowMinimum() const;
    bool getShowSaddles() const;
    bool getShowLinesAscending() const;
    bool getShowLinesDescending() const;
    bool getShowAreas() const;

    void setShowMaximum(bool value);
    void setShowMinimum(bool value);
    void setShowSaddles(bool value);
    void setShowLinesAscending(bool value);
    void setShowLinesDescending(bool value);
    void setShowAreas(bool value);

    DisplayMode getDisplayMode() const;
    void        setDisplayMode(DisplayMode value);

    sf::Color getMaximumColor() const;
    sf::Color getMinimumColor() const;
    sf::Color getSaddlesColor() const;
    sf::Color getLineAscColor() const;
    sf::Color getLineDescColor() const;
    sf::Color getAreasColor() const;

    void setMaximumColor(sf::Color value);
    void setMinimumColor(sf::Color value);
    void setSaddlesColor(sf::Color value);
    void setLineAscColor(sf::Color value);
    void setLineDescColor(sf::Color value);
    void setAreasColor(sf::Color value);

    bool getPointSizeScaleByLife() const;
    void setPointSizeScaleByLife(bool value);

    bool getLineColorScaleByLife() const;
    void setLineColorScaleByLife(bool value);

    float getPointSizeBase() const;
    void  setPointSizeBase(float value);

    void  setPointSizeRange(float minValue, float maxValue);
    float getPointSizeMin() const;
    float getPointSizeMax() const;

    float getLineThicknessBase() const;
    void  setLineThicknessBase(float value);

    // Normalize life to determine brightness using current filter range: life -> [0,1]
    float lifeToUnit(double life) const;

private:
    std::vector<std::string>      m_geoFiles;
    int                           m_selectedFileIndex = -1;
    std::unique_ptr<GeoCsvParser> m_geoData;

    double m_lifeMin       = 0.0;
    double m_lifeMax       = 0.0;
    double m_lifeFilterMin = 0.0;
    double m_lifeFilterMax = 0.0;

    // Grouped by type (pointers into m_geoData)
    // (for filtering by life and counts)
    struct Groups
    {
        std::vector<const GeoCsvParser::Entity*> maximum;
        std::vector<const GeoCsvParser::Entity*> minimum;
        std::vector<const GeoCsvParser::Entity*> saddles;
        std::vector<const GeoCsvParser::Entity*> linesAscending;
        std::vector<const GeoCsvParser::Entity*> linesDescending;
        std::vector<const GeoCsvParser::Entity*> areas;

        void clear()
        {
            maximum.clear();
            minimum.clear();
            saddles.clear();
            linesAscending.clear();
            linesDescending.clear();
            areas.clear();
        }
    } m_groups, m_groupsInRange;

    // Category visibility toggles
    struct Toggles
    {
        bool maximum         = true;
        bool minimum         = true;
        bool saddles         = true;
        bool linesAscending  = true;
        bool linesDescending = true;
        bool areas           = true;
    } m_toggles;

    DisplayMode m_displayMode = DisplayMode::Lines;

    sf::Color m_colorMaximum{255, 0, 0, 255};
    sf::Color m_colorMinimum{0, 0, 255, 255};
    sf::Color m_colorSaddles{0, 255, 0, 255};
    sf::Color m_colorLineAsc{255, 0, 0, 255};  // asc red
    sf::Color m_colorLineDesc{0, 0, 255, 255}; // desc blue
    sf::Color m_colorAreas{0, 200, 200, 200};

    bool m_pointSizeScaleByLife = false;
    bool m_lineColorScaleByLife = false;

    float m_pointSizeBase = 8.0f;
    float m_pointSizeMin  = 5.0f;
    float m_pointSizeMax  = 16.0f;

    float m_lineThicknessBase = 2.0f;

    void scanDataDirectory();

    void groupEntities();
    void updateLifeFilteredGroups();
};

#endif