#ifndef HEATMAP_HPP
#define HEATMAP_HPP

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <ascParser.hpp>
#include <memory>
#include <string>
#include <vector>

class Heatmap
{
public:
    static inline const std::vector<std::string> COLORMAP_NAMES =
        {"Blue-to-Red", "Grayscale", "Jet", "Turbo", "Viridis", "Plasma", "Inferno", "Magma", "Gist Earth", "Terrain"};

    Heatmap();
    ~Heatmap();

    void draw(const sf::View& view);

    void loadData(int fileIndex);
    void unloadData();
    void resetAscSettingsToDefaults();

    void setCurrentColormapID(int id);
    void updateHeatmapView(sf::View view);

    void setAutoClamp(bool enabled);
    void setManualClampRange(float min, float max);
    void calculateAutoClamp(const sf::View& view);

    const std::unique_ptr<AscParser>& getAscData() const;
    const std::vector<std::string>&   getDataFiles() const;

    int getSelectedFileIndex() const;
    int getCurrentColormapID() const;

    const sf::Texture& getHeatmapTexture() const;
    const sf::Sprite&  getHeatmapSprite() const;
    sf::Shader&        getHeatmapShader();

    bool  isAutoClamping() const;
    float getGlobalMin() const;
    float getGlobalMax() const;
    float getManualClampMin() const;
    float getManualClampMax() const;
    float getCurrentClampMin() const;
    float getCurrentClampMax() const;

private:
    std::unique_ptr<AscParser> m_ascData;
    std::vector<std::string>   m_dataFiles;

    int m_selectedFileIndex = -1;
    int m_currentColormapID = 0;

    sf::Texture m_heatmapTexture; // Float asc data
    sf::Sprite  m_heatmapSprite;
    sf::Shader  m_heatmapShader;

    bool  m_isAutoClamping  = false;
    float m_globalMin       = 0.f;
    float m_globalMax       = 0.f;
    float m_manualClampMin  = 0.f;
    float m_manualClampMax  = 0.f;
    float m_currentClampMin = 0.f;
    float m_currentClampMax = 0.f;

    void scanDataDirectory();
    void updateHeatmapTexture();
};

#endif