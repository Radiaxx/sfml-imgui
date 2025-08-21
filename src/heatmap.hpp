#ifndef HEATMAP_HPP
#define HEATMAP_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

#include "utils/AscParser.hpp"

class Heatmap
{
public:
    Heatmap();

    void loadData(int fileIndex);
    void setCurrentColormapID(int id);

    const std::unique_ptr<AscParser>& getAscData() const;
    const std::vector<std::string>&   getDataFiles() const;
    const std::vector<std::string>&   getColormapNames() const;
    int                               getSelectedFileIndex() const;
    int                               getCurrentColormapID() const;
    const sf::Texture&                getHeatmapTexture() const;
    const sf::Sprite&                 getHeatmapSprite() const;
    sf::Shader&                       getHeatmapShader();

private:
    std::unique_ptr<AscParser> m_ascData;
    std::vector<std::string>   m_dataFiles;
    std::vector<std::string>   m_colormapNames;

    int m_selectedFileIndex = -1;
    int m_currentColormapID = 0;

    sf::Texture m_heatmapTexture;
    sf::Sprite  m_heatmapSprite;
    sf::Shader  m_heatmapShader;

    void scanDataDirectory();
    void createHeatmapTexture();
};

#endif