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
    static inline const std::vector<std::string> COLORMAP_NAMES =
        {"Blue-to-Red", "Grayscale", "Jet", "Turbo", "Viridis", "Plasma", "Inferno", "Magma", "Gist Earth", "Terrain"};

    Heatmap();

    void loadData(int fileIndex);
    void setCurrentColormapID(int id);
    void updateHeatmapView(sf::View view);

    const std::unique_ptr<AscParser>& getAscData() const;
    const std::vector<std::string>&   getDataFiles() const;
    int                               getSelectedFileIndex() const;
    int                               getCurrentColormapID() const;
    const sf::Texture&                getHeatmapTexture() const;
    const sf::Sprite&                 getHeatmapSprite() const;
    sf::Shader&                       getHeatmapShader();

private:
    std::unique_ptr<AscParser> m_ascData;
    std::vector<std::string>   m_dataFiles;

    int m_selectedFileIndex = -1;
    int m_currentColormapID = 0;

    sf::Texture m_heatmapTexture;
    sf::Sprite  m_heatmapSprite;
    sf::Shader  m_heatmapShader;

    void scanDataDirectory();
    void createHeatmapTexture();
};

#endif